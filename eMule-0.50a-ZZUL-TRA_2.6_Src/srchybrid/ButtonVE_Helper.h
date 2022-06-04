#if _MSC_VER>=1600
#ifndef _BUTTONVE_HELPER_H_
#define _BUTTONVE_HELPER_H_


#ifndef _VEIMAGEPLACEMENTTYPE_
#define _VEIMAGEPLACEMENTTYPE_
typedef enum {
  IMAGEPOS_LEFT=BS_LEFT, IMAGEPOS_RIGHT=BS_RIGHT, 
  IMAGEPOS_TOP=BS_TOP, IMAGEPOS_BOTTOM=BS_BOTTOM} VEImagePlacementType;
#define ROP_PSDPxax 0x00B8074AL 
#define ROP_DSna 0x00220326L    // Dest = (not SRC) and Dest
#endif _VEIMAGEPLACEMENTTYPE_



//
// Helper class for dealing with memory drawing contexts...
//
#ifndef _CDCBITMAP_
#define _CDCBITMAP_
class CDCBitmap : public CDC
{
  HDC m_src_hDC;
  CRect m_rc;
  HBITMAP mem_bitmap;
  HBITMAP old_bitmap;
public:
  CDCBitmap(HDC hDC, HBITMAP hBitmap) {
    Attach(::CreateCompatibleDC(hDC));
    old_bitmap = (HBITMAP)SelectObject(hBitmap); 
    mem_bitmap=NULL;
  }

  CDCBitmap(HDC hDC, CBitmap *bmp) {
    ASSERT(bmp);
    Attach(::CreateCompatibleDC(hDC));
    old_bitmap = (HBITMAP)SelectObject(*bmp); 
    mem_bitmap=NULL;
  }

  CDCBitmap(HDC hDC, CRect rc) {
    m_src_hDC = hDC;
    m_rc = rc;
    Attach(::CreateCompatibleDC(hDC));
    mem_bitmap = ::CreateCompatibleBitmap(hDC,m_rc.Width(),m_rc.Height());
    old_bitmap = (HBITMAP)SelectObject(mem_bitmap);
    SetWindowOrg(m_rc.left,m_rc.top);
  }

  void SetBitmap(HBITMAP hBitmap) {
    if (mem_bitmap) 
      ::BitBlt(m_src_hDC, m_rc.left, m_rc.top, m_rc.Width(), m_rc.Height(), 
        GetSafeHdc(), m_rc.left, m_rc.top, SRCCOPY);
    SelectObject(hBitmap);
    if (mem_bitmap) ::DeleteObject(mem_bitmap);
    mem_bitmap = NULL;
  }

  ~CDCBitmap() {SetBitmap(old_bitmap);}
};
#endif // _CDCBITMAP_



//
// Helper class for various types of image processing...
//
class CButtonVE_Helper {
public:
  //
  // Draw monochrome mask.  1's in the bitmap are converted to the color paremeter.   
  // 0's are left unchanged in the destination.  Gotta love ROP's...
  //
  void DrawMonoBitmap (CDC &dc, CDC &monoDC, CPoint icoord, CSize imagesize, COLORREF color) 
  {
    CBrush brush;
    brush.CreateSolidBrush(color);
    CBrush *old_brush = dc.SelectObject(&brush);
    dc.SetTextColor(RGB(0,0,0));
    dc.SetBkColor(RGB(255,255,255));
    dc.BitBlt(icoord.x, icoord.y, imagesize.cx, imagesize.cy, &monoDC, 0, 0, ROP_PSDPxax); 
    dc.SelectObject(old_brush); 
  }
  


  //
  // Create "disabled" version of bitmap.   Replace all non-transparent pixels with shaded mask...
  //
  void DrawDisabledImage(CDC &dc, CDC &srcDC, CPoint icoord, CSize imagesize, COLORREF transparent_color) 
  {
    // Create monochrome mask based on transparent color...
    CBitmap mBitmap; mBitmap.CreateBitmap(imagesize.cx, imagesize.cy, 1, 1, NULL);
    CDCBitmap monoDC(dc,mBitmap);
    srcDC.SetBkColor(transparent_color);
    monoDC.BitBlt(0, 0, imagesize.cx, imagesize.cy, &srcDC, 0, 0, SRCCOPY); 

    // Draw monochrome mask twice.  First in highlight color (offset by a pixel), then in shadow color.
    // Creates depth in the disabled image.
    DrawMonoBitmap(dc, monoDC, icoord+CPoint(1,1), imagesize, ::GetSysColor(COLOR_BTNHIGHLIGHT));
    DrawMonoBitmap(dc, monoDC, icoord, imagesize, ::GetSysColor(COLOR_BTNSHADOW));
  }



  //
  // Draw image in source DC filtering transparent color...
  //
  void DrawTransparentImage(CDC &dc, CDC &srcDC, CPoint icoord, CSize imagesize, COLORREF transparent_color)
  {
    // Create monochrome mask based on transparent color...
    CBitmap mBitmap; mBitmap.CreateBitmap(imagesize.cx, imagesize.cy, 1, 1, NULL);
    CDCBitmap monoDC(dc, mBitmap); 
    srcDC.SetBkColor(transparent_color);
    monoDC.BitBlt(0, 0, imagesize.cx, imagesize.cy, &srcDC, 0, 0, SRCCOPY);

    // Create dc with transparent colored pixels masked out...
    CBitmap tBitmap; tBitmap.CreateCompatibleBitmap(&dc, imagesize.cx, imagesize.cy);
    CDCBitmap maskDC(dc, tBitmap);
    maskDC.BitBlt(0, 0, imagesize.cx, imagesize.cy, &srcDC, 0, 0, SRCCOPY);
    maskDC.BitBlt(0, 0, imagesize.cx, imagesize.cy, &monoDC, 0, 0, ROP_DSna);

    // Mask out non-transparent pixels & combine with background on destination DC...
    dc.SetBkColor(RGB(255,255,255));
    dc.BitBlt(icoord.x, icoord.y, imagesize.cx, imagesize.cy, &monoDC, 0, 0, SRCAND);
    dc.BitBlt(icoord.x, icoord.y, imagesize.cx, imagesize.cy, &maskDC, 0, 0, SRCPAINT);
  } 



  //
  // Get DIB pixels from bitmap.  Returns dword width of image if successful.
  //
  int GetDIBPixels(HDC dc, HBITMAP bmp, int width, int height, BITMAPINFO &bmi, DWORD **pixels)
  {
    ASSERT (pixels);
    // Init BITMAPINFO...
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    if (::GetDIBits(dc, bmp, 0, bmi.bmiHeader.biHeight, NULL, &bmi, DIB_RGB_COLORS)==0) return 0;

    // Compute image size if driver didn't...
    #define WIDTHDWORD(bits) (((bits) + 31)>>5)
    int dword_width = WIDTHDWORD((DWORD)bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount);
    if (bmi.bmiHeader.biSizeImage == 0) bmi.bmiHeader.biSizeImage = (dword_width<<2) * bmi.bmiHeader.biHeight;

    // Get pixels...
    DWORD *pixbuf = new DWORD[bmi.bmiHeader.biSizeImage];
    if (pixbuf==NULL) return 0;
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    if (::GetDIBits(dc, bmp, 0, bmi.bmiHeader.biHeight, pixbuf, &bmi, DIB_RGB_COLORS)==0) return 0;
    if (pixels) *pixels = pixbuf;
    return dword_width;
  } 


  // Draw image in source DC as blurred shadow filtering transparent color.  
  // Uses 5x5 Guassian blurring on a DIB.  Output image is larger by 4 pixels in x/y directions.
  void DrawBlurredShadowImage(CDC &dc, CDC &srcDC, CPoint icoord, CSize imagesize, COLORREF transparent_color)
  {
    #define BORDER 2
    #define SRCPIX(xofs,yofs) ((BYTE)srcpix[xofs+yofs*src_width])

    // Create monochrome mask based on transparent color...
    CBitmap mBitmap; mBitmap.CreateBitmap(imagesize.cx, imagesize.cy, 1, 1, NULL);
    CDCBitmap monoDC(dc, mBitmap);
    srcDC.SetBkColor(transparent_color);
    monoDC.BitBlt(0, 0, imagesize.cx, imagesize.cy, &srcDC, 0, 0, SRCCOPY);

    // Draw mono bitmap into 32 bit image in gray (on white background), and get DIB pixel pointer...
    CSize newimgsize = imagesize+CSize(4*BORDER,4*BORDER);
    CBitmap tBitmap; 
    if (tBitmap.CreateBitmap(newimgsize.cx, newimgsize.cy, 1, 32, NULL)) {
      CDCBitmap tempDC(dc, tBitmap); 
      tempDC.FillSolidRect(0, 0, newimgsize.cx, newimgsize.cy, RGB(0xFF,0xFF,0xFF));
      DrawMonoBitmap(tempDC, monoDC, CPoint(2*BORDER,2*BORDER), imagesize, RGB(0x80,0x80,0x80));
    }

    BITMAPINFO src_bmi;
    DWORD *src_pixels=NULL;
    int src_width = GetDIBPixels(dc, tBitmap, newimgsize.cx, newimgsize.cy, src_bmi, &src_pixels);
    if ((src_width==0) || (src_pixels==NULL)) return;

    // Now make copy of destination & get DIB pixel pointer...
    CBitmap dBitmap;
    if (dBitmap.CreateBitmap(newimgsize.cx, newimgsize.cy, 1, 32, NULL)) {
      CDCBitmap tempDC(dc, dBitmap); 
      tempDC.BitBlt(0, 0, newimgsize.cx, newimgsize.cy, &dc, icoord.x-BORDER, icoord.y-BORDER, SRCCOPY);
    }
    
    BITMAPINFO dest_bmi;
    DWORD *dest_pixels=NULL;
    int dest_width = GetDIBPixels(dc, dBitmap, newimgsize.cx, newimgsize.cy, dest_bmi, &dest_pixels);
    if ((dest_width==0) || (dest_pixels==NULL)) {delete [] src_pixels; return;}

    // Finally... the blurring can begin!  Guassian 5x5 matrix applied.
    // A weighted average of the 25 pixels surrounding the current position.
    // Once average value computed, blend with destination image.
    //int src_width2x = src_width<<1; // unused
    int linestart = (BORDER + src_width*BORDER);
    for (int y=0; y<(imagesize.cy+2*BORDER); y++) {
      DWORD *srcpix = &src_pixels[linestart];
      DWORD *destpix = &dest_pixels[linestart];
      for (int x=0; x<(imagesize.cx+2*BORDER); x++) {
        int value = // average 5x5 matrix of pixels, weighting towards center...
          (1*SRCPIX(-2,-2) + 2*SRCPIX(-1,-2) + 3*SRCPIX(0,-2) + 2*SRCPIX(1,-2) + 1*SRCPIX(2,-2) +
           2*SRCPIX(-2,-1) + 4*SRCPIX(-1,-1) + 6*SRCPIX(0,-1) + 4*SRCPIX(1,-1) + 2*SRCPIX(2,-1) +
           3*SRCPIX(-2,0)  + 6*SRCPIX(-1,0)  + 8*SRCPIX(0,0)  + 6*SRCPIX(1,0)  + 3*SRCPIX(2,0) +
           2*SRCPIX(-2,1)  + 4*SRCPIX(-1,1)  + 6*SRCPIX(0,1)  + 4*SRCPIX(1,1)  + 2*SRCPIX(2,1)  +
           1*SRCPIX(-2,2)  + 2*SRCPIX(-1,2)  + 3*SRCPIX(0,2)  + 2*SRCPIX(1,2)  + 1*SRCPIX(2,2))/80;
        if (value>0xFF) value=0xFF;

        // Blend with destination...
        DWORD dpix = *destpix;
        *destpix = RGB((GetRValue(dpix)*value/0xFF), (GetGValue(dpix)*value/0xFF), (GetBValue(dpix)*value/0xFF));

        destpix++;
        srcpix++;
      }
      linestart += src_width;
    }

    // Draw finished shadow...
    ::StretchDIBits(dc, 
      icoord.x, icoord.y, newimgsize.cx-2*BORDER, newimgsize.cy-2*BORDER,
      BORDER, BORDER, newimgsize.cx-2*BORDER, newimgsize.cy-2*BORDER, 
      dest_pixels, &dest_bmi,
      DIB_RGB_COLORS, SRCCOPY);

    // Cleanup...
    delete [] src_pixels;
    delete [] dest_pixels;
  }


  // Alpha blend bitmap into destination.  Alpha is 0 (nothing) to 255 (max).
  void AlphaBlt(HDC destdc, CRect rdest, HDC srcdc, CRect rsrc, int alpha) {
    ASSERT ((alpha>=0) && (alpha<=255));
    if (alpha<0) alpha=0;
    if (alpha>255) alpha=255;
    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = BYTE(alpha);
    bf.AlphaFormat = 0; // AC_SRC_ALPHA;
    AlphaBlend(destdc, rdest.left, rdest.top, rdest.Width(), rdest.Height(), 
      srcdc, rsrc.left, rsrc.top, rsrc.Width(), rsrc.Height(), bf);
  }


  // Alpha blend just an empty frame, of "border_size" thickness.  Alpha is 0 (nothing) to 255 (max). 
  void AlphaBltFrame(HDC destdc, HDC srcdc, CRect rc, CSize border_size, int alpha)
  {
    CRect use_rc;
    use_rc.SetRect(rc.left, rc.top, rc.right, rc.top+border_size.cy);
    AlphaBlt (destdc, use_rc, srcdc, use_rc, alpha);
    use_rc.SetRect(rc.left, rc.bottom-border_size.cy, rc.right, rc.bottom);
    AlphaBlt (destdc, use_rc, srcdc, use_rc, alpha);
    use_rc.SetRect(rc.left, rc.top+border_size.cy, rc.left+border_size.cx, rc.bottom-border_size.cy);
    AlphaBlt (destdc, use_rc, srcdc, use_rc, alpha);
    use_rc.SetRect(rc.right-border_size.cx, rc.top+border_size.cy, rc.right, rc.bottom-border_size.cy);
    AlphaBlt (destdc, use_rc, srcdc, use_rc, alpha);
  }
};

#endif // define _BUTTONVE_HELPER_H_
#endif
