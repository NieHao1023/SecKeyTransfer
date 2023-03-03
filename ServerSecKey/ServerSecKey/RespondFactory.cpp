#include "RespondFactory.h"

RespondFactory::RespondFactory(std::string enc)
{
	this->m_flag = false;
	this->m_encStr = enc;
}


RespondFactory::RespondFactory(RespondInfo* info)
{
	this->m_flag = true;
	this->m_info = info;
}


RespondFactory::~RespondFactory()
{
}


Codec* RespondFactory::createCodec()
{
	Codec* codec = nullptr;
	if (this->m_flag)
	{
		codec = new RespoendCodec(this->m_info);
	}
	else
	{
		codec = new RespoendCodec(this->m_encStr);
	}
	return codec;
}
