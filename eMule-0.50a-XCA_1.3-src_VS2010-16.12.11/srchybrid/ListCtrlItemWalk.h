#pragma once

class CListCtrlItemWalk
{
public:
	CListCtrlItemWalk(CListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }

	virtual void* GetNextSelectableItem();
	virtual void* GetPrevSelectableItem();

	CListCtrl* GetListCtrl() const { return m_pListCtrl; }

protected:
	CListCtrl* m_pListCtrl;
};
