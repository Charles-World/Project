#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <boost/filesystem.hpp>
#include "compress.hpp"
#define SERVER_BASE_DIR "www"
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT 9000
#define SERVER_BACKUP_DIR SERVER_BASE_DIR"/list/" 

namespace bf = boost::filesystem;
using namespace httplib;

CompressStore cstor;

class CloudServer
{
public:
	CloudServer(const char *cert, const char *key)
				:srv(cert, key)
	{
		bf::path path(SERVER_BASE_DIR);
		if(!bf::exists(path))
		{
			bf::create_directory(SERVER_BASE_DIR);
		}

		bf::path list_path(SERVER_BACKUP_DIR);
		if(!bf::exists(list_path))
		{
			bf::create_directory(SERVER_BACKUP_DIR);
		}
	}

	bool Start()
	{
		srv.set_base_dir(SERVER_BASE_DIR);
		srv.Get("/(list(/){0,1}){0,1}", GetFileList);
		srv.Get("/list/(.*)", GetFileDate);
		srv.Put("/list/(.*)", PutFileData);
		srv.listen(SERVER_ADDR, SERVER_PORT);
		return true;
	}
private:
	//将用户上传的文件保存在本地的函数
	static void PutFileData(const Request &req, Response &rsp)
	{					
		if(!req.has_header("Range"))
		{
			rsp.status = 400;
			return;
		}
		std::string range = req.get_header_value("Range");	
		int64_t range_start;
		if(RangeParse(range, range_start) == false)	
		{
			rsp.status = 400;
			return ;
		}
		std::string real = SERVER_BASE_DIR + req.path;
		cstor.SetFileData(real, req.body, range_start);		

		return ;
	}
	static bool RangeParse(std::string &range, int64_t &start)
	{
		std::cerr << "range:["<<range<<"] format error/n";
		//bytes = start - end
		size_t pos1 = range.find("=");
		size_t pos2 = range.find("-");
		if(pos1 == std::string::npos || pos2 == std::string::npos)
		{
			std::cerr << "range:["<<range<<"] format error/n";
			return false;
		}
		std::stringstream rs;
		rs << range.substr(pos1 + 1, pos2 - pos1 - 1);
		rs >> start;
		return true;
	}

	static void GetFileList(const Request &req, Response &rsp)
	{
		std::vector<std::string> list;
		cstor.GetFileList(list);
		std::string body;
		body = "<html><body><ol><hr />";
		for(auto i : list)
		{
			bf::path path(i);	
			std::string file = path.filename().string();
			std::string uri = "/list/" + file;
			body += "<h4><li>";
			body += "<a href='";
			body += uri;
			body += "'>";
			body += file;
			body += "</a>";
			body += "</li></h4>";
			// <h4><li><a href='/list/filename'>filename</a></li><h4>"
			// std::cerr << "file: " << file << std::endl;
		}
			//std::cerr << "hello" << std::endl;
		body += "<hr /></ol></body></html>";
		rsp.set_content(&body[0], "text/html");
		return;
	}
	static void GetFileDate(const Request &req, Response &rsp)
	{
		std::string real = SERVER_BASE_DIR + req.path;
		std::string body;
		cstor.GetFileData(real, body);
		rsp.set_content(body, "text/plain");
		return;
	}

private:
	SSLServer srv;
};					

void thr_start()
{
	cstor.LowHeatFileStore();
}

int main()
{
	std::thread thr(thr_start);
	thr.detach();
	CloudServer srv("./cert.pem", "./key.pem");
	srv.Start();

	return 0;
}			
