//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-2002, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//               Added print support. - KR
//
//       11/3/99 Fixed most common complaint. Added
//            background color fill. - KR
//
//       11/3/99 Added support for mapping modes other than
//            MM_TEXT as suggested by Lee Sang Hun. - KR
//
//       02/11/02 Added support for CScrollView as supplied
//             by Gary Kirkham. - KR
//
// This class implements a memory Device Context which allows
// flicker free drawing.

#pragma once

class CMemDC : public CDC
{
private:
	CBitmap		m_bitmap;		// Offscreen bitmap
	CBitmap		*m_oldBitmap;	// bitmap originally found in CMemDC
	CDC			*m_pDC;			// Saves CDC passed in constructor
	CRect		m_rect;			// Rectangle of drawing area
	bool		m_bMemDC;		// TRUE if CDC really is a Memory DC
	bool		m_bFlushed;
public:

	CMemDC(CDC *pDC, const RECT *pRect, COLORREF crWinBk, COLORREF crBk) : CDC()
	{
		EMULE_TRY
		ASSERT(pDC != NULL);

		// Some initialization
		m_pDC = pDC;
		m_oldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();
		m_bFlushed = false;
		m_rect = *pRect;

		if (m_bMemDC)
		{
			// Create a Memory DC
			CreateCompatibleDC(pDC);
			//pDC->LPtoDP(&m_rect);

			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_oldBitmap = SelectObject(&m_bitmap);

			SetMapMode(pDC->GetMapMode());

			SetWindowExt(pDC->GetWindowExt());
			SetViewportExt(pDC->GetViewportExt());

			//pDC->DPtoLP(&m_rect);
			SetWindowOrg(m_rect.left, m_rect.top);
		} else {
			m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}

	// Fill background
		if (crWinBk != crBk)
		{
		//	Draw cursor
			long	lRight = m_rect.right, lLeft = m_rect.left;

		//	Margins around cursor
			SetBkColor(crWinBk);
			m_rect.right = lLeft + 1;
			ExtTextOut(0, 0, ETO_OPAQUE, m_rect, NULL, 0, NULL);	// fast fill rectangle
			m_rect.right = lRight;
			m_rect.left = lRight - 1;
			ExtTextOut(0, 0, ETO_OPAQUE, m_rect, NULL, 0, NULL);

			m_rect.right = lRight - 1;
			m_rect.left = lLeft + 1;
			FillSolidRect(m_rect, crBk);
			m_rect.right++;
			m_rect.left--;
		}
		else
			FillSolidRect(m_rect, crBk);
		EMULE_CATCH2
	}
	
	~CMemDC()
	{
		Flush();
	}

	void Flush()
	{
		if (!m_bFlushed){
			m_bFlushed = true;
			if (m_bMemDC) {
				// Copy the offscreen bitmap onto the screen
				m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(),
					m_rect.Height(), this, m_rect.left, m_rect.top, SRCCOPY);

				// Swap back the original bitmap
				SelectObject(m_oldBitmap);
			}
			else
				m_hDC = m_hAttribDC = NULL;
		}
	}

// Allow usage as a pointer
	CMemDC* operator->()
	{
		return this;
	}

// Allow usage as a pointer
	operator CMemDC*()
	{
		return this;
	}
};
