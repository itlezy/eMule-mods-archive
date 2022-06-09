////////////////////////////////////////////////////////////////////////////////////////
// describes basic properties of the file-object in ed2k-Network
////////////////////////////////////////////////////////////////////////////////////////
/*
		  CKnownFile - CSharedFile - CPartFile
		/
CAbstractFile
		\
		  CSearchFile
*/
////////////////////////////////////////////////////////////////////////////////////////
#include "../EmEngine.h"
#include "../Other/ed2k_filetype.h"
#include "../Other/MDX_Hash.h"
////////////////////////////////////////////////////////////////////////////////////////
#define MAX_FILENAME_LENGTH 250
////////////////////////////////////////////////////////////////////////////////////////
class CAbstractFile : public CLoggable
{
public:
	CAbstractFile() : m_strFileName(_T(""))
	{
		m_dwFileSize = 0;
		m_uPartCount = 0;
		m_dwED2KPartCount = 0;
		m_dwED2KPartHashCount = 0;
		m_eFileType = ED2KFT_ANY;
	}
	virtual ~CAbstractFile()
	{
	}
	CString	GetFileName() const				{return m_strFileName;}
	void	SetFileName(const CString& NewName);
	BOOL	IsFileNameEmpty()				{return m_strFileName.IsEmpty();}

	//filename related property
	CString			GetFileTypeString();
	EED2KFileType	GetFileType()			{return m_eFileType;}

	uint32	GetFileSize() const				{return m_dwFileSize;}
	void 	SetFileSize(uint32 dwFileSize);

	//filesize related properties
	uint16	GetPartCount() const			{return m_uPartCount;}
	uint32	GetED2KPartHashCount() const	{return m_dwED2KPartHashCount;}
	uint32	GetED2KPartCount() const		{return m_dwED2KPartCount;}
	uint32	GetPartSize(uint16 uPart);


	//file hash
	const uchar*	GetFileHash() const			{return m_fileHash;}
	void	SetFileHash(uchar* pNewFileHash) 	{memcpy(&m_fileHash,pNewFileHash,16);}

	virtual CString CreateED2kLink() const;
	virtual CString CreateED2kSourceLink() const;
	virtual CString CreateHTMLED2kLink() const;

protected:
	CString		m_strFileName;
	uchar		m_fileHash[16];
	uint32		m_dwFileSize;

private:
	uint16			m_uPartCount;
	uint32			m_dwED2KPartCount;
	uint32			m_dwED2KPartHashCount;
	EED2KFileType 	m_eFileType;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The  file function outside the classes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString StripInvalidFilenameChars(CString strText, bool bKeepSpaces);
