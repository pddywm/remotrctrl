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
    BOOL IsInvalid;//是否有效
    char szFileName[256];//文件名
    BOOL HasNext;//是否还有后续0没有1有
    BOOL IsDirectory;//是否未目录
}FILEINFO, * PFILEINFO;


typedef struct MouseEvent {
    MouseEvent() {
        nAction = 0;
        nButton = -1;
        ptXY.x = 0;
        ptXY.y = 0;
    }
    WORD nAction;//点击 移动 双击
    WORD nButton;//左键(0) 右键(2) 中键(1)
    POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

class CCommand
{
public:
	CCommand();
	~CCommand();
    static int RunCommand(CPacket& pack, std::list<CPacket*>* list, void* arg);//网络层回调
protected:
	typedef int(CCommand::*CMDFUNC)();//函数成员指针
	std::unordered_map<int, CMDFUNC> m_mapFunction;//命令与函数映射关系
protected:
    int ExcuteCommand(int nCmd);// 执行命令
	int MakeDriverInfo();//查看磁盘分区
    int MakeDirectoryInfo();//查看指定目录下文件
    int RunFile();//运行文件
    int DownLoadFile();//下载文件
    int MouseEvent();//鼠标操作
    int SendScreen();//发送屏幕内容---发送屏幕截图
    int LockMach();
    static unsigned _stdcall threadLockDlg(void* arg);
    int LockMachine();//锁机
    int UnLockMachine();//解锁
    int DeleteLocalFile();//删除文件
    int TestConnect();//测试
protected:
    CLockDialog dlg;//锁机界面
    unsigned int threadid;//锁机进程id
protected:
    CPacket packet;//从网络层接收到的数据
    std::list<CPacket*>* plist;//指向网络层待发送区
};

