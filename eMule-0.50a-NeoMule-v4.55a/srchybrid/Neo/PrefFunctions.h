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

////////////////////////////////////
#ifndef SEC2MS
#define	SEC2MS(sec)	((sec)*1000)
#define	MIN2MS(min)	SEC2MS((min)*60)
#define	HR2MS(hr)	MIN2MS((hr)*60)

#define	MIN2S(min)	((min)*60)
#define	HR2S(hr)	MIN2S((hr)*60)
#endif

#define	MS2SEC(ms)	((ms)/1000)
#define	MS2MIN(ms)	MS2SEC((ms)/60)
#define	MS2HR(ms)	MS2MIN((ms)/60)

#define	S2MIN(s)	((s)/60)
#define	S2HR(s)		S2MIN((s)/60)

#define D2S(d)		HR2S((d)*24)
#define S2D(s)		S2HR((s)/24)

#define	MB2B(mb)	KB2B((mb)*1024)
#define	B2MB(b)		B2KB((b)/1024)

#define	KB2B(kb)	((kb)*1024)
#define	B2KB(b)		((b)/1024)
//////////////////////////////////

#ifndef UNLIMITED
#define UNLIMITED				0xFFFF
#endif


#define I2B(x) ((x) ? true : false)