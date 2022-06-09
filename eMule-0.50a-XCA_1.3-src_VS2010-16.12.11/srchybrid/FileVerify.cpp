// X: [FV] - [FileVerify]

#include "stdafx.h"
#include <share.h>
#include "resource.h"
#include "OtherFunctions.h"
#include "FileVerify.h"
#include "PartFile.h"
#include "Preferences.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

uint_ptr GetBOMSize(uint8* buff){
	if(buff[0]==0xef&&buff[1]==0xbb&&buff[2]==0xbf)//UTF-8
		return 3;
	if(buff[0]==0xfe&&buff[1]==0xff)//UTF-16 Big Endian
		return 2;
	//if(buff[0]==0&&buff[1]==0&&buff[2]==0xfe&&buff[3]==0xff)//UTF-32 Big Endian
	//	return 4;
	if(buff[0]==0xff&&buff[1]==0xfe){
		//if(buff[2]==0&&buff[3]==0)
		//	return 4;//UTF-32 Little Endian
		//
		return 2;//UTF-16 Little Endian
	}
	return 0;
}

////////////////////////////////////////////
///CFileVerify

bool CFileVerify::VerifyBinary(uint8* buff, std::vector<uint8>&header, std::vector<verifyAction>&action, CPartFile*file) const{
#define AND_FAIL 1
#define ALT_FAIL 2
	file->verifystatus = VS_NONE;
	sint_ptr j=-1;
	sint_ptr curCh=-1;
	size_t res=0, i=0, count=action.size();
	while(i<count){
		if(j==action[i].start){
			if(curCh==-1){
				++i;
				continue;
			}
		}
		else if((j=action[i].start)>=HEADERVERIFYSIZE)
			break;
		if(action[i].process==AP_COMPARE){
			if(action[i].arg>0){
				if(j+action[i].arg>HEADERVERIFYSIZE)
					action[i].arg=(uint8)(HEADERVERIFYSIZE-j);
				if(action[i].arg==1){
					if(buff[j]!=header[j])
						return false;
				}
				else if(memcmp(buff+j,&header[j],action[i].arg)!=0)
					return false;
			}
			++i;
		}
		else if(action[i].process==AP_FILESIZE){
			uint64 size=(uint64)file->GetFileSize();
			if(size < 0xFFFFFFFF){
				uint32* psize=reinterpret_cast<uint32*>((buff+j));
				uint32 size32=(uint32)(size-action[i].arg);
				if(size32>*psize)
					file->verifystatus = VS_SIZELARGE;//wrong size,large
				else if(size32<*psize)
					file->verifystatus = VS_SIZESMALL;//wrong size,small
			}
			else
				file->verifystatus = VS_SIZENA;//size,N/A
			++i;
		}
		else{
			if(curCh==-1)
				curCh=header[j];
			if(action[i].process==AP_CHARALTER){
				if(buff[j]!=curCh){
					res=ALT_FAIL;
					if(curCh==action[i].arg)
						++i;
				}
				else
					++i;
			}
			else if(action[i].process==AP_CHARAND){
				if((buff[j]&action[i].arg)!=(curCh&action[i].arg))
					res=AND_FAIL;
				++i;
			}
			if(res>0){
				if((i==count)||(j!=action[i].start)||(action[i].process!=AP_CHARALTER))
					return false;
				if(((i+1)<count)&&(j==action[i+1].start)&&(action[i+1].process==AP_CHARAND))
					++i;
				res=0;
				curCh=action[i].arg;
			}
			else
				curCh=-1;
		}
	}
	return true;
}

#define STR_BEGIN _T('[')
#define STR_END _T(']')
#define CHR_DUP _T('*')
#define SEEK_REL _T('~')
#define CHK_OPT _T('%')
#define OPT_SIZE _T('s')
#define CHR_AND _T('&')
#define CHR_ALT _T('/')
uint_ptr CFileVerify::GetHeaderfromStr(TCHAR* str, uint_ptr nLen, FileFmt_Struct* fmt){
	bool actadded=true;
	uint_ptr n=0, actstart=0, res=0, i=0;
	for (;i<nLen-1&&n<HEADERVERIFYSIZE;++i){
		int curCh=str[i];
		sint_ptr h;
		if(curCh==STR_BEGIN){// String
			while((h=str[++i])!=STR_END){
				if(i==nLen){ res=3;	break;}
				if(n==HEADERVERIFYSIZE){++i;break;}
				++n;
				fmt->header.push_back((uint8)h);
			}
			actadded=false;
		}
		else if(curCh==CHR_DUP){// repeat
			if(actadded||(h=charhexval(str[++i]))<1) return 1;
			--h;
			uint8 last=fmt->header.back();
			if(n+h>HEADERVERIFYSIZE){
				h=HEADERVERIFYSIZE-n;
				n=HEADERVERIFYSIZE;
			}
			else
				n+=h;
			while(h--!=0)
				fmt->header.push_back(last);
		}
		else if((h=charhexval(curCh))!=-1){
			sint_ptr l=charhexval(str[++i]);
			if(l==-1) return 1;
			fmt->header.push_back((uint8)((h << 4) | l));
			++n;
			actadded=false;
		}
		else{
			if(!actadded&&n>actstart)
				fmt->action.push_back(verifyAction(AP_COMPARE,(uint8)(actstart),(uint8)(n-actstart)));

			if(curCh==SEEK_REL){// skip
				if((h=charhexval(str[++i]))==-1) return 1;
				if(n+h>HEADERVERIFYSIZE)
					return 2;
				n+=h;
				while(h--!=0)
					fmt->header.push_back((uint8)0);
			}
			else if(curCh==CHK_OPT){// specific infos,%s:filesize
				if(str[++i]==OPT_SIZE){
					if(n+4>HEADERVERIFYSIZE)
						return 2;
					sint_ptr l;
					if(i>nLen-3||(h=charhexval(str[++i]))==-1||(l=charhexval(str[++i]))==-1) return 1;
					fmt->action.push_back(verifyAction(AP_FILESIZE,(uint8)n,(uint8)((h << 4) | l)));
					n+=h=4;
					do{
						fmt->header.push_back((uint8)0);
					}while(--h!=0);
				}
				else
					return 1;
			}
			else if(curCh==CHR_AND||curCh==CHR_ALT){// mask and alternative
				sint_ptr l;
				if(i>nLen-3||(h=charhexval(str[++i]))==-1||(l=charhexval(str[++i]))==-1) return 1;
				if(!actadded && ((--fmt->action.back().arg)==0))
					fmt->action.pop_back();
				fmt->action.push_back(verifyAction( (curCh==CHR_AND)?AP_CHARAND:AP_CHARALTER ,(uint8)(n-1),(uint8)((h << 4) | l)));
			}
			else
				return 1;
			actadded=true;
			actstart=n;
		}
	}
	if(!actadded)
		fmt->action.push_back(verifyAction(AP_COMPARE,(uint8)(actstart),(uint8)(n-actstart)));
	if(i==nLen){
		if(res!=0) return res;
		return 0;
	}
	else
		return 2;
}

CFileVerify::CFileVerify(){
	CString fileformatCSVfile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("Fileformat.csv");
	FILE* readFile = _tfsopen(fileformatCSVfile, _T("rS"), _SH_DENYWR);
	if (readFile != NULL) {
		DWORD wBOM = 0;// X: ensure file is ANSI
		if(fread(&wBOM, sizeof(wBOM), 1, readFile) == 1 && (((WORD)wBOM == 0xFEFF) || ((WORD)wBOM == 0xFFFE) || ((wBOM & 0xFFFFFF) == 0xBFBBEF))){
			fclose(readFile);
			LogError(_T("Failed to load %s, the file is not ANSI"), fileformatCSVfile);
			return;
		}
		(void)fseek(readFile, 0, SEEK_SET);
		TCHAR szbuffer[128];//szNameStart
		uint_ptr iLine = 0;
		TCHAR *szHeaderStart,*szOffsetStart,*szTypeStart;
		while (_fgetts(szbuffer, 128,readFile) != NULL) {
			++iLine;
			size_t len = _tcslen(szbuffer);
			if(len<5) continue;

			TCHAR *pbuffer = szbuffer;
			if(pbuffer[len-1] == 10)
				pbuffer[len-1] = 0;
			for (; *pbuffer != 0 && *pbuffer != _T(','); pbuffer++ );
			if(*pbuffer == 0) continue;
			*pbuffer = '\0';
			szHeaderStart=++pbuffer;
			for ( ; *pbuffer != 0 && *pbuffer != _T(','); pbuffer++ );
			if(szHeaderStart!=pbuffer){
				size_t nLen=pbuffer-szHeaderStart;
				bool isText = false;
				uint_ptr offset = 0;
				if(*pbuffer != 0){
					*pbuffer = '\0';
					szOffsetStart = ++pbuffer;
					for (; *pbuffer != 0 && *pbuffer != _T(','); pbuffer++ );
					if(*pbuffer != 0){
						*pbuffer = '\0';
						szTypeStart = ++pbuffer;
						if(szTypeStart[0]==_T('t'))
							isText=true;
					}
					offset = _tcstoul(szOffsetStart, NULL/*, 10*/);
				}
				uint_ptr res = 0;
				if(isText)
				{
					if(!offset){
						FileFmt_Struct*newfmt = new FileFmt_Struct();
						res = GetHeaderfromStr(szHeaderStart, nLen, newfmt);
						if(res > 0)
							delete newfmt;
						else{
							newfmt->label = szbuffer;
							textfmts.push_back(newfmt);
						}
					}
					else
						res = 3;
				}
				else
				{
					FileFmt_Struct* newfmt = new FileFmt_Struct();
					res = GetHeaderfromStr(szHeaderStart, nLen, newfmt);
					if(res > 0)
						delete newfmt;
					else{
						newfmt->label = szbuffer;
						newfmt->offset = offset;
						binfmts.push_back(newfmt);
					}
				}
				static const LPCTSTR retString[]={
					_T("the header definition is invalid"),
					_T("the header size is larger than 32"),
					_T("[ & ] not match"),
					_T("text type not allow offset")
				};
				if (res > 0)
					AddDebugLogLine(false,_T("Fileformat.csv Line %u(name=%s):%s."), iLine, szbuffer, retString[res]);
			}
		}
		fclose(readFile);

		if (thePrefs.m_bVerbose)
			AddDebugLogLine(false, _T("Parsed lines:%u  Found formats:Text %i  Binary %i"), iLine, textfmts.size(), binfmts.size());
	}
}

CFileVerify::~CFileVerify()
{
	for (std::vector<FileFmt_Struct*>::const_iterator it = textfmts.begin(); it != textfmts.end(); ++it)
		delete *it;
	for (std::vector<FileFmt_Struct*>::const_iterator it = binfmts.begin(); it != binfmts.end(); ++it)
		delete *it;
}

FileFmt_Struct*CFileVerify::CheckFileFormat(CPartFile*file, uint_ptr completedHeader) const
{
	if(textfmts.size() == 0 && binfmts.size() == 0)
		return NULL;
	try {
		uint8 headbuffer[HEADERVERIFYSIZE];
		CFile inFile;
		if (!inFile.Open(file->GetFilePath(), CFile::modeRead | CFile::osSequentialScan | CFile::shareDenyNone))
			return NULL;
		if (inFile.Read(headbuffer, HEADERVERIFYSIZE) != HEADERVERIFYSIZE)
			return NULL;
		uint8 newbuffer[HEADERVERIFYSIZE];
		size_t nLen = 0;
		size_t BOMbyte = GetBOMSize(headbuffer);
		for(size_t i = BOMbyte;i < HEADERVERIFYSIZE; ++i){
			if(headbuffer[i] > 127){
				nLen = 0;
				break;
			}
			if(headbuffer[i])
				newbuffer[nLen++] = headbuffer[i];
		}
		if(nLen){
			for(size_t i = 0;i<textfmts.size();++i){
				FileFmt_Struct* ff = textfmts[i];
				if(_memicmp(newbuffer, &ff->header[0], nLen)==0){
					inFile.Close();
					return ff;
				}
			}
		}
		if(BOMbyte){
			file->verifystatus = (uint8)BOMbyte;
			inFile.Close();
			return NULL;
		}

		for(size_t i = 0;i < binfmts.size(); ++i){
			FileFmt_Struct* ff = binfmts[i];
			if(ff->offset==0 || ff->offset+ff->header.size() <= HEADERVERIFYSIZE){
				if(VerifyBinary(headbuffer, ff->header, ff->action, file)){
					inFile.Close();
					return ff;
				}
			}
			else if(completedHeader>=ff->offset+HEADERVERIFYSIZE){
				inFile.Seek(ff->offset, CFile::begin);
				if (inFile.Read(newbuffer,HEADERVERIFYSIZE) == HEADERVERIFYSIZE){
					if(VerifyBinary(headbuffer, ff->header, ff->action, file)){
						inFile.Close();
						return ff;
					}
				}
			}
		}
		inFile.Close();
	} catch(...) {
		ASSERT(0);
	}
	return NULL;
}
