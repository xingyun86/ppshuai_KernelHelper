// PipeHelper.cpp : Defines the entry point for the console application.
//

#include "PipeHelper.h"

CPipeHelper::CPipeHelper()
{
	m_hScriptFile = NULL;

	m_hStdInReader = NULL;
	m_hStdInWriter = NULL;
	m_hStdOutReader = NULL;
	m_hStdOutWriter = NULL;
	m_dwProcessId = 0;

	m_dwReadTimeout = MAX_READ_TIMEOUT;//默认300毫秒
}

CPipeHelper::~CPipeHelper()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:初始化管道操作类
//函数参数:
//		tsAppProgName		应用程序名称
//		tsArguments			程序运行参数
//		tsScriptFileName	脚本文件路径名
//      bNoUI               是否显示界面
//      type                启动类型
//      dwWaitTime          程序启动等待时间
//返回值:
//	0,		成功
//	(-1),	打开脚本文件失败
int CPipeHelper::Initialize(tstring tsAppProgName, tstring tsArguments, tstring tsScriptFileName/* = _T("")*/, bool bNoUI/* = true*/, LAUNCHTYPE type/* = LTYPE_0*/, DWORD dwWaitTime/* = WAIT_TIMEOUT*/)
{
	int result = 0;

	SECURITY_ATTRIBUTES saAttr = {0};

	//DEBUG_TRACE(_T("\n->Start of parent execution.\n"));

	// Set the bInheritHandle flag so pipe handles are inherited.
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT.
	if (!CreatePipe(&m_hStdOutReader, &m_hStdOutWriter, &saAttr, 0))
	{
		//DEBUG_TRACE(_T("StdoutRd CreatePipe\r\n"));
		//ErrorExit(TEXT("StdoutRd CreatePipe"));
		return (-1);
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if (!SetHandleInformation(m_hStdOutReader, HANDLE_FLAG_INHERIT, 0))
	{
		//DEBUG_TRACE(_T("Stdout SetHandleInformation\r\n"));
		//ErrorExit(TEXT("Stdout SetHandleInformation"));
		return (-2);
	}
	// Create a pipe for the child process's STDIN.
	if (!CreatePipe(&m_hStdInReader, &m_hStdInWriter, &saAttr, 0))
	{
		//DEBUG_TRACE(_T("Stdin CreatePipe\r\n"));
		//ErrorExit(TEXT("Stdin CreatePipe"));
		return (-3);
	}

	// Ensure the write handle to the pipe for STDIN is not inherited.
	if (!SetHandleInformation(m_hStdInWriter, HANDLE_FLAG_INHERIT, 0))
	{
		//DEBUG_TRACE(_T("Stdin SetHandleInformation\r\n"));
		//ErrorExit(TEXT("Stdin SetHandleInformation"));
		return (-4);
	}

	// Create the child process.
	result = CreateChildProcess(tsAppProgName, tsArguments, NULL, bNoUI, type, dwWaitTime);
	if (result != 0)
	{
		//DEBUG_TRACE(_T("CreateChildProcess Failed\r\n"));
		return (-5);
	}

	if (tsScriptFileName.length())
	{
		result = InitScript(tsScriptFileName);
	}

	return result;
}

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
int CPipeHelper::CreateChildProcess(tstring tsAppProgName, tstring tsArguments/* = _T("")*/, LPCTSTR lpWorkPath/* = NULL*/, bool bNoUI/* = true*/, LAUNCHTYPE type/* = LTYPE_0*/, DWORD dwWaitTime/* = WAIT_TIMEOUT*/)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
    int result = (-1);
	BOOL bResult = FALSE;
	//TCHAR szAppProg[] = TEXT("D:\\android-eclipse\\android-sdk-windows\\platform-tools\\adb.exe");
	//TCHAR szCmdline[] = TEXT(" shell");
	LPTSTR lpAppProg = NULL;
	LPTSTR lpCmdLine = NULL;

	STARTUPINFO siStartInfo = { 0 };
	PROCESS_INFORMATION piProcInfo = { 0 };
	DWORD dwCreateFlags = CREATE_NO_WINDOW;

	if (tsAppProgName.length())
	{
		lpAppProg = (LPTSTR)tsAppProgName.c_str();
	}
	if (tsArguments.length())
	{
		lpCmdLine = (LPTSTR)tsArguments.c_str();
	}

	if (!bNoUI)
	{
		dwCreateFlags = 0;
	}

	// Set up members of the PROCESS_INFORMATION structure.
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure.
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = m_hStdOutWriter;
	siStartInfo.hStdOutput = m_hStdOutWriter;
	siStartInfo.hStdInput = m_hStdInReader;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	siStartInfo.dwFlags |= STARTF_USESHOWWINDOW;	// hide window flag
	siStartInfo.wShowWindow = SW_HIDE;				// hide window

	// Create the child process.
	bResult = CreateProcess(lpAppProg,
		lpCmdLine,     // command line
		NULL,          // process security attributes
		NULL,          // primary thread security attributes
		TRUE,          // handles are inherited
		dwCreateFlags, // creation flags
		NULL,          // use parent's environment
		lpWorkPath,    // use parent's current directory
		&siStartInfo,  // STARTUPINFO pointer
		&piProcInfo);  // receives PROCESS_INFORMATION
					   // If an error occurs, exit the application.
	if (bResult)
	{
		// Record process id
		m_dwProcessId = piProcInfo.dwProcessId;

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
			WaitForSingleObject(piProcInfo.hProcess, INFINITE);
		}
		break;
		case LTYPE_2:
		{
			// Wait until child process exits.
			WaitForSingleObject(piProcInfo.hProcess, dwWaitTime);
		}
		break;
		default:
			break;
		}

		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example.

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);

		//子进程执行成功
		result = 0;
	}
	else
	{
        //DEBUG_TRACE(_T("CreateProcess failed (%d).\n"), GetLastError());
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:读取脚本命令执行结果
//函数参数:
//返回值:
void CPipeHelper::ReadFromPipe(void)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT.
// Stop when there is no more data.
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(m_hStdOutReader, chBuf, BUFFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(hParentStdOut, chBuf,
			dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:脚本写入命令
//函数参数:
//返回值:
void CPipeHelper::WriteToPipe(void)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFFSIZE];
	BOOL bSuccess = FALSE;

	for (;;)
	{
		bSuccess = ReadFile(m_hScriptFile, chBuf, BUFFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		bSuccess = WriteFile(m_hStdInWriter, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess) break;
	}

	/*
	// Close the pipe handle so the child process stops reading.
	if (!CloseHandle(m_hChildStd_IN_Wr))
	{
		printf("StdInWr CloseHandle\r\n");
		//ErrorExit(TEXT("StdInWr CloseHandle"));
	}
	*/
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:写入命令并取得执行结果
//函数参数:
//		tsInputCommand	要写入的命令(内容处理会加入'\r\n')
//返回值:
//		失败返回空,成功返回执行结果
tstring CPipeHelper::WritePipeAndReadPipe(tstring tsInputCommand)
// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
	BOOL bSuccess = FALSE;
	bool bHadReceived = false;
	DWORD dwRead = 0;
	DWORD dwWritten = 0;
	DWORD dwTotalBytesAvail = 0;
	DWORD dwBytesLeftThisMessage = 0;
	CHAR chBuf[BUFFSIZE] = { 0 };
	std::string strOutput = "";
	std::string strInputCommand = PPSHUAI::Convert::TToA(tsInputCommand);
	strInputCommand.append("\r\n");
	bSuccess = WriteFile(m_hStdInWriter, strInputCommand.c_str(), strInputCommand.length(), &dwWritten, NULL);
	if (!bSuccess)
	{
		return _T("");
	}

	for (;;)
	{
		bSuccess = PeekNamedPipe(m_hStdOutReader, chBuf, 1, &dwRead, &dwTotalBytesAvail, &dwBytesLeftThisMessage);
		if (!bSuccess || dwRead == 0)
		{
			if(!bHadReceived)
			{
				Sleep(m_dwReadTimeout);
				bHadReceived = true;
				continue;
			}
			else
			{
				break;
			}
		}
		if (!bHadReceived)
		{
			bHadReceived = true;
		}
		bSuccess = ReadFile(m_hStdOutReader, chBuf, BUFFSIZE, &dwRead, NULL);//无数据时会阻塞，不采用
		if (!bSuccess || dwRead == 0) break;
		strOutput.append(chBuf);
	}

	if (strOutput.length() <= 0)
	{
		return _T("");
	}

	return PPSHUAI::Convert::AToT(strOutput);
}



/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:释放掉管道操作类
//函数参数:
//返回值:
void CPipeHelper::Exitialize()
{
	// Close the script file handle so the child process stops reading.
	if (m_hScriptFile)
	{
		CloseHandle(m_hScriptFile);
		m_hScriptFile = NULL;
	}

	// Close the pipe handle so the child process stops reading.
	if (m_hStdInReader)
	{
		CloseHandle(m_hStdInReader);
		m_hStdInReader = NULL;
	}
	if (m_hStdInWriter)
	{
		CloseHandle(m_hStdInWriter);
		m_hStdInWriter = NULL;
	}
	if (m_hStdOutReader)
	{
		CloseHandle(m_hStdOutReader);
		m_hStdOutReader = NULL;
	}
	if (m_hStdOutWriter)
	{
		CloseHandle(m_hStdOutWriter);
		m_hStdOutWriter = NULL;
	}

	// Exit process.
	PPSHUAI::SystemKernel::TerminateProcessByProcessId(m_dwProcessId);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:初始化脚本内容读取
//函数参数:
//		tsScriptName	脚本文件路径名
//返回值:
//	0,		成功
//	(-1),	打开脚本文件失败
int CPipeHelper::InitScript(tstring tsScriptName)
{
	int result = 0;

	// Get a handle to an input file for the parent.
	// This example assumes a plain text file and uses string output to verify data flow.
	m_hScriptFile = CreateFile(
		tsScriptName.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);

	if (m_hScriptFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile Failed!\r\n");
		//ErrorExit(TEXT("CreateFile"));
		return (-1);
	}

	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:读取脚本命令执行结果
//函数参数:
//返回值:
void CPipeHelper::Read()
{
	ReadFromPipe();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:脚本写入命令
//函数参数:
//返回值:
void CPipeHelper::Write()
{
	WriteToPipe();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:读取初始化执行结果
//函数参数:
//		无
//返回值:
//		失败返回空,成功返回执行结果
tstring CPipeHelper::ReadPipeFromInit()
// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data.
{
	BOOL bSuccess = FALSE;
	bool bHadReceived = false;
	DWORD dwRead = 0;
	DWORD dwTotalBytesAvail = 0;
	DWORD dwBytesLeftThisMessage = 0;
	CHAR chBuf[BUFFSIZE] = { 0 };
	std::string strOutput = "";

	for (;;)
	{
		Sleep(100);
		bSuccess = PeekNamedPipe(m_hStdOutReader, chBuf, 1, &dwRead, &dwTotalBytesAvail, &dwBytesLeftThisMessage);
		if (!bSuccess || dwRead == 0)
		{
			if (!bHadReceived)
			{
				Sleep(m_dwReadTimeout);
				bHadReceived = true;
				continue;
			}
			else
			{
				break;
			}
		}
#if 0
		if (!bHadReceived)
		{
			bHadReceived = true;
		}
#endif
		bSuccess = ReadFile(m_hStdOutReader, chBuf, BUFFSIZE, &dwRead, NULL);//无数据时会阻塞，不采用
		if (!bSuccess || dwRead == 0) break;
		strOutput.append(chBuf);
		memset(chBuf, 0, BUFFSIZE);
	}

	if (strOutput.length() <= 0)
	{
		return _T("");
	}

	return PPSHUAI::Convert::AToT(strOutput);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//函数功能:写入命令并取得执行结果
//函数参数:
//		tsInputCommand	要写入的命令(内容处理会加入'\r\n')
//返回值:
//		失败返回空,成功返回执行结果
tstring CPipeHelper::WriteAndRead(tstring tsInputCommand)
{
	return WritePipeAndReadPipe(tsInputCommand);
}

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
//		失败返回"__ERROR_WRONG__",成功返回执行结果
tstring CPipeHelper::RunCmd(tstring tsAppProgName, tstring tsArguments, DWORD dwReadTimeout/* = MAX_READ_TIMEOUT*/, bool bNoUI/* = true*/, LAUNCHTYPE type/* = LTYPE_0*/, DWORD dwWaitTime/* = WAIT_TIMEOUT*/)
{
	CPipeHelper pipeHelper;
	tstring tsResult = _T("__ERROR_WRONG__");//初始化为错误字符串

	if (!pipeHelper.Initialize(tsAppProgName, tsArguments, _T(""), bNoUI, type, dwWaitTime))
	{
		pipeHelper.SetReadTimeout(dwReadTimeout);
		tsResult = pipeHelper.ReadPipeFromInit();
	}
	pipeHelper.Exitialize();

	return tsResult;
}
