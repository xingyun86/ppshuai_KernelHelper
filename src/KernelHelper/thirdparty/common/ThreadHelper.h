#pragma once

#ifndef __THREADHELPER_H_
#define __THREADHELPER_H_

#include <iostream>
#include <windows.h>
#include <assert.h>
//#define DEBUG_SMARTPTR

template <typename T>
class CAtomicRefPtr;
template<typename T>
class CAtomicSmartPtr;

template <typename T>
class CAtomicRefPtr
{
	friend class CAtomicSmartPtr<T>;
	explicit CAtomicRefPtr(T * _ptr) :ptr(_ptr), cnt(0)
	{
		assert(ptr);
#ifdef DEBUG_SMARTPTR
		std::cout << "Create Pointer!" << std::endl;
#endif
	}

	CAtomicRefPtr(const CAtomicRefPtr&)
	{

	}

	CAtomicRefPtr& operator= (const CAtomicRefPtr & ref)
	{

	}

	~CAtomicRefPtr()
	{
#ifdef DEBUG_SMARTPTR
		std::cout << "Delete Pointer!" << std::endl;
#endif
		assert(ptr);
		if (ptr != NULL)
		{
			delete ptr;
			ptr = NULL;
		}
	}

	unsigned int IncreaseRefCounter()
	{
		return InterlockedIncrement((unsigned int volatile *)&cnt);
	}

	unsigned int DecreaseRefCounter()
	{
		return InterlockedDecrement((unsigned int volatile *)&cnt);
	}

	bool IncreaseRefCounter_Lock()
	{
		bool result = false;
		while (1L)
		{			
			if (!cnt)
			{
				goto __LEAVE_CLEAN__;
			}
			else
			{
				unsigned int tmp = cnt;
				if (InterlockedCompareExchange((unsigned int *)&cnt, tmp + 1, tmp) == tmp)
				{
					result = true;
					goto __LEAVE_CLEAN__;
				}
			}
		}

	__LEAVE_CLEAN__:

		return result;
	}

	T *ptr;
	unsigned int volatile cnt;
};

template<typename T>
class CAtomicSmartPtr
{
public:
	explicit CAtomicSmartPtr(T *_ptr) : ptr(new CAtomicRefPtr<T>(_ptr))
	{
		assert(_ptr);
#ifdef DEBUG_SMARTPTR
		std::cout << "Create SmartPointer!" << std::endl;
#endif
		ptr->IncreaseRefCounter();
	}

	explicit CAtomicSmartPtr(const CAtomicSmartPtr<T>& sptr) : ptr(sptr.ptr)
	{
#ifdef DEBUG_SMARTPTR
		std::cout << "Copy0 SmartPointer!" << std::endl;
#endif
		ptr->DecreaseRefCounter();
	}

	explicit CAtomicSmartPtr(const CAtomicSmartPtr<T>* sptr) : ptr(sptr->ptr)
	{
#ifdef DEBUG_SMARTPTR
		std::cout << "Copy1 SmartPointer!" << std::endl;
#endif
		ptr->IncreaseRefCount();
	}

	CAtomicSmartPtr& operator=(const CAtomicSmartPtr<T>& sptr)
	{
		if (sptr.ptr != ptr)
		{
			//注意先加后减，防止指向同对象析构的问题
			if (sptr.ptr->IncreaseRefCounter_Lock())
			{
				if (!ptr->DecreaseRefCounter())
				{
					delete ptr;
					ptr = NULL;
				}
				ptr = sptr.ptr;
			}
		}
#ifdef DEBUG_SMARTPTR
		std::cout << "Copy2 SmartPointer!" << std::endl;
#endif
		return *this;
	}

	T* operator->()
	{
		return GetPtr();
	}

	T* operator->() const
	{
		return GetPtr();
	}

	T& operator*()
	{
		return *ptr->ptr;
	}

	T& operator*() const
	{
		return *ptr->ptr;
	}

	bool operator!()
	{
		return !ptr;
	}

	~CAtomicSmartPtr()
	{
		if (!ptr->DecreaseRefCounter())
		{
			delete ptr;
			ptr = NULL;
		}
#ifdef DEBUG_SMARTPTR
		std::cout << "Delete SmartPointer!" << std::endl;
#endif
	}

	int GetRefCounter() const
	{
		return ptr->counter;
	}

	bool IsNull()
	{
		return (ptr->ptr != NULL) ? false : true;
	}

	T* GetPtr() const
	{
		assert(ptr->ptr);
		return ptr->ptr;
	}

	//返回对象
	T GetData() const
	{
		assert(ptr->ptr);
		return *ptr->ptr;
	}

private:
	CAtomicRefPtr<T> *ptr;
};

//兼容const比較
template<typename T>
inline bool operator==(const CAtomicSmartPtr<T>& a, const CAtomicSmartPtr<T>& b)
{
	return a.GetPtr() == b.GetPtr();
}
template<typename T>
inline bool operator!=(const CAtomicSmartPtr<T>& a, const CAtomicSmartPtr<T>& b)
{
	return a.GetPtr() != b.GetPtr();
}

class CThreadHelper{
public:

	enum THREADSTATUS_TYPE{
		TSTYPE_MINIMUM = (0L),
		TSTYPE_NULLPTR = (0L),
		TSTYPE_STARTED = (1L),
		TSTYPE_SUSPEND = (2L),
		TSTYPE_RUNNING = (4L),
		TSTYPE_STOPPED = (8L),
		TSTYPE_MAXIMUM = TSTYPE_NULLPTR | TSTYPE_STARTED | TSTYPE_SUSPEND | TSTYPE_RUNNING | TSTYPE_STOPPED,
	};
	enum THREADEXITCALL_TYPE{
		TECTYPE_SSYNCHRONOUS = (0L),
		TECTYPE_ASYNCHRONOUS = (1L),
	};
	
	CThreadHelper(
		LPTHREAD_START_ROUTINE fnThreadStartRoutine = (0L),
		LPVOID lpThreadParameters = (0L),
		DWORD dwThreadCreationFlags = (0L),
		SIZE_T dwThreadStackSize = (0L),
		LPSECURITY_ATTRIBUTES lpThreadSecurityAttributes = (0L),
		THREADEXITCALL_TYPE nThreadExitCall = TECTYPE_SSYNCHRONOUS
		)
	{
		Startup(fnThreadStartRoutine, lpThreadParameters, dwThreadCreationFlags, dwThreadStackSize, lpThreadSecurityAttributes, nThreadExitCall);
	}

	~CThreadHelper()
	{
		Cleanup();
	}
	static DWORD WINAPI ThreadStartRoutine(LPVOID _lpThreadParameters)
	{
		DWORD dwResult = 0L;
		LPTHREAD_START_ROUTINE fnThreadStartRoutine = NULL;
		CThreadHelper * pTH = (CThreadHelper *)_lpThreadParameters;
		if (pTH)
		{
			while (pTH->GetThreadStartRoutine())
			{
				switch (pTH->GetThreadStatus())
				{
				case TSTYPE_RUNNING:
				{
					pTH->GetThreadStartRoutine()(pTH);
				}
				break;
				default:
				{
					goto __LEAVE_CLEAN__;
				}
				break;
				}
			}

		__LEAVE_CLEAN__:

			::PostThreadMessage(pTH->GetMsgThreadId(), pTH->GetThreadExitCode(), (WPARAM)(0L), (LPARAM)(0L));
		}

		return dwResult;
	}

	VOID Start()
	{
		if (m_hThread && m_dwThreadId && m_dwThreadExitCode)
		{
			SetThreadStatus(TSTYPE_STARTED);
			Resumed();
		}
		else
		{
			Cleanup();
		}
	}
	VOID Suspend()
	{
		::SuspendThread(m_hThread);
		SetThreadStatus(TSTYPE_SUSPEND);
	}
	VOID Resumed()
	{
		::ResumeThread(m_hThread);
		SetThreadStatus(TSTYPE_RUNNING);
	}
	VOID Stopped()
	{
		Clear();
	}

	VOID SSyncWait()
	{
		MSG msg = { 0 }; while (::GetMessage(&msg, 0, 0, 0) && (msg.message != m_dwThreadExitCode)){/*::TranslateMessage(&msg);::DispatchMessage(&msg);*/ }; 
	}

	VOID ASyncWait()
	{
		MSG msg = { 0 };while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) || (msg.message != m_dwThreadExitCode)){/*::TranslateMessage(&msg);::DispatchMessage(&msg);*/}; 
	}
	VOID Clear()
	{
		if (m_hThread != (0L))
		{
			CloseHandle(m_hThread);
			m_hThread = (0L);
		}
		if (GetThreadStatus() == TSTYPE_RUNNING)
		{
			m_dwMsgThreadId = ::GetCurrentThreadId();
			SetThreadStatus(TSTYPE_STOPPED);
			if (m_dwThreadId != m_dwMsgThreadId)
			{
				if (m_dwThreadExitCode >= (WM_USER + WM_QUIT))
				{
					switch (m_nThreadExitCall)
					{
					case CThreadHelper::TECTYPE_SSYNCHRONOUS:
						SSyncWait();
						break;
					case CThreadHelper::TECTYPE_ASYNCHRONOUS:
						ASyncWait();
						break;
					default:
						break;
					}
				}
			}
		}
	}
	VOID Close()
	{
		Cleanup();
	}
	VOID Reset()
	{
		memset(&m_ThreadSecurityAttributes, 0, sizeof(m_ThreadSecurityAttributes));
		m_dwThreadStackSize = (0L);
		m_fnThreadStartRoutine = (0L);
		m_lpThreadParameters = (0L);
		m_dwThreadCreationFlags = (0L);
		m_dwThreadId = (0L);
		m_hThread = (0L);
		m_dwThreadExitCode = (0L);
		m_nThreadStatus = TSTYPE_NULLPTR;
		m_nThreadExitCall = TECTYPE_SSYNCHRONOUS;		
	}
	VOID Startup(
		LPTHREAD_START_ROUTINE fnThreadStartRoutine = (0L),
		LPVOID lpThreadParameters = (0L),
		DWORD dwThreadCreationFlags = (0L),
		SIZE_T dwThreadStackSize = (0L),
		LPSECURITY_ATTRIBUTES lpThreadSecurityAttributes = (0L),
		THREADEXITCALL_TYPE nThreadExitCall = TECTYPE_SSYNCHRONOUS
		)
	{
				
		if (lpThreadSecurityAttributes != (0L))
		{
			memcpy(&m_ThreadSecurityAttributes, lpThreadSecurityAttributes, sizeof(m_ThreadSecurityAttributes));
		}
		else
		{
			memset(&m_ThreadSecurityAttributes, 0, sizeof(m_ThreadSecurityAttributes));
		}
		
		m_hThread = (0L);
		
		m_dwThreadStackSize = dwThreadStackSize;
		m_fnThreadStartRoutine = fnThreadStartRoutine;
		m_lpThreadParameters = lpThreadParameters;
		m_dwThreadCreationFlags = dwThreadCreationFlags | CREATE_SUSPENDED;
		m_dwThreadId = (0L);

		m_dwMsgThreadId = (0L);
		m_dwThreadExitCode = (0L);
		m_nThreadStatus = TSTYPE_NULLPTR;
		m_nThreadExitCall = nThreadExitCall;

		m_hThread = CreateThread(&m_ThreadSecurityAttributes, m_dwThreadStackSize, &CThreadHelper::ThreadStartRoutine, this, m_dwThreadCreationFlags, &m_dwThreadId);
		if (m_hThread && m_dwThreadId)
		{
			m_dwThreadExitCode = (WM_USER + WM_QUIT + m_dwThreadId);
		}
	}
	VOID Cleanup()
	{
		Clear();
		Reset();
	}

	VOID SetThreadStatus(THREADSTATUS_TYPE nThreadStatus = TSTYPE_NULLPTR)
	{
		m_nThreadStatus = (THREADSTATUS_TYPE)(nThreadStatus | TSTYPE_MINIMUM);
	}

	THREADSTATUS_TYPE GetThreadStatus()
	{
		return (THREADSTATUS_TYPE)(m_nThreadStatus & TSTYPE_MAXIMUM);
	}
	
	VOID SetThreadExitCall(THREADEXITCALL_TYPE nThreadExitCall= TECTYPE_SSYNCHRONOUS)
	{
		m_nThreadExitCall = nThreadExitCall;
	}

	THREADEXITCALL_TYPE GetThreadExitCall()
	{
		return m_nThreadExitCall;
	}

	VOID SetThreadPriority(INT nPriority)
	{
		::SetThreadPriority(m_hThread, nPriority);
	}

	INT GetThreadPriority()
	{
		return ::GetThreadPriority(m_hThread);
	}
	BOOL IsThreadRunning()
	{
		return ((GetThreadStatus() & TSTYPE_RUNNING) == TSTYPE_RUNNING);
	}

	LPTHREAD_START_ROUTINE GetThreadStartRoutine()
	{
		return m_fnThreadStartRoutine;
	}
	LPVOID GetThreadParameters()
	{
		return m_lpThreadParameters;
	}

	DWORD GetThreadId()
	{
		return m_dwThreadId;
	}
	DWORD GetThreadCreationFlags()
	{
		return m_dwThreadCreationFlags;
	}
	SIZE_T GetThreadStackSize()
	{
		return m_dwThreadStackSize;
	}
	LPSECURITY_ATTRIBUTES GetThreadSecurityAttributes()
	{
		return &m_ThreadSecurityAttributes;
	}
	DWORD GetMsgThreadId()
	{
		return m_dwMsgThreadId;
	}
	DWORD GetThreadExitCode()
	{
		return m_dwThreadExitCode;
	}
	
private:

	HANDLE m_hThread;	
	DWORD m_dwMsgThreadId;
	DWORD m_dwThreadExitCode;
	THREADSTATUS_TYPE m_nThreadStatus;
	THREADEXITCALL_TYPE m_nThreadExitCall;

	LPTHREAD_START_ROUTINE m_fnThreadStartRoutine;
	LPVOID m_lpThreadParameters;
	DWORD m_dwThreadCreationFlags;
	SIZE_T m_dwThreadStackSize;
	SECURITY_ATTRIBUTES m_ThreadSecurityAttributes;
	DWORD m_dwThreadId;
};

DWORD WINAPI TestRun(LPVOID lpParameter)
{
	DWORD dwResult = 0L;

	printf("This is test run!\r\n");

	return dwResult;
}

__inline static void test()
{
	CAtomicSmartPtr<CThreadHelper> pThreadHelper(new CThreadHelper(TestRun));
	pThreadHelper->Start();

	int i = 0;
	while (1)
	{
		Sleep(1000);
		if (i++ > 100)
		{
			break;
		}
	}
	pThreadHelper->Close();
	
	return;
}

DWORD WINAPI ThreadSample_1(LPVOID lpParams)
{
	DWORD dwResult = (0L);
	CThreadHelper * pTh = (CThreadHelper *)lpParams;
	if (pTh)
	{
		while (pTh->IsThreadRunning())
		{
			if (dwResult++ > 20)
			{
				break;
			}
		}
	}

	Sleep(1000);

	return dwResult;
}

DWORD WINAPI ThreadSample_2(LPVOID lpParams)
{
	DWORD dwResult = (0L);
	CThreadHelper * pTh = (CThreadHelper *)lpParams;
	CThreadHelper * pThTT = (CThreadHelper *)pTh->GetThreadParameters();
	if (pTh && pThTT && pThTT->GetThreadStatus() == CThreadHelper::TSTYPE_RUNNING)
	{
		Sleep(1000);
		pThTT->Suspend();
		Sleep(2000);

		pThTT->Resumed();
		Sleep(3000);

		pThTT->Suspend();
		Sleep(4000);

		pThTT->Resumed();
		Sleep(5000);

		pThTT->Stopped();

		//pTh->Stopped();
	}

	Sleep(1000);

	return dwResult;
}


DWORD WINAPI ThreadSample_3(LPVOID lpParams)
{
	DWORD dwResult = (0L);
	CThreadHelper * pTh = (CThreadHelper *)lpParams;
	if (pTh)
	{
		while (pTh->IsThreadRunning())
		{
			if (dwResult++ > (DWORD)pTh->GetThreadParameters())
			{
				break;
			}
			printf("tid=%ld--%ld\r\n", pTh->GetThreadId(), dwResult);
			Sleep(1000);
		}

		pTh->Close();
	}

	Sleep(100);

	return dwResult;
}
#include <map>
static std::map<DWORD, std::string> G_SSMap;
static std::map<std::string, CThreadHelper*> G_STMap;

DWORD WINAPI PrintStatusThread(LPVOID lpParams)
{
	DWORD dwResult = (0L);
	CThreadHelper * pTh = (CThreadHelper *)lpParams;
	if (pTh)
	{
		DWORD dwLeft = 0;
		std::map<DWORD, std::string> * pDS = &G_SSMap;
		std::map<std::string, CThreadHelper*> * pST = &G_STMap;
		if (pST && pDS)
		{
			for (auto it = pST->begin(); it != pST->end(); it++)
			{
				//printf("\ttid=%s(%08d) status=%s(%08d)\r\n", \
									it.first.c_str(), it.second->GetThreadId(), \
									pDS->at(it.second->GetThreadStatus()).c_str(), \
									it.second->GetThreadStatus());
				if (!it->second->IsThreadRunning())
				{
					printf("\t%s已退出!\r\n", it->first.c_str());
				}
				else
				{
					dwLeft++;
				}
			}
			printf("状态线程(%08d)-线程个数=%ld-活动线程个数=%ld!\r\n", GetCurrentThreadId(), pST->size(), dwLeft);
			printf("=============================================================================\r\n");
		}
		else
		{
			pTh->Stopped();
		}
	}

	Sleep(100);

	return dwResult;
}
int ThreadTestSample()
{
	int nResult = (0L);
	size_t stNumber = 20;
	CThreadHelper * pTH = NULL;
	CThreadHelper th(PrintStatusThread);
	G_SSMap.insert(std::map<DWORD, std::string>::value_type(CThreadHelper::TSTYPE_NULLPTR, "未启动"));
	G_SSMap.insert(std::map<DWORD, std::string>::value_type(CThreadHelper::TSTYPE_STARTED, "已启动"));
	G_SSMap.insert(std::map<DWORD, std::string>::value_type(CThreadHelper::TSTYPE_SUSPEND, "已挂起"));
	G_SSMap.insert(std::map<DWORD, std::string>::value_type(CThreadHelper::TSTYPE_RUNNING, "运行中"));
	G_SSMap.insert(std::map<DWORD, std::string>::value_type(CThreadHelper::TSTYPE_STOPPED, "已停止"));

	th.Start();

	for (size_t i = 0; i < stNumber; i++)
	{
		char ch[128] = { 0 };
		pTH = new CThreadHelper(ThreadSample_1);
		sprintf(ch, "子线程%03d(%ld)", i, pTH->GetThreadId());
		G_STMap.insert(std::map<std::string, CThreadHelper*>::value_type(ch, pTH));
	}
	auto it = G_STMap.begin();
	for (size_t i = stNumber; i < stNumber * 2; i++, it++)
	{
		char ch[128] = { 0 };
		pTH = new CThreadHelper(ThreadSample_2, it->second);
		sprintf(ch, "子线程%03d(%ld)", i, pTH->GetThreadId());
		G_STMap.insert(std::map<std::string, CThreadHelper*>::value_type(ch, pTH));
	}

	for (size_t i = stNumber * 2; i < stNumber * 3; i++)
	{
		char ch[128] = { 0 };
		pTH = new CThreadHelper(ThreadSample_3, (LPVOID)(i));
		sprintf(ch, "子线程%03d(%ld)", i, pTH->GetThreadId());
		G_STMap.insert(std::map<std::string, CThreadHelper*>::value_type(ch, pTH));
	}

	for each (auto it in G_STMap)
	{
		it.second->Start();
	}

	getchar();

	for each (auto it in G_STMap)
	{
		it.second->Close();
	}

	for each (auto it in G_STMap)
	{
		delete it.second;
		it.second = NULL;
	}

	th.Close();

	G_STMap.clear();

	return nResult;
}

#endif //__THREADHELPER_H_