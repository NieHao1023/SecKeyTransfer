#pragma once

#include <string>


struct ClientInfo
{
	std::string serverID;
	std::string clientID;
	std::string ip;
	unsigned short port;
};


class ClientOP
{
public:
	ClientOP(std::string jsonFile);
	~ClientOP();

	// 秘钥协商
	bool seckeyAgree();

	// 秘钥校验
	void seckeyCheck();

	// 秘钥注销
	void seckeyLogout();

private:
	ClientInfo m_info;
};



