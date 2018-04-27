//////////////////////////////////////////////////////////////////////
//
// Created by ppshuai in 2016-08-22
// Common utility interface for crypt data handling.

#pragma once

#ifndef __CRYPTHELPER_H_
#define __CRYPTHELPER_H_


#include <Base64.h>
#include <Rijndael.h>
#include <md5.h>

static const char* my_iv = "";// "1234567812345678";
static const char* my_key = "1234567812345678";


/////////////////////////////////////////////
//��������:aes cbc padding�����ַ���
//��������:
//		in	Ҫ���ܵ��ַ���
//����ֵ:
//		���ܺ���ַ���
string encrypt_cbc(std::string in);

/////////////////////////////////////////////
//��������:aes ecb padding�����ַ���(��Ӧphp��java��aes pkcs5padding����)
//��������:
//		in	Ҫ���ܵ��ַ���
//����ֵ:
//		���ܺ���ַ���
string encrypt_ecb(std::string in);

#endif //__CRYPTHELPER_H_
