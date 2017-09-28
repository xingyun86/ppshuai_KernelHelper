#pragma once

#ifndef __COMMONHEADER_H_
#define __COMMONHEADER_H_



#if !defined(KERNELHELPER_BUILDING)

#pragma comment (lib, "GDI32.LIB")
#pragma comment (lib, "USER32.LIB")
#pragma comment (lib, "ADVAPI32.LIB")
#pragma comment (lib, "WS2_32.lib")
#pragma comment (lib, "MSWSOCK.lib")
#pragma comment (lib, "wldap32.lib")
#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "normaliz.lib")
#pragma comment (lib, "libeay32.lib")
#pragma comment (lib, "ssleay32.lib")
#pragma comment (lib, "zlibstatic.lib")
#pragma comment (lib, "winsqlite.lib")
#pragma comment (lib, "libssh2.lib")
#pragma comment (lib, "libcares.lib")
#pragma comment (lib, "libcurl.lib")
#pragma comment (lib, "lib_json.lib")
#pragma comment (lib, "libgetopt.lib")
#pragma comment (lib, "libuv.lib")
#pragma comment (lib, "libiconv.lib")
#pragma comment (lib, "libcharset.lib")
#pragma comment (lib, "libgettimeofday.lib")
#pragma comment (lib, "kernelhelper.lib")

#endif

#include <log4z.h>

#include "CommonHelper.h"
#include "CurlHelper.h"
#include "PipeHelper.h"
#include "CryptHelper.h"
#include "SocketHelper.h"
#include "Sqlite3Helper.h"

#endif //__COMMONHEADER_H_
