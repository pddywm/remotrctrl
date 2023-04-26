#pragma once
#include <iostream>
#pragma pack(push)
#pragma pack(1)

class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {};
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize){
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}
		sSum = 0;
		for (size_t j{}; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
	};
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		strData = pack.strData;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
	};
	CPacket(const BYTE* pData, int& nSize) {
		size_t i{};
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		if (i + 8 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);
		i += 4;
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 4);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}

		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum{};
		for (size_t j{}; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			strData = pack.strData;
			sCmd = pack.sCmd;
			sSum = pack.sSum;
		}
		return *this;
	}
	~CPacket(){};
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size());
		pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
	WORD sHead;//固定位FE FF
	DWORD nLength;//包长度（从控制命令开始到校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
};
#pragma pack(pop)