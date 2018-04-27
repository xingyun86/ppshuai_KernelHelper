#pragma once

#ifndef __PIPEHELPER_H_
#define __PIPEHELPER_H_

#include "CommonHelper.h"

#define MAX_READ_TIMEOUT	WAIT_TIMEOUT	//默认300毫秒超时
#define BUFFSIZE			0xFFFF	        //默认缓冲区大小

class CPipeHelper
{
public:
	CPipeHelper();
	~CPipeHelper();

private:
	HANDLE m_hScriptFile;//脚本句柄

	HANDLE m_hStdInReader;//标准读输入流
	HANDLE m_hStdInWriter;//标准写输入流
	HANDLE m_hStdOutReader;//标准读输出流
	HANDLE m_hStdOutWriter;//标准读输出流
	DWORD m_dwProcessId;//进程标识

	DWORD m_dwReadTimeout;//读取超时时间,单位为毫秒

    /////////////////////////////////////////////////////////////////////////////////////////////////
    //函数功能:创建子应用进程
    //函数参数:
    //		tsAppProgName	应用程序名称
    //		tsArguments 	程序运行参数
    //      bNoUI           是否显示界面
    //      type            启动类型
    //      dwWaitTime      程序启动等待时间
	//返回值:
	//	0,		成功
	//	(-1),	创建进程失败
	int CreateChildProcess(tstring tsAppProgName, tstring tsArguments = _T(""), LPCTSTR lpWorkPath = NULL, bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:读取脚本命令执行结果
	//函数参数:
	//返回值:
	void ReadFromPipe(void);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:脚本写入命令
	//函数参数:
	//返回值:
	void WriteToPipe(void);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:写入命令并取得执行结果
	//函数参数:
	//		tsInputCommand	要写入的命令(内容处理会加入'\r\n')
	//返回值:
	//		失败返回空,成功返回执行结果
	tstring WritePipeAndReadPipe(tstring tsInputCommand);

public:
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:初始化管道操作类
	//函数参数:
	//		tsAppProg			应用程序名称
	//		tsArugments			程序运行参数
	//		tsScriptFileName	脚本文件路径名
    //      bNoUI               是否显示界面
    //      type                启动类型
    //      dwWaitTime          程序启动等待时间
	//返回值:
	//	0,		成功
	//	(-1),	打开脚本文件失败
	int Initialize(tstring tsAppProgName, tstring tsArugments, tstring tsScriptFileName = _T(""), bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:释放掉管道操作类
	//函数参数:
	//返回值:
	void Exitialize();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:初始化管道脚本
	//函数参数:
	//		tsScriptName	脚本文件路径名
	//返回值:
	//	0,		成功
	//	(-1),	打开脚本文件失败
	int InitScript(tstring tsScriptFileName);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:设置管道读超时时间(设置即时生效)
	//函数参数:
	//		dwReadTimeout	超时时间,单位为毫秒
	//返回值:
	void SetReadTimeout(DWORD dwReadTimeout) { this->m_dwReadTimeout = dwReadTimeout; }

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:读取脚本命令执行结果
	//函数参数:
	//返回值:
	void Read();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:脚本写入命令
	//函数参数:
	//返回值:
	void Write();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:读取初始化执行结果
	//函数参数:
	//		无
	//返回值:
	//		失败返回空,成功返回执行结果
	tstring ReadPipeFromInit();

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:写入命令并取得执行结果
	//函数参数:
	//		tsInputCommand	要写入的命令(内容处理会加入'\r\n')
	//返回值:
	//		失败返回空,成功返回执行结果
	tstring WriteAndRead(tstring tsInputCommand);

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能:静态执行命令并取得执行结果
	//函数参数:
	//		tsAppProg			应用程序名称
	//		tsCmdLine			要写入的命令(内容处理会加入'\r\n')
    //      dwReadTimeout       读取输出轮询等待事件
    //      bNoUI               是否显示界面
    //      type                启动类型
    //      dwWaitTime          程序启动等待时间
	//返回值:
	//		失败返回"_ERROR_FAILURE_",成功返回执行结果
	static tstring RunCmd(tstring tsAppProgName, tstring tsArguments, DWORD dwReadTimeout = MAX_READ_TIMEOUT, bool bNoUI = true, LAUNCHTYPE type = LTYPE_0, DWORD dwWaitTime = WAIT_TIMEOUT);
};

#endif //__PIPEHELPER_H_
