#include <CommonHeader.h>
#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <regex>


#define	CONFIG_URL_FILENAME	"url.csv"

//////////////////////////////////////////////////////////////////////
//函数功能:传入手机号码查询字符串
//函数参数:
//		mn		传入传出的结果集合
//返回值:
//		0, 成功
//		-1,curl初始化失败
//		-2,curl访问URL失败
//		-9,获取结果失败
int VisitHtmls(STRINGVECTORVECTOR & svvUrl, STRINGVECTORVECTOR & svvMobile, std::string strRequestUrl, std::string strHeaderData = (""))
{
	int nresult = (-9);
	std::string strJsonData = ("");
	std::string strResult = "";
	std::string strUrl = "";
	STRINGVECTORVECTOR svv;
	std::smatch smatch;

	if (string_regex_find(strResult, svv, strRequestUrl + "/", "(.*?)://(.*?)/"))
	{
		strUrl = svv.at(0).at(0) + "://" + svv.at(1).at(0);
	}
	
	nresult = curl_http_get_data(strJsonData, strRequestUrl, strHeaderData);
	
	if (strJsonData.find("charset=utf-8") != std::string::npos)
	{
		strJsonData = UTF82ANSI(strJsonData);
	}

	//string_regex_replace_all(strResult, strJsonData, "", "\x09|\x0A|\x0D|\x20|\\\x2B|<!--(.*?)-->|<a (.*?)/a>");

	//string_regex_replace_all(strResult, strJsonData, "", "\x09|\x0A|\x0D|\x20|\\\x2B|<!--(.*?)-->");
	string_regex_replace_all(strResult, strJsonData, "href=\"http://", "href=\"//");
	string_regex_replace_all(strResult, strJsonData, "href=\"" + strUrl + "/", "href=\"/");

	//file_writer(strJsonData, "d:\\aaa.html");

	string_regex_find(strResult, svvUrl, (strJsonData), "href=\"(.*?)\"");

	string_regex_find(strResult, svvMobile, (strJsonData), "(1[3|5|7|8]\\d{9})");

	return nresult;
}

class CThreadParams {
public:

	CSqlite3DB * pdb;
	std::string url;
	CThreadParams(CThreadParams * ptp)
	{
		this->pdb = ptp->pdb;
		this->url = ptp->url;
	}
	CThreadParams(CSqlite3DB * pdb, std::string url)
	{
		this->pdb = pdb;
		this->url = url;
	}
	void Clear()
	{
		url = "";
	}
};

class CSuburls {
public:
	std::string url;
	std::string from_url;
	CSuburls(std::string url, std::string from_url)
	{
		this->url = url;
		this->from_url = from_url;
	}
	void Clear()
	{
		url = "";
		from_url = "";
	}
};

class CPhone {
public:
	sqlite_uint64 phone;
	std::string from_url;
	CPhone(std::string phone, std::string from_url)
	{
		this->phone = strtoull(phone.c_str(), 0, 10);
		this->from_url = from_url;
	}
	CPhone(sqlite_uint64 phone, std::string from_url)
	{
		this->phone = phone;
		this->from_url = from_url;
	}
	void Clear()
	{
		phone = 0;
		from_url = "";
	}
};

int WriteToDatabse(std::string & result, CSqlite3DB * pdb, CSuburls suburls)
{
	int nresult = (-1);
	std::string strDBCommand = "";

	result = "";

	try
	{
		strDBCommand = "INSERT INTO `suburls`(url,from_url,update_time) VALUES('" + suburls.url + "','" + suburls.from_url + "',CURRENT_TIMESTAMP);";
		pdb->execDML(ANSI2UTF8(strDBCommand).c_str());

		nresult = 0;
	}
	catch (CSqlite3Exception e)
	{
		result = e.errorMessage();
	}
	return nresult;
}
int WriteToDatabse(std::string & result, CSqlite3DB * pdb, CPhone phone)
{
	int nresult = (-1);
	std::string strDBCommand = "";

	result = "";

	try
	{
		char cPhone[16] = { 0 };
		_snprintf(cPhone, sizeof(cPhone) - 1, "%lld", phone.phone);
		strDBCommand = std::string("INSERT INTO `phone`(phone,from_url,update_time) VALUES('") + cPhone + "','" + phone.from_url + "',CURRENT_TIMESTAMP);";
		pdb->execDML(ANSI2UTF8(strDBCommand).c_str());

		nresult = 0;
	}
	catch (CSqlite3Exception e)
	{
		result = e.errorMessage();
	}
	return nresult;
}
int UpdateToDatabse(std::string & result, CSqlite3DB * pdb, CSuburls suburls)
{
	int nresult = (-1);
	std::string strDBCommand = "";

	result = "";

	try
	{
		strDBCommand = "UPDATE `suburls` SET type=1,update_time=CURRENT_TIMESTAMP WHERE url='" + suburls.url + "';";
		pdb->execDML(ANSI2UTF8(strDBCommand).c_str());

		nresult = 0;
	}
	catch (CSqlite3Exception e)
	{
		result = e.errorMessage();
	}
	return nresult;
}
int UpdateToDatabse(std::string & result, CSqlite3DB * pdb, std::string url)
{
	int nresult = (-1);
	std::string strDBCommand = "";

	result = "";

	try
	{
		strDBCommand = std::string("UPDATE `urls` SET type=1,update_time=CURRENT_TIMESTAMP WHERE url='") + url + "';";
		pdb->execDML(ANSI2UTF8(strDBCommand).c_str());

		nresult = 0;
	}
	catch (CSqlite3Exception e)
	{
		result = e.errorMessage();
	}
	return nresult;
}

void HandlerProducerThread(void * p)
{
	std::string strResult = ("");
	if (p)
	{
		CThreadParams tp((CThreadParams *)p);
		delete p;

		STRINGVECTORVECTOR svvUrl;
		STRINGVECTORVECTOR svvMobile;
		VisitHtmls(svvUrl, svvMobile, tp.url);
		
		for (size_t stidx = 0; stidx < svvUrl.size(); stidx++)
		{
			std::cout << tp.url << ", urls= " << svvUrl.at(stidx).size() << std::endl;
			for (size_t stidy = 0; stidy < svvUrl.at(stidx).size(); stidy++)
			{
				if (svvUrl.at(stidx).at(stidy).find("://") != std::string::npos)
				{
					//std::cout << svvUrl.at(stidx).at(stidy) << std::endl;
					WriteToDatabse(strResult, tp.pdb, CSuburls(svvUrl.at(stidx).at(stidy), tp.url));
				}
				else
				{
					svvUrl.at(stidx).erase(svvUrl.at(stidx).begin() + stidy);
				}
			}
		}

		for (size_t stidx = 0; stidx < svvMobile.size(); stidx++)
		{
			std::cout << tp.url << ", phone= " << svvMobile.at(stidx).size() << std::endl;
			for (size_t stidy = 0; stidy < svvMobile.at(stidx).size(); stidy++)
			{
				//std::cout << svvMobile.at(stidx).at(stidy) << std::endl;
				WriteToDatabse(strResult, tp.pdb, CPhone(svvMobile.at(stidx).at(stidy), tp.url));
			}
		}
		UpdateToDatabse(strResult, tp.pdb, tp.url);
	}
}

void StartCrawlProducer(CSqlite3DB & pdb)
{
	std::thread t;
	CSqlite3Query query;
	CThreadParams * pTP = 0;
	std::string strDBCommand = ("");
	std::string strDBErrorText = ("");

	try {
		strDBCommand = "SELECT url FROM `urls` WHERE type=0 ORDER BY update_time ASC;";
		query = pdb.execQuery(ANSI2UTF8(strDBCommand).c_str());

		while (!query.eof())
		{
			pTP = new CThreadParams(&pdb, query.getStringField(0));
			if (pTP)
			{
				t = std::thread(HandlerProducerThread, pTP);
				t.detach();
			}
			query.nextRow();
		}
	}
	catch (CSqlite3Exception e)
	{
		strDBErrorText = e.errorMessage();
		printf("%s\n", strDBErrorText.c_str());
	}
}

void HandlerTasksThread(void * p)
{
	bool bRunning = true;
	std::string strResult = ("");
	std::string strDBCommand = ("");
	std::string strDBErrorText = ("");
	CSqlite3DB * pdb = (CSqlite3DB *)p;
	if (pdb)
	{
		while (bRunning)
		{
			try {
				strDBCommand = "SELECT url,from_url FROM `suburls` WHERE type=0 ORDER BY update_time ASC;";
				CSqlite3Query query = pdb->execQuery(ANSI2UTF8(strDBCommand).c_str());
				while (!query.eof())
				{
					std::string url = query.getStringField(0);
					std::string from_url = query.getStringField(1);
					STRINGVECTORVECTOR svvUrl;
					STRINGVECTORVECTOR svvMobile;
					VisitHtmls(svvUrl, svvMobile, url);
					for (size_t stidx = 0; stidx < svvUrl.size(); stidx++)
					{
						std::cout << url << ", urls= " << svvUrl.at(stidx).size() << std::endl;
						for (size_t stidy = 0; stidy < svvUrl.at(stidx).size(); stidy++)
						{
							if (svvUrl.at(stidx).at(stidy).find("://") != std::string::npos)
							{
								//std::cout << svvUrl.at(stidx).at(stidy) << std::endl;
								WriteToDatabse(strResult, pdb, CSuburls(svvUrl.at(stidx).at(stidy), from_url));
							}
							else
							{
								svvUrl.at(stidx).erase(svvUrl.at(stidx).begin() + stidy);
							}
						}
					}

					for (size_t stidx = 0; stidx < svvMobile.size(); stidx++)
					{
						std::cout << url << ", phone= " << svvMobile.at(stidx).size() << std::endl;
						for (size_t stidy = 0; stidy < svvMobile.at(stidx).size(); stidy++)
						{
							//std::cout << svvMobile.at(stidx).at(stidy) << std::endl;
							WriteToDatabse(strResult, pdb, CPhone(svvMobile.at(stidx).at(stidy), url));
						}
					}
					UpdateToDatabse(strResult, pdb, CSuburls(url, from_url));
					query.nextRow();
				}
			}
			catch (CSqlite3Exception e)
			{
				strDBErrorText = e.errorMessage();
				printf("%s\n", strDBErrorText.c_str());
			}

			//休眠5秒
			Sleep(5000);
		}
	}
}

void StartCrawlTasks(CSqlite3DB & pdb)
{
	std::thread t;
	t = std::thread(HandlerTasksThread, &pdb);
	t.detach();
}

void init_urls(CSqlite3DB * pdb, std::string strFileName)
{
	std::string data;
	STRINGVECTOR sv;
	std::string strDBCommand = ("");

	file_reader(data, strFileName);
	string_replace_all(data, "\n", "\r\n");
	string_split_to_vector(sv, data, "\n");
	for (size_t stidx = 0; stidx < sv.size(); stidx++)
	{
		if (sv.at(stidx).find("://") != std::string::npos)
		{
			std::string strDBErrorText = ("");
			
			try {
				strDBCommand = "SELECT count(url) FROM `urls` WHERE url='" + sv.at(stidx) + "';";
				if (pdb->execScalar(UTF82ANSI(strDBCommand).c_str()) > 0)
				{
					continue;
				}
			}
			catch (CSqlite3Exception e)
			{
				strDBErrorText = e.errorMessage();
				printf("%s\n", strDBErrorText.c_str());
			}
			strDBCommand = "INSERT INTO urls(url) VALUES('" + sv.at(stidx) + "');";
			pdb->execDML(ANSI2UTF8(strDBCommand).c_str());
		}
	}
}
void Usage(const char * p)
{
	printf("Usage:\n\t%s from_file [from_filter]\n"
		"\tExamples:[from_filter=filter.ini]\n"
		"\t%s 100.txt filter.ini\n"
		"\t%s 100.txt\n",
		p, p, p
	);
}
int main(int argc, char ** argv)
{
	STRINGVECTOR sv;
	STRINGVECTOR svINI;
	std::string strData = ("");
	std::string strFilter = ("");
	std::string strDBCommand = ("");
	std::string strINFileName = ("");
	std::string strOUTFileName = ("");
	std::string strINIFileName = ("");
	switch (argc)
	{
	case 2:
		strINFileName = argv[1];
		strOUTFileName = strINFileName + "out";
		strINIFileName = "filter.ini";
		break;
	case 3:
		strINFileName = argv[1];
		strOUTFileName = strINFileName + "out";
		strINIFileName = argv[2];
		break;
	default:
		Usage(argv[0]); return 0;
		break;
	}

	file_reader(strFilter, strINIFileName);
	file_reader(strData, strINFileName);

	string_replace_all(strData, "\n", "\r\n");
	string_split_to_vector(sv, strData, "\n");
	string_replace_all(strFilter, "\n", "\r\n");
	string_split_to_vector(svINI, strFilter, "\n");
	strData.clear();
	for (size_t stidx = 0; stidx < sv.size(); stidx++)
	{
		bool bFound = false;
		for (size_t stidy = 0; stidy < svINI.size(); stidy++)
		{
			if (!svINI.at(stidy).compare(sv.at(stidx).substr(0, svINI.at(stidy).length())))
			{
				bFound = true;
				break;
			}
		}
		if (bFound)
		{
			sv.erase(sv.begin() + stidx);
			stidx--;
		}
		else
		{
			strData.append(sv.at(stidx)).append("\r\n");
		}
	}
	if (strData.length() > 0)
	{
		file_writer(strData, strOUTFileName);
	}
	return 0;
}