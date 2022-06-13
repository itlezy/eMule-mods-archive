#pragma once
#include "enbitmap.h"
#include <afxhtml.h>

class CHtmlCtrl : public CHtmlView //Anis->5 righe di codice contro 2 moduli interi .h e cpp che effettuavano la stessa operazione. Secondo voi sono stato bravo?
{
public:
	BOOL Create(const RECT& rc, CWnd* pParent, UINT nID, DWORD dwStyle = WS_CHILD|WS_VISIBLE, CCreateContext* pContext = NULL)
	{
		return CHtmlView::Create(NULL, NULL, dwStyle, rc, pParent, nID, pContext);
	}
};

class CSplashScreen : public CDialog
{
	DECLARE_DYNAMIC(CSplashScreen)
public:
	CSplashScreen(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSplashScreen();

// Dialog Data
	enum { IDD = IDD_SPLASH };

protected:
	CBitmap m_imgSplash;
    CHtmlCtrl*   m_phtmlAds;
    CEnBitmap*   m_pimgSplash; //Mod Adu - Ded - Splash screen remoto

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	void OnPaint(); 
};
