#include "ClientOP.h"
#include <sstream>
#include <json/json.h>
#include <fstream>
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "Hash.h"
#include "RsaCrypto.h"
#include "TcpSocket.h"
#include "Message.pb.h"


// 构造函数 解析json文件
ClientOP::ClientOP(std::string jsonFile)
{
	// 读json文件， jsoncpp --》Value
	std::ifstream ifs(jsonFile);
	Json::Reader r;
	Json::Value root;
	r.parse(ifs, root);
	
	// 将root中的键值对value取出来
	this->m_info.serverID = root["ServerID"].asString();
	this->m_info.clientID = root["ClientID"].asString();
	this->m_info.ip = root["ServerIP"].asString();
	this->m_info.port = (unsigned short)root["Port"].asInt();
}


// 析构函数
ClientOP::~ClientOP()
{
}


// 秘钥协商
bool ClientOP::seckeyAgree()
{
	// 0.生成秘钥对，将公钥字符串读出来
	RsaCrypto rsa;
	// 生成秘钥对
	rsa.generateRsakey(1024);
	// 读公钥文件
	std::ifstream ifs("public.pem");
	stringstream str;
	str << ifs.rdbuf();
	
	// 1.初始化序列化数据
	// 序列化的对象 ---》工厂类创建
	RequestInfo reqInfo;
	reqInfo.clientID = m_info.clientID;
	reqInfo.serverID = m_info.serverID;
	reqInfo.cmd = 1;	// 密钥协商
	reqInfo.data = str.str();	// 非对称加密的公钥
	
	Hash sha1(T_SHA1);
	sha1.addData(reqInfo.data);
	reqInfo.sign = rsa.rsaSign(sha1.result()); // 公钥的哈希的签名
	cout << "公钥的哈希的签名完成..." << endl;

	CodecFactory* factory = new RequestFactory(&reqInfo);
	Codec* c = factory->createCodec();
	
	// 得到序列化之后的数据，可以将其发送给服务器
	std::string enCstr = c->encodeMsg();

	// 释放资源
	delete factory;
	delete c;

	// 套接字通信
	TcpSocket* tcp = new TcpSocket;
	// 连接服务器
	int ret = tcp->connectToHost(m_info.ip, m_info.port);
	if (ret != 0)
	{
		cout << "连接服务器失败" << endl;
		return false;
	}
	cout << "连接服务器成功" << endl;

	// 发送序列化的数据
	tcp->sendMsg(enCstr);
	// 等待服务器回复
	std::string msg = tcp->recvMsg();

	// 解析数据 --》解码（反序列化）
	factory = new RespondFactory(msg);
	c = factory->createCodec();
	RespondMsg* resData = (RespondMsg*)c->decodeMsg();
	
	// 判断状态
	if (!resData->status())
	{
		std:cout << "秘钥协商失败" << std::endl;
		return false;
	}

	// 将得到的密文解密
	std::string key = rsa.rsaPriKeyDecrypt(resData->data());
	cout << "对称加密的秘钥" << key << endl;
	
	// 秘钥写入共享内存


	// 释放资源
	delete factory;
	delete c;
	tcp->disConnect();
	delete tcp;


	return true;
}


// 秘钥校验
void ClientOP::seckeyCheck()
{

}


// 秘钥注销
void ClientOP::seckeyLogout()
{

}

