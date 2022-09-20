// BrowserToolbarCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "otherfunctions.h"
#include "BrowserToolbarCtrl.h"


// CBrowserToolbarCtrl

IMPLEMENT_DYNAMIC(CBrowserToolbarCtrl, CToolBarCtrl)
CBrowserToolbarCtrl::CBrowserToolbarCtrl()
{
}

CBrowserToolbarCtrl::~CBrowserToolbarCtrl()
{
}


BEGIN_MESSAGE_MAP(CBrowserToolbarCtrl, CToolBarCtrl)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//  CBrowserToolbarCtrl 消息处理程序

void CBrowserToolbarCtrl::Init()
{
	ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER |CCS_NORESIZE);


	CImageList ImageList;
	ImageList.Create( 16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, NUM_BROWSER_BUTTON,1);
	ImageList.Add(CTempIconLoader(_T("BTBACK"),16 ,16));
	ImageList.Add(CTempIconLoader(_T("BTFORWARD"),16 ,16));
	ImageList.Add(CTempIconLoader(_T("BTSTOP"),16 ,16));
	ImageList.Add(CTempIconLoader(_T("BTREFRESH"),16,16));
	ImageList.Add(CTempIconLoader(_T("BTVERYCD"),16 ,16));
	ASSERT( ImageList.GetImageCount() == NUM_BROWSER_BUTTON );

	CImageList* pimlOld = SetImageList(&ImageList);
	ImageList.Detach();
	if (pimlOld)
		pimlOld->DeleteImageList(); 

    TBButtons[0].idCommand = TB_BACK;
	TBButtons[1].idCommand = TB_FORWARD;
	TBButtons[2].idCommand = TB_STOP;
	TBButtons[3].idCommand = TB_REFRESH;
	TBButtons[4].idCommand = TB_HOME;

	// add button-text:
	TCHAR cButtonStrings[500];
	int lLen, lLen2;
	
	_tcscpy(cButtonStrings, GetResString(IDS_BROWSER_BACK));
	lLen = _tcslen(GetResString(IDS_BROWSER_BACK)) + 1;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_FORWARD)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_FORWARD), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_STOP)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_STOP), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_REFRESH)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_REFRESH), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	lLen2 = _tcslen(GetResString(IDS_BROWSER_VERYCD)) + 1;
	memcpy(cButtonStrings+lLen, GetResString(IDS_BROWSER_VERYCD), lLen2*sizeof(TCHAR));
	lLen += lLen2;

	// terminate
	memcpy(cButtonStrings+lLen, _T("\0"), sizeof(TCHAR));
	
	int iRet =AddStrings(cButtonStrings);

	TBBUTTON sepButton = {0};
	sepButton.idCommand = 0;
	sepButton.fsStyle = TBSTYLE_SEP;
	sepButton.fsState = TBSTATE_ENABLED;
	sepButton.iString = -1;
	sepButton.iBitmap = -1;
	AddButtons(1,&sepButton);
    
	for( int i = 0; i < NUM_BROWSER_BUTTON; i++ )
	{
		TBButtons[i].iBitmap = i;
		TBButtons[i].fsState = TBSTATE_ENABLED;
		TBButtons[i].fsStyle = TBSTYLE_BUTTON | TBSTATE_ELLIPSES ;
		TBButtons[i].iString = i;
		AddButtons(1, &TBButtons[i]);
	}

	
	AddButtons(1,&sepButton);



}

void CBrowserToolbarCtrl::OnSize(UINT nType, int cx, int cy)
{
	CToolBarCtrl::OnSize(nType, cx, cy);
	CToolBarCtrl::AutoSize();
}

void CBrowserToolbarCtrl::Localize()
{
	static const int TBStringIDs[] =
	{
		IDS_BROWSER_BACK, 
		IDS_BROWSER_FORWARD,
		IDS_BROWSER_STOP,
		IDS_BROWSER_REFRESH,
		IDS_BROWSER_VERYCD //Added by GGSoSo for webbrowser
	};
	TBBUTTONINFO tbi;
	tbi.dwMask = TBIF_TEXT;
	tbi.cbSize = sizeof(TBBUTTONINFO);

	for (int i = 0; i < NUM_BROWSER_BUTTON; i++)
	{
		_sntprintf(TBStrings[i], ARRSIZE(TBStrings[0]), _T("%s"), GetResString(TBStringIDs[i]));
		tbi.pszText = TBStrings[i];
		SetButtonInfo(IDC_BROWSERBUTTON+i, &tbi);
	}

}