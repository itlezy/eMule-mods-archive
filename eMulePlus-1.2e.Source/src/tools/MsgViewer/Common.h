#pragma once

#include <vector>
using namespace std;

struct CTask_Tcp {};
struct CEmEngine {};
enum T_CLIENT_TYPE {
	T_CLIENT_PEER,
	T_CLIENT_SERVER,
	T_CLIENT_PEER_SERVER,
	T_CLIENT_WEB
};

#define AFX_TASKPROCESSOR_H__186018D8_26C9_4A91_9F7C_DDC406696088__INCLUDED_
#define AFX_TASKS_H__595D01B7_787E_4BAA_8F57_B70CFE759503__INCLUDED_

#ifndef PCVOID
#	define PCVOID LPCVOID
#endif

#ifndef _countof
#	define _countof(x) (sizeof(x) / sizeof((x)[0]))
#endif

typedef struct
{
	BYTE		hash[16];
} HashType;

struct AddrPort
{
	ULONG	Addr;
	USHORT	Port;
};

struct CEmClient_Peer;

#include "../../Engine/Sockets/OpCode.h"
