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
	bool IsValid() const;//�Ƿ���Ч
	int operator()();//���й��ܺ���
private:
	ThreadFuncBase* that;
	FUNCTYPE func;//���ܺ���
};

class CThread
{
public:
	CThread();
	~CThread();
	bool Start();//�����߳�
	bool IsValid();//����true��ʾ��Ч������false��ʾ�߳��쳣�����Ѿ���ֹ
	bool Stop();//�ر��߳�
	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker());
	bool IsIdle();//true ��ʾ����  false ��ʾ�Ѿ������˹���
private:
	void ThreadWorker();//�����߳�
	static void ThreadEntry(void* arg);//�߳���ڵ�
	HANDLE m_hThread;//�߳̾��
	bool m_bStatus;//false ��ʾ�߳̽�Ҫ�ر� true��ʾ�߳���������
	std::atomic<::ThreadWorker*>m_worker;//ָ�����ʵ�ֺ���
};

class CThreadPool
{
public:
	CThreadPool(size_t size);
	~CThreadPool();
	bool Invoke();//�����̳߳�
	void Stop();//�ر��̳߳�
	int DispatchWorker(const ThreadWorker& worker);
	//����-1 ��ʾ����ʧ�ܣ������̶߳���æ  ���ڵ���0 ��ʾ��n���̷߳��������������
	bool CheckThreadValid(size_t index);//����߳��Ƿ���Ч
private:
	std::mutex m_lock;
	std::vector<CThread*>m_threads;//�̳߳��߳�
};