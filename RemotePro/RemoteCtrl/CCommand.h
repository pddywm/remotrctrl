#pragma once
#include <unordered_map>
#include "LockDialog.h"
#include <list>
#include "CCPacket.h"

typedef struct file_info {
    file_info() {
        IsInvalid = FALSE;
        IsDirectory = -1;
        HasNext = TRUE;
        memset(szFileName, 0, sizeof(szFileName));
    }
    BOOL IsInvalid;//�Ƿ���Ч
    char szFileName[256];//�ļ���
    BOOL HasNext;//�Ƿ��к���0û��1��
    BOOL IsDirectory;//�Ƿ�δĿ¼
}FILEINFO, * PFILEINFO;


typedef struct MouseEvent {
    MouseEvent() {
        nAction = 0;
        nButton = -1;
        ptXY.x = 0;
        ptXY.y = 0;
    }
    WORD nAction;//��� �ƶ� ˫��
    WORD nButton;//���(0) �Ҽ�(2) �м�(1)
    POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;

class CCommand
{
public:
	CCommand();
	~CCommand();
    static int RunCommand(CPacket& pack, std::list<CPacket*>* list, void* arg);//�����ص�
protected:
	typedef int(CCommand::*CMDFUNC)();//������Աָ��
	std::unordered_map<int, CMDFUNC> m_mapFunction;//�����뺯��ӳ���ϵ
protected:
    int ExcuteCommand(int nCmd);// ִ������
	int MakeDriverInfo();//�鿴���̷���
    int MakeDirectoryInfo();//�鿴ָ��Ŀ¼���ļ�
    int RunFile();//�����ļ�
    int DownLoadFile();//�����ļ�
    int MouseEvent();//������
    int SendScreen();//������Ļ����---������Ļ��ͼ
    int LockMach();
    static unsigned _stdcall threadLockDlg(void* arg);
    int LockMachine();//����
    int UnLockMachine();//����
    int DeleteLocalFile();//ɾ���ļ�
    int TestConnect();//����
protected:
    CLockDialog dlg;//��������
    unsigned int threadid;//��������id
protected:
    CPacket packet;//���������յ�������
    std::list<CPacket*>* plist;//ָ��������������
};

