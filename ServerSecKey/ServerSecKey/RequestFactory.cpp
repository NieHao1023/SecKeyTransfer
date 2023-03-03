#include "RequestFactory.h"


RequestFactory::RequestFactory(std::string enc)
{
	this->m_flag = false;
	this->m_encStr = enc;
}


RequestFactory::RequestFactory(RequestInfo* info)
{
	this->m_flag = true;
	this->m_info = info;
}


RequestFactory::~RequestFactory()
{
}


Codec* RequestFactory::createCodec()
{
	Codec* codec = nullptr;
	if (this->m_flag)
	{
		codec = new RequestCodec(this->m_info);
	}
	else
	{
		codec = new RequestCodec(this->m_encStr);
	}
	return codec;
}