#pragma once

class CShellContextMenu
{
    LPCONTEXTMENU                   m_lpcm;
    CString                         m_sAbsPath;
    HWND                            m_hWnd;
public:    
    CShellContextMenu(HWND m_hWnd, const CString& sAbsPath);
    ~CShellContextMenu();
    bool IsMenuCommand(int iCmd) const;
    void InvokeCommand(int iCmd, CKnownFile* file) const;
    void CShellContextMenu::SetMenu(CMenu *pMenu);
	UINT GetItemCount (LPITEMIDLIST pidl);
	LPITEMIDLIST GetNextItem (LPITEMIDLIST pidl);
	LPITEMIDLIST DuplicateItem (LPMALLOC pMalloc, LPITEMIDLIST pidl);
	void CleanUp();
};