#pragma once

#include "Codec.h"
#include <string>

class CodecFactory
{
public:
	CodecFactory();
	virtual ~CodecFactory();
	virtual Codec* createCodec();

private:

};