#pragma once
/////////////////////////////////////////////////////////////////////////////////////////
// Description:
//	This class is used to create steam of the tags & save it in database
/////////////////////////////////////////////////////////////////////////////////////////

class CTagStream
{
public:
	CTagStream(UINT ulSize = 0);
	CTagStream(LPVOID pStream, UINT ulSize);
	~CTagStream();

	LPVOID	GetStream()			{return m_pStream;}
	UINT	GetStreamSize()		{return m_uiStreamSize;}

	void		MoveReadPointer(UINT uiIndex);

	USHORT	GetTagName();
	ULONG	GetTagSize();
	void	GetTagValue(LPVOID pVariable);

	void PutTag(USHORT uTag, ULONG ulSize, LPVOID pVariable);

protected:
	LPVOID		m_pStream;
	UINT		m_uiStreamSize;
	LPVOID		m_pTagReadPointer;
	LPVOID		m_pTagWritePointer;
	LPVOID		m_pStreamEnd;
};
