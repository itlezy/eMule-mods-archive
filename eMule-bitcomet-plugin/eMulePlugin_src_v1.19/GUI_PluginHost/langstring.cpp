#include "bitcomet_inc.h"
#include "langstring.h"

CLangString::LangIndex CLangString::m_lang_index = LANG_EN;

TCHAR* CLangString::m_string_list[][LANG_NUM] = 
{
	//S_HELP
	_T("This program is used to run BitComet plugins."),
	_T("本程序用于运行BitComet插件。"),

	////S_ADD_PORT_MAP_SUCCEED,
	//_T("Add port mapping succeed!"),
	//_T("添加端口映射成功！"),

	////S_ADD_PORT_MAP_FAIL,
	//_T("Add port mapping failed!"),
	//_T("添加端口映射失败！"),

	////S_DELETE_PORT_MAP_SUCCEED,
	//_T("Delete port mapping succeed!"),
	//_T("删除端口映射成功！"),

	////S_DELETE_PORT_MAP_FAIL,
	//_T("Delete port mapping failed!"),
	//_T("删除端口映射失败！"),

	////S_ADD_FIREWALL_PORT_SUCCEED,
	//_T("Add firewall port succeed!"),
	//_T("添加防火墙端口成功！"),

	////S_ADD_FIREWALL_PORT_FAIL,
	//_T("Add firewall port failed!"),
	//_T("添加防火墙端口失败！"),

	////S_DELETE_FIREWALL_PORT_SUCCEED,
	//_T("Delete firewall portg succeed!"),
	//_T("删除防火墙端口成功！"),

	////S_DELETE_FIREWALL_PORT_FAIL,
	//_T("Delete firewall port failed!"),
	//_T("删除防火墙端口失败！"),

};