#pragma once

struct PASTEURLDATA;

//////////////////////////////////////////////////////////////////////////////
// CMainFrameDropTarget

class CMainFrameDropTarget : public COleDropTarget
{
public:
	CMainFrameDropTarget();

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);

protected:
	BOOL m_bDropDataValid;

	BOOL IsSupportedDropData(COleDataObject* pDataObject);
	HRESULT PasteText(CLIPFORMAT cfData, COleDataObject& data);
};
