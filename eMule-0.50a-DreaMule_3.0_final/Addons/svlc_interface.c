/*****************************************************************************
 * svlc_interface.c: a simple shared library wrapper around libvlc
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

#include <stdlib.h>

#define __BUILTIN__
#define __VLC__

#include <vlc/vlc.h>
#include <vlc/intf.h>
#include <vlc/input.h>
#include <vlc/vout.h>
#include <vlc/aout.h>
#include <aout_internal.h>

#include "svlc_interface.h"
#include "svlc_event.h"


#define VOUT_PLUGINS "directx,dummy"
#define AOUT_PLUGINS "directx,waveout,dummy"
#define VLC_PLUGIN_DIR "vlcplugins"
#define VLC_CONFIG_FILE "vlc.conf"


/******************************************************************************
 * Some internal helper functions
 *****************************************************************************/

/**
 * This resides in libvcl.c but is not in the headers.
 */
vlc_t * vlc_current_object (int i_object);


static vlc_t * aquire_vlc (HSVLC svlc)
{
    vlc_t *vlc;

	if (!svlc)
		return NULL;

    if (!(vlc = vlc_current_object (svlc->vlc)))
        return NULL;

	return vlc;
}

static void release_vlc (vlc_t *vlc)
{
    vlc_object_release (vlc);
}

/*****************************************************************************
 * Instance management
 *****************************************************************************/

/**
 * Create simplevlc instance.
 *
 * @param verbosity  >= 0 Logging verbosity.
 *                     -1 Quiet, don't log anything.
 *
 * @return  Handle  to instance on success.
 *          NULL on error.
 */
static HSVLC SVLC_CC if_create (int verbosity)
{
	SVlcInstance *svlc;
    vlc_t *vlc;
	int ret;
	vlc_value_t val;

#ifdef DEBUG
	char *ppsz_argv[] = { "vlc",
	                      "--config=" VLC_CONFIG_FILE,
	                      "--plugin-path=" VLC_PLUGIN_DIR,
	                      "--vout=" VOUT_PLUGINS,
	                      "--aout=" AOUT_PLUGINS,
						  "--no-plugins-cache",
	                      "--intf=none",
	                      "--fast-mutex",
	                      "--win9x-cv-method=1" }; 
#else
	char *ppsz_argv[] = { "vlc",
	                      "--config=" VLC_CONFIG_FILE,
	                      "--plugin-path=" VLC_PLUGIN_DIR,
	                      "--vout=" VOUT_PLUGINS,
	                      "--aout=" AOUT_PLUGINS,
						  "--no-plugins-cache",
	                      "--intf=none" };
#endif

	if ((svlc = malloc (sizeof (SVlcInstance))) == NULL)
		return NULL;

	svlc->callback = NULL;
	svlc->cb_events = 0;
	svlc->udata = NULL;
	svlc->playback_state = SVLC_PLAYBACK_CLOSED;
	svlc->saved_volume = -1;

    if((svlc->vlc = VLC_Create()) < 0)
	{
		free (svlc);
		return NULL;
	}

    if((ret = VLC_Init(svlc->vlc, sizeof(ppsz_argv)/sizeof(char*), ppsz_argv)) != VLC_SUCCESS)
	{
        VLC_Destroy (svlc->vlc);
		free (svlc);
		return NULL;
	}

	/* set verbosity */
	val.i_int = verbosity;
	VLC_VariableSet (svlc->vlc, "verbose", val);

	/* add logger interface if verbosity not set to quiet */
	if (verbosity >= 0)
	{
		VLC_AddIntf (svlc->vlc, "logger,none", VLC_FALSE, VLC_FALSE);
	}

    if (!(vlc = aquire_vlc (svlc)))
	{
        VLC_Destroy (svlc->vlc);
		free (svlc);
		return NULL;
	}

	/* create some vars so we start in a consistent state */
    var_Create (vlc, "scale", VLC_VAR_BOOL);
	val.b_bool = VLC_TRUE;
	var_Set (vlc, "scale", val);

    var_Create (vlc, "fullscreen", VLC_VAR_BOOL);
	val.b_bool = VLC_FALSE;
	var_Set (vlc, "fullscreen", val);

    var_Create (vlc, "zoom", VLC_VAR_FLOAT);
	val.f_float = 1.0;
	var_Set (vlc, "zoom", val);

    release_vlc (vlc);

	return svlc;
}

/**
 * Destroy simplevlc instance.
 */
static void SVLC_CC if_destroy (HSVLC svlc)
{
	if (!svlc)
		return;

	/* Don't ask me why there are three functions */
	VLC_Die (svlc->vlc);
	VLC_CleanUp (svlc->vlc);
	VLC_Destroy (svlc->vlc);

	free (svlc);
}

/**
 * Set callback function.
 *
 * @param callback  The function that should be called.
 * @param events    The events you want to be notified about.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_callback (HSVLC svlc, SVlcCallbackFunc callback, unsigned int events)
{
    vlc_t *vlc;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	/* remove any previously registered vlc callbacks */
	if (!svlc_event_unregister_callbacks (svlc, vlc))
	{
	    release_vlc (vlc);
		return -1;
	}

	svlc->callback = NULL;
	svlc->cb_events = 0;

	if (callback != NULL)
	{
		/* set new func and events */
		svlc->callback = callback;
		svlc->cb_events = events;

		/* register vlc callbacks */
		if (!svlc_event_register_callbacks (svlc, vlc))
		{
			release_vlc (vlc);
			return -1;
		}
	}

    release_vlc (vlc);
    
	return 0;
}

/**
 * Return static VLC version string.
 */
static char const * SVLC_CC if_get_vlc_version (void)
{
	return VERSION_MESSAGE;
}

/**
 * Retrieve user data.
 */
static void * SVLC_CC if_get_udata (HSVLC svlc)
{
	if (!svlc)
		return NULL;

	return svlc->udata;
}

/**
 * Attach user data.
 */
static void SVLC_CC if_set_udata (HSVLC svlc, void *udata)
{
	if (!svlc)
		return;

	svlc->udata = udata;
}


/*****************************************************************************
 * Video output
 *****************************************************************************/

/**
 * Set video output window.
 *
 * @param window  Window for video output (HWND on windows).
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_window (HSVLC svlc, unsigned int window)
{
    vlc_t *vlc;
	vlc_value_t value;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    value.i_int = (int) (ptrdiff_t) (void *) window;
	var_Set (vlc, "drawable", value);

    release_vlc (vlc);
    
	return 0;
}

/**
 * Set visualization plugin used for audio playback.
 *
 * @param name  Name of the visualization plugin, e.g. "goom". NULL removes
 *              anything previously set.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_visualization (HSVLC svlc, const char *name)
{
    vlc_t *vlc;
	vlc_value_t value;
    aout_instance_t *aout;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((aout = vlc_object_find (vlc, VLC_OBJECT_AOUT, FIND_CHILD)) &&
		var_Type (aout, "visual") != 0)
    {
		msg_Dbg (vlc, "Setting visualization '%s' directly with aout", name);

		value.psz_string = (name != NULL) ? (char *)name : "";
		var_Set (aout, "visual", value);

		vlc_object_release (aout);
    }
	else
	{
		/* HACKHACK: Set filter directly in vlc object which is inherited by
		 * future aouts.
		 *
		 * FIXME: Does this really work?
		 */
		char *p;

		/* Can happen because of the above ANDed if clause. */
		if (aout)
			vlc_object_release (aout);

		if (var_Type (vlc, "audio-filter") == 0)
			var_Create (vlc, "audio-filter", VLC_VAR_STRING);

		var_Get (vlc, "audio-filter", &value);
		
		if (!value.psz_string)
			value.psz_string = strdup("");

		if ((p = strstr (value.psz_string, name)))
		{
			/* remove if requested */
			if (name == NULL)
			{
				char *rest = p + strlen(name) + ((p[strlen(name)] == ',') ? 1 : 0);
				memmove (p, rest, strlen(rest) + 1);
				var_Set (vlc, "audio-filter", value);
			}
		}
		else
		{
			/* add if requested */
			if (name != NULL)
			{
				char *tmp = value.psz_string;
		
				asprintf (&value.psz_string,
				          (value.psz_string[0] != '\0') ? "%s,%s" : "%s%s",
		                  value.psz_string, name);
				free (tmp);
				var_Set (vlc, "audio-filter", value);
			}
		}

        free (value.psz_string);
	}

    release_vlc (vlc);
    
	return 0;
}

/**
 * Get fullscreen mode.
 *
 * @return  1 Fullscreen on.
 *          0 Fullscreen off.
 *         -1 Error.
 */
static int SVLC_CC if_get_fullscreen (HSVLC svlc)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
	int ret = VLC_ENOVAR;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
		ret = var_Get (vout, "fullscreen", &val);
        vlc_object_release (vout);
    }
    else if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		ret = var_Get (playlist, "fullscreen", &val);
        vlc_object_release (playlist);
    }

	if (ret != VLC_SUCCESS)
		ret = var_Get (vlc, "fullscreen", &val);
 
    release_vlc (vlc);
    
	return (ret == VLC_SUCCESS) ? val.b_bool : -1;
}

/**
 * Set fullscreen mode. This merely maximizes the output window on windows
 * (probably other platforms too) so you need to set fitwindow to true and
 * the window set with set_window must have no (non-maximized) parents.
 *
 * @param fullscreen  1 Switch fullscreen on
 *                    0 Switch fullscreen off
 *                   -1 Toggle fullscreen
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_fullscreen (HSVLC svlc, int fullscreen)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
	int current;

	if((current = if_get_fullscreen (svlc)) < 0)
		return -1;

	if (fullscreen == -1)
		val.b_bool = (current == 0);
	else if (current != fullscreen)
		val.b_bool = (fullscreen != 0);
	else
		return 0; /* the specified state is already set */

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
		/* setting this negates the actual fullscreen while ignoring val */
		var_Set (vout, "fullscreen", val);
	    vlc_object_release (vout);
    }

    if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		var_Set (playlist, "fullscreen", val);
        vlc_object_release (playlist);
    }

	var_Set (vlc, "fullscreen", val);

    release_vlc (vlc);
    
	return 0;
}

/**
 * Get fit to window mode.
 *
 * @return  1 Fitting on.
 *          0 Fitting off.
 *         -1 Error.
 */
static int SVLC_CC if_get_fitwindow (HSVLC svlc)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
	int ret = VLC_ENOVAR;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
		ret = var_Get (vout, "scale", &val);
        vlc_object_release (vout);
    }
    else if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		ret = var_Get (playlist, "scale", &val);
        vlc_object_release (playlist);
    }

	if (ret != VLC_SUCCESS)
		ret = var_Get (vlc, "scale", &val);

    release_vlc (vlc);
    
	return (ret == VLC_SUCCESS) ? val.b_bool : -1;
}

/**
 * Scale video output to match window size.
 *
 * @param fullscreen  1 Fit to window.
 *                    0 Don't fit to window.
 *                   -1 Toggle fit.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_fitwindow (HSVLC svlc, int fit)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
	int current;

	if((current = if_get_fitwindow (svlc)) < 0)
		return -1;

	if (fit == -1)
		val.b_bool = (current == 0);
	else if (current != fit)
		val.b_bool = (fit != 0);
	else
		return 0; /* the specified state is already set */

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
		var_Set (vout, "scale", val);
	    vlc_object_release (vout);
    }

    if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		var_Set (playlist, "scale", val);
        vlc_object_release (playlist);
    }

	var_Set (vlc, "scale", val);

    release_vlc (vlc);
    
	return 0;
}

/**
 * Get current zoom factor.
 *
 * @return >= 0 Current zoom factor.
 *           -1 Error.
 */
static float SVLC_CC if_get_zoom (HSVLC svlc)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
	int ret = VLC_SUCCESS;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
	    ret = var_Get (vout, "zoom", &val);
	    vlc_object_release (vout);
    }
    else if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		ret = var_Get (playlist, "zoom", &val);
        vlc_object_release (playlist);
    }

	if (ret != VLC_SUCCESS)
		ret = var_Get (vlc, "zoom", &val);

    release_vlc (vlc);
    
	return (ret == VLC_SUCCESS) ? val.f_float : -1;
}

/**
 * Zoom video. This only has an effect if fitwindow is false.
 *
 * @param zoom  Zoom factor (e.g. 0.5, 1.0, 1.5, 2.0).
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_zoom (HSVLC svlc, float zoom)
{
    vlc_t *vlc;
    vout_thread_t *vout;
    playlist_t *playlist;
	vlc_value_t val;
    
	if (zoom <= 0)
		return -1;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	val.f_float = zoom;

    if ((vout = vlc_object_find (vlc, VLC_OBJECT_VOUT, FIND_CHILD)))
    {
		var_Set (vout, "zoom", val);

		/* FIXME: This will recreate the picture buffers and everything. I
		 * believe that's a Bad Thing.
		 */
	    vout->i_changes |= VOUT_SIZE_CHANGE;
	    vlc_object_release (vout);
    }

    if ((playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
		var_Set (playlist, "zoom", val);
        vlc_object_release (playlist);
    }

	var_Set (vlc, "zoom", val);

    release_vlc (vlc);
    
	return 0;
}


/*****************************************************************************
 * Audio output
 *****************************************************************************/

/**
 * Get audio volume.
 *
 * @return >= 0 Current volume in [0,1].
 *           -1 Error.
 */
static float SVLC_CC if_get_volume (HSVLC svlc)
{
    vlc_t *vlc;
    audio_volume_t volume;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	if (svlc->saved_volume >= 0)
		volume = svlc->saved_volume;
	else
		aout_VolumeGet (vlc, &volume);
	
    release_vlc (vlc);
    
	return ((float)volume) / AOUT_VOLUME_DEFAULT;
}

/**
 * Set audio volume.
 *
 * @param volume  Volume in [0,1].
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_volume (HSVLC svlc, float volume)
{
    vlc_t *vlc;

	if (volume < 0) volume = 0;
	if (volume > 1) volume = 1;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	/* Note: We use AOUT_VOLUME_DEFAULT instead of AOUT_VOLUME_MAX because the
	 * current software scaling used by vlc uses AOUT_VOLUME_DEFAULT as 100%
	 * of original volume. This prevents sound distortion when going above it.
	 */
	volume *= AOUT_VOLUME_DEFAULT;

	if (svlc->saved_volume >= 0)
		svlc->saved_volume = volume;
	else
	    aout_VolumeSet (vlc, (audio_volume_t)volume);

    release_vlc (vlc);
    
	return 0;
}

/**
 * Get audio mute.
 *
 * @return  1 Muted.
 *          0 Not muted.
 *         -1 Error.
 */
static int SVLC_CC if_get_mute (HSVLC svlc)
{
	return (svlc->saved_volume < 0) ? 0 : 1;
}

/**
 * Set audio mute.
 *
 * @param mute  1 Mute audio
 *              0 Unmute audio
 *             -1 Toggle mute
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_mute (HSVLC svlc, int mute)
{
    vlc_t *vlc;
    audio_volume_t volume;
	int res = 0;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	/* if svlc->saved_volume is negative we are not muted */
    
	if (svlc->saved_volume < 0 && (mute == 1 || mute == -1))
	{
		/* Mute */
		aout_VolumeGet (vlc, &volume);
	    svlc->saved_volume = (int)volume;
        res = aout_VolumeSet (vlc, AOUT_VOLUME_MIN);
	}
	else if (svlc->saved_volume >= 0 && (mute == 0 || mute == -1))
	{
		/* Un-Mute */
        res = aout_VolumeSet (vlc, (audio_volume_t)svlc->saved_volume);
		svlc->saved_volume = -1;
	}

	release_vlc (vlc);

	return res;
}


/*****************************************************************************
 * Playback
 *****************************************************************************/

/**
 * Play target.
 *
 * @param target  Target to play, e.g. filename.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_play (HSVLC svlc, const char *target)
{
    vlc_t *vlc;
    playlist_t *playlist;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

	/* get playlist or create one if necessary */
    if (!(playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
        msg_Dbg (vlc, "no playlist present, creating one" );

        if (!(playlist = playlist_Create (vlc)))
        {
			release_vlc (vlc);
            return -1;
        }

        vlc_object_yield (playlist);

		/* reregister playlist hooks */
		svlc_event_register_callbacks (svlc, vlc);
    }

	/* set loading state */
	svlc_event_set_playback_state (svlc, SVLC_PLAYBACK_LOADING);

	/* empty playlist */
	playlist_Clear (playlist);

	/* add target to playlist and start playback */
    playlist_Add (playlist, target, target, PLAYLIST_INSERT | PLAYLIST_GO, 0);

    vlc_object_release (playlist);
	release_vlc (vlc);

	return 0;
}

/**
 * Stop playback and remove any targets.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_stop (HSVLC svlc)
{
    vlc_t *vlc;
    playlist_t *playlist;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(playlist = vlc_object_find (vlc, VLC_OBJECT_PLAYLIST, FIND_CHILD)))
    {
        msg_Dbg (vlc, "no playlist to stop" );
		release_vlc (vlc);
        return -1;
    }

	/* stop playlist */
    playlist_Stop (playlist);

#if 0 /* race condition with input */
	/* empty playlist */
	playlist_Clear (playlist);
#endif

    vlc_object_release (playlist);
	release_vlc (vlc);

	return 0;
}

/**
 * Set pause.
 *
 * @param pause  1 Pause playback
 *               0 Resume playback
 *              -1 Toggle pause
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_pause (HSVLC svlc, int pause)
{
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t status;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

	var_Get (input, "state", &status);
	
	if (status.i_int == PAUSE_S)
	{
		if (pause == -1 || pause == 0)
		{
			status.i_int = PLAYING_S;
			/* calling var_Set twice with PAUSE_S resumes playback */
			var_Set (input, "state", status);
		}
	}
	else
	{
		if (pause == -1 || pause == 1)
		{
			status.i_int = PAUSE_S;
			var_Set (input, "state", status);
		}
	}

    vlc_object_release (input);
	release_vlc (vlc);

	return 0;
}

/**
 * Retrieve playback state.
 *
 * @return  See declaration of SVlcPlaybackState
 */
static SVlcPlaybackState SVLC_CC if_get_playback_state (HSVLC svlc)
{
#if 0
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t status;

    if (!(vlc = aquire_vlc (svlc)))
        return SVLC_PLAYBACK_STOPPED;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return SVLC_PLAYBACK_CLOSED;
    }

	var_Get (input, "state", &status);

    vlc_object_release (input);
	release_vlc (vlc);

	switch (status.i_int)
	{
	case INIT_S:		return SVLC_PLAYBACK_LOADING;
	case PLAYING_S:		return SVLC_PLAYBACK_PLAYING;
	case PAUSE_S:		return SVLC_PLAYBACK_PAUSED;
	case END_S:         return SVLC_PLAYBACK_STOPPED;
	}

	return SVLC_PLAYBACK_CLOSED;
#else
	return svlc->playback_state;
#endif
}

/**
 * Get current stream position.
 *
 * @return >= 0 Current postition in [0,1].
 *           -1 Error.
 */
static float SVLC_CC if_get_position (HSVLC svlc)
{
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t position;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

    vlc_mutex_lock (&input->object_lock);
	var_Get (input, "position", &position);
    vlc_mutex_unlock (&input->object_lock);

    vlc_object_release (input);
	release_vlc (vlc);

	return position.f_float;
}

/**
 * Seek to new stream position (if seekable).
 *
 * @param position  Position in [0,1] to seek to.
 *
 * @return  0 Success.
 *         -1 Error.
 */
static int SVLC_CC if_set_position (HSVLC svlc, float position)
{
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t val;

	if (position < 0) position = 0;
	if (position > 1) position = 1;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

	var_Get (input, "seekable", &val);

	if (!val.b_bool)
	{
	    vlc_object_release (input);
		release_vlc (vlc);
		return -1;
	}

	val.f_float = position;
	var_Set (input, "position", val);

    vlc_object_release (input);
	release_vlc (vlc);

	return 0;
}

/**
 * Get seekability of current stream.
 *
 * @return  1 Current stream is seekable
 *          0 Current stream is not seekable
 *         -1 Error.
 */
static int SVLC_CC if_is_seekable (HSVLC svlc)
{
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t val;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

	var_Get (input, "seekable", &val);

    vlc_object_release (input);
	release_vlc (vlc);

	return val.b_bool;
}

/**
 * Get play length of current stream.
 *
 * @return > 0 Stream duration in milliseconds.
 *           0 No time data available for this stream.
 *          -1 Error.
 */
static int SVLC_CC if_get_duration (HSVLC svlc)
{
    vlc_t *vlc;
    input_thread_t *input;
    vlc_value_t length;

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

    var_Get (input, "length", &length);

    vlc_object_release (input);
	release_vlc (vlc);

	return (length.i_time / 1000);
}

/**
 * Get information about an the currently opened stream.
 *
 * @param info  Pointer to SVlcStreamInfo which is filled with the data.
 *
 * @return  0 Success.
 *         -1 Error.
 */
int SVLC_CC if_get_stream_info (HSVLC svlc, SVlcStreamInfo *info)
{
    vlc_t *vlc;
    input_thread_t *input;
	vlc_value_t val;

	if (!info)
		return -1;

	if (svlc->playback_state != SVLC_PLAYBACK_OPEN &&
	    svlc->playback_state != SVLC_PLAYBACK_PLAYING &&
	    svlc->playback_state != SVLC_PLAYBACK_PAUSED &&
	    svlc->playback_state != SVLC_PLAYBACK_STOPPED)
	{
		return -1;
	}

    if (!(vlc = aquire_vlc (svlc)))
        return -1;

    if (!(input = vlc_object_find (vlc, VLC_OBJECT_INPUT, FIND_CHILD)))
    {
	    release_vlc (vlc);
        return -1;
    }

	/* audio */
    var_Change (input, "audio-es", VLC_VAR_CHOICESCOUNT, &val, NULL);
	/* - 1 for "Disabled" choice */
	info->audio_streams = val.i_int > 0 ? val.i_int - 1 : 0;

	/* video */
    var_Change (input, "video-es", VLC_VAR_CHOICESCOUNT, &val, NULL);
	/* - 1 for "Disabled" choice */
	info->video_streams = val.i_int > 0 ? val.i_int - 1 : 0;

#ifdef DEBUG
	msg_Dbg (input, "audio streams: %d, video_streams: %d",
	         info->audio_streams, info->video_streams);
#endif

    vlc_object_release (input);
	release_vlc (vlc);

	/* demuxer doesn't yet know how much streams there are */
	if (info->audio_streams == 0 && info->video_streams == 0)
		return -1;

	return 0;
}

/*****************************************************************************
 * Interface initialization
 *****************************************************************************/

int svlc_init_interface (SVlcInterface *intf)
{
	if (intf == NULL)
		return -1;

	/* instance management */
	intf->create				= if_create;
	intf->destroy				= if_destroy;
	intf->set_callback			= if_set_callback;
	intf->get_vlc_version		= if_get_vlc_version;
	intf->set_udata				= if_set_udata;
	intf->get_udata				= if_get_udata;

	/* video out */
	intf->set_window			= if_set_window;
	intf->set_visualization     = if_set_visualization;
	intf->set_fullscreen		= if_set_fullscreen;
	intf->get_fullscreen		= if_get_fullscreen;
	intf->set_fitwindow			= if_set_fitwindow;
	intf->get_fitwindow			= if_get_fitwindow;
	intf->set_zoom				= if_set_zoom;
	intf->get_zoom				= if_get_zoom;

	/* audio out */
	intf->set_volume			= if_set_volume;
	intf->get_volume			= if_get_volume;
	intf->set_mute				= if_set_mute;
	intf->get_mute				= if_get_mute;

	/* playback */
	intf->play					= if_play;
	intf->stop					= if_stop;
	intf->pause					= if_pause;
	intf->get_playback_state	= if_get_playback_state;
	intf->set_position			= if_set_position;
	intf->get_position			= if_get_position;
	intf->is_seekable			= if_is_seekable;
	intf->get_duration			= if_get_duration;
	intf->get_stream_info		= if_get_stream_info;

	return 0;
}
