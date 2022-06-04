/**
 *
  */
#ifndef	FAKEALYZER_H
#define	FAKEALYZER_H

#include "SearchFile.h"

class
CFakealyzer
{
public:
	static bool IsFakeKadNode(const uchar* kad_id, uint32 ip, uint16 udp_port);
	static bool IsFakeClient(const uchar* client_hash, uint32 ip, uint16 tcp_port);
	static int CheckSearchResult(CSearchFile* content);
	enum
	{
		UNKNOWN,
		GOOD,
		OK,
		SUSPECT,
		FAKE
	};
};

#endif // FAKEALYZER_H