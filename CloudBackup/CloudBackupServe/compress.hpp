
//此头文件需要Zlib压缩文件的库支持，linux下自带

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <sys/stat.h>
#include <zlib.h>
#include <pthread.h>
#include <sys/file.h>

#define SERVER_BASE_DIR "www"        //客户端存储备份文件的根目录
#define UNGIZPFILE_PATH "www/list/"  //客户端的备份文件存储在此目录下
#define GZIPFILE_PATH "www/zip/"     //如果这个备份文件是低热度文件，则存储在此目录下
#define RECORD_FILE "record.list"    //文件的备份信息存储路径
#define HEAT_TIME       180          //客户端的备份文件，如果超过了这个时间则对此文件进行压缩
#define S               3            //每隔3s检查一下有没有需要压缩的文件和更新备份信息文件一次

namespace bf = boost::filesystem;

class CompressStore
{
public:
	CompressStore()
	{                                               
		bf::path base(SERVER_BASE_DIR);           //获取一下备份文件的根目录路径
		if(!bf::exists(base))                     //如果此根目录不存在，则创建此目录
		{
			bf::create_directory(SERVER_BASE_DIR);
		}
	
		bf::path path(GZIPFILE_PATH);             //获取一下存储压缩文件的路径
		if(!bf::exists(path))                     //判断此目录是否存在
		{
			bf::create_directory(GZIPFILE_PATH);  //如果不存在则创建
		}

		pthread_rwlock_init(&_rwlock, NULL);      //初始化读写锁
	}

	~CompressStore()
	{
		pthread_rwlock_destroy(&_rwlock);         //销毁读写锁
	}

    //用来获取备份文件在服务端的存储路径
	bool GetFileList(std::vector<std::string>& list)
	{
		pthread_rwlock_rdlock(&_rwlock);    //在获取备份信息的文件列表时，要先加读锁，允许其它线程，也就是客户端
                                            //过来获取文件列表，同时也防止在有客户端读取文件列表时
                                            //有线程向文件备份信息中写入数据
		for(auto i : _file_list)            //获取文件备份信息就是，获取现在哈希表中存储的备份信息，因为文件中的
        {                                   //备份信息，并不是立即更新的，可能客户端上传文件
			list.push_back(i.first);        //后要等待一段时间，才能写入到文件中
		}                                   //按照哈希表中的存储格式，key值就是文件所在路径，所以就可以将key写入到
                                            //vector中来获取文件列表
		pthread_rwlock_unlock(&_rwlock);    //当文件列表获取成功后，就可以解开这个线程加入的读锁了

		return true;                        //因为vector是传引用过来的，所以可以直接获取到文件列表，然后返回true
	}

    //用来将客户端传过来的分块备份到本地的函数
	bool SetFileData(const std::string &file, const std::string &body, const int64_t offset) 
	{
		int fd = open(file.c_str(), O_CREAT|O_WRONLY, 0664); //如果这个备份文件不存在，则创建，如果存在则打开
		if(fd < 0)                                           //如果文件描述符小于0，则表示文件创建失败
		{                                                    //然后提醒服务端文件打开失败，并返回false
			std::cerr << "open file " << file << "error\n";
			return false;
		}

		flock(fd, LOCK_EX);  
        //文件锁，当一个线程正在对此文件上传分块信息时，其它线程不允许对此文件上传分块数据
		lseek(fd, offset, SEEK_SET);               //设置用户备份的这个分块，从指定位置开始备份，偏移量就是
        //文件的起始位置，从文件开头开始偏移，因为客户端是采用多线程的方式上传分块的，所以先上传哪个分块并不确定
		int ret = write(fd, &body[0], body.size());//将这个分块的数据向文件中指定的位置写入
		if(ret < 0)                                //如果write返回的值小于0，说明写入数据失败
		{                                          //则提醒服务端分块上传失败
			std::cerr << "store file " << file << " data error\n"; //然后解开文件锁
		    flock(fd, LOCK_UN);                    //并关闭文件，返回false
            close(fd);
			return false;
		}
		flock(fd, LOCK_UN);                        //如果上传分块成功，则先解开文件锁，让下一个线程赶紧上传分块
		close(fd);                                 //然后再关闭文件
		AddFileRecord(file, "");                   //最后再添加一下这个文件的备份信息，因为此是文件不需要压缩
		return true;                               //所以将压缩路径置为空
	}
    
    //用于让客户端获取备份的文件
	bool GetFileData(std::string &file, std::string &body)
	{
		if(bf::exists(file))            //首先查看一下客户端请求的文件是否在备份目录下
		{                               //如果在，那么就用body来获取此文件数据
			GetNormalFile(file, body);  //因为文件可能是低热度文件被压缩了
		}                               //所以分了二种情况，如果请求的文件在这个备份目录
		else                            //说明不是低热度文件，可以正常获取文件数据
		{
			std::string gzip;           //如果是低热度文件，被压缩了
			GetFileGzip(file, gzip);    //那么就先获取一下压缩文件的路径
			UnCompressFile(gzip, file); //然后将此文件解压
			GetNormalFile(file, body);  //之后再获取正常文件数据
		}
		return true;                    //再返回true
	}

    //用于检查备份文件中有没有低热度文件，如果有那么就压缩此低热度文件
	bool LowHeatFileStore()
	{
		GetListRecord();             //先将备份信息文件中的数据，存储在哈希表中，以备之后使用
                                     //因为这个线程是先执行的，而且服务端是等待客户端，所以有一定的缓冲时间
                                     //所以可以将获取备份信息的函数放在这里执行
		while(1)                     //然后每隔一段时间检查一次是否有需要压缩的文件
		{
			DirectoryCheck();
			SetListRecord();         //并且也更新一次备份信息文件的数据
			sleep(S);
		}
	} 
                                         
private:
    //用来将备份信息文件中的数据写入到哈希表中
	bool GetListRecord()
	{
		                                  //备份信息文件中的存储格式：filepath gzipfilepath\n
		bf::path name(RECORD_FILE);       //首先获取备份信息文件的路径
		if(!bf::exists(name))             //如果此路径不存在，则返回false
		{
			return false;
		}

		std::ifstream file(RECORD_FILE, std::ios::binary);  //如果有备份信息文件，说明服务端有客户端备份的文件
                                                            //所以以二进制流的形式，打开此文件
		if(!file.is_open())                                 //如果这个文件打开错误，则提醒服务端文件打开错误
		{
			std::cerr << "record file body read error\n";
			return false;                                   //然后返回false
		}

		int64_t fsize = bf::file_size(name);        //创建一个变量，用来存储这个备份信息文件的大小
		std::string body;                           //方便创建一个string变量申请这么大的空间，来临时保存备份信息
		body.resize(fsize);                         
		file.read(&body[0], fsize);                 //将这个备份信息文件中的数据临时读入到body中
		if(!file.good())                            //如果这个备份信息文件读取成功
		{                                           //那么就提醒服务端备份信息文件读取错误
			std::cerr << "record file body read error\n";
			file.close();                           //然后关闭文件，并返回false
			return false;
		}

		file.close();                               //如果读取成功，则直接关闭文件 
		std::vector<std::string> list;              //然后创建一个vector将所有的备份信息，全部保存
                                                    //先做一个简单的分离操作，以便存入到哈希表中
		boost::split(list, body, boost::is_any_of("\n")); //每条备份信息都是以\n为标志的
                                                          //所以可以利用它来分离每条备份信息 
		for(auto  i : list)                         //然后遍历整个vector将每条备份信息存入哈希表中
		{
			size_t pos =  i.find(" ");              //用每个备份文件在服务端的存储路径作为key值
                                                    //用此文件的压缩路径作为val，存储在哈希表中
                                                    //它们之间都是以一个空格作为分隔的
			if(pos == std::string::npos)            //如果没有找到哪条备份信息的空格，那么就说明这条备份信息无效
			{                                       //继续记录下一条备份信息
				continue;
			}

			std::string key = i.substr(0, pos);     //key值就是从起始位置开始，到空格结束的位置
			std::string val = i.substr(pos + 1);    //val值是从空格开始到这条备份信息结束的位置
			_file_list[key] = val;                  
		}

		return true;
	}
		
    //用来更新备份信息文件
	bool SetListRecord()
	{
		std::stringstream tmp;//先创建一个字符串流对象，用来将哈希表中的备份信息组织成备份信息文件需要的格式字符串
		for(auto i : _file_list) //遍历哈希表，将哈希表中的备份信息以文件需要的格式，存储在字符串流对象中
		{                        //文件中的备份信息格式："FilePath ZipFilePath"
			tmp << i.first << " " << i.second << "\n";
		}
                                                             //然后以二进制流，截断的方式打开备份信息文件 
		std::ofstream file(RECORD_FILE, std::ios::binary|std::ios::trunc); //每次更新备份信息都是全部更新
		if(!file.is_open())                                  //如果备份信息文件打开成功
		{                                                    //那么就是提醒服务端文件打开失败，然后返回false
			std::cerr << "record file open error\n"; 
			return false;
		}
		file.write(tmp.str().c_str(), tmp.str().size());     //最后就直接将组织好的备份信息字符串存入文件中
		if(!file.good())                                     //如果写入失败
		{                                                    //则提醒服务端备份文件信息写入失败
			std::cerr << "record file write body error\n";   //然后关闭文件，再返回false
            file.close();
			return false;
		}
		file.close();                                        //如果备份信息更新成功，则直接关闭文件，返回true
		return true;
	}
	
    //用来判断此文件是否是低热度文件	
	bool IsNeedCompress(std::string &file)
	{
		struct stat st;                         //定义一个stat的结构体来获取文件的状态信息
		if(stat(file.c_str(), &st) < 0)         //如果stat返回值小于0，说明获取文件状态信息失败
		{                                       //那么就提醒服务端获取文件状态信息失败
			std::cerr << "get file:[" << file << "]stat error\n";
			return false;
		}

		time_t cur_time = time(NULL);          //获取现在的时间，用来计算是否超过不处于低热度状态的时间
		time_t acc_time = st.st_atime;         //获取文件最后一次被操作的时间
		if((cur_time - acc_time) < HEAT_TIME)  //现在的时间减去文件最后一次被操作的时间如果小于低热度时间	
		{                                      //那么就是返回false，说明此文件不需要压缩，是活跃的文件
			return false;
		}
			
		return true;                           //否则返回true，说明文件是低热度文件
	}

    //用来压缩低热度文件
	bool CompressFile(std::string &file, std::string &gzip)
	{
		int fd = open(file.c_str(), O_RDONLY);       //打开需要被压缩的低热度文件
		if(fd < 0)                                   //如果获取的文件描述符小0，说明此文件打开失败
		{                                            //则提醒服务端文件打开失败
			std::cerr << "com open file:[" << file << "] error\n";
			return false;
		}

		gzFile gf = gzopen(gzip.c_str(), "wb");      //以只写和二进制流的方式打开此用于的压缩文件
                                                     //如果此文件不存在则会创建
		if(gf == NULL)                               //如果返回的gf是空，则说明此用于压缩的文件，打开错误
		{	                                         //那么就提醒服务端哪个文件打开错误
			std::cerr << "com open gzip:[" << gzip << "] error\n";
            close(fd);                               //然后关闭原文件，并返回false
			return false;
		}

		int ret;                                    //定义一个变量用来存储每次读取到的文件数据大小
		char buf[1024];                             //因为原文件可能很大，所以要经过多次读取和写入
                                              //客户端在传输文件时是分块传输的，所以我们接收文件时，没有这个问题
                                                    //然后再创建一个缓冲区用来存储读入的数据
		flock(fd, LOCK_SH);                       //在读取数据前，先加上一个文件锁，防止其它线程向此文件写入数据
		while((ret = read(fd, buf, 1024)) > 0)      //循环读取文件的数据
		{                                           //每读取一次就调用压缩函数向压缩文件中写入一次数据
			gzwrite(gf, buf, ret);
		}
		flock(fd, LOCK_UN);                         //当文件压缩完成后，就先解开文件锁
		close(fd);                                  //然后关闭原文件和压缩文件
		gzclose(gf);
		unlink(file.c_str());                       //当文件被压缩后，就可以删除原文件了

		return true;                                
	}

    //用来解压备份文件，还原原始数据
	bool UnCompressFile(std::string &gzip, std::string &file)
	{
		int fd = open(file.c_str(), O_CREAT|O_WRONLY, 0664); //创建一个用于接收解压后的原始数据的文件
		if(fd < 0)                                          //如果fd小于0，则提醒服务端，创建文件失败，并返回false
		{
			std::cerr << "open file " << file << "failed\n";
			return false;
		}

		gzFile gf = gzopen(gzip.c_str(), "rb");             //然后以只读和二进制流的方式打开需要解压的文件
		if(gf == NULL)                                      //如果没有打开压缩文件
		{                                                   //则提醒用户打开压缩文件错误，并返回false
			std::cerr << "open gzip " << gzip << "failed\n";
			return false;
		}
		int ret;                                     //创建一个变量用于存储，从压缩文件中解压出来的数据大小
		char buf[1024] = {0};                        //创建一个缓冲区，用来存储从压缩文件中读出的数据
		flock(fd, LOCK_EX);                          //然后对存储原始数据的文件加上文件锁，防止其它线程同时操作
                                                     //这个文件，向里面写入数据
		while((ret = gzread(gf, buf, 1024))  > 0)    //然后开始从压缩文件中读取数据，因为压缩文件可能很大
		{                                            //所以循环读入，因为客户端上传是分块上传，所以没这个问题
			int len = write(fd, buf, ret);           //每读入一次，就将解压出来的数据写入文件中
			if(len < 0)                              //如果写入的数据小于，说明写入数据错误
			{                                        //那么提醒服务端写入数据错误
				std::cerr << "get gzip data failed\n";
				gzclose(gf);                         //然后关闭压缩文件，和接收数据的文件，并解开文件锁
				close(fd);
				flock(fd, LOCK_UN);
				return false;
			}
		}

		flock(fd, LOCK_UN);                          //如果解压数据都成功了，那么先解开文件锁
		gzclose(gf);                                 //然后关闭压缩文件和接收数据的文件
		close(fd);
		unlink(gzip.c_str());                        //因为这个压缩文件已经解压，所以没有存在的必要了，就删除了它
		return true;
	}
		
	//当一个分块备份完成后，就要添加一下备份信息			
	bool AddFileRecord(const std::string file, const std::string &gzip)
	{
		pthread_rwlock_wrlock(&_rwlock);   //添加备份信息时，要加写锁，如果多个线程一起添加备份信息就乱了
		_file_list[file] = gzip;           //添加的备份信息就先添加到哈希表中，添加方法是
                                           //用用文件在服务端的路径作为key值，将此文件的压缩路径，作为值
		pthread_rwlock_unlock(&_rwlock);   //当备份信息添加完成后，就解开写锁，让其它线程添加备份信息
		return true;                       //然后返回true
	}
		
    //用来获取客户端请求的文件数据
	bool GetNormalFile(std::string &name, std::string &body)
	{
		int64_t fsize = bf::file_size(name);       //先获取此文件的大小
		body.resize(fsize);                        //以便申请足够的空间来存储这个备份数据
		int fd = open(name.c_str(), O_RDONLY);     //然后以只读的方式打开这个备份文件
		if(fd < 0)                                 //如果fd小于0，说明文件打开失败
		{                                          //那么提醒服务端文件打开失败，然后返回false
			std::cerr << "open file " << name << " failed\n";
			return false;
		}

		flock(fd, LOCK_SH);                       //然后获取备份文件数据加一个互斥的文件锁
                                                  //防止有线程过来向此文件写入数据或更新数据
		int ret = read(fd, &body[0], fsize);      //然后将此文件的内容读取到body中，然后返回给客户端
		flock(fd, LOCK_UN);                       //再解开文件锁
		if(ret != fsize)                          //如果读取的文件大小不等于之前获取的文件大小
		{                                         //则提醒服务端文件获取错误，然后关闭文件，并返回false
			std::cerr << "get file " << name << "body error\n";
			close(fd);
			return false;
		}

		return true;
	}
				
    //用来获取备份文件的压缩文件路径
	bool GetFileGzip(std::string &file, std::string &gzip)
	{
		pthread_rwlock_rdlock(&_rwlock);  //在获取压缩文件路径时，先加读锁，让其它线程不能加写锁修改哈希表中的数据
		auto it = _file_list.find(file);     //然后在哈希表中找到这个文件的备份信息
		if(it == _file_list.end())           //如果这个文件不存在，则先解开读锁，然后再返回false
		{
			pthread_rwlock_unlock(&_rwlock);
			return false;
		}
			gzip = it->second;               //如果这个文件存在，此获取备份信息中压缩文件路径，也就是val
			pthread_rwlock_unlock(&_rwlock); //然后解开读锁，返回true
			return true;
	}
		
    //检查备份文件的目录，判断是否有需要压缩的文件
	bool DirectoryCheck()
	{
		if(!bf::exists(UNGIZPFILE_PATH))                    //先检查存储压缩文件的目录是否存在
		{
			bf::create_directory(UNGIZPFILE_PATH);          //如果此目录不存在，则创建此目录
		}

		bf::directory_iterator item_begin(UNGIZPFILE_PATH); //获取备份文件目录中第一个文件的位置
		bf::directory_iterator item_end;                    //获取备份文件目录中最后一个文件后面的位置
		for(; item_begin != item_end; ++item_begin)         //用送代器，遍历整个备份文件目录
		{
			if(bf::is_directory(item_begin->status()))      //如果当前检查的文件是一个目录，那么就不管
			{                                               //因为我们只把文件存储在一个目录下
				continue;
			}
			std::string name = item_begin->path().string(); //然后获取一下检查的这个文件的路径
			if(IsNeedCompress(name))                        //调用函数判断此文件是否需要被压缩
			{                                               //如果需要被压缩，则创建一个string对象
				std::string gzip = GZIPFILE_PATH + item_begin->path().filename().string() + ".gz"; 
				CompressFile(name, gzip);                   //用来存储此压缩文件的路径，然后调用函数压缩此文件
				AddFileRecord(name, gzip);                  //再设置一下此文件备份信息
			}
		}
			return true;
	}
									

private:
	std::string _file_dir;
	std::unordered_map<std::string, std::string> _file_list; //创建一个哈希表，用于存储文件的备份信息，不直接添
//加到备份文件中的原因是，首先客户端是分块上传文件的，所以如果直接备份到文件，会进行多次I/O操作，其次可能同时有
//多个客户端上传备份文件，所以先将它们保存在哈希表中，每过一段时间向备份信息文件中写入备份信息是比较好的选择，间
//隔时间，刚好用每次循环检测低热度的时间来充当
	pthread_rwlock_t _rwlock;                 //读写锁，每来一个客户端openssl都会产生一个线程来处理他们的请求
                                              //所以为了让客户端在读文件，也就获取资源时，可以一起共享资源
                                              //修改资源时，只能有一个客户端修改，且这时没有读者
                                              //所以使用读写锁，实现读共享写互斥
};
		
