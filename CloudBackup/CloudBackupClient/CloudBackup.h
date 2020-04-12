#pragma once
#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#define SERVER_PORT 9000
#define SERVER_IP "192.168.184.128"
#define RANGE_MAX_SIZE (10 << 20)
#define CLIENT_BACKUP_DIR "backup"
#define CLIENT_BACKUP_INFO_FILE "back.list"
#define BACKUP_URI "/list/"
namespace bf = boost::filesystem;

//在使用本项目之前需要用到boost_1_70_0和OpenSSL-Win64
//vs下配置方法：在调试下的属性对话中设置"C/C++" -> "常规" -> "附加包含目录" D:\库\boost_1_70_0和D:\库\OpenSSL-Win64\include
//"链接器" -> "常规" -> "附加库目录" -> D:\库\boost_1_70_0\lib64-msvc-14.1和D:\库\OpenSSL-Win64\lib
//"链接器" -> "输入" -> "附加依赖项" libssl.lib和libcrypto.lib
class ThrBackUp
{
private:
	std::string _file;
	int64_t _range_start;
	int64_t _range_len;
public:
	bool _res;
	ThrBackUp(const std::string &file, int64_t start, int64_t len) :_res(true)
		, _file(file), _range_start(start), _range_len(len) {}

	bool Start()
	{
		std::ifstream path(_file, std::ios::binary);
		if (!path.is_open())
		{
			std::cerr << "range backup file " << _file << " failed\n";
			return true;
		}
		path.seekg(_range_start, std::ios::beg);
		std::string body;
		body.resize(_range_len);
		path.read(&body[0], _range_len);
		if (!path.good())
		{
			std::cerr << "read file " << _file << " range data faild\n";
			return false;
		}
		path.close();
		//上传range数据
		//PUT/list/filename HTTP/1.1
		bf::path name(_file);
		std::string url = BACKUP_URI + name.filename().string();
		httplib::SSLClient cli(SERVER_IP, SERVER_PORT);
		httplib::Headers hdr;
		hdr.insert(std::make_pair("Content-Length", std::to_string(_range_len)));
		std::stringstream tmp;
		tmp << "bytes=" << _range_start << "-" << (_range_start + _range_len - 1);
		hdr.insert(std::make_pair("Range", tmp.str().c_str()));
		auto rsp = cli.Put(url.c_str(), hdr, body, "text/plain");
		if (rsp && rsp->status != 200)
		{
			_res = false;
		}
		std::stringstream ss;
		ss << "backup file[" << _file << "] range:[" << _range_start << "-" << _range_len << "]\n";
		std::cout << ss.str();
		return true;
	}
};
class CloudClient
{
public:
	CloudClient()
	{
		bf::path file(CLIENT_BACKUP_DIR);
		if (!bf::exists(file))
		{
			bf::create_directory(file);
		}
	}
private:
	std::unordered_map<std::string, std::string> _backup_list;
private:
	bool GetBackupInfo()
	{
		//filename1 etag\n
		//filename2 etag\n
		bf::path path(CLIENT_BACKUP_INFO_FILE);
		if (!bf::exists(path))
		{
			std::cerr << "list file" << path.string() << "is not exist\n";
			return false;
		}
		int64_t fsize = bf::file_size(path);
		if (fsize == 0)
		{
			std::cerr << "have no backup info\n";
			return false;
		}
		std::string body;
		body.resize(fsize);
		std::ifstream file(CLIENT_BACKUP_INFO_FILE, std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "list file open error\n";
			return false;
		}
		file.read(&body[0], fsize);
		if (!file.good())
		{
			std::cerr << "read list file body error\n";
			return false;
		}
		file.close();
		std::vector<std::string> list;
		boost::split(list, body, boost::is_any_of("\n"));
		for (auto i : list)
		{
			//filename2 etag
			size_t pos = i.find(" ");
			if (pos == std::string::npos)
			{
				continue;
			}
			std::string key = i.substr(0, pos);
			std::string val = i.substr(pos + 1);
			_backup_list[key] = val;
		}
		return true;
	}

	bool BackupDirListen(const std::string &path)
	{
		bf::directory_iterator item_begin(path);
		bf::directory_iterator item_end;
		for (; item_begin != item_end; ++item_begin)
		{
			if (bf::is_directory(item_begin->status()))
			{
				BackupDirListen(item_begin->path().string());
				continue;
			}
			if (FileIsNeedBackup(item_begin->path().string()) == false)
			{
				continue;
			}
			std::cerr << "file:[" << item_begin->path().string() << " need backup\n";
			if (PutFileData(item_begin->path().string()) == false)
			{
				continue;
			}
		}
		return true;
	}

	bool FileIsNeedBackup(const std::string &file)
	{
		std::string etag;
		if (GetFileEtag(file, etag) == false)
		{
			return false;
		}
		auto it = _backup_list.find(file);
		if (it != _backup_list.end() && it->second == etag)
		{
			return false;
		}
		return true;
	}

	bool GetFileEtag(const std::string &file, std::string &etag)
	{
		bf::path path(file);
		/*if (bf::exists(path))
		{
			std::cerr << "get file " << file << "etag error\n";
			return false;
		}*/
		int64_t fsize = bf::file_size(path);
		int64_t mtime = bf::last_write_time(path);
		std::stringstream tmp;
		tmp << std::hex << fsize << "-" << std::hex << mtime;
		etag = tmp.str();
		return true;
	}

	bool PutFileData(const std::string &file)
	{
		//按分块传输大小（10M）对文件内容进行分块传输
		//通过获取分块传输是否成功判断整个文件是否上传成功
		//选择多线程处理
		//1.获取文件大小
		//2.计算总共需要分多少块，得到每块大小以及起始位置
		//3.循环创建线程，在线程中上传文件结果
		//4.等待所有线程退出，判断文件上传结果
		//5.上传成功，则添加文件的备份信息记录
		int64_t fsize = bf::file_size(file);
		if (fsize <= 0)
		{
			std::cerr << "file" << file << "unnecessary backup\n";
			return false;
		}
		int count = (int)fsize / RANGE_MAX_SIZE;
		std::vector<ThrBackUp> thr_res;
		std::vector<std::thread> thr_list;
		std::cerr << "file:[" <<file<< "] fsize:[" << fsize << "] count:["<< count + 1 << "]\n";
		for (int i = 0; i <= count; i++)
		{
			int64_t range_start = i * RANGE_MAX_SIZE;
			int64_t range_end = ((i + 1) * RANGE_MAX_SIZE) - 1;
			if (i == count)//最后一个分块时
			{
				range_end = fsize - 1;
			}
			int64_t range_len = range_end - range_start + 1;
			ThrBackUp backup_info(file, range_start, range_len);
			std::cerr << "file:[" << file << "] range:[" << range_start << "-" << range_end << "-] -" << range_len << "\n";
			thr_res.push_back(backup_info);
		}
		for (int i = 0; i <= count; i++)
		{
			thr_list.push_back(std::thread(thr_start, &thr_res[i]));
		}


		bool ret = true;
		for (int i = 0; i <= count; i++) //这里的 i<=count少了个等号，浪费了我有 4、5个小时调试吧
		{
			thr_list[i].join();
			if (thr_res[i]._res == true)
			{
				continue;
			}
			ret = false;
		}
		/*if (ret == true)
		{
		AddBackupInfo(file);
		}*/
		if (ret == false)
		{
			std::cerr << "file:[" << file << "]failed\n";
			return false;
		}
		AddBackupInfo(file);
		std::cerr << "file:[" << file << "]backup success\n";
		return true;
	}


	static void thr_start(ThrBackUp *backup_info)
	{
		backup_info->Start();

		return;
	}

	bool AddBackupInfo(const std::string &file)
	{
		//return false;
		//etag = "mtime-fsize"
		std::string etag;
		if (GetFileEtag(file, etag) == false)
		{
			return false;
		}

		_backup_list[file] = etag;
		return true;
	}

	bool SetBackupInfo()
	{
		std::string body;
		for (auto i : _backup_list)
		{
			body += i.first + " " + i.second + "\n";
		}
		std::ofstream file(CLIENT_BACKUP_INFO_FILE, std::ios::binary);
		if (!file.is_open())
		{
			std::cerr << "open list file error\n";
			return false;
		}
		file.write(&body[0], body.size());
		if (!file.good())
		{
			std::cerr << "set backup info error\n";
			return false;
		}
		file.close();
		return true;
	}

public:
	bool Start()
	{
		GetBackupInfo();
		while (1)
		{
			BackupDirListen(CLIENT_BACKUP_DIR);
			SetBackupInfo();
			Sleep(100);
		}
		return true;
	}
};