#include "stdafx.h"
#include "FilterDlg.h"
#include "MenuCmds.h"
#include "otherfunctions.h"
#include "resource.h"
#include "titlemenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const UINT FilterSHStrID[]={
	 IDS_SEARCH_ANY
	,0
	,IDS_AUDIO
	,IDS_VIDEO
	,IDS_SEARCH_PICS
	,IDS_SEARCH_PRG
	,IDS_SEARCH_DOC
	,IDS_SEARCH_ARC
	,IDS_SEARCH_CDIMG
	,IDS_SEARCH_EMULECOLLECTION
};
//morph4u show no icons
/* 
static const LPCTSTR FilterSHIcon[]={
	 _T("SearchFileType_Any")
	,NULL
	,_T("SearchFileType_Audio")
	,_T("SearchFileType_Video")
	,_T("SearchFileType_Picture")
	,_T("SearchFileType_Program")
	,_T("SearchFileType_Document")
	,_T("SearchFileType_Archive")
	,_T("SearchFileType_CDImage")
	,_T("AABCollectionFileType")
};
*/
CString CFilterDlg::GetFilterLabel() const{
	ASSERT(filter < _countof(FilterSHStrID));
	return GetResString(FilterSHStrID[filter]);
}

void CFilterDlg::CreateFilterMenu(CTitleMenu&CatMenu) const// X: [FI] - [FilterItem]
{
	ASSERT(_countof(FilterSHStrID) /*== _countof(FilterSHIcon)*/); //morph4u show no icons
	for(size_t i = 0;i < _countof(FilterSHStrID);++i){
		if(FilterSHStrID[i] > 0)
			CatMenu.AppendMenu(MF_STRING, MP_CAT_SET0 + i, GetResString(FilterSHStrID[i])/*, FilterSHIcon[i]*/ ); //morph4u show no icons
		else/* if(FilterSHStrID[i] == 0)*/
			CatMenu.AppendMenu(MF_SEPARATOR);
	}

	CatMenu.CheckMenuItem( MP_CAT_SET0 + filter ,MF_CHECKED | MF_BYCOMMAND);
}
