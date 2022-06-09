// AddBuddy.h: ----------------------------------------------------------------

#pragma once

#define BDS_LEFT		0x01
#define BDS_RIGHT	0x02
#define BDS_TOP		0x04
#define BDS_BOTTOM	0x08

extern BOOL AddBuddy(HWND hwndTarget, HWND hwndBuddy, UINT uStyle);

//-----------------------------------------------------------------------------
// (p) 2003 by FoRcHa  ... (based on Gipsysoft's BuddyButton)