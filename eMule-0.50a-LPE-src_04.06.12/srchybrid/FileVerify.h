// X: [FV] - [FileVerify]
#pragma once
#include<vector>

#define HEADERVERIFYSIZE 32
enum EVerifyStatus{
	VS_NONE,
	VS_RESERVED,
	VS_UTF8,
	VS_UTF16,
	VS_SIZESMALL,
	VS_SIZELARGE,
	VS_SIZENA
};

enum EActionProcess{
	AP_COMPARE,
	AP_CHARAND,
	AP_CHARALTER,
	AP_FILESIZE
};

struct verifyAction{
	EActionProcess process;
	uint8 start;
	uint8 arg;
	verifyAction(EActionProcess p,uint8 s,uint8 a):process(p),start(s),arg(a){}
};


struct FileFmt_Struct{
	CString label;
	size_t offset;
	std::vector<uint8> header;
	std::vector<verifyAction> action;
};

class CPartFile;
class CFileVerify{
	std::vector<FileFmt_Struct*> textfmts;
	std::vector<FileFmt_Struct*> binfmts;
	uint_ptr GetHeaderfromStr(TCHAR* str, uint_ptr nLen, FileFmt_Struct* fmt);
	bool VerifyBinary(uint8* buff, std::vector<uint8>&header, std::vector<verifyAction>&action, CPartFile*file) const;
public:
	CFileVerify();
	~CFileVerify();
	FileFmt_Struct*CheckFileFormat(CPartFile*file, uint_ptr completedHeader) const;
};