// xImaWnd.cpp : Windows functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

#include "ximaiter.h" 
#include "ximabmp.h"

////////////////////////////////////////////////////////////////////////////////
#if defined (_WIN32_WCE)

#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT 17
#endif

#ifndef PROOF_QUALITY
#define PROOF_QUALITY 2
#endif

struct DIBINFO : public BITMAPINFO
{
	RGBQUAD    arColors[255];    // Color table info - adds an extra 255 entries to palette
	operator LPBITMAPINFO()          { return (LPBITMAPINFO) this; }
	operator LPBITMAPINFOHEADER()    { return &bmiHeader;          }
	RGBQUAD* ColorTable()            { return bmiColors;           }
};

int32_t BytesPerLine(int32_t nWidth, int32_t nBitsPerPixel)
{
    return ( (nWidth * nBitsPerPixel + 31) & (~31) ) / 8;
}

int32_t NumColorEntries(int32_t nBitsPerPixel, int32_t nCompression, uint32_t biClrUsed)
{
	int32_t nColors = 0;
	switch (nBitsPerPixel)
	{
	case 1:
		nColors = 2;  break;
	case 2:
		nColors = 4;  break;   // winCE only
	case 4:
		nColors = 16; break;
	case 8:
		nColors =256; break;
	case 24:
		nColors = 0;  break;
	case 16:
	case 32:
		nColors = 3;  break; // I've found that PocketPCs need this regardless of BI_RGB or BI_BITFIELDS
	default:
		ASSERT(FALSE);
	}
	// If biClrUsed is provided, and it is a legal value, use it
	if (biClrUsed > 0 && biClrUsed <= (uint32_t)nColors)
		return biClrUsed;
	
	return nColors;
}

int32_t GetDIBits(
  HDC hdc,           // handle to DC
  HBITMAP hbmp,      // handle to bitmap
  uint32_t uStartScan,   // first scan line to set
  uint32_t cScanLines,   // number of scan lines to copy
  LPVOID lpvBits,    // array for bitmap bits
  LPBITMAPINFO lpbi, // bitmap data buffer
  uint32_t uUsage        // RGB or palette index
)
{
	uint32_t	iColorTableSize = 0;

	if (!hbmp)
		return 0;

	// Get dimensions of bitmap
	BITMAP bm;
	if (!::GetObject(hbmp, sizeof(bm),(LPVOID)&bm))
		return 0;

	//3. Creating new bitmap and receive pointer to it's bits.
	HBITMAP hTargetBitmap;
	void *pBuffer;
	
	//3.1 Initilize DIBINFO structure
	DIBINFO  dibInfo;
	dibInfo.bmiHeader.biBitCount = 24;
	dibInfo.bmiHeader.biClrImportant = 0;
	dibInfo.bmiHeader.biClrUsed = 0;
	dibInfo.bmiHeader.biCompression = 0;
	dibInfo.bmiHeader.biHeight = bm.bmHeight;
	dibInfo.bmiHeader.biPlanes = 1;
	dibInfo.bmiHeader.biSize = 40;
	dibInfo.bmiHeader.biSizeImage = bm.bmHeight*BytesPerLine(bm.bmWidth,24);
	dibInfo.bmiHeader.biWidth = bm.bmWidth;
	dibInfo.bmiHeader.biXPelsPerMeter = 3780;
	dibInfo.bmiHeader.biYPelsPerMeter = 3780;
	dibInfo.bmiColors[0].rgbBlue = 0;
	dibInfo.bmiColors[0].rgbGreen = 0;
	dibInfo.bmiColors[0].rgbRed = 0;
	dibInfo.bmiColors[0].rgbReserved = 0;

	//3.2 Create bitmap and receive pointer to points into pBuffer
	HDC hDC = ::GetDC(NULL);
	ASSERT(hDC);
	hTargetBitmap = CreateDIBSection(
		hDC,
		(const BITMAPINFO*)dibInfo,
		DIB_RGB_COLORS,
		(void**)&pBuffer,
		NULL,
		0);

	::ReleaseDC(NULL, hDC);

	//4. Copy source bitmap into the target bitmap.

	//4.1 Create 2 device contexts
	HDC memDc = CreateCompatibleDC(NULL);
	if (!memDc) {
		ASSERT(FALSE);
	}
	
	HDC targetDc = CreateCompatibleDC(NULL);
	if (!targetDc) {
		ASSERT(FALSE);
	}

	//4.2 Select source bitmap into one DC, target into another
	HBITMAP hOldBitmap1 = (HBITMAP)::SelectObject(memDc, hbmp);
	HBITMAP hOldBitmap2 = (HBITMAP)::SelectObject(targetDc, hTargetBitmap);

	//4.3 Copy source bitmap into the target one
	BitBlt(targetDc, 0, 0, bm.bmWidth, bm.bmHeight, memDc, 0, 0, SRCCOPY);

	//4.4 Restore device contexts
	::SelectObject(memDc, hOldBitmap1);
	::SelectObject(targetDc, hOldBitmap2);
	DeleteDC(memDc);
	DeleteDC(targetDc);

	//Here we can bitmap bits: pBuffer. Note:
	// 1. pBuffer contains 3 bytes per point
	// 2. Lines ane from the bottom to the top!
	// 3. Points in the line are from the left to the right
	// 4. Bytes in one point are BGR (blue, green, red) not RGB
	// 5. Don't delete pBuffer, it will be automatically deleted
	//    when delete hTargetBitmap
	lpvBits = pBuffer;

	DeleteObject(hbmp);
	//DeleteObject(hTargetBitmap);

	return 1;
}
#endif 

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_WINDOWS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/**
 * Transfer the image in a  bitmap handle
 * \param hdc: target device context (the screen, usually)
 * \param bTransparency : (optional) exports trancparency 
 * \return bitmap handle, or NULL if an error occurs.
 * \sa Draw2HBITMAP, MakeIcon
 * \author []; changes [brunom]
 */
HBITMAP CxImage::MakeBitmap(HDC hdc, bool bTransparency)
{
	if (!pDib)
		return NULL;

	// Create HBITMAP with Trancparency
	if( (pAlpha!=0) && bTransparency )
	{
		HDC hMemDC;
		if (hdc)
			hMemDC = hdc;
		else
			hMemDC = CreateCompatibleDC(NULL);

		BITMAPINFO bi;

		// Fill in the BITMAPINFOHEADER
		bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = GetWidth();
		bi.bmiHeader.biHeight = GetHeight();
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biBitCount = 32;
		bi.bmiHeader.biCompression = BI_RGB;
		bi.bmiHeader.biSizeImage = 4 * GetWidth() * GetHeight();
		bi.bmiHeader.biXPelsPerMeter = 0;
		bi.bmiHeader.biYPelsPerMeter = 0;
		bi.bmiHeader.biClrUsed = 0;
		bi.bmiHeader.biClrImportant = 0;

		COLORREF* pCrBits = NULL;
		HBITMAP hbmp = CreateDIBSection (
			hMemDC, &bi, DIB_RGB_COLORS, (void **)&pCrBits,
			NULL, NULL);

		if (!hdc)
			DeleteDC(hMemDC);

		DIBSECTION ds;
		if (::GetObject (hbmp, sizeof (DIBSECTION), &ds) == 0)
		{
			return 0;
		}

		// transfer Pixels from CxImage to Bitmap
		RGBQUAD* pBit = (RGBQUAD*) ds.dsBm.bmBits;
		int32_t lPx,lPy;
		for( lPy=0 ; lPy < bi.bmiHeader.biHeight ; ++lPy )
		{
			for( lPx=0 ; lPx < bi.bmiHeader.biWidth ; ++lPx )
			{
				RGBQUAD lPixel = GetPixelColor(lPx,lPy,true);
				*pBit = lPixel;
				pBit++;
			}
		}

		return hbmp;
	}

	// Create HBITMAP without Trancparency
	if (!hdc){
		// this call to CreateBitmap doesn't create a DIB <jaslet>
		// // Create a device-independent bitmap <CSC>
		//  return CreateBitmap(head.biWidth,head.biHeight,	1, head.biBitCount, GetBits());
		// use instead this code
		HDC hMemDC = CreateCompatibleDC(NULL);
		LPVOID pBit32;
		HBITMAP bmp = CreateDIBSection(hMemDC,(LPBITMAPINFO)pDib,DIB_RGB_COLORS, &pBit32, NULL, 0);
		if (pBit32) memcpy(pBit32, GetBits(), head.biSizeImage);
		DeleteDC(hMemDC);
		return bmp;
	}

	// this single line seems to work very well
	//HBITMAP bmp = CreateDIBitmap(hdc, (LPBITMAPINFOHEADER)pDib, CBM_INIT,
	//	GetBits(), (LPBITMAPINFO)pDib, DIB_RGB_COLORS);
	// this alternative works also with _WIN32_WCE
	LPVOID pBit32;
	HBITMAP bmp = CreateDIBSection(hdc, (LPBITMAPINFO)pDib, DIB_RGB_COLORS, &pBit32, NULL, 0);
	if (pBit32) memcpy(pBit32, GetBits(), head.biSizeImage);

	return bmp;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Bitmap resource constructor
 * \param hbmp : bitmap resource handle
 * \param hpal : (optional) palette, useful for 8bpp DC 
 * \param bTransparency : (optional) for 32bpp images only, imports trancparency 
 * \return true if everything is ok
 * \author []; changes [brunom]
 */
bool CxImage::CreateFromHBITMAP(HBITMAP hbmp, HPALETTE hpal)
{
	if (!Destroy())
		return false;

	if (hbmp) { 
        BITMAP bm;
		// get informations about the bitmap
        GetObject(hbmp, sizeof(BITMAP), (LPSTR) &bm);
		// create the image
        if (!Create(bm.bmWidth, bm.bmHeight, bm.bmBitsPixel, 0))
			return false;
		// create a device context for the bitmap
        HDC dc = ::GetDC(NULL);
		if (!dc)
			return false;

		if (hpal){
			SelectObject(dc,hpal); //the palette you should get from the user or have a stock one
			RealizePalette(dc);
		}

		// copy the pixels
        if (GetDIBits(dc, hbmp, 0, head.biHeight, info.pImage,
			(LPBITMAPINFO)pDib, DIB_RGB_COLORS) == 0){ //replace &head with pDib <Wil Stark>
            strcpy(info.szLastError,"GetDIBits failed");
			::ReleaseDC(NULL, dc);
			return false;
        }
        ::ReleaseDC(NULL, dc);
		return true;
    }
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Draws the image in the specified device context, with support for alpha channel, alpha palette, transparency, opacity.
 * \param hdc : destination device context
 * \param x,y : (optional) offset
 * \param cx,cy : (optional) size.
 *                 - If cx or cy are not specified (or less than 0), the normal width or height will be used
 *                 - If cx or cy are different than width or height, the image will be stretched
 *
 * \param pClipRect : limit the drawing operations inside a given rectangle in the output device context.
 * \param bSmooth : activates a bilinear filter that will enhance the appearence for zommed pictures.
 *                   Quite slow. Needs CXIMAGE_SUPPORT_INTERPOLATION.
 * \param bFlipY : draws a mirror image along the y-axis
 * \return true if everything is ok
 */
int32_t CxImage::Draw(HDC hdc, int32_t x, int32_t y, int32_t cx, int32_t cy, RECT* pClipRect, bool bSmooth, bool bFlipY)
{
	if((pDib==0)||(hdc==0)||(cx==0)||(cy==0)||(!info.bEnabled)) return 0;

	if (cx < 0) cx = head.biWidth;
	if (cy < 0) cy = head.biHeight;
	bool bTransparent = info.nBkgndIndex >= 0;
	bool bAlpha = pAlpha != 0;

	//required for MM_ANISOTROPIC, MM_HIENGLISH, and similar modes [Greg Peatfield]
	int32_t hdc_Restore = ::SaveDC(hdc);
	if (!hdc_Restore) 
		return 0;

#if !defined (_WIN32_WCE)
	RECT mainbox; // (experimental) 
	if (pClipRect){
		GetClipBox(hdc,&mainbox);
		HRGN rgn = CreateRectRgnIndirect(pClipRect);
		ExtSelectClipRgn(hdc,rgn,RGN_AND);
		DeleteObject(rgn);
	}
#endif

	//find the smallest area to paint
	RECT clipbox,paintbox;
	GetClipBox(hdc,&clipbox);

	paintbox.top = min(clipbox.bottom,max(clipbox.top,y));
	paintbox.left = min(clipbox.right,max(clipbox.left,x));
	paintbox.right = max(clipbox.left,min(clipbox.right,x+cx));
	paintbox.bottom = max(clipbox.top,min(clipbox.bottom,y+cy));

	int32_t destw = paintbox.right - paintbox.left;
	int32_t desth = paintbox.bottom - paintbox.top;

	if (!(bTransparent || bAlpha || info.bAlphaPaletteEnabled)){
		if (cx==head.biWidth && cy==head.biHeight){ //NORMAL
#if !defined (_WIN32_WCE)
			SetStretchBltMode(hdc,COLORONCOLOR);
#endif
			if (bFlipY){
				StretchDIBits(hdc, x, y+cy-1,
					cx, -cy, 0, 0, cx, cy,
					info.pImage,(BITMAPINFO*)pDib,DIB_RGB_COLORS,SRCCOPY);
			} else {
				SetDIBitsToDevice(hdc, x, y, cx, cy, 0, 0, 0, cy,
						info.pImage,(BITMAPINFO*)pDib,DIB_RGB_COLORS);
			}
		} else { //STRETCH
			//pixel informations
			RGBQUAD c={0,0,0,0};
			//Preparing Bitmap Info
			BITMAPINFO bmInfo;
			memset(&bmInfo.bmiHeader,0,sizeof(BITMAPINFOHEADER));
			bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
			bmInfo.bmiHeader.biWidth=destw;
			bmInfo.bmiHeader.biHeight=desth;
			bmInfo.bmiHeader.biPlanes=1;
			bmInfo.bmiHeader.biBitCount=24;
			uint8_t *pbase;	//points to the final dib
			uint8_t *pdst;		//current pixel from pbase
			uint8_t *ppix;		//current pixel from image
			//get the background
			HDC TmpDC=CreateCompatibleDC(hdc);
			HBITMAP TmpBmp=CreateDIBSection(hdc,&bmInfo,DIB_RGB_COLORS,(void**)&pbase,0,0);
			HGDIOBJ TmpObj=SelectObject(TmpDC,TmpBmp);

			if (pbase){
				int32_t xx,yy;
				int32_t sx,sy;
				float dx,dy;
				uint8_t *psrc;

				int32_t ew = ((((24 * destw) + 31) / 32) * 4);
				int32_t ymax = paintbox.bottom;
				int32_t xmin = paintbox.left;
				float fx=(float)head.biWidth/(float)cx;
				float fy=(float)head.biHeight/(float)cy;

				for(yy=0;yy<desth;yy++){
					dy = head.biHeight-(ymax-yy-y)*fy;
					sy = max(0L,(int32_t)floor(dy));
					psrc = info.pImage+sy*info.dwEffWidth;
					if (bFlipY){
						pdst = pbase+(desth-1-yy)*ew;
					} else {
						pdst = pbase+yy*ew;
					}
					for(xx=0;xx<destw;xx++){
						dx = (xx+xmin-x)*fx;
						sx = max(0L,(int32_t)floor(dx));
#if CXIMAGE_SUPPORT_INTERPOLATION
						if (bSmooth){
							if (fx > 1 && fy > 1) { 
								c = GetAreaColorInterpolated(dx - 0.5f, dy - 0.5f, fx, fy, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
							} else { 
								c = GetPixelColorInterpolated(dx - 0.5f, dy - 0.5f, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
							} 
						} else
#endif //CXIMAGE_SUPPORT_INTERPOLATION
						{
							if (head.biClrUsed){
								c=GetPaletteColor(GetPixelIndex(sx,sy));
							} else {
								ppix = psrc + sx*3;
								c.rgbBlue = *ppix++;
								c.rgbGreen= *ppix++;
								c.rgbRed  = *ppix;
							}
						}
						*pdst++=c.rgbBlue;
						*pdst++=c.rgbGreen;
						*pdst++=c.rgbRed;
					}
				}
			}
			//paint the image & cleanup
			SetDIBitsToDevice(hdc,paintbox.left,paintbox.top,destw,desth,0,0,0,desth,pbase,&bmInfo,0);
			DeleteObject(SelectObject(TmpDC,TmpObj));
			DeleteDC(TmpDC);
		}
	} else {	// draw image with transparent/alpha blending
	//////////////////////////////////////////////////////////////////
		//Alpha blend - Thanks to Florian Egel

		//pixel informations
		RGBQUAD c={0,0,0,0};
		RGBQUAD ct = GetTransColor();
		int32_t* pc = (int32_t*)&c;
		int32_t* pct= (int32_t*)&ct;
		int32_t cit = GetTransIndex();
		int32_t ci = 0;

		//Preparing Bitmap Info
		BITMAPINFO bmInfo;
		memset(&bmInfo.bmiHeader,0,sizeof(BITMAPINFOHEADER));
		bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth=destw;
		bmInfo.bmiHeader.biHeight=desth;
		bmInfo.bmiHeader.biPlanes=1;
		bmInfo.bmiHeader.biBitCount=24;

		uint8_t *pbase;	//points to the final dib
		uint8_t *pdst;		//current pixel from pbase
		uint8_t *ppix;		//current pixel from image

		//get the background
		HDC TmpDC=CreateCompatibleDC(hdc);
		HBITMAP TmpBmp=CreateDIBSection(hdc,&bmInfo,DIB_RGB_COLORS,(void**)&pbase,0,0);
		HGDIOBJ TmpObj=SelectObject(TmpDC,TmpBmp);
		BitBlt(TmpDC,0,0,destw,desth,hdc,paintbox.left,paintbox.top,SRCCOPY);

		if (pbase){
			int32_t xx,yy,alphaoffset,ix,iy;
			uint8_t a,a1,*psrc;
			int32_t ew = ((((24 * destw) + 31) / 32) * 4);
			int32_t ymax = paintbox.bottom;
			int32_t xmin = paintbox.left;

			if (cx!=head.biWidth || cy!=head.biHeight){
				//STRETCH
				float fx=(float)head.biWidth/(float)cx;
				float fy=(float)head.biHeight/(float)cy;
				float dx,dy;
				int32_t sx,sy;
				
				for(yy=0;yy<desth;yy++){
					dy = head.biHeight-(ymax-yy-y)*fy;
					sy = max(0L,(int32_t)floor(dy));

					alphaoffset = sy*head.biWidth;
					if (bFlipY){
						pdst = pbase+(desth-1-yy)*ew;
					} else {
						pdst = pbase + yy*ew;
					}
					psrc = info.pImage + sy*info.dwEffWidth;

					for(xx=0;xx<destw;xx++){
						dx = (xx+xmin-x)*fx;
						sx = max(0L,(int32_t)floor(dx));

						if (bAlpha) a=pAlpha[alphaoffset+sx]; else a=255;
						a =(uint8_t)((a*(1+info.nAlphaMax))>>8);

						if (head.biClrUsed){
							ci = GetPixelIndex(sx,sy);
#if CXIMAGE_SUPPORT_INTERPOLATION
							if (bSmooth){
								if (fx > 1 && fy > 1) { 
									c = GetAreaColorInterpolated(dx - 0.5f, dy - 0.5f, fx, fy, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
								} else { 
									c = GetPixelColorInterpolated(dx - 0.5f, dy - 0.5f, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
								} 
							} else
#endif //CXIMAGE_SUPPORT_INTERPOLATION
							{
								c = GetPaletteColor(GetPixelIndex(sx,sy));
							}
							if (info.bAlphaPaletteEnabled){
								a = (uint8_t)((a*(1+c.rgbReserved))>>8);
							}
						} else {
#if CXIMAGE_SUPPORT_INTERPOLATION
							if (bSmooth){
								if (fx > 1 && fy > 1) { 
									c = GetAreaColorInterpolated(dx - 0.5f, dy - 0.5f, fx, fy, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
								} else { 
									c = GetPixelColorInterpolated(dx - 0.5f, dy - 0.5f, CxImage::IM_BILINEAR, CxImage::OM_REPEAT); 
								} 
							} else
#endif //CXIMAGE_SUPPORT_INTERPOLATION
							{
								ppix = psrc + sx*3;
								c.rgbBlue = *ppix++;
								c.rgbGreen= *ppix++;
								c.rgbRed  = *ppix;
							}
						}
						//if (*pc!=*pct || !bTransparent){
						//if ((head.biClrUsed && ci!=cit) || ((!head.biClrUsed||bSmooth) && *pc!=*pct) || !bTransparent){
						if ((head.biClrUsed && ci!=cit) || (!head.biClrUsed && *pc!=*pct) || !bTransparent){
							// DJT, assume many pixels are fully transparent or opaque and thus avoid multiplication
							if (a == 0) {			// Transparent, retain dest 
								pdst+=3; 
							} else if (a == 255) {	// opaque, ignore dest 
								*pdst++= c.rgbBlue; 
								*pdst++= c.rgbGreen; 
								*pdst++= c.rgbRed; 
							} else {				// semi transparent 
								a1=(uint8_t)~a;
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbBlue)>>8); 
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbGreen)>>8); 
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbRed)>>8); 
							} 
						} else {
							pdst+=3;
						}
					}
				}
			} else {
				//NORMAL
				iy=head.biHeight-ymax+y;
				for(yy=0;yy<desth;yy++,iy++){
					alphaoffset=iy*head.biWidth;
					ix=xmin-x;
					if (bFlipY){
						pdst = pbase+(desth-1-yy)*ew;
					} else {
						pdst = pbase+yy*ew;
					}
					ppix=info.pImage+iy*info.dwEffWidth+ix*3;
					for(xx=0;xx<destw;xx++,ix++){

						if (bAlpha) a=pAlpha[alphaoffset+ix]; else a=255;
						a = (uint8_t)((a*(1+info.nAlphaMax))>>8);

						if (head.biClrUsed){
							ci = GetPixelIndex(ix,iy);
							c = GetPaletteColor((uint8_t)ci);
							if (info.bAlphaPaletteEnabled){
								a = (uint8_t)((a*(1+c.rgbReserved))>>8);
							}
						} else {
							c.rgbBlue = *ppix++;
							c.rgbGreen= *ppix++;
							c.rgbRed  = *ppix++;
						}

						//if (*pc!=*pct || !bTransparent){
						if ((head.biClrUsed && ci!=cit) || (!head.biClrUsed && *pc!=*pct) || !bTransparent){
							// DJT, assume many pixels are fully transparent or opaque and thus avoid multiplication
							if (a == 0) {			// Transparent, retain dest 
								pdst+=3; 
							} else if (a == 255) {	// opaque, ignore dest 
								*pdst++= c.rgbBlue; 
								*pdst++= c.rgbGreen; 
								*pdst++= c.rgbRed; 
							} else {				// semi transparent 
								a1=(uint8_t)~a;
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbBlue)>>8); 
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbGreen)>>8); 
								*pdst++=(uint8_t)((*pdst * a1 + a * c.rgbRed)>>8); 
							} 
						} else {
							pdst+=3;
						}
					}
				}
			}
		}
		//paint the image & cleanup
		SetDIBitsToDevice(hdc,paintbox.left,paintbox.top,destw,desth,0,0,0,desth,pbase,&bmInfo,0);
		DeleteObject(SelectObject(TmpDC,TmpObj));
		DeleteDC(TmpDC);
	}

#if !defined (_WIN32_WCE)
	if (pClipRect){  // (experimental)
		HRGN rgn = CreateRectRgnIndirect(&mainbox);
		ExtSelectClipRgn(hdc,rgn,RGN_OR);
		DeleteObject(rgn);
	}
#endif

	::RestoreDC(hdc,hdc_Restore);
	return 1;
}
////////////////////////////////////////////////////////////////////////////////
// For UNICODE support: char -> TCHAR
int32_t CxImage::DrawString(HDC hdc, int32_t x, int32_t y, const TCHAR* text, RGBQUAD color, const TCHAR* font, int32_t lSize, int32_t lWeight, uint8_t bItalic, uint8_t bUnderline, bool bSetAlpha)
//int32_t CxImage::DrawString(HDC hdc, int32_t x, int32_t y, const char* text, RGBQUAD color, const char* font, int32_t lSize, int32_t lWeight, uint8_t bItalic, uint8_t bUnderline, bool bSetAlpha)
{
	if (IsValid()){
		//get the background
		HDC pDC;
		if (hdc) pDC=hdc; else pDC = ::GetDC(0);
		if (pDC==NULL) return 0;
		HDC TmpDC=CreateCompatibleDC(pDC);
		if (hdc==NULL) ::ReleaseDC(0, pDC);
   		if (TmpDC==NULL) return 0;
		//choose the font
		HFONT m_Font;
		LOGFONT* m_pLF;
		m_pLF=(LOGFONT*)calloc(1,sizeof(LOGFONT));
		_tcsncpy(m_pLF->lfFaceName,font,31);	// For UNICODE support
		//strncpy(m_pLF->lfFaceName,font,31);
		m_pLF->lfHeight=lSize;
		m_pLF->lfWeight=lWeight;
		m_pLF->lfItalic=bItalic;
		m_pLF->lfUnderline=bUnderline;
		m_Font=CreateFontIndirect(m_pLF);
		//select the font in the dc
		HFONT pOldFont=NULL;
		if (m_Font)
			pOldFont = (HFONT)SelectObject(TmpDC,m_Font);
		else
			pOldFont = (HFONT)SelectObject(TmpDC,GetStockObject(DEFAULT_GUI_FONT));

		//Set text color
		SetTextColor(TmpDC,RGB(255,255,255));
		SetBkColor(TmpDC,RGB(0,0,0));
		//draw the text
		SetBkMode(TmpDC,OPAQUE);
		//Set text position;
		RECT pos = {0,0,0,0};
		//int32_t len = (int32_t)strlen(text);
		int32_t len = (int32_t)_tcslen(text);	// For UNICODE support
		::DrawText(TmpDC,text,len,&pos,DT_CALCRECT);
		pos.right+=pos.bottom; //for italics

		//Preparing Bitmap Info
		int32_t width=pos.right;
		int32_t height=pos.bottom;
		BITMAPINFO bmInfo;
		memset(&bmInfo.bmiHeader,0,sizeof(BITMAPINFOHEADER));
		bmInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth=width;
		bmInfo.bmiHeader.biHeight=height;
		bmInfo.bmiHeader.biPlanes=1;
		bmInfo.bmiHeader.biBitCount=24;
		uint8_t *pbase; //points to the final dib

		HBITMAP TmpBmp=CreateDIBSection(TmpDC,&bmInfo,DIB_RGB_COLORS,(void**)&pbase,0,0);
		HGDIOBJ TmpObj=SelectObject(TmpDC,TmpBmp);
		memset(pbase,0,height*((((24 * width) + 31) / 32) * 4));

		::DrawText(TmpDC,text,len,&pos,0);

		CxImage itext;
		itext.CreateFromHBITMAP(TmpBmp);

		y=head.biHeight-y-1;
		for (int32_t ix=0;ix<width;ix++){
			for (int32_t iy=0;iy<height;iy++){
				if (itext.GetPixelColor(ix,iy).rgbBlue) SetPixelColor(x+ix,y+iy,color,bSetAlpha);
			}
		}

		//cleanup
		if (pOldFont) SelectObject(TmpDC,pOldFont);
		DeleteObject(m_Font);
		free(m_pLF);
		DeleteObject(SelectObject(TmpDC,TmpObj));
		DeleteDC(TmpDC);
	}

	return 1;
}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_WINDOWS
////////////////////////////////////////////////////////////////////////////////
