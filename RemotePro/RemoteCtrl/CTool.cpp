#include "pch.h"
#include "CTool.h"

void CTool::Dump(BYTE* pData, size_t nSize)
{
        std::string strOut;
        for (size_t i{}; i < nSize; i++) {
            char buf[8] = "";
            if (i > 0 && (i % 4 == 0))strOut += "\n";
            snprintf(buf, sizeof(buf), "%02X ", pData[i] & 0xFF);
            strOut += buf;
        }
        strOut += "\n";
        OutputDebugStringA(strOut.c_str());
}
