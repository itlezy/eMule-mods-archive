// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32	sentBytesStandardPackets;
	uint32	sentBytesControlPackets;
};

// ==> Mephisto Upload - Mephisto
struct FullTimes_struct{
	DWORD dwTimeStamp;
	bool bWasFull;
};
// <== Mephisto Upload - Mephisto

//Xman count block/success send/ upload health
#define HISTORY_SIZE 20
//Xman end

class ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
	//Xman 
	virtual bool IsSocketUploading() const = 0;
};

class ThrottledFileSocket : public ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
    virtual DWORD GetLastCalledSend() = 0;
	//Xman
	/*
    virtual uint32	GetNeededBytes() = 0;
	*/
	//Xman end
	virtual bool	IsBusy() const = 0;
    virtual bool    HasQueues() const = 0;
	virtual bool	UseBigSendBuffer()								{ return false; }

	//Xman Full chunk:
	virtual bool StandardPacketQueueIsEmpty() const = 0;

	//Xman count block/success send
	virtual float GetBlockRatio() const =0;
	virtual float GetBlockRatio_overall() const =0;
	virtual float GetandStepBlockRatio() =0;
	//virtual void  ResetBlockRatio() = 0;
    
//Xman Xtreme Upload
	bool IsSocketUploading() const {return slotstate!=0 && isready==true;} //Xman 

	ThrottledFileSocket(void) {slotstate=0; isready=false; m_dwMSS=0;} //Xman // netfinity: Maximum Segment Size

	// ==> Mephisto Upload - Mephisto
	/*
	bool IsTrickle() const	{return slotstate==3;}
	bool IsFull() const		{return slotstate==1;}

	void SetTrickle()	{slotstate=3; CSlope=1;}
	void SetFull()		{slotstate=1;}
	*/
	// ==> Upload Debuging [Stulle] - Mephisto
#ifndef UPLOAD_DEBUGING
	void SetUploading()	{slotstate=1;CSlope=1;m_uFullTimesCounter=0;}
#else
	void SetUploading()	{slotstate=1;CSlope=1;m_uFullTimesCounter=0;m_uFullBytesCount=0;m_uAllFullTimesCounter=0;}
#endif
	// <== Upload Debuging [Stulle] - Mephisto
	void SetTrickle() {slotstate=1;}
	void SetFull() {slotstate=2;}
	bool IsFull() const {return slotstate==2;}
	bool IsTrickle() const {return slotstate==1;}
	
	CList<FullTimes_struct> m_FullTimes;
	uint16 m_uFullTimesCounter;

	// ==> Upload Debuging [Stulle] - Mephisto
#ifdef UPLOAD_DEBUGING
	uint64 m_uFullBytesCount;
	uint64 m_uAllFullTimesCounter;
	uint64 GetAvgFullBytes() const	{return (m_uAllFullTimesCounter?(uint64)(m_uFullBytesCount/m_uAllFullTimesCounter):0);}
	uint64 GetFullBytesCount() const {return m_uFullBytesCount;}
	uint64 GetAllFullTimesCounter() const {return m_uAllFullTimesCounter;}
	int GetFullTimesListCount() const {return m_FullTimes.GetCount();}
	uint16 GetFullTimesCounter() const {return m_uFullTimesCounter;}
	float GetFullTimesRatio() const {return (m_FullTimes.GetCount()?(((float)m_uFullTimesCounter)/((float)m_FullTimes.GetCount())):0.0f);}
#endif
	// <== Upload Debuging [Stulle] - Mephisto
	// <== Mephisto Upload - Mephisto
	void SetNoUploading() {slotstate=0;} //Xman 

	bool	isready;
	sint32	CSlope;

	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	DWORD		m_dwMSS;

private:
	uint8	slotstate;	//1=full, 3=trickle
//Xman end

	// ==> Multiple friendslots [ZZ] - Mephisto
	bool	m_bFriendSlot;
public:
	bool	GetFriendSlot() const {return m_bFriendSlot;}
	void	SetFriendSlot(bool in) {m_bFriendSlot=in;}
	// <== Multiple friendslots [ZZ] - Mephisto
};

// <-- ZZ:UploadBandWithThrottler (UDP)
