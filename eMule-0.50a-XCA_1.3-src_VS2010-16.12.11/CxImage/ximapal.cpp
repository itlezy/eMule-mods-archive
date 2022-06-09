// xImaPal.cpp : Palette and Pixel functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

////////////////////////////////////////////////////////////////////////////////
/**
 * returns the palette dimension in byte
 */
uint32_t CxImage::GetPaletteSize()
{
	return (head.biClrUsed * sizeof(RGBQUAD));
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPaletteColor(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
{
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			iDst[ldx++] = (uint8_t) b;
			iDst[ldx++] = (uint8_t) g;
			iDst[ldx++] = (uint8_t) r;
			iDst[ldx] = (uint8_t) alpha;
			info.last_c_isvalid = false;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPaletteColor(uint8_t idx, RGBQUAD c)
{
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			iDst[ldx++] = (uint8_t) c.rgbBlue;
			iDst[ldx++] = (uint8_t) c.rgbGreen;
			iDst[ldx++] = (uint8_t) c.rgbRed;
			iDst[ldx] = (uint8_t) c.rgbReserved;
			info.last_c_isvalid = false;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * returns the pointer to the first palette index
 */
RGBQUAD* CxImage::GetPalette() const
{
	if ((pDib)&&(head.biClrUsed))
		return (RGBQUAD*)((uint8_t*)pDib + sizeof(BITMAPINFOHEADER));
	return NULL;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the color of the specified index.
 */
RGBQUAD CxImage::GetPaletteColor(uint8_t idx)
{
	RGBQUAD rgb = {0,0,0,0};
	if ((pDib)&&(head.biClrUsed)){
		uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
		if (idx<head.biClrUsed){
			int32_t ldx=idx*sizeof(RGBQUAD);
			rgb.rgbBlue = iDst[ldx++];
			rgb.rgbGreen=iDst[ldx++];
			rgb.rgbRed =iDst[ldx++];
			rgb.rgbReserved = iDst[ldx];
		}
	}
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the palette index of the specified pixel.
 */
uint8_t CxImage::GetPixelIndex(int32_t x,int32_t y)
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return 0;

	if ((x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight)) {
		if (info.nBkgndIndex >= 0)	return (uint8_t)info.nBkgndIndex;
		else return *info.pImage;
	}
	if (head.biBitCount==8){
		return info.pImage[y*info.dwEffWidth + x];
	} else {
		uint8_t pos;
		uint8_t iDst= info.pImage[y*info.dwEffWidth + (x*head.biBitCount >> 3)];
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uint8_t)(iDst >> pos);
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			iDst &= (0x01<<pos);
			return (uint8_t)(iDst >> pos);
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
uint8_t CxImage::BlindGetPixelIndex(const int32_t x,const int32_t y)
{
#ifdef _DEBUG
	if ((pDib==NULL) || (head.biClrUsed==0) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return 0;
  #endif
#endif

	if (head.biBitCount==8){
		return info.pImage[y*info.dwEffWidth + x];
	} else {
		uint8_t pos;
		uint8_t iDst= info.pImage[y*info.dwEffWidth + (x*head.biBitCount >> 3)];
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			iDst &= (0x0F<<pos);
			return (uint8_t)(iDst >> pos);
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			iDst &= (0x01<<pos);
			return (uint8_t)(iDst >> pos);
		}
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::GetPixelColor(int32_t x,int32_t y, bool bGetAlpha)
{
//	RGBQUAD rgb={0,0,0,0};
	RGBQUAD rgb=info.nBkgndColor; //<mpwolski>
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight)){
		if (info.nBkgndIndex >= 0){
			if (head.biBitCount<24) return GetPaletteColor((uint8_t)info.nBkgndIndex);
			else return info.nBkgndColor;
		} else if (pDib) return GetPixelColor(0,0);
		return rgb;
	}

	if (head.biClrUsed){
		rgb = GetPaletteColor(BlindGetPixelIndex(x,y));
	} else {
		uint8_t* iDst  = info.pImage + y*info.dwEffWidth + x*3;
		rgb.rgbBlue = *iDst++;
		rgb.rgbGreen= *iDst++;
		rgb.rgbRed  = *iDst;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (pAlpha && bGetAlpha) rgb.rgbReserved = BlindAlphaGet(x,y);
#else
	rgb.rgbReserved = 0;
#endif //CXIMAGE_SUPPORT_ALPHA
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * This is (a bit) faster version of GetPixelColor. 
 * It tests bounds only in debug mode (_DEBUG defined).
 * 
 * It is an error to request out-of-borders pixel with this method. 
 * In DEBUG mode an exception will be thrown, and data will be violated in non-DEBUG mode. 
 * \author ***bd*** 2.2004
 */
RGBQUAD CxImage::BlindGetPixelColor(const int32_t x,const int32_t y, bool bGetAlpha)
{
  RGBQUAD rgb;
#ifdef _DEBUG
	if ((pDib==NULL) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		{rgb.rgbReserved = 0; return rgb;}
  #endif
#endif

	if (head.biClrUsed){
		rgb = GetPaletteColor(BlindGetPixelIndex(x,y));
	} else {
		uint8_t* iDst  = info.pImage + y*info.dwEffWidth + x*3;
		rgb.rgbBlue = *iDst++;
		rgb.rgbGreen= *iDst++;
		rgb.rgbRed  = *iDst;
		rgb.rgbReserved = 0; //needed for images without alpha layer
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (pAlpha && bGetAlpha) rgb.rgbReserved = BlindAlphaGet(x,y);
#else
	rgb.rgbReserved = 0;
#endif //CXIMAGE_SUPPORT_ALPHA
	return rgb;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::BlindSetPixelIndex(int32_t x,int32_t y,uint8_t i)
{
#ifdef _DEBUG
	if ((pDib==NULL)||(head.biClrUsed==0)||
		(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return;
  #endif
#endif

	if (head.biBitCount==8){
		info.pImage[y*info.dwEffWidth + x]=i;
		return;
	} else {
		uint8_t pos;
		uint8_t* iDst= info.pImage + y*info.dwEffWidth + (x*head.biBitCount >> 3);
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((i & 0x0F)<<pos);
			return;
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((i & 0x01)<<pos);
			return;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelIndex(int32_t x,int32_t y,uint8_t i)
{
	if ((pDib==NULL)||(head.biClrUsed==0)||
		(x<0)||(y<0)||(x>=head.biWidth)||(y>=head.biHeight)) return ;

	if (head.biBitCount==8){
		info.pImage[y*info.dwEffWidth + x]=i;
		return;
	} else {
		uint8_t pos;
		uint8_t* iDst= info.pImage + y*info.dwEffWidth + (x*head.biBitCount >> 3);
		if (head.biBitCount==4){
			pos = (uint8_t)(4*(1-x%2));
			*iDst &= ~(0x0F<<pos);
			*iDst |= ((i & 0x0F)<<pos);
			return;
		} else if (head.biBitCount==1){
			pos = (uint8_t)(7-x%8);
			*iDst &= ~(0x01<<pos);
			*iDst |= ((i & 0x01)<<pos);
			return;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelColor(int32_t x,int32_t y,COLORREF cr)
{
	SetPixelColor(x,y,RGBtoRGBQUAD(cr));
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::BlindSetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha)
{
#ifdef _DEBUG
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return;
  #endif
#endif
	if (head.biClrUsed)
		BlindSetPixelIndex(x,y,GetNearestIndex(c));
	else {
		uint8_t* iDst = info.pImage + y*info.dwEffWidth + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (bSetAlpha) AlphaSet(x,y,c.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPixelColor(int32_t x,int32_t y,RGBQUAD c, bool bSetAlpha)
{
	if ((pDib==NULL)||(x<0)||(y<0)||
		(x>=head.biWidth)||(y>=head.biHeight)) return;
	if (head.biClrUsed)
		BlindSetPixelIndex(x,y,GetNearestIndex(c));
	else {
		uint8_t* iDst = info.pImage + y*info.dwEffWidth + x*3;
		*iDst++ = c.rgbBlue;
		*iDst++ = c.rgbGreen;
		*iDst   = c.rgbRed;
	}
#if CXIMAGE_SUPPORT_ALPHA
	if (bSetAlpha) AlphaSet(x,y,c.rgbReserved);
#endif //CXIMAGE_SUPPORT_ALPHA
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the best palette index that matches a specified color.
 */
uint8_t CxImage::GetNearestIndex(RGBQUAD c)
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return 0;

	// <RJ> check matching with the previous result
	if (info.last_c_isvalid && (*(int32_t*)&info.last_c == *(int32_t*)&c)) return info.last_c_index;
	info.last_c = c;
	info.last_c_isvalid = true;

	uint8_t* iDst = (uint8_t*)(pDib) + sizeof(BITMAPINFOHEADER);
	int32_t distance=200000;
	int32_t i,j = 0;
	int32_t k,l;
	int32_t m = (int32_t)(head.biClrImportant==0 ? head.biClrUsed : head.biClrImportant);
	for(i=0,l=0;i<m;i++,l+=sizeof(RGBQUAD)){
		k = (iDst[l]-c.rgbBlue)*(iDst[l]-c.rgbBlue)+
			(iDst[l+1]-c.rgbGreen)*(iDst[l+1]-c.rgbGreen)+
			(iDst[l+2]-c.rgbRed)*(iDst[l+2]-c.rgbRed);
//		k = abs(iDst[l]-c.rgbBlue)+abs(iDst[l+1]-c.rgbGreen)+abs(iDst[l+2]-c.rgbRed);
		if (k==0){
			j=i;
			break;
		}
		if (k<distance){
			distance=k;
			j=i;
		}
	}
	info.last_c_index = (uint8_t)j;
	return (uint8_t)j;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * swaps the blue and red components (for RGB images)
 * \param buffer : pointer to the pixels
 * \param length : number of bytes to swap. lenght may not exceed the scan line.
 */
void CxImage::RGBtoBGR(uint8_t *buffer, int32_t length)
{
	if (buffer && (head.biClrUsed==0)){
		uint8_t temp;
		length = min(length,(int32_t)info.dwEffWidth);
		length = min(length,(int32_t)(3*head.biWidth));
		for (int32_t i=0;i<length;i+=3){
			temp = buffer[i]; buffer[i] = buffer[i+2]; buffer[i+2] = temp;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
RGBQUAD CxImage::RGBtoRGBQUAD(COLORREF cr)
{
	RGBQUAD c;
	c.rgbRed = GetRValue(cr);	/* get R, G, and B out of uint32_t */
	c.rgbGreen = GetGValue(cr);
	c.rgbBlue = GetBValue(cr);
	c.rgbReserved=0;
	return c;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the color of the specified index.
 * \param i = palette index
 * \param r, g, b = output color channels
 */
bool CxImage::GetPaletteColor(uint8_t i, uint8_t* r, uint8_t* g, uint8_t* b)
{
	RGBQUAD* ppal=GetPalette();
	if (ppal) {
		*r = ppal[i].rgbRed;
		*g = ppal[i].rgbGreen;
		*b = ppal[i].rgbBlue; 
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPalette(rgb_color *rgb,uint32_t nColors)
{
	if ((!rgb)||(pDib==NULL)||(head.biClrUsed==0)) return;
	RGBQUAD* ppal=GetPalette();
	uint32_t m=min(nColors,head.biClrUsed);
	for (uint32_t i=0; i<m;i++){
		ppal[i].rgbRed=rgb[i].r;
		ppal[i].rgbGreen=rgb[i].g;
		ppal[i].rgbBlue=rgb[i].b;
	}
	info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::SetPalette(RGBQUAD* pPal,uint32_t nColors)
{
	if ((pPal==NULL)||(pDib==NULL)||(head.biClrUsed==0)) return;
	memcpy(GetPalette(),pPal,min(GetPaletteSize(),nColors*sizeof(RGBQUAD)));
	info.last_c_isvalid = false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets (or replaces) the palette to gray scale palette.
 * The function doesn't change the pixels; for standard
 * gray scale conversion use GrayScale().
 */
void CxImage::SetGrayPalette()
{
	if ((pDib==NULL)||(head.biClrUsed==0)) return;
	RGBQUAD* pal=GetPalette();
	for (uint32_t ni=0;ni<head.biClrUsed;ni++)
		pal[ni].rgbBlue=pal[ni].rgbGreen = pal[ni].rgbRed = (uint8_t)(ni*(255/(head.biClrUsed-1)));
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns true if the image has 256 colors and a linear grey scale palette.
 */
bool CxImage::IsGrayScale()
{
	RGBQUAD* ppal=GetPalette();
	if(!(pDib && ppal && head.biClrUsed)) return false;
	for(uint32_t i=0;i<head.biClrUsed;i++){
		if (ppal[i].rgbBlue!=i || ppal[i].rgbGreen!=i || ppal[i].rgbRed!=i) return false;
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * swap two indexes in the image and their colors in the palette
 */
void CxImage::SwapIndex(uint8_t idx1, uint8_t idx2)
{
	RGBQUAD* ppal=GetPalette();
	if(!(pDib && ppal)) return;
	//swap the colors
	RGBQUAD tempRGB=GetPaletteColor(idx1);
	SetPaletteColor(idx1,GetPaletteColor(idx2));
	SetPaletteColor(idx2,tempRGB);
	//swap the pixels
	uint8_t idx;
	for(int32_t y=0; y < head.biHeight; y++){
		for(int32_t x=0; x < head.biWidth; x++){
			idx=BlindGetPixelIndex(x,y);
			if (idx==idx1) BlindSetPixelIndex(x,y,idx2);
			if (idx==idx2) BlindSetPixelIndex(x,y,idx1);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \sa SetClrImportant
 */
uint32_t CxImage::GetClrImportant() const
{
	return head.biClrImportant;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * sets the maximum number of colors that some functions like
 * DecreaseBpp() or GetNearestIndex() will use on indexed images
 * \param ncolors should be less than 2^bpp,
 * or 0 if all the colors are important.
 */
void CxImage::SetClrImportant(uint32_t ncolors)
{
	if (ncolors==0 || ncolors>256) {
		head.biClrImportant = 0;
		return;
	}

	switch(head.biBitCount){
	case 1:
		head.biClrImportant = min(ncolors,2);
		break;
	case 4:
		head.biClrImportant = min(ncolors,16);
		break;
	case 8:
		head.biClrImportant = ncolors;
		break;
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns pointer to pixel. Currently implemented only for truecolor images.
 *  
 * \param  x,y - coordinates
 *
 * \return pointer to first byte of pixel data
 *
 * \author ***bd*** 2.2004
 */
void* CxImage::BlindGetPixelPointer(const int32_t x, const int32_t y)
{
#ifdef _DEBUG
	if ((pDib==NULL) || !IsInside(x,y))
  #if CXIMAGE_SUPPORT_EXCEPTION_HANDLING
		throw 0;
  #else
		return 0;
  #endif
#endif
  if (!IsIndexed())
    return info.pImage + y*info.dwEffWidth + x*3;
  else
    return 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets a palette with standard colors for 1, 4 and 8 bpp images.
 */
void CxImage::SetStdPalette()
{
	if (!pDib) return;
	switch (head.biBitCount){
	case 8:
		{
			const uint8_t pal256[1024] = {0,0,0,0,0,0,128,0,0,128,0,0,0,128,128,0,128,0,0,0,128,0,128,0,128,128,0,0,192,192,192,0,
			192,220,192,0,240,202,166,0,212,240,255,0,177,226,255,0,142,212,255,0,107,198,255,0,
			72,184,255,0,37,170,255,0,0,170,255,0,0,146,220,0,0,122,185,0,0,98,150,0,0,74,115,0,0,
			50,80,0,212,227,255,0,177,199,255,0,142,171,255,0,107,143,255,0,72,115,255,0,37,87,255,0,0,
			85,255,0,0,73,220,0,0,61,185,0,0,49,150,0,0,37,115,0,0,25,80,0,212,212,255,0,177,177,255,0,
			142,142,255,0,107,107,255,0,72,72,255,0,37,37,255,0,0,0,254,0,0,0,220,0,0,0,185,0,0,0,150,0,
			0,0,115,0,0,0,80,0,227,212,255,0,199,177,255,0,171,142,255,0,143,107,255,0,115,72,255,0,
			87,37,255,0,85,0,255,0,73,0,220,0,61,0,185,0,49,0,150,0,37,0,115,0,25,0,80,0,240,212,255,0,
			226,177,255,0,212,142,255,0,198,107,255,0,184,72,255,0,170,37,255,0,170,0,255,0,146,0,220,0,
			122,0,185,0,98,0,150,0,74,0,115,0,50,0,80,0,255,212,255,0,255,177,255,0,255,142,255,0,255,107,255,0,
			255,72,255,0,255,37,255,0,254,0,254,0,220,0,220,0,185,0,185,0,150,0,150,0,115,0,115,0,80,0,80,0,
			255,212,240,0,255,177,226,0,255,142,212,0,255,107,198,0,255,72,184,0,255,37,170,0,255,0,170,0,
			220,0,146,0,185,0,122,0,150,0,98,0,115,0,74,0,80,0,50,0,255,212,227,0,255,177,199,0,255,142,171,0,
			255,107,143,0,255,72,115,0,255,37,87,0,255,0,85,0,220,0,73,0,185,0,61,0,150,0,49,0,115,0,37,0,
			80,0,25,0,255,212,212,0,255,177,177,0,255,142,142,0,255,107,107,0,255,72,72,0,255,37,37,0,254,0,
			0,0,220,0,0,0,185,0,0,0,150,0,0,0,115,0,0,0,80,0,0,0,255,227,212,0,255,199,177,0,255,171,142,0,
			255,143,107,0,255,115,72,0,255,87,37,0,255,85,0,0,220,73,0,0,185,61,0,0,150,49,0,0,115,37,0,
			0,80,25,0,0,255,240,212,0,255,226,177,0,255,212,142,0,255,198,107,0,255,184,72,0,255,170,37,0,
			255,170,0,0,220,146,0,0,185,122,0,0,150,98,0,0,115,74,0,0,80,50,0,0,255,255,212,0,255,255,177,0,
			255,255,142,0,255,255,107,0,255,255,72,0,255,255,37,0,254,254,0,0,220,220,0,0,185,185,0,0,150,150,0,
			0,115,115,0,0,80,80,0,0,240,255,212,0,226,255,177,0,212,255,142,0,198,255,107,0,184,255,72,0,
			170,255,37,0,170,255,0,0,146,220,0,0,122,185,0,0,98,150,0,0,74,115,0,0,50,80,0,0,227,255,212,0,
			199,255,177,0,171,255,142,0,143,255,107,0,115,255,72,0,87,255,37,0,85,255,0,0,73,220,0,0,61,185,0,
			0,49,150,0,0,37,115,0,0,25,80,0,0,212,255,212,0,177,255,177,0,142,255,142,0,107,255,107,0,72,255,72,0,
			37,255,37,0,0,254,0,0,0,220,0,0,0,185,0,0,0,150,0,0,0,115,0,0,0,80,0,0,212,255,227,0,177,255,199,0,
			142,255,171,0,107,255,143,0,72,255,115,0,37,255,87,0,0,255,85,0,0,220,73,0,0,185,61,0,0,150,49,0,0,
			115,37,0,0,80,25,0,212,255,240,0,177,255,226,0,142,255,212,0,107,255,198,0,72,255,184,0,37,255,170,0,
			0,255,170,0,0,220,146,0,0,185,122,0,0,150,98,0,0,115,74,0,0,80,50,0,212,255,255,0,177,255,255,0,
			142,255,255,0,107,255,255,0,72,255,255,0,37,255,255,0,0,254,254,0,0,220,220,0,0,185,185,0,0,
			150,150,0,0,115,115,0,0,80,80,0,242,242,242,0,230,230,230,0,218,218,218,0,206,206,206,0,194,194,194,0,
			182,182,182,0,170,170,170,0,158,158,158,0,146,146,146,0,134,134,134,0,122,122,122,0,110,110,110,0,
			98,98,98,0,86,86,86,0,74,74,74,0,62,62,62,0,50,50,50,0,38,38,38,0,26,26,26,0,14,14,14,0,240,251,255,0,
			164,160,160,0,128,128,128,0,0,0,255,0,0,255,0,0,0,255,255,0,255,0,0,0,255,0,255,0,255,255,0,0,255,255,255,0};
			memcpy(GetPalette(),pal256,1024);
			break;
		}
	case 4:
		{
			const uint8_t pal16[64]={0,0,0,0,0,0,128,0,0,128,0,0,0,128,128,0,128,0,0,0,128,0,128,0,128,128,0,0,192,192,192,0,
								128,128,128,0,0,0,255,0,0,255,0,0,0,255,255,0,255,0,0,0,255,0,255,0,255,255,0,0,255,255,255,0};
			memcpy(GetPalette(),pal16,64);
			break;
		}
	case 1:
		{
			const uint8_t pal2[8]={0,0,0,0,255,255,255,0};
			memcpy(GetPalette(),pal2,8);
			break;
		}
	}
	info.last_c_isvalid = false;
	return;
}
////////////////////////////////////////////////////////////////////////////////
