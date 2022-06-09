//this file is part of eMule
//Copyright (C)2002-2005 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "CollectionViewDialog.h"
#include "OtherFunctions.h"
#include "Collection.h"
#include "CollectionFile.h"
#include "DownloadQueue.h"
#include "TransferDlg.h"
#include "CatDialog.h"
#include "SearchDlg.h"
#include "Partfile.h"
#include "KnownFileList.h"//Xman [MoNKi: -Check already downloaded files-]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	PREF_INI_SECTION	_T("CollectionViewDlg")

enum ECols
{
	colName = 0,
	colSize,
	colHash
};

IMPLEMENT_DYNAMIC(CCollectionViewDialog, CDialog)

BEGIN_MESSAGE_MAP(CCollectionViewDialog, CModResizableDialog) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_BN_CLICKED(IDC_VCOLL_CLOSE, OnBnClickedOk)
	ON_BN_CLICKED(IDC_VIEWCOLLECTIONDL, OnBnClickedViewCollection)
	ON_NOTIFY(NM_DBLCLK, IDC_COLLECTIONVEWLIST, OnNmDblClkCollectionList)
	ON_BN_CLICKED(IDC_COLLECTIONVIEWCATEGORYCHECK, OnBnClickedCat)// X: [TD] - [TempDir]
END_MESSAGE_MAP()

CCollectionViewDialog::CCollectionViewDialog(CWnd* pParent /*=NULL*/)
	: CModResizableDialog(CCollectionViewDialog::IDD, pParent) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	, m_pCollection(NULL)
	, m_bSelfDelete(false) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	m_icoWnd = NULL;
	m_icoColl = NULL;
}

CCollectionViewDialog::~CCollectionViewDialog()
{
	if (m_icoWnd)
		VERIFY( DestroyIcon(m_icoWnd) );
	if (m_icoColl)
		VERIFY( DestroyIcon(m_icoColl) );

	// NEO: MLD - [ModelesDialogs] -- Xanatos -->
	if(m_bSelfDelete)
		delete m_pCollection;
	// NEO: MLD END <-- Xanatos --
}

void CCollectionViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLLECTIONVEWLIST, m_CollectionViewList);
	DDX_Control(pDX, IDC_COLLECTIONVIEWCATEGORYCHECK, m_AddNewCatagory);
	DDX_Control(pDX, IDC_COLLECTIONVIEWLISTLABEL, m_CollectionViewListLabel);
	DDX_Control(pDX, IDC_COLLECTIONVIEWLISTICON, m_CollectionViewListIcon);
	DDX_Control(pDX, IDC_VIEWCOLLECTIONDL, m_CollectionDownload);
	DDX_Control(pDX, IDC_VCOLL_CLOSE, m_CollectionExit);
	DDX_Control(pDX, IDC_COLLECTIONVIEWAUTHOR, m_CollectionViewAuthor);
	DDX_Control(pDX, IDC_COLLECTIONVIEWAUTHORKEY, m_CollectionViewAuthorKey);
	DDX_Control(pDX, IDC_TEMPCOMBO, m_temp);// X: [TD] - [TempDir]
}

void CCollectionViewDialog::SetCollection(CCollection* pCollection, bool bSelfDelete) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	if (!pCollection) {
		ASSERT(0);
		return;
	}
	m_pCollection = pCollection;
	m_bSelfDelete = bSelfDelete; // NEO: MLD - [ModelesDialogs] <-- Xanatos --
}

BOOL CCollectionViewDialog::OnInitDialog(void)
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	InitWindowStyles(this);

	if (!m_pCollection) {
		ASSERT(0);
		return TRUE;
	}

	m_CollectionViewList.Init(_T("CollectionView"));
	SetIcon(m_icoWnd = theApp.LoadIcon(_T("Collection_View")), FALSE);

	m_AddNewCatagory.SetCheck(false);

	SetWindowText(GetResString(IDS_VIEWCOLLECTION) + _T(": ") + m_pCollection->m_sCollectionName);

	m_CollectionViewListIcon.SetIcon(m_icoColl = theApp.LoadIcon(_T("AABCollectionFileType")));
	m_CollectionDownload.SetWindowText(GetResString(IDS_DOWNLOAD));
	SetDlgItemText(IDC_STATIC_TEMP,GetResString(IDS_PW_TEMP));// X: [TD] - [TempDir]
	m_CollectionExit.SetWindowText(GetResString(IDS_CW_CLOSE));
	SetDlgItemText(IDC_COLLECTIONVIEWAUTHORLABEL, GetResString(IDS_AUTHOR) + _T(':'));
	SetDlgItemText(IDC_COLLECTIONVIEWAUTHORKEYLABEL, GetResString(IDS_AUTHORKEY) + _T(':'));
	m_AddNewCatagory.SetWindowText(GetResString(IDS_COLL_ADDINCAT));// X: [CI] - [Code Improvement]
	SetDlgItemText(IDC_VCOLL_DETAILS, GetResString(IDS_DETAILS));
	SetDlgItemText(IDC_VCOLL_OPTIONS, GetResString(IDS_OPTIONS));

	m_CollectionViewAuthor.SetWindowText(m_pCollection->m_sCollectionAuthorName);
	m_CollectionViewAuthorKey.SetWindowText(m_pCollection->GetAuthorKeyHashString());

	AddAnchor(m_CollectionViewList, TOP_LEFT, BOTTOM_RIGHT);// X: [CI] - [Code Improvement]
	AddAnchor(IDC_VCOLL_DETAILS, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_VCOLL_OPTIONS, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHORLABEL, BOTTOM_LEFT);
	AddAnchor(IDC_COLLECTIONVIEWAUTHORKEYLABEL, BOTTOM_LEFT);
	AddAnchor(IDC_COLLECTIONVIEWCATEGORYCHECK, BOTTOM_LEFT);
	AddAnchor(m_CollectionViewAuthor, BOTTOM_LEFT, BOTTOM_RIGHT);// X: [CI] - [Code Improvement]
	AddAnchor(m_CollectionViewAuthorKey, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(m_CollectionExit, BOTTOM_RIGHT);
	AddAnchor(m_CollectionDownload, BOTTOM_RIGHT);
	AddAnchor(IDC_STATIC_TEMP, BOTTOM_RIGHT);// X: [TD] - [TempDir]
	AddAnchor(m_temp, BOTTOM_RIGHT);// X: [TD] - [TempDir]
	EnableSaveRestore(PREF_INI_SECTION, !thePrefs.prefReadonly); // X: [ROP] - [ReadOnlyPreference]

#ifdef REPLACE_ATLMAP
	for (CCollectionFilesMap::const_iterator it = m_pCollection->m_CollectionFilesMap.begin(); it != m_pCollection->m_CollectionFilesMap.end(); ++it)
	{
		CCollectionFile* pCollectionFile = it->second;
#else
	POSITION pos = m_pCollection->m_CollectionFilesMap.GetStartPosition();
	while (pos != NULL)
	{
		CCollectionFile* pCollectionFile;
		CSKey key;
		m_pCollection->m_CollectionFilesMap.GetNextAssoc(pos, key, pCollectionFile);
#endif
		int iImage = theApp.GetFileTypeSystemImageIdx(pCollectionFile->GetFileName());
		int iItem = m_CollectionViewList.InsertItem(LVIF_TEXT | LVIF_PARAM | (iImage > 0 ? LVIF_IMAGE : 0), m_CollectionViewList.GetItemCount(), NULL, 0, 0, iImage, (LPARAM)pCollectionFile);
		if (iItem != -1)
		{
			m_CollectionViewList.SetItemText(iItem, colName, pCollectionFile->GetFileName());
			m_CollectionViewList.SetItemText(iItem, colSize, CastItoXBytes(pCollectionFile->GetFileSize()));
			m_CollectionViewList.SetItemText(iItem, colHash, md4str(pCollectionFile->GetFileHash()));
		}
	}

	int iItem = m_CollectionViewList.GetItemCount();
	while (iItem)
		m_CollectionViewList.SetItemState(--iItem, LVIS_SELECTED, LVIS_SELECTED);

	CString strTitle;
	strTitle.Format(GetResString(IDS_COLLECTIONLIST) + _T(" (%u)"), m_CollectionViewList.GetItemCount());
	m_CollectionViewListLabel.SetWindowText(strTitle);

	m_temp.ResetContent();// X: [TD] - [TempDir]
	m_temp.AddString(_T(""));
	for (size_t i = 0; i < thePrefs.tempdir.GetCount(); i++) 
		m_temp.AddString(thePrefs.GetTempDir(i));
	m_temp.SetCurSel(0);
	OnBnClickedCat();
	return TRUE;
}

void CCollectionViewDialog::OnNmDblClkCollectionList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	DownloadSelected();
	*pResult = 0;
}

void CCollectionViewDialog::DownloadSelected(void)
{
	size_t iNewIndex = 0;
	for (size_t iIndex = 1; iIndex < thePrefs.GetCatCount(); iIndex++)
	{
		if (!m_pCollection->m_sCollectionName.CompareNoCase(thePrefs.GetCategory(iIndex)->strTitle))
		{
			iNewIndex = iIndex;
			break;
		}
	}

	if (m_AddNewCatagory.GetCheck() && !iNewIndex)
	{
		CString tempdir;// X: [TD] - [TempDir]
		m_temp.GetWindowText(tempdir);
		iNewIndex = theApp.emuledlg->transferwnd->AddCategory(m_pCollection->m_sCollectionName, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),tempdir, _T(""), _T(""), true);
		theApp.emuledlg->searchwnd->UpdateCatTabs();
	}

	CAtlList<CCollectionFile*> collectionFileList;
	POSITION pos = m_CollectionViewList.GetFirstSelectedItemPosition();
	while (pos != NULL)
	{
		int index = m_CollectionViewList.GetNextSelectedItem(pos);
		if (index >= 0)
			collectionFileList.AddTail((CCollectionFile*)m_CollectionViewList.GetItemData(index));
	}

	while (collectionFileList.GetCount() > 0)
	{
		CCollectionFile* pCollectionFile = collectionFileList.RemoveHead();
		if (pCollectionFile
			//Xman [MoNKi: -Check already downloaded files-]
			&& theApp.knownfiles->CheckAlreadyDownloadedFileQuestion(pCollectionFile->GetFileHash(), pCollectionFile->GetFileName()) )
			theApp.downloadqueue->AddSearchToDownload(pCollectionFile->GetED2kLink(), thePrefs.AddNewFilesPaused(), iNewIndex);
			//Xman end
	}
}

void CCollectionViewDialog::OnBnClickedViewCollection()
{
	DownloadSelected();
	OnBnClickedOk();
}

void CCollectionViewDialog::OnBnClickedOk()
{
	OnOK();
}

void CCollectionViewDialog::OnBnClickedCat()// X: [TD] - [TempDir]
{
	m_temp.EnableWindow(m_AddNewCatagory.GetCheck());
}
