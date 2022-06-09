/////////////////////////////////////////////////////////
// XMLMsgs.h - definition of all XML messages
// (communication between Engine and GUI
//
// NOTE Don't put <#pragma once> directive here. 
// This file should be included multiple times.

BEGIN_XML_CMD(hello)
	XML_PARAM_STRING(Client)
END_XML_CMD


BEGIN_XML_CMD(quit)
END_XML_CMD


BEGIN_XML_CMD(connect)
	XML_PARAM_STRING(Addr)
	XML_PARAM_DWORD(Port)
END_XML_CMD


BEGIN_XML_CMD(disconnect)
END_XML_CMD


BEGIN_XML_CMD(get_logs)
	XML_PARAM_DWORD(TypeMin)
	XML_PARAM_DWORD(TypeMax)
	XML_INTERNAL_VAR(int m_nState, m_nState = XML_GET_START)
	XML_INTERNAL_VAR(XmlDoc m_spDoc, )
END_XML_CMD


BEGIN_XML_CMD(get_shared)
	XML_INTERNAL_VAR(int m_nState, m_nState = XML_GET_START)
	XML_INTERNAL_VAR(XmlDoc m_spDoc, )
END_XML_CMD


BEGIN_XML_CMD(set_param)
	XML_PARAM_DWORD(Type)
	XML_PARAM_DWORD(Value)
END_XML_CMD

// Events
BEGIN_XML_CMD(server_connected)
END_XML_CMD

BEGIN_XML_CMD(server_disconnected)
END_XML_CMD
