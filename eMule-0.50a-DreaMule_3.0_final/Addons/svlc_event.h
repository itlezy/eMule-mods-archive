/*****************************************************************************
 * svlc_event.h: a simple shared library wrapper around libvlc
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

#ifndef __SVLC_EVENT_H
#define __SVLC_EVENT_H

#include "simplevlc.h"
#include "svlc_interface.h"

/*****************************************************************************
 * Our vlc object used with the polling thread
 *****************************************************************************/

#if 0

typedef struct
{
    VLC_COMMON_MEMBERS

	/* pointer to svlc instance */
	HSVLC svlc;
} svlc_event_thread_t;

/* create and start event polling thread */
svlc_event_thread_t * svlc_event_create_thread (HSVLC svlc, vlc_t *vlc);

/* join and free event polling thread */
vlc_bool_t svlc_event_join_thread (svlc_event_thread_t *thread);

#endif

/*****************************************************************************
 * Event registration
 *****************************************************************************/

vlc_bool_t svlc_event_register_callbacks (HSVLC svlc, vlc_t *vlc);
vlc_bool_t svlc_event_unregister_callbacks (HSVLC svlc, vlc_t *vlc);

vlc_bool_t svlc_event_set_playback_state (HSVLC svlc,
										  SVlcPlaybackState new_state);

#endif /* __SVLC_INTERFACE_H */

