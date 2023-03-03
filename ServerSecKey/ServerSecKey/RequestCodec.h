#pragma once

#include "Codec.h"
#include "Message.pb.h"
#include <string>

struct RequestInfo
{
	int cmd;
	std::string clientID;
	std::string serverID;
	std::string sign;
	std::string data;
};

class RequestCodec : public Codec
{
public:
	// 空对象
	RequestCodec();
	// 构造函数 用于解码
	RequestCodec(std::string encstr);
	// 构造函数 用于编码
	RequestCodec(RequestInfo *info);
	// init是给空构造准备的 解码使用
	void initMessage(std::string encStr);
	// init是给空构造准备的 编码使用
	void initMessage(RequestInfo* info);
	// 重写父类函数 --》 序列换函数，返回序列化的字符串
	std::string encodeMsg();
	// 重写父类函数 --》 反序列化函数，返回结构体\类对象
	void* decodeMsg();

	~RequestCodec();

private:
	// 保存要解码的字符串
	std::string m_encStr;
	// 要序列化的类都在msg中
	RequestMsg m_msg;
};

