
#define CURL_STATICLIB
#include <curl/curl.h>

#include <string>
using namespace std;

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
int curl_http_get_file(string strSavePathFileName, string strRequestURL, string strPostFields = "", string strHeaderData = "", bool bVerbose = false, int nDelayTime = 60);

//////////////////////////////////////////////////////////////////////
//函数功能:GET方法传入URL获取JSON字符串
//函数参数:
//		strJsonData		返回的JSON数据字符串
//		strRequestURL	下载地址URL
//		strHeaderData	要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields	发送的POST域数据
//		bVerbose		是否为详细日志信息
//		nDelayTime		超时设置，默认为60秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_get_data(string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60);

//////////////////////////////////////////////////////////////////////
//函数功能:POST方法传入URL及参数获取JSON字符串
//函数参数:
//		strJsonData		返回的JSON数据字符串
//		strRequestURL	下载地址URL
//		strHeaderData	要发送的头部数据字符串数组(\r\n为分隔符)
//		strPostFields	发送的POST域数据
//		nDelayTime		超时设置，默认为60秒
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
int curl_http_post_data(string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60);

CURL * curl_http_init();
void curl_http_exit(CURL *pCurl);
void curl_http_print_cookies(CURL *pCurl);
int curl_http_post_data(CURL *pCurl, string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60);
int curl_http_get_data(CURL * pCurl, string & strJsonData, string strRequestURL, string strHeaderData = "", string strPostFields = "", bool bVerbose = false, int nDelayTime = 60);

int curl_http_post_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData = "", const char * pPostFields = "", bool bVerbose = false, int nDelayTime = 60);
int curl_http_get_data(CURL * pCurl, char * pJsonData, unsigned int nJsonDataSize, const char * pRequestUrl, const char * pHeaderData = "", const char * pPostFields = "", bool bVerbose = false, int nDelayTime = 60);

void init_curl_env(CURL ** p_p_curl);
void exit_curl_env(CURL ** p_p_curl);
int visit_sites(CURL *p_curl, string strUrl, string strHeaderDataList,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""));

int visit_posts_sites(CURL *p_curl, string strUrl, string strHeaderDataList, string strPostFields,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""));
int visit_sites(string strUrl, string strHeaderDataList,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""));

int visit_posts_sites(string strUrl, string strHeaderDataList, string strPostFields,
	string strHeaderFileName = ("head.dat"), string strBodyFileName = ("body.html"),
	string strCookIEFileName = ("cookie.dat"), string strCookIEJarFileName = ("cookie.dat"),
	string strEncodingUncompressMethodType = (""));

typedef struct tagHeaderData {
	int	   size;
	char * data;
}HeaderData, *PHeaderData;

static size_t header_callback(char *buffer, size_t size,
	size_t nitems, void *userdata)
{
	HeaderData *hd = (HeaderData *)userdata;

	/* now this callback can access the my_info struct */
	//printf("[%s]\n", hd->data);
	return nitems * size;
}

__inline static void dump(const char *text,
	FILE *stream, unsigned char *ptr, size_t size)
{
	size_t i;
	size_t c;
	unsigned int width = 0x10;

	fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
		text, (long)size, (long)size);

	for (i = 0; i<size; i += width) {
		fprintf(stream, "%4.4lx: ", (long)i);

		/* show hex to the left */
		for (c = 0; c < width; c++) {
			if (i + c < size)
				fprintf(stream, "%02x ", ptr[i + c]);
			else
				fputs("   ", stream);
		}

		/* show data on the right */
		for (c = 0; (c < width) && (i + c < size); c++) {
			char x = (ptr[i + c] >= 0x20 && ptr[i + c] < 0x80) ? ptr[i + c] : '.';
			fputc(x, stream);
		}

		fputc('\n', stream); /* newline */
	}
}

__inline static int debug_trace(CURL *handle, curl_infotype type,
	char *data, size_t size,
	void *userp)
{
	const char *text;
	(void)handle; /* prevent compiler warning */
	(void)userp;

	switch (type) {
	case CURLINFO_TEXT:
		fprintf(stderr, "== Info: %s", data);
	default: /* in case a new one is introduced to shock us */
		return 0;

	case CURLINFO_HEADER_OUT:
		text = "=> Send header";
		break;
	case CURLINFO_DATA_OUT:
		text = "=> Send data";
		break;
	case CURLINFO_SSL_DATA_OUT:
		text = "=> Send SSL data";
		break;
	case CURLINFO_HEADER_IN:
		text = "<= Recv header";
		break;
	case CURLINFO_DATA_IN:
		text = "<= Recv data";
		break;
	case CURLINFO_SSL_DATA_IN:
		text = "<= Recv SSL data";
		break;
	}

	dump(text, stderr, (unsigned char *)data, size);
	return 0;
}
typedef struct _tagCallBackData {
#define call_back_data_size 16384
	char * p;
	unsigned int s;
	unsigned int v;
	static void * startup()
	{
		void * thiz = malloc(sizeof(struct _tagCallBackData));
		if (thiz)
		{
			((struct _tagCallBackData *)thiz)->init();
		}
		return thiz;
	}
	_tagCallBackData()
	{
	}
	_tagCallBackData(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
	{
		init(_p, _s, _v);
	}
	void init(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
	{
		p = _p ? (*_p) : 0; s = _s; v = (!p || !_v) ? call_back_data_size : _v;
		if (!p)
		{
			p = (char *)malloc(v * sizeof(char));
		}
		if (p && v > 0)
		{
			memset(p, 0, v * sizeof(char));
		}
	}
	char * append(char * _p, unsigned int _s)
	{
		if (_s > 0)
		{
			if (s + _s > v)
			{
				v += call_back_data_size;
				p = (char *)realloc(p, v * sizeof(char));
			}
			if (p)
			{
				memcpy(p + s, _p, _s);
				s += _s;
			}
		}
		return p;
	}
	void exit(char ** _p)
	{
		if (_p && (*_p))
		{
			free((*_p));
			(*_p) = 0;
		}
		s = v = 0;
	}
	void cleanup()
	{
		exit(&p);
		free(this);
	}
}CallBackData, *PCallBackData;

//curl 回调处理返回数据函数
__inline static size_t write_native_data(void * buffer, size_t size, size_t nmenb, void * pv)
{
	CallBackData * pcbd = (CallBackData*)pv;
	if (pcbd && buffer) {
		pcbd->append((char *)buffer, size*nmenb);
	}
	return nmenb;
}
//curl 回调处理返回数据函数
__inline static size_t write_dynamic_data(void * buffer, size_t size, size_t nmenb, void * pv)
{
	string * pstr = dynamic_cast<string *>((string*)pv);
	if (pstr != nullptr && buffer != nullptr) {
		pstr->append((char *)buffer, size*nmenb);
	}
	
	return nmenb;
}
__inline static void curl_exec()
{
	CURL *curl = curl_easy_init();
	if (curl) {
				
		// set accept encoding
		char * pAcceptEncoding = ""; //"'gzip,deflate'"
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, pAcceptEncoding);

		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

		// send it verbose for max debuggaility
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		// set debug function callback
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_trace);

		/* pass in custom data to the callback */
		HeaderData hd = { 10, "" };
		curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &hd);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);

		// HTTP/2 please
		curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

		// set request url
		char * pUrl = "https://www.baidu.com";
		curl_easy_setopt(curl, CURLOPT_URL, pUrl);

		// set http header list data
		char * pHeaderData = "";
		struct curl_slist *p_curl_slist_header = 0;
		p_curl_slist_header = curl_slist_append(p_curl_slist_header, pHeaderData);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, p_curl_slist_header);
				
		// start cookie engine
		char * pCookieFile = "";
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, pCookieFile);
		char * pCookieJar = "";
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, pCookieJar);

		// set timeout
		long lDelayTime = 10;
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, lDelayTime);

		// write header and body data
		CallBackData * pcbd_header = (CallBackData *)CallBackData::startup();
		CallBackData * pcbd_data = (CallBackData *)CallBackData::startup();
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_native_data);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)pcbd_header);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)pcbd_data);

		// write header and body data
		std::string str_header = "";
		std::string str_data = "";
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_dynamic_data);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, (void *)&str_header);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&str_data);

		curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 1L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);

		// post data
		char * pPostFields = "";
		curl_easy_setopt(curl, CURLOPT_POST, 0L);
		//curl_easy_setopt(curl, CURLOPT_POST, 1L);
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pPostFields);

		// we use a self-signed test server, skip verification during debugging
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

		curl_easy_perform(curl);
		
		printf("==[%s]\n==[%s]\n", str_header.c_str(), str_data.c_str());

		printf("==[%s]\n==[%s]\n", pcbd_header->p, pcbd_data->p);

		curl_easy_cleanup(curl);

		pcbd_header->cleanup();
		pcbd_data->cleanup();
	}
}


__inline static int post_form(void)
{
	CURL *curl;

	CURLM *multi_handle;
	int still_running;

	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";
	//static const char buf[] = "Content-Type: multipart/form-data";
	std::string strFileData;
	FILE * pF = fopen(std::string("D:\\123.png").c_str(), "rb");
	//std::string strError = strerror(errno);
	if (pF)
	{
		fseek(pF, 0, SEEK_END);
		size_t len = ftell(pF);
		if (len)
		{
			fseek(pF, 0, SEEK_SET);
			strFileData.resize(len, '\0');
			fread((void *)strFileData.c_str(), strFileData.size(), 1, pF);
		}
		fclose(pF);
	}

	std::string strMsgData = "{\"id\":123}";
	// set up the header
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "cache-control",
		CURLFORM_COPYCONTENTS, "no-cache",
		CURLFORM_END);
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "msg",
		CURLFORM_COPYCONTENTS, strMsgData.c_str(),
		CURLFORM_END);
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "content-type",
		CURLFORM_COPYCONTENTS, "multipart/form-data",
		CURLFORM_END);

	curl_formadd(&formpost, &lastptr,
		CURLFORM_COPYNAME, "file",  // <--- the (in this case) wanted file-Tag!
		CURLFORM_BUFFER, "data",
		CURLFORM_BUFFERPTR, strFileData.data(),
		CURLFORM_BUFFERLENGTH, strFileData.size(),
		CURLFORM_END);

	curl = curl_easy_init();

	/* initialize custom header list (stating that Expect: 100-continue is not
	wanted */
	headerlist = curl_slist_append(headerlist, buf);
	if (curl) {

		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.246:8099/ZSWXApi/api/ZSWX/UploadFile");
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

		CURLcode res = curl_easy_perform(curl);
		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}

		/* always cleanup */
		curl_easy_cleanup(curl);

		/* then cleanup the formpost chain */
		curl_formfree(formpost);

		/* free slist */
		curl_slist_free_all(headerlist);
	}
	return 0;
}