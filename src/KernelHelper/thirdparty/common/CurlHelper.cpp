#include "CurlHelper.h"

//////////////////////////////////////////////////////////////////////
//函数功能:传入URL和要保存的文件名称下载文件
//函数参数:
//		strSavePathFileName		要保存的文件路径名
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60秒
//返回值:
//		0, 成功
//		-1,curl访问URL失败
//		-2,curl初始化失败
//		-3,创建或打开文件失败
int curl_http_get_file(string strSavePathFileName, string strRequestURL, string strHeaderData/* = ""*/, string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	FILE * pFile = 0;
	CURL * pCurl = nullptr;
	curl_slist *plist = nullptr;

	pFile = fopen(strSavePathFileName.c_str(), "wb");
	if (pFile)
	{
		curlCode = curl_global_init(CURL_GLOBAL_DEFAULT);

		pCurl = curl_easy_init();
		if (pCurl)
		{
			// start cookie engine
			curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");
			
			//设置头部数据
			if (strHeaderData.length() > 0)
			{
				plist = curl_slist_append(plist, strHeaderData.c_str());
				curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
			}
			
			curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
			curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
			curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
			curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

			// send it verbose for max debuggaility
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);

			// HTTP/2 please
			curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

			// we use a self-signed test server, skip verification during debugging
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

			curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

			// write to this file
			curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, pFile);
			curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, nDelayTime);
			
			curl_easy_setopt(pCurl, CURLOPT_POST, 0L);
			// Now specify the POST data
			//设置头部数据
			if (strPostFields.length() > 0)
			{
				curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPostFields.c_str());
			}
			curl_easy_setopt(pCurl, CURLOPT_URL, strRequestURL.c_str());

			if (curlopt_handler)
			{
				curlopt_handler(pCurl);
			}

			curlCode = curl_easy_perform(pCurl);

			if (curlCode != CURLE_OK)
			{
				result = -1;//curl访问网页失败
			}
			curl_easy_cleanup(pCurl);
		}
		else
		{
			result = -2;//curl初始化失败
		}
		curl_global_cleanup();

		fclose(pFile);
		pFile = 0;
	}
	else
	{
		result = -3;//创建或打开文件失败
	}

	return result;
}
//////////////////////////////////////////////////////////////////////
//函数功能:传入URL获取JSON字符串
//函数参数:
//		strJsonData				返回的JSON数据字符串
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_get_data(string & strJsonData, string strRequestURL, string strHeaderData/* = ""*/, string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	CURL * pCurl = nullptr;
	curl_slist *plist = nullptr;

	curlCode = curl_global_init(CURL_GLOBAL_DEFAULT);

	pCurl = curl_easy_init();
	if (pCurl)
	{
		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (strHeaderData.length() > 0)
		{
			plist = curl_slist_append(plist, strHeaderData.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_dynamic_data_callback);//调用处理函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strJsonData);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 0L);
		// Now specify the POST data
		//设置头部数据
		if (strPostFields.length() > 0)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPostFields.c_str());
		}
		//char *output = curl_easy_escape(pCurl, strRequestURL.c_str(), strRequestURL.length());
		curl_easy_setopt(pCurl, CURLOPT_URL, strRequestURL.c_str());

		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}

		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
		curl_easy_cleanup(pCurl);
	}
	else
	{
		result = -2;//curl初始化失败
	}
	curl_global_cleanup();

	return result;
}

//////////////////////////////////////////////////////////////////////
//函数功能:POST方法传入URL及参数获取JSON字符串
//函数参数:
//		strJsonData				返回的JSON数据字符串
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_post_data(string & strJsonData, string strRequestURL, string strHeaderData/* = ""*/, string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	CURL * pCurl = nullptr;
	curl_slist *plist = nullptr;

	curlCode = curl_global_init(CURL_GLOBAL_DEFAULT);

	pCurl = curl_easy_init();
	if (pCurl)
	{
		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (strHeaderData.length() > 0)
		{
			plist = curl_slist_append(plist, strHeaderData.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_dynamic_data_callback);//调用处理函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strJsonData);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 1L);

		// Now specify the POST data
		//设置POST数据
		if (strPostFields.length() > 0)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPostFields.c_str());
		}

		curl_easy_setopt(pCurl, CURLOPT_URL, strRequestURL.c_str());
		
		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}

		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
		curl_easy_cleanup(pCurl);
	}
	else
	{
		result = -2;//curl初始化失败
	}
	curl_global_cleanup();

	return result;
}

CURL * curl_http_init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	return curl_easy_init();
}

void curl_http_exit(CURL * pCurl)
{
	curl_easy_cleanup(pCurl);
	curl_global_cleanup();
}
void curl_http_print_cookies(CURL *curl)
{
	CURLcode res;
	struct curl_slist *cookies;
	struct curl_slist *nc;
	int i;
	printf("Cookies, curl knows:\n");
	res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
	if (res != CURLE_OK) {
		fprintf(stderr, "Curl curl_easy_getinfo failed: %s\n",
			curl_easy_strerror(res));
		exit(1);
	}
	nc = cookies, i = 1;
	while (nc) {
		printf("[%d]: %s\n", i, nc->data);
		nc = nc->next;
		i++;
	}
	if (i == 1) {
		printf("(none)\n");
	}
	curl_slist_free_all(cookies);
}
//////////////////////////////////////////////////////////////////////
//函数功能:POST方法传入URL及参数获取JSON字符串
//函数参数:
//      pCurl					CURL句柄
//		strJsonData				返回的JSON数据字符串
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_post_data(CURL * pCurl, string & strJsonData, string strRequestURL, string strHeaderData/* = ""*/, string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	curl_slist *plist = nullptr;

	if (pCurl)
	{
		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (strHeaderData.length() > 0)
		{
			plist = curl_slist_append(plist, strHeaderData.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_dynamic_data_callback);//调用处理掉函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strJsonData);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
		// Now specify the POST data
		//设置POST数据
		if (strPostFields.length() > 0)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPostFields.c_str());
		}
		
		curl_easy_setopt(pCurl, CURLOPT_URL, strRequestURL.c_str());

		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}

		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
	}
	else
	{
		result = -2;//curl初始化失败
	}

	return result;
}
//////////////////////////////////////////////////////////////////////
//函数功能:传入URL获取JSON字符串
//函数参数:
//      pCurl					CURL句柄
//		strJsonData				返回的JSON数据字符串
//		strRequestURL			下载地址URL
//		strHeaderData			要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields			发送的POST域数据
//		bVerbose				是否为相信日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_get_data(CURL * pCurl, string & strJsonData, string strRequestURL, string strHeaderData/* = ""*/, string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	curl_slist *plist = nullptr;

	if (pCurl)
	{
		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (strHeaderData.length() > 0)
		{
			plist = curl_slist_append(plist, strHeaderData.c_str());
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}

		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_dynamic_data_callback);//调用处理函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)&strJsonData);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 0L);
		// Now specify the POST data
		//设置头部数据
		if (strPostFields.length() > 0)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strPostFields.c_str());
		}

		curl_easy_setopt(pCurl, CURLOPT_URL, strRequestURL.c_str());
		
		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}
		
		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
	}
	else
	{
		result = -2;//curl初始化失败
	}

	return result;
}

//////////////////////////////////////////////////////////////////////
//函数功能:POST方法传入URL及参数获取JSON字符串
//函数参数:
//      pCurl					CURL句柄
//		pJsonData				返回的JSON数据字符串
//		nJsonDataSize			返回的JSON数据字符串长度
//		pDownloadURL			下载地址URL
//		pHeaderData				要发送的头部数据字符串数组(\r\n为分隔符)
//		pPostFields				发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_post_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData/* = ""*/, const char * pPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	curl_slist *plist = 0;
	CURLTOOL::CALLBACKDATA * pcbd = (CURLTOOL::CALLBACKDATA *)malloc(sizeof(CURLTOOL::CALLBACKDATA));

	if (pcbd && pCurl)
	{
	    pcbd->init(0, 0, 0);

		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (*pHeaderData)
		{
			plist = curl_slist_append(plist, pHeaderData);
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}
		
		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_native_data_callback);//调用处理掉函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)pcbd);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
		// Now specify the POST data
		//设置POST数据
		if (*pPostFields)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, pPostFields);
		}

		curl_easy_setopt(pCurl, CURLOPT_URL, pRequestUrl);
		
		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}

		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
	}
	else
	{
		result = -2;//curl初始化失败
	}
    if(pcbd)
    {
		if (pcbd->p && pcbd->s)
		{
			memcpy(pJsonData, pcbd->p, pcbd->s > nJsonDataSize ? nJsonDataSize : pcbd->s);
		}
        free(pcbd);
		pcbd = 0;
    }
	return result;
}
//////////////////////////////////////////////////////////////////////
//函数功能:传入URL获取JSON字符串
//函数参数:
//      pCurl					CURL句柄
//		pJsonData				返回的JSON数据字符串
//		nJsonDataSize			返回的JSON数据字符串长度
//		pDownloadURL			下载地址URL
//		pHeaderData				要发送的头部数据字符串数组(\r\n为分隔符)
//		pPostFields				发送的POST域数据
//		bVerbose				是否为详细日志信息
//		nDelayTime				超时设置，默认为60000毫秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_get_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData/* = ""*/, const char * pPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	int result = 0;//成功返回0
	CURLcode curlCode = CURLE_OK;
	curl_slist *plist = 0;
	CURLTOOL::CALLBACKDATA * pcbd = (CURLTOOL::CALLBACKDATA *)malloc(sizeof(CURLTOOL::CALLBACKDATA));

	if (pcbd && pCurl)
	{
        pcbd->init(0, 0, 0);

		// start cookie engine
		curl_easy_setopt(pCurl, CURLOPT_COOKIEFILE, "");

		//设置头部数据
		if (pHeaderData && *pHeaderData)
		{
			plist = curl_slist_append(plist, pHeaderData);
			curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, plist);
		}
		
		curl_easy_setopt(pCurl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(pCurl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(pCurl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		if (bVerbose)
		{
			curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1L);
		}

		// HTTP/2 please
		curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(pCurl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, CURLTOOL::write_native_data_callback);//调用处理函数
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void *)pcbd);//返回的数据，这里可以加个函数指针

		curl_easy_setopt(pCurl, CURLOPT_TIMEOUT_MS, nDelayTime);

		curl_easy_setopt(pCurl, CURLOPT_POST, 0L);
		// Now specify the POST data
		//设置头部数据
		if (pPostFields && *pPostFields)
		{
			curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, pPostFields);
		}

		curl_easy_setopt(pCurl, CURLOPT_URL, pRequestUrl);

		if (curlopt_handler)
		{
			curlopt_handler(pCurl);
		}

		curlCode = curl_easy_perform(pCurl);

		if (curlCode != CURLE_OK)
		{
			result = -1;//curl访问网页失败
		}
	}
	else
	{
		result = -2;//curl初始化失败
	}
    if(pcbd)
    {
		if (pcbd->p && pcbd->s)
		{
			memcpy(pJsonData, pcbd->p, pcbd->s > nJsonDataSize ? nJsonDataSize : pcbd->s);
		}
        free(pcbd);
		pcbd = 0;
    }

	return result;
}


void init_curl_env(CURL ** p_p_curl)
{
	CURLcode curl_code = curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL * p_curl = curl_easy_init();
	if (p_curl)
	{
		*p_p_curl = p_curl;
	}
}
void exit_curl_env(CURL ** p_p_curl)
{
	CURL * p_curl = *p_p_curl;
	if (p_curl)
	{
		// always cleanup
		curl_easy_cleanup(p_curl);
		p_curl = 0;
	}

	curl_global_cleanup();
}

size_t callback_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
}

int visit_sites(CURL *p_curl, std::string strUrl, std::string strHeaderDataList,
	std::string strHeaderFileName/* = ("head.dat")*/, std::string strBodyFileName/* = ("body.html")*/,
	std::string strCookIEFileName/* = ("cookie.dat")*/, std::string strCookIEJarFileName/* = ("cookie.dat")*/,
	std::string strEncodingUncompressMethodType/* = ("")*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	CURLcode curl_code = CURLE_OK;

	FILE *p_header_file = 0;
	FILE *p_body_file = 0;

	std::string str_Url = strUrl;
	struct curl_slist *p_curl_slist_header = 0;
	std::string str_HeaderDataList = strHeaderDataList;
	std::string str_CookIEFileName = strCookIEFileName;
	std::string str_CookIEJarFileName = strCookIEJarFileName;

	p_curl_slist_header = curl_slist_append(p_curl_slist_header, str_HeaderDataList.c_str());
	if (p_curl)
	{
		p_header_file = fopen(strHeaderFileName.c_str(), ("wb"));
		if (!p_header_file)
		{
			printf("open head.out file failed!\n");
			goto __LEAVE_CLOSE;
		}

		p_body_file = fopen(strBodyFileName.c_str(), ("wb"));
		if (!p_body_file)
		{
			printf("open body.html file failed!\n");
			goto __LEAVE_CLOSE;
		}

		curl_easy_setopt(p_curl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(p_curl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(p_curl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

		if (strEncodingUncompressMethodType.length())
		{
			//理想值gzip
			curl_easy_setopt(p_curl, CURLOPT_ACCEPT_ENCODING, strEncodingUncompressMethodType.c_str());
		}

		// HTTP/2 please
		curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(p_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

		curl_easy_setopt(p_curl, CURLOPT_URL, str_Url.c_str());
		curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, p_curl_slist_header);

		curl_easy_setopt(p_curl, CURLOPT_COOKIEFILE, str_CookIEFileName.c_str());//提交cookie
		curl_easy_setopt(p_curl, CURLOPT_COOKIEJAR, str_CookIEJarFileName.c_str());//保存cookie

		curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, callback_write_data);

		curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, p_header_file);
		curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, p_body_file);

		if (curlopt_handler)
		{
			curlopt_handler(p_curl);
		}

		curl_code = curl_easy_perform(p_curl);
	}

__LEAVE_CLOSE:

	if (p_body_file)
	{
		fclose(p_body_file);
		p_body_file = 0;
	}

	if (p_header_file)
	{
		fclose(p_header_file);
		p_header_file = 0;
	}

	if (p_curl_slist_header)
	{
		// free slist
		curl_slist_free_all(p_curl_slist_header);
		p_curl_slist_header = 0;
	}

	return 0;
}

int visit_posts_sites(CURL *p_curl, std::string strUrl, std::string strHeaderDataList, std::string strPostFields,
	std::string strHeaderFileName/* = ("head.dat")*/, std::string strBodyFileName/* = ("body.html")*/,
	std::string strCookIEFileName/* = ("cookie.dat")*/, std::string strCookIEJarFileName/* = ("cookie.dat")*/,
	std::string strEncodingUncompressMethodType/* = ("")*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	CURLcode curl_code = CURLE_OK;

	FILE *p_header_file = 0;
	FILE *p_body_file = 0;

	std::string str_Url = strUrl;
	struct curl_slist *p_curl_slist_header = 0;
	std::string str_HeaderDataList = strHeaderDataList;
	std::string str_PostFields = strPostFields;
	std::string str_HeaderFileName = strHeaderFileName;
	std::string str_BodyFileName = strBodyFileName;
	std::string str_CookIEFileName = strCookIEFileName;
	std::string str_CookIEJarFileName = strCookIEJarFileName;

	p_curl_slist_header = curl_slist_append(p_curl_slist_header, (strHeaderDataList).c_str());
	if (p_curl)
	{
		p_header_file = fopen(strHeaderFileName.c_str(), ("wb"));
		if (!p_header_file)
		{
			printf("open head.out file failed!\n");
			goto __LEAVE_CLOSE;
		}

		p_body_file = fopen(strBodyFileName.c_str(), ("wb"));
		if (!p_body_file)
		{
			printf("open body.html file failed!\n");
			goto __LEAVE_CLOSE;
		}

		curl_easy_setopt(p_curl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(p_curl, CURLOPT_AUTOREFERER, 1L);
		curl_easy_setopt(p_curl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(p_curl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

		if (strEncodingUncompressMethodType.length())
		{
			//理想值gzip
			curl_easy_setopt(p_curl, CURLOPT_ACCEPT_ENCODING, strEncodingUncompressMethodType.c_str());
		}

		// HTTP/2 please
		curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 2L);

		curl_easy_setopt(p_curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

		curl_easy_setopt(p_curl, CURLOPT_URL, str_Url.c_str());
		curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, p_curl_slist_header);

		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L); // 使用自动跳转
		curl_easy_setopt(p_curl, CURLOPT_AUTOREFERER, 1L); // 自动设置Referer
		curl_easy_setopt(p_curl, CURLOPT_POST, 1L); // 发送一个常规的Post请求

		curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, str_PostFields.size());//设置帐号密码，其余的信息是页面要求的，抓包即可看见。
		curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, str_PostFields.c_str());//设置帐号密码，其余的信息是页面要求的，抓包即可看见。

		curl_easy_setopt(p_curl, CURLOPT_COOKIEFILE, strCookIEFileName.c_str());//提交cookie
		curl_easy_setopt(p_curl, CURLOPT_COOKIEJAR, strCookIEJarFileName.c_str());//保存cookie

		curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, callback_write_data);

		curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, p_header_file);
		curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, p_body_file);

		if (curlopt_handler)
		{
			curlopt_handler(p_curl);
		}

		curl_code = curl_easy_perform(p_curl);
	}
__LEAVE_CLOSE:

	if (p_body_file)
	{
		fclose(p_body_file);
		p_body_file = 0;
	}

	if (p_header_file)
	{
		fclose(p_header_file);
		p_header_file = 0;
	}

	if (p_curl_slist_header)
	{
		// free slist
		curl_slist_free_all(p_curl_slist_header);
		p_curl_slist_header = 0;
	}

	return 0;
}
	int visit_sites(std::string strUrl, std::string strHeaderDataList,
		std::string strHeaderFileName/* = ("head.dat")*/, std::string strBodyFileName/* = ("body.html")*/,
		std::string strCookIEFileName/* = ("cookie.dat")*/, std::string strCookIEJarFileName/* = ("cookie.dat")*/,
		std::string strEncodingUncompressMethodType/* = ("")*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	CURL *p_curl = 0;

	init_curl_env(&p_curl);

	visit_sites(p_curl, strUrl, strHeaderDataList, strHeaderFileName, strBodyFileName, strCookIEFileName, strCookIEJarFileName, strEncodingUncompressMethodType);

	exit_curl_env(&p_curl);

	return 0;
}

	int visit_posts_sites(std::string strUrl, std::string strHeaderDataList, std::string strPostFields,
		std::string strHeaderFileName/* = ("head.dat")*/, std::string strBodyFileName/* = ("body.html")*/,
		std::string strCookIEFileName/* = ("cookie.dat")*/, std::string strCookIEJarFileName/* = ("cookie.dat")*/,
		std::string strEncodingUncompressMethodType/* = ("")*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
{
	CURL *p_curl = 0;

	init_curl_env(&p_curl);

	visit_posts_sites(p_curl, strUrl, strHeaderDataList, strHeaderFileName, strBodyFileName, strCookIEFileName, strCookIEJarFileName, strEncodingUncompressMethodType);

	exit_curl_env(&p_curl);

	return 0;
}