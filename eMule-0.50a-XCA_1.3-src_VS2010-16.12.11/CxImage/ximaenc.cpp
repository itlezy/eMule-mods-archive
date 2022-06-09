// xImaCodec.cpp : Encode Decode functions
/* 07/08/2001 v1.00 - Davide Pizzolato - www.xdp.it
 * CxImage version 7.0.2 07/Feb/2011
 */

#include "ximage.h"

#if CXIMAGE_SUPPORT_PNG
#include "ximapng.h"
#endif

#if CXIMAGE_SUPPORT_BMP
#include "ximabmp.h"
#endif
////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////
bool CxImage::EncodeSafeCheck(CxFile *hFile)
{
	if (hFile==NULL) {
		strcpy(info.szLastError,CXIMAGE_ERR_NOFILE);
		return true;
	}

	if (pDib==NULL){
		strcpy(info.szLastError,CXIMAGE_ERR_NOIMAGE);
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to memory buffer the image in a specific format.
 * \param buffer: output memory buffer pointer. Must be NULL,
 * the function allocates and fill the memory,
 * the application must free the buffer, see also FreeMemory().
 * \param size: output memory buffer size.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Encode(uint8_t * &buffer, int32_t &size, uint32_t imagetype)
{
	if (buffer!=NULL){
		strcpy(info.szLastError,"the buffer must be empty");
		return false;
	}
	CxMemFile file;
	file.Open();
	if(Encode(&file,imagetype)){
		buffer=file.GetBuffer();
		size=file.Size();
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Saves to disk the image in a specific format.
 * \param hFile: file handle (CxMemFile or CxIOFile), with write access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 * \sa ENUM_CXIMAGE_FORMATS
 */
bool CxImage::Encode(CxFile *hFile, uint32_t imagetype)
{

#if CXIMAGE_SUPPORT_BMP
	if (CXIMAGE_FORMAT_BMP==imagetype){
		CxImageBMP *newima = new CxImageBMP;
		if (!newima) return false;
		newima->Ghost(this);
		if (newima->Encode(hFile)){
			delete newima;
			return true;
		} else {
			strcpy(info.szLastError,newima->GetLastError());
			delete newima;
			return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PNG
	if (CXIMAGE_FORMAT_PNG==imagetype){
		CxImagePNG *newima = new CxImagePNG;
		if (!newima) return false;
		newima->Ghost(this);
		if (newima->Encode(hFile)){
			delete newima;
			return true;
		} else {
			strcpy(info.szLastError,newima->GetLastError());
			return false;
		}
	}
#endif

	strcpy(info.szLastError,"Encode: Unknown format");
	return false;
}
////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_ENCODE
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#if CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor from memory buffer, see Decode()
 * \param buffer: memory buffer
 * \param size: size of buffer
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 */
CxImage::CxImage(uint8_t * buffer, uint32_t size, uint32_t imagetype)
{
	Startup(imagetype);
	CxMemFile stream(buffer,size);
	Decode(&stream,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from memory buffer
 * \param buffer: memory buffer
 * \param size: size of buffer
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 */
bool CxImage::Decode(uint8_t * buffer, uint32_t size, uint32_t imagetype)
{
	CxMemFile file(buffer,size);
	return Decode(&file,imagetype);
}
////////////////////////////////////////////////////////////////////////////////
/**
 * Loads an image from CxFile object
 * \param hFile: file handle (CxMemFile or CxIOFile), with read access.
 * \param imagetype: file format, see ENUM_CXIMAGE_FORMATS
 * \return true if everything is ok
 * \sa ENUM_CXIMAGE_FORMATS
 */
bool CxImage::Decode(CxFile *hFile, uint32_t imagetype)
{
	if (hFile == NULL){
		strcpy(info.szLastError,CXIMAGE_ERR_NOFILE);
		return false;
	}

	uint32_t pos = hFile->Tell();

#if CXIMAGE_SUPPORT_BMP
	if (CXIMAGE_FORMAT_UNKNOWN==imagetype || CXIMAGE_FORMAT_BMP==imagetype){
		CxImageBMP *newima = new CxImageBMP;
		if (!newima)
			return false;
		newima->CopyInfo(*this);
		if (newima->Decode(hFile)) {
			Transfer(*newima);
			delete newima;
			return true;
		} else {
			strcpy(info.szLastError,newima->GetLastError());
			hFile->Seek(pos,SEEK_SET);
			delete newima;
			if (CXIMAGE_FORMAT_UNKNOWN!=imagetype)
				return false;
		}
	}
#endif
#if CXIMAGE_SUPPORT_PNG
	if (CXIMAGE_FORMAT_UNKNOWN==imagetype || CXIMAGE_FORMAT_PNG==imagetype){
		CxImagePNG *newima = new CxImagePNG;
		if (!newima)
			return false;
		newima->CopyInfo(*this);
		if (newima->Decode(hFile)) {
			Transfer(*newima);
			delete newima;
			return true;
		} else {
			strcpy(info.szLastError,newima->GetLastError());
			hFile->Seek(pos,SEEK_SET);
			delete newima;
			if (CXIMAGE_FORMAT_UNKNOWN!=imagetype)
				return false;
		}
	}
#endif

	strcpy(info.szLastError,"Decode: Unknown or wrong format");
	return false;
}
////////////////////////////////////////////////////////////////////////////////
#endif //CXIMAGE_SUPPORT_DECODE
////////////////////////////////////////////////////////////////////////////////
