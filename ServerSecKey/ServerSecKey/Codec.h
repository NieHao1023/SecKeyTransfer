#pragma once

#include <iostream>
#include <string>

class Codec
{
public:
	Codec();
	virtual std::string encodeMsg();
	virtual void* decodeMsg();
	// 虚析构函数 ---> 多态
	virtual ~Codec();

private:

};

