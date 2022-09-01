//PLEASE see notes in AntiNick.cpp!

#pragma once
class CUpDownClient;

class CAntiNick
{
//>>> Global functions
public:
	static	void	Init();
	static	void	UnInit();
//<<< Global functions
//>>> AntiNickThief
public:
	//this creates a new string or returns the current one
	static	CString	GetAntiNickThiefNick();

	//this returns if we have a nickthief
	static	bool	FindOurTagIn(const CString& tocomp);

private:
	//this will be automatically called
	static	void	CreateAntiNickThiefTag();

	//the string
	static	CString m_sAntiNickThiefTag;

	//the update timer
	static	uint32	m_uiAntiNickThiefCreateTimer;

//<<< AntiNickThief
};

extern CAntiNick theAntiNickClass;