#pragma once

#include "CodecFactory.h"
#include "RespondCodec.h"

class RespondFactory : public CodecFactory
{
public:
	RespondFactory(std::string enc);
	RespondFactory(RespondInfo* info);
	~RespondFactory();

	Codec* createCodec();
private:
	bool m_flag;
	std::string m_encStr;
	RespondInfo* m_info;
};

