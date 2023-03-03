#include "TcpServer.h"


TcpServer::TcpServer()
{
}


TcpServer::~TcpServer()
{
}


// 服务器设置监听
int TcpServer::setListen(unsigned short port)
{
	int ret = 0;
	
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof servaddr);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// 创建监听套接字
	this->m_lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->m_lfd == -1)
	{
		ret = errno;
		printf("TcpServer scoket() errno:%d\n", ret);
		return ret;
	}

	// 设置端口复用
	int on = 1;
	ret = setsockopt(this->m_lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	if (ret == -1)
	{
		ret = errno;
		printf("TcpServer setsockopt() errno:%d\n", ret);
		return ret;
	}

	// 监听的套接字绑定到本地的ip和端口
	ret = bind(this->m_lfd, (struct sockaddr*)&servaddr, sizeof servaddr);
	if (ret == -1)
	{
		ret = errno;
		printf("TcpServer bind() errno:%d\n", ret);
		return ret;
	}

	ret = listen(this->m_lfd, 128);
	if (ret == -1)
	{
		ret = errno;
		printf("TcpServer listen() errno:%d\n", ret);
		return ret;
	}

	return ret;
}


// 等待客户端请求，默认连接超时时间wait_seconds=10000
TcpSocket* TcpServer::acceptConn(int wait_seconds)
{
	int ret;
	if (wait_seconds > 0)
	{
		struct timeval timeout = { wait_seconds, 0 };
		fd_set accept_fdset;
		FD_ZERO(&accept_fdset);
		FD_SET(this->m_lfd, &accept_fdset);
		do
		{
			ret = select(this->m_lfd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret == -1 && errno == EINTR);	// 被信号打断了就继续执行

		if (ret <= 0)
		{
			return nullptr;
		}
	}

	// select有事件发生，三次握手完成
	// 此时使用accpet不会阻塞
	struct sockaddr_in addrCli;
	socklen_t addrlen = sizeof addrCli;
	int connfd = accept(this->m_lfd, (struct sockaddr*)&addrCli, &addrlen);
	if (connfd == -1)
	{
		return nullptr;
	}
	else
	{
		return new TcpSocket(connfd);
	}
}


// 关闭套接字
void TcpServer::closefd()
{
	close(this->m_lfd);
}

