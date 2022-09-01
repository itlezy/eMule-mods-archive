//eWombat
//PING CLASS AND FUNCTIONS
//USING RAW-WINSOCK2 IF AVAILABLE
//IF NOT USING ICMP:DLL
#pragma once
#define CPING_USE_ICMP

namespace WombatAgent 
{
	//#ifdef CPING_USE_ICMP

	//These defines & structure definitions are taken from the "ipexport.h" and
	//"icmpapi.h" header files as provided with the Platform SDK and
	//are used internally by the CPing class. Including them here allows
	//you to compile the CPing code without the need to have the full 
	//Platform SDK installed.

	typedef unsigned long IPAddr;     // An IP address.

	typedef struct tagIP_OPTION_INFORMATION 
	{
		unsigned char      Ttl;              // Time To Live
		unsigned char      Tos;              // Type Of Service
		unsigned char      Flags;            // IP header flags
		unsigned char      OptionsSize;      // Size in bytes of options data
		unsigned char FAR *OptionsData;      // Pointer to options data
	} IP_OPTION_INFORMATION;

	typedef struct tagICMP_ECHOREPLY 
	{
		IPAddr                Address;       // Replying address
		unsigned long         Status;        // Reply IP_STATUS
		unsigned long         RoundTripTime; // RTT in milliseconds
		unsigned short        DataSize;      // Reply data size in bytes
		unsigned short        Reserved;      // Reserved for system use
		void FAR              *Data;         // Pointer to the reply data
		IP_OPTION_INFORMATION Options;       // Reply options
	} ICMP_ECHOREPLY;

	typedef IP_OPTION_INFORMATION FAR* LPIP_OPTION_INFORMATION;
	typedef ICMP_ECHOREPLY FAR* LPICMP_ECHOREPLY;
	typedef HANDLE (WINAPI IcmpCreateFile)(VOID);
	typedef IcmpCreateFile* lpIcmpCreateFile;
	typedef BOOL (WINAPI IcmpCloseHandle)(HANDLE IcmpHandle);
	typedef IcmpCloseHandle* lpIcmpCloseHandle;
	typedef DWORD (WINAPI IcmpSendEcho)(HANDLE IcmpHandle, IPAddr DestinationAddress,
		LPVOID RequestData, WORD RequestSize,
		LPIP_OPTION_INFORMATION RequestOptions,
		LPVOID ReplyBuffer, DWORD ReplySize, DWORD Timeout);
	typedef IcmpSendEcho* lpIcmpSendEcho;

	//#endif //CPING_USE_ICMP






	/////////////////////////// Classes /////////////////////////////////


	struct CPingReply
	{
		in_addr	 Address;  //The IP address of the replier
		unsigned long RTT; //Round Trip time in Milliseconds
		in_addr	 ip;
	};

	class CPing
	{
	public:
		//Methods
		//#ifdef CPING_USE_ICMP
		BOOL Ping1(LPCTSTR pszHostName, CPingReply& pr, UCHAR nTTL = 10, DWORD dwTimeout = 5000, UCHAR nPacketSize = 32) const;
		//#endif
		//#ifdef CPING_USE_WINSOCK2
		BOOL Ping2(LPCTSTR pszHostName, CPingReply& pr, UCHAR nTTL = 10, DWORD dwTimeout = 5000, UCHAR nPacketSize = 32) const;
		//#endif
	protected:
		//#ifdef CPING_USE_ICMP
		BOOL Initialise1() const;
		static BOOL sm_bAttemptedIcmpInitialise;
		static lpIcmpCreateFile sm_pIcmpCreateFile;
		static lpIcmpSendEcho sm_pIcmpSendEcho;
		static lpIcmpCloseHandle sm_pIcmpCloseHandle;
		//#endif

		//#ifdef CPING_USE_WINSOCK2
		BOOL Initialise2() const;
		static BOOL sm_bAttemptedWinsock2Initialise;
		static BOOL sm_bWinsock2OK;
		//#endif
		static BOOL IsSocketReadible(SOCKET socket, DWORD dwTimeout, BOOL& bReadible);
		static __int64 sm_TimerFrequency;
	};
}