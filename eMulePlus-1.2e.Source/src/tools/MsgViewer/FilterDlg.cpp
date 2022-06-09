// FilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MsgViewer.h"
#include "MsgViewerDlg.h"
#include "FilterDlg.h"
#include <afxtempl.h>

#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFilter g_stFilter;

void CFilter::Open()
{
	m_bIn = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_In"), TRUE);
	m_bOut = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_Out"), TRUE);
	m_bNeither = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_Neither"), TRUE);
	m_bUnknown = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_Unknown"), TRUE);
	m_bTypeServ = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_TServ"), TRUE);
	m_bTypePeer = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_TPeer"), TRUE);

//	g_strRemote = AfxGetApp()->GetProfileString(CMsgViewerDlg::s_szSection, _T("F_Remote"));
//	g_nRemoteMeaning = AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, _T("F_RemoteF"), 1);
//
	m_mapFilter.RemoveAll();
	CEntry stEntry;
	CString strTxt;
	ULONG nIdProtoType;

//#include "../../Engine/Sockets/EmUndef.h"

#define BEGIN_OPCODE(id, name, proto, type) { \
	stEntry.m_szName = #name; \
	nIdProtoType = (T_CLIENT_##type << 16) | (OP_PROT_##proto << 8) | id; \
	strTxt.Format("F_Msg%u", nIdProtoType); \
	stEntry.m_bAllowed = (0 != AfxGetApp()->GetProfileInt(CMsgViewerDlg::s_szSection, strTxt, TRUE));\
	m_mapFilter.SetAt(nIdProtoType, stEntry); }
#define END_OPCODE

#undef PARAM_BYTE
#define PARAM_BYTE(name)
#undef PARAM_DWORD
#define PARAM_DWORD(name)
#undef PARAM_USHORT
#define PARAM_USHORT(name)

#define PARAM_BYTE_JUNK(value)
#define PARAM_BUF(name, type, count)
#define PARAM_DATABLOCK(name)
#define PARAM_SIMPLE(name, type)
#define PARAM_TAGS_BEGIN
#define PARAM_TAGS_END
#define PARAM_TAG_STR(name, id)
#define PARAM_TAG_DWORD(name, id)
#define PARAM_ARRAY(name, type, counter)
#define PARAM_BITARRAY(name)
#define PARAM_COMPLEXARRAY_BEGIN(msg, name)
#define PARAM_COMPLEXARRAY_END(name)


#include "../../Engine/Sockets/EmMsgs.h"

	//m_arrRemote.RemoveAll();
	//ArrFromStr(m_strRemote, &m_arrRemote);
}

void CFilter::Save()
{
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_In"), m_bIn));
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_Out"), m_bOut));
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_Neither"), m_bNeither));
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_Unknown"), m_bUnknown));
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_Group"), m_bTypeServ));
	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_GroupInternals"), m_bTypePeer));

//	VERIFY(AfxGetApp()->WriteProfileString(CMsgViewerDlg::s_szSection, _T("F_TServ"), m_strRemote));
//	VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, _T("F_TPeer"), m_nRemoteMeaning));

	// save messages
	for (POSITION pos = m_mapFilter.GetStartPosition(); pos; )
	{
		ULONG nIdProtoType = 0;
		CEntry stEntry;
		m_mapFilter.GetNextAssoc(pos, nIdProtoType, stEntry);

		CString strTxt;
		strTxt.Format(_T("F_Msg%u"), nIdProtoType);

		VERIFY(AfxGetApp()->WriteProfileInt(CMsgViewerDlg::s_szSection, strTxt, stEntry.m_bAllowed));
	}
}

/*BOOL CFilter::ArrFromStr(PCTSTR wszStr, CArray<DWORD, DWORD&>* parrDest)
{
	while (*wszStr)
	{
		DWORD dwVal = _tcstoul(wszStr, (PTSTR*) &wszStr, 0);
		if (_T(',') == *wszStr)
			wszStr++;
		else
			if (*wszStr)
				return FALSE; // invalid!

		// add this value
		if (parrDest)
			parrDest->Add(dwVal);
	}
	return TRUE;
}
*/

BOOL CFilter::AllowMsg(MSG_FILE_INFO::MSG_DATA& stData)
{
	switch (stData.m_bOut)
	{
	case 0:
		if (!m_bIn)
			return FALSE;
		break;
	case 1:
		if (!m_bOut)
			return FALSE;
		break;
	default:
		if (!m_bNeither)
			return FALSE;
	}

	CEntry stEntry;
	if (m_mapFilter.Lookup(stData.m_nIdProtoType, stEntry))
		if (stEntry.m_bAllowed)
			stData.m_szName = stEntry.m_szName;
		else
			return FALSE;
	else
		if (m_bUnknown)
			stData.m_szName = _T("<Unknown>");
		else
			return FALSE;

//	// 'RemoteID' parameter
//	for (int nPos = 0; nPos < g_arrRemote.GetSize(); nPos++)
//		if (g_arrRemote[nPos] == stData.m_nSurfaceID)
//			break; // found
//
//	if ((g_arrRemote.GetSize() == nPos) ^ g_nRemoteMeaning)
//		return FALSE;

	// Parse the IdProtoType field;
	stData.m_nID = (UCHAR) stData.m_nIdProtoType;
	stData.m_nProto = (UCHAR) (stData.m_nIdProtoType >> 8);
	stData.m_nType = stData.m_nIdProtoType >> 16;

	return TRUE; // all passed
}

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog


CFilterDlg::CFilterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFilterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nSel_KeepAlive = 0;
	m_nSel_Disconnect = 0;
}


void CFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilterDlg)
	DDX_Control(pDX, IDC_MESSAGES, m_wndMessages);
	//}}AFX_DATA_MAP

	DDX_Check(pDX, IDC_KEEPALIVE, m_nSel_KeepAlive);
	DDX_Check(pDX, IDC_CONNECTION_BREAK, m_nSel_Disconnect);
	DDX_Check(pDX, IDC_UNKNOWN, g_stFilter.m_bUnknown);

	DDX_Check(pDX, IDC_INCOMING, g_stFilter.m_bIn);
	DDX_Check(pDX, IDC_OUTCOMING, g_stFilter.m_bOut);
	DDX_Check(pDX, IDC_NEITHER, g_stFilter.m_bNeither);

	DDX_Check(pDX, IDC_TYPE_SERV, g_stFilter.m_bTypeServ);
	DDX_Check(pDX, IDC_TYPE_PEER, g_stFilter.m_bTypePeer);

//	DDX_Text(pDX, IDC_REMOTE_LIST, g_stFilter.m_strRemote);
//	DDX_Radio(pDX, IDC_INC, g_stFilter.m_nRemoteMeaning);

}


BEGIN_MESSAGE_MAP(CFilterDlg, CDialog)
	//{{AFX_MSG_MAP(CFilterDlg)
	ON_LBN_SELCHANGE(IDC_MESSAGES, OnSelchangeMessages)
	ON_BN_CLICKED(IDC_SELECTALL, OnSelectall)
	ON_BN_CLICKED(IDC_DESELECT, OnDeselectAll)
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDC_KEEPALIVE, OnCheckButton)
	ON_BN_CLICKED(IDC_CONNECTION_BREAK, OnCheckButton)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg message handlers

BOOL CFilterDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// insert all items
	for (POSITION pos = g_stFilter.m_mapFilter.GetStartPosition(); pos; )
	{
		ULONG nIdProtoType;
		CFilter::CEntry stEntry;
		g_stFilter.m_mapFilter.GetNextAssoc(pos, nIdProtoType, stEntry);

		int nListPos = m_wndMessages.AddString(stEntry.m_szName);
		ASSERT(LB_ERR != nListPos);

		VERIFY(m_wndMessages.SetSel(nListPos, stEntry.m_bAllowed) != LB_ERR);
		VERIFY(m_wndMessages.SetItemData(nListPos, nIdProtoType) != LB_ERR);
	}

	ProcessButtons(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


#define HandleButton(IDC_THIS_BUTTON, nButtonValue, pButtonMsgArray) \
	if (!nButtonID || (IDC_THIS_BUTTON == nButtonID)) \
		ProcessSingleButton(nButtonID, nButtonValue, pButtonMsgArray, _countof(pButtonMsgArray));

void CFilterDlg::ProcessButtons(UINT nButtonID)
{
	if (nButtonID && !UpdateData())
		return;

	int nFirstPos = m_wndMessages.GetTopIndex();
	VERIFY(m_wndMessages.LockWindowUpdate());

//	HandleButton(IDC_KEEPALIVE, m_nSel_KeepAlive, g_pKeepAlive)
//	HandleButton(IDC_CONNECTION_BREAK, m_nSel_Disconnect, g_pDisconnect)


/*	HandleButton(IDC_KEEPALIVE, m_nSel_IAA, g_pIAA)

*/
	UpdateData(FALSE);
	VERIFY(m_wndMessages.SetTopIndex(nFirstPos) != LB_ERR);
	m_wndMessages.UnlockWindowUpdate();

	if (nButtonID)
		ProcessButtons(0);
}

void CFilterDlg::ProcessSingleButton(BOOL bApply, int &nButton, const USHORT* pMsg, UINT nCount)
{
	ASSERT(pMsg && nCount);

	if (bApply)
	{
		if (2 == nButton)
			return; // don't touch this button

		for (UINT nIndex = 0; nIndex < nCount; nIndex++)
			MsgSelect(pMsg[nIndex], nButton);


	} else
	{
		UINT nMatch = 0;
		for (UINT nIndex = 0; nIndex < nCount; nIndex++)
			if (MsgIsSelected(pMsg[nIndex]))
				nMatch++;

		nButton = (nMatch == nCount) ? 1 : nMatch ? 2 : 0;
	}
}

BOOL CFilterDlg::MsgIsSelected(DWORD dwMsg)
{
	// Find this item
	for (int nPos =  0; nPos < m_wndMessages.GetCount(); nPos++)
		if (m_wndMessages.GetItemData(nPos) == dwMsg)
			return (m_wndMessages.GetSel(nPos) > 0);

	return FALSE; // not found
}

void CFilterDlg::MsgSelect(DWORD dwMsg, BOOL bSelect)
{
	// Find this item
	for (int nPos =  0; nPos < m_wndMessages.GetCount(); nPos++)
		if (m_wndMessages.GetItemData(nPos) == dwMsg)
		{
			VERIFY(m_wndMessages.SetSel(nPos, bSelect) != LB_ERR);
			break;
		}

}

void CFilterDlg::OnSelchangeMessages() 
{
	ProcessButtons(0);	
}


void CFilterDlg::OnSelectall() 
{
	if (!UpdateData())
		return;

	for (int nPos =  0; nPos < m_wndMessages.GetCount(); nPos++)
		VERIFY(m_wndMessages.SetSel(nPos) != LB_ERR);
	ProcessButtons(0);
}

void CFilterDlg::OnDeselectAll() 
{
	if (!UpdateData())
		return;

	for (int nPos =  0; nPos < m_wndMessages.GetCount(); nPos++)
		VERIFY(m_wndMessages.SetSel(nPos, FALSE) != LB_ERR);
	ProcessButtons(0);
}

void CFilterDlg::OnOK() 
{
	// check to / from values
	if (!UpdateData())
		return;

//	if (!g_stFilter.ArrFromStr(FILTER::g_strRemote, NULL))
//	{
//		AfxMessageBox(_T("\"Remote ID\" control contains invalid data"));
//		return;
//	}

	// must also store all messages states
	for (int nPos =  0; nPos < m_wndMessages.GetCount(); nPos++)
	{
		DWORD dwItem = m_wndMessages.GetItemData(nPos);
		ASSERT(dwItem);

		CFilter::CEntry stEntry;
		VERIFY(g_stFilter.m_mapFilter.Lookup(dwItem, stEntry));

		stEntry.m_bAllowed = 0 != m_wndMessages.GetSel(nPos);
		g_stFilter.m_mapFilter.SetAt(dwItem, stEntry);
	}

	CDialog::OnOK();

	g_stFilter.Save(); // right now!
	g_stFilter.Open();
}

void CFilterDlg::OnCheckButton()
{
	// check which button has the focus
	CWnd* pWnd = GetFocus();
	ASSERT(pWnd);

	if (((CButton*) pWnd)->GetCheck() == 2)
		((CButton*) pWnd)->SetCheck(0);

	ProcessButtons((UINT) pWnd->GetDlgCtrlID());
}
