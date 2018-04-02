#include <CommonHeader.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment(lib, "kernelhelper.lib")
void Usage(const char * p)
{
	printf("Usage:\n\t%s from_file [from_db]\n"
		"\tExamples:[from_db=telephone-in.db]\n"
		"\t%s 100.txt my.db\n"
		"\t%s 100.txt\n",
		p, p, p
	);
}
int main(int argc, char ** argv)
{
	STRINGVECTOR sv;
	CSqlite3DB sqldb;
	size_t stidx = 0;
	CSqlite3Query query;
	std::string strResult;
	std::string strData;
	std::string strFrom;
	std::string strINFileName = "";
	std::string strOutFileName = "";
	std::string strDBFileName = "";
	std::string strDBCommand = 
		"CREATE TABLE IF NOT EXISTS `urls`(url TEXT NOT NULL PRIMARY KEY, type INTEGER NOT NULL DEFAULT 0, comment TEXT, update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);" // type = 0,未采集过;type = 1,已采集过
		"CREATE TABLE IF NOT EXISTS `suburls`(url TEXT NOT NULL PRIMARY KEY, from_url TEXT NOT NULL, type INTEGER NOT NULL DEFAULT 0, comment TEXT, update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);" // type = 0,未采集过;type = 1,已采集过
		"CREATE TABLE IF NOT EXISTS `phone`(phone INTEGER NOT NULL PRIMARY KEY, from_url TEXT NOT NULL, comment TEXT, update_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, create_time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP);";
	std::string strDBErrorText = "";
	printf("%s\n", sqldb.SQLiteVersion());

	switch (argc)
	{
	case 2:
		strINFileName = argv[1];
		strOutFileName = strINFileName + ".csv";
		strDBFileName = "telephone-in.db";
		break;
	case 3:
		strINFileName = argv[1];
		strOutFileName = strINFileName + ".csv";
		strDBFileName = argv[2];
		break;
	default:
		Usage(argv[0]); return 0;
		break;
	}
	try {
		sqldb.open(strDBFileName.c_str());

		sqldb.execDML(PPSHUAI::Convert::ANSI2UTF8(strDBCommand).c_str());
	}
	catch (CSqlite3Exception e)
	{
		;//no op
	}

	try {
		//strDBCommand = "SELECT p.phone,s.from_url FROM phone p,suburls s WHERE p.from_url==s.url GROUP BY phone ORDER BY p.update_time;";
		PPSHUAI::String::file_reader(strData, strINFileName);
		PPSHUAI::String::string_split_to_vector(sv, strData, "\r\n");
		strData = "";
		for (stidx = 0; stidx < sv.size(); stidx++)
		{
			if (sv.at(stidx).at(0))
			{
				strDBCommand = "SELECT p.phone,s.from_url FROM phone p,suburls s WHERE p.from_url==s.url AND p.phone=" + sv.at(stidx) + " GROUP BY phone ORDER BY p.update_time;";
				query = sqldb.execQuery(PPSHUAI::Convert::ANSI2UTF8(strDBCommand).c_str());
				if (!query.eof())
				{
					strFrom = query.getStringField(1);
					PPSHUAI::String::string_regex_replace_all(strResult, strFrom, "", "(.*?//.*?\\.)");
					if (strFrom.find("/") != std::string::npos)
					{
						strFrom = strFrom.substr(0, strFrom.find("/"));
					}
					strData += sv.at(stidx) + "," + strFrom + "\r\n";
				}
			}
		}
		sv.clear();
		PPSHUAI::String::string_split_to_vector(sv, strData, "\r\n");
		printf("Output %ld resources!\n", sv.size());
		if (strData.size())
		{
			PPSHUAI::String::file_writer(strData, strOutFileName);
			printf("Write to file ok!\n");
		}
		
	} catch (CSqlite3Exception e)
	{
		strDBErrorText = e.errorMessage();
		printf("%s\n", strDBErrorText.c_str());
	}
	
	try {
		sqldb.close();
	}
	catch (CSqlite3Exception e)
	{
		;//no op
	}
	
	return 0;
}