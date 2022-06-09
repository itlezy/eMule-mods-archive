/*
 * File:	ximage.h
 * Purpose:	General Purpose Image Class 
 */
/*
  --------------------------------------------------------------------------------

	COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:

	CxImage version 7.0.2 07/Feb/2011

	CxImage : Copyright (C) 2001 - 2010, Davide Pizzolato

	Original CImage and CImageIterator implementation are:
	Copyright (C) 1995, Alejandro Aguilar Sierra (asierra(at)servidor(dot)unam(dot)mx)

	Covered code is provided under this license on an "as is" basis, without warranty
	of any kind, either expressed or implied, including, without limitation, warranties
	that the covered code is free of defects, merchantable, fit for a particular purpose
	or non-infringing. The entire risk as to the quality and performance of the covered
	code is with you. Should any covered code prove defective in any respect, you (not
	the initial developer or any other contributor) assume the cost of any necessary
	servicing, repair or correction. This disclaimer of warranty constitutes an essential
	part of this license. No use of any covered code is authorized hereunder except under
	this disclaimer.

	Permission is hereby granted to use, copy, modify, and distribute this
	source code, or portions hereof, for any purpose, including commercial applications,
	freely and without fee, subject to the following restrictions: 

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.

  --------------------------------------------------------------------------------

	Other information about CxImage, and the latest version, can be found at the
	CxImage home page: http://www.xdp.it/cximage/

  --------------------------------------------------------------------------------
 */
#if !defined(__CXIMAGE_H)
#define __CXIMAGE_H

#if _MSC_VER > 1000
#pragma once
#endif 

#ifdef _LINUX
  #define _XOPEN_SOURCE
  #include <unistd.h>
  #include <arpa/inet.h>
#endif

/////////////////////////////////////////////////////////////////////////////
#include "xfile.h"
#include "xiofile.h"
#include "xmemfile.h"
#include "ximadef.h"	//<vho> adjust some #define

/* see "ximacfg.h" for CxImage configuration options */

/////////////////////////////////////////////////////////////////////////////
// CxImage formats enumerator
enum ENUM_CXIMAGE_FORMATS{
CXIMAGE_FORMAT_UNKNOWN = 0,
#if CXIMAGE_SUPPORT_BMP
CXIMAGE_FORMAT_BMP = 1,
#endif
#if CXIMAGE_SUPPORT_PNG
CXIMAGE_FORMAT_PNG = 2,
#endif
CMAX_IMAGE_FORMATS = CXIMAGE_SUPPORT_BMP + CXIMAGE_SUPPORT_PNG + 1
};
/////////////////////////////////////////////////////////////////////////////
// CxImage class
/////////////////////////////////////////////////////////////////////////////
class DLL_EXP CxImage
{
//extensible information collector
typedef struct tagCxImageInfo {
	uint32_t	dwEffWidth;			///< uint32_t aligned scan line width
	uint8_t*	pImage;				///< THE IMAGE BITS
	CxImage* pGhost;			///< if this is a ghost, pGhost points to the body
	CxImage* pParent;			///< if this is a layer, pParent points to the body
	uint32_t	dwType;				///< original image format
	char	szLastError[256];	///< debugging
	int32_t	nProgress;			///< monitor
	int32_t	nEscape;			///< escape
	int32_t	nBkgndIndex;		///< used for GIF, PNG, MNG
	RGBQUAD nBkgndColor;		///< used for RGB transparency
	float	fQuality;			///< used for JPEG, JPEG2000 (0.0f ... 100.0f)
	uint8_t	nJpegScale;			///< used for JPEG [ignacio]
	int32_t	nFrame;				///< used for TIF, GIF, MNG : actual frame
	int32_t	nNumFrames;			///< used for TIF, GIF, MNG : total number of frames
	uint32_t	dwFrameDelay;		///< used for GIF, MNG
	int32_t	xDPI;				///< horizontal resolution
	int32_t	yDPI;				///< vertical resolution
	RECT	rSelectionBox;		///< bounding rectangle
	uint8_t	nAlphaMax;			///< max opacity (fade)
	bool	bAlphaPaletteEnabled; ///< true if alpha values in the palette are enabled.
	bool	bEnabled;			///< enables the painting functions
	int32_t	xOffset;
	int32_t	yOffset;
	uint32_t	dwCodecOpt[CMAX_IMAGE_FORMATS];	///< for GIF, TIF : 0=def.1=unc,2=fax3,3=fax4,4=pack,5=jpg
	RGBQUAD last_c;				///< for GetNearestIndex optimization
	uint8_t	last_c_index;
	bool	last_c_isvalid;
	int32_t	nNumLayers;
	uint32_t	dwFlags;			///< 0x??00000 = reserved, 0x00??0000 = blend mode, 0x0000???? = layer id - user flags
	uint8_t	dispmeth;
	bool	bGetAllFrames;
	bool	bLittleEndianHost;

} CXIMAGEINFO;

public:
	//public structures
struct rgb_color { uint8_t r,g,b; };

/** \addtogroup Constructors */ //@{
	CxImage(uint32_t imagetype = 0);
	CxImage(uint32_t dwWidth, uint32_t dwHeight, uint32_t wBpp, uint32_t imagetype = 0);
	CxImage(uint8_t * buffer, uint32_t size, uint32_t imagetype);
	virtual ~CxImage() { DestroyFrames(); Destroy(); };
//@}

/** \addtogroup Initialization */ //@{
	void*	Create(uint32_t dwWidth, uint32_t dwHeight, uint32_t wBpp, uint32_t imagetype = 0);
	bool	Destroy();
	bool	DestroyFrames();
	void	Clear(uint8_t bval=0);
	void	Copy(const CxImage &src, bool copypixels = true, bool copyselection = true, bool copyalpha = true);
	bool	Transfer(CxImage &from, bool bTransferFrames = true);

//@}

/** \addtogroup Attributes */ //@{
	int32_t	GetSize();
	uint8_t*	GetBits(uint32_t row = 0);
	void*	GetDIB() const;
	uint32_t	GetHeight() const;
	uint32_t	GetWidth() const;
	uint32_t	GetEffWidth() const;
	uint32_t	GetNumColors() const;
	uint16_t	GetBpp() const;
	uint32_t	GetType() const;
	const char*	GetLastError();
	void	SetXDPI(int32_t dpi);
	void	SetYDPI(int32_t dpi);
	uint32_t	GetClrImportant() const;
	void	SetClrImportant(uint32_t ncolors = 0);
	int32_t	GetTransIndex() const;
	RGBQUAD	GetTransColor();
	void	SetTransIndex(int32_t idx);
	uint32_t	GetCodecOption(uint32_t imagetype = 0);
	bool	SetCodecOption(uint32_t opt, uint32_t imagetype = 0);
	static uint32_t GetTypeIndexFromId(const uint32_t id);
//@}

/** \addtogroup Palette
 * These functions have no effects on RGB images and in this case the returned value is always 0.
 * @{ */
	bool	IsGrayScale();
	bool	IsIndexed() const;
	uint32_t	GetPaletteSize();
	RGBQUAD* GetPalette() const;
	RGBQUAD GetPaletteColor(uint8_t idx);
	bool	GetPaletteColor(uint8_t i, uint8_t* r, uint8_t* g, uint8_t* b);
	uint8_t	GetNearestIndex(RGBQUAD c);
	void	SetGrayPalette();
	void	SetPalette(RGBQUAD* pPal,uint32_t nColors=256);
	void	SetPalette(rgb_color *rgb,uint32_t nColors=256);
	void	SetPaletteColor(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha=0);
	void	SetPaletteColor(uint8_t idx, RGBQUAD c);
	void	SwapIndex(uint8_t idx1, uint8_t idx2);
	void	SetStdPalette();
//@}

/** \addtogroup Pixel */ //@{
	bool	IsInside(int32_t x, int32_t y);
	RGBQUAD GetPixelColor(int32_t x,int32_t y, bool bGetAlpha = true);
	uint8_t	GetPixelIndex(int32_t x,int32_t y);
	void	SetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha = false);
	void	SetPixelColor(int32_t x,int32_t y,COLORREF cr);
	void	SetPixelIndex(int32_t x,int32_t y,uint8_t i);
//@}

protected:
/** \addtogroup Protected */ //@{
	uint8_t BlindGetPixelIndex(const int32_t x,const int32_t y);
	RGBQUAD BlindGetPixelColor(const int32_t x,const int32_t y, bool bGetAlpha = true);
	void *BlindGetPixelPointer(const int32_t x,const  int32_t y);
	void BlindSetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha = false);
	void BlindSetPixelIndex(int32_t x,int32_t y,uint8_t i);
//@}

public:

#if CXIMAGE_SUPPORT_INTERPOLATION
/** \addtogroup Interpolation */ //@{
	//overflow methods:
	enum OverflowMethod {
		OM_BACKGROUND=2,
		OM_REPEAT=5
	};
	void OverflowCoordinates(int32_t  &x, int32_t &y, OverflowMethod const ofMethod);
	RGBQUAD GetPixelColorWithOverflow(int32_t x, int32_t y, OverflowMethod const ofMethod=OM_BACKGROUND, RGBQUAD* const rplColor=0);
	//interpolation methods:
	enum InterpolationMethod {
		IM_BILINEAR		=2
	};
	RGBQUAD GetPixelColorInterpolated(float x,float y, InterpolationMethod const inMethod=IM_BILINEAR, OverflowMethod const ofMethod=OM_BACKGROUND, RGBQUAD* const rplColor=0);
	RGBQUAD GetAreaColorInterpolated(float const xc, float const yc, float const w, float const h, InterpolationMethod const inMethod, OverflowMethod const ofMethod=OM_BACKGROUND, RGBQUAD* const rplColor=0);
//@}

protected:
/** \addtogroup Protected */ //@{
	void  AddAveragingCont(RGBQUAD const &color, float const surf, float &rr, float &gg, float &bb, float &aa);
//@}
#endif //CXIMAGE_SUPPORT_INTERPOLATION
	
/** \addtogroup Painting */ //@{
#if CXIMAGE_SUPPORT_WINDOWS
public:
	HBITMAP MakeBitmap(HDC hdc = NULL, bool bTransparency = false);
	bool	CreateFromHBITMAP(HBITMAP hbmp, HPALETTE hpal=0);	//Windows resource
	int32_t	Draw(HDC hdc, int32_t x=0, int32_t y=0, int32_t cx = -1, int32_t cy = -1, RECT* pClipRect = 0, bool bSmooth = false, bool bFlipY = false);
	int32_t	DrawString(HDC hdc, int32_t x, int32_t y, const TCHAR* text, RGBQUAD color, const TCHAR* font, int32_t lSize=0, int32_t lWeight=400, uint8_t bItalic=0, uint8_t bUnderline=0, bool bSetAlpha=false);
#endif //CXIMAGE_SUPPORT_WINDOWS
//@}

	// file operations
#if CXIMAGE_SUPPORT_DECODE
/** \addtogroup Decode */ //@{
	bool Decode(CxFile * hFile, uint32_t imagetype);
	bool Decode(uint8_t * buffer, uint32_t size, uint32_t imagetype);
//@}
#endif //CXIMAGE_SUPPORT_DECODE

#if CXIMAGE_SUPPORT_ENCODE
protected:
/** \addtogroup Protected */ //@{
	bool EncodeSafeCheck(CxFile *hFile);
//@}

public:
/** \addtogroup Encode */ //@{
	bool Encode(CxFile * hFile, uint32_t imagetype);
	bool Encode(uint8_t * &buffer, int32_t &size, uint32_t imagetype);
//@}
#endif //CXIMAGE_SUPPORT_ENCODE

/** \addtogroup Attributes */ //@{
	//misc.
	bool IsValid() const;
//@}

#if CXIMAGE_SUPPORT_BASICTRANSFORMATIONS
/** \addtogroup BasicTransformations */ //@{
	bool GrayScale();
	bool Flip(bool bFlipSelection = false, bool bFlipAlpha = true);
	bool RotateLeft(CxImage* iDst = NULL);
	bool RotateRight(CxImage* iDst = NULL);
//@}
#endif //CXIMAGE_SUPPORT_BASICTRANSFORMATIONS

#if CXIMAGE_SUPPORT_TRANSFORMATION
/** \addtogroup Transformations */ //@{
	// image operations
	bool Rotate2(float angle, CxImage *iDst = NULL, InterpolationMethod inMethod=IM_BILINEAR,
                OverflowMethod ofMethod=OM_BACKGROUND, RGBQUAD *replColor=0,
                bool const optimizeRightAngles=true, bool const bKeepOriginalSize=false);
	bool Rotate180(CxImage* iDst = NULL);
	bool Resample(int32_t newx, int32_t newy, int32_t mode = 1, CxImage* iDst = NULL);
	bool DecreaseBpp(uint32_t nbit, bool errordiffusion, RGBQUAD* ppal = 0, uint32_t clrimportant = 0);

//@}
#endif //CXIMAGE_SUPPORT_TRANSFORMATION

//#if CXIMAGE_SUPPORT_DSP
/** \addtogroup DSP */ //@{
	bool Jitter(int32_t radius=2);
//@}

//#endif //CXIMAGE_SUPPORT_DSP
	static RGBQUAD RGBtoRGBQUAD(COLORREF cr);
protected:
/** \addtogroup Protected */ //@{
#if CXIMAGE_SUPPORT_ALPHA
/** \addtogroup Alpha */ //@{
	bool AlphaCreate();
	void AlphaDelete();
	void AlphaInvert();
	bool AlphaFlip();
	bool AlphaCopy(CxImage &from);
	bool AlphaSplit(CxImage *dest);
	bool AlphaSet(CxImage &from);
	void AlphaSet(const int32_t x,const int32_t y,const uint8_t level);
	uint8_t AlphaGet(const int32_t x,const int32_t y);
	bool AlphaIsValid();
	uint8_t* AlphaGetPointer(const int32_t x = 0,const int32_t y = 0);
//@}

protected:
/** \addtogroup Protected */ //@{
	uint8_t BlindAlphaGet(const int32_t x,const int32_t y);
//@}
#endif //CXIMAGE_SUPPORT_ALPHA
	void Startup(uint32_t imagetype = 0);
	void CopyInfo(const CxImage &src);
	void Ghost(const CxImage *src);
	void RGBtoBGR(uint8_t *buffer, int32_t length);
	void Bitfield2RGB(uint8_t *src, uint32_t redmask, uint32_t greenmask, uint32_t bluemask, uint8_t bpp);
	int16_t m_ntohs(const int16_t word);
	int32_t m_ntohl(const int32_t dword);
	void bihtoh(BITMAPINFOHEADER* bih);

	void*				pDib; //contains the header, the palette, the pixels
    BITMAPINFOHEADER    head; //standard header
	CXIMAGEINFO			info; //extended information
	uint8_t*			pSelection;	//selected region
	uint8_t*			pAlpha; //alpha channel
	CxImage**			ppLayers; //generic layers
	CxImage**			ppFrames;
//@}
};

////////////////////////////////////////////////////////////////////////////
#endif // !defined(__CXIMAGE_H)
