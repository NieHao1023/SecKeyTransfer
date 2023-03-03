#pragma once

#include "CodecFactory.h"
#include "RequestCodec.h"

class RequestFactory : public CodecFactory
{
public:
	RequestFactory(std::string enc);	// 构造函数
	RequestFactory(RequestInfo* info);
	~RequestFactory();	
	Codec* createCodec();	// 创建一个codec
private:
	bool m_flag;
	std::string m_encStr;
	RequestInfo* m_info;
};
