#pragma once

// ALL WM_USER messages are to be declared here *after* 'WM_FIRST_EMULE_USER_MSG'
enum EUserWndMessages
{
	// *) Do *NOT* place any message *before* WM_FIRST_EMULE_USER_MSG !!
	// *) Do *NOT* use any WM_USER messages in the range WM_USER - WM_USER+0x100 !!
	UM_FIRST_EMULE_USER_MSG = (WM_USER + 0x100 + 1),

	// Taskbar
	UM_TASKBARNOTIFIERCLICKED,
	UM_TRAY_ICON_NOTIFY_MESSAGE,
	UM_CLOSE_MINIMULE,

	// Webserver
	WEB_GUI_INTERACTION,
	WEB_CLEAR_COMPLETED,
	WEB_FILE_RENAME,
	WEB_ADDDOWNLOADS,
	WEB_CATPRIO,
	WEB_ADDREMOVEFRIEND,
	WEB_COPYDATA, // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	// VC
	UM_VERSIONCHECK_RESPONSE,

	// PC
	UM_PEERCHACHE_RESPONSE,

	UM_CLOSETAB,
	UM_QUERYTAB,
	UM_DBLCLICKTAB,

	UM_CPN_SELCHANGE,
	UM_CPN_DROPDOWN,
	UM_CPN_CLOSEUP,
	UM_CPN_SELENDOK,
	UM_CPN_SELENDCANCEL,

	UM_MEDIA_INFO_RESULT,
	UM_ITEMSTATECHANGED,
	UM_SPN_SIZED,
	UM_TABMOVED,
	UM_TREEOPTSCTRL_NOTIFY,
	UM_DATA_CHANGED,
	UM_OSCOPEPOSITION,
	UM_DELAYED_EVALUATE,
	UM_ARCHIVESCANDONE,
	//Xman versions check
	UM_MVERSIONCHECK_RESPONSE,
	UM_DLPVERSIONCHECK_RESPONSE, //Xman DLP
	//Xman end
	// ==> ScarAngel Version Check - Stulle
	UM_SVERSIONCHECK_RESPONSE,
	// <== ScarAngel Version Check - Stulle
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	UM_DLPAUTOVERCHECK_RESPONSE,
	UM_IPFFILTERAUTOVERCHECK_RESPONSE,
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	UM_SERVERSTATUS // Run eMule as NT Service [leuk_he/Stulle] - Stulle

	// UPnP
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	UM_UPNP_RESULT
	*/
	// <== UPnP support [MoNKi] - leuk_he
};
