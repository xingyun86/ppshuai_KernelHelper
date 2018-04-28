#pragma once

#ifndef __COMMONHEADER_H_
#define __COMMONHEADER_H_



#if !defined(KERNELHELPER_BUILDING)

#pragma comment (lib, "GDI32.LIB")
#pragma comment (lib, "USER32.LIB")
#pragma comment (lib, "COMDLG32.LIB")
#pragma comment (lib, "ADVAPI32.LIB")
#pragma comment (lib, "WS2_32.LIB")
#pragma comment (lib, "WININET.LIB")
#pragma comment (lib, "SHLWAPI.LIB")
#pragma comment (lib, "SHELL32.LIB")
#pragma comment (lib, "MSWSOCK.LIB")
#pragma comment (lib, "WLDAP32.LIB")
#pragma comment (lib, "CRYPT32.LIB")
#pragma comment (lib, "NORMALIZ.LIB")
#pragma comment (lib, "MSIMG32.lib")
#pragma comment (lib, "OLEAUT32.lib")
#pragma comment (lib, "OLE32.lib")
#pragma comment (lib, "OLEACC.lib")
#pragma comment (lib, "GDIPLUS.LIB")
#pragma comment (lib, "COMCTL32.lib")
#pragma comment (lib, "libeay32.lib")
#pragma comment (lib, "ssleay32.lib")
///////////////////////////////////////////
//#pragma comment (lib, "richedit.lib")  //
//#pragma comment (lib, "msftedit50.lib")//
///////////////////////////////////////////
#pragma comment (lib, "zlibstatic.lib")
#pragma comment (lib, "winsqlite.lib")
#pragma comment (lib, "libssh2.lib")
#pragma comment (lib, "libcares.lib")
#pragma comment (lib, "libcurl.lib")
#pragma comment (lib, "libjson.lib")
#pragma comment (lib, "libgetopt.lib")
#pragma comment (lib, "libuv.lib")
#pragma comment (lib, "pthreads.lib")
#pragma comment (lib, "libiconv.lib")
#pragma comment (lib, "libcharset.lib")
#pragma comment (lib, "libgettimeofday.lib")
#pragma comment (lib, "kernelhelper.lib")

#endif

#include "PipeHelper.h"
#include "CurlHelper.h"
#include "CryptHelper.h"
#include "SocketHelper.h"
#include "Sqlite3Helper.h"

#include "MiniDump.h"

#if !defined(KERNELHELPER_BUILDING)
CMiniDumper G_MD(true);
#endif

#endif //__COMMONHEADER_H_
