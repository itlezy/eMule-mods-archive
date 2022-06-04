// ShellContextMenu :: Start
#include "stdafx.h"
#include "emule.h"
#include "ShellContextMenu.h"
#include "SharedFileList.h"
#include "InputBox.h"
#include "Preferences.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// global variables used for passing data to the subclassing wndProc
WNDPROC g_pOldWndProc; // regular window proc
LPCONTEXTMENU2 g_pIContext2; // active shell context menu

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool handle=false;
	switch (message)
	{
		case WM_MENUSELECT:
			{
				UINT uItem = (UINT) LOWORD(wParam);
				handle=(uItem >= 1 && uItem <=  10000)?true:false;
			}
			break;
		case WM_DRAWITEM:
		case WM_MEASUREITEM:
			if(wParam) break; // not menu related
		case WM_INITMENUPOPUP:
			if(handle&&g_pIContext2->HandleMenuMsg(message, wParam, lParam)==NOERROR)
				return (message==WM_INITMENUPOPUP ? 0 : TRUE); // handled
			break;
		default:
			break;
	}
	return ::CallWindowProc(g_pOldWndProc, hWnd, message, wParam, lParam);
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
	CKnownFile* pKnownFile = NULL;
		if (file != NULL && IsKindOfCKnownFile(file)/*file->IsKindOf(RUNTIME_CLASS(CKnownFile))*/)
			pKnownFile = (CKnownFile*)file;

	if (iCmd)
	{
		TCHAR pcBuf[128] = _T("");

		m_lpcm->GetCommandString(iCmd - 1, GCS_VERB, NULL, (LPSTR)pcBuf, ARRSIZE(pcBuf));

		if (!_tcscmp(pcBuf, _T("rename")))
		{
			if (pKnownFile && !pKnownFile->IsPartFile()){
					InputBox inputbox;
					CString title = GetResString(IDS_RENAME);
					title.Remove(_T('&'));
					inputbox.SetLabels(title, GetResString(IDS_DL_FILENAME), pKnownFile->GetFileName());
					inputbox.SetEditFilenameMode();
					inputbox.DoModal();
					CString newname = inputbox.GetInput();
					if (!inputbox.WasCancelled() && newname.GetLength()>0)
					{
						// at least prevent users from specifying something like "..\dir\file"
						static const TCHAR _szInvFileNameChars[] = _T("\\/:*?\"<>|");
						if (newname.FindOneOf(_szInvFileNameChars) != -1){
							AfxMessageBox(GetErrorMessage(ERROR_BAD_PATHNAME));
							return;
						}

						CString newpath;
						PathCombine(newpath.GetBuffer(MAX_PATH), file->GetPath(), newname);
						newpath.ReleaseBuffer();
						if (_trename(pKnownFile->GetFilePath(), newpath) != 0){
							CString strError;
							strError.Format(GetResString(IDS_ERR_RENAMESF), file->GetFilePath(), newpath, _tcserror(errno));
							AfxMessageBox(strError);
							return;
						}

						if (IsCPartFile(pKnownFile)/*pKnownFile->IsKindOf(RUNTIME_CLASS(CPartFile))*/)
						{
							pKnownFile->SetFileName(newname);
							//STATIC_DOWNCAST(CPartFile, pKnownFile)->SetFullName(newpath); 
							((CPartFile*)pKnownFile)->SetFullName(newpath); 
						}
						else
						{
							//theApp.sharedfiles->RemoveKeywords(pKnownFile);// X: [BF] - [Bug Fix] Don't need to remove & add
							pKnownFile->SetFileName(newname);
							//theApp.sharedfiles->AddKeywords(pKnownFile);
						}
						pKnownFile->SetFilePath(newpath);
						theApp.sharedfiles->UpdateFile(pKnownFile);
					}
				}
				else
					MessageBeep(MB_OK);
				
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
		theApp.sharedfiles->Reload(thePrefs.queryOnHashing!=1);// X: [QOH] - [QueryOnHashing]
	}
}

void CShellContextMenu::SetMenu(CMenu *pMenu)
{
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
								SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
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

	memcpy(pidlNew, pidl, nLen);
	*((USHORT*) (((LPBYTE) pidlNew) + nLen)) = 0;

	return pidlNew;
}

void CShellContextMenu::CleanUp()
{
	if(g_pOldWndProc) // restore old wndProc
		SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)g_pOldWndProc);
	g_pIContext2 = NULL; // prevents accidental use
}
// ShellContextMenu :: End