#pragma once

class CListCtrlItemWalk
{
public:
	CListCtrlItemWalk(CListCtrl* pListCtrl) { m_pListCtrl = pListCtrl; }

	// Tux: Feature: Modeless dialogs [start]
	// -> changed objects
	// account for multiple dialogs
	virtual CObject* GetNextSelectableItem(CObject* pCurrentObj);
	virtual CObject* GetPrevSelectableItem(CObject* pCurrentObj);
	// Tux: Feature: Modeless dialogs [end]

	CListCtrl* GetListCtrl() const { return m_pListCtrl; }

protected:
	CListCtrl* m_pListCtrl;
};
