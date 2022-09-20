/*****************************************************************************
 * svlc_event.c: a simple shared library wrapper around libvlc
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

/*****************************************************************************
 * Preamble
 *****************************************************************************/
#include "config.h"

//#include <stdlib.h>

#define __BUILTIN__
#define __VLC__

#include <vlc/vlc.h>
#include <vlc/intf.h>
#include <vlc/input.h>
#include <vlc/vout.h>
#include <vlc/aout.h>

#include "svlc_event.h"

#define EVENT_THREAD_SLEEP 100 /* ms */

/******************************************************************************
 * Some internal helper functions
 *****************************************************************************/

#ifdef DEBUG

/* This resides in libvcl.c but is not in the headers. */
vlc_t * vlc_current_object (int i_object);

static char * state_str (SVlcPlaybackState state)
{
	switch (state)
	{
	case SVLC_PLAYBACK_CLOSED:  return "Closed";
	case SVLC_PLAYBACK_LOADING: return "Loading";
	case SVLC_PLAYBACK_OPEN:    return "Open";
	case SVLC_PLAYBACK_PLAYING: return "Playing";
	case SVLC_PLAYBACK_PAUSED:  return "Paused";
	case SVLC_PLAYBACK_STOPPED: return "Stopped";
	default:                    return "UNKNOWN";
	};
}

static void log_state_change (HSVLC svlc, SVlcPlaybackState old_state,
                              SVlcPlaybackState new_state)
{
    vlc_t *vlc;

    if (!(vlc = vlc_current_object (svlc->vlc)))
        return;
 
    msg_Dbg (vlc, "STATE CHANGE: %s => %s",
	         state_str (old_state), state_str (new_state));

    vlc_object_release (vlc);
}

#endif

/*****************************************************************************
 * Polling thread
 *****************************************************************************/

#if 0

/* polling thread function */
static void event_thread_func (svlc_event_thread_t *thread)
{
#if 0
    playlist_t *playlist;

	/* find playlist */
    if (!(playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
	{

	}

	/* start with closed state */	
	svlc_event_set_playback_state (thread->svlc, SVLC_PLAYBACK_CLOSED);
#endif
    
	/* tell creating thread that we're ready */
    vlc_thread_ready (thread);

	while (!thread->b_die)
	{
		msleep (EVENT_THREAD_SLEEP);
	}

#if 0
	/* end with closed state */	
	svlc_event_set_playback_state (thread->svlc, SVLC_PLAYBACK_CLOSED);

	vlc_object_release (playlist);
#endif
}

/* create and start event polling thread */
svlc_event_thread_t * svlc_event_create_thread (HSVLC svlc, vlc_t *vlc)
{
	svlc_event_thread_t *thread;

    if(!(thread = vlc_object_create (vlc, sizeof (svlc_event_thread_t))))
    {
        msg_Err (vlc, "out of memory");
        return NULL;
    }

	thread->svlc = svlc;

	/* start thread and wait till it is ready */
    if (vlc_thread_create( thread, "svlc_event_thread", event_thread_func,
                           VLC_THREAD_PRIORITY_LOW, VLC_TRUE ) != 0)
    {
        msg_Err (thread, "cannot spawn event polling thread");
        vlc_object_destroy (thread);
        return NULL;
    }

#if 0
    /* The object has been initialized, now attach it */
    vlc_object_attach (thread, vlc);
#endif

	return thread;
}

/* join and free event polling thread */
vlc_bool_t svlc_event_join_thread (svlc_event_thread_t *thread)
{
	/* wait for thread completion */
	thread->b_die = 1;
    vlc_thread_join (thread);

	/* free object */
    vlc_object_destroy (thread);

	return VLC_TRUE;
}

#endif

/*****************************************************************************
 * Handlers for VLC variable changes
 *****************************************************************************/

static int popup_callback (vlc_object_t *this, char const *cmd,
                           vlc_value_t oldval, vlc_value_t newval,
                           void *data)
{
	HSVLC svlc = (HSVLC)data;

	if (newval.b_bool != oldval.b_bool)
	{
		SVlcCbDisplayPopupData popup_data;
		popup_data.show = newval.b_bool;

		svlc->callback (svlc, SVLC_CB_DISPLAY_POPUP, (void*)&popup_data,
		                svlc->udata);
	}

    return VLC_SUCCESS;
}

static int key_pressed_callback (vlc_object_t *this, char const *cmd,
                                 vlc_value_t oldval, vlc_value_t newval,
                                 void *data)
{
	HSVLC svlc = (HSVLC)data;
	SVlcCbKeyPressedData key_data;

#ifdef DEBUG
	vlc_t *vlc = (vlc_t *)this;
    msg_Dbg (vlc, "cmd=%s old=%d new=%d", cmd,
	         oldval.i_int, newval.i_int);
#endif

	key_data.key = newval.i_int;

	svlc->callback (svlc, SVLC_CB_KEY_PRESSED, (void*)&key_data,
	                svlc->udata);

    return VLC_SUCCESS;
}

static int intf_change_callback (vlc_object_t *this, char const *cmd,
                                 vlc_value_t oldval, vlc_value_t newval,
                                 void *data)
{
	HSVLC svlc = (HSVLC)data;
    input_thread_t *input = (input_thread_t *)this;
	SVlcPlaybackState new_state;

#if 0
    msg_Dbg (input, "cmd=%s old=%d new=%d", cmd,
	         oldval.i_int, newval.i_int);
#endif

	/* check for state changes */

	switch (input->i_state)
	{
	case INIT_S:	new_state = SVLC_PLAYBACK_LOADING; break;
	case PLAYING_S:	new_state = SVLC_PLAYBACK_PLAYING; break;
	case PAUSE_S:	new_state = SVLC_PLAYBACK_PAUSED;  break;
	case END_S:     new_state = SVLC_PLAYBACK_STOPPED; break;
	default:        new_state = svlc->playback_state; /* change nothing */
	}

	if(input->b_error)
		new_state = SVLC_PLAYBACK_ERROR; /* implies SVLC_PLAYBACK_CLOSED */

	/* if we go from loading to playing squeze an open in */
	if (svlc->playback_state == SVLC_PLAYBACK_LOADING &&
	    new_state == SVLC_PLAYBACK_PLAYING)
		svlc_event_set_playback_state (svlc, SVLC_PLAYBACK_OPEN);

	svlc_event_set_playback_state (svlc, new_state);

	/* currently stopping means closing as well */
	if (new_state == SVLC_PLAYBACK_STOPPED)
		svlc_event_set_playback_state (svlc, SVLC_PLAYBACK_CLOSED);


	/* update position */

	if (svlc->cb_events & SVLC_CB_POSITION_CHANGE &&
	    svlc->playback_state == SVLC_PLAYBACK_PLAYING)
	{
		vlc_value_t val;
		SVlcCbPositionChangeData position_data;

		var_Get (input, "position", &val);
		position_data.position = val.f_float;

		var_Get (input, "length", &val);
		position_data.duration = val.i_time / 1000;

/*
		msg_Dbg (input, "POSITION: %f", position_data.position);
*/

		/* call back to user */
		svlc->callback (svlc, SVLC_CB_POSITION_CHANGE, (void*)&position_data,
			            svlc->udata);
	}

    return VLC_SUCCESS;
}

/* we use this to hook the new input */
static int input_change_callback (vlc_object_t *this, char const *cmd,
                                  vlc_value_t oldval, vlc_value_t newval,
                                  void *data)
{
	HSVLC svlc = (HSVLC)data;
    playlist_t *playlist = (playlist_t *)this;

#ifdef DEBUG
    msg_Dbg (playlist, "cmd=%s old=%d new=%d", cmd,
	         oldval.i_int, newval.i_int);
#endif

	/* Clearly we are loading with a new input */
	svlc_event_set_playback_state (svlc, SVLC_PLAYBACK_LOADING);

	/* hook new input */
    vlc_mutex_lock (&playlist->object_lock);

	var_AddCallback (playlist->p_input, "intf-change",
		             intf_change_callback, (void*)svlc);
    
	vlc_mutex_unlock (&playlist->object_lock);

    return VLC_SUCCESS;
}

/*****************************************************************************
 * Event registration
 *****************************************************************************/

vlc_bool_t svlc_event_register_callbacks (HSVLC svlc, vlc_t *vlc)
{
	input_thread_t *input;
	playlist_t     *playlist;

	/* vlc events */

	if (svlc->cb_events & SVLC_CB_KEY_PRESSED)
	{
		var_AddCallback (vlc, "key-pressed", key_pressed_callback,
		                 (void*)svlc);
	}

	/* input events */

    if ((input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
	{
		vlc_object_release (input);
	}

	/* playlist events */

    if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
	{
		if (svlc->cb_events & SVLC_CB_DISPLAY_POPUP)
			var_AddCallback (playlist, "intf-popupmenu",
			                 popup_callback, (void*)svlc);

		/* we always want this to hook the input if neccessary */
		var_AddCallback (playlist, "playlist-current",
		                 input_change_callback, (void*)svlc);

		vlc_object_release (playlist);
	}

	return VLC_TRUE;
}

vlc_bool_t svlc_event_unregister_callbacks (HSVLC svlc, vlc_t *vlc)
{
	input_thread_t *input;
	playlist_t     *playlist;

	/* vlc events */

	if (svlc->cb_events & SVLC_CB_KEY_PRESSED)
	{
		var_DelCallback (vlc, "key-pressed", key_pressed_callback,
		                 (void*)svlc);
	}

	/* input events */
    if ((input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
	{
		vlc_object_release (input);
	}

	/* playlist events */

    if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
	{
		if (svlc->cb_events & SVLC_CB_DISPLAY_POPUP)
			var_DelCallback (playlist, "intf-popupmenu",
			                 popup_callback, (void*)svlc);

		/* we always want this to hook the input if neccessary */
		var_DelCallback (playlist, "playlist-current",
		                 input_change_callback, (void*)svlc);
		
		vlc_object_release (playlist);
	}

	return VLC_TRUE;
}

vlc_bool_t svlc_event_set_playback_state (HSVLC svlc,
										  SVlcPlaybackState new_state)
{
	SVlcCbStateData state_data;

	if (new_state == svlc->playback_state)
		return VLC_FALSE;

	state_data.old_state = svlc->playback_state;
	state_data.new_state = new_state;

	svlc->playback_state = new_state;

#ifdef DEBUG
	log_state_change (svlc, state_data.old_state, state_data.new_state);
#endif

	if (svlc->cb_events & SVLC_CB_STATE_CHANGE)
	{
		/* call back to user */
		svlc->callback (svlc, SVLC_CB_STATE_CHANGE, (void*)&state_data,
		                svlc->udata);
	}

    return VLC_SUCCESS;
}

