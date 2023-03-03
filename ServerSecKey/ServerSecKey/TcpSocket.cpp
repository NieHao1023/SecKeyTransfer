#include "TcpSocket.h"


// 空构造函数
TcpSocket::TcpSocket()
{
}


// 用一个通信套接字实例化对象
TcpSocket::TcpSocket(int connfd)
{
	this->m_socket = connfd;
}


// 空析构函数
TcpSocket::~TcpSocket()
{
}


// 连接服务器
// 成功返回0，其它表示错误
int TcpSocket::connectToHost(std::string ip, unsigned short port, int timeout)
{
	// 先检查参数是否合法
	if (port <= 0 || port >= 65535 || timeout < 0)
	{
		return ParamError;
	}

	int ret = 0;

	// 1.创建套接字
	this->m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->m_socket < 0)	// 检查是否创建成功
	{
		ret = errno;
		printf("function socket() errono：%d\n", ret);
		return errno;
	}
	//printf("this->m_socket:%d\n", this->m_socket);

	// 2.连接服务器
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof servaddr);
	servaddr.sin_family = AF_INET;	// Ipv4
	servaddr.sin_addr.s_addr = inet_addr(ip.data()); // ip地址
	servaddr.sin_port = htons(port);	// 端口号

	ret = connectTimeout(&servaddr, (unsigned int)timeout);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			puts("连接超时...");
			return TimeOutError;
		}
		else
		{
			printf("function connectTimeout() errno:%d\n", errno);
			perror("connectTimeout");
			return errno;
		}
	}
	
	return ret;
}


// 发送数据给服务器
int TcpSocket::sendMsg(std::string data, int timeout)
{
	int ret = writeTimeout((unsigned int)timeout);

	if (ret == 0)	// writeTimeout() 没有出错
	{
		int writed = 0;
		int dataLen = data.size() + 4;	// 添加4Byte 作为数据头，存储数据长度
		unsigned char* netdata = (unsigned char*)malloc(dataLen);
		if (netdata == nullptr)	// 分配堆区失败
		{
			ret = MallocError;
			printf("function client_sendMsg malloc errno:%d\n", ret);
			return ret;
		}

		// 转换为网络字节序
		int netLen = htonl(data.size());
		memcpy(netdata, &netLen, 4);
		memcpy(netdata + 4, data.data(), data.size());

		writed = writen(netdata, dataLen);
		// 释放内存
		if (netdata != nullptr)
		{
			free(netdata);
			netdata = nullptr;
		}

		if (writed < dataLen)	// 发送失败
		{
			return writed;
		}
	}
	else
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = TimeOutError;
			printf("function client_sendMsg timeout errno:%d\n", ret);
		}
	}
	
	return ret;
}


// 从服务器接受数据
std::string  TcpSocket::recvMsg(int timeout)
{
	int ret = readTimeout((unsigned int)timeout);
	if (ret != 0)	// readTimeout调用失败
	{
		if (ret == -1 || errno == ETIMEDOUT)
		{
			puts("TcpSocket::readTimeout TimeoutError...");
		}
		else
		{
			printf("TcpSocket::readTimeout errno:%d...\n", ret);
		}
		return std::string();
	}

	int netdatalen = 0;
	ret = readn(&netdatalen, 4); // 读取数据头 4字节
	if (ret == -1)
	{
		printf("function readn() errno:%d\n", ret);
		return std::string();
	}
	else if (ret < 4)
	{
		printf("function readn() err peer closed:%d\n", ret);
		return std::string();
	}

	int n = ntohl(netdatalen);
	char* tmpBuf = (char*)malloc(n + 1);	// 根据数据头接受数据
	if (tmpBuf == nullptr)
	{
		ret = MallocError;
		printf("malloc errno:%d\n", ret);
		return nullptr;
	}

	ret = readn(tmpBuf, n);	// 读取数据
	if (ret == -1)
	{
		printf("function readn() errno:%d\n", ret);
		return std::string();
	}
	else if (ret < n)
	{
		printf("function readn() err peer closed:%d\n", ret);
		return std::string();
	}

	tmpBuf[n] = '\0';	// 字符串以 '\0' 结尾
	std::string data = std::string(tmpBuf);
	
	if (tmpBuf != nullptr)
	{			
		free(tmpBuf);	// 释放内存
		tmpBuf = nullptr;
	}

	return data;
}


// 与服务器断开连接
void TcpSocket::disConnect()
{
	if (this->m_socket >= 0)
	{
		close(this->m_socket);
	}
}


///////////////////////////////////////////
//////////        子函数         //////////
///////////////////////////////////////////
/* setNonBlock : 设置I/O为非阻塞模式
 * @fd : 文件描述符
 * 返回值：
 *		失败：-1
 *		成功：其他值
 */
int TcpSocket::setNonBlock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1) 
	{
		return flag;
	}

	flag |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flag);
}


/* setBlock : 设置I/O为阻塞模式
 * @fd : 文件描述符
 * 返回值：
 *		失败：-1
 *		成功：其他值
 */
int TcpSocket::setBlock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if (flag == -1)
	{
		return flag;
	}

	flag &= ~O_NONBLOCK;
	return fcntl(fd, F_SETFL, flag);
}


/* readTimeout : 超时检测函数，不含读操作
 * @wait_seconds : 超时时间
 * 返回值：
 *		成功（未超时）：返回0
 *		失败：返回-1，并且errno = ETIMEDOUT
 */
int TcpSocket::readTimeout(unsigned int wait_seconds)
{
	if (wait_seconds <= 0) return 0;

	int ret = 0;
	
	struct timeval timeout = { wait_seconds, 0 };
	fd_set read_fdset;
	FD_ZERO(&read_fdset);
	FD_SET(this->m_socket, &read_fdset);

	// select成功时返回就绪（可读、可写和异常）文件描述符的总数。
	// 如果在超时时间内没有任何文件描述符就绪，select将返回0。
	// select失败时返回 - 1并设置errno。
	// 如果在select等待期间，程序接收到信号，则select立即返回 - 1，并设置errno为EINTR
	do
	{
		ret = select(this->m_socket + 1, &read_fdset, NULL, NULL, &timeout);
	} while (ret < 0 && errno == EINTR);	// 如果被信号打断了就重复做

	if (ret == 0)	// 超时了
	{
		ret == -1;
		errno = ETIMEDOUT;
	}
	else if(ret == 1)// 没有超时
	{
		ret = 0;
	}

	return ret;
}


/* writeTimeout : 超时检测函数，不含写操作
 * @wait_seconds : 超时时间
 * 返回值：
 *		成功（未超时）：返回0
 *		失败：返回-1，并且errno = ETIMEDOUT
 */
int TcpSocket::writeTimeout(unsigned int wait_seconds)
{
	if (wait_seconds <= 0) return 0;

	int ret = 0;

	struct timeval timeout = { wait_seconds, 0 };
	fd_set write_fdset;
	FD_ZERO(&write_fdset);
	FD_SET(this->m_socket, &write_fdset);

	// select成功时返回就绪（可读、可写和异常）文件描述符的总数。
	// 如果在超时时间内没有任何文件描述符就绪，select将返回0。
	// select失败时返回 - 1并设置errno。
	// 如果在select等待期间，程序接收到信号，则select立即返回 - 1，并设置errno为EINTR
	do
	{
		ret = select(this->m_socket + 1, NULL, &write_fdset, NULL, &timeout);
	} while (ret < 0 && errno == EINTR);	// 如果被信号打断了就重复做

	if (ret == 0)	// 超时了
	{
		ret == -1;
		errno = ETIMEDOUT;
	}
	else if (ret == 1)// 没有超时
	{
		ret = 0;
	}

	return ret;
}


/* connectTimeout : 带有超时检测的connect函数
 * @wait_seconds : 超时时间
 * @addr : 要连接的服务器的地址
 * 返回值：
 *		成功（未超时）：返回0
 *		失败：返回-1，并且errno
 */
int TcpSocket::connectTimeout(struct sockaddr_in* addr, unsigned int wait_seconds)
{
	int ret = 0;
	//connect出错时的一种errno值：EINPROGRESS。
	//这种错误发生在对非阻塞的socket调用connect，而连接又没有立即建立时。
	//根据man文档的解释，在这种情况下，我们可以调用select、poll等函数来监听这个连接失败的socket上的可写事件。
	//当select、poll等函数返回后，再利用getsockopt来读取错误码并清除该socket上的错误。
	//如果错误码是0，表示连接成功建立，否则连接失败。
	if (wait_seconds > 0)
	{
		this->setNonBlock(this->m_socket);	// 套接字设置为非阻塞IO
	}

	ret = connect(this->m_socket, (struct sockaddr*)addr, sizeof(struct sockaddr));
	if (ret == -1 && errno == EINPROGRESS) // 返回-1，并且错误码为EINPROGRESS， 表示连接正在进行
	{
		struct timeval timeout = { wait_seconds, 0 };
		fd_set connect_fdset;
		FD_ZERO(&connect_fdset);
		FD_SET(this->m_socket, &connect_fdset);

		do
		{
			// 使用select监听写事件
			ret = select(this->m_socket + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);	// 被信号中断则继续 
	}

	if (ret == 0)	// 超时
	{
		ret = -1;
		errno = ETIMEDOUT;
	}
	else if (ret < 0) // 失败
	{
		return -1;
	}
	else if (ret == 1)
	{
		//ret返回为1（表示套接字可写），可能有两种情况，
		//一种是连接建立成功，一种是套接字产生错误，
		//此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取
		int err;
		socklen_t sockLen = sizeof(err);
		int sockopt_ret = getsockopt(this->m_socket, SOL_SOCKET, SO_ERROR, &err, &sockLen);
		
		if (sockopt_ret == -1)
		{
			puts("getsockopt failed...");
			return -1;
		}
		else if (err == 0)
		{
			ret = 0;	// 建立连接成功
		}
		else
		{
			ret = -1;	// 建立连接失败
			errno = err;
		}
	}
	
	if (wait_seconds > 0)
	{
		this->setBlock(this->m_socket);	// 套接字设置为阻塞IO
	}

	return ret;
}


/* readn : 从缓冲区读取n个字符
 * @buf : 缓冲区
 * @count : 预读取个数
 * 返回值：
 *		成功：返回count
 *		失败：返回-1，读到EOF返回 小于count
 */
int TcpSocket::readn(void* buf, int count)
{
	size_t nleft = count;
	size_t read_ret;
	char* bufp = (char*)buf;

	while (nleft > 0)
	{
		read_ret = read(this->m_socket, bufp, nleft);
		if (read_ret < 0)
		{
			if (errno == EINTR) continue;	// 被信号中断了继续读取、
			else return -1;	// 表示读取失败
		}
		else if (read_ret == 0)
		{
			return count - nleft;
		}
		else	//成功读取了read_ret个字符
		{
			bufp += read_ret;
			nleft -= read_ret;
		}
	}

	return count;
}


/* readn : 往缓冲区写入n个字符
 * @buf : 缓冲区
 * @count : 预发送个数
 * 返回值：
 *		成功：返回count
 *		失败：返回-1
 */
int TcpSocket::writen(const void* buf, int count)
{
	size_t nleft = count;
	ssize_t write_ret;
	char* bufp = (char*)buf;
	
	while (nleft > 0)
	{
		write_ret = write(this->m_socket, bufp, nleft);
		if (write_ret < 0)
		{
			if (errno == EINTR) continue;	// write操作被信号中断了，继续做
			else return -1;
		}
		else if (write_ret == 0)
		{
			continue;
		}
		else
		{
			bufp += write_ret;
			nleft -= write_ret;
		}
	}

	return count;
}
