// PluginHostDlg.cpp : ʵ���ļ�
//

#include "bitcomet_inc.h"

#include "Core_eMule_include/InterfaceEMulePlugin.h"
#include "Core_Common_include/string_path.h"
#include "Core_Common_include/System.h"
#include "Core_Common_include/FileSystem.h"
#include "Core_Common_include/InterfaceMessage.h"
#include "Core_Common_include/InterfaceTimer.h"
#include "Core_Common_include/string_conv.h"
#include "Core_Common_include/string_url.h"
#include "Core_Common_include/InterfaceExceptionDebugger.h"
using namespace Core_Common;

#include "Registry.h"
#include "PluginHost.h"
#include "PluginHostDlg.h"
#include "BCHostCallback.h"


// CPluginHostDlg �Ի���
bool CPluginHostDlg::m_is_debug_mode = false;

CPluginHostDlg::CPluginHostDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPluginHostDlg::IDD, pParent)
	, m_tcp_port(0)
	, m_udp_port(0)
	, m_is_show_emule(false)
	, m_is_show_host_wnd(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CPluginHostDlg::~CPluginHostDlg()
{

}

void CPluginHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPluginHostDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_REGISTERED_MESSAGE(WM_WHERE_ARE_YOU, onWhereAreYou)
	ON_BN_CLICKED(IDC_BUTTON1, &CPluginHostDlg::OnBnClickedButton1)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


// CPluginHostDlg ��Ϣ�������

LRESULT CPluginHostDlg::onWhereAreYou(WPARAM wParam, LPARAM /*lParam*/) 
{
	// ��ֱ�����е�¿���exe�ļ�ʱ����ʾ������
	BOOL show_window = wParam;
	if(show_window)
	{
		Core_eMule::InterfaceEMulePlugin::show_emule_dlg(true);	
	}

	return WM_WHERE_ARE_YOU;
}

BOOL CPluginHostDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// process cmd line args
	process_cmd_line();

	// new install process
	new_install_process();

	// load eMule plugin 
	bool load_ok = load_plugin_emule();
	if(!load_ok)
	{
		PostMessage(WM_QUIT);
		return TRUE;
	}

	static BCHostCallBack bchost_callback;
	Core_eMule::InterfaceEMulePlugin::m_callback = &bchost_callback;
	bchost_callback.set_host_wnd(GetSafeHwnd());

	theApp.set_startup_event();

	SetTimer(0, 100, NULL);

	return FALSE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}


void CPluginHostDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CPluginHostDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CPluginHostDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPluginHostDlg::OnDestroy()
{
	// ע�ⲻҪ�� CWnd::OnClose() ������ģ�飬�ú�����OnOK()��OnCancle()ʱ��������

	// unload emule plugin
	Core_eMule::InterfaceEMulePlugin::dll_stop();
	Core_eMule::InterfaceEMulePlugin::dll_unload();

	CDialog::OnDestroy();
}

void CPluginHostDlg::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == 0)
	{
		InterfaceMessage::time_tick();
		InterfaceTimer::time_tick();
	}
}

bool CPluginHostDlg::load_plugin_emule()
{
	tpath_t emule_path = System::system_app_path() / _T("plugin_eMule.dll");
#ifdef _DEBUG
	tpath_t emule_path_debug = _T("D:\\Develop\\Core_eMule\\eMulePlugin\\app\\debug_unicode\\plugin_eMule.dll");
	if(FileSystem::file_is_existed(emule_path_debug))
		emule_path = emule_path_debug;
#endif
	bool load_ok = Core_eMule::InterfaceEMulePlugin::dll_load(emule_path.get_string());
	if(!load_ok)
		return false;

	Core_eMule::InterfaceEMule::connection_setting_t setting;
	setting.listen_port_tcp = m_tcp_port;
	setting.listen_port_udp = m_udp_port;
	Core_eMule::InterfaceEMulePlugin::set_connection_setting(setting);

	// #1917 (�����û��޸ĵ�¿���������Ŀ¼)
	//tpath_t emule_path_incoming = FileSystem::system_app_path() / _T("Incoming");
	//tpath_t emule_path_temp		= FileSystem::system_app_path() / _T("Temp");
	//Core_eMule::InterfaceEMulePlugin::set_download_path(emule_path_incoming.get_string(), emule_path_temp.get_string());

	Core_eMule::InterfaceEMulePlugin::dll_start();

	if(m_is_show_emule)
	{
		Core_eMule::InterfaceEMulePlugin::show_emule_dlg(true);	
	}

	if( !m_strPendingLink.empty() )
	{
		string_fixed16 file_hash;
		Core_eMule::InterfaceEMulePlugin::add_task(m_strPendingLink, file_hash);

		Core_eMule::InterfaceEMulePlugin::show_emule_dlg(true);	
	}

	return true;
}

void CPluginHostDlg::process_cmd_line()
{
	LPWSTR *szArglist;
	int nArgs;
	int i;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if( NULL == szArglist )
	{
		return;
	}
	
	for( i=0; i<nArgs; i++) 
	{
		wstring arg = szArglist[i];

		wstring token = L"/tcp_port=";
		if(arg.find(token) != wstring::npos)
		{
			wstring value = arg.substr(token.size());
			int n = string_conv::to_number(value, 0);
			if(n != 0)
			{
				m_tcp_port = (uint16_t)n;
			}
		}

		token = L"/udp_port=";
		if(arg.find(token) != wstring::npos)
		{
			wstring value = arg.substr(token.size());
			int n = string_conv::to_number(value, 0);
			if(n != 0)
			{
				m_udp_port = (uint16_t)n;
			}
		}

		token = L"/debug";
		if(arg.find(token) != wstring::npos)
		{
			m_is_debug_mode = true;
			m_is_show_emule = true;
		}

		token = L"/show";
		if(arg.find(token) != wstring::npos)
		{
			m_is_show_emule = true;
		}

		token = L"/host_wnd";
		if(arg.find(token) != wstring::npos)
		{
			m_is_show_host_wnd = true;
		}
	}

	if(nArgs == 1)
	{
		m_is_show_emule = true;
	}

	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);

}


void CPluginHostDlg::OnClose()
{
	PostMessage(WM_QUIT);

	//CDialog::OnClose();
}

void CPluginHostDlg::OnOK()
{
	CDialog::OnClose();
	PostMessage(WM_QUIT);

	//CDialog::OnOK();
}

void CPluginHostDlg::OnCancel()
{
	CDialog::OnClose();
	PostMessage(WM_QUIT);

	//CDialog::OnCancel();
}

void CPluginHostDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	//Core_eMule::InterfaceEMulePlugin::search_file(_T("�ļ�(����"), 8589934590l);

	AfxThrowMemoryException();
}

void CPluginHostDlg::new_install_process()
{
	bool is_new_install = CRegistry::GetEMuleNewInstallFlag();
	if(!is_new_install)
		return;

	CRegistry::ClearEMuleNewInstallFlag();

	wstring install_lang = CRegistry::GetEMuleInstallLang();
	if(!install_lang.empty())
	{
		if (install_lang == L"2052" ||	// zh_cn
			install_lang == L"1028" ||	// zh_tw
			install_lang == L"1033" )	// en_us
		{
			update_config_file(string_conv::to_string(install_lang));
		}
	}
}

void CPluginHostDlg::update_config_file(const string& lang_id)
{
	tpath_t ini_file_path = System::system_app_path();
	ini_file_path /= _T("config/preferences.ini");
	if(!FileSystem::file_is_existed(ini_file_path))
	{
		string content = "[eMule]\r\nLanguage=" + lang_id;
		FileSystem::file_write(ini_file_path, content);
		return;
	}

	string content;
	if(!FileSystem::file_read(ini_file_path, content))
		return;

	vector<string> lines;
	Core_Common::string_conv::split(content, "\n\r", lines);

	string out;
	for(vector<string>::iterator iter = lines.begin(); iter!= lines.end(); iter++)
	{
		string& line = *iter;
		Core_Common::string_conv::trim(line, "\r\n\t " );

		string tag = "Language=";
		if(line.substr(0, tag.size()) == tag)
		{
			line = tag + lang_id;
		}

		if(!out.empty())
			out += "\r\n";
		out += line;
	}

	Core_Common::string_conv::trim(out, "\r\n\t " );

	FileSystem::file_write(ini_file_path, out);
}

LRESULT CPluginHostDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// overload this function to avoid all CException* catched by MFC in AfxCallWndProc() which calls this function.
	boost::function<LRESULT(void)> func = boost::bind(&CPluginHostDlg::WindowProcInternal,this, message, wParam, lParam);
	return Core_Common::InterfaceExceptionDebugger::call_with_exception_debugger( &func, LRESULT(0) );
}

LRESULT CPluginHostDlg::WindowProcInternal(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CPluginHostDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	if(pCopyDataStruct && pCopyDataStruct->lpData)
	{
		tstring cmd_line = (LPCTSTR)pCopyDataStruct->lpData;
		string ed2k_link_utf8 = string_conv::to_single_byte_utf8(cmd_line);
		tstring ed2k_link = string_conv::to_tstring_from_utf8(ed2k_link_utf8);

		string_fixed16 file_hash;
		Core_eMule::InterfaceEMulePlugin::add_task(ed2k_link, file_hash);
	}

	return TRUE;
}
