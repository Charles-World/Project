
//在使用本项目之前需要用到boost_1_70_0和OpenSSL-Win64
//vs下配置方法：在调试下的属性对话中设置"C/C++" -> "常规" -> "附加包含目录" D:\库\boost_1_70_0和D:\库\OpenSSL-Win64\include
//"链接器" -> "常规" -> "附加库目录" -> D:\库\boost_1_70_0\lib64-msvc-14.1和D:\库\OpenSSL-Win64\lib
//"链接器" -> "输入" -> "附加依赖项" libssl.lib和libcrypto.lib
//另外还要加入一个httplib的头的文件，方便使用http协议传输数据
//从服务器上获取文件时，注意使用https协议，浏览器默认是http，如果使用http是访问不到服务器的

#pragma once   //只包含一次这些头文件

#define _SCL_SECURE_NO_WARNINGS       //在vs中认为一些c++函数是不安全，所以要加上这个宏，就不会报错，可以继续使用这个函数
#define CPPHTTPLIB_OPENSSL_SUPPORT    //只有定义了这个宏才可以使用httplib中的SSL服务，它必须得定义在httplib头文件上面
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include "httplib.h"

#define SERVER_PORT 9000               //服务端使用的端口号
#define SERVER_IP "192.168.184.128"    //服务端的IP地址
#define RANGE_MAX_SIZE (10 << 20)        //分块传输的最大长度是10M，1左移20位相当于乘20个2，也就是2 ^ 20
#define CLIENT_BACKUP_DIR "backup"     //备份的文件相对路径，只要将文件放在此文件夹下，就可以自动备份
#define CLIENT_BACKUP_INFO_FILE "back.list" //备份日志相对路径，这个文件夹用来存储备份文件的信息
#define BACKUP_URI "/list/"                 //服务端用来接收，客户端备份文件的目录路径
#define S 100                               //循环监听的间隔时间以ms计

namespace bf = boost::filesystem;

class ThrBackUp     //这个类用来上传分块到服务器
{
public:

	bool _res;      //用来记录此分块是否上传成功，可以在类外调用查看

	ThrBackUp(const std::string &file, int64_t start, int64_t len)
		:_res(true)            //初始化每个分块默认是上传成功的
		, _file(file)          //初始化要上传的文件的路径
		, _range_start(start)  //初始化要上传的分块的起始位置
		, _range_len(len)      //初始化要上传分块的长度
	{}

	bool Start()               //上传分块到服务端的函数
	{
		std::ifstream path(_file, std::ios::binary); //先以二进制流的方式打开要上传的文件
		if (!path.is_open())                         //判断这个文件是否被打开
		{                                            //如果打开失败，则提醒用户这个需要备份的文件打开失败,并返回false
			std::cerr << "range backup file " << _file << " failed\n";
			return false;
		}

		path.seekg(_range_start, std::ios::beg);    //从文件的起始位置开始，偏移到需要上传的分块的，第一个字节的位置
		std::string body;                           //创建一个用于存储要上传的分块的数据的string对象
		body.resize(_range_len);                    //为了能够存储这个分块的数据，那么就要将body的空间扩大为这个分块的大小
		path.read(&body[0], _range_len);            //然后将这个文件中的对应分块的数据读入body中

		if (!path.good())                           //如果上一步读入分块的操作失败，则提醒用户这个文件读取错误，然后返回false
		{
			std::cerr << "read file " << _file << " range data faild\n";
			return false;
		}
		path.close();                               //能执行到这一步，说明文件读取成功，那么就可以关闭此文件了
													//上传range数据
													//PUT /list/filename HTTP/1.1
		bf::path name(_file);                       //获取一下上传分块信息文件的文件名
		std::string url = BACKUP_URI + name.filename().string();  //配置URL为备份的目录加上文件名
		httplib::SSLClient cli(SERVER_IP, SERVER_PORT);           //实例化出一个SSL的客户端对象，用于获取服务端的IP地址和端口号
		httplib::Headers hdr;                                     //实例化一个http请求包的首部信息对象，用于配置请求包的首部信息
		hdr.insert(std::make_pair("Content-Length", std::to_string(_range_len))); //每个首部行都是一个pair对象，我们只配置第一个首部行
																				  //就是这个分块的大小，因为首部行是以字符串的形式存储的，所以要把数字转成字符串
		std::stringstream tmp;                                    //然后生成一个字符串流对象，用来配置第二个首部行
		tmp << "bytes=" << _range_start << "-" << (_range_start + _range_len - 1);  //第二个首部行的内容是，传输的分块的起始位置和结束位置
		hdr.insert(std::make_pair("Range", tmp.str().c_str()));   //然后将第二个首部行的字段设置为Range，内容是之前设置的tmp里面的内容
		auto rsp = cli.Put(url.c_str(), hdr, body, "text/plain"); //然后发送一个客户端的请求包，请求方法是PUT，因为备份的文件也可能是更新之前的文件
																  //将URL传入，还有它的首部行信息，和上传的分块内容，最后就是上传内容所用格式是纯文件类型
																  //text/html表示上传的内容是html的格式，所以浏览器会自动解析这种格式的内容
		if (rsp && rsp->status != 200)                            //如果上传成功，那么服务端会发送一个响应包，包里的状态码一般就是200 OK
		{                                                         //如果上传失败，也就是服务端发送的状态码不是200 OK，那么就说明上传分块失败
			_res = false;                                         //然后设置用于标志此分块上传成功与否的变量为false，表示上传分块失败
		}

		std::stringstream ss;                                     //然后再生成一个字符串流对象，用来记录一段信息
																  //将备份文件的超始位置和它的长度记录下来
		ss << "backup file[" << _file << "] range:[" << _range_start << "-" << _range_len << "]\n";
		std::cout << ss.str();                                    //然后提醒用户正在上传哪个分块
		return true;                                              //然后返回成功，因为如果上传失败可以通过_res成员来进行检查
	}
private:
	std::string _file;      //要上传的文件的路径
	int64_t _range_start;   //要上传的文件分块的起始位置
	int64_t _range_len;     //要上传的文件分块的长度
};

class CloudClient
{
public:
	CloudClient()
	{
		bf::path file(CLIENT_BACKUP_DIR);   //监视文件的目录，也就是备份文件的目录，把它的路径读出来
		if (!bf::exists(file))              //如果这个备份文件目录不存在则创建此目录
		{
			bf::create_directory(file);
		}
	}

	bool Start()                           //唯一提供给用户调用的接口，可以通过这个接口来使用客户端提供功能
	{
		GetBackupInfo();                   //用来获取备份文件信息

		while (1)                          //用于循环监听
		{
			BackupDirListen(CLIENT_BACKUP_DIR); //监听用于备份的目录是否有变化，如果有变化进行相应的处理
			SetBackupInfo();                    //用来在文件中重新设置备份信息，因为在检查目录的时候
												//可能添加了一些其它的备份信息在哈希表中
			Sleep(S);                         //因为客户端不能直接像服务端一样使用listen函数循环监听
		}                                       //所以要用while一直监听，不用循环监听时间太短，cpu利用率太高
		return true;                            //所以指定间隔时间
	}

private:
	bool GetBackupInfo()  //用来获取文件的备份信息到哈希表中
	{
		//filename1 etag\n
		//filename2 etag\n
		bf::path path(CLIENT_BACKUP_INFO_FILE);  //获取用来存储文件备份信息的文件路径

		if (!bf::exists(path))                   //如果这个目录不存，则提醒用户此备份信息文件不存在，直接返回false
		{                                        //因为这个文件不存在，则说明没有备份信息，所以也就不用获取了
			std::cerr << "list file" << path.string() << "is not exist\n";
			return false;
		}

		int64_t fsize = bf::file_size(path);     //如果这个备份信息目录存在，则将这个文件中数据的总大小读入
		if (fsize == 0)                          //如果这个文件是空的
		{                                        //则提醒用户没有备份信息，然后返回false，不用再获取备份信息了
			std::cerr << "have no backup info\n";
			return false;
		}

		std::string body;                        //程序运行到这就说明，这个备份信息目录中有文件的备份信息
		body.resize(fsize);                      //那么创建一个string变量，用于读入文件备份信息，先将这个变量的空间
												 //扩充到能够存在这个备份信息的大小
		std::ifstream file(CLIENT_BACKUP_INFO_FILE, std::ios::binary); //以二进制流的形式打开这个备份信息文件
		if (!file.is_open())                     //如果这个没有被打开
		{                                        //那么提示用户备份信息文件打开错误，然后返回false
			std::cerr << "list file open error\n";
			return false;
		}

		file.read(&body[0], fsize);              //如果这个文件打开了，那么就把这个备份信息文件中的数据读入到body中
		if (!file.good())                        //用来判断上一步的文件是否成功，如果不成功
		{                                        //则提醒用户读入备份信息文件错误，然后返回false
			std::cerr << "read list file body error\n";
			return false;
		}
		file.close();                            //将文件读入完后，再关闭这个文件

		std::vector<std::string> list;           //创建一个用于记录每一个文件备份信息的数组
												 //它的每个元素都会记录一个备份信息字符串

		boost::split(list, body, boost::is_any_of("\n"));//用来将body中的字符串，按每一条备份信息存入list中
														 //因为我们的备份信息是以"\n"作为每隔符的，所以可以用它来做分隔

		for (auto i : list)                      //遍历这个备份信息数组，将每一条备份信息传给i
		{
			//备份信息的格式：filename2 etag
			size_t pos = i.find(" ");            //每一条备份信息都是以空格来区分文件名和标签的，
			if (pos == std::string::npos)        //npos实际上初始化值是一个-1，用它可以来判断有没有找到空格
			{                                    //如果没有找到空格，那么说明这条备份信息有误
				continue;                        //就继续查询下一条备份信息
			}

			std::string key = i.substr(0, pos);  //程序走到这里说明找到分隔符空格了，所以在哈希表中存储数据时，用文件名来当key
			std::string val = i.substr(pos + 1); //用这条备份信息的标签来当val
			_backup_list[key] = val;             //然后根据key和val将这条信息存储在哈希表中
		}

		return true;                             //当备份信息在哈希表中存储完后，就可以返回了
	}

	bool BackupDirListen(const std::string &path)    //用来检查用于备份目录中的文件是否有变化
	{
		bf::directory_iterator item_begin(path);     //将此目录传入，获取第一个文件地址
		bf::directory_iterator item_end;             //获取最后一个文件，再后面一个的地址

		for (; item_begin != item_end; ++item_begin) //用来遍历整个目录中的文件是否有改变
		{
			if (bf::is_directory(item_begin->status()))//如果现在遍历的这个文件是一个目录
			{                                          //那么就是继续递规查找这个目录中的文件是否有改变
				BackupDirListen(item_begin->path().string());//然后等这个目录遍历完成后
				continue;                                    //再继续遍历下一个文件
			}

			if (FileIsNeedBackup(item_begin->path().string()) == false)//如果现在遍历的这个文件没有备份过
			{                                                          //那么就跳出这个条件
				continue;                                              //如果这个文件已经备份过了，那么就检查下一个文件
			}
			//程序能执行到这里，就说明这个文件需要备份，所以打印一句话提醒用户，哪个文件需要备份
			std::cerr << "file:[" << item_begin->path().string() << " need backup\n";

			if (PutFileData(item_begin->path().string()) == false)//调用备份文件的函数，对需要备份的文件进行备份
			{                                                     //并在哈希表中记录备份信息
				continue;
			}
		}
		return true;                                              //当整个目录检查完成后就返回true，说明这此检查完毕
	}

	bool FileIsNeedBackup(const std::string &file)  //用于判断此文件是否需要备份
	{
		std::string etag;                           //要知道这个文件是否需要备份，有两个因素，一个是原来没有保存过这个文件
													//另一个就是原来备份过这个文件，但这个文件又被修改了，所以又得备份
													//所以定义一个string变量，用于获取文件标签信息，当然并不是从备份信息文件中获取
		if (GetFileEtag(file, etag) == false)       //用来获取此文件的标签信息，etag是引用的方式传过去的
		{                                           //所以这个函数返回后，可以直接通过etag获取到标签信息，
			return false;                           //如果获取失败，则返回false，说明此文件不需要备份
		}                                           //因为返回false的唯一可能就是这个文件不存在

		auto it = _backup_list.find(file);          //然后去哈希表中利用文件名找这个文件对应的标签信息

		if (it != _backup_list.end() && it->second == etag) //如果这个哈希表中的此文件的标签信息，和刚才获取的标签信息相同 
		{                                                   //并且这个文件在哈希表中，则返回false，说明此文件不需要重新备份
			return false;
		}
		return true;                                        //否则就返回true，表示此文件需要备份
	}

	bool GetFileEtag(const std::string &file, std::string &etag)  //用于获取文件备份信息中的标签信息
	{
		bf::path path(file);                            //用于获取需要获取备份信息的文件的路径
		if (!bf::exists(path))                          //如果这个文件不存在则提醒用户获取文件失败，然后返回false
		{
			std::cerr << "get file " << file << "etag error\n";
			return false;
		}

		int64_t fsize = bf::file_size(path);            //将这个文件的大小读入
		int64_t mtime = bf::last_write_time(path);      //然后获取一下这个文件，最后一次被写入的时间
														//因为文件在备份信息中的标签就是以这个文件的大小和最后一次写入文件的时间来保存的
		std::stringstream tmp;                          //stringsteam就是以流的形式存储着字符串，有一个可以直接拼接字符串的功能
														//ios_base& hex(ios_base& str);用来将输入的整型值转换为16进制数
		tmp << std::hex << fsize << "-" << std::hex << mtime; //将这个文件的大小和最后一次进行写入的时间合起来，组成标签信息
		etag = tmp.str();                               //因为tmp是以流的形式存储字符串的，所以转成string类型的字符串，返回回去，让用户获取

		return true;
	}

	bool PutFileData(const std::string &file) //如果此文件需要备份那么就调用此函数将文件上传至服务端
	{
		//按分块传输大小（10M）对文件内容进行分块传输
		//通过获取分块传输是否成功判断整个文件是否上传成功
		//选择多线程处理
		//1.获取文件大小
		//2.计算总共需要分多少块，得到每块大小以及起始位置
		//3.循环创建线程，在线程中上传文件结果
		//4.等待所有线程退出，判断文件上传结果
		//5.上传成功，则添加文件的备份信息记录到哈希表

		int64_t fsize = bf::file_size(file);     //先计算要上传的文件的大小
		if (fsize <= 0)                          //如果文件大小小于0，那么就说明这个文件是空的或坏的
		{                                        //那么就提醒用户要备份的文件没有必要进行备份，然后返回false
			std::cerr << "file" << file << "unnecessary backup\n";
			return false;
		}

		int count = (int)fsize / RANGE_MAX_SIZE; //然后在上传文件采用分块上传，每块是10M，所以计算总共要上传多个块
												 //当然了因为是是整形的除法计算，所以不会有小数，所以总体要上传的块数应该是计算出的结果加1
		std::vector<ThrBackUp> thr_res;          //因为上传每个块时需要不同的参数信息，所以采用设计一个专门用于上传文件到服务器的类
												 //然后每个分块都是一个对象，要上传哪个分块就调用哪个对象，所以这里用一个vector来存储这些类对象
		std::vector<std::thread> thr_list;       //采用多线程的方法进行分块上传，增加效率，所以要传多少个分块，就用多少个线程，这里用一个vector来存储这些线程对象
												 //然后提醒一下用户这个文件，每个分块（除最后一个分埠）有多大，总共有多少块，
		std::cerr << "file:[" << file << "] fsize:[" << fsize << "byte] count:[" << count + 1 << "]\n";

		for (int i = 0; i <= count; i++)         //然后开始循环上传分块，总共上传count+1块
		{
			int64_t range_start = i * RANGE_MAX_SIZE;  //计算上传的分块的第一个字节的位置
			int64_t range_end = ((i + 1) * RANGE_MAX_SIZE) - 1;//计算上传的分块的最后一个字节的位置，就是它下一个分块的位置减1
			if (i == count)              //当上传最后一个分块时，需要进行其它操作，因为最后一个分块的最后位置，不能用它下一个分块的位置计算
			{                            //因为最后一个分块要比其它分块小
				range_end = fsize - 1;   //最后一个分块的最后位置就是文件总大小减1
			}
			int64_t range_len = range_end - range_start + 1;     //每个分块的长度就是每个分块的最后位置减去第一个位置
			ThrBackUp backup_info(file, range_start, range_len); //将每个分块的信息传入各自上传时要使用的类中
																 //然后提醒一下用户，这个文件的每个分块上传时的开始位置和最后位置，以及每个分块的大小
			std::cerr << "file:[" << file << "] range:[" << range_start << "-" << range_end << "-] -" << range_len << "\n";
			thr_res.push_back(backup_info);                      //然后将每个分块使用的类对象传入thr_res中进行保存
		}

		for (int i = 0; i <= count; i++)                         //然后就可以进行分块上传了
		{                                                        //生成一个线程对象，然后存入thr_list中，进行保存，同时线路也开始上传分块了
			thr_list.push_back(std::thread(thr_start, &thr_res[i]));
		}


		bool ret = true;                     //定义一个变量，用来记录整个文件是否上传成功
		for (int i = 0; i <= count; i++)     //等待每一个线程上传分块成功后，来处理剩余的资源，释放每个线程
		{
			thr_list[i].join();              //按序，等待每一个线程
			if (thr_res[i]._res == true)     //如果这个线程上传分块成功，则继续判断下一个线程否上传成功 
			{
				continue;
			}
			ret = false;                     //如果任一一个线程上传分块失败，则说明整个文件上传失败，让返回结果为false
		}

		if (ret == false)                    //如果上传文件失败，则提醒用户此文件上传失败，并返回false
		{
			std::cerr << "file:[" << file << "]failed\n";
			return false;
		}

		AddBackupInfo(file);                //程序能执行到这里，说明文件上传成功，所以要在哈希表中添加备份信息
		std::cerr << "file:[" << file << "]backup success\n"; //然后提醒用户文件上传成功
		return true;                                          //最后再返回true
	}


	static void thr_start(ThrBackUp *backup_info)  //线程的执行函数，通过这个函数来上传对应分块
	{
		backup_info->Start();                      //调用上传的分块的各自类中的函数，进行分块的上传

		return;
	}

	bool AddBackupInfo(const std::string &file)   //如果一个文件备份成功，则要在哈希表中添加备份信息
	{
		//etag = "mtime-fsize"
		std::string etag;                         //哈希表中存储备份信息的格式是，文件名加标签信息
												  //所以先创建一个string变量，用来获取此文件的标签信息
		if (GetFileEtag(file, etag) == false)     //调用获取备份文件标签信息的函数，如果获取失败，则返回fallse
		{                                         //表示文件添加备份信息失败
			return false;
		}

		_backup_list[file] = etag;                //然后以文件名为key来存储标签信息
		return true;                              //在哈希表中添加备份信息成功后，就可以返回true了
	}

	bool SetBackupInfo()           //用来重新设置文件的备份信息
	{
		std::string body;          //先创建一个string变量，用来存储哈希表中的备份信息
								   //因为哈希表中存储的就是最新的备份信息
		for (auto i : _backup_list)//将哈希表中每一条的备份信息都存储在i中
		{                          //将哈希表中的备份信息，转换成文件中存储备份信息的格式，然后放入body中
			body += i.first + " " + i.second + "\n";
		}                          //再以二进制流的方式打开备份信息文件，准备用新的备份信息来替换旧的备份信息
		std::ofstream file(CLIENT_BACKUP_INFO_FILE, std::ios::binary);
		if (!file.is_open())       //如果上一步打开文件的操作是失败的，那么提醒用户打开文件失败，并返回false
		{
			std::cerr << "open list file error\n";
			return false;
		}
		file.write(&body[0], body.size());           //程序执行到这一步，说明新的备份信息已经写入body中
													 //然后从body中将新的备份信息写入到备份信息文件中
		if (!file.good())                            //如果上一步的向文件中写入备份信息操作失败
		{                                            //那么提醒用户写入备份信息失败，并返回false
			std::cerr << "set backup info error\n";
			return false;
		}

		file.close();                               //当备份信息更新完成，则关闭此文件，然后返回true
		return true;
	}

private:
	std::unordered_map<std::string, std::string> _backup_list;  //这个哈希表用于存储文件的备份信息
};