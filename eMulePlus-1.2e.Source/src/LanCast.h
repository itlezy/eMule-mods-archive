#pragma once

#include "Loggable.h"

class CKnownFile;

class CLanCast : public CAsyncSocket, public CLoggable
{
public:
	CLanCast();
	~CLanCast();

	virtual void OnReceive(int iErrorCode);
	bool SendPacket(Packet* packet);

	void CLanCast::Start();
	void CLanCast::Stop();

	void CLanCast::BroadcastHash(CKnownFile *pKnownFile);

private:
	void CLanCast::ReceiveHash(byte *pbytePacketBuf, uint32 dwSize);

	bool JoinGroup(const char *pcGroupIP, uint32 dwGroupPort, uint32 dwTTL, bool bLoopback);
	bool LeaveGroup();

	void SetLoopBack(bool bLoop);
	bool SetTTL(uint32 dwTTL);
	bool CreateSendingSocket(uint32 dwTTL, bool bLoopBack);
	bool CreateReceivingSocket(const char *pcGroupIP, uint32 dwGroupPort);

	void static CALLBACK UDPTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	bool CLanCast::ProcessPacket(byte *pbytePacketBuf, uint32 size, byte byteOpcode);

	bool GetLancastEnabled();
	uint32 GetLancastIP();
	uint16 GetLancastPort();

private:
	SOCKADDR_IN m_saHostGroup;	// SOCKADDR structure to hold IP/Port of the Host group to send data to it
	ip_mreq m_mrMReq;			// Contains IP and interface of the host group

	CAsyncSocket m_SendSocket;	// Socket for sending data to the host group

	uint32 udp_timer;			// Timer to fire packet sends
	bool bStarted;
};
