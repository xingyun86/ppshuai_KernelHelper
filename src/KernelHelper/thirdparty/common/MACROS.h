
#pragma once

#ifndef __MACROS_H_
#define __MACROS_H_

#include <map>
#include <vector>
#include <string>

#define __MY_A(V)				#V
#define __MY_W(V)				L##V

#define STRING	std::string
#define WSTRING std::wstring

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRING std::string
#define __MY_T(V)				#V
#define __MY__TEXT(quote)		quote
#else
#define TSTRING std::wstring
#define __MY_T(V)				L###V
#define __MY__TEXT(quote)		L##quote
#endif

#define _MY_TEXT(x)	__MY__TEXT(x)
#define	_MY_T(x)    __MY__TEXT(x)
#define _tstring TSTRING
#define tstring TSTRING

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

typedef std::vector<STRINGVECTOR> STRINGVECTORVECTOR;
typedef std::vector<WSTRINGVECTOR> WSTRINGVECTORVECTOR;

typedef std::map<std::string, STRINGVECTOR> STRINGVECTORMAP;
typedef std::map<std::wstring, WSTRINGVECTOR> WSTRINGVECTORMAP;

typedef std::map<std::string, std::string> STRINGSTRINGMAP;
typedef std::map<std::wstring, std::wstring> WSTRINGWSTRINGMAP;

typedef std::map<std::string, STRINGSTRINGMAP> STRINGSTRINGMAPMAP;
typedef std::map<std::wstring, WSTRINGWSTRINGMAP> WSTRINGWSTRINGMAPMAP;


#define __MY_A(V)				#V
#define __MY_W(V)				L##V

#if !defined(_UNICODE) && !defined(UNICODE)
#define TSTRINGVECTOR			STRINGVECTOR
#define TSTRINGVECTORVECTOR		STRINGVECTORVECTOR
#define TSTRINGTSTRINGMAP		STRINGSTRINGMAP
#define TSTRINGTSTRINGMAPMAP	STRINGSTRINGMAPMAP
#define TSTRINGVECTORMAP		STRINGVECTORMAP
#else
#define TSTRINGVECTOR			WSTRINGVECTOR
#define TSTRINGVECTORVECTOR		WSTRINGVECTORVECTOR
#define TSTRINGTSTRINGMAP		WSTRINGWSTRINGMAP
#define TSTRINGTSTRINGMAPMAP	WSTRINGWSTRINGMAPMAP
#define TSTRINGVECTORMAP		WSTRINGVECTORMAP
#endif // !defined(_UNICODE) && !defined(UNICODE)

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
typedef enum LaunchType {
	LTYPE_0 = 0, //立即
	LTYPE_1 = 1, //直等
	LTYPE_2 = 2, //延迟(设定等待时间)
}LAUNCHTYPE;

#endif //__MACROS_H_