//this file is part of NeoMule
//Copyright (C)2013 David Xanatos ( XanatosDavid (a) gmail.com / http://NeoLoader.to )
//

#include "stdafx.h"
#include "Address.h"

#ifndef WIN32
   #include <unistd.h>
   #include <cstdlib>
   #include <cstring>
   #include <netdb.h>
   #include <arpa/inet.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <fcntl.h>
   #define SOCKET_ERROR (-1)
   #define closesocket close
   #define WSAGetLastError() errno
   #define SOCKET int
   #define INVALID_SOCKET (-1)
#ifdef __APPLE__
    #include "errno.h"
#endif
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #include <wspiapi.h>
   #define EAFNOSUPPORT    102
#endif

CAddress::CAddress(EAF eAF)
{
	m_eAF = eAF;
	memset(m_IP, 0, 16);
}

CAddress::CAddress(const byte* IP)
{
	m_eAF = IPv6;
	memcpy(m_IP, IP, Len());
}

CAddress::CAddress(uint32 IP) // must be same as with Qt
{
	m_eAF = IPv4;
	IP = _ntohl(IP);
	memcpy(m_IP, &IP, Len());
}

uint32 CAddress::ToIPv4() const // must be same as with Qt*/
{
	uint32 ip = 0;
	if(m_eAF == IPv4)
		memcpy(&ip, m_IP, Len());
	return _ntohl(ip);
}

size_t CAddress::Len() const
{
	return m_eAF == IPv6 ? 16 : 4;
}

int CAddress::AF() const
{
	return m_eAF == IPv6 ? AF_INET6 : AF_INET;
}

std::string CAddress::ToString() const
{
	char Dest[65] = {'\0'};
	if(m_eAF != None)
		_inet_ntop(AF(), m_IP, Dest, 65);
	return Dest;
}

bool CAddress::FromString(const std::string Str)
{
	if(Str.find(".") != std::string::npos)
		m_eAF = IPv4;
	else if(Str.find(":") != std::string::npos)
		m_eAF = IPv6;
	else
		return false;
	return _inet_pton(AF(), Str.c_str(), m_IP) == 1;
}

void CAddress::FromSA(const sockaddr* sa, int sa_len, uint16* pPort) 
{
	switch(sa->sa_family)
	{
		case AF_INET:
		{
			ASSERT(sizeof(sockaddr_in) == sa_len);
			sockaddr_in* sa4 = (sockaddr_in*)sa;

			m_eAF = IPv4;
			*((uint32*)m_IP) = sa4->sin_addr.s_addr;
			if(pPort)
				*pPort = _ntohs(sa4->sin_port);
			break;
		}
		case AF_INET6:
		{
			ASSERT(sizeof(sockaddr_in6) == sa_len);
			sockaddr_in6* sa6 = (sockaddr_in6*)sa;

			m_eAF = IPv6;
			memcpy(m_IP, &sa6->sin6_addr, 16);
			if(pPort)
				*pPort  = _ntohs(sa6->sin6_port);
			break;
		}
		default:
			m_eAF = None;
	}
}

void CAddress::ToSA(sockaddr* sa, int *sa_len, uint16 uPort) const
{
	switch(m_eAF)
	{
		case IPv4:
		{
			ASSERT(sizeof(sockaddr_in) <= *sa_len);
			sockaddr_in* sa4 = (sockaddr_in*)sa;
			*sa_len = sizeof(sockaddr_in);
			memset(sa, 0, *sa_len);

			sa4->sin_family = AF_INET;
			sa4->sin_addr.s_addr = *((uint32*)m_IP);
			sa4->sin_port = _ntohs(uPort);
			break;
		}
		case IPv6:
		{
			ASSERT(sizeof(sockaddr_in6) <= *sa_len);
			sockaddr_in6* sa6 = (sockaddr_in6*)sa;
			*sa_len = sizeof(sockaddr_in6);
			memset(sa, 0, *sa_len);

			sa6->sin6_family = AF_INET6;
			memcpy(&sa6->sin6_addr, m_IP, 16);
			sa6->sin6_port = _ntohs(uPort);
			break;
		}
		default:
			ASSERT(0);
	}
}

bool CAddress::IsNull() const
{
	switch(m_eAF)
	{
		case None:	return true;
		case IPv4:	return *((uint32*)m_IP) == INADDR_ANY;
		case IPv6:	return ((uint64*)m_IP)[0] == 0 && ((uint64*)m_IP)[1] == 0;
	}
	return true;
}

bool CAddress::Convert(EAF eAF)
{
	if(eAF == m_eAF)
		return true;
	if(eAF == IPv6)
	{
		m_IP[12] = m_IP[0];
		m_IP[13] = m_IP[1];
		m_IP[14] = m_IP[2];
		m_IP[15] = m_IP[3];

		m_IP[10] = m_IP[11] = 0xFF;
		m_IP[0] = m_IP[1] = m_IP[2] = m_IP[3] = m_IP[4] = m_IP[5] = m_IP[6] = m_IP[7] = m_IP[8] = m_IP[9] = 0;
	}
	else if(m_IP[10] == 0xFF && m_IP[11] == 0xFF 
	 && !m_IP[0] && !m_IP[1] && !m_IP[2] && !m_IP[3] && !m_IP[4] && !m_IP[5] && !m_IP[6] && !m_IP[7] && !m_IP[8] && !m_IP[9])
	{
		m_IP[0] = m_IP[12];
		m_IP[1] = m_IP[13];
		m_IP[2] = m_IP[14];
		m_IP[3] = m_IP[15];
	}
	else
		return false;
	m_eAF = eAF;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// from BSD sources: http://code.google.com/p/plan9front/source/browse/sys/src/ape/lib/bsd/?r=320990f52487ae84e28961517a4fa0d02d473bac

char* _inet_ntop(int af, const void *src, char *dst, int size)
{
	unsigned char *p;
	char *t;
	int i;

	if(af == AF_INET){
		if(size < INET_ADDRSTRLEN){
			errno = ENOSPC;
			return 0;
		}
		p = (unsigned char*)&(((struct in_addr*)src)->s_addr);
		sprintf(dst, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
		return dst;
	}

	if(af != AF_INET6){
		errno = EAFNOSUPPORT;
		return 0;
	}
	if(size < INET6_ADDRSTRLEN){
		errno = ENOSPC;
		return 0;
	}

	p = (unsigned char*)((struct in6_addr*)src)->s6_addr;
	t = dst;
	for(i=0; i<16; i += 2){
		unsigned int w;

		if(i > 0)
			*t++ = ':';
		w = p[i]<<8 | p[i+1];
		sprintf(t, "%x", w);
		t += strlen(t);
	}
	return dst;
}

//////////////////////////////////////////////////////////////////////////////////////

#define CLASS(x)        (x[0]>>6)

int _inet_aton(const char *from, struct in_addr *in)
{
	unsigned char *to;
	unsigned long x;
	char *p;
	int i;

	in->s_addr = 0;
	to = (unsigned char*)&in->s_addr;
	if(*from == 0)
		return 0;
	for(i = 0; i < 4 && *from; i++, from = p){
		x = strtoul(from, &p, 0);
		if(x != (unsigned char)x || p == from)
			return 0;       /* parse error */
		to[i] = x;
		if(*p == '.')
			p++;
		else if(*p != 0)
			return 0;       /* parse error */
	}

	switch(CLASS(to)){
	case 0: /* class A - 1 byte net */
	case 1:
		if(i == 3){
			to[3] = to[2];
			to[2] = to[1];
			to[1] = 0;
		} else if (i == 2){
			to[3] = to[1];
			to[1] = 0;
		}
		break;
	case 2: /* class B - 2 byte net */
		if(i == 3){
			to[3] = to[2];
			to[2] = 0;
		}
		break;
	}
	return 1;
}


static int ipcharok(int c)
{
	return c == ':' || isascii(c) && isxdigit(c);
}

static int delimchar(int c)
{
	if(c == '\0')
		return 1;
	if(c == ':' || isascii(c) && isalnum(c))
		return 0;
	return 1;
}

int _inet_pton(int af, const char *src, void *dst)
{
	int i, elipsis = 0;
	unsigned char *to;
	unsigned long x;
	const char *p, *op;

	if(af == AF_INET)
		return _inet_aton(src, (struct in_addr*)dst);

	if(af != AF_INET6){
		errno = EAFNOSUPPORT;
		return -1;
	}

	to = ((struct in6_addr*)dst)->s6_addr;
	memset(to, 0, 16);

	p = src;
	for(i = 0; i < 16 && ipcharok(*p); i+=2){
		op = p;
		x = strtoul(p, (char**)&p, 16);

		if(x != (unsigned short)x || *p != ':' && !delimchar(*p))
			return 0;                       /* parse error */

		to[i] = x>>8;
		to[i+1] = x;
		if(*p == ':'){
			if(*++p == ':'){        /* :: is elided zero short(s) */
				if (elipsis)
					return 0;       /* second :: */
				elipsis = i+2;
				p++;
			}
		} else if (p == op)             /* strtoul made no progress? */
			break;
	}
	if (p == src || !delimchar(*p))
		return 0;                               /* parse error */
	if(i < 16){
		memmove(&to[elipsis+16-i], &to[elipsis], i-elipsis);
		memset(&to[elipsis], 0, 16-i);
	}
	return 1;
}
