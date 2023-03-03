#pragma once

#include <string>
#include <map>
#include "TcpSocket.h"
#include "TcpServer.h"
#include <pthread.h>
#include "Message.pb.h"
#include "SecKeyInfo.h"


// 处理客户端请求
class ServerOP
{
public:
	enum KeyLen{Len16 = 16, Len24 = 24, Len32 = 32};	// 秘钥长度

	ServerOP(std::string json);
	~ServerOP();

	void startServer();	// 启动服务器
	static void* workHard(void* arg); // 线程回调函数
	std::string seckeyAgree(RequestMsg* reqMsg);	// 秘钥协商

private:
	std::string getRandKey(KeyLen len);

private:
	std::string m_serverID;
 	unsigned short m_port;
	std::map<pthread_t, TcpSocket*> m_list;
	std::map<string, NodeSecKeyInfo*> m_key_list;
	TcpServer* m_server = NULL;
};


