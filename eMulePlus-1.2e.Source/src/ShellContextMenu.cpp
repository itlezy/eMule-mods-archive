#include "stdafx.h"
#include "emule.h"
#include "ShellContextMenu.h"
#include "SharedFileList.h"
#include "InputBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// global variables used for passing data to the subclassing wndProc
WNDPROC g_pOldWndProc; // regular window proc
LPCONTEXTMENU2 g_pIContext2; // active shell context menu

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	EMULE_TRY
	static bool handle=false;
	switch (msg)
	{
		case WM_MENUSELECT:
			{
				UINT uItem = (UINT) LOWORD(wp);
				handle=(uItem >= 1 && uItem <=  10000)?true:false;
			}
			break;
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			if(wp) break; // not menu related
		case WM_INITMENUPOPUP:
			if(handle&&g_pIContext2->HandleMenuMsg(msg, wp, lp)==NOERROR)
				return (msg==WM_INITMENUPOPUP ? 0 : TRUE); // handled
			break;
		default:
			break;
	}
	return ::CallWindowProc(g_pOldWndProc, hWnd, msg, wp, lp);
	EMULE_CATCH2
	return 0;
}

CShellContextMenu::CShellContextMenu(HWND hWnd, const CString& sAbsPath) :
	m_hWnd(hWnd), m_sAbsPath(sAbsPath)
{
	m_lpcm = NULL;
}

CShellContextMenu::~CShellContextMenu()
{
	if (m_lpcm)
		m_lpcm->Release();
}

bool CShellContextMenu::IsMenuCommand(int iCmd) const
{
	return ((1 <= iCmd) && (iCmd <= 10000));
}

void CShellContextMenu::InvokeCommand(int iCmd, CKnownFile* file) const
{
	EMULE_TRY

	if (iCmd)
	{
		TCHAR pcBuf[128] = _T("");

		m_lpcm->GetCommandString(iCmd - 1, GCS_VERB, NULL, (LPSTR)pcBuf, ARRSIZE(pcBuf));

		if (!_tcscmp(pcBuf, _T("rename")))
		{
			InputBox namebox(GetResString(IDS_RENAME), file->GetFileName(), true);

			if (namebox.DoModal() == IDOK)
			{
				CString strNewName(namebox.GetInput());
				CString path(file->GetPath());

				if (path.Right(1) != _T('\\'))
					path += _T('\\');
				if (_trename(path + file->GetFileName(), path + strNewName) == 0)
				{
					file->SetFileName(strNewName);
					g_App.m_pSharedFilesList->UpdateItem(file, true);
				}
				else
					g_App.m_pMDlg->AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to rename '%s' - %s"), file->GetFileName(), _tcserror(errno));
			}
			return;
		}

		CMINVOKECOMMANDINFO cmi;
		cmi.cbSize       = sizeof(cmi);
		cmi.fMask		 = 0;
		cmi.hwnd         = NULL;	//formerly m_hWnd, NULL brings windows like "open with" to front
		cmi.lpVerb       = (LPCSTR)MAKEINTRESOURCE(iCmd - 1);
		cmi.lpParameters = NULL;
		cmi.lpDirectory  = NULL;
		cmi.nShow        = SW_SHOWNORMAL;
		cmi.dwHotKey     = 0;
		cmi.hIcon        = NULL;
		m_lpcm->InvokeCommand(&cmi);
		g_App.m_pSharedFilesList->Reload();
	}
	EMULE_CATCH2
}

void CShellContextMenu::SetMenu(CMenu *pMenu)
{
	EMULE_TRY

	m_lpcm = NULL;
	g_pOldWndProc = NULL;

	if (m_sAbsPath.GetLength() == 0)
		return;

	LPMALLOC pMalloc;
	LPSHELLFOLDER psfFolder, psfNextFolder;
	LPITEMIDLIST pidlMain, pidlItem, pidlNextItem, *ppidl;
	ULONG ulCount, ulAttr;
	TCHAR tchPath[MAX_PATH];
	WCHAR wchPath[MAX_PATH];
	UINT nCount;

	//
	// Make sure the file name is fully qualified and in Unicode format.
	//
	GetFullPathName(m_sAbsPath, MAX_PATH, tchPath, NULL);
#ifdef _UNICODE
	lstrcpy(wchPath, tchPath);
#else
	MultiByteToWideChar(CP_ACP, 0, tchPath, -1, wchPath, MAX_PATH);
#endif

	if (SUCCEEDED (SHGetMalloc (&pMalloc)))
	{
		if (SUCCEEDED (SHGetDesktopFolder (&psfFolder)))
		{
			if (SUCCEEDED (psfFolder->ParseDisplayName (m_hWnd, NULL, wchPath, &ulCount, &pidlMain, &ulAttr)) && (pidlMain != NULL))
			{
				if ((nCount = GetItemCount(pidlMain)) != 0)
				{
					pidlItem = pidlMain;
					while (--nCount)
					{
						if ((pidlNextItem = DuplicateItem(pMalloc, pidlItem)) != NULL)
							if (SUCCEEDED (psfFolder->BindToObject (pidlNextItem, NULL, IID_IShellFolder, (void**)&psfNextFolder)))
							{
								psfFolder->Release();
								psfFolder = psfNextFolder;
								pMalloc->Free(pidlNextItem);
								pMalloc->Release();
								pidlItem = GetNextItem (pidlItem);
							}
							else
							{
								pMalloc->Free(pidlNextItem);
								pMalloc->Free (pidlMain);
								psfFolder->Release();
								pMalloc->Release();
								return;
							}
						else
						{
							pMalloc->Free (pidlMain);
							psfFolder->Release();
							pMalloc->Release();
							return;
						}
					}
					ppidl = &pidlItem;
					LPCONTEXTMENU pICv1 = NULL; // plain version
					if (SUCCEEDED (psfFolder->GetUIObjectOf (m_hWnd, 1, (const ITEMIDLIST**)ppidl, (const GUID)IID_IContextMenu, NULL, (void**)&pICv1)))
					{
						int cmType;
						if(pICv1)
						{ // try to obtain a higher level pointer, first 3 then 2
							if(pICv1->QueryInterface(IID_IContextMenu3, (void**)&m_lpcm)==NOERROR) cmType = 3;
							else {
								if(pICv1->QueryInterface(IID_IContextMenu2, (void**)&m_lpcm)==NOERROR) cmType = 2;
							}

							if(m_lpcm)
								pICv1->Release(); // free initial "v1.0" interface
							else
							{ // no higher version supported
								cmType = 1;
								m_lpcm = pICv1;
							}
						}
						if (SUCCEEDED (m_lpcm->QueryContextMenu (*pMenu, 0, 1, 10000, CMF_NORMAL|CMF_NODEFAULT|CMF_CANRENAME/*|CMF_EXPLORE|CMF_INCLUDESTATIC*/)))
						{
							// install the subclassing "hook", for versions 2 or 3
							if(cmType > 1)
							{
								g_pOldWndProc = (WNDPROC)
								SetWindowLong(m_hWnd, GWL_WNDPROC, (DWORD)HookWndProc);
								g_pIContext2 = (LPCONTEXTMENU2)m_lpcm; // cast ok for ICMv3
							}
							else
								g_pOldWndProc = NULL;
						}
					}
				}
			}
		}
	}
	pMalloc->Free(pidlMain);
	psfFolder->Release();
	pMalloc->Release();

	EMULE_CATCH2
}

UINT CShellContextMenu::GetItemCount (LPITEMIDLIST pidl)
{
	USHORT nLen;
	UINT nCount = 0;

	while ((nLen = pidl->mkid.cb) != 0)
	{
		pidl = GetNextItem (pidl);
		nCount++;
	}
	return nCount;
}

LPITEMIDLIST CShellContextMenu::GetNextItem (LPITEMIDLIST pidl)
{
	USHORT nLen;

	if ((nLen = pidl->mkid.cb) == 0)
		return NULL;

	return (LPITEMIDLIST) (((LPBYTE) pidl) + nLen);
}

LPITEMIDLIST CShellContextMenu::DuplicateItem (LPMALLOC pMalloc, LPITEMIDLIST pidl)
{
	USHORT nLen;
	LPITEMIDLIST pidlNew;

	nLen = pidl->mkid.cb;
	if (nLen == 0)
		return NULL;

	pidlNew = (LPITEMIDLIST) pMalloc->Alloc (nLen + sizeof (USHORT));
	if (pidlNew == NULL)
		return NULL;

	memcpy2(pidlNew, pidl, nLen);
	*((USHORT*) (((LPBYTE) pidlNew) + nLen)) = 0;

	return pidlNew;
}

void CShellContextMenu::CleanUp()
{
	if(g_pOldWndProc) // restore old wndProc
		SetWindowLong(m_hWnd, GWL_WNDPROC, (DWORD)g_pOldWndProc);
	g_pIContext2 = NULL; // prevents accidental use
}
