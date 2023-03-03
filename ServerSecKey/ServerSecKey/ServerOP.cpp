#include "ServerOP.h"
#include "TcpServer.h"
#include "TcpSocket.h"
#include "RequestCodec.h"
#include "RequestFactory.h"
#include "RespondCodec.h"                
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include "Hash.h"
#include <pthread.h>
#include <fstream>
#include <json/json.h>
#include <iostream>
#include <cstdlib>


ServerOP::ServerOP(std::string jsonFile)
{
	// 读json文件， jsoncpp --》Value
	std::ifstream ifs(jsonFile);
	Json::Reader r;
	Json::Value root;
	r.parse(ifs, root);

	// 将root中的键值对value取出来
	this->m_port = (unsigned short)root["Port"].asInt();
	this->m_serverID = root["ServerID"].asString();
	//this->m_info.clientID = root["ClientID"].asString();
	//this->m_info.ip = root["ServerIP"].asString();
}


ServerOP::~ServerOP()
{
	if (this->m_server != NULL)
	{
		delete this->m_server;
		this->m_server = NULL;
	}
}


void ServerOP::startServer()
{
	m_server = new TcpServer;	// 监听套接字
	m_server->setListen(m_port);
	while (true)
	{
		std::cout << "等待客户端连接..." << std::endl;
		TcpSocket *tcp = m_server->acceptConn();
		if (tcp == nullptr)
		{
			continue;
		}

		// 创建子线程
		pthread_t tid;
		pthread_create(&tid, NULL, workHard, (void*)this);
		pthread_detach(tid);
		
		m_list.insert({ tid, tcp });
	}
}


// 注意线程同步
void* ServerOP::workHard(void* arg)
{
	sleep(1);	// 为了让线程同步
	std::string data = std::string();

	// 读出该线程的tcp套接字  
	ServerOP* op = (ServerOP*)arg;
	TcpSocket* tcp = op->m_list[pthread_self()];
	
	// 1.接受客户端的数据 ---> 编码
	std::string msg = tcp->recvMsg();
	
	// 2.反序列化 ---》 得到原始数据
	CodecFactory* fac = new RequestFactory(msg);
	Codec* c = fac->createCodec();
	RequestMsg* req = (RequestMsg*)c->decodeMsg();

	// 3.取出数据
	switch (req->cmdtype())
	{
	case 1:
		// 秘钥协商
		data = op->seckeyAgree(req);
		break;
	case 2:
		// 秘钥校验
		break;
	case 3:
		// 注销
		break;
	case 4:
		// 
		break;
	default:
		break;
	}

	// 释放资源
	delete fac;
	delete c;

	// tcp对象操作
	tcp->sendMsg(data);
	tcp->disConnect();
	op->m_list.erase(pthread_self());
	delete tcp;

	return nullptr;
}


// 秘钥协商
std::string ServerOP::seckeyAgree(RequestMsg * reqMsg)
{
	// 0.校验签名 ---》公钥解密 ---》得到公钥 
	// 将收到的公钥数据写入本地磁盘
	ofstream ofs("public.pem");
	ofs << reqMsg->data();
	ofs.close();

	// 创建非对称加密对象
	RespondInfo info;
	RsaCrypto rsa("public.pem", false);
	
	// 创建Hash对象
	Hash sha1(T_SHA1);
	sha1.addData(reqMsg->data());
	//cout << "111111111111111111111111" << endl;
	bool flag = rsa.rsaVerify(sha1.result(), reqMsg->sign());	// 校验
	
	NodeSecKeyInfo* seckeyInfo = new NodeSecKeyInfo;
	//cout << "000000000000000000000" << endl;
	if (flag == false)
	{
		std::cout << "签名校验失败..." << std::endl;
		info.status = false;
	}
	else
	{
		std::cout << "签名校验成功..." << std::endl;
		// 1.生成随机字符串 [对称加密的秘钥]
		std::string key = getRandKey(Len16);
		cout << "生成的秘钥：" << key << endl;
		// 秘钥写入共享内存
		
		seckeyInfo->status = 1;
		seckeyInfo->seckeyID = 12;
		strcpy(seckeyInfo->clientID, reqMsg->clientid().data());
		strcpy(seckeyInfo->serverID, reqMsg->serverid().data());
		strcpy(seckeyInfo->seckey, key.data());

		// 2.通过公钥加密
		std::string seckey = rsa.rsaPubKeyEncrypt(key);
		cout << "加密之后的秘钥：" << seckey << endl;
		// 3.初始化回复数据
		info.clientID = reqMsg->clientid();
		info.data = seckey;
		info.seckeyID = 12;
		info.serverID = m_serverID;
		info.status = true;
	}

	// 4.序列化
	CodecFactory* fac = new RespondFactory(&info);
	Codec* c = fac->createCodec();
	std::string encMsg = c->encodeMsg();

	// 5.发送数据
	return encMsg;
}


// 要求秘钥中包含a-z,A-Z,0-9，特殊字符
std::string ServerOP::getRandKey(KeyLen len)
{
	// 设置随机数种子
	srand(time(NULL));

	int flag = 0;
	std::string randStr = string();
	char sc[] = "~!@#$%^&*()_+|{}[]\'\\'";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;
		switch (flag)
		{
		case 0:	// a-z
			randStr.append(1, 'a' + rand() % 26);
			break;
		case 1:	// A-Z
			randStr.append(1, 'A' + rand() % 26);
			break;
		case 2:	// 0-9
			randStr.append(1, '0' + rand() % 10);
			break;
		case 3:	// 特殊字符
			randStr.append(1, sc[rand() % strlen(sc)]);
			break;
		default:
			break;
		}
	}

	return randStr;
}
