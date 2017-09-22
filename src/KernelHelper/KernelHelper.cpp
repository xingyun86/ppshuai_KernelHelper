// TantanHelper.cpp : Defines the entry point for the console application.
//

#include <CommonHeader.h>
#include <sys/timeb.h>
#include <vector>

#define URL_FILENAME	"url.txt"
#define COOKIE_FILENAME	"cookie.txt"

string current_time()
{
	struct timeb tb = { 0 };
	ftime(&tb);
	char szTime[128] = { 0 };
	snprintf(szTime, sizeof(szTime) / sizeof(*szTime), "%lld%03u", tb.time, tb.millitm);
	return string(szTime);
}

string read_file(string strFileName)
{
	string strData = "";
	long lFileSize = 0;
	FILE * pFile = fopen(strFileName.c_str(), "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		lFileSize = ftell(pFile);
		if (lFileSize > 0)
		{
			fseek(pFile, 0, SEEK_SET);
			strData.resize(lFileSize * sizeof(unsigned char));
			fread((void *)strData.c_str(), strData.size(), sizeof(unsigned char), pFile);
		}
		fclose(pFile);
	}
	return strData;
}

int main(int argc, char ** argv)
{
	int result = 0;
	CURL * pCurl = 0;
	string strJson = "";
	string strRequestUrl = read_file(URL_FILENAME);
	string strPostData = "";
	string strCookieData = read_file(COOKIE_FILENAME);
	string strHeaderData = read_file(COOKIE_FILENAME);
	curl_exec();
	return 0;
}

