// xImaDsp.cpp : DSP functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

#include "ximaiter.h"
////////////////////////////////////////////////////////////////////////////////
/**
 * Adds a random offset to each pixel in the image
 * \param radius: maximum pixel displacement
 * \return true if everything is ok
 */
bool CxImage::Jitter(int32_t radius)
{
	if (!pDib) return false;

	int32_t nx,ny;

	CxImage tmp(*this);
	if (!tmp.IsValid()){
		strcpy(info.szLastError,tmp.GetLastError());
		return false;
	}

	int32_t xmin,xmax,ymin,ymax;
	if (pSelection){
		xmin = info.rSelectionBox.left; xmax = info.rSelectionBox.right;
		ymin = info.rSelectionBox.bottom; ymax = info.rSelectionBox.top;
	} else {
		xmin = ymin = 0;
		xmax = head.biWidth; ymax=head.biHeight;
	}

	for(int32_t y=ymin; y<ymax; y++){
		info.nProgress = (int32_t)(100*(y-ymin)/(ymax-ymin));
		if (info.nEscape) break;
		for(int32_t x=xmin; x<xmax; x++){
#if CXIMAGE_SUPPORT_SELECTION
			if (BlindSelectionIsInside(x,y))
#endif //CXIMAGE_SUPPORT_SELECTION
			{
				nx=x+(int32_t)((rand()/(float)RAND_MAX - 0.5)*(radius*2));
				ny=y+(int32_t)((rand()/(float)RAND_MAX - 0.5)*(radius*2));
				if (!IsInside(nx,ny)) {
					nx=x;
					ny=y;
				}
				if (head.biClrUsed==0){
					tmp.BlindSetPixelColor(x,y,BlindGetPixelColor(nx,ny));
				} else {
					tmp.BlindSetPixelIndex(x,y,BlindGetPixelIndex(nx,ny));
				}
#if CXIMAGE_SUPPORT_ALPHA
				tmp.AlphaSet(x,y,AlphaGet(nx,ny));
#endif //CXIMAGE_SUPPORT_ALPHA
			}
		}
	}
	Transfer(tmp);
	return true;
}
