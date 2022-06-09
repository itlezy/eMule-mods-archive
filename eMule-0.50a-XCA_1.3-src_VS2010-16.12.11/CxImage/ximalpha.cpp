// xImalpha.cpp : Alpha channel functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

#if CXIMAGE_SUPPORT_ALPHA

////////////////////////////////////////////////////////////////////////////////
/**
 * Checks if the image has a valid alpha channel.
 */
bool CxImage::AlphaIsValid()
{
	return pAlpha!=0;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * Allocates an empty (opaque) alpha channel.
 */
bool CxImage::AlphaCreate()
{
	if (pAlpha==NULL) {
		pAlpha = (uint8_t*)malloc(head.biWidth * head.biHeight);
		if (pAlpha) memset(pAlpha,255,head.biWidth * head.biHeight);
	}
	return (pAlpha!=0);
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::AlphaDelete()
{
	if (pAlpha) { free(pAlpha); pAlpha=0; }
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::AlphaInvert()
{
	if (pAlpha) {
		uint8_t *iSrc=pAlpha;
		int32_t n=head.biHeight*head.biWidth;
		for(int32_t i=0; i < n; i++){
			*iSrc=(uint8_t)~(*(iSrc));
			iSrc++;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Imports an existing alpa channel from another image with the same width and height.
 */
bool CxImage::AlphaCopy(CxImage &from)
{
	if (from.pAlpha == NULL || head.biWidth != from.head.biWidth || head.biHeight != from.head.biHeight) return false;
	if (pAlpha==NULL) pAlpha = (uint8_t*)malloc(head.biWidth * head.biHeight);
	if (pAlpha==NULL) return false;
	memcpy(pAlpha,from.pAlpha,head.biWidth * head.biHeight);
	info.nAlphaMax=from.info.nAlphaMax;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Creates the alpha channel from a gray scale image.
 */
bool CxImage::AlphaSet(CxImage &from)
{
	if (!from.IsGrayScale() || head.biWidth != from.head.biWidth || head.biHeight != from.head.biHeight) return false;
	if (pAlpha==NULL) pAlpha = (uint8_t*)malloc(head.biWidth * head.biHeight);
	uint8_t* src = from.info.pImage;
	uint8_t* dst = pAlpha;
	if (src==NULL || dst==NULL) return false;
	for (int32_t y=0; y<head.biHeight; y++){
		memcpy(dst,src,head.biWidth);
		dst += head.biWidth;
		src += from.info.dwEffWidth;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the alpha level for a single pixel 
 */
void CxImage::AlphaSet(const int32_t x,const int32_t y,const uint8_t level)
{
	if (pAlpha && IsInside(x,y)) pAlpha[x+y*head.biWidth]=level;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the alpha level for a single pixel 
 */
uint8_t CxImage::AlphaGet(const int32_t x,const int32_t y)
{
	if (pAlpha && IsInside(x,y)) return pAlpha[x+y*head.biWidth];
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns pointer to alpha data for pixel (x,y).
 *
 * \author ***bd*** 2.2004
 */
uint8_t* CxImage::AlphaGetPointer(const int32_t x,const int32_t y)
{
	if (pAlpha && IsInside(x,y)) return pAlpha+x+y*head.biWidth;
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Get alpha value without boundscheck (a bit faster). Pixel must be inside the image.
 *
 * \author ***bd*** 2.2004
 */
uint8_t CxImage::BlindAlphaGet(const int32_t x,const int32_t y)
{
#ifdef _DEBUG
	if (!IsInside(x,y) || (pAlpha==0))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return 0;
  #endif
#endif
	return pAlpha[x+y*head.biWidth];
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::AlphaFlip()
{
	if (!pAlpha) return false;

	uint8_t *buff = (uint8_t*)malloc(head.biWidth);
	if (!buff) return false;

	uint8_t *iSrc,*iDst;
	iSrc = pAlpha + (head.biHeight-1)*head.biWidth;
	iDst = pAlpha;
	for (int32_t i=0; i<(head.biHeight/2); ++i)
	{
		memcpy(buff, iSrc, head.biWidth);
		memcpy(iSrc, iDst, head.biWidth);
		memcpy(iDst, buff, head.biWidth);
		iSrc-=head.biWidth;
		iDst+=head.biWidth;
	}

	free(buff);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Exports the alpha channel in a 8bpp grayscale image. 
 */
bool CxImage::AlphaSplit(CxImage *dest)
{
	if (!pAlpha || !dest) return false;

	CxImage tmp(head.biWidth,head.biHeight,8);
	if (!tmp.IsValid()){
		strcpy(info.szLastError,tmp.GetLastError());
		return false;
	}

	uint8_t* src = pAlpha;
	uint8_t* dst = tmp.info.pImage;
	for (int32_t y=0; y<head.biHeight; y++){
		memcpy(dst,src,head.biWidth);
		dst += tmp.info.dwEffWidth;
		src += head.biWidth;
	}

	tmp.SetGrayPalette();
	dest->Transfer(tmp);

	return true;
}
#endif //CXIMAGE_SUPPORT_ALPHA
