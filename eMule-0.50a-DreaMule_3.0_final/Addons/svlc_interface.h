/*****************************************************************************
 * svlc_interface.h: a simple shared library wrapper around libvlc
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

#ifndef __SVLC_INTERFACE_H
#define __SVLC_INTERFACE_H

#include "simplevlc.h"

/*****************************************************************************
 * Instance data
 *****************************************************************************/

struct _SVlcInstance
{
	/* libvlc handle */
	int vlc;

	/* callback function for notifying the user */
	SVlcCallbackFunc callback;
	
	/* the events the user wants to be notified of. */
	unsigned int cb_events;

	/* user data */
	void *udata;

	/* internal stuff */
	SVlcPlaybackState playback_state;
	int saved_volume; /* used for muting */

};

/*****************************************************************************
 * Interface initialization
 *****************************************************************************/

int svlc_init_interface (SVlcInterface *intf);

#endif /* __SVLC_INTERFACE_H */

