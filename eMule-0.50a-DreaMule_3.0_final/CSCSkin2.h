// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// ISCSkin2 wrapper class

class ISCSkin2 : public COleDispatchDriver
{
public:
	ISCSkin2() {}		// Calls COleDispatchDriver default constructor
	ISCSkin2(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	ISCSkin2(const ISCSkin2& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	void InitDecoration(long mode);
	void LoadSkinFromFile(LPCTSTR path);
	void DoNotDecorate(long hWnd);
	void DoDecorate(long hWnd);
	void IncludeWnd(long hWnd, long withchildren);
	void ExcludeWnd(long hWnd, long withchildren);
	void DeInitDecoration();
	void DecorateAs(long hWnd, long type);
	void AddDrawText(long hWnd, LPCTSTR bsTxt, long left, long top, long right, long bottom, long nFormat, short nBkMode, long clrText, long clrBk, long hFont, short nID);
	void RemoveDrawItem(long hWnd, short nID);
	void AddDrawImage(long hWnd, long hImage, long left, long top, long right, long bottom, short bBlit, short nBkMode, long clrBk, short nID);
	void UpdateControl(long nID);
	void InitLicenKeys(LPCTSTR reg_name, LPCTSTR company, LPCTSTR email, LPCTSTR licenkey);
	void UpdateWindow(long hWnd);
	void RemoveSkin();
	void DefineLanguage(long langID);
	void ApplySkin();
	void GetSkinCopyRight(LPCTSTR skinpath, BSTR* name, BSTR* author, BSTR* date, BSTR* email);
	void SetCustomSkinWnd(long hWnd, LPCTSTR skinName, long isFrame);
	void AddAdditionalThread();
	void DeleteAdditionalThread();
	void AddSkinFromFile(LPCTSTR path, short sID);
	void ApplyAddedSkin(long hWnd, short sID);
	void RemoveAddedSkin(short sID);
	void SetCustomScrollbars(long hWnd, LPCTSTR skinName);
	void SetAddedCustomScrollbars(long hWnd, short sID, LPCTSTR skinName);
	void SetAddedCustomSkinWnd(long hWnd, short sID, LPCTSTR skinName, long isFrame);
	void LoadSkinFromResource(long hModule, long hrsrcResInfo);
	void GetUserDataSize(LPCTSTR skinName, long lptrSize);
	void GetUserData(LPCTSTR skinName, long lptrData, long lDataSize);
};
