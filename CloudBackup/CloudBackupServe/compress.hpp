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

#define SERVER_BASE_DIR "www"
#define UNGIZPFILE_PATH "www/list/"
#define GZIPFILE_PATH "www/zip/"
#define RECORD_FILE "record.list"
#define HEAT_TIME        60

namespace bf = boost::filesystem;

class CompressStore
{
public:
	CompressStore()
	{                                               
		bf::path base(SERVER_BASE_DIR);           //将根目录进行记录
		if(!bf::exists(base))                     //判断此目录是否存在
		{
			bf::create_directory(SERVER_BASE_DIR);//如果不存在则创建
		}
	
		bf::path path(GZIPFILE_PATH);             //将压缩文件目录路径记录
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
    //用来获取文件列表的函数
	bool GetFileList(std::vector<std::string>& list)
	{
		pthread_rwlock_rdlock(&_rwlock);
		for(auto i : _file_list)
		{
			list.push_back(i.first);
		}
		pthread_rwlock_unlock(&_rwlock);

		return true;
	}

	bool SetFileData(const std::string &file, const std::string &body, const int64_t offset)
	{
		int fd = open(file.c_str(), O_CREAT|O_WRONLY, 0664);
		if(fd < 0)
		{
			std::cerr << "open file " << file << "error\n";
			return false;
		}
		flock(fd, LOCK_EX);
		lseek(fd, offset, SEEK_SET);
		int ret = write(fd, &body[0], body.size());
		if(ret < 0)
		{
			std::cerr << "store file " << file << " data error\n";
		    flock(fd, LOCK_UN);
			return false;
		}
		flock(fd, LOCK_UN);
		close(fd);
		AddFileRecord(file, "");
		return true;
	}

	bool GetFileData(std::string &file, std::string &body)
	{
		if(bf::exists(file))
		{
			GetNormalFile(file, body);
		}
		else
		{
			std::string gzip;
			GetFileGzip(file, gzip);
			UnCompressFile(gzip, file);
			GetNormalFile(file, body);
		}
		return true;
	}

	bool LowHeatFileStore()
	{
		GetListRecord();
		while(1)
		{
			DirectoryCheck();
			SetListRecord();
			sleep(3);
		}
	} 
                                         
private:
	bool GetListRecord()
	{
		//filename gzipfilename\n
		bf::path name(RECORD_FILE);
		if(!bf::exists(name))
		{
			return false;
		}
		std::ifstream file(RECORD_FILE, std::ios::binary);
		if(!file.is_open())
		{
			std::cerr << "record file body read error\n";
			return false;
		}
		int64_t fsize = bf::file_size(name);
		std::string body;
		body.resize(fsize);
		file.read(&body[0], fsize);
		if(!file.good())
		{
			std::cerr << "record file body read error\n";
			return false;
		}
		file.close();
		std::vector<std::string> list;
		boost::split(list, body, boost::is_any_of("\n"));
		for(auto  i : list)
		{
			size_t pos =  i.find(" ");
			if(pos == std::string::npos)
			{
				continue;
			}
			std::string key = i.substr(0, pos);
			std::string val = i.substr(pos + 1);
			_file_list[key] = val;
		}

		return true;
	}
		
	bool SetListRecord()
	{
		std::stringstream tmp;
		for(auto i : _file_list)
		{
			tmp << i.first << " " << i.second << "\n";
		}
		std::ofstream file(RECORD_FILE, std::ios::binary|std::ios::trunc);
		if(!file.is_open())
		{
			std::cerr << "record file open error\n";
			return false;
		}
		file.write(tmp.str().c_str(), tmp.str().size());
		if(!file.good())
		{
			std::cerr << "record file write body error\n";
			return false;
		}
		file.close();
		return true;
	}
		
	bool IsNeedCompress(std::string &file)
	{
		struct stat st;
		if(stat(file.c_str(), &st) < 0)
		{
			std::cerr << "get file:[" << file << "]stat error\n";
			return false;
		}
		time_t cur_time = time(NULL);
		time_t acc_time = st.st_atime;
		if((cur_time - acc_time) < HEAT_TIME)	
		{
			return false;
		}
			
		return true;
	}

	bool CompressFile(std::string &file, std::string &gzip)
	{
		int fd = open(file.c_str(), O_RDONLY);
		if(fd < 0)
		{
			std::cerr << "com open file:[" << file << "] error\n";
			return false;
		}
		gzFile gf = gzopen(gzip.c_str(), "wb");
		if(gf == NULL)
		{	
			std::cerr << "com open gzip:[" << gzip << "] error\n";
			return false;
		}
		int ret;
		char buf[1024];
		flock(fd, LOCK_SH);
		while((ret = read(fd, buf, 1024)) > 0)
		{
			gzwrite(gf, buf, ret);
		}
		flock(fd, LOCK_UN);
		close(fd);
		gzclose(gf);
		unlink(file.c_str());

		return true;
	}
	bool UnCompressFile(std::string &gzip, std::string &file)
	{
		int fd = open(file.c_str(), O_CREAT|O_WRONLY, 0664);
		if(fd < 0)
		{
			std::cerr << "open file " << file << "failed\n";
			return false;
		}
		gzFile gf = gzopen(gzip.c_str(), "rb");
		if(gf == NULL)
		{
			std::cerr << "open gzip " << gzip << "failed\n";
			close(fd);
			return false;
		}
		int ret;
		char buf[1024] = {0};
		flock(fd, LOCK_EX);
		while((ret = gzread(gf, buf, 1024))  > 0)
		{
			int len = write(fd, buf, ret);
			if(len < 0)
			{
				std::cerr << "get gzip data failed\n";
				gzclose(gf);
				close(fd);
				flock(fd, LOCK_UN);
				return false;
			}
		}
		flock(fd, LOCK_UN);
		gzclose(gf);
		close(fd);
		unlink(gzip.c_str());
		return true;
	}
		
				
	bool AddFileRecord(const std::string file, const std::string &gzip)
	{
		pthread_rwlock_wrlock(&_rwlock);
		_file_list[file] = gzip;
		pthread_rwlock_unlock(&_rwlock);
		return true;
	}
		
	bool GetNormalFile(std::string &name, std::string &body)
	{
		int64_t fsize = bf::file_size(name);
		body.resize(fsize);
		int fd = open(name.c_str(), O_RDONLY);
		if(fd < 0)
		{
			std::cerr << "open file " << name << " failed\n";
			return false;
		}
		flock(fd, LOCK_SH);
		int ret = read(fd, &body[0], fsize);
		flock(fd, LOCK_UN);
		if(ret != fsize)
		{
			std::cerr << "get file " << name << "body error\n";
			close(fd);
			return false;
		}
		return true;
	}
				
	bool GetFileGzip(std::string &file, std::string &gzip)
	{
		pthread_rwlock_rdlock(&_rwlock);
		auto it = _file_list.find(file);
		if(it == _file_list.end())
		{
			pthread_rwlock_unlock(&_rwlock);
			return false;
		}
			gzip = it->second;
			pthread_rwlock_unlock(&_rwlock);
			return true;
	}
		
	bool DirectoryCheck()
	{
		if(!bf::exists(UNGIZPFILE_PATH))
		{
			bf::create_directory(UNGIZPFILE_PATH);
		}
		bf::directory_iterator item_begin(UNGIZPFILE_PATH);
		bf::directory_iterator item_end;
		for(; item_begin != item_end; ++item_begin)
		{
			if(bf::is_directory(item_begin->status()))
			{
				continue;
			}
			std::string name = item_begin->path().string();
			if(IsNeedCompress(name))
			{
				std::string gzip = GZIPFILE_PATH + item_begin->path().filename().string() + ".gz";
				CompressFile(name, gzip);
				AddFileRecord(name, gzip);
			}
		}
			return true;
	}
									

private:
	std::string _file_dir;
	std::unordered_map<std::string, std::string> _file_list;
	pthread_rwlock_t _rwlock;
};
		
