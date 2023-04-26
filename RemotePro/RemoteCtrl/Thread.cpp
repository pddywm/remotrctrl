#include "pch.h"
#include "Thread.h"
//ThreadWorker
ThreadWorker::ThreadWorker() : that(NULL), func(NULL) { }

ThreadWorker::ThreadWorker(void* obj, FUNCTYPE f) :that((ThreadFuncBase*)obj), func(f) { }

ThreadWorker::ThreadWorker(const ThreadWorker& worker)
{
	if ((void*)&worker != nullptr) {
		that = worker.that;
		func = worker.func;
	}
}

ThreadWorker& ThreadWorker::operator=(const ThreadWorker& worker)
{
	that = worker.that;
	func = worker.func;
	return *this;
}

bool ThreadWorker::IsValid() const
{
	return (this != NULL) && (func != NULL);
}

int ThreadWorker::operator()() {
	if (IsValid())	return (that->*func)();
	return -1;
}


//CThread
CThread::CThread()
{
	m_hThread = NULL;
	m_bStatus = false;
}

CThread::~CThread()
{
	Stop();
}

bool CThread::Start()
{
	m_bStatus = true;
	m_hThread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
	if (!IsValid())m_bStatus = false;
	return m_bStatus;
}

bool CThread::IsValid()
{
	if ((m_hThread == NULL) || (m_hThread == INVALID_HANDLE_VALUE))
		return false;
	return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
}

bool CThread::Stop()
{
	if (m_bStatus == false)return true;
	m_bStatus = false;
	DWORD ret = WaitForSingleObject(m_hThread, 1000);
	if (ret == WAIT_TIMEOUT)
	{
		TerminateThread(m_hThread, -1);
	}
	UpdateWorker();
	return ret == WAIT_OBJECT_0;
}

void CThread::UpdateWorker(const::ThreadWorker& worker)
{
	if ((m_worker.load() != NULL) && (m_worker.load() != &worker))
	{
		::ThreadWorker* pWorker = m_worker.load();
		TRACE("delete pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
		m_worker.store(NULL);
		delete pWorker;
	}
	if (m_worker.load() == &worker)return;
	if (!worker.IsValid())
	{
		m_worker.store(NULL);
		return;
	}
	::ThreadWorker* pWorker = new::ThreadWorker(worker);
	TRACE("new pWorker = %08X m_worker = %08X\r\n", pWorker, m_worker.load());
	m_worker.store(pWorker);
}

bool CThread::IsIdle()
{
	if (m_worker.load() == NULL)return true;
	return !m_worker.load()->IsValid();
}

void CThread::ThreadWorker()
{
	while (m_bStatus)
	{
		::ThreadWorker worker = *m_worker.load();
		if (worker.IsValid())
		{
			if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
			{
				int ret = worker();
				if (ret != 0)
				{
					CString str;
					str.Format(_T("thread found warning code %d\r\n"), ret);
					OutputDebugString(str);
				}
				if (ret < 0)
				{
					m_worker.store(NULL);
				}
			}
		}
		else
		{
			Sleep(1);
		}
	}
}

void CThread::ThreadEntry(void* arg)
{
	CThread* that = (CThread*)arg;
	if (that)that->ThreadWorker();
	_endthread();
}

//CThreadPool

CThreadPool::CThreadPool(size_t size)
{
	m_threads.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		m_threads[i] = new CThread();
	}
}

CThreadPool::~CThreadPool()
{
	Stop();
	m_threads.clear();
}

bool CThreadPool::Invoke()
{
	bool ret = true;
	for (size_t i = 0; i < m_threads.size(); i++)
		if (m_threads[i]->Start() == false)
		{
			ret = false;
			break;
		}

	if (ret == false)
		for (size_t i = 0; i < m_threads.size(); i++)
			m_threads[i]->Stop();

	return ret;
}

void CThreadPool::Stop()
{
	for (size_t i = 0; i < m_threads.size(); i++)
		m_threads[i]->Stop();
}

int CThreadPool::DispatchWorker(const ThreadWorker& worker)
{
	int index = -1;
	m_lock.lock();
	for (size_t i = 0; i < m_threads.size(); i++)
	{
		if (m_threads[i]->IsIdle())
		{
			m_threads[i]->UpdateWorker(worker);
			index = i;
			break;
		}
	}
	m_lock.unlock();
	return index;
}

bool CThreadPool::CheckThreadValid(size_t index)
{
	if (index < m_threads.size())
		return m_threads[index]->IsValid();
	return false;
}

