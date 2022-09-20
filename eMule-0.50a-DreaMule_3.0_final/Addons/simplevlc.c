/*****************************************************************************
 * simplevlc.c: a simple shared library wrapper around libvlc
 *****************************************************************************
 * Copyright (C) 2004 Markus Kern <mkern@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#include <stdlib.h>

#include "simplevlc.h"
#include "svlc_interface.h"


int SVLC_CC SVLC_GetInterface(SVlcInterface *intf)
{
	if (intf == NULL)
		return -1;

	if (svlc_init_interface (intf) < 0)
		return -1;

	return 0;
}


void SVLC_CC SVLC_GetVersion(int *major, int *minor, int *micro)
{
	if (major)
		*major = SVLC_VERSION_MAJOR;

	if (minor)
		*minor = SVLC_VERSION_MINOR;

	if (micro)
		*micro = SVLC_VERSION_MICRO;
}


int SVLC_CC SVLC_Initialize(void)
{
	return 0;
}


void SVLC_CC SVLC_Shutdown(void)
{
	return;
}
