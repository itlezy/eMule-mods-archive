// 3DPreviewControl.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "3DPreviewControl.h"
#include "BarShader.h"

CBarShader C3DPreviewControl::s_preview(16, 16, RGB(192,192,255), 16);

IMPLEMENT_DYNAMIC(C3DPreviewControl, CStatic)
C3DPreviewControl::C3DPreviewControl()
: m_iSliderPos(0) // use flat
{
}

C3DPreviewControl::~C3DPreviewControl()
{
}

BEGIN_MESSAGE_MAP(C3DPreviewControl, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// Sets "slider" position for type of preview
void C3DPreviewControl::SetSliderPos(int iPos)
{
	if (iPos <= 5 && iPos >= -5)
		m_iSliderPos = iPos;

	if (GetSafeHwnd())
	{
		Invalidate();
		UpdateWindow();
	}
}

void C3DPreviewControl::OnPaint()
{
	CPaintDC	dc(this); // device context for painting
	CBrush		FrameBrush(RGB(127, 127, 127));
	RECT		outline_rec;

	outline_rec.top = 0;
	outline_rec.bottom = 18;
	outline_rec.left = 0;
	outline_rec.right = 18;
	dc.FrameRect(&outline_rec, &FrameBrush);
	s_preview.DrawPreview(&dc, 1, 1, static_cast<byte>(m_iSliderPos));
}
