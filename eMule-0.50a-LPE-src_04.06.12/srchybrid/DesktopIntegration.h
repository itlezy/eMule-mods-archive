// netfinity: Better support for WINE Gnome/KDE desktops

#ifndef DESKTOPINTEGRATION_H
#define DESKTOPINTEGRATION_H

#include <windef.h>
#include <WinSafer.h>
  
// WINE support 
extern "C" {
	typedef	char* (FAR PASCAL *t_wine_get_unix_file_name)(const LPCWSTR dosW);
	typedef LPCWSTR (FAR PASCAL *t_wine_get_dos_file_name)(const char* dosA);
} 

class CDesktopIntegration
{
public:
			CDesktopIntegration();
			~CDesktopIntegration();
	BOOL	ShellExecute(LPCTSTR lpOperation, LPCTSTR lpFile);
protected:
	// WINE support 
	enum EDesktop
	{
		WINDOWS_DESKTOP = 0,
		GNOME_DESKTOP,
		KDE_DESKTOP
	};
	EDesktop	_GetDesktopType() {return m_eDesktop;}
	CString		_NativeToWindowsPath(CStringA path);
	CString		_WindowsToNativePath(CStringW path);
private:
	// WINE support
	EDesktop	m_eDesktop;
	t_wine_get_dos_file_name	m_pWineUnix2NTPath;
	t_wine_get_unix_file_name	m_pWineNT2UnixPath;
	// Safer ShellExecute
	HINSTANCE _SaferShellExecute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd, DWORD dwSaferLevelId = SAFER_LEVELID_NORMALUSER);
};

extern CDesktopIntegration theDesktop;

#endif //DESKTOPINTEGRATION_H