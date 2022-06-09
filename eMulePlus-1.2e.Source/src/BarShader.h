#pragma once

class CBarShader
{
public:
	CBarShader(uint32 dwHeight, uint32 dwWidth, COLORREF crColor = 0, uint64 qwFileSize = 1ui64);
	~CBarShader(void);

	void SetWidth(int width);
	void SetHeight(int height);

	int GetWidth() const		{ return m_iWidth; }
	int GetHeight() const		{ return m_iHeight; }

	//sets new file size and resets the shader
	void SetFileSize(uint64 qwFileSize);

	//fills in a range with a certain color, new ranges overwrite old
	void FillRange(uint64 qwStart, uint64 qwEnd, COLORREF crColor);

	//fills in entire range with a certain color
	void Fill(COLORREF crColor);

	//draws the bar
	void Draw(CDC* dc, int iLeft, int iTop, bool bFlat);
	void DrawPreview(CDC* dc, int iLeft, int iTop, byte previewLevel);
	void GenerateWSBar(CString *pstrBar);

protected:
	void BuildModifiers();
	void FillRect(CDC *dc, LPCRECT rectSpan, COLORREF crColor, bool bFlat);

	uint64		m_qwFileSize;
	int			m_iWidth;
	int			m_iHeight;
	double		m_dblPixelsPerByte;
	double		m_dblBytesPerPixel;

private:
	CRBMap<uint64, COLORREF> m_Spans;
	double		*m_pdblModifiers;
	byte		m_used3dlevel;
	bool		m_bIsPreview;
};
