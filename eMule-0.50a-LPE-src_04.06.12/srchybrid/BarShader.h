#pragma once
// X: [CI] - [Code Improvement] BarShader
class CRectShader
{
public:
	CRectShader(uint32 height = 1);
	~CRectShader(void);

	//set the height of the bar
	void SetHeight(int height);

	//returns the height of the bar
	int GetHeight() {
		return m_iHeight;
	}
	void DrawRect(CDC* dc, int iLeft, int iTop, int iWidth, COLORREF color, bool bFlat);
	void DrawPreview(CDC* dc, int iLeft, int iTop, int iWidth, COLORREF color, UINT previewLevel);		//Cax2 aqua bar

protected:
	void BuildModifiers();
	void FillRect(CDC *dc, LPRECT rectSpan, float fRed, float fGreen, float fBlue); // X: [CI] - [Code Improvement]
	int    m_iHeight;
	float *m_Modifiers;

	//bool	m_bIsPreview;
	UINT m_used3dlevel;
};

class CBarShader : public CRectShader
{
public:
	CBarShader(uint32 height = 1);

	//call this to blank the shaderwithout changing file size
	void Reset();

	//sets new file size and resets the shader
	void SetFileSize(EMFileSize fileSize);

	//fills in a range with a certain color, new ranges overwrite old
	void FillRange(uint64 start, uint64 end, COLORREF color);

	//fills in entire range with a certain color
	void Fill(COLORREF color);

	//draws the bar
	void Draw(CDC* dc, int iLeft, int iTop, int iWidth, bool bFlat);
	void Draw(CDC* dc, int iLeft, int iTop, int iWidth, uint64 start, uint64 end, bool bFlat);

protected:
	EMFileSize m_uFileSize;

private:
	CRBMap<uint64, COLORREF> m_Spans;	// SLUGFILLER: speedBarShader
};
