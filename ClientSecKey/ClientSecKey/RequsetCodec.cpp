#include "RequestCodec.h"

// 空构造函数
RequestCodec::RequestCodec()
{
}

// 空析构函数
RequestCodec::~RequestCodec()
{
}

// 构造函数 解码时使用
RequestCodec::RequestCodec(std::string encstr)
{
	initMessage(encstr);
}

// 构造函数 编码是使用
RequestCodec::RequestCodec(RequestInfo* info)
{
	initMessage(info);
}

// 初始化用于构造 解码时使用
void RequestCodec::initMessage(std::string encStr)
{
	this->m_encStr = encStr;
}

// 初始化用于构造 编码时使用
void RequestCodec::initMessage(RequestInfo* info)
{
	this->m_msg.set_cmdtype(info->cmd);
	this->m_msg.set_clientid(info->clientID);
	this->m_msg.set_serverid(info->serverID);
	this->m_msg.set_sign(info->sign);
	this->m_msg.set_data(info->data);
}

// 序列化
std::string RequestCodec::encodeMsg()
{
	std::string output;
	m_msg.SerializeToString(&output);
	return output;
}

// 反序列化
void* RequestCodec::decodeMsg()
{
	m_msg.ParseFromString(m_encStr);
	return &m_msg;
}
