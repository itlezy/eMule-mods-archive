#include "bitcomet_inc.h"
#include "langstring.h"

CLangString::LangIndex CLangString::m_lang_index = LANG_EN;

TCHAR* CLangString::m_string_list[][LANG_NUM] = 
{
	//S_HELP
	_T("This program is used to run BitComet plugins."),
	_T("��������������BitComet�����"),

	////S_ADD_PORT_MAP_SUCCEED,
	//_T("Add port mapping succeed!"),
	//_T("��Ӷ˿�ӳ��ɹ���"),

	////S_ADD_PORT_MAP_FAIL,
	//_T("Add port mapping failed!"),
	//_T("��Ӷ˿�ӳ��ʧ�ܣ�"),

	////S_DELETE_PORT_MAP_SUCCEED,
	//_T("Delete port mapping succeed!"),
	//_T("ɾ���˿�ӳ��ɹ���"),

	////S_DELETE_PORT_MAP_FAIL,
	//_T("Delete port mapping failed!"),
	//_T("ɾ���˿�ӳ��ʧ�ܣ�"),

	////S_ADD_FIREWALL_PORT_SUCCEED,
	//_T("Add firewall port succeed!"),
	//_T("��ӷ���ǽ�˿ڳɹ���"),

	////S_ADD_FIREWALL_PORT_FAIL,
	//_T("Add firewall port failed!"),
	//_T("��ӷ���ǽ�˿�ʧ�ܣ�"),

	////S_DELETE_FIREWALL_PORT_SUCCEED,
	//_T("Delete firewall portg succeed!"),
	//_T("ɾ������ǽ�˿ڳɹ���"),

	////S_DELETE_FIREWALL_PORT_FAIL,
	//_T("Delete firewall port failed!"),
	//_T("ɾ������ǽ�˿�ʧ�ܣ�"),

};