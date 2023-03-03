#include "RespondCodec.h"


// 空对象
RespoendCodec::RespoendCodec()
{
}


RespoendCodec::~RespoendCodec()
{
}

// 构造函数 用于解码
RespoendCodec::RespoendCodec(std::string encStr)
{
	initMessage(encStr);
}

// 构造函数 用于编码
RespoendCodec::RespoendCodec(RespondInfo* info)
{
	initMessage(info);
}

// init是给空构造准备的 解码使用
void RespoendCodec::initMessage(std::string encStr)
{
	this->m_encStr = encStr;
}


// init是给空构造准备的 编码使用
void RespoendCodec::initMessage(RespondInfo* info)
{
	this->m_msg.set_status(info->status);
	this->m_msg.set_seckeyid(info->seckeyID);
	this->m_msg.set_clientid(info->clientID);
	this->m_msg.set_serverid(info->serverID);
	this->m_msg.set_data(info->data);
}


// 重写父类函数 --》 序列换函数，返回序列化的字符串
std::string RespoendCodec::encodeMsg()
{
	std::string output;
	this->m_msg.SerializeToString(&output);
	return output;
}

// 重写父类函数 --》 反序列化函数，返回结构体\类对象
void* RespoendCodec::decodeMsg()
{
	m_msg.ParseFromString(m_encStr);
	return &m_msg;
}
