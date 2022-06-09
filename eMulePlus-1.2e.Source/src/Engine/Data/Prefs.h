// Prefs.h: interface for the CPrefs class.
//
//////////////////////////////////////////////////////////////////////

#include "../../Preferences.h"

class CPrefs : public CPreferences
{
public:
	CPrefs();
	virtual ~CPrefs();
	void LoadPrefs();
	void SavePrefs();

	// Connection
	__declspec(property(get=_GetPortXml))	USHORT	PortXml;
	__declspec(property(get=_GetXmlLocal))	bool	XmlLocalBind;

	USHORT	_GetPortXml()	const	{ return m_uPortXml; }
	bool	_GetXmlLocal()	const	{ return m_bXmlLocalBind; }

	// Upload
	__declspec(property(get=_GetMaxUpSlots,put=_PutMaxUpSlots))	USHORT MaxUploadSlots;

	USHORT _GetMaxUpSlots() const;

	void _PutMaxUpSlots(USHORT uSlots){ m_uMaxUploadSlots = uSlots; }

	// Debug
	__declspec(property(get=_GetAlertOnErrors))	bool AlertOnErrors;
	__declspec(property(get=_GetSaveLogsIO))		bool SaveLogsIO;

	bool _GetAlertOnErrors()	const	{ return m_bAlertOnErrors; }
	bool _GetSaveLogsIO()		const	{ return m_bSaveLogsIO; }

protected:
	// Connection
	USHORT	m_uPortXml;
	bool	m_bXmlLocalBind;

	// Upload
	USHORT	m_uMaxUploadSlots;

	// Debug
	bool	m_bAlertOnErrors;
	bool	m_bSaveLogsIO;
};