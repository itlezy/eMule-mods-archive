// PluginHostDlg.h : ͷ�ļ�
//

#pragma once

// ����ʶ����Ϣ 
#define WM_WHERE_ARE_YOU_MSG _T("WM_WHERE_ARE_YOU-{E67CC07E-ACF5-42b5-8AFF-6DE349BDFC6B}")
const UINT WM_WHERE_ARE_YOU = ::RegisterWindowMessage( WM_WHERE_ARE_YOU_MSG );

// CPluginHostDlg �Ի���
class CPluginHostDlg : public CDialog
{
// ����
public:
	CPluginHostDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CPluginHostDlg();

// �Ի�������
	enum { IDD = IDD_PLUGINHOST_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg LRESULT onWhereAreYou(WPARAM wParam, LPARAM /*lParam*/);

	DECLARE_MESSAGE_MAP()

public:
	static bool is_debug_mode() {return m_is_debug_mode;};
	bool is_show_host_wnd() {return m_is_show_host_wnd;};

protected:
	void new_install_process();
	void update_config_file(const string& lang_id);

	bool load_plugin_emule();
	uint16_t	m_tcp_port;
	uint16_t	m_udp_port;
	bool		m_is_show_emule;
	bool		m_is_show_host_wnd;
	static bool	m_is_debug_mode;

	void process_cmd_line();

public:
	afx_msg void OnClose();
protected:
	virtual void OnOK();
	virtual void OnCancel();
public:
	afx_msg void OnBnClickedButton1();
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProcInternal(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

public:
	tstring		m_strPendingLink;
};
