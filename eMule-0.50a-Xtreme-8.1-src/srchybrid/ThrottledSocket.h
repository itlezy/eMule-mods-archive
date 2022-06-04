// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32	sentBytesStandardPackets;
	uint32	sentBytesControlPackets;
};

//Xman count block/success send/ upload health
#define HISTORY_SIZE 20
//Xman end

class ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
	//Xman 
	virtual bool IsSocketUploading() const = false;
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
	virtual bool StandardPacketQueueIsEmpty() const = false ;

	//Xman count block/success send
	virtual float GetBlockRatio() const =0;
	virtual float GetBlockRatio_overall() const =0;
	virtual float GetandStepBlockRatio() =0;
	//virtual void  ResetBlockRatio() = 0;
    
//Xman Xtreme Upload
	bool IsSocketUploading() const {return slotstate!=0 && isready==true;} //Xman 

	ThrottledFileSocket(void) {slotstate=0; isready=false; m_dwMSS=0;} //Xman // netfinity: Maximum Segment Size

	bool IsTrickle() const	{return slotstate==3;}
	bool IsFull() const		{return slotstate==1;}

	void SetTrickle()	{slotstate=3; CSlope=1;}
	void SetFull()		{slotstate=1;}
	void SetNoUploading() {slotstate=0;} //Xman 

	bool	isready;
	sint32	CSlope;

	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	DWORD		m_dwMSS;

private:
	uint8	slotstate;	//1=full, 3=trickle
//Xman end
};

// <-- ZZ:UploadBandWithThrottler (UDP)
