//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#define GET_BIT(x,i) ((x[(i) / 8] >> ((i) % 8)) & 1)
#define SET_BIT(x,i) x[(i) / 8] |=  (1 << ((i) % 8))
#define CLR_BIT(x,i) x[(i) / 8] &= ~(1 << ((i) % 8))

// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
#pragma pack(1)
struct tBlockMap{
	uint8 map[7]; // the last byte contains 3 obional bit flags

	void Reset()						{memset(&map,0,sizeof(uint8)*7);}
	bool IsEmpty()						{return ((map[0] | map[1] | map[2] | map[3] | map[4] | map[5] | (map[6] & 0x1F)) == 0x00); }
	//bool IsFull()						{return ((map[0] & map[1] & map[2] & map[3] & map[4] & map[5] & (map[6] | 0xE0)) == 0xFF); }

	bool IsBlockDone(uint8 block)		{return GET_BIT(map,block) != 0;}
	void SetBlockDone(uint8 block)		{SET_BIT(map,block);}
	void ClearBlockDone(uint8 block)	{CLR_BIT(map,block);}

	void Read(uint8 *BitMap, uint8 uDiv){
			for (uint8 i=0; i < 53; i++){
				if(GET_BIT(BitMap,i/uDiv))
					SET_BIT(map,i);
				else
					CLR_BIT(map,i);
			}
		}

	void Write(uint8 *BitMap, uint8 uDiv){
			uint8 done = 1;
			for (uint8 i=0; i < 53; i++){
				done &= GET_BIT(map,i);
				if((i+1)%uDiv == 0){
					if(done)
						SET_BIT(BitMap,i/uDiv);
					else
						CLR_BIT(BitMap,i/uDiv);
					done = 1; // reset
				}
			}
		}
};
#pragma pack()

typedef CMap<uint16,uint16,tBlockMap,tBlockMap&> CBlockMaps;
// NEO: SCT END <-- Xanatos --