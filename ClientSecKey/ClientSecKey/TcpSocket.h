#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>


static const int TIMEOUT = 10000;	// 定义默认的超时时间

// 用于TCP通信的套接字类
class TcpSocket
{
public:
	enum ErrorType
	{
		ParamError = 3001,	// 参数不合法
		TimeOutError,		// 超时错误
		MallocError,		// 开辟内存错误
		PeerCloseError
	};

	// 空构造函数
	TcpSocket();
	// 用一个通信套接字实例化对象
	TcpSocket(int connfd);
	// 空析构函数
	~TcpSocket();
	// 连接服务器
	int connectToHost(std::string ip, unsigned short port, int wait_seconds = TIMEOUT);
	// 发送数据给服务器
	int sendMsg(std::string sendData, int wait_seconds = TIMEOUT);
	// 从服务器接受数据
	std::string  recvMsg(int wait_seconds = TIMEOUT);
	// 与服务器断开连接
	void disConnect();

private:
	// 设置I/O为非阻塞模式
	int setNonBlock(int fd);
	// 设置I/O为阻塞模式
	int setBlock(int fd);
	// 超时检测函数，不含读操作
	int readTimeout(unsigned int wait_seconds);
	// 超时检测函数，不含写操作
	int writeTimeout(unsigned int wait_seconds);
	// 带有超时检测的connect函数
	int connectTimeout(struct sockaddr_in * addr, unsigned int wait_seconds);
	// 从缓冲区读取n个字符
	int readn(void* buf, int count);
	// 往缓冲区写入n个字符
	int writen(const void* buf, int count);
	
	int m_socket;	// 用于通信的套接字
};
