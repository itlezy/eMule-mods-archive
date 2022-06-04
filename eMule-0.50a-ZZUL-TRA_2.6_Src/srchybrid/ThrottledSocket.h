// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32	sentBytesStandardPackets;
	uint32	sentBytesControlPackets;
//ZZUL +
    uint32  errorThatOccured;
//ZZUL -
};

class ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
};

class ThrottledFileSocket : public ThrottledControlSocket
{
public:
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
 //ZZUL +
   //virtual DWORD GetLastCalledSend() = 0;
    //virtual uint32	GetNeededBytes() = 0;
//ZZUL -
	virtual bool	IsBusy() const = 0;
    virtual bool    HasQueues() const = 0;
	virtual bool	UseBigSendBuffer()								{ return false; }

	// ZZUL-TRA :: BlockRatio :: Start
	virtual float GetBlockRatio() const = 0;
	virtual float	GetBlockRatioOverall() const = 0;
	virtual void GetAndStepBlockRatio() = 0;
	// ZZUL-TRA :: BlockRatio :: End
};

// <-- ZZ:UploadBandWithThrottler (UDP)
