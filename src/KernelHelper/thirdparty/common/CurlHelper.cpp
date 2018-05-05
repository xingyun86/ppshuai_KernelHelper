#include "CurlHelper.h"
namespace PPSHUAI{
	namespace CURLTOOL
	{
		int curl_http_exec(
			PFN_REQUEST_HANDLER request_handler/* = 0*/,
			PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/,
			PFN_RESPONSE_HANDLER response_handler/* = 0*/,
			CURL * pCurl/* = 0*/,
			CALLBACKDATA * p_req_url/* = 0*/,
			CALLBACKDATA * p_req_header/* = 0*/,
			CALLBACKDATA * p_req_postfields/* = 0*/,
			CALLBACKDATA * p_resp_header/* = 0*/,
			CALLBACKDATA * p_resp_data/* = 0*/)
		{
			int result = (-1);
			CURL * p_curl = 0;
			CURLcode curl_code = CURLE_OK;
			struct curl_slist *p_curl_slist_header = 0;

			CALLBACKDATA * pcbd_req_url = 0;
			CALLBACKDATA * pcbd_req_header = 0;
			CALLBACKDATA * pcbd_req_postfields = 0;
			CALLBACKDATA * pcbd_resp_header = 0;
			CALLBACKDATA * pcbd_resp_data = 0;

			p_curl = (pCurl != (0)) ? pCurl : curl_easy_init();

			if (p_curl)
			{
				pcbd_req_url = (p_req_url != (0)) ? p_req_url : (CALLBACKDATA *)CALLBACKDATA::startup();
				pcbd_req_header = (p_req_header != (0)) ? p_req_header : (CALLBACKDATA *)CALLBACKDATA::startup();
				pcbd_req_postfields = (p_req_postfields != (0)) ? p_req_postfields : (CALLBACKDATA *)CALLBACKDATA::startup();
				pcbd_resp_header = (p_resp_header != (0)) ? p_resp_header : (CALLBACKDATA *)CALLBACKDATA::startup();
				pcbd_resp_data = (p_resp_data != (0)) ? p_resp_data : (CALLBACKDATA *)CALLBACKDATA::startup();

				// write header and body data		
				curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_native_data_callback);
				curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, (void *)pcbd_resp_header);
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, (void *)pcbd_resp_data);

				// write header and body data
				//std::string str_header = "";
				//std::string str_data = "";
				//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_dynamic_data);
				//curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, (void *)&str_header);
				//curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, (void *)&str_data);

				if (request_handler)
				{
					result = request_handler(p_curl, pcbd_req_url, pcbd_req_header, pcbd_req_postfields);
				}

				// set http header list data
				p_curl_slist_header = curl_slist_append(p_curl_slist_header, pcbd_req_header->p);
				curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, p_curl_slist_header);

				// set request url
				curl_easy_setopt(p_curl, CURLOPT_URL, pcbd_req_url->p);

				// post data
				curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, pcbd_req_postfields->p);


				if (curlopt_handler)
				{
					curlopt_handler(p_curl);
				}

				result = curl_code = curl_easy_perform(p_curl);

				if (response_handler)
				{
					result = response_handler(p_curl, pcbd_resp_header, pcbd_resp_data, curl_code);
				}

				if (!p_resp_data)
				{
					pcbd_resp_data->cleanup();
				}
				if (!p_resp_header)
				{
					pcbd_resp_header->cleanup();
				}

				if (!p_req_postfields)
				{
					pcbd_req_postfields->cleanup();
				}
				if (!p_req_header)
				{
					pcbd_req_header->cleanup();
				}
				if (!p_req_url)
				{
					pcbd_req_url->cleanup();
				}

				if (p_curl_slist_header)
				{
					curl_slist_free_all(p_curl_slist_header);
					p_curl_slist_header = 0;
				}
				if (!pCurl)
				{
					curl_easy_cleanup(p_curl);
					p_curl = 0;
				}
			}

			return result;
		}

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
		int curl_http_get_file(std::string strSavePathFileName, std::string strRequestURL, std::string strHeaderData/* = ""*/, std::string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
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
		int curl_http_get_data(std::string & strJsonData, std::string strRequestURL, std::string strHeaderData/* = ""*/, std::string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
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
		int curl_http_post_data(std::string & strJsonData, std::string strRequestURL, std::string strHeaderData/* = ""*/, std::string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
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
		int curl_http_post_data(CURL * pCurl, std::string & strJsonData, std::string strRequestURL, std::string strHeaderData/* = ""*/, std::string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
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
		int curl_http_get_data(CURL * pCurl, std::string & strJsonData, std::string strRequestURL, std::string strHeaderData/* = ""*/, std::string strPostFields/* = ""*/, bool bVerbose/* = false*/, int nDelayTime/* = 60000*/, CURLTOOL::PFN_CURLOPT_HANDLER curlopt_handler/* = 0*/)
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
			if (pcbd)
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
			if (pcbd)
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
	}
	namespace HTTPTOOL{

		CHttpClient::CHttpClient()
		{
			InitializeParameters();
		}

		CHttpClient::~CHttpClient()
		{
			CleanupRequest();
		}

		VOID CHttpClient::InitializeParameters()
		{
			m_hSession = NULL;
			m_hConnect = NULL;
			m_hRequest = NULL;

			m_hCompleteEvent = NULL;
			m_hCancelEvent = NULL;

			m_dwConnectTimeOut = 60 * 1000;
			m_dwContext = 0;

			memset(m_tzProxy, 0, sizeof(m_tzProxy));
			memset(m_tzProxyBypass, 0, sizeof(m_tzProxyBypass));
			memset(m_tzUsername, 0, sizeof(m_tzUsername));
			memset(m_tzPassword, 0, sizeof(m_tzPassword));
			memset(m_tzUserAgent, 0, sizeof(m_tzUserAgent));
		}

		// 打开HTTP请求函数
		BOOL CHttpClient::StartupRequest(LPCTSTR lpszUrl, HTTP_REQ_METHOD nReqMethod/* = REQ_METHOD_GET*/, HTTP_REQ_VERSION nReqVersion/* = REQ_VERSION_HTTP_11*/, LPCTSTR lpReferrer/* = NULL*/, LPCTSTR * lppAcceptTypes/* = NULL*/)
		{
			BOOL bResult = FALSE;
			_TCHAR szScheme[INTERNET_MAX_URL_LENGTH] = { 0 };
			_TCHAR szHostName[INTERNET_MAX_URL_LENGTH] = { 0 };
			_TCHAR szUrlPath[INTERNET_MAX_URL_LENGTH] = { 0 };

			WORD nPort = 0;
			DWORD dwAccessType = 0L;
			LPTSTR lpMethod = NULL;
			LPTSTR lpVersion = NULL;
			LPTSTR lpProxyUsername = NULL;
			LPTSTR lpProxyPassword = NULL;
			LPTSTR lpszProxy = NULL;
			LPTSTR lpszProxyBypass = NULL;
			LPTSTR lpszUserAgent = NULL;
			INTERNET_STATUS_CALLBACK lpStatusCallBackFunc = 0;
			DWORD dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;

			// 解析Url
			bResult = ExplainURLParse(lpszUrl, szScheme, INTERNET_MAX_URL_LENGTH, szHostName, INTERNET_MAX_URL_LENGTH, nPort, szUrlPath, INTERNET_MAX_URL_LENGTH);
			if (!bResult)
			{
				goto __CLEAN_UP__;
			}

			// 创建事件句柄
			m_hCompleteEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hCancelEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

			if (NULL == m_hCompleteEvent || NULL == m_hCancelEvent)
			{
				bResult = FALSE;
				goto __CLEAN_UP__;;
			}

			if (*m_tzProxy)
			{
				dwAccessType = INTERNET_OPEN_TYPE_PROXY;
				lpszProxy = m_tzProxy;
				if (*m_tzProxyBypass)
				{
					lpszProxyBypass = m_tzProxyBypass;
				}
				else
				{
					lpszProxyBypass = NULL;
				}
			}
			else
			{
				dwAccessType = INTERNET_OPEN_TYPE_PRECONFIG;
				lpszProxy = NULL;
				lpszProxyBypass = NULL;
			}
			if (*m_tzUserAgent)
			{
				lpszUserAgent = m_tzUserAgent;
			}
			else
			{
				lpszUserAgent = _T(IE8_USER_AGENT);
			}
			m_hSession = ::InternetOpen(lpszUserAgent, dwAccessType, lpszProxy, lpszProxyBypass, INTERNET_FLAG_ASYNC);
			if (NULL == m_hSession)
			{
				bResult = FALSE;
				goto __CLEAN_UP__;;
			}

			if (*m_tzUsername)
			{
				lpProxyUsername = m_tzUsername;
				// 设置代理用户名
				::InternetSetOptionEx(m_hSession, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)lpProxyUsername, lstrlen(lpProxyUsername) + sizeof(_TCHAR), 0);
			}
			else
			{
				lpProxyUsername = NULL;
			}

			if (*m_tzPassword)
			{
				lpProxyPassword = m_tzPassword;
				// 设置代理密码
				::InternetSetOptionEx(m_hSession, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)lpProxyPassword, lstrlen(lpProxyPassword) + sizeof(_TCHAR), 0);
			}
			else
			{
				lpProxyPassword = NULL;
			}

			lpStatusCallBackFunc = ::InternetSetStatusCallback(m_hSession, (INTERNET_STATUS_CALLBACK)&StatusCallback);
			if (INTERNET_INVALID_STATUS_CALLBACK == lpStatusCallBackFunc)
			{
				bResult = FALSE;
				goto __CLEAN_UP__;
			}

			m_dwContext = CONNECTED_EVENT;

			m_hConnect = ::InternetConnect(m_hSession, szHostName, nPort, lpProxyUsername, lpProxyPassword, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)this);
			if (NULL == m_hConnect)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					bResult = FALSE;
					goto __CLEAN_UP__;
				}
			}

			bResult = WaitForEvent(CONNECTED_EVENT, m_dwConnectTimeOut);
			if (!bResult)
			{
				bResult = FALSE;
				goto __CLEAN_UP__;
			}

			switch (nReqMethod)
			{
			case REQ_METHOD_GET:
			{
				lpMethod = _T("GET");
			}
			break;
			case REQ_METHOD_POST:
			{
				lpMethod = _T("POST");
			}
			break;
			default:
			{
				lpMethod = NULL;
			}
			break;
			}

			if (INTERNET_DEFAULT_HTTPS_PORT == nPort)
			{
				dwFlags |= INTERNET_FLAG_SECURE;
			}

			m_dwContext = REQUEST_OPENED_EVENT;

			switch (nReqVersion)
			{
			case REQ_VERSION_HTTP_10:
			{
				lpVersion = _T("HTTP/1.0");
			}
			break;
			case REQ_VERSION_HTTP_11:
			{
				lpVersion = _T("HTTP/1.1");
			}
			break;
			case REQ_VERSION_HTTP_20:
			{
				lpVersion = _T("HTTP/2.0");
			}
			break;
			default:
			{
				lpVersion = NULL;
			}
			break;
			}
			m_hRequest = ::HttpOpenRequest(m_hConnect, lpMethod, szUrlPath, lpVersion, lpReferrer, lppAcceptTypes, dwFlags, (DWORD_PTR)this);
			if (NULL == m_hRequest)
			{
				if (::GetLastError() != ERROR_IO_PENDING)
				{
					bResult = FALSE;
					goto __CLEAN_UP__;
				}
			}

			bResult = WaitForEvent(REQUEST_OPENED_EVENT, INFINITE);
			if (!bResult)
			{
				bResult = FALSE;
				goto __CLEAN_UP__;
			}

		__CLEAN_UP__:

			if (!bResult)
			{
				CleanupRequest();
			}

			return bResult;
		}

		// 添加一个或多个HTTP请求头函数
		BOOL CHttpClient::AddHeadersData(LPCTSTR lpHeadersData, DWORD dwHeaderLength, DWORD dwModifiers/* = HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD*/)
		{
			return (NULL != m_hRequest && ::HttpAddRequestHeaders(m_hRequest, lpHeadersData, dwHeaderLength, dwModifiers));
		}

		// 发送HTTP请求函数
		BOOL CHttpClient::ExecuteRequest(LPCTSTR lpHeadersData/* = NULL*/, DWORD dwHeadersDataSize/* = 0*/, LPVOID lpData/* = NULL*/, DWORD dwDataSize/* = 0*/)
		{
			return  (NULL != m_hRequest && (::HttpSendRequest(m_hRequest, lpHeadersData, dwHeadersDataSize, (LPVOID)lpData, dwDataSize) || ::GetLastError() == ERROR_IO_PENDING) && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));
		}

		// 开始发送HTTP请求函数
		BOOL CHttpClient::ExecuteRequest(LPINTERNET_BUFFERS lpInternetBuffers, LPINTERNET_BUFFERS lpOutInternetBuffers/* = NULL*/, DWORD dwFlags/* = 0*/)
		{
			return (NULL != m_hRequest && (::HttpSendRequestEx(m_hRequest, lpInternetBuffers, lpOutInternetBuffers, dwFlags, (DWORD_PTR)this) || ::GetLastError() == ERROR_IO_PENDING) && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));
		}

		// 发送HTTP请求消息体数据函数
		BOOL CHttpClient::SendReqBodyData(LPCVOID lpSendData, DWORD dwSendDataSize, PDWORD pdwNumberOfBytesWritten/* = NULL*/)
		{
			return (NULL != m_hRequest && ::InternetWriteFile(m_hRequest, lpSendData, dwSendDataSize, pdwNumberOfBytesWritten)) ? WaitForEvent(USER_CANCEL_EVENT, (0L)) : (::GetLastError() == ERROR_IO_PENDING && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));
		}

		// 结束发送HTTP请求函数
		BOOL CHttpClient::CleanupRequest()
		{
			BOOL bResult = (NULL != m_hRequest && (::HttpEndRequest(m_hRequest, NULL, HSR_INITIATE, (DWORD_PTR)this) || ::GetLastError() == ERROR_IO_PENDING) && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));

			if (m_hCompleteEvent != NULL)
			{
				::CloseHandle(m_hCompleteEvent);				
			}

			if (m_hCancelEvent != NULL)
			{
				::CloseHandle(m_hCancelEvent);				
			}

			if (m_hRequest)
			{
				::InternetCloseHandle(m_hRequest);				
			}

			if (m_hConnect)
			{
				::InternetCloseHandle(m_hConnect);				
			}

			if (m_hSession)
			{
				::InternetCloseHandle(m_hSession);				
			}

			m_hCompleteEvent = NULL;
			m_hCancelEvent = NULL;
			m_hRequest = NULL;
			m_hConnect = NULL;
			m_hSession = NULL;

			return bResult;
		}

		// 获取HTTP信息数据长度
		BOOL CHttpClient::QueryHttpInfoDataSize(DWORD & dwHttpDataInfoSize, DWORD dwInfoLevel/* = HTTP_QUERY_RAW_HEADERS_CRLF*/, PDWORD pdwIndex/* = NULL*/)
		{
			return (!::HttpQueryInfo(m_hRequest, dwInfoLevel, (LPVOID)NULL, &dwHttpDataInfoSize, pdwIndex) && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER);
		}
		// 获取HTTP信息数据
		BOOL CHttpClient::QueryHttpInfoData(LPVOID pHttpInfoData, DWORD dwHttpInfoDataSize, DWORD dwInfoLevel/* = HTTP_QUERY_RAW_HEADERS_CRLF*/, PDWORD pdwIndex/* = NULL*/)
		{
			return (::HttpQueryInfo(m_hRequest, dwInfoLevel, (LPVOID)pHttpInfoData, &dwHttpInfoDataSize, pdwIndex));
		}

		// 获取HTTP响应码函数
		BOOL CHttpClient::GetRespStatusCode(DWORD & dwRespStatusCode)
		{
			return QueryHttpInfoData(&dwRespStatusCode, sizeof(dwRespStatusCode), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER);
		}

		// 获取全部HTTP头部长度
		BOOL CHttpClient::GetRespHeadersDataSize(DWORD & dwRespHeaderDataSize, DWORD dwInfoLevel/* = HTTP_QUERY_RAW_HEADERS_CRLF*/, PDWORD pdwIndex/* = NULL*/)
		{
			return QueryHttpInfoDataSize(dwRespHeaderDataSize, dwInfoLevel, pdwIndex);
		}
		// 获取全部HTTP头部数据
		BOOL CHttpClient::GetRespHeadersData(_TCHAR * pRespHeaderData, DWORD dwRespHeaderDataSize, DWORD dwInfoLevel/* = HTTP_QUERY_RAW_HEADERS_CRLF*/, PDWORD pdwIndex/* = NULL*/)
		{
			return QueryHttpInfoData((LPVOID)pRespHeaderData, dwRespHeaderDataSize, dwInfoLevel, pdwIndex);
		}

		// 获取HTTP响应数据
		BOOL CHttpClient::GetRespDataSize(DWORD & dwRespDataSize, DWORD dwInfoLevel/* = HTTP_QUERY_CONTENT_LENGTH*/, PDWORD pdwIndex/* = NULL*/)
		{
#ifndef _CONTENT_LENGTH_T
#define _CONTENT_LENGTH_T _T("Content-Length: ")
			BOOL bResult = FALSE;
			DWORD dwContentLength = 0;
			
			_TCHAR * pT = NULL, *pS = NULL;
			_TCHAR tzHeadersData[USHRT_MAX] = { 0 };

			bResult = QueryHttpInfoDataSize(dwContentLength, dwInfoLevel, pdwIndex);

			bResult = GetRespHeadersData(tzHeadersData, _countof(tzHeadersData));
			if (bResult)
			{
				pT = _tcsstr(tzHeadersData, _CONTENT_LENGTH_T);
				if (pT)
				{
					pT += _countof(_CONTENT_LENGTH_T) - 1;
					pS = _tcsstr(pT, _T("\r\n"));
					if (pS)
					{
						*pS = _T('\0');
						dwRespDataSize = (lstrlen(pT) > 0 && dwContentLength > _ttol(pT)) ? dwContentLength : _ttol(pT);
					}
				}
			}
			return bResult;
#undef _CONTENT_LENGTH_T
#endif //_CONTENT_LENGTH_T
		}
		// 获取HTTP响应数据
		BOOL CHttpClient::GetRespData(CHAR * pRespData, DWORD dwRespDataSize, PDWORD pdwRespDataSize/* = NULL*/)
		{
			DWORD dwNumberOfBytesRead = 0;
			PDWORD pdwNumberOfBytesRead = pdwRespDataSize ? pdwRespDataSize : &dwNumberOfBytesRead;
			return RecvRespBodyData((LPVOID)pRespData, dwRespDataSize, pdwNumberOfBytesRead);
		}
		// 获取HTTP响应数据
		BOOL CHttpClient::GetRespData(BYTE ** ppRespData, DWORD & dwRespDataSize, DWORD dwUnitSize/* = USHRT_MAX*/)
		{
			BOOL bResult = FALSE;
			BYTE * pData = NULL;
			DWORD dwRecvIndex = 0;
			DWORD dwNumberOfBytesRead = 0;
			BYTE bData[USHRT_MAX] = { 0 };
			if (ppRespData && dwUnitSize > 0)
			{
				dwRespDataSize = 0;
				while (bResult = RecvRespBodyData((LPVOID)bData, dwUnitSize, &dwNumberOfBytesRead))
				{
					if (dwNumberOfBytesRead > 0)
					{
						dwRespDataSize += dwNumberOfBytesRead;
						if (!(*ppRespData))
						{
							(*ppRespData) = (BYTE*)malloc(sizeof(BYTE));
						}
						pData = (BYTE *)realloc((*ppRespData), dwRespDataSize * sizeof(BYTE));
						if (!pData)
						{
							if ((*ppRespData))
							{
								free((*ppRespData));
								(*ppRespData) = NULL;
							}
							break;
						}
						else
						{
							memcpy(pData + dwRecvIndex, bData, dwNumberOfBytesRead);
							dwRecvIndex = dwRespDataSize;
							(*ppRespData) = pData;
						}
					}
					else
					{
						bResult = TRUE;
						break;
					}
				}
				if (!bResult && (*ppRespData))
				{
					free((*ppRespData));
					(*ppRespData) = NULL;
				}
			}

			return bResult;
		}

		// 获取HTTP响应消息体数据函数
		BOOL CHttpClient::RecvRespBodyData(LPVOID lpData, DWORD dwDataSize, PDWORD pdwNumberOfBytesRead/* = NULL*/)
		{
			return (NULL != m_hRequest && ::InternetReadFile(m_hRequest, lpData, dwDataSize, pdwNumberOfBytesRead)) ? WaitForEvent(USER_CANCEL_EVENT, (0L)) : (::GetLastError() == ERROR_IO_PENDING && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));
		}

		// 获取HTTP响应消息体数据函数
		BOOL CHttpClient::RecvRespBodyData(LPINTERNET_BUFFERS lpInternetBuffers, DWORD dwFlags/* = IRF_ASYNC*/)
		{
			return (NULL != m_hRequest && ::InternetReadFileEx(m_hRequest, lpInternetBuffers, dwFlags, (DWORD_PTR)this)) ? WaitForEvent(USER_CANCEL_EVENT, (0L)) : (::GetLastError() == ERROR_IO_PENDING && WaitForEvent(REQUEST_COMPLETE_EVENT, INFINITE));
		}

		// 设置取消事件函数
		void CHttpClient::SetCancelEvent()
		{
			if (m_hCancelEvent != NULL)
			{
				::SetEvent(m_hCancelEvent);
			}
		}

		// 设置连接超时(单位：毫秒)
		void CHttpClient::SetConnectTimeOut(DWORD dwTimeOut/* = 20 * 3600*/)
		{
			m_dwConnectTimeOut = dwTimeOut;
		}

		// 设置HTTP代理服务器
		void CHttpClient::SetProxy(LPCTSTR lpProxy/* = _T("")*/, LPCTSTR lpszUsername/* = _T("")*/, LPCTSTR lpszPassword/* = _T("")*/)
		{
			lstrcpy(m_tzProxy, lpProxy);
			lstrcpy(m_tzUsername, lpszUsername);
			lstrcpy(m_tzPassword, lpszPassword);
		}

		// 设置HTTP代理服务器例外
		void CHttpClient::SetProxyBypass(LPCTSTR lpszProxyBypass/* = _T("")*/)
		{
			lstrcpy(m_tzProxyBypass, lpszProxyBypass);
		}

		// 设置用户代理
		void CHttpClient::SetUserAgent(LPCTSTR lpszUserAgent/* = IE8_USER_AGENT*/)
		{
			lstrcpy(m_tzUserAgent, lpszUserAgent);
		}

		// 状态回调函数
		void CHttpClient::StatusCallback(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus, LPVOID lpStatusInfo, DWORD dwStatusInfoLen)
		{
			CHttpClient * lpThis = (CHttpClient *)dwContext;
			if (NULL != lpThis)
			{
				switch (dwInternetStatus)
				{
				case INTERNET_STATUS_HANDLE_CREATED:
				{

					if (CONNECTED_EVENT == lpThis->m_dwContext)
					{

						INTERNET_ASYNC_RESULT * lpRes = (INTERNET_ASYNC_RESULT *)lpStatusInfo;

						lpThis->m_hConnect = (HINTERNET)lpRes->dwResult;

						::SetEvent(lpThis->m_hCompleteEvent);

					}
					else if (REQUEST_OPENED_EVENT == lpThis->m_dwContext)
					{
						INTERNET_ASYNC_RESULT * lpRes = (INTERNET_ASYNC_RESULT *)lpStatusInfo;
						lpThis->m_hRequest = (HINTERNET)lpRes->dwResult;
						::SetEvent(lpThis->m_hCompleteEvent);
					}
				}
				break;

				case INTERNET_STATUS_REQUEST_SENT:
				{
					DWORD * lpBytesSent = (DWORD *)lpStatusInfo;
				}
				break;

				case INTERNET_STATUS_REQUEST_COMPLETE:
				{

					INTERNET_ASYNC_RESULT * lpRes = (INTERNET_ASYNC_RESULT *)lpStatusInfo;
					::SetEvent(lpThis->m_hCompleteEvent);
				}
				break;

				case INTERNET_STATUS_RECEIVING_RESPONSE:
				{

				}
				break;

				case INTERNET_STATUS_RESPONSE_RECEIVED:
				{
					DWORD * dwBytesReceived = (DWORD *)lpStatusInfo;
				}
				break;
				}
			}
		}

		// 解析Url函数(协议，主机名，端口，文件路径)
		BOOL CHttpClient::ExplainURLParse(LPCTSTR lpszUrl, LPTSTR lpszScheme, DWORD dwSchemeLength,
			LPTSTR lpszHostName, DWORD dwHostNameLength, WORD& nPort,
			LPTSTR lpszUrlPath, DWORD dwUrlPathLength)
		{
			URL_COMPONENTS stUrlComponents = { 0 };
			stUrlComponents.dwStructSize = sizeof(URL_COMPONENTS);
			stUrlComponents.lpszScheme = lpszScheme;
			stUrlComponents.dwSchemeLength = dwSchemeLength;
			stUrlComponents.lpszHostName = lpszHostName;
			stUrlComponents.dwHostNameLength = dwHostNameLength;
			stUrlComponents.lpszUrlPath = lpszUrlPath;
			stUrlComponents.dwUrlPathLength = dwUrlPathLength;

			BOOL bRet = ::InternetCrackUrl(lpszUrl, 0, 0, &stUrlComponents);
			if (bRet)
			{
				nPort = stUrlComponents.nPort;
			}

			return bRet;
		}


		// 等待事件函数
		BOOL CHttpClient::WaitForEvent(HTTP_STATUS_EVENT nEvent, DWORD dwTimeOut)
		{
			BOOL bResult = FALSE;
			HANDLE hEventArray[] = { 0, 0 };
			switch (nEvent)
			{
			case CONNECTED_EVENT:
			case REQUEST_OPENED_EVENT:
			case REQUEST_COMPLETE_EVENT:
			{
				*hEventArray = m_hCancelEvent;
				*(hEventArray + 1) = m_hCompleteEvent;
				bResult = (::WaitForMultipleObjects(_countof(hEventArray), hEventArray, FALSE, dwTimeOut) != (WAIT_OBJECT_0 + 1)) ? FALSE : TRUE;
			}
			break;
			case USER_CANCEL_EVENT:
			{
				*hEventArray = m_hCancelEvent;
				bResult = (WAIT_OBJECT_0 == ::WaitForSingleObject(*hEventArray, dwTimeOut)) ? FALSE : TRUE;
			}
			break;
			default:
			{
				bResult = FALSE;
			}
			break;
			}

			return bResult;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//

		/*++
		Copyright (C) Microsoft.  All Rights Reserved.

		--*/

		//#include "async.h"

#pragma warning(disable:4127) // Conditional expression is constant


		int __cdecl
			t_tmain(
			__in int argc,
			__in_ecount(argc) LPTSTR *argv
			)
		{
			DWORD Error;
			DWORD OpenType = INTERNET_OPEN_TYPE_PRECONFIG; // Use pre-configured options as default

			CONFIGURATION Configuration = { 0 };

			PREQUEST_CONTEXT ReqContext = NULL;
			HINTERNET SessionHandle = NULL;

			// Callback function
			INTERNET_STATUS_CALLBACK CallbackPointer;


			// Parse the command line arguments
			Error = ParseArguments(argc, argv, &Configuration);
			if (Error != ERROR_SUCCESS)
			{
				ShowUsage();
				goto Exit;
			}

			if (Configuration.UseProxy)
			{
				OpenType = INTERNET_OPEN_TYPE_PROXY;
			}

			// Create Session handle and specify async Mode
			SessionHandle = InternetOpen(_T("AsyncHttpClient"),  // User Agent
				OpenType,                      // Preconfig or Proxy
				Configuration.ProxyName,       // Proxy name
				Configuration.ProxyBypass,     // Proxy bypass, do not bypass any address
				INTERNET_FLAG_ASYNC);          // 0 for Synchronous

			if (SessionHandle == NULL)
			{
				LogInetError(GetLastError(), _T("InternetOpen"));
				goto Exit;
			}


			// Set the status callback for the handle to the Callback function
			CallbackPointer = InternetSetStatusCallback(SessionHandle,
				(INTERNET_STATUS_CALLBACK)CallBack);

			if (CallbackPointer == INTERNET_INVALID_STATUS_CALLBACK)
			{
				fprintf(stderr, "InternetSetStatusCallback failed with INTERNET_INVALID_STATUS_CALLBACK\n");
				goto Exit;
			}

			// Initialize the ReqContext to be used in the asynchronous calls
			Error = AllocateAndInitializeRequestContext(SessionHandle,
				&Configuration,
				&ReqContext);
			if (Error != ERROR_SUCCESS)
			{
				fprintf(stderr, "AllocateAndInitializeRequestContext failed with error %d\n", Error);
				goto Exit;
			}

			//
			// Send out request and receive response
			//

			ProcessRequest(ReqContext, ERROR_SUCCESS);


			//
			// Wait for request completion or timeout
			//

			WaitForRequestCompletion(ReqContext, Configuration.UserTimeout);


		Exit:
			{
				ReqContext->pResponseHeaderData = NULL;
				ReqContext->dwResponseHeaderDataSize = 0;
				if (!::HttpQueryInfo(ReqContext->RequestHandle, HTTP_QUERY_RAW_HEADERS_CRLF,
					(LPVOID)ReqContext->pResponseHeaderData, &ReqContext->dwResponseHeaderDataSize, NULL) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
				{
					ReqContext->pResponseHeaderData = (_TCHAR *)malloc(ReqContext->dwResponseHeaderDataSize * sizeof(_TCHAR));
					memset(ReqContext->pResponseHeaderData, 0, ReqContext->dwResponseHeaderDataSize * sizeof(_TCHAR));
					::HttpQueryInfo(ReqContext->RequestHandle, HTTP_QUERY_RAW_HEADERS_CRLF,
						(LPVOID)ReqContext->pResponseHeaderData, &ReqContext->dwResponseHeaderDataSize, NULL);

					/*if (GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND)
					{
					// Code to handle the case where the header isn't available.
					return TRUE;
					}
					else
					{
					// Check for an insufficient buffer.
					if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					}*/
				}
			}
			FILE * pFile = fopen("d:\\a.zip", "wb");
			if (pFile)
			{
				fwrite(ReqContext->pResponseData, ReqContext->dwResponseDataSize, 1, pFile);
				fclose(pFile);
			}
			// Clean up the allocated resources
			CleanUpRequestContext(ReqContext);

			CleanUpSessionHandle(SessionHandle);

			return (Error != ERROR_SUCCESS) ? 1 : 0;
		}

		VOID CALLBACK
			CallBack(
			__in HINTERNET hInternet,
			__in DWORD_PTR dwContext,
			__in DWORD dwInternetStatus,
			__in_bcount(dwStatusInformationLength) LPVOID lpvStatusInformation,
			__in DWORD dwStatusInformationLength
			)
			/*++

			Routine Description:
			Callback routine for asynchronous WinInet operations

			Arguments:
			hInternet - The handle for which the callback function is called.
			dwContext - Pointer to the application defined context.
			dwInternetStatus - Status code indicating why the callback is called.
			lpvStatusInformation - Pointer to a buffer holding callback specific data.
			dwStatusInformationLength - Specifies size of lpvStatusInformation buffer.

			Return Value:
			None.

			--*/
		{
			InternetCookieHistory cookieHistory;
			PREQUEST_CONTEXT ReqContext = (PREQUEST_CONTEXT)dwContext;

			UNREFERENCED_PARAMETER(dwStatusInformationLength);

			fprintf(stderr, "Callback Received for Handle %p \t", hInternet);

			switch (dwInternetStatus)
			{
			case INTERNET_STATUS_COOKIE_SENT:
				fprintf(stderr, "Status: Cookie found and will be sent with request\n");
				break;

			case INTERNET_STATUS_COOKIE_RECEIVED:
				fprintf(stderr, "Status: Cookie Received\n");
				break;

			case INTERNET_STATUS_COOKIE_HISTORY:

				fprintf(stderr, "Status: Cookie History\n");

				ASYNC_ASSERT(lpvStatusInformation);
				ASYNC_ASSERT(dwStatusInformationLength == sizeof(InternetCookieHistory));

				cookieHistory = *((InternetCookieHistory*)lpvStatusInformation);

				if (cookieHistory.fAccepted)
				{
					fprintf(stderr, "Cookie Accepted\n");
				}
				if (cookieHistory.fLeashed)
				{
					fprintf(stderr, "Cookie Leashed\n");
				}
				if (cookieHistory.fDowngraded)
				{
					fprintf(stderr, "Cookie Downgraded\n");
				}
				if (cookieHistory.fRejected)
				{
					fprintf(stderr, "Cookie Rejected\n");
				}

				break;

			case INTERNET_STATUS_CLOSING_CONNECTION:
				fprintf(stderr, "Status: Closing Connection\n");
				break;

			case INTERNET_STATUS_CONNECTED_TO_SERVER:
				fprintf(stderr, "Status: Connected to Server\n");
				break;

			case INTERNET_STATUS_CONNECTING_TO_SERVER:
				fprintf(stderr, "Status: Connecting to Server\n");
				break;

			case INTERNET_STATUS_CONNECTION_CLOSED:
				fprintf(stderr, "Status: Connection Closed\n");
				break;

			case INTERNET_STATUS_HANDLE_CLOSING:
				fprintf(stderr, "Status: Handle Closing\n");

				//
				// Signal the cleanup routine that it is
				// safe to cleanup the request context
				//

				ASYNC_ASSERT(ReqContext);
				SetEvent(ReqContext->CleanUpEvent);

				break;

			case INTERNET_STATUS_HANDLE_CREATED:
				ASYNC_ASSERT(lpvStatusInformation);
				fprintf(stderr,
					"Handle %x created\n",
					((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwResult);

				break;

			case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
				fprintf(stderr, "Status: Intermediate response\n");
				break;

			case INTERNET_STATUS_RECEIVING_RESPONSE:
				fprintf(stderr, "Status: Receiving Response\n");
				break;

			case INTERNET_STATUS_RESPONSE_RECEIVED:
				ASYNC_ASSERT(lpvStatusInformation);
				ASYNC_ASSERT(dwStatusInformationLength == sizeof(DWORD));

				fprintf(stderr, "Status: Response Received (%d Bytes)\n", *((LPDWORD)lpvStatusInformation));

				break;

			case INTERNET_STATUS_REDIRECT:
				fprintf(stderr, "Status: Redirect\n");
				break;

			case INTERNET_STATUS_REQUEST_COMPLETE:
				fprintf(stderr, "Status: Request complete\n");

				ASYNC_ASSERT(lpvStatusInformation);

				ProcessRequest(ReqContext, ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwError);

				break;

			case INTERNET_STATUS_REQUEST_SENT:
				ASYNC_ASSERT(lpvStatusInformation);
				ASYNC_ASSERT(dwStatusInformationLength == sizeof(DWORD));

				fprintf(stderr, "Status: Request sent (%d Bytes)\n", *((LPDWORD)lpvStatusInformation));
				break;

			case INTERNET_STATUS_DETECTING_PROXY:
				fprintf(stderr, "Status: Detecting Proxy\n");
				break;

			case INTERNET_STATUS_RESOLVING_NAME:
				fprintf(stderr, "Status: Resolving Name\n");
				break;

			case INTERNET_STATUS_NAME_RESOLVED:
				fprintf(stderr, "Status: Name Resolved\n");
				break;

			case INTERNET_STATUS_SENDING_REQUEST:
				fprintf(stderr, "Status: Sending request\n");
				break;

			case INTERNET_STATUS_STATE_CHANGE:
				fprintf(stderr, "Status: State Change\n");
				break;

			case INTERNET_STATUS_P3P_HEADER:
				fprintf(stderr, "Status: Received P3P header\n");
				break;

			default:
				fprintf(stderr, "Status: Unknown (%d)\n", dwInternetStatus);
				break;
			}

			return;
		}


		VOID
			ProcessRequest(
			__inout PREQUEST_CONTEXT ReqContext,
			__in DWORD Error
			)
			/*++

			Routine Description:
			Process the request context - Sending the request and
			receiving the response

			Arguments:
			ReqContext - Pointer to request context structure
			Error - error returned from last asynchronous call

			Return Value:
			None.

			--*/
		{
			BOOL Eof = FALSE;

			ASYNC_ASSERT(ReqContext);


			while (Error == ERROR_SUCCESS && ReqContext->State != REQ_STATE_COMPLETE)
			{

				switch (ReqContext->State)
				{
				case REQ_STATE_SEND_REQ:

					ReqContext->State = REQ_STATE_RESPONSE_RECV_DATA;
					Error = SendRequest(ReqContext);

					break;

				case REQ_STATE_SEND_REQ_WITH_BODY:

					ReqContext->State = REQ_STATE_POST_GET_DATA;
					Error = SendRequestWithBody(ReqContext);

					break;

				case REQ_STATE_POST_GET_DATA:

					ReqContext->State = REQ_STATE_POST_SEND_DATA;
					Error = GetDataToPost(ReqContext);

					break;

				case REQ_STATE_POST_SEND_DATA:

					ReqContext->State = REQ_STATE_POST_GET_DATA;
					Error = PostDataToServer(ReqContext, &Eof);

					if (Eof)
					{
						ASYNC_ASSERT(Error == ERROR_SUCCESS);
						ReqContext->State = REQ_STATE_POST_COMPLETE;
					}

					break;

				case REQ_STATE_POST_COMPLETE:

					ReqContext->State = REQ_STATE_RESPONSE_RECV_DATA;
					Error = CompleteRequest(ReqContext);

					break;

				case REQ_STATE_RESPONSE_RECV_DATA:

					ReqContext->State = REQ_STATE_RESPONSE_WRITE_DATA;
					Error = RecvResponseData(ReqContext);

					break;

				case REQ_STATE_RESPONSE_WRITE_DATA:

					ReqContext->State = REQ_STATE_RESPONSE_RECV_DATA;

					Error = WriteResponseData(ReqContext, &Eof);

					if (Eof)
					{
						ASYNC_ASSERT(Error == ERROR_SUCCESS);
						ReqContext->State = REQ_STATE_COMPLETE;
					}

					break;

				default:

					ASYNC_ASSERT(FALSE);

					break;
				}
			}

			if (Error != ERROR_IO_PENDING)
			{
				//
				// Everything has been procesed or has failed. 
				// In either case, the signal processing has
				// completed
				//

				SetEvent(ReqContext->CompletionEvent);
			}

			return;
		}

		DWORD
			SendRequest(
			__in PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Send the request using HttpSendRequest

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			Error code for the operation.

			--*/
		{
			BOOL Success;
			DWORD Error = ERROR_SUCCESS;


			ASYNC_ASSERT(ReqContext->Method == METHOD_GET);

			Success = AcquireRequestHandle(ReqContext);
			if (!Success)
			{
				Error = ERROR_OPERATION_ABORTED;
				goto Exit;
			}

			Success = HttpSendRequest(ReqContext->RequestHandle,
				ReqContext->pRequestHeaderData,// do not provide additional Headers
				ReqContext->dwRequestHeaderDataSize,// dwHeadersLength 
				ReqContext->pRequestData,// Do not send any data 
				ReqContext->dwRequestDataSize);// dwOptionalLength 

			ReleaseRequestHandle(ReqContext);

			if (!Success)
			{
				Error = GetLastError();

				if (Error != ERROR_IO_PENDING)
				{
					LogInetError(Error, _T("HttpSendRequest"));
				}
				goto Exit;
			}

		Exit:

			return Error;
		}


		DWORD
			SendRequestWithBody(
			__in PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Send the request with entity-body using HttpSendRequestEx

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			Error code for the operation.

			--*/
		{
			BOOL Success;
			INTERNET_BUFFERS BuffersIn;
			DWORD Error = ERROR_SUCCESS;

			//
			// HttpSendRequest can also be used also to post data to a server, 
			// to do so, the data should be provided using the lpOptional
			// parameter and it's size on dwOptionalLength.
			// Here we decided to depict the use of HttpSendRequestEx function.
			//


			ASYNC_ASSERT(ReqContext->Method == METHOD_POST);

			//Prepare the Buffers to be passed to HttpSendRequestEx
			ZeroMemory(&BuffersIn, sizeof(INTERNET_BUFFERS));
			BuffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
			BuffersIn.lpvBuffer = NULL;
			BuffersIn.dwBufferLength = 0;
			BuffersIn.dwBufferTotal = ReqContext->FileSize; // content-length of data to post


			Success = AcquireRequestHandle(ReqContext);
			if (!Success)
			{
				Error = ERROR_OPERATION_ABORTED;
				goto Exit;
			}

			Success = HttpSendRequestEx(ReqContext->RequestHandle,
				&BuffersIn,
				NULL,                 // Do not use output buffers
				0,                    // dwFlags reserved
				(DWORD_PTR)ReqContext);

			ReleaseRequestHandle(ReqContext);

			if (!Success)
			{
				Error = GetLastError();

				if (Error != ERROR_IO_PENDING)
				{
					LogInetError(Error, _T("HttpSendRequestEx"));
				}

				goto Exit;
			}

		Exit:

			return Error;
		}

		DWORD
			GetDataToPost(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Reads data from a file

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			Error code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			BOOL Success;


			//
			//
			// ReadFile is done inline here assuming that it will return quickly
			// I.E. the file is on disk
			//
			// If you plan to do blocking/intensive operations they should be
			// queued to another thread and not block the callback thread
			//
			//

			Success = ReadFile(ReqContext->UploadFile,
				ReqContext->OutputBuffer,
				BUFFER_LEN,
				&ReqContext->ReadBytes,
				NULL);
			if (!Success)
			{
				Error = GetLastError();
				LogSysError(Error, _T("ReadFile"));
				goto Exit;
			}


		Exit:

			return Error;
		}

		DWORD
			PostDataToServer(
			__inout PREQUEST_CONTEXT ReqContext,
			__out PBOOL Eof
			)
			/*++

			Routine Description:
			Post data in the http request

			Arguments:
			ReqContext - Pointer to request context structure
			Eof - Done posting data to server

			Return Value:
			Error code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			BOOL Success;

			*Eof = FALSE;

			if (ReqContext->ReadBytes == 0)
			{
				*Eof = TRUE;
				goto Exit;
			}


			Success = AcquireRequestHandle(ReqContext);
			if (!Success)
			{
				Error = ERROR_OPERATION_ABORTED;
				goto Exit;
			}


			//
			// The lpdwNumberOfBytesWritten parameter will be
			// populated on async completion, so it must exist
			// until INTERNET_STATUS_REQUEST_COMPLETE.
			// The same is true of lpBuffer parameter.
			//

			Success = InternetWriteFile(ReqContext->RequestHandle,
				ReqContext->OutputBuffer,
				ReqContext->ReadBytes,
				&ReqContext->WrittenBytes);


			ReleaseRequestHandle(ReqContext);

			if (!Success)
			{
				Error = GetLastError();

				if (Error == ERROR_IO_PENDING)
				{
					fprintf(stderr, "Waiting for InternetWriteFile to complete\n");
				}
				else
				{
					LogInetError(Error, _T("InternetWriteFile"));
				}

				goto Exit;

			}



		Exit:


			return Error;
		}


		DWORD
			CompleteRequest(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Perform completion of asynchronous post.

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			Error Code for the operation.

			--*/
		{

			DWORD Error = ERROR_SUCCESS;
			BOOL Success;

			fprintf(stderr, "Finished posting file\n");

			Success = AcquireRequestHandle(ReqContext);
			if (!Success)
			{
				Error = ERROR_OPERATION_ABORTED;
				goto Exit;
			}

			Success = HttpEndRequest(ReqContext->RequestHandle, NULL, 0, 0);

			ReleaseRequestHandle(ReqContext);

			if (!Success)
			{
				Error = GetLastError();
				if (Error == ERROR_IO_PENDING)
				{
					fprintf(stderr, "Waiting for HttpEndRequest to complete \n");
				}
				else
				{
					LogInetError(Error, _T("HttpEndRequest"));
					goto Exit;
				}
			}

		Exit:

			return Error;
		}



		DWORD
			RecvResponseData(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Receive response

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			Error Code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			BOOL Success;

			Success = AcquireRequestHandle(ReqContext);
			if (!Success)
			{
				Error = ERROR_OPERATION_ABORTED;
				goto Exit;
			}

			//
			// The lpdwNumberOfBytesRead parameter will be
			// populated on async completion, so it must exist
			// until INTERNET_STATUS_REQUEST_COMPLETE.
			// The same is true of lpBuffer parameter.
			//
			// InternetReadFile will block until the buffer
			// is completely filled or the response is exhausted.
			//


			Success = InternetReadFile(ReqContext->RequestHandle,
				ReqContext->OutputBuffer,
				BUFFER_LEN,
				&ReqContext->DownloadedBytes);

			ReleaseRequestHandle(ReqContext);

			if (!Success)
			{
				Error = GetLastError();
				if (Error == ERROR_IO_PENDING)
				{
					fprintf(stderr, "Waiting for InternetReadFile to complete\n");
				}
				else
				{
					LogInetError(Error, _T("InternetReadFile"));
				}

				goto Exit;
			}


		Exit:

			return Error;
		}


		DWORD
			WriteResponseData(
			__in PREQUEST_CONTEXT ReqContext,
			__out PBOOL Eof
			)
			/*++

			Routine Description:
			Write response to a file

			Arguments:
			ReqContext - Pointer to request context structure
			Eof - Done with response

			Return Value:
			Error Code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			DWORD BytesWritten = 0;

			BOOL Success;

			*Eof = FALSE;

			//
			// Finished receiving response
			//

			if (ReqContext->DownloadedBytes == 0)
			{
				*Eof = TRUE;
				goto Exit;

			}

			//
			//
			// WriteFile is done inline here assuming that it will return quickly
			// I.E. the file is on disk
			//
			// If you plan to do blocking/intensive operations they should be
			// queued to another thread and not block the callback thread
			//
			//
			if (ReqContext->DownloadFile != INVALID_HANDLE_VALUE)
			{
				Success = WriteFile(ReqContext->DownloadFile,
					ReqContext->OutputBuffer,
					ReqContext->DownloadedBytes,
					&BytesWritten,
					NULL);

				if (!Success)
				{
					Error = GetLastError();
					LogSysError(Error, _T("WriteFile"));
					goto Exit;;
				}
			}
			if (ReqContext->DownloadedBytes > 0)
			{
				ReqContext->pResponseData = (BYTE *)realloc(ReqContext->pResponseData, ReqContext->dwResponseDataSize + ReqContext->DownloadedBytes);
				if (ReqContext->pResponseData)
				{
					memset(ReqContext->pResponseData + ReqContext->dwResponseDataSize, 0, ReqContext->DownloadedBytes);
					memcpy(ReqContext->pResponseData + ReqContext->dwResponseDataSize, ReqContext->OutputBuffer, ReqContext->DownloadedBytes);
					ReqContext->dwResponseDataSize += ReqContext->DownloadedBytes;
				}
			}

		Exit:

			return Error;
		}





		VOID
			CloseRequestHandle(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Safely  close the request handle by synchronizing
			with all threads using the handle.

			When this function returns no more calls can be made with the
			handle.

			Arguments:
			ReqContext - Pointer to Request context structure
			Return Value:
			None.

			--*/
		{
			BOOL Close = FALSE;

			EnterCriticalSection(&ReqContext->CriticalSection);

			//
			// Current implementation only supports the main thread
			// kicking off the request handle close
			//
			// To support multiple threads the lifetime 
			// of the request context must be carefully controlled
			// (most likely guarded by refcount/critsec)
			// so that they are not trying to abort a request
			// where the context has already been freed.
			//

			ASYNC_ASSERT(ReqContext->Closing == FALSE);
			ReqContext->Closing = TRUE;

			if (ReqContext->HandleUsageCount == 0)
			{
				Close = TRUE;
			}

			LeaveCriticalSection(&ReqContext->CriticalSection);



			if (Close)
			{
				//
				// At this point there must be the guarantee that all calls
				// to wininet with this handle have returned with some value
				// including ERROR_IO_PENDING, and none will be made after
				// InternetCloseHandle.
				//        
				(VOID)InternetCloseHandle(ReqContext->RequestHandle);
			}

			return;
		}

		BOOL
			AcquireRequestHandle(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Acquire use of the request handle to make a wininet call
			Arguments:
			ReqContext - Pointer to Request context structure
			Return Value:
			TRUE - Success
			FALSE - Failure
			--*/
		{
			BOOL Success = TRUE;

			EnterCriticalSection(&ReqContext->CriticalSection);

			if (ReqContext->Closing == TRUE)
			{
				Success = FALSE;
			}
			else
			{
				ReqContext->HandleUsageCount++;
			}

			LeaveCriticalSection(&ReqContext->CriticalSection);

			return Success;
		}


		VOID
			ReleaseRequestHandle(
			__inout PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			release use of the request handle
			Arguments:
			ReqContext - Pointer to Request context structure
			Return Value:
			None.

			--*/
		{
			BOOL Close = FALSE;

			EnterCriticalSection(&ReqContext->CriticalSection);

			ASYNC_ASSERT(ReqContext->HandleUsageCount > 0);

			ReqContext->HandleUsageCount--;

			if (ReqContext->Closing == TRUE && ReqContext->HandleUsageCount == 0)
			{
				Close = TRUE;

			}

			LeaveCriticalSection(&ReqContext->CriticalSection);


			if (Close)
			{
				//
				// At this point there must be the guarantee that all calls
				// to wininet with this handle have returned with some value
				// including ERROR_IO_PENDING, and none will be made after
				// InternetCloseHandle.
				//        
				(VOID)InternetCloseHandle(ReqContext->RequestHandle);
			}

			return;
		}


		DWORD
			AllocateAndInitializeRequestContext(
			__in HINTERNET SessionHandle,
			__in PCONFIGURATION Configuration,
			__deref_out PREQUEST_CONTEXT *ReqContext
			)
			/*++

			Routine Description:
			Allocate the request context and initialize it values

			Arguments:
			ReqContext - Pointer to Request context structure
			Configuration - Pointer to configuration structure
			SessionHandle - Wininet session handle to use when creating
			connect handle

			Return Value:
			Error Code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			BOOL Success;
			PREQUEST_CONTEXT LocalReqContext;

			*ReqContext = NULL;

			LocalReqContext = (PREQUEST_CONTEXT)malloc(sizeof(REQUEST_CONTEXT));

			if (LocalReqContext == NULL)
			{
				Error = ERROR_NOT_ENOUGH_MEMORY;
				goto Exit;
			}

			LocalReqContext->RequestHandle = NULL;
			LocalReqContext->ConnectHandle = NULL;
			LocalReqContext->DownloadedBytes = 0;
			LocalReqContext->WrittenBytes = 0;
			LocalReqContext->ReadBytes = 0;
			LocalReqContext->UploadFile = INVALID_HANDLE_VALUE;
			LocalReqContext->DownloadFile = INVALID_HANDLE_VALUE;
			LocalReqContext->FileSize = 0;
			LocalReqContext->HandleUsageCount = 0;
			LocalReqContext->Closing = FALSE;
			LocalReqContext->Method = Configuration->Method;
			LocalReqContext->CompletionEvent = NULL;
			LocalReqContext->CleanUpEvent = NULL;
			LocalReqContext->OutputBuffer = NULL;

			LocalReqContext->pRequestData = NULL;
			LocalReqContext->dwRequestDataSize = 0;

			LocalReqContext->pRequestHeaderData = NULL;
			LocalReqContext->dwRequestHeaderDataSize = 0;

			LocalReqContext->pResponseData = NULL;
			LocalReqContext->dwResponseDataSize = 0;

			LocalReqContext->pResponseHeaderData = NULL;
			LocalReqContext->dwResponseHeaderDataSize = 0;

			LocalReqContext->State =
				(LocalReqContext->Method == METHOD_TYPE_GET) ? REQ_STATE_SEND_REQ : REQ_STATE_SEND_REQ_WITH_BODY;
			LocalReqContext->CritSecInitialized = FALSE;


			// initialize critical section

			Success = InitializeCriticalSectionAndSpinCount(&LocalReqContext->CriticalSection, SPIN_COUNT);

			if (!Success)
			{
				Error = GetLastError();
				LogSysError(Error, _T("InitializeCriticalSectionAndSpinCount"));
				goto Exit;
			}

			LocalReqContext->CritSecInitialized = TRUE;

			LocalReqContext->OutputBuffer = (LPSTR)malloc(BUFFER_LEN);

			if (LocalReqContext->OutputBuffer == NULL)
			{
				Error = ERROR_NOT_ENOUGH_MEMORY;
				goto Exit;
			}

			// create events
			LocalReqContext->CompletionEvent = CreateEvent(NULL,  // Sec attrib
				FALSE, // Auto reset
				FALSE, // Initial state unsignalled
				NULL); // Name
			if (LocalReqContext->CompletionEvent == NULL)
			{
				Error = GetLastError();
				LogSysError(Error, _T("CreateEvent CompletionEvent"));
				goto Exit;
			}

			// create events
			LocalReqContext->CleanUpEvent = CreateEvent(NULL,  // Sec attrib
				FALSE, // Auto reset
				FALSE, // Initial state unsignalled
				NULL); // Name
			if (LocalReqContext->CleanUpEvent == NULL)
			{
				Error = GetLastError();
				LogSysError(Error, _T("CreateEvent CleanUpEvent"));
				goto Exit;
			}

			// Open the file to dump the response entity body and
			// if required the file with the data to post
			Error = OpenFiles(LocalReqContext,
				Configuration->Method,
				Configuration->InputFileName,
				Configuration->OutputFileName);

			if (Error != ERROR_SUCCESS)
			{
				fprintf(stderr, "OpenFiles failed with %d\n", Error);
				goto Exit;
			}

			// Verify if we've opened a file to post and get its size
			if (LocalReqContext->UploadFile != INVALID_HANDLE_VALUE)
			{
				LocalReqContext->FileSize = GetFileSize(LocalReqContext->UploadFile, NULL);
				if (LocalReqContext->FileSize == INVALID_FILE_SIZE)
				{
					Error = GetLastError();
					LogSysError(Error, _T("GetFileSize"));
					goto Exit;
				}
			}


			Error = CreateWininetHandles(LocalReqContext,
				SessionHandle,
				Configuration->HostName,
				Configuration->ResourceOnServer,
				Configuration->IsSecureConnection);

			if (Error != ERROR_SUCCESS)
			{
				fprintf(stderr, "CreateWininetHandles failed with %d\n", Error);
				goto Exit;
			}


			*ReqContext = LocalReqContext;

		Exit:

			if (Error != ERROR_SUCCESS)
			{
				CleanUpRequestContext(LocalReqContext);
			}

			return Error;
		}


		DWORD
			CreateWininetHandles(
			__inout PREQUEST_CONTEXT ReqContext,
			__in HINTERNET SessionHandle,
			__in LPTSTR HostName,
			__in LPTSTR Resource,
			__in BOOL IsSecureConnection
			)
			/*++

			Routine Description:
			Create connect and request handles

			Arguments:
			ReqContext - Pointer to Request context structure
			SessionHandle - Wininet session handle used to create
			connect handle
			HostName - Hostname to connect
			Resource - Resource to get/post
			IsSecureConnection - SSL?

			Return Value:
			Error Code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;
			INTERNET_PORT ServerPort = INTERNET_DEFAULT_HTTP_PORT;
			DWORD RequestFlags = 0;
			LPTSTR Verb;


			//
			// Set the correct server port if using SSL
			// Also set the flag for HttpOpenRequest 
			//

			if (IsSecureConnection)
			{
				ServerPort = INTERNET_DEFAULT_HTTPS_PORT;
				RequestFlags = INTERNET_FLAG_SECURE;
			}

			// Create Connection handle and provide context for async operations
			ReqContext->ConnectHandle = InternetConnect(SessionHandle,
				HostName,                  // Name of the server to connect to
				ServerPort,                // HTTP (80) or HTTPS (443)
				NULL,                      // Do not provide a user name for the server
				NULL,                      // Do not provide a password for the server
				INTERNET_SERVICE_HTTP,
				0,                         // Do not provide any special flag
				(DWORD_PTR)ReqContext);    // Provide the context to be
			// used during the callbacks
			//                                                                        
			// For HTTP InternetConnect returns synchronously because it does not
			// actually make the connection.
			//
			// For FTP InternetConnect connects the control channel, and therefore
			// can be completed asynchronously.  This sample would have to be
			// changed, so that the InternetConnect's asynchronous completion
			// is handled correctly to support FTP.
			//

			if (ReqContext->ConnectHandle == NULL)
			{
				Error = GetLastError();
				LogInetError(Error, _T("InternetConnect"));
				goto Exit;
			}


			// Set the Verb depending on the operation to perform
			if (ReqContext->Method == METHOD_TYPE_GET)
			{
				Verb = _T("GET");
			}
			else
			{
				ASYNC_ASSERT(ReqContext->Method == METHOD_POST);

				Verb = _T("POST");
			}

			//
			// We're overriding WinInet's default behavior.
			// Setting these flags, we make sure we get the response from the server and not the cache.
			// Also ask WinInet not to store the response in the cache.
			//
			// These flags are NOT performant and are only used to show case WinInet's Async I/O.
			// A real WinInet application would not want to use this flags.
			//

			RequestFlags |= INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;

			// Create a Request handle
			ReqContext->RequestHandle = HttpOpenRequest(ReqContext->ConnectHandle,
				Verb,                     // GET or POST
				Resource,                 // root "/" by default
				NULL,                     // Use default HTTP/1.1 as the version
				NULL,                     // Do not provide any referrer
				NULL,                     // Do not provide Accept types
				RequestFlags,
				(DWORD_PTR)ReqContext);

			if (ReqContext->RequestHandle == NULL)
			{
				Error = GetLastError();
				LogInetError(Error, _T("HttpOpenRequest"));

				goto Exit;
			}


		Exit:

			return Error;
		}

		VOID
			WaitForRequestCompletion(
			__in PREQUEST_CONTEXT ReqContext,
			__in DWORD Timeout
			)
			/*++

			Routine Description:
			Wait for the request to complete or timeout to occur

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			None.

			--*/
		{
			DWORD SyncResult;

			//
			// The preferred method of doing timeouts is to
			// use the timeout options through InternetSetOption,
			// but this overall timeout is being used to show 
			// the correct way to abort and close a request.
			//

			SyncResult = WaitForSingleObject(ReqContext->CompletionEvent,
				Timeout);              // Wait until we receive the completion

			switch (SyncResult)
			{
			case WAIT_OBJECT_0:

				printf("Done!\n");
				break;

			case WAIT_TIMEOUT:

				fprintf(stderr,
					"Timeout while waiting for completion event (request will be cancelled)\n");
				break;

			case WAIT_FAILED:

				fprintf(stderr,
					"Wait failed with Error %d while waiting for completion event (request will be cancelled)\n",
					GetLastError());
				break;

			default:
				// Not expecting any other error codes
				ASYNC_ASSERT(FALSE);
				break;


			}

			return;
		}

		VOID
			CleanUpRequestContext(
			__inout_opt PREQUEST_CONTEXT ReqContext
			)
			/*++

			Routine Description:
			Used to cleanup the request context before exiting.

			Arguments:
			ReqContext - Pointer to request context structure

			Return Value:
			None.

			--*/
		{
			if (ReqContext == NULL)
			{
				goto Exit;
			}

			if (ReqContext->RequestHandle)
			{
				CloseRequestHandle(ReqContext);

				//
				// Wait for the closing of the handle to complete
				// (waiting for all async operations to complete)
				//
				// This is the only safe way to get rid of the context
				//

				(VOID)WaitForSingleObject(ReqContext->CleanUpEvent, INFINITE);
			}

			if (ReqContext->ConnectHandle)
			{
				//
				// Remove the callback from the ConnectHandle since
				// we don't want the closing notification
				// The callback was inherited from the session handle
				//
				(VOID)InternetSetStatusCallback(ReqContext->ConnectHandle,
					NULL);

				(VOID)InternetCloseHandle(ReqContext->ConnectHandle);
			}

			if (ReqContext->UploadFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(ReqContext->UploadFile);
			}

			if (ReqContext->DownloadFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(ReqContext->DownloadFile);
			}

			if (ReqContext->CompletionEvent)
			{
				CloseHandle(ReqContext->CompletionEvent);
			}

			if (ReqContext->CleanUpEvent)
			{
				CloseHandle(ReqContext->CleanUpEvent);
			}

			if (ReqContext->CritSecInitialized)
			{
				DeleteCriticalSection(&ReqContext->CriticalSection);
			}

			if (ReqContext->OutputBuffer)
			{
				free(ReqContext->OutputBuffer);
			}

			if (ReqContext->pResponseData)
			{
				free(ReqContext->pResponseData);
			}

			free(ReqContext);

		Exit:

			return;
		}



		VOID
			CleanUpSessionHandle(
			__in HINTERNET SessionHandle
			)
			/*++

			Routine Description:
			Used to cleanup session before exiting.

			Arguments:
			SessionHandle - Wininet session handle

			Return Value:
			None.

			--*/
		{

			if (SessionHandle)
			{
				//
				// Remove the callback from the SessionHandle since
				// we don't want the closing notification
				//
				(VOID)InternetSetStatusCallback(SessionHandle,
					NULL);
				//
				// Call InternetCloseHandle and do not wait for the closing notification 
				// in the callback function
				//
				(VOID)InternetCloseHandle(SessionHandle);

			}

			return;
		}


		DWORD
			OpenFiles(
			__inout PREQUEST_CONTEXT ReqContext,
			__in DWORD Method,
			__in LPTSTR InputFileName,
			__in LPTSTR OutputFileName
			)
			/*++

			Routine Description:
			This routine opens files, one to post data from, and
			one to write response into

			Arguments:
			ReqContext - Pointer to request context structure
			Method - GET or POST - do we need to open the input file
			InputFileName - Input file name
			OutputFileName - output file name

			Return Value:
			Error Code for the operation.

			--*/
		{
			DWORD Error = ERROR_SUCCESS;

			if (Method == METHOD_TYPE_POST)
			{
				// Open input file
				ReqContext->UploadFile = CreateFile(InputFileName,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,                   // handle cannot be inherited
					OPEN_ALWAYS,            // if file exists, open it
					FILE_ATTRIBUTE_NORMAL,
					NULL);                  // No template file

				if (ReqContext->UploadFile == INVALID_HANDLE_VALUE)
				{
					Error = GetLastError();
					LogSysError(Error, _T("CreateFile for input file"));
					goto Exit;
				}
			}

			// Open output file
			ReqContext->DownloadFile = CreateFile(OutputFileName,
				GENERIC_WRITE,
				0,                        // Open exclusively
				NULL,                     // handle cannot be inherited
				CREATE_ALWAYS,            // if file exists, delete it
				FILE_ATTRIBUTE_NORMAL,
				NULL);                    // No template file

			if (ReqContext->DownloadFile == INVALID_HANDLE_VALUE)
			{
				//Error = GetLastError();
				LogSysError(Error, _T("CreateFile for output file"));
				goto Exit;
			}

		Exit:
			return Error;
		}


		DWORD
			ParseArguments(
			__in int argc,
			__in_ecount(argc) LPTSTR *argv,
			__inout PCONFIGURATION Configuration
			)
			/*++

			Routine Description:
			This routine is used to Parse command line arguments. Flags are
			case sensitive.

			Arguments:
			argc - Number of arguments
			argv - Pointer to the argument vector
			Configuration - pointer to configuration struct to write configuration

			Return Value:
			Error Code for the operation.

			--*/
		{
			int i;
			DWORD Error = ERROR_SUCCESS;

			for (i = 1; i < argc; ++i)
			{
				if (_tcsncmp(argv[i], _T("-"), 1))
				{
					_tprintf(_T("Invalid switch %s\n"), argv[i]);
					i++;
					continue;
				}

				switch (argv[i][1])
				{
				case _T('p'):

					Configuration->UseProxy = 1;
					if (i < argc - 1)
					{
						Configuration->ProxyName = argv[++i];
					}
					break;

				case _T('h'):

					if (i < argc - 1)
					{
						Configuration->HostName = argv[++i];
					}

					break;

				case _T('o'):

					if (i < argc - 1)
					{
						Configuration->ResourceOnServer = argv[++i];
					}

					break;

				case _T('r'):

					if (i < argc - 1)
					{
						Configuration->InputFileName = argv[++i];
					}

					break;

				case _T('w'):

					if (i < argc - 1)
					{
						Configuration->OutputFileName = argv[++i];
					}

					break;

				case _T('m'):

					if (i < argc - 1)
					{
						if (!_tcsnicmp(argv[i + 1], _T("get"), 3))
						{
							Configuration->Method = METHOD_TYPE_GET;
						}
						else if (!_tcsnicmp(argv[i + 1], _T("post"), 4))
						{
							Configuration->Method = METHOD_TYPE_POST;
						}
					}
					++i;
					break;

				case L's':
					Configuration->IsSecureConnection = TRUE;
					break;

				case L't':
					if (i < argc - 1)
					{
						Configuration->UserTimeout = _ttoi(argv[++i]);
					}
					break;

				default:
					Error = ERROR_INVALID_PARAMETER;
					break;
				}
			}

			if (Error == ERROR_SUCCESS)
			{
				if (Configuration->UseProxy && Configuration->ProxyName == NULL)
				{
					printf("No proxy server name provided!\n\n");
					Error = ERROR_INVALID_PARAMETER;
					goto Exit;
				}

				if (Configuration->HostName == NULL)
				{
					printf("Defaulting hostname to: %s\n", DEFAULT_HOSTNAME);
					Configuration->HostName = _T(DEFAULT_HOSTNAME);
				}

				if (Configuration->Method == METHOD_TYPE_NONE)
				{
					printf("Defaulting method to: GET\n");
					Configuration->Method = METHOD_TYPE_GET;
				}

				if (Configuration->ResourceOnServer == NULL)
				{
					printf("Defaulting resource to: %s\n", DEFAULT_RESOURCE);
					Configuration->ResourceOnServer = _T(DEFAULT_RESOURCE);
				}

				if (Configuration->UserTimeout == 0)
				{
					printf("Defaulting timeout to: %d\n", DEFAULT_TIMEOUT);
					Configuration->UserTimeout = DEFAULT_TIMEOUT;
				}

				if (Configuration->InputFileName == NULL && Configuration->Method == METHOD_TYPE_POST)
				{
					printf("Error: File to post not specified\n");
					Error = ERROR_INVALID_PARAMETER;
					goto Exit;
				}

				if (Configuration->OutputFileName == NULL)
				{
					printf("Defaulting output file to: %s\n", DEFAULT_OUTPUT_FILE_NAME);

					Configuration->OutputFileName = _T(DEFAULT_OUTPUT_FILE_NAME);
				}

			}

		Exit:
			return Error;
		}


		VOID
			ShowUsage(
			VOID
			)
			/*++

			Routine Description:
			Shows the usage of the application.

			Arguments:
			None.

			Return Value:
			None.

			--*/
		{
			printf("Usage: async [-m {GET|POST}] [-h <hostname>] [-o <resourcename>] [-s] ");
			printf("[-p <proxyname>] [-w <output filename>] [-r <file to post>] [-t <userTimeout>]\n");
			printf("Option Semantics: \n");
			printf("-m : Specify method (Default: \"GET\")\n");
			printf("-h : Specify hostname (Default: \"%s\"\n", DEFAULT_HOSTNAME);
			printf("-o : Specify resource name on the server (Default: \"%s\")\n", DEFAULT_RESOURCE);
			printf("-s : Use secure connection - https\n");
			printf("-p : Specify proxy\n");
			printf("-w : Specify file to write output to (Default: \"%s\")\n", DEFAULT_OUTPUT_FILE_NAME);
			printf("-r : Specify file to post data from\n");
			printf("-t : Specify time to wait in ms for operation completion (Default: %d)\n", DEFAULT_TIMEOUT);

			return;
		}

		VOID
			LogInetError(
			__in DWORD Err,
			__in LPCTSTR Str
			)
			/*++

			Routine Description:
			This routine is used to log WinInet errors in human readable form.

			Arguments:
			Err - Error number obtained from GetLastError()
			Str - String pointer holding caller-context information

			Return Value:
			None.

			--*/
		{
			DWORD Result;
			PTSTR MsgBuffer = NULL;

			Result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE,
				GetModuleHandle(_T("wininet.dll")),
				Err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&MsgBuffer,
				ERR_MSG_LEN,
				NULL);

			if (Result)
			{
				fprintf(stderr, "%ws: %ws\n", Str, MsgBuffer);
				LocalFree(MsgBuffer);
			}
			else
			{
				fprintf(stderr,
					"Error %d while formatting message for %d in %ws\n",
					GetLastError(),
					Err,
					Str);
			}

			return;
		}

		VOID
			LogSysError(
			__in DWORD Err,
			__in LPCTSTR Str
			)
			/*++

			Routine Description:
			This routine is used to log System Errors in human readable form.

			Arguments:
			Err - Error number obtained from GetLastError()
			Str - String pointer holding caller-context information

			Return Value:
			None.

			--*/
		{
			DWORD Result;
			PTSTR MsgBuffer = NULL;

			Result = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				Err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&MsgBuffer,
				ERR_MSG_LEN,
				NULL);

			if (Result)
			{
				fprintf(stderr,
					"%ws: %ws\n",
					Str,
					MsgBuffer);
				LocalFree(MsgBuffer);
			}
			else
			{
				fprintf(stderr,
					"Error %d while formatting message for %d in %ws\n",
					GetLastError(),
					Err,
					Str);
			}

			return;
		}

	}
}