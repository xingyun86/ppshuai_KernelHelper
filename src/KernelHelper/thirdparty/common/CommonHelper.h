#pragma once

#ifndef __USUALLYUTILITY_H_
#define __USUALLYUTILITY_H_

#include "MACROS.h"
#include <map>
#include <regex>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <tchar.h>
#include <string.h>
//#include <windows.h>
#include <winsock2.h>
#include <shellapi.h>
#include <sys/stat.h>
#include <iconv.h>
using namespace std;

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

typedef std::vector<STRINGVECTOR> STRINGVECTORVECTOR;
typedef std::vector<WSTRINGVECTOR> WSTRINGVECTORVECTOR;

__inline static std::string STRING_FORMAT_A(const CHAR * paFormat, ...)
{
	INT nAS = 0;
	std::string A = ("");
	
	va_list valist = { 0 };

	va_start(valist, paFormat);

	nAS = _vscprintf_p(paFormat, valist);
	if (nAS > 0)
	{
		A.resize((nAS + sizeof(CHAR)) * sizeof(CHAR), ('\0'));
		_vsnprintf((CHAR *)A.c_str(), nAS * sizeof(CHAR), paFormat, valist);
	}

	va_end(valist);

	return A.c_str();
}

__inline static std::wstring STRING_FORMAT_W(const WCHAR * pwFormat, ...)
{
	INT nWS = 0;
	std::wstring W = (L"");

	va_list valist = { 0 };

	va_start(valist, pwFormat);

	nWS = _vscwprintf_p(pwFormat, valist);
	if (nWS > 0)
	{
		W.resize((nWS + sizeof(WCHAR)) * sizeof(WCHAR), (L'\0'));
		_vsnwprintf((WCHAR *)W.c_str(), nWS * sizeof(WCHAR), pwFormat, valist);
	}

	va_end(valist);

	return W.c_str();
}

//解析错误标识为字符串
__inline static std::string ParseErrorA(DWORD dwErrorCodes, HINSTANCE hInstance = NULL)
{
	BOOL bResult = FALSE;
	HLOCAL hLocal = NULL;
	std::string strErrorText = ("");

	bResult = ::FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		hInstance,
		dwErrorCodes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPSTR)&hLocal,
		0,
		NULL);
	if (!bResult)
	{
		if (hInstance)
		{
			bResult = ::FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hInstance,
				dwErrorCodes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				(LPSTR)&hLocal,
				0,
				NULL);
			if (!bResult)
			{
				// failed
				// Unknown error code %08x (%d)
				strErrorText = STRING_FORMAT_A(("Unknown error code 0x%08X"), dwErrorCodes);
			}
		}
	}

	if (bResult && hLocal)
	{
		// Success
		LPSTR pT = (LPSTR)strchr((LPCSTR)hLocal, ('\r'));
		if (pT != NULL)
		{
			//Lose CRLF
			*pT = ('\0');
		}
		strErrorText = (LPCSTR)hLocal;
	}

	if (hLocal)
	{
		::LocalFree(hLocal);
		hLocal = NULL;
	}

	return strErrorText;
}
//解析错误标识为字符串
__inline static std::wstring ParseErrorW(DWORD dwErrorCodes, HINSTANCE hInstance = NULL)
{
	BOOL bResult = FALSE;
	HLOCAL hLocal = NULL;
	std::wstring strErrorText = (L"");

	bResult = ::FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		hInstance,
		dwErrorCodes,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		(LPWSTR)&hLocal,
		0,
		NULL);
	if (!bResult)
	{
		if (hInstance)
		{
			bResult = ::FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hInstance,
				dwErrorCodes,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
				(LPWSTR)&hLocal,
				0,
				NULL);
			if (!bResult)
			{
				// failed
				// Unknown error code %08x (%d)
				strErrorText = STRING_FORMAT_W((L"Unknown error code 0x%08X"), dwErrorCodes);
			}
		}
	}

	if (bResult && hLocal)
	{
		// Success
		LPWSTR pT = (LPWSTR)wcschr((LPCWSTR)hLocal, (L'\r'));
		if (pT != NULL)
		{
			//Lose CRLF
			*pT = (L'\0');
		}
		strErrorText = (LPCWSTR)hLocal;
	}

	if (hLocal)
	{
		::LocalFree(hLocal);
		hLocal = NULL;
	}

	return strErrorText;
}

__inline static std::string ToUpperCaseA(LPCSTR pA)
{
	return strupr((LPSTR)pA);
}
__inline static std::wstring ToUpperCaseW(LPCWSTR pW)
{
	return _wcsupr((LPWSTR)pW);
}
__inline static std::string ToLowerCaseA(LPCSTR pA)
{
	return strlwr((LPSTR)pA);
}
__inline static std::wstring ToLowerCaseW(LPCWSTR pW)
{
	return _wcsupr((LPWSTR)pW);
}

__inline static std::string GetFilePathDriveA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDrive;
}
__inline static std::wstring GetFilePathDriveW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDrive;
}
__inline static std::string GetFilePathDirA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDir;
}
__inline static std::wstring GetFilePathDirW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szDir;
}
__inline static std::string GetFilePathExtA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szExt;
}
__inline static std::wstring GetFilePathExtW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szExt;
}
__inline static std::string GetFilePathFnameA(LPCSTR lpFileName)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szFname;
}
__inline static std::wstring GetFilePathFnameW(LPCWSTR lpFileName)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);

	return szFname;
}
__inline static void SplitFilePathA(LPCSTR lpFileName, std::string & strDrive, 
	std::string & strDir, std::string & strFname, std::string & strExt)
{
	CHAR szDrive[_MAX_DRIVE] = { 0 };
	CHAR szDir[_MAX_DIR] = { 0 };
	CHAR szFname[_MAX_FNAME] = { 0 };
	CHAR szExt[_MAX_EXT] = { 0 };

	_splitpath(lpFileName, szDrive, szDir, szFname, szExt);
	strDrive = szDrive;
	strDir = szDir;
	strFname = szFname;
	strExt = szExt;
}
__inline static void SplitFilePathW(LPCWSTR lpFileName, std::wstring & strDrive,
	std::wstring & strDir, std::wstring & strFname, std::wstring & strExt)
{
	WCHAR szDrive[_MAX_DRIVE] = { 0 };
	WCHAR szDir[_MAX_DIR] = { 0 };
	WCHAR szFname[_MAX_FNAME] = { 0 };
	WCHAR szExt[_MAX_EXT] = { 0 };

	_wsplitpath(lpFileName, szDrive, szDir, szFname, szExt);
	strDrive = szDrive;
	strDir = szDir;
	strFname = szFname;
	strExt = szExt;
}

__inline static void * MemoryRealloc(void * p, size_t s)
{
	return realloc(p, s);
}
__inline static void MemoryRelease(void ** p)
{
	free((*p));	(*p) = 0;
}

//初始化调试窗口显示
__inline static void InitDebugConsole()
{
	FILE *pStdOut = stdout;
	FILE *pStdIn = stdin;
	FILE *pStdErr = stderr;

	if (!AllocConsole())
	{
		_TCHAR tErrorInfos[16384] = { 0 };
		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("控制台生成失败! 错误代码:0x%X。"), GetLastError());
		MessageBox(NULL, tErrorInfos, _T("错误提示"), 0);
		return;
	}
	SetConsoleTitle(_T("TraceDebugWindow"));

	pStdOut = _tfreopen(_T("CONOUT$"), _T("w"), stdout);
	pStdIn = _tfreopen(_T("CONIN$"), _T("r"), stdin);
	pStdErr = _tfreopen(_T("CONERR$"), _T("w"), stderr);
	_tsetlocale(LC_ALL, _T("chs"));
}

//释放掉调试窗口显示
__inline static void ExitDebugConsole()
{
	FreeConsole();
}

//获取毫秒时间计数器(返回结果为100纳秒的时间, 1ns=1 000 000ms=1000 000 000s)
#define MILLI_100NANO (ULONGLONG)(1000000ULL / 100ULL)
__inline static std::string GetCurrentSystemTimeA()
{
	CHAR szTime[MAXCHAR] = {0};
	SYSTEMTIME st = { 0 };
	//FILETIME ft = { 0 };
	//::GetSystemTimeAsFileTime(&ft);
	::GetLocalTime(&st);
	//::GetSystemTime(&st);
	//::SystemTimeToFileTime(&st, &ft);
	//::SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
	wsprintfA(szTime, ("%04d-%02d-%02d %02d:%02d:%02d.%03d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return std::string(szTime);
}
__inline static std::wstring GetCurrentSystemTimeW()
{
	WCHAR wzTime[MAXCHAR] = { 0 };
	SYSTEMTIME st = { 0 };
	//FILETIME ft = { 0 };
	//::GetSystemTimeAsFileTime(&ft);
	::GetLocalTime(&st);
	//::GetSystemTime(&st);
	//::SystemTimeToFileTime(&st, &ft);
	//::SystemTimeToTzSpecificLocalTime(NULL, &st, &st);
	wsprintfW(wzTime, (L"%04d-%02d-%02d %02d:%02d:%02d.%03d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return std::wstring(wzTime);
}

#if !defined(_UNICODE) && !defined(UNICODE)
#define ToUpperCase				ToUpperCaseA
#define ToLowerCase				ToLowerCaseA
#define STRING_FORMAT			STRING_FORMAT_A
#define GetCurrentSystemTime	GetCurrentSystemTimeA
#define ParseError				ParseErrorA
#define GetFilePathDrive		GetFilePathDriveA
#define GetFilePathDir			GetFilePathDirA
#define GetFilePathExt			GetFilePathExtA
#define GetFilePathFname		GetFilePathFnameA
#define SplitFilePath			SplitFilePathA

#else
#define ToUpperCase				ToUpperCaseW
#define ToLowerCase				ToLowerCaseW
#define STRING_FORMAT			STRING_FORMAT_W
#define GetCurrentSystemTime	GetCurrentSystemTimeW
#define ParseError				ParseErrorW
#define GetFilePathDrive		GetFilePathDriveW
#define GetFilePathDir			GetFilePathDirW
#define GetFilePathExt			GetFilePathExtW
#define GetFilePathFname		GetFilePathFnameW
#define SplitFilePath			SplitFilePathW
#endif // !defined(_UNICODE) && !defined(UNICODE)

//返回值单位为100ns
__inline static LONGLONG GetCurrentTimerTicks()
{
	FILETIME ft = { 0 };
	SYSTEMTIME st = { 0 };
	ULARGE_INTEGER u = { 0, 0 };
	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);
	u.HighPart = ft.dwHighDateTime;
	u.LowPart = ft.dwLowDateTime;
	return u.QuadPart;
}

//获取运行时间间隔差值(输入参数单位为100纳秒)
__inline static LONGLONG GetIntervalTimerTicks(LONGLONG llTime)
{
	return (LONGLONG)((GetCurrentTimerTicks() - llTime) / MILLI_100NANO);
}
//时间间隔差值(输入参数单位为100纳秒)
__inline static LONGLONG SubtractTimerTicks(LONGLONG llTimeA, LONGLONG llTimeB)
{
	return (LONGLONG)((llTimeA - llTimeB) / MILLI_100NANO);
}

#if !defined(_DEBUG) && !defined(DEBUG)
#define START_TIMER_TICKS(x)
#define RESET_TIMER_TICKS(x)
#define CLOSE_TIMER_TICKS(x)
#else
#define START_TIMER_TICKS(x) ULONGLONG ull##x = PPSHUAI::GetCurrentTimerTicks();
#define RESET_TIMER_TICKS(x) ull##x = PPSHUAI::GetCurrentTimerTicks();
#define CLOSE_TIMER_TICKS(x) printf(("%s %s: %s() %llu ms\r\n"), PPSHUAI::GetCurrentSystemTimeA().c_str(), #x, __FUNCTION__, (PPSHUAI::GetCurrentTimerTicks() - ull##x) / MILLI_100NANO);
#endif

__inline static size_t file_reader(std::string&data, std::string filename, std::string mode = "rb")
{
#define DATA_BASE_SIZE	0x200

	FILE * pF = 0;
	size_t size = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		while (!feof(pF))
		{
			data.resize(data.size() + DATA_BASE_SIZE);
			size += fread((void *)(data.c_str() + data.size() - DATA_BASE_SIZE), sizeof(char), DATA_BASE_SIZE, pF);
		}
		fclose(pF);
		pF = 0;
	}

	return size;

#undef DATA_BASE_SIZE
}

__inline static size_t file_writer(std::string data, std::string filename, std::string mode = "wb")
{
	FILE * pF = 0;
	size_t size = 0;

	pF = fopen(filename.c_str(), mode.c_str());
	if (pF)
	{
		size = fwrite((void *)(data.c_str()), sizeof(char), data.size(), pF);
		fclose(pF);
		pF = 0;
	}

	return size;
}

__inline static bool string_regex_valid(std::string data, std::string pattern)
{
	return std::regex_match(data, std::regex(pattern));
}

__inline static int string_regex_replace_all(std::string & result, std::string & data, std::string replace, std::string pattern, std::regex_constants::match_flag_type matchflagtype = std::regex_constants::match_default)
{
	int nresult = (-1);
	try
	{
		data = std::regex_replace(data, std::regex(pattern), replace, matchflagtype);
		nresult = data.length();
	}
	catch (const std::exception & e)
	{
		result = e.what();
	}
	return nresult;
}

__inline static bool string_regex_find(std::string & result, STRINGVECTORVECTOR & svv, std::string & data, std::string pattern)
{
	std::smatch smatch;
	bool bresult = false;

	result = ("");

	try
	{
		std::string::const_iterator itb = data.begin();
		std::string::const_iterator ite = data.end();
		while (std::regex_search(itb, ite, smatch, std::regex(pattern)))//如果匹配成功  
		{
			if (smatch.size() > 1)
			{
				for (size_t stidx = svv.size() + 1; stidx < smatch.size(); stidx++)
				{
					svv.push_back(STRINGVECTOR());
				}
				for (size_t stidx = 1; stidx < smatch.size(); stidx++)
				{
					svv.at(stidx - 1).push_back(std::string(smatch[stidx].first, smatch[stidx].second));
					itb = smatch[stidx].second;
				}
				bresult = true;
			}
		}
	}
	catch (const std::exception & e)
	{
		result = e.what();
	}

	return bresult;
}

//获取指定两个字符串之间的字符串数据
__inline static std::string string_reader(std::string strData,
                                      std::string strStart, std::string strFinal,
                                      bool bTakeStart = false, bool bTakeFinal = false)
{
    std::string strRet = ("");
    std::string::size_type stStartPos = std::string::npos;
    std::string::size_type stFinalPos = std::string::npos;
    stStartPos = strData.find(strStart);
    if(stStartPos != std::string::npos)
    {
        stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
        if(stFinalPos != std::string::npos)
        {
            if(!bTakeStart)
            {
                stStartPos += strStart.length();
            }
            if(bTakeFinal)
            {
                stFinalPos += strFinal.length();
            }
            strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
        }
    }

    return strRet;
}
//获取指定两个字符串之间的字符串数据
__inline static std::string::size_type string_reader(std::string &strRet, std::string strData,
	std::string strStart, std::string strFinal, std::string::size_type stPos = 0,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::string::size_type stRetPos = std::string::npos;
	std::string::size_type stStartPos = stPos;
	std::string::size_type stFinalPos = std::string::npos;

	strRet = ("");

	stStartPos = strData.find(strStart, stStartPos);
	if (stStartPos != std::string::npos)
	{
		stRetPos = stFinalPos = strData.find(strFinal, stStartPos + strStart.length());
		if (stFinalPos != std::string::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += strStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += strFinal.length();
			}
			strRet = strData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return stRetPos;
}

__inline static std::string string_replace_all(std::string &strData, std::string strDst, std::string strSrc, std::string::size_type stPos = 0)
{
	while ((stPos = strData.find(strSrc, stPos)) != std::string::npos)
	{
		strData.replace(stPos, strSrc.length(), strDst);
	}

	return strData;
}

__inline static void string_split_to_vector(STRINGVECTOR & sv, std::string strData, std::string strSplitter, std::string::size_type stPos = 0)
{
	std::string strTmp = ("");
	std::string::size_type stIdx = 0;
	std::string::size_type stSize = strData.length();

	while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
	{
		strTmp = strData.substr(stIdx, stPos - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);

		stIdx = stPos + strSplitter.length();
	}

	if (stIdx < stSize)
	{
		strTmp = strData.substr(stIdx, stSize - stIdx);
		if (!strTmp.length())
		{
			strTmp = ("");
		}
		sv.push_back(strTmp);
	}
}

//获取指定两个字符串之间的字符串数据
__inline static std::wstring wstring_reader(std::wstring wstrData,
                                      std::wstring wstrStart, std::wstring wstrFinal,
                                      bool bTakeStart = false, bool bTakeFinal = false)
{
    std::wstring wstrRet = (L"");
    std::wstring::size_type stStartPos = std::wstring::npos;
    std::wstring::size_type stFinalPos = std::wstring::npos;
    stStartPos = wstrData.find(wstrStart);
    if(stStartPos != std::wstring::npos)
    {
        stFinalPos = wstrData.find(wstrFinal, stStartPos + wstrStart.length());
        if(stFinalPos != std::wstring::npos)
        {
            if(!bTakeStart)
            {
                stStartPos += wstrStart.length();
            }
            if(bTakeFinal)
            {
                stFinalPos += wstrFinal.length();
            }
            wstrRet = wstrData.substr(stStartPos, stFinalPos - stStartPos);
        }
    }

    return wstrRet;
}

//获取指定两个字符串之间的字符串数据
__inline static std::wstring::size_type wstring_reader(std::wstring &wstrRet, std::wstring wstrData,
	std::wstring wstrStart, std::wstring wstrFinal, std::wstring::size_type stPos = std::wstring::npos,
	bool bTakeStart = false, bool bTakeFinal = false)
{
	std::wstring::size_type stRetPos = std::wstring::npos;
	std::wstring::size_type stStartPos = stPos;
	std::wstring::size_type stFinalPos = std::wstring::npos;

	wstrRet = (L"");

	stStartPos = wstrData.find(wstrStart);
	if (stStartPos != std::wstring::npos)
	{
		stRetPos = stFinalPos = wstrData.find(wstrFinal, stStartPos + wstrStart.length());
		if (stFinalPos != std::wstring::npos)
		{
			if (!bTakeStart)
			{
				stStartPos += wstrStart.length();
			}
			if (bTakeFinal)
			{
				stFinalPos += wstrFinal.length();
			}
			wstrRet = wstrData.substr(stStartPos, stFinalPos - stStartPos);
		}
	}

	return stRetPos;
}
__inline static std::wstring wstring_replace_all(std::wstring &wstrData, std::wstring wstrDst, std::wstring wstrSrc, std::wstring::size_type stPos = 0)
{
	while ((stPos = wstrData.find(wstrSrc, stPos)) != std::wstring::npos)
	{
		wstrData.replace(stPos, wstrSrc.length(), wstrDst);
	}

	return wstrData;
}
__inline static void wstring_split_to_vector(WSTRINGVECTOR & wsv, std::wstring wstrData, std::wstring wstrSplitter, std::wstring::size_type stPos = 0)
{
	std::wstring wstrTemp = (L"");
	std::wstring::size_type stIdx = 0;
	std::wstring::size_type stSize = wstrData.length();

	while ((stPos = wstrData.find(wstrSplitter, stIdx)) != std::wstring::npos)
	{
		wstrTemp = wstrData.substr(stIdx, stPos - stIdx);
		if (!wstrTemp.length())
		{
			wstrTemp = (L"");
		}
		wsv.push_back(wstrTemp);

		stIdx = stPos + wstrSplitter.length();
	}

	if (stIdx < stSize)
	{
		wstrTemp = wstrData.substr(stIdx, stSize - stIdx);
		if (!wstrTemp.length())
		{
			wstrTemp = (L"");
		}
		wsv.push_back(wstrTemp);
	}
}

__inline static void DebugTrace(LPCTSTR lpszFormat, ...)
{
#if !defined(MAX_DEBUGTRACE_NUM)
#define MAX_DEBUGTRACE_NUM 65536
	va_list args;
	tstring tstr(MAX_DEBUGTRACE_NUM, _T('\0'));
	va_start(args, lpszFormat);
	_vsntprintf((_TCHAR *)tstr.c_str(), tstr.size(), lpszFormat, args);
	OutputDebugString(tstr.c_str());
	va_end(args);
#undef MAX_DEBUGTRACE_NUM
#endif // MAX_DEBUGTRACE_NUM
}

#define DEBUG_TRACE DebugTrace

__inline static void LogDebugPrint(LPCTSTR lpszFormat, ...)
{
#if !defined(MAX_DEBUGPRINT_NUM)
#define MAX_DEBUGPRINT_NUM 65536
	va_list args;
	tstring tstr(MAX_DEBUGPRINT_NUM, _T('\0'));
	va_start(args, lpszFormat);
	_vsntprintf((_TCHAR *)tstr.c_str(), tstr.size(), lpszFormat, args);
	_ftprintf(stdout, _T("%s"), tstr.c_str());
	va_end(args);
#undef MAX_DEBUGPRINT_NUM
#endif // MAX_DEBUGPRINT_NUM
}

#define LOG_DEBUG_PRINT LogDebugPrint

__inline static void LogErrorPrint(LPCTSTR lpszFormat, ...)
{
#if !defined(MAX_ERRORPRINT_NUM)
#define MAX_ERRORPRINT_NUM 65536
	va_list args;
	tstring tstr(MAX_ERRORPRINT_NUM, _T('\0'));
	va_start(args, lpszFormat);
	_vsntprintf((_TCHAR *)tstr.c_str(), tstr.size(), lpszFormat, args);
	_ftprintf(stderr, _T("%s"), tstr.c_str());
	va_end(args);
#undef MAX_ERRORPRINT_NUM
#endif // MAX_ERRORPRINT_NUM
}

#define LOG_ERROR_PRINT LogErrorPrint

////初始化调试窗口显示
//__inline static void InitDebugConsole()
//{
//	FILE *pStdOut = stdout;
//	FILE *pStdIn = stdin;
//	FILE *pStdErr = stderr;
//
//	if (!AllocConsole())
//	{
//		_TCHAR tErrorInfos[16384] = { 0 };
//		_sntprintf(tErrorInfos, sizeof(tErrorInfos) / sizeof(_TCHAR), _T("控制台生成失败! 错误代码:0x%X。"), GetLastError());
//		MessageBox(NULL, tErrorInfos, _T("错误提示"), 0);
//		return;
//	}
//	SetConsoleTitle(_T("TraceDebugWindow"));
//
//	pStdOut = _tfreopen(_T("CONOUT$"), _T("w"), stdout);
//	pStdIn = _tfreopen(_T("CONIN$"), _T("r"), stdin);
//	pStdErr = _tfreopen(_T("CONERR$"), _T("w"), stderr);
//	_tsetlocale(LC_ALL, _T("chs"));
//}

////释放掉调试窗口显示
//__inline static void ExitDebugConsole()
//{
//	FreeConsole();
//}

//	ANSI to Unicode
__inline static std::wstring ANSIToUnicode(const std::string str)
{
	int len = 0;
	len = str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t * pUnicode;
	pUnicode = new  wchar_t[(unicodeLen + 1)];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen);
	std::wstring rt;
	rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}

//Unicode to ANSI
__inline static std::string UnicodeToANSI(const std::wstring str)
{
	char* pElementText;
	int iTextLen;
	iTextLen = WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_ACP,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}
//UTF - 8 to Unicode
__inline static std::wstring UTF8ToUnicode(const std::string str)
{
	int len = 0;
	len = str.length();
	int unicodeLen = ::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	wchar_t * pUnicode;
	pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pUnicode,
		unicodeLen);
	std::wstring rt;
	rt = (wchar_t*)pUnicode;
	delete pUnicode;
	return rt;
}
//Unicode to UTF - 8
__inline static std::string UnicodeToUTF8(const std::wstring str)
{
	char*   pElementText;
	int iTextLen;
	iTextLen = WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, sizeof(char) * (iTextLen + 1));
	::WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pElementText,
		iTextLen,
		NULL,
		NULL);
	std::string strText;
	strText = pElementText;
	delete[] pElementText;
	return strText;
}

__inline static std::string TToA(tstring tsT)
{
    std::string str = "";

    #if !defined(UNICODE) && !defined(_UNICODE)
    str = tsT;
    #else
    str = UnicodeToANSI(tsT);
    #endif

    return str;
}

__inline static std::wstring TToW(tstring tsT)
{
    std::wstring wstr = L"";

    #if !defined(UNICODE) && !defined(_UNICODE)
    wstr = ANSIToUnicode(tsT);
    #else
    wstr = tsT;
    #endif

    return wstr;
}

__inline static tstring AToT(std::string str)
{
    tstring ts = _T("");

    #if !defined(UNICODE) && !defined(_UNICODE)
    ts = str;
    #else
    ts = ANSIToUnicode(str);
    #endif

    return ts;
}

__inline static tstring WToT(std::wstring wstr)
{
    tstring ts = _T("");

    #if !defined(UNICODE) && !defined(_UNICODE)
    ts = UnicodeToANSI(wstr);
    #else
    ts = wstr;
    #endif

    return ts;
}

//utf8 转 Unicode
__inline static std::wstring Utf82Unicode(const std::string& utf8string)
{
	int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
	{
		return std::wstring(L"");
	}

	std::vector<wchar_t> resultstring(widesize);

	int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);

	if (convresult != widesize)
	{
		return std::wstring(L"");
	}

	return std::wstring(&resultstring[0]);
}

//unicode 转为 ascii
__inline static std::string WideByte2Acsi(std::wstring& wstrcode)
{
	int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
	if (asciisize == ERROR_NO_UNICODE_TRANSLATION || asciisize == 0)
	{
		return std::string("");
	}
	std::vector<char> resultstring(asciisize);
	int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);

	if (convresult != asciisize)
	{
		return std::string("");
	}

	return std::string(&resultstring[0]);
}

//utf-8 转 ascii
__inline static std::string UTF_82ASCII(std::string& strUtf8Code)
{
	std::string strRet("");
	//先把 utf8 转为 unicode
	std::wstring wstr = Utf82Unicode(strUtf8Code);
	//最后把 unicode 转为 ascii
	strRet = WideByte2Acsi(wstr);
	return strRet;
}

///////////////////////////////////////////////////////////////////////


//ascii 转 Unicode
__inline static std::wstring Acsi2WideByte(std::string& strascii)
{
	int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
	if (widesize == ERROR_NO_UNICODE_TRANSLATION || widesize == 0)
	{
		return std::wstring(L"");
	}
	std::vector<wchar_t> resultstring(widesize);
	int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);


	if (convresult != widesize)
	{
		return std::wstring(L"");
	}

	return std::wstring(&resultstring[0]);
}


//Unicode 转 Utf8
__inline static std::string Unicode2Utf8(const std::wstring& widestring)
{
	int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
	if (utf8size == 0)
	{
		return std::string("");
	}

	std::vector<char> resultstring(utf8size);

	int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);

	if (convresult != utf8size)
	{
		return std::string("");
	}

	return std::string(&resultstring[0]);
}

//ascii 转 Utf8
__inline static std::string ASCII2UTF_8(std::string& strAsciiCode)
{
	std::string strRet("");
	//先把 ascii 转为 unicode
	std::wstring wstr = Acsi2WideByte(strAsciiCode);
	//最后把 unicode 转为 utf8
	strRet = Unicode2Utf8(wstr);
	return strRet;
}

__inline static int codeset_convert(char *from_charset, char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	const char **pin = &inbuf;
	char **pout = &outbuf;

	iconv_t cd = iconv_open(to_charset, from_charset);
	if (cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, (char **)pin, &inlen, pout, &outlen) == -1) return -1;
	iconv_close(cd);
	return 0;
}

/* UTF-8 to GBK  */
__inline static int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return codeset_convert("UTF-8", "GBK", inbuf, inlen, outbuf, outlen);
}

/* GBK to UTF-8 */
__inline static int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return codeset_convert("GBK", "UTF-8", inbuf, inlen, outbuf, outlen);
}

//显示在屏幕中央
__inline static void CenterWindowInScreen(HWND hWnd)
{
	RECT rcWindow = { 0 };
	RECT rcScreen = { 0 };
	SIZE szAppWnd = { 300, 160 };
	POINT ptAppWnd = { 0, 0 };

	// Get workarea rect.
	BOOL fResult = SystemParametersInfo(SPI_GETWORKAREA,   // Get workarea information
		0,              // Not used
		&rcScreen,    // Screen rect information
		0);             // Not used

	GetWindowRect(hWnd, &rcWindow);
	szAppWnd.cx = rcWindow.right - rcWindow.left;
	szAppWnd.cy = rcWindow.bottom - rcWindow.top;

	//居中显示
	ptAppWnd.x = (rcScreen.right - rcScreen.left - szAppWnd.cx) / 2;
	ptAppWnd.y = (rcScreen.bottom - rcScreen.top - szAppWnd.cy) / 2;
	MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
}

//显示在父窗口中央
__inline static void CenterWindowInParent(HWND hWnd, HWND hParentWnd)
{
	RECT rcWindow = { 0 };
	RECT rcParent = { 0 };
	SIZE szAppWnd = { 300, 160 };
	POINT ptAppWnd = { 0, 0 };

	GetWindowRect(hParentWnd, &rcParent);
	GetWindowRect(hWnd, &rcWindow);
	szAppWnd.cx = rcWindow.right - rcWindow.left;
	szAppWnd.cy = rcWindow.bottom - rcWindow.top;

	//居中显示
	ptAppWnd.x = (rcParent.right - rcParent.left - szAppWnd.cx) / 2;
	ptAppWnd.y = (rcParent.bottom - rcParent.top - szAppWnd.cy) / 2;
	MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
}

//根据进程ID终止进程
__inline static void TerminateProcessByProcessId(DWORD dwProcessId)
{
	DWORD dwExitCode = 0;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessId);
	if (hProcess)
	{
		GetExitCodeProcess(hProcess, &dwExitCode);
		TerminateProcess(hProcess, dwExitCode);
		CloseHandle(hProcess);
		hProcess = 0;
	}
}

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
typedef enum LaunchType {
	LTYPE_0 = 0, //立即
	LTYPE_1 = 1, //直等
	LTYPE_2 = 2, //延迟(设定等待时间)
}LAUNCHTYPE;

//传入应用程序文件名称、参数、启动类型及等待时间启动程序
__inline static BOOL LaunchAppProg(tstring tsAppProgName, tstring tsArguments = _T(""), bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT)
{
	BOOL bRet = FALSE;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	DWORD dwCreateFlags = CREATE_NO_WINDOW;
	LPTSTR lpArguments = NULL;

	if (tsArguments.length())
	{
		lpArguments = (LPTSTR)tsArguments.c_str();
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!bNoUI)
	{
		dwCreateFlags = 0;
	}

	// Start the child process.
	bRet = CreateProcess(tsAppProgName.c_str(),   // No module name (use command line)
		lpArguments,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		dwCreateFlags,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi);           // Pointer to PROCESS_INFORMATION structure
	if (bRet)
	{
		switch (type)
		{
		case LTYPE_0:
		{
			// No wait until child process exits.
		}
		break;
		case LTYPE_1:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		break;
		case LTYPE_2:
		{
			// Wait until child process exits.
			WaitForSingleObject(pi.hProcess, dwWaitTime);
		}
		break;
		default:
			break;
		}

		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Exit process.
		TerminateProcessByProcessId(pi.dwProcessId);
	}
	else
	{
		DEBUG_TRACE(_T("CreateProcess failed (%d).\n"), GetLastError());
	}
	return bRet;
}

//系统提权函数
__inline static BOOL EnablePrivilege(LPCTSTR lpszPrivilegeName, BOOL bEnable)
{
	HANDLE hToken = 0;
	TOKEN_PRIVILEGES tp = { 0 };
	LUID luid = { 0 };
	if (OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken) &&
		LookupPrivilegeValue(NULL, lpszPrivilegeName, &luid))
	{
		tp.PrivilegeCount = 0x01;
		tp.Privileges[0].Luid = luid;
		tp.Privileges[0].Attributes = (bEnable) ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL);
		CloseHandle(hToken);
		hToken = 0;
	}

	return (GetLastError() == ERROR_SUCCESS);
}

//检查系统版本是否是Vista或更高的版本
__inline static bool IsOsVersionVistaOrGreater()
{
	OSVERSIONINFOEX ovex = {0};
	_TCHAR tzVersionInfo[MAX_PATH] = {0};

	//设置参数的大小，调用并判断是否成功
	ovex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!GetVersionEx((OSVERSIONINFO *)(&ovex)))
	{
		DEBUG_TRACE(_T("检查系统版本失败\n"));
		return false;
	}
	//通过版本号，判断是否是vista及之后版本
	if (ovex.dwMajorVersion > 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}
//检查并根据系统版本选择打开程序方式
__inline static void ShellExecuteExOpen(tstring tsAppName, tstring tsArguments, tstring tsWorkPath)
{
	if (IsOsVersionVistaOrGreater())
	{
		SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = _T("runas");
		sei.lpFile = tsAppName.c_str();
		sei.lpParameters = tsArguments.c_str();
		sei.lpDirectory = tsWorkPath.c_str();
		sei.nShow = SW_SHOWNORMAL;
		if (!ShellExecuteEx(&sei))
		{
			DWORD dwStatus = GetLastError();
			if (dwStatus == ERROR_CANCELLED)
			{
				DEBUG_TRACE(_T("提升权限被用户拒绝\n"));
			}
			else if (dwStatus == ERROR_FILE_NOT_FOUND)
			{
				DEBUG_TRACE(_T("所要执行的文件没有找到\n"));
			}
			else
            {
                DEBUG_TRACE(_T("失败原因未找到\n"));
            }
		}
	}
	else
	{
		//appPath.Replace(L"\\", L"\\\\");
		ShellExecute(NULL, _T("open"), tsAppName.c_str(), NULL, tsWorkPath.c_str(), SW_SHOWNORMAL);
	}

}
__inline static void LaunchAppProgByAdmin(tstring tsAppProgName, tstring tsArguments, bool bNoUI/* = true*/)
{
	ShellExecuteExOpen(tsAppProgName, tsArguments, _T(""));
	/*const HWND hWnd = 0;
	const _TCHAR * pCmd = _T("runas");
	const _TCHAR * pWorkPath = _T("");
	int nShowType = bNoUI ? SW_HIDE : SW_SHOW;
	::ShellExecute(hWnd, pCmd, tsAppProgName.c_str(), tsArguments.c_str(), pWorkPath, nShowType);*/
}

//程序实例只允许一个
__inline static BOOL RunAppOnce(tstring tsName)
{
	HANDLE hMutexInstance = ::CreateMutex(NULL, FALSE, tsName.c_str());  //创建互斥
	if (hMutexInstance)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//OutputDebugString(_T("互斥检测返回！"));
			CloseHandle(hMutexInstance);
			return FALSE;
		}
	}
	return TRUE;
}

//获取程序工作路径
__inline static tstring GetWorkPath()
{
    tstring tsWorkPath = _T("");
	_TCHAR tWorkPath[MAX_PATH] = { 0 };
	GetCurrentDirectory(MAX_PATH, tWorkPath);
	if(*tWorkPath)
	{
	    tsWorkPath = tstring(tWorkPath) + _T("\\");
	}
	return tsWorkPath;
}

//获取系统临时路径
__inline static tstring GetTempPath()
{
	_TCHAR tTempPath[MAX_PATH] = { 0 };
	GetTempPath(MAX_PATH, tTempPath);
	return tstring(tTempPath);
}

//获取程序文件路径
__inline static tstring GetProgramPath()
{
	tstring tsFilePath = _T("");
	_TCHAR * pFoundPosition = 0;
	_TCHAR tFilePath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, tFilePath, MAX_PATH);
	if (*tFilePath)
	{
		pFoundPosition = _tcsrchr(tFilePath, _T('\\'));
		if (*(++pFoundPosition))
		{
			*pFoundPosition = _T('\0');
		}
		tsFilePath = tFilePath;
	}
	return tsFilePath;
}

//获取系统路径
__inline static tstring GetSystemPath()
{
    tstring tsSystemPath = _T("");
	_TCHAR tSystemPath[MAX_PATH] = { 0 };
	GetSystemDirectory(tSystemPath, MAX_PATH);
	if(*tSystemPath)
	{
	    tsSystemPath = tstring(tSystemPath) + _T("\\");
	}
	return tsSystemPath;
}

__inline static //获取系统路径
tstring GetSystemPathX64()
{
	tstring tsSystemPath = _T("");
	_TCHAR tSystemPath[MAX_PATH] = { 0 };
	GetSystemWow64Directory(tSystemPath, MAX_PATH);
	if (*tSystemPath)
	{
		tsSystemPath = tstring(tSystemPath) + _T("\\");
	}
	return tsSystemPath;
}
__inline static
BOOL FileIsExists(LPCTSTR pFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };

	return (GetFileAttributesEx(pFileName, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &wfad)
		? ((wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) : FALSE);
}
__inline static
BOOL PathIsExists(LPCTSTR pFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
	return (GetFileAttributesEx(pFileName, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &wfad)
		? !((wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) : FALSE);
}
__inline static
BOOL IsPathExists(LPCTSTR pFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
	return (GetFileAttributesEx(pFileName, GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &wfad)
		? !((wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) : FALSE);
}
__inline static
BOOL IsFileExist(LPCTSTR fileName)
{
	HANDLE hFindFile = NULL;
	WIN32_FIND_DATA	findData = { 0 };

	hFindFile = FindFirstFile(fileName, &findData);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		FindClose(hFindFile);
		hFindFile = NULL;
		return TRUE;
	}

	return FALSE;
}
__inline static
BOOL IsFileExistEx(LPCTSTR lpFileName)
{
	WIN32_FILE_ATTRIBUTE_DATA wfad = { 0 };
	GET_FILEEX_INFO_LEVELS gfil = GetFileExInfoStandard;

	if (GetFileAttributes(lpFileName) != INVALID_FILE_ATTRIBUTES)
	{
		return TRUE;
	}
	else
	{
		if (GetFileAttributesEx(lpFileName, gfil, &wfad) &&
			wfad.dwFileAttributes != INVALID_FILE_ATTRIBUTES)
		{
			return TRUE;
		}
	}
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////
// 函数说明：遍历目录获取指定文件列表
// 参    数：输出的文件行内容数据、过滤后缀名、过滤的前缀字符
// 返 回 值：bool返回类型，成功返回true；失败返回false
// 编 写 者: ppshuai 20141112
//////////////////////////////////////////////////////////////////////////
__inline static BOOL DirectoryTraversal(std::map<SIZE_T, TSTRING> * pTTMAP, LPCTSTR lpDirectory = _T("."), LPCTSTR lpFormat = _T(".ext"))
{
	BOOL bResult = FALSE;
	HANDLE hFindFile = NULL;
	WIN32_FIND_DATA wfd = { 0 };
	_TCHAR tRootPath[MAX_PATH + 1] = { 0 };

	//构建遍历根目录
	wsprintf(tRootPath, TEXT("%s\\*%s"), lpDirectory, lpFormat);

	//hFileHandle = FindFirstFileEx(tPathFile, FindExInfoStandard, &wfd, FindExSearchNameMatch, NULL, 0);
	hFindFile = FindFirstFile(tRootPath, &wfd);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		pTTMAP->insert(std::map<SIZE_T, TSTRING>::value_type(pTTMAP->size(), TSTRING(TSTRING(lpDirectory) + _T("\\") + wfd.cFileName)));
		while (FindNextFile(hFindFile, &wfd))
		{
			pTTMAP->insert(std::map<SIZE_T, TSTRING>::value_type(pTTMAP->size(), TSTRING(TSTRING(lpDirectory) + _T("\\") + wfd.cFileName)));
		}
		FindClose(hFindFile);
		hFindFile = NULL;
		bResult = TRUE;
	}

	return bResult;
}
//判断目录是否存在，若不存在则创建
__inline static BOOL CreateCascadeDirectory(LPCTSTR lpPathName, //Directory name
	LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL  // Security attribute
	)
{
	_TCHAR *pToken = NULL;
	_TCHAR tPathTemp[MAX_PATH] = { 0 };
	_TCHAR tPathName[MAX_PATH] = { 0 };

	_tcscpy(tPathName, lpPathName);
	pToken = _tcstok(tPathName, _T("\\"));
	while (pToken)
	{
		_sntprintf(tPathTemp, sizeof(tPathTemp) / sizeof(_TCHAR), _T("%s%s\\"), tPathTemp, pToken);
		if (!IsFileExistEx(tPathTemp))
		{
			//创建失败时还应删除已创建的上层目录，此次略
			if (!CreateDirectory(tPathTemp, lpSecurityAttributes))
			{
				_tprintf(_T("CreateDirectory Failed: %d\n"), GetLastError());
				return FALSE;
			}
		}
		pToken = _tcstok(NULL, _T("\\"));
	}
	return TRUE;
}
#define CMD_PATH_NAME				"CMD.EXE" //相对路径名称

//#define ADB_PATH_NAME				"adb\\adb.exe" //相对路径名称

//获取cmd.exe文件路径
__inline static tstring GetCmdPath()
{
	return GetSystemPath() + _T(CMD_PATH_NAME);
}

//设定cmd.exe路径
static const tstring CMD_FULL_PATH_NAME = GetCmdPath();

//获取adb.exe文件路径
//__inline static tstring GetAdbPath()
//{
//	return /*GetProgramPath()*/GetWorkPath() + _T(ADB_PATH_NAME);
//}

//设定adb.exe路径
//static const tstring ADB_FULL_PATH_NAME = GetAdbPath();

#define ANSI2UTF8(x) UnicodeToUTF8(ANSIToUnicode(x))
#define UTF82ANSI(x) UnicodeToANSI(UTF8ToUnicode(x))

#endif //__USUALLYUTILITY_H_
