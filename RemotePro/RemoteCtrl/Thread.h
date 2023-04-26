#pragma once
#include <atomic>
#include <vector>
#include <mutex>

class ThreadFuncBase {};

typedef int(ThreadFuncBase::* FUNCTYPE)();

class ThreadWorker
{
public:
	ThreadWorker();
	ThreadWorker(void* obj, FUNCTYPE f);
	ThreadWorker(const ThreadWorker& worker);
	ThreadWorker& operator=(const ThreadWorker& worker);
	bool IsValid() const;//是否有效
	int operator()();//运行功能函数
private:
	ThreadFuncBase* that;
	FUNCTYPE func;//功能函数
};

class CThread
{
public:
	CThread();
	~CThread();
	bool Start();//开启线程
	bool IsValid();//返回true表示有效，返回false表示线程异常或者已经终止
	bool Stop();//关闭线程
	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker());
	bool IsIdle();//true 表示空闲  false 表示已经分配了工作
private:
	void ThreadWorker();//工作线程
	static void ThreadEntry(void* arg);//线程入口点
	HANDLE m_hThread;//线程句柄
	bool m_bStatus;//false 表示线程将要关闭 true表示线程正在运行
	std::atomic<::ThreadWorker*>m_worker;//指向具体实现函数
};

class CThreadPool
{
public:
	CThreadPool(size_t size);
	~CThreadPool();
	bool Invoke();//启动线程池
	void Stop();//关闭线程池
	int DispatchWorker(const ThreadWorker& worker);
	//返回-1 表示分配失败，所有线程都在忙  大于等于0 表示第n个线程分配来做这个事情
	bool CheckThreadValid(size_t index);//检查线程是否有效
private:
	std::mutex m_lock;
	std::vector<CThread*>m_threads;//线程池线程
};