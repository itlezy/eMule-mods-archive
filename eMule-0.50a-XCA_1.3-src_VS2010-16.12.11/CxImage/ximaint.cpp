// xImaInt.cpp : interpolation functions
/* 02/2004 - Branko Brevensek 
 * CxImage version 7.0.2 07/Feb/2011 - Davide Pizzolato - www.xdp.it
 */

#include "ximage.h"
#include "ximath.h"

#if CXIMAGE_SUPPORT_INTERPOLATION

////////////////////////////////////////////////////////////////////////////////
/**
 * Recalculates coordinates according to specified overflow method.
 * If pixel (x,y) lies within image, nothing changes.
 *
 *  \param x, y - coordinates of pixel
 *  \param ofMethod - overflow method
 * 
 *  \return x, y - new coordinates (pixel (x,y) now lies inside image)
 *
 *  \author ***bd*** 2.2004
 */
void CxImage::OverflowCoordinates(int32_t &x, int32_t &y, OverflowMethod const ofMethod)
{
  if (IsInside(x,y)) return;  //if pixel is within bounds, no change
  //switch (ofMethod) {
    //case OM_REPEAT:
      //clip coordinates
      x=max(x,0); x=min(x, head.biWidth-1);
      y=max(y,0); y=min(y, head.biHeight-1);
     //break;
  //}//switch
}

////////////////////////////////////////////////////////////////////////////////
/**
 * Method return pixel color. Different methods are implemented for out of bounds pixels.
 * If an image has alpha channel, alpha value is returned in .RGBReserved.
 *
 *  \param x,y : pixel coordinates
 *  \param ofMethod : out-of-bounds method:
 *    - OF_WRAP - wrap over to pixels on other side of the image
 *    - OF_REPEAT - repeat last pixel on the edge
 *    - OF_COLOR - return input value of color
 *    - OF_BACKGROUND - return background color (if not set, return input color)
 *    - OF_TRANSPARENT - return transparent pixel
 *
 *  \param rplColor : input color (returned for out-of-bound coordinates in OF_COLOR mode and if other mode is not applicable)
 *
 * \return color : color of pixel
 * \author ***bd*** 2.2004
 */
RGBQUAD CxImage::GetPixelColorWithOverflow(int32_t x, int32_t y, OverflowMethod const ofMethod, RGBQUAD* const rplColor)
{
  RGBQUAD color;          //color to return
  if ((!IsInside(x,y)) || pDib==NULL) {     //is pixel within bouns?:
    //pixel is out of bounds or no DIB
    if (rplColor!=NULL)
      color=*rplColor;
    else {
      color.rgbRed=color.rgbGreen=color.rgbBlue=255; color.rgbReserved=0; //default replacement colour: white transparent
    }//if
    if (pDib==NULL) return color;
    //pixel is out of bounds:
    switch (ofMethod) {
      case OM_BACKGROUND:
		  //return background color (if it exists, otherwise input value)
		  if (info.nBkgndIndex >= 0) {
			  if (head.biBitCount<24) color = GetPaletteColor((uint8_t)info.nBkgndIndex);
			  else color = info.nBkgndColor;
		  }//if
		  return color;
      case OM_REPEAT:
        OverflowCoordinates(x,y,ofMethod);
        break;
    }//switch
  }//if
  //just return specified pixel (it's within bounds)
  return BlindGetPixelColor(x,y);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * This method reconstructs image according to chosen interpolation method and then returns pixel (x,y).
 * (x,y) can lie between actual image pixels. If (x,y) lies outside of image, method returns value
 * according to overflow method.
 * This method is very useful for geometrical image transformations, where destination pixel
 * can often assume color value lying between source pixels.
 *
 *  \param (x,y) - coordinates of pixel to return
 *           GPCI method recreates "analogue" image back from digital data, so x and y
 *           are float values and color value of point (1.1,1) will generally not be same
 *           as (1,1). Center of first pixel is at (0,0) and center of pixel right to it is (1,0).
 *           (0.5,0) is half way between these two pixels.
 *  \param inMethod - interpolation (reconstruction) method (kernel) to use:
 *    - IM_NEAREST_NEIGHBOUR - returns colour of nearest lying pixel (causes stairy look of 
 *                            processed images)
 *    - IM_BILINEAR - interpolates colour from four neighbouring pixels (softens image a bit)
 *    - IM_BICUBIC - interpolates from 16 neighbouring pixels (can produce "halo" artifacts)
 *    - IM_BICUBIC2 - interpolates from 16 neighbouring pixels (perhaps a bit less halo artifacts 
                     than IM_BICUBIC)
 *    - IM_BSPLINE - interpolates from 16 neighbouring pixels (softens image, washes colours)
 *                  (As far as I know, image should be prefiltered for this method to give 
 *                   good results... some other time :) )
 *                  This method uses bicubic interpolation kernel from CXImage 5.99a and older
 *                  versions.
 *    - IM_LANCZOS - interpolates from 12*12 pixels (slow, ringing artifacts)
 *
 *  \param ofMethod - overflow method (see comments at GetPixelColorWithOverflow)
 *  \param rplColor - pointer to color used for out of borders pixels in OM_COLOR mode
 *              (and other modes if colour can't calculated in a specified way)
 *
 *  \return interpolated color value (including interpolated alpha value, if image has alpha layer)
 * 
 *  \author ***bd*** 2.2004
 */
RGBQUAD CxImage::GetPixelColorInterpolated(
  float x,float y, 
  InterpolationMethod const inMethod, 
  OverflowMethod const ofMethod, 
  RGBQUAD* const rplColor)
{
  //calculate nearest pixel
  int32_t xi=(int32_t)(x); if (x<0) xi--;   //these replace (incredibly slow) floor (Visual c++ 2003, AMD Athlon)
  int32_t yi=(int32_t)(y); if (y<0) yi--;
  RGBQUAD color;                    //calculated colour

  //switch (inMethod) {
    //default: {
      //IM_BILINEAR: bilinear interpolation
      if (xi<-1 || xi>=head.biWidth || yi<-1 || yi>=head.biHeight) {  //all 4 points are outside bounds?:
        //switch (ofMethod) {
          //case OM_COLOR: case OM_TRANSPARENT: case OM_BACKGROUND:
            //we don't need to interpolate anything with all points outside in this case
            return GetPixelColorWithOverflow(-999, -999, ofMethod, rplColor);
        //}//switch
      }//if
      //get four neighbouring pixels
      if ((xi+1)<head.biWidth && xi>=0 && (yi+1)<head.biHeight && yi>=0 && head.biClrUsed==0) {
        //all pixels are inside RGB24 image... optimize reading (and use fixed point arithmetic)
        uint16_t wt1=(uint16_t)((x-xi)*256.0f), wt2=(uint16_t)((y-yi)*256.0f);
        uint16_t wd=wt1*wt2>>8;
        uint16_t wb=wt1-wd;
        uint16_t wc=wt2-wd;
        uint16_t wa=256-wt1-wc;
        uint16_t wrr,wgg,wbb;
        uint8_t *pxptr=(uint8_t*)info.pImage+yi*info.dwEffWidth+xi*3;
        wbb=wa*(*pxptr++); wgg=wa*(*pxptr++); wrr=wa*(*pxptr++);
        wbb+=wb*(*pxptr++); wgg+=wb*(*pxptr++); wrr+=wb*(*pxptr);
        pxptr+=(info.dwEffWidth-5); //move to next row
        wbb+=wc*(*pxptr++); wgg+=wc*(*pxptr++); wrr+=wc*(*pxptr++); 
        wbb+=wd*(*pxptr++); wgg+=wd*(*pxptr++); wrr+=wd*(*pxptr); 
        color.rgbRed=(uint8_t) (wrr>>8); color.rgbGreen=(uint8_t) (wgg>>8); color.rgbBlue=(uint8_t) (wbb>>8);
#if CXIMAGE_SUPPORT_ALPHA
        if (pAlpha) {
          uint16_t waa;
          //image has alpha layer... we have to do the same for alpha data
          pxptr=AlphaGetPointer(xi,yi);                           //pointer to first byte
          waa=wa*(*pxptr++); waa+=wb*(*pxptr);   //first two pixels
          pxptr+=(head.biWidth-1);                                //move to next row
          waa+=wc*(*pxptr++); waa+=wd*(*pxptr);   //and second row pixels
          color.rgbReserved=(uint8_t) (waa>>8);
        } else
#endif
		{ //Alpha not supported or no alpha at all
			color.rgbReserved = 0;
		}
        return color;
      } else {
        //default (slower) way to get pixels (not RGB24 or some pixels out of borders)
        float t1=x-xi, t2=y-yi;
        float d=t1*t2;
        float b=t1-d;
        float c=t2-d;
        float a=1-t1-c;
        RGBQUAD rgb11,rgb21,rgb12,rgb22;
        rgb11=GetPixelColorWithOverflow(xi, yi, ofMethod, rplColor);
        rgb21=GetPixelColorWithOverflow(xi+1, yi, ofMethod, rplColor);
        rgb12=GetPixelColorWithOverflow(xi, yi+1, ofMethod, rplColor);
        rgb22=GetPixelColorWithOverflow(xi+1, yi+1, ofMethod, rplColor);
        //calculate linear interpolation
        color.rgbRed=(uint8_t) (a*rgb11.rgbRed+b*rgb21.rgbRed+c*rgb12.rgbRed+d*rgb22.rgbRed);
        color.rgbGreen=(uint8_t) (a*rgb11.rgbGreen+b*rgb21.rgbGreen+c*rgb12.rgbGreen+d*rgb22.rgbGreen);
        color.rgbBlue=(uint8_t) (a*rgb11.rgbBlue+b*rgb21.rgbBlue+c*rgb12.rgbBlue+d*rgb22.rgbBlue);
#if CXIMAGE_SUPPORT_ALPHA
		color.rgbReserved=(uint8_t) (a*rgb11.rgbReserved+b*rgb21.rgbReserved+c*rgb12.rgbReserved+d*rgb22.rgbReserved);
#else
		color.rgbReserved = 0;
#endif
        return color;
      }//if
    //}//default
  //}//switch
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Helper function for GetAreaColorInterpolated.
 * Adds 'surf' portion of image pixel with color 'color' to (rr,gg,bb,aa).
 */
void CxImage::AddAveragingCont(RGBQUAD const &color, float const surf, float &rr, float &gg, float &bb, float &aa)
{
  rr+=color.rgbRed*surf;
  gg+=color.rgbGreen*surf;
  bb+=color.rgbBlue*surf;
#if CXIMAGE_SUPPORT_ALPHA
  aa+=color.rgbReserved*surf;
#endif
}
////////////////////////////////////////////////////////////////////////////////
/**
 * This method is similar to GetPixelColorInterpolated, but this method also properly handles 
 * subsampling.
 * If you need to sample original image with interval of more than 1 pixel (as when shrinking an image), 
 * you should use this method instead of GetPixelColorInterpolated or aliasing will occur.
 * When area width and height are both less than pixel, this method gets pixel color by interpolating
 * color of frame center with selected (inMethod) interpolation by calling GetPixelColorInterpolated. 
 * If width and height are more than 1, method calculates color by averaging color of pixels within area.
 * Interpolation method is not used in this case. Pixel color is interpolated by averaging instead.
 * If only one of both is more than 1, method uses combination of interpolation and averaging.
 * Chosen interpolation method is used, but since it is averaged later on, there is little difference
 * between IM_BILINEAR (perhaps best for this case) and better methods. IM_NEAREST_NEIGHBOUR again
 * leads to aliasing artifacts.
 * This method is a bit slower than GetPixelColorInterpolated and when aliasing is not a problem, you should
 * simply use the later. 
 *
 * \param  xc, yc - center of (rectangular) area
 * \param  w, h - width and height of area
 * \param  inMethod - interpolation method that is used, when interpolation is used (see above)
 * \param  ofMethod - overflow method used when retrieving individual pixel colors
 * \param  rplColor - replacement colour to use, in OM_COLOR
 *
 * \author ***bd*** 2.2004
 */
RGBQUAD CxImage::GetAreaColorInterpolated(
  float const xc, float const yc, float const w, float const h, 
  InterpolationMethod const inMethod, 
  OverflowMethod const ofMethod, 
  RGBQUAD* const rplColor)
{
	RGBQUAD color;      //calculated colour
	
	if (h<=1 && w<=1) {
		//both width and height are less than one... we will use interpolation of center point
		return GetPixelColorInterpolated(xc, yc, inMethod, ofMethod, rplColor);
	} else {
		//area is wider and/or taller than one pixel:
		CxRect2 area(xc-w/2.0f, yc-h/2.0f, xc+w/2.0f, yc+h/2.0f);   //area
		int32_t xi1=(int32_t)(area.botLeft.x+0.49999999f);                //low x
		int32_t yi1=(int32_t)(area.botLeft.y+0.49999999f);                //low y
		
		
		int32_t xi2=(int32_t)(area.topRight.x+0.5f);                      //top x
		int32_t yi2=(int32_t)(area.topRight.y+0.5f);                      //top y (for loops)
		
		float rr,gg,bb,aa;                                        //red, green, blue and alpha components
		rr=gg=bb=aa=0;
		int32_t x,y;                                                  //loop counters
		float s=0;                                                //surface of all pixels
		float cps;                                                //surface of current crosssection
		if (h>1 && w>1) {
			//width and height of area are greater than one pixel, so we can employ "ordinary" averaging
			CxRect2 intBL, intTR;     //bottom left and top right intersection
			intBL=area.CrossSection(CxRect2(((float)xi1)-0.5f, ((float)yi1)-0.5f, ((float)xi1)+0.5f, ((float)yi1)+0.5f));
			intTR=area.CrossSection(CxRect2(((float)xi2)-0.5f, ((float)yi2)-0.5f, ((float)xi2)+0.5f, ((float)yi2)+0.5f));
			float wBL, wTR, hBL, hTR;
			wBL=intBL.Width();            //width of bottom left pixel-area intersection
			hBL=intBL.Height();           //height of bottom left...
			wTR=intTR.Width();            //width of top right...
			hTR=intTR.Height();           //height of top right...
			
			AddAveragingCont(GetPixelColorWithOverflow(xi1,yi1,ofMethod,rplColor), wBL*hBL, rr, gg, bb, aa);    //bottom left pixel
			AddAveragingCont(GetPixelColorWithOverflow(xi2,yi1,ofMethod,rplColor), wTR*hBL, rr, gg, bb, aa);    //bottom right pixel
			AddAveragingCont(GetPixelColorWithOverflow(xi1,yi2,ofMethod,rplColor), wBL*hTR, rr, gg, bb, aa);    //top left pixel
			AddAveragingCont(GetPixelColorWithOverflow(xi2,yi2,ofMethod,rplColor), wTR*hTR, rr, gg, bb, aa);    //top right pixel
			//bottom and top row
			for (x=xi1+1; x<xi2; x++) {
				AddAveragingCont(GetPixelColorWithOverflow(x,yi1,ofMethod,rplColor), hBL, rr, gg, bb, aa);    //bottom row
				AddAveragingCont(GetPixelColorWithOverflow(x,yi2,ofMethod,rplColor), hTR, rr, gg, bb, aa);    //top row
			}
			//leftmost and rightmost column
			for (y=yi1+1; y<yi2; y++) {
				AddAveragingCont(GetPixelColorWithOverflow(xi1,y,ofMethod,rplColor), wBL, rr, gg, bb, aa);    //left column
				AddAveragingCont(GetPixelColorWithOverflow(xi2,y,ofMethod,rplColor), wTR, rr, gg, bb, aa);    //right column
			}
			for (y=yi1+1; y<yi2; y++) {
				for (x=xi1+1; x<xi2; x++) { 
					color=GetPixelColorWithOverflow(x,y,ofMethod,rplColor);
					rr+=color.rgbRed;
					gg+=color.rgbGreen;
					bb+=color.rgbBlue;
#if CXIMAGE_SUPPORT_ALPHA
					aa+=color.rgbReserved;
#endif
				}//for x
			}//for y
		} else {
			//width or height greater than one:
			CxRect2 intersect;                                          //intersection with current pixel
			CxPoint2 center;
			for (y=yi1; y<=yi2; y++) {
				for (x=xi1; x<=xi2; x++) {
					intersect=area.CrossSection(CxRect2(((float)x)-0.5f, ((float)y)-0.5f, ((float)x)+0.5f, ((float)y)+0.5f));
					center=intersect.Center();
					color=GetPixelColorInterpolated(center.x, center.y, inMethod, ofMethod, rplColor);
					cps=intersect.Surface();
					rr+=color.rgbRed*cps;
					gg+=color.rgbGreen*cps;
					bb+=color.rgbBlue*cps;
#if CXIMAGE_SUPPORT_ALPHA
					aa+=color.rgbReserved*cps;
#endif
				}//for x
			}//for y      
		}//if
		
		s=area.Surface();
		rr/=s; gg/=s; bb/=s; aa/=s;
		if (rr>255) rr=255; if (rr<0) rr=0; color.rgbRed=(uint8_t) rr;
		if (gg>255) gg=255; if (gg<0) gg=0; color.rgbGreen=(uint8_t) gg;
		if (bb>255) bb=255; if (bb<0) bb=0; color.rgbBlue=(uint8_t) bb;
#if CXIMAGE_SUPPORT_ALPHA
		if (aa>255) aa=255; if (aa<0) aa=0; color.rgbReserved=(uint8_t) aa;
#else
		color.rgbReserved = 0;
#endif
	}//if
	return color;
}
#endif
