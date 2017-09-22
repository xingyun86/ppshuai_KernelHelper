#pragma once

#ifndef __USUALLYUTILITY_H_
#define __USUALLYUTILITY_H_

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

using namespace std;

typedef std::vector<std::string> STRINGVECTOR;
typedef std::vector<std::wstring> WSTRINGVECTOR;

typedef std::vector<STRINGVECTOR> STRINGVECTORVECTOR;
typedef std::vector<WSTRINGVECTOR> WSTRINGVECTORVECTOR;

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

#if !defined(_UNICODE) && !defined(UNICODE)
#define tstring std::string
#define __MY__TEXT(quote) quote
#else
#define tstring std::wstring
#define __MY__TEXT(quote) L##quote
#endif

#define _MY_TEXT(x)	__MY__TEXT(x)
#define	_MY_T(x)    __MY__TEXT(x)
#define TSTRING     tstring

typedef std::vector<tstring> TSTRINGVECTOR;

typedef std::vector<TSTRINGVECTOR> TSTRINGVECTORVECTOR;

typedef struct tagConfigureDatablock{
    _TCHAR tExchangeCode[2];//-空 1-郑州商品交易所 2-大连商品交易所 3-中国金融期货交易所 4-上海期货交易所 5-其它
    _TCHAR tContractVarietyCode[5];//a
    _TCHAR tStartDate[9];//19880101
    _TCHAR tFinalDate[9];//19880101

}CONFIGUREDATABLOCK, *PCONFIGUREDATABLOCK;

typedef enum{
 TFTYPE_0=0, //YYYYMMDDHHMMSS
 TFTYPE_1=1, //YYYY-MM-DD HH:MM:SS
 TFTYPE_2=2, //YYYYMMDD
 TFTYPE_3=3, //YYYY-MM-DD
 TFTYPE_4=4, //HHMMSS
 TFTYPE_5=5, //HH:MM:SS
 TFTYPE_6=6, //MM-DD HH:MM
} TIME_FORMAT_TYPE;

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define ALT_E          0x01
#define ALT_O          0x02
#define LEGAL_ALT(x)   { if (alt_format & ~(x)) return (0); }
#define TM_YEAR_BASE   (1970)

static int conv_num(const _TCHAR **, int *, int, int);
//static int strncasecmp(char *s1, char *s2, size_t n);

static const _TCHAR *day[] = {
  _MY_T("Sunday"), _MY_T("Monday"), _MY_T("Tuesday"),
  _MY_T("Wednesday"), _MY_T("Thursday"), _MY_T("Friday"),
  _MY_T("Saturday")
};
static const _TCHAR *abday[] = {
  _MY_T("Sun"),_MY_T("Mon"),_MY_T("Tue"),
  _MY_T("Wed"),_MY_T("Thu"),_MY_T("Fri"),
  _MY_T("Sat")
};
static const _TCHAR *mon[] = {
  _MY_T("January"), _MY_T("February"), _MY_T("March"),
  _MY_T("April"), _MY_T("May"), _MY_T("June"),
  _MY_T("July"), _MY_T("August"), _MY_T("September"),
  _MY_T("October"), _MY_T("November"), _MY_T("December")
};
static const _TCHAR *abmon[12] = {
  _MY_T("Jan"), _MY_T("Feb"), _MY_T("Mar"),
  _MY_T("Apr"), _MY_T("May"), _MY_T("Jun"),
  _MY_T("Jul"), _MY_T("Aug"), _MY_T("Sep"),
  _MY_T("Oct"), _MY_T("Nov"), _MY_T("Dec")
};
static const _TCHAR *am_pm[] = {
  _MY_T("AM"), _MY_T("PM")
};

static int conv_num(const _TCHAR **buf, int *dest, int llim, int ulim)
{
 int result = 0;

 /* The limit also determines the number of valid digits. */
 int rulim = ulim;

 if (**buf < _MY_T('0') || **buf > _MY_T('9'))
  return (0);

 do {
  result *= 10;
  result += *(*buf)++ - _MY_T('0');
  rulim /= 10;
 } while ((result * 10 <= ulim) && rulim && **buf >= _MY_T('0') && **buf <= _MY_T('9'));

 if (result < llim || result > ulim)
  return (0);

 *dest = result;
 return (1);
}

__inline static _TCHAR * _tcsptime(const _TCHAR *buf, const _TCHAR *fmt, struct tm *tm)
{
     _TCHAR c;
     const _TCHAR *bp;
     size_t len = 0;
     int alt_format = 0;
  int i = 0;
  int split_year = 0;

     bp = buf;

     while ((c = *fmt) != _MY_T('/0')) {
         /* Clear 'alternate' modifier prior to new conversion. */
         alt_format = 0;

         /* Eat up white-space. */
         if (_istspace(c)) {
    while (_istspace(*bp))
     bp++;

              fmt++;
              continue;
         }

         if ((c = *fmt++) != _MY_T('%'))
              goto literal;


again:        switch (c = *fmt++) {
         case _MY_T('%'): /* "%%" is converted to "%". */
literal:
              if (c != *bp++)
                   return (0);
              break;

         /*
          * "Alternative" modifiers. Just set the appropriate flag
          * and start over again.
          */
         case _MY_T('E'): /* "%E" alternative conversion modifier. */
              LEGAL_ALT(0);
              alt_format |= ALT_E;
              goto again;

         case _MY_T('O'): /* "%O" alternative conversion modifier. */
              LEGAL_ALT(0);
              alt_format |= ALT_O;
              goto again;

         /*
          * "Complex" conversion rules, implemented through recursion.
          */
         case _MY_T('c'): /* Date and time, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = _tcsptime(bp, _MY_T("%x %X"), tm)))
                   return (0);
              break;

         case _MY_T('D'): /* The date as "%m/%d/%y". */
              LEGAL_ALT(0);
              if (!(bp = _tcsptime(bp, _MY_T("%m/%d/%y"), tm)))
                   return (0);
              break;

         case _MY_T('R'): /* The time as "%H:%M". */
              LEGAL_ALT(0);
              if (!(bp = _tcsptime(bp, _MY_T("%H:%M"), tm)))
                   return (0);
              break;

         case _MY_T('r'): /* The time in 12-hour clock representation. */
              LEGAL_ALT(0);
              if (!(bp = _tcsptime(bp, _MY_T("%I:%M:%S %p"), tm)))
                   return (0);
              break;

         case _MY_T('T'): /* The time as "%H:%M:%S". */
              LEGAL_ALT(0);
              if (!(bp = _tcsptime(bp, _MY_T("%H:%M:%S"), tm)))
                   return (0);
              break;

         case _MY_T('X'): /* The time, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = _tcsptime(bp, _MY_T("%H:%M:%S"), tm)))
                   return (0);
              break;

         case _MY_T('x'): /* The date, using the locale's format. */
              LEGAL_ALT(ALT_E);
              if (!(bp = _tcsptime(bp, _MY_T("%m/%d/%y"), tm)))
                   return (0);
              break;

         /*
          * "Elementary" conversion rules.
          */
         case _MY_T('A'): /* The day of week, using the locale's form. */
         case _MY_T('a'):
              LEGAL_ALT(0);
              for (i = 0; i < 7; i++) {
                   /* Full name. */
                   len = _tcslen(day[i]);
                   if (_tcsnicmp((_TCHAR *)(day[i]), (_TCHAR *)bp, len) == 0)
                       break;

                   /* Abbreviated name. */
                   len = _tcslen(abday[i]);
                   if (_tcsnicmp((_TCHAR *)(abday[i]), (_TCHAR *)bp, len) == 0)
                       break;
              }

              /* Nothing matched. */
              if (i == 7)
                   return (0);

              tm->tm_wday = i;
              bp += len;
              break;

         case _MY_T('B'): /* The month, using the locale's form. */
         case _MY_T('b'):
         case _MY_T('h'):
              LEGAL_ALT(0);
              for (i = 0; i < 12; i++) {
                   /* Full name. */
                   len = _tcslen(mon[i]);
                   if (_tcsnicmp((_TCHAR *)(mon[i]), (_TCHAR *)bp, len) == 0)
                       break;

                   /* Abbreviated name. */
                   len = _tcslen(abmon[i]);
                   if (_tcsnicmp((_TCHAR *)(abmon[i]),(_TCHAR *) bp, len) == 0)
                       break;
              }

              /* Nothing matched. */
              if (i == 12)
                   return (0);

              tm->tm_mon = i;
              bp += len;
              break;

         case _MY_T('C'): /* The century number. */
              LEGAL_ALT(ALT_E);
              if (!(conv_num(&bp, &i, 0, 99)))
                   return (0);

              if (split_year) {
                   tm->tm_year = (tm->tm_year % 100) + (i * 100);
              } else {
                   tm->tm_year = i * 100;
                   split_year = 1;
              }
              break;

         case _MY_T('d'): /* The day of month. */
         case _MY_T('e'):
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
                   return (0);
              break;

         case _MY_T('k'): /* The hour (24-hour clock representation). */
              LEGAL_ALT(0);
              /* FALLTHROUGH */
         case _MY_T('H'):
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
                   return (0);
              break;

         case _MY_T('l'): /* The hour (12-hour clock representation). */
              LEGAL_ALT(0);
              /* FALLTHROUGH */
         case _MY_T('I'):
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
                   return (0);
              if (tm->tm_hour == 12)
                   tm->tm_hour = 0;
              break;

         case _MY_T('j'): /* The day of year. */
              LEGAL_ALT(0);
              if (!(conv_num(&bp, &i, 1, 366)))
                   return (0);
              tm->tm_yday = i - 1;
              break;

          case _MY_T('M'): /* The minute. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
                   return (0);
              break;

         case _MY_T('m'): /* The month. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &i, 1, 12)))
                   return (0);
              tm->tm_mon = i - 1;
              break;

    case _MY_T('p'): /* The locale's equivalent of AM/PM. */
      LEGAL_ALT(0);
      /* AM */
      if (_tcscmp(am_pm[0], bp) == 0) {
     if (tm->tm_hour > 11)
      return (0);

     bp += _tcslen(am_pm[0]);
     break;
      }
      /* PM */
      else if (_tcscmp(am_pm[1], bp) == 0) {
     if (tm->tm_hour > 11)
      return (0);

     tm->tm_hour += 12;
     bp += _tcslen(am_pm[1]);
     break;
      }

      /* Nothing matched. */
      return (0);

         case _MY_T('S'): /* The seconds. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
                   return (0);
              break;

         case _MY_T('U'): /* The week of year, beginning on sunday. */
         case _MY_T('W'): /* The week of year, beginning on monday. */
              LEGAL_ALT(ALT_O);
              /*
               * XXX This is bogus, as we can not assume any valid
               * information present in the tm structure at this
               * point to calculate a real value, so just check the
               * range for now.
               */
               if (!(conv_num(&bp, &i, 0, 53)))
                   return (0);
               break;

         case _MY_T('w'): /* The day of week, beginning on sunday. */
              LEGAL_ALT(ALT_O);
              if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
                   return (0);
              break;

         case _MY_T('Y'): /* The year. */
    LEGAL_ALT(ALT_E);
    if (!(conv_num(&bp, &i, 0, 9999)))
     return (0);

    tm->tm_year = i - TM_YEAR_BASE;
    break;

         case _MY_T('y'): /* The year within 100 years of the epoch. */
              LEGAL_ALT(ALT_E | ALT_O);
              if (!(conv_num(&bp, &i, 0, 99)))
                   return (0);

              if (split_year) {
                   tm->tm_year = ((tm->tm_year / 100) * 100) + i;
                   break;
              }
              split_year = 1;
              if (i <= 68)
                   tm->tm_year = i + 2000 - TM_YEAR_BASE;
              else
                   tm->tm_year = i + 1900 - TM_YEAR_BASE;
              break;

         /*
          * Miscellaneous conversions.
          */
         case _MY_T('n'): /* Any kind of white-space. */
         case _MY_T('t'):
              LEGAL_ALT(0);
              while (_istspace(*bp))
                   bp++;
              break;


         default: /* Unknown/unsupported conversion. */
              return (0);
         }
     }

     /* LINTED functional specification */
     return ((_TCHAR *)bp);
}
#define TIME_ZONE_HOUR ((_timezone > 0 ? _timezone : -_timezone) / 3600)//时区相差小时数

__inline static tstring T2S(time_t tt, TIME_FORMAT_TYPE tft)
{
 _TCHAR tTmp[128] = {0};
 struct tm *pTm = localtime(&tt);
 if (pTm)
 {
  switch(tft)
  {
  case TFTYPE_0:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%Y%m%d%H%M%S"), pTm);
   }
   break;
  case TFTYPE_1:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%Y-%m-%d %H:%M:%S"), pTm);
   }
   break;
  case TFTYPE_2:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%Y%m%d"), pTm);
   }
   break;
  case TFTYPE_3:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%Y-%m-%d"), pTm);
   }
   break;
  case TFTYPE_4:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%H%M%S"), pTm);
   }
   break;
  case TFTYPE_5:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%H:%M:%S"), pTm);
   }
   break;
  case TFTYPE_6:
   {
    _tcsftime(tTmp, sizeof(tTmp), _MY_T("%m-%d %H:%M"), pTm);
   }
   break;
  default:
   {

   }
   break;
  }
 }
 return tTmp;
}

__inline static time_t S2T(tstring ts, TIME_FORMAT_TYPE tft)
{
 struct tm tm = {0};
 switch(tft)
 {
 case TFTYPE_0:
  {
   _tcsptime(ts.c_str(), _MY_T("%Y%m%d%H%M%S"), &tm);
  }
  break;
 case TFTYPE_1:
  {
   _tcsptime(ts.c_str(), _MY_T("%Y-%m-%d %H:%M:%S"), &tm);
  }
  break;
 case TFTYPE_2:
  {
   _tcsptime(ts.c_str(), _MY_T("%Y%m%d"), &tm);
  }
  break;
 case TFTYPE_3:
  {
   _tcsptime(ts.c_str(), _MY_T("%Y-%m-%d"), &tm);
  }
  break;
 case TFTYPE_4:
  {
   _tcsptime(ts.c_str(), _MY_T("%H%M%S"), &tm);
   tm.tm_mon += 0;
   tm.tm_mday += 1;
   tm.tm_hour += TIME_ZONE_HOUR;
  }
  break;
 case TFTYPE_5:
  {
   _tcsptime(ts.c_str(), _MY_T("%H:%M:%S"), &tm);
   tm.tm_mon += 0;
   tm.tm_mday += 1;
   tm.tm_hour += TIME_ZONE_HOUR;
  }
  break;
 case TFTYPE_6:
  {
   _tcsptime(ts.c_str(), _MY_T("%m-%d %H:%M"), &tm);
  }
  break;
 default:
  {

  }
  break;
 }
 tm.tm_year += TM_YEAR_BASE - 1900;
 return mktime(&tm);
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

__inline static int code_convert(char *from_charset, char *to_charset, const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	/*const char **pin = &inbuf;
	char **pout = &outbuf;

	iconv_t cd = iconv_open(to_charset, from_charset);
	if (cd == 0) return -1;
	memset(outbuf, 0, outlen);
	if (iconv(cd, (char **)pin, &inlen, pout, &outlen) == -1) return -1;
	iconv_close(cd);*/
	return 0;
}

/* UTF-8 to GBK  */
__inline static int u2g(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("UTF-8", "GBK", inbuf, inlen, outbuf, outlen);
}

/* GBK to UTF-8 */
__inline static int g2u(const char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
	return code_convert("GBK", "UTF-8", inbuf, inlen, outbuf, outlen);
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

//判断目录是否存在
__inline static BOOL IsDirectoryExists(LPCTSTR lpDirectory)
{
	BOOL bResult = TRUE;
	struct _stat st = { 0 };
	if ((_tstat(lpDirectory, &st) != 0) || (st.st_mode & S_IFDIR != S_IFDIR))
	{
		bResult = FALSE;
	}

	return bResult;
}
//判断目录是否存在，若不存在则创建
__inline static BOOL CreateCascadeDirectory(LPCTSTR lpPathName,        //Directory name
	LPSECURITY_ATTRIBUTES lpSecurityAttributes/* = NULL*/  // Security attribute
)
{
	if (IsDirectoryExists(lpPathName))       //如果目录已存在，直接返回
	{
		return TRUE;
	}

	_TCHAR tPathSect[MAX_PATH] = { 0 };
	_TCHAR tPathName[MAX_PATH] = { 0 };
	_tcscpy(tPathName, lpPathName);
	_TCHAR *pToken = _tcstok(tPathName, _T("\\"));
	while (pToken)
	{
		_sntprintf(tPathSect, sizeof(tPathSect) / sizeof(_TCHAR), _T("%s%s\\"), tPathSect, pToken);
		if (!IsDirectoryExists(tPathSect))
		{
			//创建失败时还应删除已创建的上层目录，此次略
			if (!CreateDirectory(tPathSect, lpSecurityAttributes))
			{
				_tprintf(_T("CreateDirectory Failed: %d\n"), GetLastError());
				return FALSE;
			}
		}
		pToken = _tcstok(NULL, _T("\\"));
	}
	return TRUE;
}
#define CMD_PATH_NAME				"cmd.exe" //相对路径名称

#define ADB_PATH_NAME				"adb\\adb.exe" //相对路径名称

//获取cmd.exe文件路径
__inline static tstring GetCmdPath()
{
	return GetSystemPath() + _T(CMD_PATH_NAME);
}

//设定cmd.exe路径
static const tstring CMD_FULL_PATH_NAME = GetCmdPath();

//获取adb.exe文件路径
__inline static tstring GetAdbPath()
{
	return /*GetProgramPath()*/GetWorkPath() + _T(ADB_PATH_NAME);
}

//设定adb.exe路径
static const tstring ADB_FULL_PATH_NAME = GetAdbPath();

#define ANSI2UTF8(x) UnicodeToUTF8(ANSIToUnicode(x))
#define UTF82ANSI(x) UnicodeToANSI(UTF8ToUnicode(x))

#endif //__USUALLYUTILITY_H_
