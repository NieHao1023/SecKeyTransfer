#pragma once

#include "TcpSocket.h"

class TcpServer
{
public:
	TcpServer();
	~TcpServer();
	// 服务器设置监听
	int setListen(unsigned short port);
	// 等待客户端请求，默认连接超时时间timeout=10000
	TcpSocket* acceptConn(int timeout = 10000);
	// 关闭套接字
	void closefd();

private:
	int m_lfd;	// 监听套接字
};
