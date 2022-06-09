// ximainfo.cpp : main attributes
/* 03/10/2004 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

#if defined(_LINUX) || defined(__APPLE__)
 #define _tcsnicmp(a,b,c) strcasecmp(a,b)
#endif

////////////////////////////////////////////////////////////////////////////////
/**
 * \return the color used for transparency, and/or for background color
 */
RGBQUAD	CxImage::GetTransColor()
{
	if (head.biBitCount<24 && info.nBkgndIndex>=0) return GetPaletteColor((uint8_t)info.nBkgndIndex);
	return info.nBkgndColor;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the index used for transparency. Returns -1 for no transparancy.
 */
int32_t CxImage::GetTransIndex() const
{
	return info.nBkgndIndex;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the index used for transparency with 1, 4 and 8 bpp images. Set to -1 to remove the effect.
 */
void CxImage::SetTransIndex(int32_t idx)
{
	if (idx<(int32_t)head.biClrUsed)
		info.nBkgndIndex = idx;
	else 
		info.nBkgndIndex = 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns true if the image has 256 colors or less.
 */
bool CxImage::IsIndexed() const
{
	return head.biClrUsed!=0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Set resolution for TIFF, JPEG, PNG and BMP formats.
 */
void CxImage::SetXDPI(int32_t dpi)
{
	if (dpi<=0) dpi = CXIMAGE_DEFAULT_DPI;
	info.xDPI = dpi;
	head.biXPelsPerMeter = (int32_t) floor(dpi * 10000.0 / 254.0 + 0.5);
	if (pDib) ((BITMAPINFOHEADER*)pDib)->biXPelsPerMeter = head.biXPelsPerMeter;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Set resolution for TIFF, JPEG, PNG and BMP formats.
 */
void CxImage::SetYDPI(int32_t dpi)
{
	if (dpi<=0) dpi = CXIMAGE_DEFAULT_DPI;
	info.yDPI = dpi;
	head.biYPelsPerMeter = (int32_t) floor(dpi * 10000.0 / 254.0 + 0.5);
	if (pDib) ((BITMAPINFOHEADER*)pDib)->biYPelsPerMeter = head.biYPelsPerMeter;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \sa SetCodecOption
 */
uint32_t CxImage::GetCodecOption(uint32_t imagetype)
{
	imagetype = GetTypeIndexFromId(imagetype);
	if (imagetype==0){
		imagetype = GetTypeIndexFromId(GetType());
	}
	return info.dwCodecOpt[imagetype];
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Encode option for GIF, TIF, JPG, PNG and RAW
 * - GIF : 0 = LZW (default), 1 = none, 2 = RLE.
 * - TIF : 0 = automatic (default), or a valid compression code as defined in "tiff.h" (COMPRESSION_NONE = 1, COMPRESSION_CCITTRLE = 2, ...)
 * - JPG : valid values stored in enum CODEC_OPTION ( ENCODE_BASELINE = 0x01, ENCODE_PROGRESSIVE = 0x10, ...)
 * - PNG : combination of interlace option and compression option
 *         interlace option :  1 = interlace, 0 = no interlace
 *         compression option : 2 = no compression, 4 = best speed, 6 = best compression, 8 = default compression
 *         default is no interlace and default compression
 *         example : 5 = 1+4 = interlace + best speed
 * - RAW : valid values stored in enum CODEC_OPTION ( DECODE_QUALITY_LIN = 0x00, DECODE_QUALITY_VNG = 0x01, ...)
 *
 * \return true if everything is ok
 */
bool CxImage::SetCodecOption(uint32_t opt, uint32_t imagetype)
{
	imagetype = GetTypeIndexFromId(imagetype);
	if (imagetype==0){
		imagetype = GetTypeIndexFromId(GetType());
	}
	info.dwCodecOpt[imagetype] = opt;
	return true;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \return internal hDib object..
 */
void* CxImage::GetDIB() const
{
	return pDib;
}
////////////////////////////////////////////////////////////////////////////////
uint32_t CxImage::GetHeight() const
{
	return head.biHeight;
}
////////////////////////////////////////////////////////////////////////////////
uint32_t CxImage::GetWidth() const
{
	return head.biWidth;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \return uint32_t aligned width of the image.
 */
uint32_t CxImage::GetEffWidth() const
{
	return info.dwEffWidth;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \return 2, 16, 256; 0 for RGB images.
 */
uint32_t CxImage::GetNumColors() const
{
	return head.biClrUsed;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \return: 1, 4, 8, 24.
 */
uint16_t CxImage::GetBpp() const
{
	return head.biBitCount;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * \return original image format
 * \sa ENUM_CXIMAGE_FORMATS.
 */
uint32_t CxImage::GetType() const
{
	return info.dwType;
}
////////////////////////////////////////////////////////////////////////////////
uint32_t CxImage::GetTypeIndexFromId(const uint32_t id)
{
	uint32_t n;

	n=0; if (id == CXIMAGE_FORMAT_UNKNOWN) return n;
#if CXIMAGE_SUPPORT_BMP
	n++; if (id == CXIMAGE_FORMAT_BMP) return n;
#endif
#if CXIMAGE_SUPPORT_PNG
	n++; if (id == CXIMAGE_FORMAT_PNG) return n;
#endif

	return 0;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Checks if the image is correctly initializated.
 */
bool CxImage::IsValid() const
{
	return pDib!=0;
}
////////////////////////////////////////////////////////////////////////////////
int16_t CxImage::m_ntohs(const int16_t word)
{
	if (info.bLittleEndianHost) return word;
	return ( (word & 0xff) << 8 ) | ( (word >> 8) & 0xff );
}
////////////////////////////////////////////////////////////////////////////////
int32_t CxImage::m_ntohl(const int32_t dword)
{
	if (info.bLittleEndianHost) return dword;
	return  ((dword & 0xff) << 24 ) | ((dword & 0xff00) << 8 ) |
			((dword >> 8) & 0xff00) | ((dword >> 24) & 0xff);
}
////////////////////////////////////////////////////////////////////////////////
void CxImage::bihtoh(BITMAPINFOHEADER* bih)
{
	bih->biSize = m_ntohl(bih->biSize);
	bih->biWidth = m_ntohl(bih->biWidth);
	bih->biHeight = m_ntohl(bih->biHeight);
	bih->biPlanes = m_ntohs(bih->biPlanes);
	bih->biBitCount = m_ntohs(bih->biBitCount);
	bih->biCompression = m_ntohl(bih->biCompression);
	bih->biSizeImage = m_ntohl(bih->biSizeImage);
	bih->biXPelsPerMeter = m_ntohl(bih->biXPelsPerMeter);
	bih->biYPelsPerMeter = m_ntohl(bih->biYPelsPerMeter);
	bih->biClrUsed = m_ntohl(bih->biClrUsed);
	bih->biClrImportant = m_ntohl(bih->biClrImportant);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Returns the last reported error.
 */
const char* CxImage::GetLastError()
{
	return info.szLastError;
}
