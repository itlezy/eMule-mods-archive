// ZZ:UploadBandWithThrottler (UDP) -->

#pragma once

struct SocketSentBytes {
    bool    success;
	uint32	sentBytesStandardPackets;
	uint32	sentBytesControlPackets;
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	uint16	sentStandardPackets;
	uint16	sentControlPackets;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
};

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
enum eSlotState 
{
	SS_NONE,
	SS_FULL,
	SS_TRICKLE,
	SS_BLOCKED
};

 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
enum eSlotType
{
	ST_RELEASE = 0,
	ST_FRIEND = 1,
	ST_NORMAL = 2 // must be always last, it is used a counter for arrays !
};
 #endif // BW_MOD // NEO: BM END
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

class ThrottledControlSocket
{
public:
	// NEO: MOD - [ThrottledControlSocket] -- Xanatos -->
	ThrottledControlSocket() 
	{
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
		onDownProcessQueue = false; 
		onDownQueue = false; 
		onDownControlQueue = false; 
#endif // NEO_DBT // NEO: NDBT END

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
		onUpQueue = false; 
		onUpControlQueue = false; 
		onBlockedQueue = false; 
#endif // NEO_UBT // NEO: NUBT END

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		onDownLanQueue = false; 
		onUpLanQueue = false; 
#endif //LANCAST // NEO: NLC END
	}

	virtual bool	isUDP() const {return false;}
	virtual bool	IsConnected() const {return true;}

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
	virtual bool	IsEmpty() const = 0;
	virtual int		Receive(uint32 size, UINT hyperreceiving = FALSE) = 0;
	virtual	bool	ProcessData(bool ignore = false) = 0;
	virtual bool	IsPriorityReceive() {return false;}
	bool onDownControlQueue;
	bool onDownProcessQueue;
	bool onDownQueue;
#endif // NEO_DBT // NEO: NDBT END

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
	virtual bool	IsBusy() const = 0;
	virtual bool	IsPrioritySend() {return false;}
	virtual bool	ControlPacketQueueIsEmpty() const = 0;
	virtual	bool	StandardPacketQueueIsEmpty() const {return true;}

	virtual void	CountBusyTick(){ASSERT(0);}
	virtual void	CountReadyTick(){ASSERT(0);}

	bool onUpControlQueue;
	bool onBlockedQueue;
	bool onUpQueue;
#endif // NEO_UBT // NEO: NUBT END

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	bool onDownLanQueue;
	bool onUpLanQueue;
	virtual bool	IsLanSocket() { return false; }
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	virtual bool	IsVoodooSocket() { return false; }
#endif // VOODOO // NEO: VOODOO END

	// NEO: MOD END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    virtual SocketSentBytes Send(uint32 maxNumberOfBytesToSend, uint32 minFragSize, bool onlyAllowedToSendControlPacket = false, uint16 maxNumberOfPacketsToSend = 0xFFFF) = 0;
#else
    virtual SocketSentBytes SendControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
};

class ThrottledFileSocket : public ThrottledControlSocket
{
public:
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	ThrottledFileSocket(){
		m_eSlotState = SS_NONE;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
		m_eSlotType = ST_NORMAL;
		m_ModerateSpeed = 0.0;
 #endif // BW_MOD // NEO: BM END
		m_IsReady = false; // NEO: NUSM - [NeoUploadSlotManagement]
		m_bDownloadSocket = false;
		m_SocketSlope = 0;
		avg_ratio = 0.75f;
	}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    virtual SocketSentBytes SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 minFragSize) = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
    virtual DWORD GetLastCalledSend() = 0;
    virtual uint32	GetNeededBytes() = 0;
	virtual bool	IsBusy() const = 0;
    virtual bool    HasQueues() const = 0;

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	#define			RATIO_AVG	100
	void			ResetProcessRate(float rate = 0.75f){
						cur_ratio = rate;
						avg_ratio = rate;
						for (int i = 0; i < RATIO_AVG; i++)
							ratios[i] = rate;
						ratioIndex = 0;
						m_uBusyTicks=0;
						m_uReadyTicks=0;
					}

	virtual void	CountBusyTick(){
						m_uReadyTicks = 0;
						m_uBusyTicks ++;
						CalcRatio(true);
					}
	virtual void	CountReadyTick(){
						m_uBusyTicks = 0;
						m_uReadyTicks ++;
						CalcRatio(false);
					}

	float			GetAvgRatio() {return avg_ratio;}

	eSlotState	m_eSlotState;
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
	eSlotType	m_eSlotType;
	float		m_ModerateSpeed;
 #endif // BW_MOD // NEO: BM END
	bool		m_IsReady; // NEO: NUSM - [NeoUploadSlotManagement]
	bool		m_bDownloadSocket;

	int			m_SocketSlope;

private:
	uint32		m_uBusyTicks;
	uint32		m_uReadyTicks;

	void		CalcRatio(bool busy){
					float multiplier = 0.1f;
					if (busy){
						if (m_uBusyTicks < 40)
							multiplier =log10f((float)m_uBusyTicks) / 2.4f + 0.33f;
						cur_ratio -= cur_ratio/250.0F * multiplier;
					}	else{
						if (m_uReadyTicks < 160) // David: was 40
							multiplier = log10f((float)m_uReadyTicks) / 2.4f + 0.33f;
						cur_ratio += (1.0f-cur_ratio)/250.0F * multiplier;
					}
					float tmp_ratio = avg_ratio; // to have athread safe GetAvgRatio
					tmp_ratio *= RATIO_AVG;
					tmp_ratio -= ratios[ratioIndex];
					tmp_ratio += cur_ratio;
					tmp_ratio /= RATIO_AVG;
					ratios[ratioIndex++] = cur_ratio;
					ratioIndex %= RATIO_AVG;
					avg_ratio = tmp_ratio;
				}


	float		cur_ratio;
	float		avg_ratio;
	float		ratios[RATIO_AVG];
	uint8		ratioIndex;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
};

// <-- ZZ:UploadBandWithThrottler (UDP)
