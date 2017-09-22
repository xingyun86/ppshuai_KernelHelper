
#include <CommonHeader.h>
#include <thread>

int test2(void)
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

	std::string strMsgData = encrypt_ecb(std::string("{"
		"\"msgid\": 1231231566,"
		"\"msg_content\": \"\","
		"\"from_user\": \"123\","
		"\"to_user\": \"456\","
		"\"msg_type\": 3,"
		"\"timestamp\": 0,"
		"\"md5\": \"" + MD5Data(strFileData.c_str(), strFileData.size()) + "\","
		"\"file_name\": \"123.png\""
		"} ").c_str());

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

typedef std::map<std::string, struct _iobuf *> StringIobuffMap;
typedef StringIobuffMap::iterator StringIobuffMapIT;
typedef StringIobuffMap::value_type StringIobuffMapPair;

StringIobuffMap G_SIMap;
void write_city(std::string strCityName)
{
	FILE * pFile = fopen("last.txt", "a+b");
	if (pFile)
	{
		fwrite(std::string(strCityName + "\r\n").c_str(), std::string(strCityName + "\r\n").length(), 1, pFile);
		fclose(pFile);
	}
}
void write_last(std::string stPrefix, size_t stMiddle)
{
	char cFileData[MAXCHAR] = { 0 };
	FILE * pFile = fopen("last.txt", "wb");
	if (pFile)
	{
		_snprintf(cFileData, sizeof(cFileData) - 1, "%s%04d", stPrefix.c_str(), stMiddle);
		fwrite((void *)cFileData, strlen(cFileData), 1, pFile);
		fclose(pFile);
	}
}
void init_addr_list(std::string strFileName)
{
	size_t stSize = 0;
	std::string strData = "";
	STRINGVECTOR sv;
	FILE * pFile = 0;
	
	pFile = fopen(strFileName.c_str(), "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		stSize = ftell(pFile);
		if (stSize > 0)
		{
			strData.resize(stSize, '\0');
			fseek(pFile, 0, SEEK_SET);
			fread((void*)strData.c_str(), strData.size(), 1, pFile);
		}
		fclose(pFile);
	}
	if (strData.length() > 0)
	{
		string_split_to_vector(sv, strData.c_str(), "\r\n");
		for (size_t stIdx = 0; stIdx < sv.size(); stIdx++)
		{
			pFile = fopen(std::string(sv.at(stIdx) + ".txt").c_str(), "a+b");
			if (pFile)
			{
				G_SIMap.insert(StringIobuffMapPair(sv.at(stIdx), pFile));
			}
		}
	}
}
void retrieve_lastsize(std::string strFileName, const char ** ppList, size_t stListSize, size_t & stListIdx, size_t & stMiddle)
{
	size_t stSize = 0;
	std::string strData = "";
	STRINGVECTOR sv;
	FILE * pFile = 0;

	stListIdx = 0;
	stMiddle = 0;
	pFile = fopen(strFileName.c_str(), "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		stSize = ftell(pFile);
		if (stSize > 0)
		{
			strData.resize(stSize, '\0');
			fseek(pFile, 0, SEEK_SET);
			fread((void*)strData.c_str(), strData.size(), 1, pFile);
		}
		fclose(pFile);
	}
	if (strData.length() > 0)
	{
		std::string strPrefix = strData.substr(0, 3);
		std::string strMiddle= strData.substr(3, 4);
		//号码前缀3位
		for (size_t i = 0; i < stListSize; i++)
		{
			if (!strPrefix.compare(ppList[i]))
			{
				stListIdx = i;
				stMiddle = strtol(strMiddle.c_str(), 0, 10);
				break;
			}
		}		
	}
}

class CMobileNumber {
public:
	std::string strSectionNumber;
	std::string strCallNumber;
	std::string strProvince;
	std::string strCity;
	std::string strAreaCode;
	std::string strPostCode;

	void Clear()
	{
		strSectionNumber = "";
		strCallNumber = "";
		strProvince = "";
		strCity = "";
		strAreaCode = "";
		strPostCode = "";
	}
};

//////////////////////////////////////////////////////////////////////
//函数功能:传入手机号码查询字符串
//函数参数:
//		mn		传入传出的结果集合
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
//		-9,获取结果失败
int VerifyMobileNumber(CMobileNumber & mn)
{
	int result = (-1);
	int curlresult = (-1);
	std::string strStart = ("");
	std::string strFinal = ("");
	std::string strKeyData = ("");
	std::string strJsonData = ("");
	std::string::size_type stPos = 0;
	std::string::size_type stIdxPos = 0;
	
	std::string strRequestUrl = "http://www.ip138.com:8080/search.asp?mobile=" + mn.strCallNumber + "&action=mobile";
	std::string strHeaderData = 
		"Host: www.ip138.com:8080\r\n"
		"Connection: keep-alive\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
		"Accept-Encoding: gzip, deflate, sdch\r\n"
		"Accept-Language: zh-CN,zh;q=0.8";
	std::string strPostFields = "";
	curlresult = curl_http_get_data(strJsonData, strRequestUrl, strHeaderData, strPostFields);
	if (curlresult != 0)
	{
		result = curlresult;
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(strJsonData, "", "<!-- <td width=\"138\" align=\"center\" noswap></td> -->");
	string_replace_all(strJsonData, "", "<!-- <td></td> -->");
	string_replace_all(strJsonData, ">", " noswap>");

	strStart = "<TD width=\"138\" align=\"center\">";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<TD width=* align=\"center\" class=tdc2>";
	strFinal = " <a href=";
	stPos = string_reader(mn.strCallNumber, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(mn.strCallNumber, "", "&nbsp;");
	stIdxPos = stPos;

	strStart = "<TD width=\"138\" align=\"center\">";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<td align=\"center\" class=tdc2>";
	strFinal = "&nbsp";
	stPos = string_reader(mn.strProvince, strJsonData, strStart, strFinal, stIdxPos);
	string_replace_all(mn.strProvince, "", " ");
	if (stPos == std::string::npos || mn.strProvince.length() <= 0 || 
		(mn.strProvince.find("TD") != std::string::npos) || 
		(mn.strProvince.find("未知") != std::string::npos))
	{
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(mn.strProvince, "", "&nbsp;");
	stIdxPos = stPos;

	strStart = ";";
	strFinal = "</TD>";
	stPos = string_reader(mn.strCity, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(mn.strCity, "", "&nbsp;");
	stIdxPos = stPos;

	strStart = "<TD width=\"138\" align=\"center\">";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<TD align=\"center\" class=tdc2>";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<TD align=\"center\">";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<TD align=\"center\" class=tdc2>";
	strFinal = "</TD>";
	stPos = string_reader(mn.strAreaCode, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(mn.strAreaCode, "", "&nbsp;");
	stIdxPos = stPos;

	strStart = "<TD align=\"center\">";
	strFinal = "</TD>";
	stPos = string_reader(strKeyData, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	stIdxPos = stPos;

	strStart = "<TD align=\"center\" class=tdc2>";
	strFinal = " <a href=";
	stPos = string_reader(mn.strPostCode, strJsonData, strStart, strFinal, stIdxPos);
	if (stPos == std::string::npos)
	{
		goto __LEAVE_CLOSE__;
	}
	string_replace_all(mn.strPostCode, "", "&nbsp;");
	stIdxPos = stPos;

	result = 0;

__LEAVE_CLOSE__:
	
	return result;
}
#include <regex>
int string_regex_replace_all(std::string & strData, std::string strReplace, std::string strPattern)
{
	int result = (-1);
	strData = std::regex_replace(strData, std::regex(strPattern), strReplace, std::regex_constants::match_default);
	if (strData.length() > 0)
	{
		result = strData.length();
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
//函数功能:传入手机号码查询字符串
//函数参数:
//		mn		传入传出的结果集合
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
//		-9,获取结果失败
int VerifyMobileNumberEx(CMobileNumber & mn)
{
	int result = (-9);
	int curlresult = (-1);
	std::string strStart = ("");
	std::string strFinal = ("");
	std::string strKeyData = ("");
	std::string strJsonData = ("");
	std::string::size_type stPos = 0;
	std::string::size_type stIdxPos = 0;

	size_t stidx = 0;
	std::smatch searchmatch;

	std::string strRequestUrl = "http://www.ip138.com:8080/search.asp?mobile=" + mn.strCallNumber + "&action=mobile";
	std::string strHeaderData =
		"Host: www.ip138.com:8080\r\n"
		"Connection: keep-alive\r\n"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
		"Upgrade-Insecure-Requests: 1\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/45.0.2454.101 Safari/537.36\r\n"
		"Accept-Encoding: gzip, deflate, sdch\r\n"
		"Accept-Language: zh-CN,zh;q=0.8";
	std::string strPostFields = "";
	curlresult = curl_http_get_data(strJsonData, strRequestUrl, strHeaderData, strPostFields);
	if (curlresult != 0)
	{
		result = curlresult;
		goto __LEAVE_CLOSE__;
	}
	string_regex_replace_all(strJsonData, "", "\x09|\x0A|\x0D|\x20|\\\x2B|<!--(.*?)-->|<a (.*?)/a>");

	if (regex_search(strJsonData, searchmatch, std::regex("您查询的手机号码段(.*?)</TABLE>")))
	{
		stidx = 0;
		if (searchmatch.size() == (1 + 1))
		{
			stidx++;
			strJsonData = searchmatch[stidx];

			//string_regex_replace_all(strJsonData, "", "\x09|\x0A|\x0D|\x20|\\\x2B|<!--(.*?)-->|<a (.*?)/a>");
			string_regex_replace_all(strJsonData, "<TABLE>", "<TABLE(.*?)>|<table(.*?)>");
			string_regex_replace_all(strJsonData, "/TABLE>", "/TABLE>|/table>");
			string_regex_replace_all(strJsonData, "<TD>", "<TD(.*?)>|<td(.*?)>");
			string_regex_replace_all(strJsonData, "/TD>", "/TD>|/td>");
			string_regex_replace_all(strJsonData, "<TR>", "<TR(.*?)>|<tr(.*?)>");
			string_regex_replace_all(strJsonData, "/TR>", "/TR>|/tr>");

			if (regex_search(strJsonData, searchmatch, std::regex("</TD><TD>(.*?)</TD></TR><TR><TD>卡号归属地</TD><TD>(.*?)&nbsp;(.*?)</TD></TR><TR><TD>卡&nbsp;类&nbsp;型</TD><TD>(.*?)</TD></TR><TR><TD>区号</TD><TD>(.*?)</TD></TR><TR><TD>邮编</TD><TD>(.*?)</TD></TR>")))
			{
				stidx = 0;
				if (searchmatch.size() == (6 + 1))
				{
					stidx++;
					mn.strCallNumber = searchmatch[stidx];
					stidx++;
					mn.strProvince = searchmatch[stidx];
					stidx++;
					mn.strCity = searchmatch[stidx];
					stidx++; stidx++;
					mn.strAreaCode = searchmatch[stidx];
					stidx++;
					mn.strPostCode = searchmatch[stidx];
					stidx++;
					if (mn.strCallNumber.length() > 0 &&
						mn.strProvince.length() > 0 &&
						mn.strAreaCode.length() > 0 && 
						mn.strPostCode.length() > 0)
					{
						result = 0;
					}
					else
					{
						goto __LEAVE_CLOSE__;
					}
				}
			}
		}
	}
	
__LEAVE_CLOSE__:

	return result;
}

typedef struct tagThreadParams {
	CSqlite3DB * pdb;
	size_t section_number;
	size_t middler_number;
}ThreadParams, *PThreadParams;

int WriteToDatabse(std::string & strResult, CSqlite3DB * pdb, CMobileNumber & mn)
{
	int result = (-1);
	std::string strDBCommand = "";
	try 
	{
		if (mn.strCity.length() <= 0)
		{
			mn.strCity = mn.strProvince;
		}
		strDBCommand = "INSERT INTO `call`(call_number,section_number,city) VALUES('" + mn.strCallNumber + "','" + mn.strSectionNumber + "','" + mn.strCity + "');"
			"INSERT INTO `area`(city, province, area_code, post_code) VALUES('" + mn.strCity + "','" + mn.strProvince + "','" + mn.strAreaCode + "','" + mn.strPostCode + "')";
		pdb->execDML(ANSI2UTF8(strDBCommand).c_str());
		strResult = "ok";
		result = 0;
	}
	catch (CSqlite3Exception e)
	{
		strResult = e.errorMessage();
	}
	return result;
}
void HandlerThread(void * p)
{
	CMobileNumber mn;
	ThreadParams tp = { 0 };
	std::string strResult = ("");
	char cSectionNumber[16] = { 0 };
	char cMiddlerNumber[16] = { 0 };
	if (p)
	{
		memcpy(&tp, p, sizeof(tp));
		free(p);

		_snprintf(cSectionNumber, sizeof(cSectionNumber) - 1, "%d", tp.section_number);
		
		tp.middler_number = (0);

		try {
			CSqlite3Query query = tp.pdb->execQuery(ANSI2UTF8(std::string("SELECT (max(call_number) - section_number * 10000) AS number FROM call WHERE call_number LIKE '") + cSectionNumber + "%'").c_str());
			if (!query.eof())
			{
				tp.middler_number = query.getIntField(0);
			}
		}
		catch (CSqlite3Exception e)
		{
			tp.middler_number = (0);
		}

		for (size_t stM = tp.middler_number; stM < 10000; stM++)
		{
			mn.Clear();
			_snprintf(cMiddlerNumber, sizeof(cMiddlerNumber) - 1, "%04d", stM);

			mn.strSectionNumber = cSectionNumber;
			mn.strCallNumber = mn.strSectionNumber + cMiddlerNumber;
			printf("%s\n", mn.strCallNumber.c_str());
			switch (VerifyMobileNumberEx(mn)) {
			case 0:
				WriteToDatabse(strResult, tp.pdb, mn);
				break;
			default:

				break;
			}			
		}
	}
}
int main(int argc, char ** argv)
{
	CSqlite3DB sqldb;
	ThreadParams * pTP = 0;
	std::string strDBFileName = "telephone.db";
	std::string strDBCommand = 
		"CREATE TABLE IF NOT EXISTS `misp`(section_number INTEGER NOT NULL PRIMARY KEY, provider_name VARCHAR(128) NOT NULL, comment TEXT);"
		"CREATE TABLE IF NOT EXISTS `area`(city VARCHAR(128) NOT NULL PRIMARY KEY, province VARCHAR(128) NOT NULL, area_code VARCHAR(32) NOT NULL, post_code VARCHAR(32) NOT NULL, comment TEXT);"
		"CREATE TABLE IF NOT EXISTS `call`(call_number INTEGER NOT NULL PRIMARY KEY, section_number INTEGER NOT NULL, city VARCHAR(128) NOT NULL, comment TEXT);";
	std::string strDBErrorText = "";
	printf("%s\n", sqldb.SQLiteVersion());

	try {
		sqldb.open(strDBFileName.c_str());

		sqldb.execDML(ANSI2UTF8(strDBCommand).c_str());

		strDBCommand =
			"INSERT INTO `misp`(section_number,provider_name) VALUES"
			"('133','电信'),('153','电信'),('180','电信'),('181','电信'),('189','电信'),('177','电信'),('173','电信'),('149','电信'),"
			"('130','联通'),('131','联通'),('132','联通'),('155','联通'),('156','联通'),('145','联通'),('185','联通'),('186','联通'),('176','联通'),('175','联通'),"
			"('134','移动'),('135','移动'),('136','移动'),('137','移动'),('138','移动'),('139','移动'),('150','移动'),('151','移动'),('152','移动'),('157','移动'),('158','移动'),('159','移动'),('182','移动'),('183','移动'),('184','移动'),('187','移动'),('188','移动'),('147','移动'),('178','移动');";

		sqldb.execDML(ANSI2UTF8(strDBCommand).c_str());
	}
	catch (CSqlite3Exception e)
	{
		;//no op
	}

	try {
		strDBCommand = "SELECT section_number FROM `misp` ORDER BY section_number ASC;";
		CSqlite3Query query = sqldb.execQuery(ANSI2UTF8(strDBCommand).c_str());
		
		while (!query.eof())
		{
			pTP = (ThreadParams *)malloc(sizeof(ThreadParams));
			if (pTP)
			{
				pTP->pdb = &sqldb;
				pTP->middler_number = 0;
				pTP->section_number = query.getIntField(0);				
				std::thread t(HandlerThread, pTP);
				t.detach();
			}
			query.nextRow();
		}
		getchar();
		sqldb.close();
	} catch (CSqlite3Exception e)
	{
		strDBErrorText = e.errorMessage();
		printf("%s\n", strDBErrorText.c_str());
	}

	return 0;
}