#pragma once

#include <cstring>

class NodeSecKeyInfo
{
public:
	NodeSecKeyInfo() : status(0), seckeyID(0)
	{
		memset(clientID, 0, sizeof clientID);
		memset(serverID, 0, sizeof serverID);
		memset(seckey, 0, sizeof seckey);
	}
	int status;		// 秘钥状态: 1可用, 0:不可用
	int seckeyID;	// 秘钥的编号
	char clientID[12];	// 客户端ID, 客户端的标识
	char serverID[12];	// 服务器ID, 服务器标识
	char seckey[128];	// 对称加密的秘钥
};