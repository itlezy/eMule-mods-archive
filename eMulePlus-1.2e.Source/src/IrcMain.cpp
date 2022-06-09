//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//A lot of documentation for the IRC protocol within this code came
//directly from http://www.irchelp.org/irchelp/rfc/rfc2812.txt
//Much of it may never be used, but it's here just in case..

#include "stdafx.h"
#include "IrcMain.h"
#include "IrcSocket.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define EP_IRC_VERSION		_T("ePlus") CURRENT_VERSION_LONG Irc_Version

CIrcMain::CIrcMain(void)
{
	m_pIrcSocket = NULL;
	m_pWndIRC = 0;
}

CIrcMain::~CIrcMain(void)
{
}

void CIrcMain::PreParseMessage(const char *pcBuf, int iLen)
{
	CString	strMessage;
	int		iIdx;

	m_strPreParseBufA.Append(pcBuf, iLen);
	for (;;)
	{
		if ((iIdx = m_strPreParseBufA.Find('\n')) < 0)
			break;
		if (((iLen = iIdx) != 0) && (m_strPreParseBufA.GetString()[iLen - 1] == '\r'))
			iLen--;
		MB2Str(cfUTF8, &strMessage, m_strPreParseBufA.GetString(), iLen);
		ParseMessage(strMessage);
		m_strPreParseBufA.Delete(0, iIdx + 1);
	}
}

void CIrcMain::ParseMessage(CString strRawMessage)
{
	if (strRawMessage.GetLength() < 6)
	{
		// TODO : We probably should disconnect here as I don't know of anything that should
		// come from the server this small
		return;
	}

	if (strRawMessage.Left(6) == _T("PING :"))
	{
		// If the server pinged us, we must pong back or get disconnected
		// Anything after the ":" must be sent back or it will fail
		m_pIrcSocket->SendString(_T("PONG ") + strRawMessage.Mid(6));
		m_pWndIRC->AddMessage(_T(""), _T(""), RGB(0, 147, 0), _T("PING?/PONG"));
		return;
	}

	CString	strSource, strCommand, strTarget, strTarget2, strMessage, strTemp;
	int		iSourceIndex, iCommandIndex, iTargetIndex, iMessageIndex, iTarget2Index, iCommand;

	// Check if there is a source which always ends with a "!"
	// Messages without a source is usually a server message
	iSourceIndex = strRawMessage.Find(_T('!'));
	if (strRawMessage.Left(1) == _T(":"))
	{
		// If the message starts with a ":", the first space must be the beginning of the command
		// Although, it seems some servers are miscofigured
		iCommandIndex = strRawMessage.Find(_T(' '));
	}
	else
	{
		// If the message doesn't start with a ":", the first word is the command
		iCommandIndex = 0;
	}
	// The second space is the beginning of the target or message
	iTargetIndex = strRawMessage.Find(_T(' '), iCommandIndex + 1);
	if (iTargetIndex < 0)
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received a message with no target or message."));
		return;
	}
	// This will pull out the command
	if (iCommandIndex == 0)
	{
		// Command is the first word. Strange as I don't see this in any standard
		strCommand = strRawMessage.Mid(iCommandIndex, iTargetIndex - iCommandIndex);
	}
	else
	{
		// Command is where it should be
		strCommand = strRawMessage.Mid(iCommandIndex + 1, iTargetIndex - iCommandIndex - 1);
	}
	if (iSourceIndex < iCommandIndex && iSourceIndex > 0)
	{
		// Get source and IP of source
		strSource = strRawMessage.Mid(1, iSourceIndex - 1);
		iMessageIndex = strRawMessage.Find(_T(' '), iTargetIndex + 1);
		if (iMessageIndex > iSourceIndex)
		{
			// Get target and message
			strTarget = strRawMessage.Mid(iTargetIndex + 1, iMessageIndex - iTargetIndex - 1);
			strMessage = strRawMessage.Mid(iMessageIndex);
		}
		else
		{
			//Get target
			strTarget = strRawMessage.Mid(iTargetIndex + 1, strRawMessage.GetLength() - iTargetIndex - 1);
		}
		// I'm not sure why some messages have different formats, but this cleans them up
		if (strTarget[0] == _T(':'))
			strTarget = strTarget.Mid(1);
		if (strMessage[0] == _T(':'))
			strMessage = strMessage.Mid(1);
		if (strTarget.Left(2) == _T(" :"))
			strTarget = strTarget.Mid(2);
		if (strMessage.Left(2) == _T(" :"))
			strMessage = strMessage.Mid(2);
	}
	else
	{
		if (iCommandIndex != 0)
		{
			// Get source
			strSource = strRawMessage.Mid(1, iCommandIndex - 1);
		}
		strMessage = strRawMessage.Mid(iTargetIndex + 1);
	}
	if (strCommand == _T("PRIVMSG"))
	{
		if (strTarget[0] == _T('#'))
		{
			// Belongs to a channel
			if (strMessage[0] == _T('\x01'))
			{
				// This is a special message. Find out what kind.
				if (strMessage.GetLength() < 4)
				{
					AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received Invalid special channel message."));
					return;
				}
				strMessage = strMessage.Mid(1, strMessage.GetLength() - 2);
				if (strMessage.Left(6) == _T("ACTION"))
				{
					// Channel Action
					if (strMessage.GetLength() < 8)
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received Invalid channel action."));
						return;
					}
					m_pWndIRC->AddMessageF(strTarget, _T(""), RGB(156, 0, 156), _T("* %s %s"), strSource, strMessage.Mid(7));
					return;
				}
				if (strMessage.Left(7) == _T("VERSION"))
				{
					// Get client version
					m_pWndIRC->AddMessageF(_T(""), _T(""), RGB(255, 0, 0), _T("* [%s] VERSION"), strSource);
					strTemp.Format(_T("NOTICE %s :\001VERSION ") EP_IRC_VERSION _T("\001"), strSource);
					m_pIrcSocket->SendString(strTemp);
					return;
				}
			}
			else
			{
			//	This is a normal channel message
				m_pWndIRC->AddMessage(strTarget, strSource, CLR_DEFAULT, strMessage);
				return;
			}
		}
		else
		{
			// Private Message
			if (strMessage[0] == _T('\x01'))
			{
				// Special message
				if (strMessage.GetLength() < 4)
				{
					AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received Invalid special private message."));
					return;
				}
				strMessage = strMessage.Mid(1, strMessage.GetLength() - 2);
				if (strMessage.Left(6) == _T("ACTION"))
				{
					// Action
					if (strMessage.GetLength() < 8)
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received Invalid private action."));
						return;
					}
					m_pWndIRC->AddMessageF(strSource, _T(""), RGB(0, 147, 0), _T("* %s %s"), strSource, strMessage.Mid(7));
					return;
				}
				if (strMessage.Left(7) == _T("VERSION"))
				{
					// Get client version
					m_pWndIRC->AddMessageF(_T(""), _T(""), RGB(255, 0, 0), _T("* [%s] VERSION"), strSource);
					strTemp.Format(_T("NOTICE %s :\001VERSION ") EP_IRC_VERSION _T("\001"), strSource);
					m_pIrcSocket->SendString(strTemp);
				}
				return;
			}
			else
			{
				m_pWndIRC->AddMessage(strSource, strSource, CLR_DEFAULT, strMessage);
				return;
			}
		}
	}
	if (strCommand == _T("JOIN"))
	{
		// Channel join
		if (strSource == m_strNick)
		{
			// This was you. So for it to add a new channel
			m_pWndIRC->AddMessageF(strTarget, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_HASJOINED), strSource, strTarget);
			return;
		}
		if (!g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
			m_pWndIRC->AddMessageF(strTarget, _T(""), RGB(0, 147, 0), GetResString(IDS_IRC_HASJOINED), strSource, strTarget);
		// Add new nick to your channel
		m_pWndIRC->m_ctlNickList.NewNick(strTarget, strSource);
		return;
	}
	if (strCommand == _T("PART"))
	{
		// Part message
		if (strSource == m_strNick)
		{
			// This was you, so remove channel
			m_pWndIRC->RemoveChannel(strTarget);
			return;
		}
		// Remove nick from your channel
		m_pWndIRC->m_ctlNickList.RemoveNick(strTarget, strSource);
		if (!g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
		{
			strMessage.Trim();
			strTemp.Format(GetResString(IDS_IRC_HASPARTED), strSource, strTarget, strMessage);
			if (strMessage.IsEmpty())
				strTemp.Truncate(strTemp.GetLength() - 3);
			else
				strTemp.Insert(strTemp.GetLength() - 1, _T('\x0F'));
			m_pWndIRC->AddMessage(strTarget, _T(""), RGB(51, 102, 255), strTemp);
		}
		return;
	}
	if (strCommand == _T("TOPIC"))
	{
		// Topic was set, update it
		m_pWndIRC->SetTitle(strTarget, strMessage);
		return;
	}
	if (strCommand == _T("QUIT"))
	{
		// This user left the network. Remove from all Channels
		m_pWndIRC->m_ctlNickList.DeleteNickInAll(strSource, strMessage);
		return;
	}
	if (strCommand == _T("NICK"))
	{
		// Someone changed a nick
		if (strSource == m_strNick)
		{
			// It was you. Update
			m_strNick = strTarget;
		}
		// Update new nick in all channles
		m_pWndIRC->ChangeAllNick(strSource, strTarget);
		return;
	}
	if (strCommand == _T("KICK"))
	{
		// Someone was kicked from a channel
		iTarget2Index = strMessage.Find(_T(':'));
		if (iTarget2Index > 0)
		{
			strTarget2 = strMessage.Mid(1, iTarget2Index - 2);
			strMessage = strMessage.Mid(iTarget2Index + 1);
		}
		strMessage.Trim();
		strTemp.Format(GetResString(IDS_IRC_WASKICKEDBY), strTarget2, strSource, strMessage);
		if (strMessage.IsEmpty())
			strTemp.Truncate(strTemp.GetLength() - 3);
		else
			strTemp.Insert(strTemp.GetLength() - 1, _T('\x0F'));
		if (strTarget2 == m_strNick)
		{
			// It was you!
			m_pWndIRC->RemoveChannel(strTarget);
			m_pWndIRC->AddMessage(_T(""), _T(""), RGB(252, 127, 0), strTemp);
			return;
		}
		// Remove nick from your channel
		m_pWndIRC->m_ctlNickList.RemoveNick(strTarget, strTarget2);
		if (!g_App.m_pPrefs->GetIrcIgnoreInfoMessage())
			m_pWndIRC->AddMessage(strTarget, _T(""), RGB(252, 127, 0), strTemp);
		return;
	}
	if (strCommand == _T("MODE"))
	{
		if (!strTarget.IsEmpty())
		{
			iCommandIndex = strMessage.Find(_T(' '), 1);
			if (iCommandIndex < 2)
			{
				AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("SMIRC Error: Received Invalid Mode change."));
				return;
			}
			strCommand = strMessage.Mid(1, iCommandIndex - 1);
			strTarget2 = strMessage.Mid(iCommandIndex + 1);
			m_pWndIRC->m_ctlNickList.ParseChangeMode(strTarget, strSource, strCommand, strTarget2);
		}
		else
		{
			// The server just set a server user mode that relates to no channels!
			// Atm, we do not handle these modes.
		}
		return;
	}
	if (strCommand == _T("NOTICE"))
	{
		// Receive notice
		m_pWndIRC->NoticeMessage(strSource, strMessage);
		return;
	}

	if (strCommand.GetLength() == 3
		&& strCommand[0] >= _T('0') && strCommand[0] <= _T('9')
		&& strCommand[1] >= _T('0') && strCommand[1] <= _T('9')
		&& strCommand[2] >= _T('0') && strCommand[2] <= _T('9'))
	{
		iCommand = _tstoi(strCommand);

		if (iCommand == 1) // RPL_WELCOME
		{
			m_pWndIRC->SetLoggedIn(true);
			if (g_App.m_pPrefs->GetIRCListOnConnect())
				m_pIrcSocket->SendString(_T("list"));
			ParsePerform();
		}
		else if (iCommand == 275 || (iCommand >= 307 && iCommand <= 320) || iCommand == 369  || iCommand == 378
			|| iCommand == 401 || iCommand == 406) // RPL_WHOIS*/RPL_WHOWAS*
		{
			int	iNameIndex = strMessage.Find(_T(' '), m_strNick.GetLength() + 1);

			if (iNameIndex > -1 && iNameIndex + 1 < strMessage.GetLength())
			{
				if (strMessage[iNameIndex + 1] == _T(':'))
					strMessage.Delete(iNameIndex + 1, 1);
				switch (iCommand)
				{
					case 311: // RPL_WHOISUSER
					{
						strMessage.Insert(iNameIndex, _T(" is"));
						iNameIndex = strMessage.Find(_T(' '), iNameIndex + 4);
						if (iNameIndex > -1)
							strMessage.SetAt(iNameIndex, _T('@'));
						break;
					}
					case 312: // RPL_WHOISSERVER
					{
						strMessage.Insert(iNameIndex, _T(" using"));
						break;
					}
					case 317: // RPL_WHOISIDLE
					{
						int	iTimeIndex = strMessage.Find(_T(' '), iNameIndex + 1);

						if (iTimeIndex != -1)
						{
							int		iTimeIndexEnd = strMessage.Find(_T(' '), iTimeIndex + 1);
							CString	strIdleTime = ::CastSecondsToHM(_tstoi(strMessage.Mid(iNameIndex + 1, iTimeIndex - iNameIndex - 1)));
							CTime	ctTime((iTimeIndexEnd > -1) ? _tstoi(strMessage.Mid(iTimeIndex + 1, iTimeIndexEnd - iTimeIndex - 1)) : 0);

							strMessage.Truncate(iNameIndex + 1);
							strMessage.AppendFormat(_T("has been idle %s"), strIdleTime);

							if (ctTime != 0)
							{
								SYSTEMTIME	st;

								ctTime.GetAsSystemTime(st);

								COleDateTime	odtTime(st);

								strMessage.AppendFormat(_T(", signed on %s"), odtTime.Format(_T("%c")));
							}
						}
						break;
					}
					case 319: // RPL_WHOISCHANNELS
					{
						strMessage.Insert(iNameIndex, _T(" on"));
						break;
					}
				}
			}

			m_pWndIRC->NoticeMessage(_T(""), strMessage);
			return;
		}
		else if (iCommand == 321) // RPL_LISTSTART
		{
			m_pWndIRC->ResetServerChannelList();
			return;
		}
		else if (iCommand == 322) // RPL_LIST
		{
			CString	strChanName, strChanNum, strChanDesc;
			int		iChanNameIndex, iChanNumIndex, iChanDescIndex;

			iChanNameIndex = strMessage.Find(_T(' '));
			iChanNumIndex = strMessage.Find(_T(' '), iChanNameIndex + 1);
			iChanDescIndex = strMessage.Find(_T(" :"), iChanNumIndex + 1);
			strChanName = strMessage.Mid(iChanNameIndex + 1, iChanNumIndex - iChanNameIndex - 1);
			strChanNum = strMessage.Mid(iChanNumIndex + 1, iChanDescIndex - iChanNumIndex - 1);
			if (iChanDescIndex > 0)
				strChanDesc = strMessage.Mid(iChanDescIndex + 2);
			m_pWndIRC->AddChannelToList(strChanName, strChanNum, strChanDesc);
			return;
		}
		else if (iCommand == 332) // RPL_TOPIC
		{
			int	iChannelIndex = strMessage.Find(_T(' '));

			if (iChannelIndex > -1)
			{
				int	iTopicIndex = strMessage.Find(_T(" :"), iChannelIndex + 1);

				if (iTopicIndex > -1)
				{
					strTarget2 = strMessage.Mid(iChannelIndex + 1, iTopicIndex - iChannelIndex - 1);
					strMessage = strMessage.Mid(iTopicIndex + 2);
					m_pWndIRC->AddMessageF(strTarget2, _T(""), RGB(0, 147, 0), _T("* Channel Title: %s"), strMessage);
					m_pWndIRC->SetTitle(strTarget2, strMessage);
				}
			}
			return;
		}
		else if (iCommand == 333) // RPL_TOPICWHOTIME
		{
			int	iChannelIndex = strMessage.Find(_T('#'));

			if (iChannelIndex > -1)
			{
				int	iNameIndex = strMessage.Find(_T(' '), iChannelIndex + 1);

				if (iNameIndex > -1)
				{
					int		iTimeIndex = strMessage.Find(_T(' '), iNameIndex + 1);
					CTime	ctTime((iTimeIndex > -1) ? _tstoi(strMessage.Mid(iTimeIndex + 1, strMessage.GetLength() - iTimeIndex - 1)) : 0);

					strTarget2 = strMessage.Mid(iChannelIndex, iNameIndex - iChannelIndex);

					if (ctTime != 0 && iTimeIndex > -1)
					{
						SYSTEMTIME	st;

						ctTime.GetAsSystemTime(st);

						COleDateTime	odtTime(st);

						strMessage = strMessage.Mid(iNameIndex, iTimeIndex - iNameIndex + 1);
						strMessage.AppendFormat(_T("on %s"), odtTime.Format(_T("%c")));
					}
					else
						strMessage = strMessage.Mid(iNameIndex);

					strMessage.Insert(0, _T("* Set by"));
					m_pWndIRC->AddMessage(strTarget2, _T(""), RGB(0, 147, 0), strMessage);
				}
			}
			return;
		}
		else if (iCommand == 353) // RPL_NAMREPLY
		{
			int		iGetNickChannelIndex, iGetNickIndex, iCount = 0;
			CString	strGetNickChannel, strGetNick;

			VERIFY ( (iGetNickChannelIndex = strRawMessage.Find(_T(' '), iTargetIndex + 1)) != (-1) );
			iGetNickChannelIndex = strRawMessage.Find(_T(' '), iTargetIndex + 1);
			iGetNickIndex = strRawMessage.Find(_T(' '), iGetNickChannelIndex + 3);
			strGetNickChannel = strRawMessage.Mid(iGetNickChannelIndex + 2, iGetNickIndex - iGetNickChannelIndex - 2);
			iGetNickChannelIndex = strRawMessage.Find(_T(':'), iGetNickChannelIndex);
			iGetNickIndex = strRawMessage.Find(_T(' '), iGetNickChannelIndex);

			while (iGetNickIndex > 0)
			{
				iCount++;
				strGetNick = strRawMessage.Mid(iGetNickChannelIndex + 1, iGetNickIndex - iGetNickChannelIndex - 1);
				iGetNickChannelIndex = iGetNickIndex;
				m_pWndIRC->m_ctlNickList.NewNick(strGetNickChannel, strGetNick);
				iGetNickIndex = strRawMessage.Find(_T(' '), iGetNickChannelIndex + 1);
			}
			return;
		}
		else if (iCommand == 433) // ERR_NICKNAMEINUSE
		{
			if (!m_pWndIRC->GetLoggedIn())
				Disconnect();
			m_pWndIRC->AddMessage(_T(""), _T(""), RGB(252, 127, 0), GetResString(IDS_IRC_NICKUSED));
			return;
		}
	}

	m_pWndIRC->AddMessage(_T(""), _T(""), CLR_DEFAULT, strMessage);
}

void CIrcMain::SendLogin()
{
	m_pIrcSocket->SendString(m_strUser);
	m_pIrcSocket->SendString(_T("NICK ") + m_strNick);
}

void CIrcMain::ParsePerform()
{
	CString	strRawPerform;

	if (g_App.m_pPrefs->GetIrcUsePerform())
		strRawPerform += g_App.m_pPrefs->GetIrcPerformString();

	if (g_App.m_pPrefs->GetIRCServer().MakeLower() == _T("irc.emuleplus.info"))
	{
		const TCHAR	*pcChannel = NULL;

		strRawPerform += _T("|/join #emule+");

		switch (g_App.m_pPrefs->GetLanguageID())
		{
			case MAKELANGID(LANG_CROATIAN, SUBLANG_DEFAULT):
				pcChannel = _T("croatian");
				break;
			case MAKELANGID(LANG_CZECH, SUBLANG_DEFAULT):
				pcChannel = _T("czech");
				break;
			case MAKELANGID(LANG_DANISH, SUBLANG_DEFAULT):
				pcChannel = _T("danish");
				break;
			case MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT):
				pcChannel = _T("english");
				break;
			case MAKELANGID(LANG_FINNISH, SUBLANG_DEFAULT):
				pcChannel = _T("finnish");
				break;
			case MAKELANGID(LANG_FRENCH, SUBLANG_DEFAULT):
				pcChannel = _T("french");
				break;
			case MAKELANGID(LANG_GERMAN, SUBLANG_DEFAULT):
				pcChannel = _T("german");
				break;
			case MAKELANGID(LANG_HEBREW, SUBLANG_DEFAULT):
				pcChannel = _T("israel");
				break;
			case MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT):
				pcChannel = _T("italian");
				break;
			case MAKELANGID(LANG_LITHUANIAN, SUBLANG_DEFAULT):
				pcChannel = _T("lithuanian");
				break;
			case MAKELANGID(LANG_DUTCH, SUBLANG_DEFAULT):
				pcChannel = _T("netherlands");
				break;
			case MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT):
				pcChannel = _T("polish");
				break;
			case MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE):
			case MAKELANGID(LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN):
				pcChannel = _T("portuguese");
				break;
			case MAKELANGID(LANG_ROMANIAN, SUBLANG_DEFAULT):
				pcChannel = _T("romanian");
				break;
			case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT):
				pcChannel = _T("russian");
				break;
			case MAKELANGID(LANG_SPANISH, SUBLANG_DEFAULT):
				pcChannel = _T("spanish");
				break;
			case MAKELANGID(LANG_TURKISH, SUBLANG_DEFAULT):
				pcChannel = _T("turkish");
				break;
			case MAKELANGID(LANG_UKRAINIAN, SUBLANG_DEFAULT):
				pcChannel = _T("ukrainian");
				break;
			case MAKELANGID(LANG_SERBIAN, SUBLANG_SERBIAN_LATIN):
				pcChannel = _T("serbian");
		}

		if (pcChannel != NULL)
		{
			strRawPerform += _T("|/join #emule+");
			strRawPerform += pcChannel;
		}
	}

	if (strRawPerform.IsEmpty())
		return;

	int		iIndex = 0;
	CString	strNextPerform;

	while (strRawPerform.Find(_T('|')) != -1)
	{
		iIndex = strRawPerform.Find(_T('|'));
		strNextPerform = strRawPerform.Left(iIndex);
		strNextPerform.TrimLeft(_T(' '));
		if (strNextPerform[0] == _T('/'))
			strNextPerform = strNextPerform.Mid(1);
		if (strNextPerform.Left(3) == _T("msg"))
			strNextPerform = _T("PRIVMSG") + strNextPerform.Mid(3);
		if ((strNextPerform.Left(16)).CompareNoCase(_T("PRIVMSG nickserv")) == 0)
			strNextPerform = _T("ns") + strNextPerform.Mid(16);
		if ((strNextPerform.Left(16)).CompareNoCase(_T("PRIVMSG chanserv")) == 0)
			strNextPerform = _T("cs") + strNextPerform.Mid(16);
		m_pIrcSocket->SendString(strNextPerform);
		strRawPerform = strRawPerform.Mid(iIndex + 1);
	}
	if (!strRawPerform.IsEmpty())
	{
		strRawPerform.TrimLeft(_T(' '));
		if (strRawPerform[0] == _T('/'))
			strRawPerform = strRawPerform.Mid(1);
		if (strRawPerform.Left(3) == _T("msg"))
			strRawPerform = _T("PRIVMSG") + strRawPerform.Mid(3);
		if ((strRawPerform.Left(16)).CompareNoCase(_T("PRIVMSG nickserv")) == 0)
			strRawPerform = _T("ns") + strRawPerform.Mid(16);
		if ((strRawPerform.Left(16)).CompareNoCase(_T("PRIVMSG chanserv")) == 0)
			strRawPerform = _T("cs") + strRawPerform.Mid(16);
		if (!strRawPerform.IsEmpty())
			m_pIrcSocket->SendString(strRawPerform);
	}
}

void CIrcMain::Connect()
{
	uint32	dwHash = 0;

//	Robert Jenkins One-at-a-time hashing algorithm
	for (int i = 0; i < 16; i++)
	{
		dwHash += g_App.m_pPrefs->GetUserHash()[i];
		dwHash += (dwHash << 10);
		dwHash ^= (dwHash >> 6);
	}
	dwHash += (dwHash << 3);
	dwHash ^= (dwHash >> 11);
	dwHash += (dwHash << 15);

	m_pIrcSocket = new CIrcSocket(this);
	m_strNick = g_App.m_pPrefs->GetIRCNick();
	m_strNick.Remove(_T('.'));
	m_strNick.Remove(_T(' '));
	m_strNick.Remove(_T(':'));
	m_strNick.Remove(_T('/'));
	m_strNick.Remove(_T('@'));
	if (m_strNick.GetLength() > 20)
		m_strNick.Truncate(20);
	m_strUser.Format(_T("USER e%08X 8 * :") EP_IRC_VERSION, dwHash);
	m_pIrcSocket->Create();
	m_pIrcSocket->Connect();
	m_pWndIRC->SetConnectStatus(IRC_CONNECTING);
}

void CIrcMain::Disconnect(bool bIsShuttingDown)
{
	m_pIrcSocket->Close();
	if (m_pIrcSocket != NULL)
		delete m_pIrcSocket;
	m_pIrcSocket = NULL;
	if (!bIsShuttingDown)
		m_pWndIRC->SetConnectStatus(IRC_DISCONNECTED);
}

void CIrcMain::SetConnectStatus(bool bStatus)
{
	m_pWndIRC->SetConnectStatus(bStatus ? IRC_CONNECTED : IRC_DISCONNECTED);
}

int CIrcMain::SendString(const CString &strSend)
{
	return m_pIrcSocket->SendString(strSend);
}
