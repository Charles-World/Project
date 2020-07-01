
//本项的服务端使用了Zlib、OpenSSL和BOOST库，linux中自带
//还使用了一个httplib的头文件，方便使用http协议传输数据
//利用SSL在外部生成公钥和私钥，生成私钥方法是,openssl genrsa 2048 > key.pem 
//意思是将生成的2048位的私钥放入key.pem文件中
//因为我们要使用https协议，所以就要给自己颁发证书
//不过因为正式的证书是收费的，所以颁发一个假的证书,意思一下
//方法是：openssl req -new -key key.pem | openssl x509 -day 3650 -req -signkey key.pem > cert.pem
//意思是利用我们生成的私钥来生成一个证书文件，并写入到cert.pem文件中,req是证书请求子命令
//

#define CPPHTTPLIB_OPENSSL_SUPPORT             //要想使用httplib中的SSL的接口，就要定义这个宏

#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include "httplib.h"
#include <boost/filesystem.hpp>
#include "compress.hpp"

#define SERVER_BASE_DIR "www"                 //存储客户端备份文件的根目录
#define SERVER_ADDR "0.0.0.0"                 //服务端监听地址，表示服务端的所有ip地址都会监听
#define SERVER_PORT 9000                      //服务端使用的端口号
#define SERVER_BACKUP_DIR SERVER_BASE_DIR "/list/" //服务端用来真正存储客户端备份文件的目录

namespace bf = boost::filesystem;
using namespace httplib;

CompressStore cstor;                         //创建一个用来压缩客户端备份文件的对象

class CloudServer
{
public:
	CloudServer(const char *cert, const char *key) //用来初始化一些资源
				:srv(cert, key)                    //将我们服务端的证书和私钥传入,ssl会根据我们传入的私钥
												   //自动生成公钥，然后进行基于RSA算法的https协议
	{
		bf::path path(SERVER_BASE_DIR);            //获取一下，用于存储客户端备份文件的根目录路径
		if(!bf::exists(path))                      //如果这个目录不存在，则创建此目录
		{
			bf::create_directory(SERVER_BASE_DIR);
		}

		bf::path list_path(SERVER_BACKUP_DIR);     //获取一下，真正用于存储，客户端备份文件的目录路径
		if(!bf::exists(list_path))                 //如果不存在，则创建此目录
		{
			bf::create_directory(SERVER_BACKUP_DIR);
		}
	}

	bool Start()                              //服务端等待客户端发出请求的函数
	{
		srv.set_base_dir(SERVER_BASE_DIR);    //设置www目录为服务端的根目录 

		srv.Get("/(list(/){0,1}){0,1}", GetFileList); //如果客户端使用Get请求，的相对URL路径为/list/
                                                      //那么就调用GetFileDate函数，返回文件备份列表给用户
                                              
		srv.Get("/list/(.*)", GetFileDate);   //如果客户端使用Get请求，的相对URL路径为/list/*，也就是list目录下的
                                              //任一文件，那么就是调用GetFileDate函数
                                              //返回客户端请求的文件数据，给客户端获取他所备份的文件

		srv.Put("/list/(.*)", PutFileData);   //如果客户端使用Put请求，的相对URL路径为/list/*
                                              //那么就调用PutFileDate函数将用户的文件，备份到服务端上
		srv.listen(SERVER_ADDR, SERVER_PORT); //开始监听服务端的对应的IP地址和端口，是否有客户端过来请求数据

                                              //如果有客户端过来请求数据，那么就调用设置好的对应函数进行处理
		return true;
	}
private:
	static void PutFileData(const Request &req, Response &rsp) //将用户上传的文件保存在服务端
	{					
		if(!req.has_header("Range"))                       //如果客户端的请求包中的首部行，没有Range字段
		{                                                  //则返回400表示客户端的语法错误
			rsp.status = 400;                              //因为之前我们在写客户端时，首部行应该有二行
			return;                                        //一行是Content-Length，一行是Range
		}

		std::string range = req.get_header_value("Range"); //如果有Range信息，则将它的值，存储在一个string对象中
                                                           //方便后面获取客户端上传分块的超始位置和结束位置
		int64_t range_start;                               //创建一个用于存储分块起始位置的变量
		if(RangeParse(range, range_start) == false)	       //调用相关函数，获取上传的分块的起始位置
		{                                                 //如果获取失败，则返回400状态码，表示客户端Range语法错误
			rsp.status = 400;
			return;
		}
		std::string real = SERVER_BASE_DIR + req.path;    //设置用户上传的文件路径存储的位置，为我们的根目录
                                                          //和用户所请求的相对URL路径的值的拼接
		cstor.SetFileData(real, req.body, range_start);	  //调用上传分块的函数，进行上传	

		return;
	}

	static bool RangeParse(std::string &range, int64_t &start) //将Range记录的对应的分块起始位置的值解析出来,
                                                               //并用start返回回去
	{                                                          //Range: bytes = start - end "Range对应的格式"
		size_t pos1 = range.find("=");                         //要解析出分块起始位置的值，就得先找到=和-的位置
		size_t pos2 = range.find("-");                         //以它们的位置来读取出起始位置的值
		if(pos1 == std::string::npos || pos2 == std::string::npos) //如果没有找到=或-，那么就提醒用户Range字段
		{                                                          //格式错误，并返回false
			std::cerr << "range:["<<range<<"] format error/n";
			return false;
		}
		std::stringstream rs;                                  //创建一个字符串流对象，用来存储解析出的值
		rs << range.substr(pos1 + 1, pos2 - pos1 - 1);         //Range字段的起始值就是=后面的值，字符串长度就是
		rs >> start;                                           //从起始位置开始，到-前面的长度
		return true;                                           //然后利用start返回回去，并返回true
	}
    
    //用来让客户端过来获取备份文件的列表
	static void GetFileList(const Request &req, Response &rsp)
	{
		std::vector<std::string> list;    //先定义一个vector用来存储服务端备份的文件的文件名
		cstor.GetFileList(list);          //然后将list传入函数，因为是传引用，所以可以将获取的文件列表存储在list中
		std::string body;                 //因为我们的文件列表是要显示在浏览器中的
                                          //所以要使用html格式来编写这个字符串，所以创建一个string对象
		body = "<html><body><ol><hr />";  //然后简单的设置一下文件在浏览器中显示的格式
                                          //<html>是html文档的开始标志，<body>表示文档可见部分的开始
                                          //<ol>表示每一行都是有序的，直到遇到<ol />，<hr>表示设置水平线
		for(auto i : list)                //将获取到的文件名，一个一个的编辑在html文档中
		{
			bf::path path(i);	          //因为vector获取的是文件存储的路径，而html上显示的是文件名  
			std::string file = path.filename().string(); //所以先获取到文件的路径名，然后再提出出文件名
			std::string uri = "/list/" + file;           //设置每个文件对应的相对URL路径名
			body += "<h4><li>";                          //<h4>表示设置标题的等级为四，<li>表示项目列表
			body += "<a href='";                         //<a href表示设置超链接
			body += uri;                                 //将之前的设置好的此文件的URL设置为此文件的超链接
			body += "'>";                                //表示设置完此超链接
			body += file;                                //设置超链接名设置为此文件名
			body += "</a>";                              //表示此超链接的提示文字设置完成
			body += "</li></h4>";                        //表示项目列表和标题设置完成
		                                          	// <h4><li><a href='/list/filename'>filename</a></li><h4>"
		}

		body += "<hr /></ol></body></html>";       //表示水平线设置完成，有序行设置完成，浏览器可见内容设置完成
                                                   //html文档结束
		rsp.set_content(&body[0], "text/html");    //设置http响应包中的内容为body中的字符串，文本格式为html
		return;
	}

    //用来让客户获取自己备份的文件数据
	static void GetFileDate(const Request &req, Response &rsp)
	{
		std::string real = SERVER_BASE_DIR + req.path; //获取客户端请求的文件路径
		std::string body;                              //创建一个string变量，用来存储客户端请求的文件数据
		cstor.GetFileData(real, body);                 //调用函数，将body传入，传入的方式是引用，所以可以
                                                       //通过函数将数据写入到body中返回回来
		rsp.set_content(body, "text/plain");           //然后将客户端请求的备份文件，以纯文本的格式返回回去
                                                       //然后让用户进行下载
		return;
	}

private:
	SSLServer srv;      //服务端的对象，用来设置一些服务端的响应
};					

//用来检查备份文件中哪个文件是低热度文件，如果是低热度文件就进行压缩
void thr_start()
{
	cstor.LowHeatFileStore();
}

int main()
{
	std::thread thr(thr_start);                 //创建一个线程用来监听，是否有低热度文件需要被压缩
	thr.detach();                               //进行线程分离，也就是当线程执行完任务后，自动清理所有资源
                                                //不用等待它的父进程来等待处理它
	CloudServer srv("./cert.pem", "./key.pem"); //初始化服务端的类，传入证书信息和私钥信息
	srv.Start();                                //开始监听，等待客户端和服务端进行交互

	return 0;
}			
