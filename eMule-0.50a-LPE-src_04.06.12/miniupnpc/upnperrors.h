/* $Id: upnperrors.h,v 1.3 2012/03/05 19:42:47 nanard Exp $ */
/* (c) 2007 Thomas Bernard
 * All rights reserved.
 * MiniUPnP Project.
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * This software is subjet to the conditions detailed in the
 * provided LICENCE file. */
#ifndef __UPNPERRORS_H__
#define __UPNPERRORS_H__

#include "declspec.h"

#ifdef __cplusplus
extern "C" {
#endif

/* strupnperror()
 * Return a string description of the UPnP error code 
 * or NULL for undefinded errors */
LIBSPEC const char * strupnperror(int err);

#ifdef __cplusplus
}
#endif

#endif
