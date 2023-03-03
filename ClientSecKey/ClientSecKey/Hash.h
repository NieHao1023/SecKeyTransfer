#pragma once


#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <openssl/md5.h>
#include <openssl/sha.h>


enum HashType
{	// 枚举哈希算法
	T_MD5 = 0,
	T_SHA1,
	T_SHA224,
	T_SHA256,
	T_SHA384,
	T_SHA512
};


class Hash
{
public:

	// 得到一个哈希对象, 创建不同的哈希对象
	Hash(HashType type);
	~Hash();
	// 添加数据
	void addData(std::string data);
	// 计算哈希值
	std::string result();

private:
	// md5
	inline void md5Init();
	inline void md5AddData(const char* data);
	std::string md5Result();

	// sha1
	inline void sha1Init();
	inline void sha1AddData(const char* data);
	std::string sha1Result();

	// sha224
	inline void sha224Init();
	inline void sha224AddData(const char* data);
	std::string sha224Result();

	// sha256
	inline void sha256Init();
	inline void sha256AddData(const char* data);
	std::string sha256Result();

	// sha384
	inline void sha384Init();
	inline void sha384AddData(const char* data);
	std::string sha384Result();

	// sha512
	inline void sha512Init();
	inline void sha512AddData(const char* data);
	std::string sha512Result();

private:
	HashType m_type;
	MD5_CTX m_md5;
	SHA_CTX m_sha1;
	SHA256_CTX m_sha224;
	SHA256_CTX m_sha256;
	SHA512_CTX m_sha384;
	SHA512_CTX m_sha512;
};

