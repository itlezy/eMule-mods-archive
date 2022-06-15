#pragma once

// This file provides a way for local compiler site configurations (e.g. installed platform SDK).

// By default, assume that we have the latest SDKs and enable all optional features
#define	HAVE_SAPI_H
#define HAVE_QEDIT_H

//////////////////////////////////////////////////////////////////////////////
// Visual Studio 2003
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER==1310

// 'sapi.h' is not shipped with VS2003.
// Uncomment the following line if you get compile errors due to missing 'sapi.h'
//#undef HAVE_SAPI_H

// 'qedit.h' is shipped with VS2003.
//#undef HAVE_QEDIT_H

#endif


//////////////////////////////////////////////////////////////////////////////
// Visual Studio 2005
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER==1400	// Visual Studio 2005

// NOTE: eMule can not get compiled with VS2005 out of the box because the SDK
// which is shipped with VS2005 does not contain the ‘upnp.h’ header file - and
// this feature is not yet optional for compiling eMule. Thus you need to install
// an additional more recent SDK when compiling with VS2005.
//
// It is supposed that eMule can get compiled with VS2005 and the latest 
// "Windows Server 2003 (Windows XP) SDK" - but it was not yet verified.
//
// It is known that eMule can get compiled with VS2005 and the "Vista SDK 6.0/6.1".
// To compile eMule with the "Vista SDK 6.0/6.1", define 'HAVE_VISTA_SDK'.

#define HAVE_VISTA_SDK
#define HAVE_DIRECTX_SDK

// 'sapi.h' is not shipped with VS2005.
// 'sapi.h' is shipped with Vista SDK 6.1
// You need to install the Speach SDK to enable this feature.
#ifndef HAVE_VISTA_SDK
#undef HAVE_SAPI_H
#endif//HAVE_VISTA_SDK

// 'qedit.h' file is shipped with VS2005, but it needs an additional file ('ddraw.h') which
// is only shipped with the DirectX SDK.
// You need to install the DirectX SDK to enable this feature.
#if !defined(HAVE_VISTA_SDK) || !defined(HAVE_DIRECTX_SDK)
#undef HAVE_QEDIT_H
#endif//!defined(HAVE_VISTA_SDK) || !defined(HAVE_DIRECTX_SDK)

#endif


//////////////////////////////////////////////////////////////////////////////
// Visual Studio 2008
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER==1500

#define HAVE_VISTA_SDK		// VS2008 is already shipped with a Vista SDK
#define HAVE_DIRECTX_SDK

// 'sapi.h' is shipped with VS2008 as part of the Vista SDK
#ifndef HAVE_VISTA_SDK
#undef HAVE_SAPI_H
#endif//HAVE_VISTA_SDK

// 'qedit.h' file is shipped with VS2008, but it needs an additional file ('ddraw.h') which
// is only shipped with the DirectX SDK.
// You need to install the DirectX SDK to enable this feature.
#if !defined(HAVE_VISTA_SDK) || !defined(HAVE_DIRECTX_SDK)
#undef HAVE_QEDIT_H
#endif//!defined(HAVE_VISTA_SDK) || !defined(HAVE_DIRECTX_SDK)

#endif
