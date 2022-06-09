//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/****************************************************************************************
 *                           * Keyboard Shortcut Management *                           *
 *                           *           by DropF           *                           *
 *                           ********************************                           *
 *                                                                                      *
 * To save the shortcut key combination, I use a 16-bit variable to store all the       *
 * needed information. The 3 first bits are unused. Then comes the bits repectively to: *
 *     - know if the shortcut is enabled                                                *
 *     - know if the shortcut combo uses the Alt key                                    *
 *     -      "          "           "       Ctrl key                                   *
 *     -      "          "           "       Shift key                                  *
 *     -      "          "           "       Win key                                    *
 * And finally, the last 8 bits are used to store the key                               *
 *                                                                                      *
 * Example: 0001100000001101                                                            *
 * -------                                                                              *
 *     - 000 --> aren't used                                                            *
 *     -  1  --> the shortcut is enabled                                                *
 *     -  1  --> the shortcut uses the Alt key                                          *
 *     -  0  --> the shortcut doesn't use the Ctrl key                                  *
 *     -  0  --> the shortcut doesn't use the Shift key                                 *
 *     -  0  --> the shortcut doesn't use the Win key                                   *
 *     - 00001101 = 13 = VK_RETURN --> the shortcuts uses the Enter key                 *
 * Finally, this shortcut is enabled and uses the combo Alt+Enter.                      *
 *                                                                                      *
 ****************************************************************************************/
#pragma once

#define	MASK_ENABLED	0x1000	// 0001000000000000
#define	MASK_ALT		0x0800	// 0000100000000000
#define	MASK_CTRL		0x0400	// 0000010000000000
#define	MASK_SHIFT		0x0200	// 0000001000000000
#define	MASK_WIN		0x0100	// 0000000100000000
#define	MASK_KEY		0x00FF	// 0000000011111111

#define SSP_TAB_PREFIX		0x01
#define SSP_SPACE_PREFIX	0x02
#define SSP_RETNONE			0x04	// return "None" if disabled

short GetCodeFromPressedKeys(MSG* pMsg);
CString GetStringFromShortcutCode(unsigned uiResID, int iIdx, int iMode);

class CKeyboardShortcut
{
public:
	CKeyboardShortcut(void);
	CKeyboardShortcut(short nCode);
	~CKeyboardShortcut(void);

	bool	IsEnabled(void) const	{ return m_bEnabled; }
	bool	IsAlt(void) const		{ return m_bAlt; }
	bool	IsCtrl(void) const	{ return m_bCtrl; }
	bool	IsShift(void) const	{ return m_bShift; }
	bool	IsWin(void) const	{ return m_bWin; }
	uchar	GetKey(void) const	{ return m_byteKey; }
	short	GetCode(void) const	{ return static_cast<short>((m_bEnabled ? MASK_ENABLED : 0) + (m_bAlt ? MASK_ALT : 0) + (m_bCtrl ? MASK_CTRL : 0) + (m_bShift ? MASK_SHIFT : 0) + (m_bWin ? MASK_WIN : 0) + static_cast<short>(m_byteKey)); }

	void SetEnabled(bool bEnabled)	{ m_bEnabled = bEnabled; }
	void SetAlt(bool bAlt)			{ m_bAlt = bAlt; }
	void SetCtrl(bool bCtrl)		{ m_bCtrl = bCtrl; }
	void SetShift(bool bShift)		{ m_bShift = bShift; }
	void SetWin(bool bWin)			{ m_bWin = bWin; }
	void SetKey(uchar byteKey)		{ m_byteKey = byteKey; }
	void SetCode(short nCode);

private:
	bool	m_bEnabled;
	bool	m_bAlt;
	bool	m_bCtrl;
	bool	m_bShift;
	bool	m_bWin;
	uchar	m_byteKey;
};
