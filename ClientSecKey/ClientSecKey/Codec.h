#pragma once
#include <iostream>
#include <string>
#include "Message.pb.h"

using namespace std;

class Codec
{
public:
	Codec();
	virtual std::string encodeMsg();
	virtual void* decodeMsg();

	virtual ~Codec();
};

