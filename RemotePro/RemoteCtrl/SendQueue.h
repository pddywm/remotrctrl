#pragma once
#include "pch.h"
#include <list>
#include "Thread.h"
#include "CQueue.h"

template<class T>
class CSendQueue :public CQueue<T>, public ThreadFuncBase
{
public:
	typedef int (ThreadFuncBase::* EDYCALLBACK)(T& data);

	CSendQueue(ThreadFuncBase* obj, EDYCALLBACK callback) : CQueue<T>(), m_base(obj), m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE)&CSendQueue<T>::threadTick));
	}
	~CSendQueue() { }

protected:
	//virtual bool PopFront(T& data) = delete;
	bool PopFront()
	{
		typename CSendQueue<T>::IocpParam* Param = new typename CSendQueue<T>::IocpParam(CSendQueue<T>::EQPop, T());
		if (CSendQueue<T>::m_lock)
		{
			delete Param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(CSendQueue<T>::m_hCompeletionPort, sizeof(CSendQueue<T>::PPARAM), (ULONG_PTR)&Param, NULL);
		if (ret == false)
		{
			delete Param;
			return false;
		}
		return ret;
	}
	int threadTick()
	{
		if (WaitForSingleObject(CQueue<T>::m_hThread, 0) != WAIT_TIMEOUT)
		{
			return 0;
		}
		if (CSendQueue<T>::m_lstData.size() > 0)
		{
			PopFront();
		}
		Sleep(1);
		return 0;
	}
	void DealParam(typename CSendQueue<T>::PPARAM* pParam)
	{
		switch (pParam->nOperator)
		{
		case CSendQueue<T>::EQPush:
			CSendQueue<T>::m_lstData.push_back(pParam->Data);
			delete pParam;
			//printf_s("delete %08p!\r\n", (void*)pParam);
			break;
		case CSendQueue<T>::EQPop:
			if (CSendQueue<T>::m_lstData.size() > 0)
			{
				pParam->Data = CSendQueue<T>::m_lstData.front();
				if ((m_base->*m_callback)(pParam->Data) == 0)
				{
					CSendQueue<T>::m_lstData.pop_front();
				}
			}
			delete pParam;
			break;
		case CSendQueue<T>::EQSize:
			pParam->nOperator = CSendQueue<T>::m_lstData.size();
			if (pParam->hEvent != NULL)
			{
				SetEvent(pParam->hEvent);
			}
			break;
		case CSendQueue<T>::EQClear:
			CSendQueue<T>::m_lstData.clear();
			delete pParam;
			//printf_s("clear %08p!\r\n", (void*)pParam);
			break;
		default:
			OutputDebugStringA("unknown operator!\r\n");
			break;
		}
	}
private:
	ThreadFuncBase* m_base;
	EDYCALLBACK m_callback;
	CThread m_thread;
};

typedef CSendQueue<std::vector<char>>::EDYCALLBACK SENDCALLBACK;