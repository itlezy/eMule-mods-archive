// xImaTran.cpp : Transformation functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"
#include "ximath.h"

#if CXIMAGE_SUPPORT_BASICTRANSFORMATIONS
////////////////////////////////////////////////////////////////////////////////
bool CxImage::GrayScale()
{
	if (!pDib) return false;
	if (head.biBitCount<=8){
		RGBQUAD* ppal=GetPalette();
		int32_t gray;
		//converts the colors to gray, use the blue channel only
		for(uint32_t i=0;i<head.biClrUsed;i++){
			gray=(int32_t)RGB2GRAY(ppal[i].rgbRed,ppal[i].rgbGreen,ppal[i].rgbBlue);
			ppal[i].rgbBlue = (uint8_t)gray;
		}
		// preserve transparency
		if (info.nBkgndIndex >= 0) info.nBkgndIndex = ppal[info.nBkgndIndex].rgbBlue;
		//create a "real" 8 bit gray scale image
		if (head.biBitCount==8){
			uint8_t *img=info.pImage;
			for(uint32_t i=0;i<head.biSizeImage;i++) img[i]=ppal[img[i]].rgbBlue;
			SetGrayPalette();
		}
		//transform to 8 bit gray scale
		if (head.biBitCount==4 || head.biBitCount==1){
			CxImage ima;
			ima.CopyInfo(*this);
			if (!ima.Create(head.biWidth,head.biHeight,8,info.dwType)) return false;
			ima.SetGrayPalette();
#if CXIMAGE_SUPPORT_SELECTION
			ima.SelectionCopy(*this);
#endif //CXIMAGE_SUPPORT_SELECTION
#if CXIMAGE_SUPPORT_ALPHA
			ima.AlphaCopy(*this);
#endif //CXIMAGE_SUPPORT_ALPHA
			for (int32_t y=0;y<head.biHeight;y++){
				uint8_t *iDst = ima.GetBits(y);
				uint8_t *iSrc = GetBits(y);
				for (int32_t x=0;x<head.biWidth; x++){
					//iDst[x]=ppal[BlindGetPixelIndex(x,y)].rgbBlue;
					if (head.biBitCount==4){
						uint8_t pos = (uint8_t)(4*(1-x%2));
						iDst[x]= ppal[(uint8_t)((iSrc[x >> 1]&((uint8_t)0x0F<<pos)) >> pos)].rgbBlue;
					} else {
						uint8_t pos = (uint8_t)(7-x%8);
						iDst[x]= ppal[(uint8_t)((iSrc[x >> 3]&((uint8_t)0x01<<pos)) >> pos)].rgbBlue;
					}
				}
			}
			Transfer(ima);
		}
	} else { //from RGB to 8 bit gray scale
		uint8_t *iSrc=info.pImage;
		CxImage ima;
		ima.CopyInfo(*this);
		if (!ima.Create(head.biWidth,head.biHeight,8,info.dwType)) return false;
		ima.SetGrayPalette();
		if (GetTransIndex()>=0){
			RGBQUAD c = GetTransColor();
			ima.SetTransIndex((uint8_t)RGB2GRAY(c.rgbRed,c.rgbGreen,c.rgbBlue));
		}
#if CXIMAGE_SUPPORT_SELECTION
		ima.SelectionCopy(*this);
#endif //CXIMAGE_SUPPORT_SELECTION
#if CXIMAGE_SUPPORT_ALPHA
		ima.AlphaCopy(*this);
#endif //CXIMAGE_SUPPORT_ALPHA
		uint8_t *img=ima.GetBits();
		int32_t l8=ima.GetEffWidth();
		int32_t l=head.biWidth * 3;
		for(int32_t y=0; y < head.biHeight; y++) {
			for(int32_t x=0,x8=0; x < l; x+=3,x8++) {
				img[x8+y*l8]=(uint8_t)RGB2GRAY(*(iSrc+x+2),*(iSrc+x+1),*(iSrc+x+0));
			}
			iSrc+=info.dwEffWidth;
		}
		Transfer(ima);
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \sa Mirror
 * \author [qhbo]
 */
bool CxImage::Flip(bool bFlipSelection, bool bFlipAlpha)
{
	if (!pDib) return false;

	uint8_t *buff = (uint8_t*)malloc(info.dwEffWidth);
	if (!buff) return false;

	uint8_t *iSrc,*iDst;
	iSrc = GetBits(head.biHeight-1);
	iDst = GetBits(0);
	for (int32_t i=0; i<(head.biHeight/2); ++i)
	{
		memcpy(buff, iSrc, info.dwEffWidth);
		memcpy(iSrc, iDst, info.dwEffWidth);
		memcpy(iDst, buff, info.dwEffWidth);
		iSrc-=info.dwEffWidth;
		iDst+=info.dwEffWidth;
	}

	free(buff);

	if (bFlipSelection){
#if CXIMAGE_SUPPORT_SELECTION
		SelectionFlip();
#endif //CXIMAGE_SUPPORT_SELECTION
	}

	if (bFlipAlpha){
#if CXIMAGE_SUPPORT_ALPHA
		AlphaFlip();
#endif //CXIMAGE_SUPPORT_ALPHA
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////
#define RBLOCK 64

////////////////////////////////////////////////////////////////////////////////
bool CxImage::RotateLeft(CxImage* iDst)
{
	if (!pDib) return false;

	int32_t newWidth = GetHeight();
	int32_t newHeight = GetWidth();

	CxImage imgDest;
	imgDest.CopyInfo(*this);
	imgDest.Create(newWidth,newHeight,GetBpp(),GetType());
	imgDest.SetPalette(GetPalette());

#if CXIMAGE_SUPPORT_ALPHA
	if (AlphaIsValid()) imgDest.AlphaCreate();
#endif

#if CXIMAGE_SUPPORT_SELECTION
	if (SelectionIsValid()) imgDest.SelectionCreate();
#endif

	int32_t x,x2,y,dlineup;
	
	// Speedy rotate for BW images <Robert Abram>
	if (head.biBitCount == 1) {
	
		uint8_t *sbits, *dbits, *dbitsmax, bitpos, *nrow,*srcdisp;
		ldiv_t div_r;

		uint8_t *bsrc = GetBits(), *bdest = imgDest.GetBits();
		dbitsmax = bdest + imgDest.head.biSizeImage - 1;
		dlineup = 8 * imgDest.info.dwEffWidth - imgDest.head.biWidth;

		imgDest.Clear(0);
		for (y = 0; y < head.biHeight; y++) {
			// Figure out the Column we are going to be copying to
			div_r = ldiv(y + dlineup, (int32_t)8);
			// set bit pos of src column byte				
			bitpos = (uint8_t)(1 << div_r.rem);
			srcdisp = bsrc + y * info.dwEffWidth;
			for (x = 0; x < (int32_t)info.dwEffWidth; x++) {
				// Get Source Bits
				sbits = srcdisp + x;
				// Get destination column
				nrow = bdest + (x * 8) * imgDest.info.dwEffWidth + imgDest.info.dwEffWidth - 1 - div_r.quot;
				for (int32_t z = 0; z < 8; z++) {
				   // Get Destination Byte
					dbits = nrow + z * imgDest.info.dwEffWidth;
					if ((dbits < bdest) || (dbits > dbitsmax)) break;
					if (*sbits & (128 >> z)) *dbits |= bitpos;
				}
			}
		}//for y

#if CXIMAGE_SUPPORT_ALPHA
		if (AlphaIsValid()) {
			for (x = 0; x < newWidth; x++){
				x2=newWidth-x-1;
				for (y = 0; y < newHeight; y++){
					imgDest.AlphaSet(x,y,BlindAlphaGet(y, x2));
				}//for y
			}//for x
		}
#endif //CXIMAGE_SUPPORT_ALPHA

#if CXIMAGE_SUPPORT_SELECTION
		if (SelectionIsValid()) {
			imgDest.info.rSelectionBox.left = newWidth-info.rSelectionBox.top;
			imgDest.info.rSelectionBox.right = newWidth-info.rSelectionBox.bottom;
			imgDest.info.rSelectionBox.bottom = info.rSelectionBox.left;
			imgDest.info.rSelectionBox.top = info.rSelectionBox.right;
			for (x = 0; x < newWidth; x++){
				x2=newWidth-x-1;
				for (y = 0; y < newHeight; y++){
					imgDest.SelectionSet(x,y,BlindSelectionGet(y, x2));
				}//for y
			}//for x
		}
#endif //CXIMAGE_SUPPORT_SELECTION

	} else {
	//anything other than BW:
	//bd, 10. 2004: This optimized version of rotation rotates image by smaller blocks. It is quite
	//a bit faster than obvious algorithm, because it produces much less CPU cache misses.
	//This optimization can be tuned by changing block size (RBLOCK). 96 is good value for current
	//CPUs (tested on Athlon XP and Celeron D). Larger value (if CPU has enough cache) will increase
	//speed somehow, but once you drop out of CPU's cache, things will slow down drastically.
	//For older CPUs with less cache, lower value would yield better results.
		
		uint8_t *srcPtr, *dstPtr;                        //source and destionation for 24-bit version
		int32_t xs, ys;                                   //x-segment and y-segment
		for (xs = 0; xs < newWidth; xs+=RBLOCK) {       //for all image blocks of RBLOCK*RBLOCK pixels
			for (ys = 0; ys < newHeight; ys+=RBLOCK) {
				if (head.biBitCount==24) {
					//RGB24 optimized pixel access:
					for (x = xs; x < min(newWidth, xs+RBLOCK); x++){    //do rotation
						info.nProgress = (int32_t)(100*x/newWidth);
						x2=newWidth-x-1;
						dstPtr = (uint8_t*) imgDest.BlindGetPixelPointer(x,ys);
						srcPtr = (uint8_t*) BlindGetPixelPointer(ys, x2);
						for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
							//imgDest.SetPixelColor(x, y, GetPixelColor(y, x2));
							*(dstPtr) = *(srcPtr);
							*(dstPtr+1) = *(srcPtr+1);
							*(dstPtr+2) = *(srcPtr+2);
							srcPtr += 3;
							dstPtr += imgDest.info.dwEffWidth;
						}//for y
					}//for x
				} else {
					//anything else than 24bpp (and 1bpp): palette
					for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
						info.nProgress = (int32_t)(100*x/newWidth); //<Anatoly Ivasyuk>
						x2=newWidth-x-1;
						for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
							imgDest.SetPixelIndex(x, y, BlindGetPixelIndex(y, x2));
						}//for y
					}//for x
				}//if (version selection)
#if CXIMAGE_SUPPORT_ALPHA
				if (AlphaIsValid()) {
					for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
						x2=newWidth-x-1;
						for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
							imgDest.AlphaSet(x,y,BlindAlphaGet(y, x2));
						}//for y
					}//for x
				}//if (alpha channel)
#endif //CXIMAGE_SUPPORT_ALPHA

#if CXIMAGE_SUPPORT_SELECTION
				if (SelectionIsValid()) {
					imgDest.info.rSelectionBox.left = newWidth-info.rSelectionBox.top;
					imgDest.info.rSelectionBox.right = newWidth-info.rSelectionBox.bottom;
					imgDest.info.rSelectionBox.bottom = info.rSelectionBox.left;
					imgDest.info.rSelectionBox.top = info.rSelectionBox.right;
					for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
						x2=newWidth-x-1;
						for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
							imgDest.SelectionSet(x,y,BlindSelectionGet(y, x2));
						}//for y
					}//for x
				}//if (selection)
#endif //CXIMAGE_SUPPORT_SELECTION
			}//for ys
		}//for xs
	}//if

	//select the destination
	if (iDst) iDst->Transfer(imgDest);
	else Transfer(imgDest);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CxImage::RotateRight(CxImage* iDst)
{
	if (!pDib) return false;

	int32_t newWidth = GetHeight();
	int32_t newHeight = GetWidth();

	CxImage imgDest;
	imgDest.CopyInfo(*this);
	imgDest.Create(newWidth,newHeight,GetBpp(),GetType());
	imgDest.SetPalette(GetPalette());

#if CXIMAGE_SUPPORT_ALPHA
	if (AlphaIsValid()) imgDest.AlphaCreate();
#endif

#if CXIMAGE_SUPPORT_SELECTION
	if (SelectionIsValid()) imgDest.SelectionCreate();
#endif

	int32_t x,y,y2;
	// Speedy rotate for BW images <Robert Abram>
	if (head.biBitCount == 1) {
	
		uint8_t *sbits, *dbits, *dbitsmax, bitpos, *nrow,*srcdisp;
		ldiv_t div_r;

		uint8_t *bsrc = GetBits(), *bdest = imgDest.GetBits();
		dbitsmax = bdest + imgDest.head.biSizeImage - 1;

		imgDest.Clear(0);
		for (y = 0; y < head.biHeight; y++) {
			// Figure out the Column we are going to be copying to
			div_r = ldiv(y, (int32_t)8);
			// set bit pos of src column byte				
			bitpos = (uint8_t)(128 >> div_r.rem);
			srcdisp = bsrc + y * info.dwEffWidth;
			for (x = 0; x < (int32_t)info.dwEffWidth; x++) {
				// Get Source Bits
				sbits = srcdisp + x;
				// Get destination column
				nrow = bdest + (imgDest.head.biHeight-1-(x*8)) * imgDest.info.dwEffWidth + div_r.quot;
				for (int32_t z = 0; z < 8; z++) {
				   // Get Destination Byte
					dbits = nrow - z * imgDest.info.dwEffWidth;
					if ((dbits < bdest) || (dbits > dbitsmax)) break;
					if (*sbits & (128 >> z)) *dbits |= bitpos;
				}
			}
		}

#if CXIMAGE_SUPPORT_ALPHA
		if (AlphaIsValid()){
			for (y = 0; y < newHeight; y++){
				y2=newHeight-y-1;
				for (x = 0; x < newWidth; x++){
					imgDest.AlphaSet(x,y,BlindAlphaGet(y2, x));
				}
			}
		}
#endif //CXIMAGE_SUPPORT_ALPHA

#if CXIMAGE_SUPPORT_SELECTION
		if (SelectionIsValid()){
			imgDest.info.rSelectionBox.left = info.rSelectionBox.bottom;
			imgDest.info.rSelectionBox.right = info.rSelectionBox.top;
			imgDest.info.rSelectionBox.bottom = newHeight-info.rSelectionBox.right;
			imgDest.info.rSelectionBox.top = newHeight-info.rSelectionBox.left;
			for (y = 0; y < newHeight; y++){
				y2=newHeight-y-1;
				for (x = 0; x < newWidth; x++){
					imgDest.SelectionSet(x,y,BlindSelectionGet(y2, x));
				}
			}
		}
#endif //CXIMAGE_SUPPORT_SELECTION

	} else {
		//anything else but BW
		uint8_t *srcPtr, *dstPtr;                        //source and destionation for 24-bit version
		int32_t xs, ys;                                   //x-segment and y-segment
		for (xs = 0; xs < newWidth; xs+=RBLOCK) {
			for (ys = 0; ys < newHeight; ys+=RBLOCK) {
				if (head.biBitCount==24) {
					//RGB24 optimized pixel access:
					for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
						info.nProgress = (int32_t)(100*y/newHeight); //<Anatoly Ivasyuk>
						y2=newHeight-y-1;
						dstPtr = (uint8_t*) imgDest.BlindGetPixelPointer(xs,y);
						srcPtr = (uint8_t*) BlindGetPixelPointer(y2, xs);
						for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
							//imgDest.SetPixelColor(x, y, GetPixelColor(y2, x));
							*(dstPtr) = *(srcPtr);
							*(dstPtr+1) = *(srcPtr+1);
							*(dstPtr+2) = *(srcPtr+2);
							dstPtr += 3;
							srcPtr += info.dwEffWidth;
						}//for x
					}//for y
				} else {
					//anything else than BW & RGB24: palette
					for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
						info.nProgress = (int32_t)(100*y/newHeight); //<Anatoly Ivasyuk>
						y2=newHeight-y-1;
						for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
							imgDest.SetPixelIndex(x, y, BlindGetPixelIndex(y2, x));
						}//for x
					}//for y
				}//if
#if CXIMAGE_SUPPORT_ALPHA
				if (AlphaIsValid()){
					for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
						y2=newHeight-y-1;
						for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
							imgDest.AlphaSet(x,y,BlindAlphaGet(y2, x));
						}//for x
					}//for y
				}//if (has alpha)
#endif //CXIMAGE_SUPPORT_ALPHA

#if CXIMAGE_SUPPORT_SELECTION
				if (SelectionIsValid()){
					imgDest.info.rSelectionBox.left = info.rSelectionBox.bottom;
					imgDest.info.rSelectionBox.right = info.rSelectionBox.top;
					imgDest.info.rSelectionBox.bottom = newHeight-info.rSelectionBox.right;
					imgDest.info.rSelectionBox.top = newHeight-info.rSelectionBox.left;
					for (y = ys; y < min(newHeight, ys+RBLOCK); y++){
						y2=newHeight-y-1;
						for (x = xs; x < min(newWidth, xs+RBLOCK); x++){
							imgDest.SelectionSet(x,y,BlindSelectionGet(y2, x));
						}//for x
					}//for y
				}//if (has alpha)
#endif //CXIMAGE_SUPPORT_SELECTION
			}//for ys
		}//for xs
	}//if

	//select the destination
	if (iDst) iDst->Transfer(imgDest);
	else Transfer(imgDest);
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Rotates image around it's center.
 * Method can use interpolation with paletted images, but does not change pallete, so results vary.
 * (If you have only four colours in a palette, there's not much room for interpolation.)
 * 
 * \param  angle - angle in degrees (positive values rotate clockwise)
 * \param  *iDst - destination image (if null, this image is changed)
 * \param  inMethod - interpolation method used
 *              (IM_NEAREST_NEIGHBOUR produces aliasing (fast), IM_BILINEAR softens picture a bit (slower)
 *               IM_SHARPBICUBIC is slower and produces some halos...)
 * \param  ofMethod - overflow method (how to choose colour of pixels that have no source)
 * \param  replColor - replacement colour to use (OM_COLOR, OM_BACKGROUND with no background colour...)
 * \param  optimizeRightAngles - call faster methods for 90, 180, and 270 degree rotations. Faster methods
 *                         are called for angles, where error (in location of corner pixels) is less
 *                         than 0.25 pixels.
 * \param  bKeepOriginalSize - rotates the image without resizing.
 *
 * \author ***bd*** 2.2004
 */
bool CxImage::Rotate2(float angle, 
                       CxImage *iDst, 
                       InterpolationMethod inMethod, 
                       OverflowMethod ofMethod, 
                       RGBQUAD *replColor,
                       bool const optimizeRightAngles,
					   bool const bKeepOriginalSize)
{
	if (!pDib) return false;					//no dib no go

	if (fmod(angle,180.0f)==0.0f && fmod(angle,360.0f)!=0.0f)
		return Rotate180(iDst);

	double ang = -angle*acos(0.0f)/90.0f;		//convert angle to radians and invert (positive angle performs clockwise rotation)
	float cos_angle = (float) cos(ang);			//these two are needed later (to rotate)
	float sin_angle = (float) sin(ang);
	
	//Calculate the size of the new bitmap (rotate corners of image)
	CxPoint2 p[4];								//original corners of the image
	p[0]=CxPoint2(-0.5f,-0.5f);
	p[1]=CxPoint2(GetWidth()-0.5f,-0.5f);
	p[2]=CxPoint2(-0.5f,GetHeight()-0.5f);
	p[3]=CxPoint2(GetWidth()-0.5f,GetHeight()-0.5f);
	CxPoint2 newp[4];								//rotated positions of corners
	//(rotate corners)
	if (bKeepOriginalSize){
		for (int32_t i=0; i<4; i++) {
			newp[i].x = p[i].x;
			newp[i].y = p[i].y;
		}//for
	} else {
		for (int32_t i=0; i<4; i++) {
			newp[i].x = (p[i].x*cos_angle - p[i].y*sin_angle);
			newp[i].y = (p[i].x*sin_angle + p[i].y*cos_angle);
		}//for i
		
		if (optimizeRightAngles) { 
			//For rotations of 90, -90 or 180 or 0 degrees, call faster routines
			if (newp[3].Distance(CxPoint2(GetHeight()-0.5f, 0.5f-GetWidth())) < 0.25) 
				//rotation right for circa 90 degrees (diagonal pixels less than 0.25 pixel away from 90 degree rotation destination)
				return RotateRight(iDst);
			if (newp[3].Distance(CxPoint2(0.5f-GetHeight(), -0.5f+GetWidth())) < 0.25) 
				//rotation left for ~90 degrees
				return RotateLeft(iDst);
			if (newp[3].Distance(CxPoint2(0.5f-GetWidth(), 0.5f-GetHeight())) < 0.25) 
				//rotation left for ~180 degrees
				return Rotate180(iDst);
			if (newp[3].Distance(p[3]) < 0.25) {
				//rotation not significant
				if (iDst) iDst->Copy(*this);		//copy image to iDst, if required
				return true;						//and we're done
			}//if
		}//if
	}//if

	//(read new dimensions from location of corners)
	float minx = (float) min(min(newp[0].x,newp[1].x),min(newp[2].x,newp[3].x));
	float miny = (float) min(min(newp[0].y,newp[1].y),min(newp[2].y,newp[3].y));
	float maxx = (float) max(max(newp[0].x,newp[1].x),max(newp[2].x,newp[3].x));
	float maxy = (float) max(max(newp[0].y,newp[1].y),max(newp[2].y,newp[3].y));
	int32_t newWidth = (int32_t) floor(maxx-minx+0.5f);
	int32_t newHeight= (int32_t) floor(maxy-miny+0.5f);
	float ssx=((maxx+minx)- ((float) newWidth-1))/2.0f;   //start for x
	float ssy=((maxy+miny)- ((float) newHeight-1))/2.0f;  //start for y

	float newxcenteroffset = 0.5f * newWidth;
	float newycenteroffset = 0.5f * newHeight;
	if (bKeepOriginalSize){
		ssx -= 0.5f * GetWidth();
		ssy -= 0.5f * GetHeight();
	}

	//create destination image
	CxImage imgDest;
	imgDest.CopyInfo(*this);
	imgDest.Create(newWidth,newHeight,GetBpp(),GetType());
	imgDest.SetPalette(GetPalette());
#if CXIMAGE_SUPPORT_ALPHA
	if(AlphaIsValid()) imgDest.AlphaCreate(); //MTA: Fix for rotation problem when the image has an alpha channel
#endif //CXIMAGE_SUPPORT_ALPHA
	
	RGBQUAD rgb;			//pixel colour
	RGBQUAD rc;
	if (replColor!=0) 
		rc=*replColor; 
	else {
		rc.rgbRed=255; rc.rgbGreen=255; rc.rgbBlue=255; rc.rgbReserved=0;
	}//if
	float x,y;              //destination location (float, with proper offset)
	float origx, origy;     //origin location
	int32_t destx, desty;       //destination location
	
	y=ssy;                  //initialize y
	if (!IsIndexed()){ //RGB24
		//optimized RGB24 implementation (direct write to destination):
		uint8_t *pxptr;
#if CXIMAGE_SUPPORT_ALPHA
		uint8_t *pxptra=0;
#endif //CXIMAGE_SUPPORT_ALPHA
		for (desty=0; desty<newHeight; desty++) {
			info.nProgress = (int32_t)(100*desty/newHeight);
			if (info.nEscape) break;
			//initialize x
			x=ssx;
			//calculate pointer to first byte in row
			pxptr=(uint8_t *)imgDest.BlindGetPixelPointer(0, desty);
#if CXIMAGE_SUPPORT_ALPHA
			//calculate pointer to first byte in row
			if (AlphaIsValid()) pxptra=imgDest.AlphaGetPointer(0, desty);
#endif //CXIMAGE_SUPPORT_ALPHA
			for (destx=0; destx<newWidth; destx++) {
				//get source pixel coordinate for current destination point
				//origx = (cos_angle*(x-head.biWidth/2)+sin_angle*(y-head.biHeight/2))+newWidth/2;
				//origy = (cos_angle*(y-head.biHeight/2)-sin_angle*(x-head.biWidth/2))+newHeight/2;
				origx = cos_angle*x+sin_angle*y;
				origy = cos_angle*y-sin_angle*x;
				if (bKeepOriginalSize){
					origx += newxcenteroffset;
					origy += newycenteroffset;
				}
				rgb = GetPixelColorInterpolated(origx, origy, inMethod, ofMethod, &rc);   //get interpolated colour value
				//copy alpha and colour value to destination
#if CXIMAGE_SUPPORT_ALPHA
				if (pxptra) *pxptra++ = rgb.rgbReserved;
#endif //CXIMAGE_SUPPORT_ALPHA
				*pxptr++ = rgb.rgbBlue;
				*pxptr++ = rgb.rgbGreen;
				*pxptr++ = rgb.rgbRed;
				x++;
			}//for destx
			y++;
		}//for desty
	} else { 
		//non-optimized implementation for paletted images
		for (desty=0; desty<newHeight; desty++) {
			info.nProgress = (int32_t)(100*desty/newHeight);
			if (info.nEscape) break;
			x=ssx;
			for (destx=0; destx<newWidth; destx++) {
				//get source pixel coordinate for current destination point
				origx=(cos_angle*x+sin_angle*y);
				origy=(cos_angle*y-sin_angle*x);
				if (bKeepOriginalSize){
					origx += newxcenteroffset;
					origy += newycenteroffset;
				}
				rgb = GetPixelColorInterpolated(origx, origy, inMethod, ofMethod, &rc);
				//***!*** SetPixelColor is slow for palleted images
#if CXIMAGE_SUPPORT_ALPHA
				if (AlphaIsValid()) 
					imgDest.SetPixelColor(destx,desty,rgb,true);
				else 
#endif //CXIMAGE_SUPPORT_ALPHA     
					imgDest.SetPixelColor(destx,desty,rgb,false);
				x++;
			}//for destx
			y++;
		}//for desty
	}
	//select the destination
	
	if (iDst) iDst->Transfer(imgDest);
	else Transfer(imgDest);
	
	return true;
}
////////////////////////////////////////////////////////////////////////////////
bool CxImage::Rotate180(CxImage* iDst)
{
	if (!pDib) return false;

	int32_t wid = GetWidth();
	int32_t ht = GetHeight();

	CxImage imgDest;
	imgDest.CopyInfo(*this);
	imgDest.Create(wid,ht,GetBpp(),GetType());
	imgDest.SetPalette(GetPalette());

#if CXIMAGE_SUPPORT_ALPHA
	if (AlphaIsValid())	imgDest.AlphaCreate();
#endif //CXIMAGE_SUPPORT_ALPHA

	int32_t x,y,y2;
	for (y = 0; y < ht; y++){
		info.nProgress = (int32_t)(100*y/ht); //<Anatoly Ivasyuk>
		y2=ht-y-1;
		for (x = 0; x < wid; x++){
			if(head.biClrUsed==0)//RGB
				imgDest.SetPixelColor(wid-x-1, y2, BlindGetPixelColor(x, y));
			else  //PALETTE
				imgDest.SetPixelIndex(wid-x-1, y2, BlindGetPixelIndex(x, y));

#if CXIMAGE_SUPPORT_ALPHA
			if (AlphaIsValid())	imgDest.AlphaSet(wid-x-1, y2,BlindAlphaGet(x, y));
#endif //CXIMAGE_SUPPORT_ALPHA

		}
	}

	//select the destination
	if (iDst) iDst->Transfer(imgDest);
	else Transfer(imgDest);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Resizes the image. mode can be 0 for slow (bilinear) method ,
 * 1 for fast (nearest pixel) method, or 2 for accurate (bicubic spline interpolation) method.
 * The function is faster with 24 and 1 bpp images, slow for 4 bpp images and slowest for 8 bpp images.
 */
bool CxImage::Resample(int32_t newx, int32_t newy, int32_t mode, CxImage* iDst)
{
	if (newx==0 || newy==0) return false;

	if (head.biWidth==newx && head.biHeight==newy){
		if (iDst) iDst->Copy(*this);
		return true;
	}

	float xScale, yScale, fX, fY;
	xScale = (float)head.biWidth  / (float)newx;
	yScale = (float)head.biHeight / (float)newy;

	CxImage newImage;
	newImage.CopyInfo(*this);
	newImage.Create(newx,newy,head.biBitCount,GetType());
	newImage.SetPalette(GetPalette());
	if (!newImage.IsValid()){
		strcpy(info.szLastError,newImage.GetLastError());
		return false;
	}
/*
	switch (mode) {
	case 1: // nearest pixel
	{ 
		for(int32_t y=0; y<newy; y++){
			info.nProgress = (int32_t)(100*y/newy);
			if (info.nEscape) break;
			fY = y * yScale;
			for(int32_t x=0; x<newx; x++){
				fX = x * xScale;
				newImage.SetPixelColor(x,y,GetPixelColor((int32_t)fX,(int32_t)fY));
			}
		}
		break;
	}
	case 2: // bicubic interpolation by Blake L. Carlson <blake-carlson(at)uiowa(dot)edu
	{
		float f_x, f_y, a, b, rr, gg, bb, r1, r2;
		int32_t   i_x, i_y, xx, yy;
		RGBQUAD rgb;
		uint8_t* iDst;
		for(int32_t y=0; y<newy; y++){
			info.nProgress = (int32_t)(100*y/newy);
			if (info.nEscape) break;
			f_y = (float) y * yScale - 0.5f;
			i_y = (int32_t) floor(f_y);
			a   = f_y - (float)floor(f_y);
			for(int32_t x=0; x<newx; x++){
				f_x = (float) x * xScale - 0.5f;
				i_x = (int32_t) floor(f_x);
				b   = f_x - (float)floor(f_x);

				rr = gg = bb = 0.0f;
				for(int32_t m=-1; m<3; m++) {
					r1 = KernelBSpline((float) m - a);
					yy = i_y+m;
					if (yy<0) yy=0;
					if (yy>=head.biHeight) yy = head.biHeight-1;
					for(int32_t n=-1; n<3; n++) {
						r2 = r1 * KernelBSpline(b - (float)n);
						xx = i_x+n;
						if (xx<0) xx=0;
						if (xx>=head.biWidth) xx=head.biWidth-1;

						if (head.biClrUsed){
							rgb = GetPixelColor(xx,yy);
						} else {
							iDst  = info.pImage + yy*info.dwEffWidth + xx*3;
							rgb.rgbBlue = *iDst++;
							rgb.rgbGreen= *iDst++;
							rgb.rgbRed  = *iDst;
						}

						rr += rgb.rgbRed * r2;
						gg += rgb.rgbGreen * r2;
						bb += rgb.rgbBlue * r2;
					}
				}

				if (head.biClrUsed)
					newImage.SetPixelColor(x,y,RGB(rr,gg,bb));
				else {
					iDst = newImage.info.pImage + y*newImage.info.dwEffWidth + x*3;
					*iDst++ = (uint8_t)bb;
					*iDst++ = (uint8_t)gg;
					*iDst   = (uint8_t)rr;
				}

			}
		}
		break;
	}
	default: // bilinear interpolation*/
		if (!(head.biWidth>newx && head.biHeight>newy && head.biBitCount==24)) {
			// (c) 1999 Steve McMahon (steve@dogma.demon.co.uk)
			int32_t ifX, ifY, ifX1, ifY1, xmax, ymax;
			float ir1, ir2, ig1, ig2, ib1, ib2, dx, dy;
			uint8_t r,g,b;
			RGBQUAD rgb1, rgb2, rgb3, rgb4;
			xmax = head.biWidth-1;
			ymax = head.biHeight-1;
			for(int32_t y=0; y<newy; y++){
				info.nProgress = (int32_t)(100*y/newy);
				if (info.nEscape) break;
				fY = y * yScale;
				ifY = (int32_t)fY;
				ifY1 = min(ymax, ifY+1);
				dy = fY - ifY;
				for(int32_t x=0; x<newx; x++){
					fX = x * xScale;
					ifX = (int32_t)fX;
					ifX1 = min(xmax, ifX+1);
					dx = fX - ifX;
					// Interpolate using the four nearest pixels in the source
					if (head.biClrUsed){
						rgb1=GetPaletteColor(GetPixelIndex(ifX,ifY));
						rgb2=GetPaletteColor(GetPixelIndex(ifX1,ifY));
						rgb3=GetPaletteColor(GetPixelIndex(ifX,ifY1));
						rgb4=GetPaletteColor(GetPixelIndex(ifX1,ifY1));
					}
					else {
						uint8_t* iDst;
						iDst = info.pImage + ifY*info.dwEffWidth + ifX*3;
						rgb1.rgbBlue = *iDst++;	rgb1.rgbGreen= *iDst++;	rgb1.rgbRed =*iDst;
						iDst = info.pImage + ifY*info.dwEffWidth + ifX1*3;
						rgb2.rgbBlue = *iDst++;	rgb2.rgbGreen= *iDst++;	rgb2.rgbRed =*iDst;
						iDst = info.pImage + ifY1*info.dwEffWidth + ifX*3;
						rgb3.rgbBlue = *iDst++;	rgb3.rgbGreen= *iDst++;	rgb3.rgbRed =*iDst;
						iDst = info.pImage + ifY1*info.dwEffWidth + ifX1*3;
						rgb4.rgbBlue = *iDst++;	rgb4.rgbGreen= *iDst++;	rgb4.rgbRed =*iDst;
					}
					// Interplate in x direction:
					ir1 = rgb1.rgbRed   + (rgb3.rgbRed   - rgb1.rgbRed)   * dy;
					ig1 = rgb1.rgbGreen + (rgb3.rgbGreen - rgb1.rgbGreen) * dy;
					ib1 = rgb1.rgbBlue  + (rgb3.rgbBlue  - rgb1.rgbBlue)  * dy;
					ir2 = rgb2.rgbRed   + (rgb4.rgbRed   - rgb2.rgbRed)   * dy;
					ig2 = rgb2.rgbGreen + (rgb4.rgbGreen - rgb2.rgbGreen) * dy;
					ib2 = rgb2.rgbBlue  + (rgb4.rgbBlue  - rgb2.rgbBlue)  * dy;
					// Interpolate in y:
					r = (uint8_t)(ir1 + (ir2-ir1) * dx);
					g = (uint8_t)(ig1 + (ig2-ig1) * dx);
					b = (uint8_t)(ib1 + (ib2-ib1) * dx);
					// Set output
					newImage.SetPixelColor(x,y,RGB(r,g,b));
				}
			} 
		} else {
			//high resolution shrink, thanks to Henrik Stellmann <henrik.stellmann@volleynet.de>
			const int32_t ACCURACY = 1000;
			int32_t i,j; // index for faValue
			int32_t x,y; // coordinates in  source image
			uint8_t* pSource;
			uint8_t* pDest = newImage.info.pImage;
			int32_t* naAccu  = new int32_t[3 * newx + 3];
			int32_t* naCarry = new int32_t[3 * newx + 3];
			int32_t* naTemp;
			int32_t  nWeightX,nWeightY;
			float fEndX;
			int32_t nScale = (int32_t)(ACCURACY * xScale * yScale);

			memset(naAccu,  0, sizeof(int32_t) * 3 * newx);
			memset(naCarry, 0, sizeof(int32_t) * 3 * newx);

			int32_t u, v = 0; // coordinates in dest image
			float fEndY = yScale - 1.0f;
			for (y = 0; y < head.biHeight; y++){
				info.nProgress = (int32_t)(100*y/head.biHeight); //<Anatoly Ivasyuk>
				if (info.nEscape) break;
				pSource = info.pImage + y * info.dwEffWidth;
				u = i = 0;
				fEndX = xScale - 1.0f;
				if ((float)y < fEndY) {       // complete source row goes into dest row
					for (x = 0; x < head.biWidth; x++){
						if ((float)x < fEndX){       // complete source pixel goes into dest pixel
							for (j = 0; j < 3; j++)	naAccu[i + j] += (*pSource++) * ACCURACY;
						} else {       // source pixel is splitted for 2 dest pixels
							nWeightX = (int32_t)(((float)x - fEndX) * ACCURACY);
							for (j = 0; j < 3; j++){
								naAccu[i] += (ACCURACY - nWeightX) * (*pSource);
								naAccu[3 + i++] += nWeightX * (*pSource++);
							}
							fEndX += xScale;
							u++;
						}
					}
				} else {       // source row is splitted for 2 dest rows       
					nWeightY = (int32_t)(((float)y - fEndY) * ACCURACY);
					for (x = 0; x < head.biWidth; x++){
						if ((float)x < fEndX){       // complete source pixel goes into 2 pixel
							for (j = 0; j < 3; j++){
								naAccu[i + j] += ((ACCURACY - nWeightY) * (*pSource));
								naCarry[i + j] += nWeightY * (*pSource++);
							}
						} else {       // source pixel is splitted for 4 dest pixels
							nWeightX = (int32_t)(((float)x - fEndX) * ACCURACY);
							for (j = 0; j < 3; j++) {
								naAccu[i] += ((ACCURACY - nWeightY) * (ACCURACY - nWeightX)) * (*pSource) / ACCURACY;
								*pDest++ = (uint8_t)(naAccu[i] / nScale);
								naCarry[i] += (nWeightY * (ACCURACY - nWeightX) * (*pSource)) / ACCURACY;
								naAccu[i + 3] += ((ACCURACY - nWeightY) * nWeightX * (*pSource)) / ACCURACY;
								naCarry[i + 3] = (nWeightY * nWeightX * (*pSource)) / ACCURACY;
								i++;
								pSource++;
							}
							fEndX += xScale;
							u++;
						}
					}
					if (u < newx){ // possibly not completed due to rounding errors
						for (j = 0; j < 3; j++) *pDest++ = (uint8_t)(naAccu[i++] / nScale);
					}
					naTemp = naCarry;
					naCarry = naAccu;
					naAccu = naTemp;
					memset(naCarry, 0, sizeof(int32_t) * 3);    // need only to set first pixel zero
					pDest = newImage.info.pImage + (++v * newImage.info.dwEffWidth);
					fEndY += yScale;
				}
			}
			if (v < newy){	// possibly not completed due to rounding errors
				for (i = 0; i < 3 * newx; i++) *pDest++ = (uint8_t)(naAccu[i] / nScale);
			}
			delete [] naAccu;
			delete [] naCarry;
		}
	//}

#if CXIMAGE_SUPPORT_ALPHA
	if (AlphaIsValid()){
		if (1 == mode){
			newImage.AlphaCreate();
			for(int32_t y=0; y<newy; y++){
				fY = y * yScale;
				for(int32_t x=0; x<newx; x++){
					fX = x * xScale;
					newImage.AlphaSet(x,y,AlphaGet((int32_t)fX,(int32_t)fY));
				}
			}
		} else {
			CxImage newAlpha;
			AlphaSplit(&newAlpha);
			newAlpha.Resample(newx, newy, mode);
			newImage.AlphaSet(newAlpha);
		}
	}
#endif //CXIMAGE_SUPPORT_ALPHA

	//select the destination
	if (iDst) iDst->Transfer(newImage);
	else Transfer(newImage);

	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Reduces the number of bits per pixel to nbit (1, 4 or 8).
 * ppal points to a valid palette for the final image; if not supplied the function will use a standard palette.
 * ppal is not necessary for reduction to 1 bpp.
 */
bool CxImage::DecreaseBpp(uint32_t nbit, bool errordiffusion, RGBQUAD* ppal, uint32_t clrimportant)
{
	if (!pDib) return false;
	if (head.biBitCount <  nbit){
		strcpy(info.szLastError,"DecreaseBpp: target BPP greater than source BPP");
		return false;
	}
	if (head.biBitCount == nbit){
		if (clrimportant==0) return true;
		if (head.biClrImportant && (head.biClrImportant<clrimportant)) return true;
	}

	int32_t er,eg,eb;
	RGBQUAD c,ce;

	CxImage tmp;
	tmp.CopyInfo(*this);
	tmp.Create(head.biWidth,head.biHeight,(uint16_t)nbit,info.dwType);
	if (clrimportant) tmp.SetClrImportant(clrimportant);
	if (!tmp.IsValid()){
		strcpy(info.szLastError,tmp.GetLastError());
		return false;
	}

#if CXIMAGE_SUPPORT_SELECTION
	tmp.SelectionCopy(*this);
#endif //CXIMAGE_SUPPORT_SELECTION

#if CXIMAGE_SUPPORT_ALPHA
	tmp.AlphaCopy(*this);
#endif //CXIMAGE_SUPPORT_ALPHA

	if (ppal) {
		if (clrimportant) {
			tmp.SetPalette(ppal,clrimportant);
		} else {
			tmp.SetPalette(ppal,1<<tmp.head.biBitCount);
		}
	} else {
		tmp.SetStdPalette();
	}

	for (int32_t y=0;y<head.biHeight;y++){
		if (info.nEscape) break;
		info.nProgress = (int32_t)(100*y/head.biHeight);
		for (int32_t x=0;x<head.biWidth;x++){
			if (!errordiffusion){
				tmp.BlindSetPixelColor(x,y,BlindGetPixelColor(x,y));
			} else {
				c = BlindGetPixelColor(x,y);
				tmp.BlindSetPixelColor(x,y,c);

				ce = tmp.BlindGetPixelColor(x,y);
				er=(int32_t)c.rgbRed - (int32_t)ce.rgbRed;
				eg=(int32_t)c.rgbGreen - (int32_t)ce.rgbGreen;
				eb=(int32_t)c.rgbBlue - (int32_t)ce.rgbBlue;

				c = GetPixelColor(x+1,y);
				c.rgbRed = (uint8_t)min(255L,max(0L,(int32_t)c.rgbRed + ((er*7)/16)));
				c.rgbGreen = (uint8_t)min(255L,max(0L,(int32_t)c.rgbGreen + ((eg*7)/16)));
				c.rgbBlue = (uint8_t)min(255L,max(0L,(int32_t)c.rgbBlue + ((eb*7)/16)));
				SetPixelColor(x+1,y,c);
				int32_t coeff=1;
				for(int32_t i=-1; i<2; i++){
					switch(i){
					case -1:
						coeff=2; break;
					case 0:
						coeff=4; break;
					case 1:
						coeff=1; break;
					}
					c = GetPixelColor(x+i,y+1);
					c.rgbRed = (uint8_t)min(255L,max(0L,(int32_t)c.rgbRed + ((er * coeff)/16)));
					c.rgbGreen = (uint8_t)min(255L,max(0L,(int32_t)c.rgbGreen + ((eg * coeff)/16)));
					c.rgbBlue = (uint8_t)min(255L,max(0L,(int32_t)c.rgbBlue + ((eb * coeff)/16)));
					SetPixelColor(x+i,y+1,c);
				}
			}
		}
	}

	Transfer(tmp);
	return true;
}


////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_TRANSFORMATION
