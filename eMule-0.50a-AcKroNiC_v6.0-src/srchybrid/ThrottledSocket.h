// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32	sentBytesStandardPackets;
	uint32	sentBytesControlPackets;
    uint32  errorThatOccured;
};

class ThrottledControlSocket
{
public:
	virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
	/*
	virtual DWORD GetBusyTimeSince() const = 0;
	virtual float GetBusyRatioTime() const = 0;
	*/
};

class ThrottledFileSocket : public ThrottledControlSocket
{
public:
	virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
	//virtual DWORD GetLastCalledSend() = 0;
    //virtual uint32	GetNeededBytes() = 0;
	virtual bool	IsBusy() const = 0;
    virtual bool    HasQueues() const = 0;
	virtual DWORD GetBusyTimeSince() const = 0;
	virtual float GetBusyRatioTime() const = 0;
};

// <-- ZZ:UploadBandWithThrottler (UDP)
