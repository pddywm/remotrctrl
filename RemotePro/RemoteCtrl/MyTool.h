#pragma once
#include "pch.h"


class CMyTool {
public:
    static void ShowError() {//输出错误
        LPSTR lpMessageBuf = NULL;
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            lpMessageBuf, 0, NULL);
        OutputDebugString(lpMessageBuf);
        LocalFree(lpMessageBuf);
    }

    static void AutoInvoke() {//开机自启
        char sPath[MAX_PATH] = "";
        char sSys[MAX_PATH] = "";
        std::string strExe = "\\RemoteCtrl.exe";
        GetCurrentDirectoryA(MAX_PATH, sPath);
        GetSystemDirectoryA(sSys, sizeof(sSys));
        std::string linkpath = sSys + strExe;
        if (PathFileExistsA(linkpath.c_str()))return;
        std::string strCmd = "mklink " + linkpath + " " + std::string(sPath) + strExe;
        system(strCmd.c_str());
        //注册表注册
        CString strSubKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
        HKEY hKey = NULL;
        int ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey.GetBuffer(), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if (ret != ERROR_SUCCESS)return;
        CString strPath = sSys + CString("\\RemoteCtrl.exe");
        ret = RegSetValueEx(hKey, _T("RemoteCtrl"), 0, REG_SZ, (const BYTE*)strPath.GetBuffer(), strPath.GetLength());
        if (ret != ERROR_SUCCESS)return;
        RegCloseKey(hKey);
    }

    static bool IsAdmin() {//判断是否为管理员
        HANDLE hToken = NULL;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            ShowError();
            return false;
        }
        TOKEN_ELEVATION eve;
        DWORD len = 0;
        if (!GetTokenInformation(hToken, TokenElevation, &eve, sizeof(eve), &len)) {
            ShowError();
            return false;
        }
        CloseHandle(hToken);
        if (len == sizeof(eve)) return eve.TokenIsElevated;
        printf("length of tokeninformation is %d\r\n", len);
        return false;
    }



};
