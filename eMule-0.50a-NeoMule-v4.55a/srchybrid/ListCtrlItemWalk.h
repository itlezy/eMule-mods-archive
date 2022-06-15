#pragma once

class CListCtrlItemWalk
{
public:
	//CListCtrlItemWalk(CListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }
	CListCtrlItemWalk(CListCtrl* pListCtrl = NULL) { m_pListCtrl = pListCtrl; } // NEO: MLD - [ModelesDialogs] <-- Xanatos --

	virtual CObject* GetNextSelectableItem();
	virtual CObject* GetPrevSelectableItem();

	CListCtrl* GetListCtrl() const { return m_pListCtrl; }

protected:
	CListCtrl* m_pListCtrl;
};
