#pragma once

#ifndef __THREADHEADER_H_
#define __THREADHEADER_H_

#include <string>
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
	enum TREADSTATE_TYPE{
		TSTYPE_MINIMUM = 0L,
		TSTYPE_NULLPTR = TSTYPE_MINIMUM,
		TSTYPE_STARTED = 1L,
		TSTYPE_SUSPEND = 2L,
		TSTYPE_RUNNING = 3L,
		TSTYPE_STOPPED = 4L,
		TSTYPE_MAXIMUM = MAXULONG_PTR,
	};

	CThreadHelper()
	{
		Startup();
	}
	CThreadHelper(LPTHREAD_START_ROUTINE _fnThreadStartRoutine, LPVOID _lpThreadParameter = NULL)
	{
		Startup();
		lpThreadParameter(_lpThreadParameter);
		fnThreadStartRoutine(_fnThreadStartRoutine);
	}
	~CThreadHelper()
	{
		Cleanup();
		Startup();
	}
	static DWORD WINAPI ThreadStartRoutine(LPVOID _lpThreadParameter)
	{
		DWORD dwResult = 0L;
		LPTHREAD_START_ROUTINE fnThreadStartRoutine = NULL;
		CThreadHelper * pTH = (CThreadHelper *)_lpThreadParameter;
		if (pTH)
		{
			pTH->SetState(TSTYPE_RUNNING);
			if (pTH->fnThreadStartRoutine())
			{
				fnThreadStartRoutine = pTH->fnThreadStartRoutine();
			}
			while (fnThreadStartRoutine)
			{
				switch (pTH->GetState())
				{
				case TSTYPE_RUNNING:
				{					
					fnThreadStartRoutine(pTH->lpThreadParameter());
				}
				break;
				case TSTYPE_STOPPED:
				{
					goto __LEAVE_CLEAN__;
				}
				break;
				case TSTYPE_SUSPEND:
				{
					Sleep(WAIT_TIMEOUT);
				}
				break;
				default:
				{
					goto __LEAVE_CLEAN__;
				}
				break;
				}
			}
		}
__LEAVE_CLEAN__:
		return dwResult;
	}

	BOOL Start()
	{
		SetState(TSTYPE_STARTED);
		m_hThread = CreateThread(NULL, NULL, CThreadHelper::ThreadStartRoutine, this, NULL, &m_dwThreadId);
		return (m_hThread ? TRUE : FALSE);
	}
	void Suspend()
	{
		SuspendThread(m_hThread);
		SetState(TSTYPE_SUSPEND);
	}
	void Resumed()
	{
		ResumeThread(m_hThread);
		SetState(TSTYPE_RUNNING);
	}
	void Close()
	{
		Cleanup();
	}

	void Startup()
	{
		m_hThread = NULL;
		m_dwThreadId = (0L);		
		SetState(TSTYPE_NULLPTR);
		SetPointer(&m_lpThreadParameter, NULL);
		SetPointer((LPVOID volatile *)&m_fnThreadStartRoutine, NULL);
	}
	void Cleanup()
	{
		SetState(TSTYPE_STOPPED);
		Sleep(WAIT_TIMEOUT);
		m_dwThreadId = (0L);
		if (m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = NULL;
		}
	}
	void SetState(TREADSTATE_TYPE tsType)
	{
		_InterlockedExchange((long volatile *)&m_nFlag, (long)tsType);
	}

	TREADSTATE_TYPE AndState()
	{
		return (TREADSTATE_TYPE)_InterlockedAnd((long volatile *)&m_nFlag, (long)MAXULONG_PTR);
	}

	TREADSTATE_TYPE GetState()
	{
		return AndState();
	}

	void PrintState()
	{
	}

	void SetPointer(LPVOID volatile * pTarget, LPVOID lpValues)
	{
		_InterlockedExchange((long volatile *)pTarget, (long)lpValues);
	}
	LPVOID GetPointer(LPVOID volatile * pTarget)
	{
		return (LPVOID)_InterlockedAnd((long volatile *)pTarget, MAXULONG_PTR);
	}

	void lpThreadParameter(LPVOID _lpThreadParameter)
	{
		SetPointer(&m_lpThreadParameter, _lpThreadParameter);
	}
	LPVOID lpThreadParameter()
	{
		return GetPointer(&m_lpThreadParameter);
	}
	void fnThreadStartRoutine(LPTHREAD_START_ROUTINE _fnThreadStartRoutine)
	{
		SetPointer((LPVOID volatile *)&m_fnThreadStartRoutine, (LPVOID)_fnThreadStartRoutine);
	}
	LPTHREAD_START_ROUTINE fnThreadStartRoutine()
	{
		return (LPTHREAD_START_ROUTINE)GetPointer((LPVOID volatile *)&m_fnThreadStartRoutine);
	}

private:

	HANDLE m_hThread;
	DWORD m_dwThreadId;
	LONG volatile m_nFlag;
	LPVOID volatile m_lpThreadParameter;
	LPTHREAD_START_ROUTINE volatile m_fnThreadStartRoutine;
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

#endif //__THREADHEADER_H_