//////////////////////////////////////////////////////////////////////
//
// Created by ppshuai in 2016-08-22
// Common utility interface for crypt data handling.

#pragma once


#include <Base64.h>
#include <Rijndael.h>
#include <md5.h>

static const char* my_iv = "";// "1234567812345678";
static const char* my_key = "1234567812345678";


/////////////////////////////////////////////
//函数功能:aes cbc padding加密字符串
//函数参数:
//		in	要加密的字符串
//返回值:
//		加密后的字符串
string encrypt_cbc(std::string in);

/////////////////////////////////////////////
//函数功能:aes ecb padding加密字符串(对应php、java的aes pkcs5padding加密)
//函数参数:
//		in	要加密的字符串
//返回值:
//		加密后的字符串
string encrypt_ecb(std::string in);