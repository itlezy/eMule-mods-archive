// $Id: misc_support.cpp,v 1.45 2009/09/02 09:07:13 nagilo Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug
// Copyright 2002 Thijmen Klok (thijmen@id3lib.org)

// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
// License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

// The id3lib authors encourage improvements and optimisations to be sent to
// the id3lib coordinator.  Please see the README file for details on where to
// send such submissions.  See the AUTHORS file for a list of people who have
// contributed to id3lib.  See the ChangeLog file for a list of changes to
// id3lib.  These files are distributed with id3lib at
// http://download.sourceforge.net/id3lib/

#include <ctype.h> //for isdigit()
#include <stdio.h>

#include "misc_support.h"
#include "id3/utils.h" // has <config.h> "id3/id3lib_streams.h" "id3/globals.h" "id3/id3lib_strings.h"

int ID3_strncasecmp (const char *s1, const char *s2, int n);
//using namespace dami;

char *ID3_GetString(const ID3_Frame *frame, ID3_FieldID fldName)
{
  char *text = NULL;
//  if (NULL != frame)
  ID3_Field* fld;
  if (NULL != frame && NULL != (fld = frame->GetField(fldName)))
  {
//    ID3_Field* fld = frame->GetField(fldName);
    ID3_TextEnc enc = fld->GetEncoding();
    fld->SetEncoding(ID3TE_ISO8859_1);
    size_t nText = fld->Size();
    text = LEAKTESTNEW(char[nText + 1]);
    fld->Get(text, nText + 1);
    fld->SetEncoding(enc);
  }
  return text;
}

char *ID3_GetString(const ID3_Frame *frame, ID3_FieldID fldName, size_t nIndex)
{
  char *text = NULL;
  if (NULL != frame)
  {
    size_t nText = frame->GetField(fldName)->Size();
    text = LEAKTESTNEW(char[nText + 1]);
    frame->GetField(fldName)->Get(text, nText + 1, nIndex);
  }
  return text;
}

void ID3_FreeString(char *str)
{
  if(str != NULL)
    delete [] str;
}

char *ID3_GetArtist(const ID3_Tag *tag)
{
  char *sArtist = NULL;
  if (NULL == tag)
  {
    return sArtist;
  }

  ID3_Frame *frame = NULL;
  if ((frame = tag->Find(ID3FID_LEADARTIST)) ||
      (frame = tag->Find(ID3FID_BAND))       ||
      (frame = tag->Find(ID3FID_CONDUCTOR))  ||
      (frame = tag->Find(ID3FID_COMPOSER)))
  {
    sArtist = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sArtist;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddArtist(ID3_Tag *tag, const char *text, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != text && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveArtists(tag);
    }
    if (replace ||
        (tag->Find(ID3FID_LEADARTIST) == NULL &&
         tag->Find(ID3FID_BAND)       == NULL &&
         tag->Find(ID3FID_CONDUCTOR)  == NULL &&
         tag->Find(ID3FID_COMPOSER)   == NULL))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_LEADARTIST));
      if (frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }
  return frame;
}

size_t ID3_RemoveArtists(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_LEADARTIST)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }
  while ((frame = tag->Find(ID3FID_BAND)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }
  while ((frame = tag->Find(ID3FID_CONDUCTOR)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }
  while ((frame = tag->Find(ID3FID_COMPOSER)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetAlbum(const ID3_Tag *tag)
{
	char *sAlbum = NULL;
  if (NULL == tag)
  {
    return sAlbum;
  }

  ID3_Frame *frame = tag->Find(ID3FID_ALBUM);
  if (frame != NULL)
  {
    sAlbum = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sAlbum;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddAlbum(ID3_Tag *tag, const char *text, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != text && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveAlbums(tag);
    }
    if (replace || tag->Find(ID3FID_ALBUM) == NULL)
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_ALBUM) );
      if (frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

size_t ID3_RemoveAlbums(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_ALBUM)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetTitle(const ID3_Tag *tag)
{
  char *sTitle = NULL;
  if (NULL == tag)
  {
    return sTitle;
  }

  ID3_Frame *frame = tag->Find(ID3FID_TITLE);
  if (frame != NULL)
  {
    sTitle = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sTitle;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddTitle(ID3_Tag *tag, const char *text, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != text && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveTitles(tag);
    }
    if (replace || tag->Find(ID3FID_TITLE) == NULL)
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_TITLE));
      if (frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

size_t ID3_RemoveTitles(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_TITLE)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetYear(const ID3_Tag *tag)
{
  char *sYear = NULL;
  if (NULL == tag)
  {
    return sYear;
  }

  ID3_Frame *frame = tag->Find(ID3FID_YEAR);
  if (frame != NULL)
  {
    sYear = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sYear;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddYear(ID3_Tag *tag, const char *text, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != text && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveYears(tag);
    }
    if (replace || tag->Find(ID3FID_YEAR) == NULL)
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_YEAR));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

size_t ID3_RemoveYears(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_YEAR)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetComment(const ID3_Tag *tag, const char* desc)
{
  char *comment = NULL;
  if (NULL == tag)
  {
    return comment;
  }

  ID3_Frame* frame = NULL;
  if (desc)
  {
    frame = tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, desc);
  }
  else
  {
    frame = tag->Find(ID3FID_COMMENT);
    if(frame == tag->Find(ID3FID_COMMENT, ID3FN_DESCRIPTION, STR_V1_COMMENT_DESC))
      frame = tag->Find(ID3FID_COMMENT);
  }

  if (frame)
    comment = ID3_GetString(frame, ID3FN_TEXT);
  return comment;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddComment(ID3_Tag *tag, const char *text, bool replace)
{
  return ID3_AddComment(tag, text, "", replace);
}

ID3_Frame* ID3_AddComment(ID3_Tag *tag, const char *text,
                          const char *desc, bool replace)
{
  return ID3_AddComment(tag, text, desc, "XXX", replace);
}

ID3_Frame* ID3_AddComment(ID3_Tag *tag, const char *text,
                          const char *desc, const char* lang, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag  &&
      NULL != text &&
      NULL != desc &&
      strlen(text) > 0)
  {
    bool bAdd = true;
    if (replace)
    {
      ID3_RemoveComments(tag, desc);
    }
    else
    {
      // See if there is already a comment with this description
      ID3_Tag::Iterator* iter = tag->CreateIterator();
      ID3_Frame* frame = NULL;
      while ((frame = iter->GetNext()) != NULL)
      {
        if (frame->GetID() == ID3FID_COMMENT)
        {
          char *tmp_desc = ID3_GetString(frame, ID3FN_DESCRIPTION);
          if (strcmp(tmp_desc, desc) == 0)
          {
            bAdd = false;
          }
          delete [] tmp_desc;
          if (!bAdd)
          {
            break;
          }
        }
      }
      delete iter;
    }
    if (bAdd)
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_COMMENT));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_LANGUAGE)->Set(lang);
        frame->GetField(ID3FN_DESCRIPTION)->Set(desc);
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }
  return frame;
}

// Remove all comments with the given description (remove all comments if
// desc is NULL)
size_t ID3_RemoveComments(ID3_Tag *tag, const char *desc)
{
  size_t num_removed = 0;

  if (NULL == tag)
  {
    return num_removed;
  }

  ID3_Tag::Iterator* iter = tag->CreateIterator();
  ID3_Frame* frame = NULL;
  while ((frame = iter->GetNext()) != NULL)
  {
    if (frame->GetID() == ID3FID_COMMENT)
    {
      bool remove = false;
      // A null description means remove all comments
      if (NULL == desc)
      {
        remove = true;
      }
      else
      {
        // See if the description we have matches the description of the
        // current comment.  If so, set the "remove the comment" flag to true.
        char *tmp_desc = ID3_GetString(frame, ID3FN_DESCRIPTION);
        remove = (strcmp(tmp_desc, desc) == 0);
        delete [] tmp_desc;
      }
      if (remove)
      {
        frame = tag->RemoveFrame(frame);
        delete frame;
        num_removed++;
      }
    }
  }
  delete iter;

  return num_removed;
}
#endif
char *ID3_GetTrack(const ID3_Tag *tag)
{
  char *sTrack = NULL;
  if (NULL == tag)
  {
    return sTrack;
  }

  ID3_Frame *frame = tag->Find(ID3FID_TRACKNUM);
  if (frame != NULL)
  {
    sTrack = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sTrack;
}

size_t ID3_GetTrackNum(const ID3_Tag *tag)
{
  char *sTrack = ID3_GetTrack(tag);
  size_t nTrack = 0;
  if (NULL != sTrack)
  {
    nTrack = atoi(sTrack);
    delete [] sTrack;
  }
  return nTrack;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddTrack(ID3_Tag *tag, uchar trk, uchar ttl, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && trk > 0)
  {
    if (replace)
    {
      ID3_RemoveTracks(tag);
    }
    if (replace || NULL == tag->Find(ID3FID_TRACKNUM))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_TRACKNUM));
      if (frame)
      {
        char *sTrack = NULL;
        if (0 == ttl)
        {
          sTrack = LEAKTESTNEW(char[4]);
          sprintf(sTrack, "%lu", (luint) trk);
        }
        else
        {
          sTrack = LEAKTESTNEW(char[8]);
          sprintf(sTrack, "%lu/%lu", (luint) trk, (luint) ttl);
        }

        frame->GetField(ID3FN_TEXT)->Set(sTrack);
        tag->AttachFrame(frame);

        delete [] sTrack;
      }
    }
  }
  return frame;
}

//following routine courtesy of John George
int ID3_GetPictureData(const ID3_Tag *tag, const char *TempPicPath)
{
  if (NULL == tag)
    return 0;
  else
  {
    ID3_Frame* frame = NULL;
    frame = tag->Find(ID3FID_PICTURE);
    if (frame != NULL)
    {
      ID3_Field* myField = frame->GetField(ID3FN_DATA);
      if (myField != NULL)
      {
        myField->ToFile(TempPicPath);
        return (int)myField->Size();
      }
      else return 0;
    }
    else return 0;
  }
}
#endif
//following routine courtesy of John George
char* ID3_GetPictureMimeType(const ID3_Tag *tag)
{
  char* sPicMimetype = NULL;
  if (NULL == tag)
    return sPicMimetype;

  ID3_Frame* frame = NULL;
  frame = tag->Find(ID3FID_PICTURE);
  if (frame != NULL)
  {
    sPicMimetype = ID3_GetString(frame, ID3FN_MIMETYPE);
  }
  return sPicMimetype;
}

//following routine courtesy of John George
bool ID3_HasPicture(const ID3_Tag* tag)
{
  if (NULL == tag)
    return false;
  else
  {
    ID3_Frame* frame = tag->Find(ID3FID_PICTURE);
    if (frame != NULL)
    {
      return frame->GetField(ID3FN_DATA) != NULL;
    }
    else return false;
  }
}
#ifdef ID3LIB_WRITE
//following routine courtesy of John George
ID3_Frame* ID3_AddPicture(ID3_Tag* tag, const char* TempPicPath, const char* MimeType, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag )
  {
    if (replace)
      ID3_RemovePictures(tag);
    if (replace || NULL == tag->Find(ID3FID_PICTURE))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_PICTURE));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_DATA)->FromFile(TempPicPath);
        frame->GetField(ID3FN_MIMETYPE)->Set(MimeType);
        tag->AttachFrame(frame);
      }
    }
  }
  return frame;
}

//following routine courtesy of John George
size_t ID3_RemovePictures(ID3_Tag* tag)
{
  size_t num_removed = 0;
  ID3_Frame* frame = NULL;

  if (NULL == tag)
    return num_removed;

  while ((frame = tag->Find(ID3FID_PICTURE)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }
  return num_removed;
}

//following routine courtesy of John George
size_t ID3_RemovePictureType(ID3_Tag* tag, ID3_PictureType pictype)
{
  size_t bremoved = 0;
  ID3_Frame* frame = NULL;

  if (NULL == tag)
    return bremoved;

  ID3_Tag::Iterator* iter = tag->CreateIterator();

  while (NULL != (frame = iter->GetNext()))
  {
    if (frame->GetID() == ID3FID_PICTURE)
    {
      if (frame->GetField(ID3FN_PICTURETYPE)->Get() == (uint32)pictype)
        break;
    }
  }
  delete iter;

  if (NULL != frame)
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    bremoved = 1;
  }
  return bremoved;
}

//following routine courtesy of John George
ID3_Frame* ID3_AddPicture(ID3_Tag *tag, const char *TempPicPath, const char *MimeType, ID3_PictureType pictype, const char* Description, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag )
  {
    if (replace)
      ID3_RemovePictureType(tag, pictype);
    if (replace || NULL == tag->Find(ID3FID_PICTURE, ID3FN_PICTURETYPE, (uint32)pictype))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_PICTURE));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_DATA)->FromFile(TempPicPath);
        frame->GetField(ID3FN_MIMETYPE)->Set(MimeType);
        frame->GetField(ID3FN_PICTURETYPE)->Set((uint32)pictype);
        frame->GetField(ID3FN_DESCRIPTION)->Set(Description);
        tag->AttachFrame(frame);
      }
    }
  }
  return frame;
}

//following routine courtesy of John George
size_t ID3_GetPictureDataOfPicType(ID3_Tag* tag, const char* TempPicPath, ID3_PictureType pictype)
{
  if (NULL == tag)
    return 0;
  else
  {
    ID3_Frame* frame = NULL;
    ID3_Tag::Iterator* iter = tag->CreateIterator();

    while (NULL != (frame = iter->GetNext() ))
    {
      if(frame->GetID() == ID3FID_PICTURE)
      {
        if(frame->GetField(ID3FN_PICTURETYPE)->Get() == (uint32)pictype)
          break;
      }
    }
    delete iter;

    if (frame != NULL)
    {
      ID3_Field* myField = frame->GetField(ID3FN_DATA);
      if (myField != NULL)
      {
        myField->ToFile(TempPicPath);
        return (size_t)myField->Size();
      }
      else return 0;
    }
    else return 0;
  }
}
#endif
//following routine courtesy of John George
char* ID3_GetMimeTypeOfPicType(ID3_Tag* tag, ID3_PictureType pictype)
{
  char* sPicMimetype = NULL;
  if (NULL == tag)
    return sPicMimetype;

  ID3_Frame* frame = NULL;
  ID3_Tag::Iterator* iter = tag->CreateIterator();

  while (NULL != (frame = iter->GetNext()))
  {
    if(frame->GetID() == ID3FID_PICTURE)
    {
      if(frame->GetField(ID3FN_PICTURETYPE)->Get() == (uint32)pictype)
        break;
    }
  }
  delete iter;

  if (frame != NULL)
  {
    sPicMimetype = ID3_GetString(frame, ID3FN_MIMETYPE);
  }
  return sPicMimetype;
}

//following routine courtesy of John George
char* ID3_GetDescriptionOfPicType(ID3_Tag* tag, ID3_PictureType pictype)
{
  char* sPicDescription = NULL;
  if (NULL == tag)
    return sPicDescription;

  ID3_Frame* frame = NULL;
  ID3_Tag::Iterator* iter = tag->CreateIterator();

  while (NULL != (frame = iter->GetNext()))
  {
    if(frame->GetID() == ID3FID_PICTURE)
    {
      if(frame->GetField(ID3FN_PICTURETYPE)->Get() == (uint32)pictype)
        break;
    }
  }
  delete iter;

  if (frame != NULL)
  {
    sPicDescription = ID3_GetString(frame, ID3FN_DESCRIPTION);
  }
  return sPicDescription;
}

#ifdef ID3LIB_WRITE
size_t ID3_RemoveTracks(ID3_Tag* tag)
{
  size_t num_removed = 0;
  ID3_Frame* frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_TRACKNUM)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetGenre(const ID3_Tag *tag)
{
  char *sGenre = NULL;
  if (NULL == tag)
  {
    return sGenre;
  }

  ID3_Frame *frame = tag->Find(ID3FID_CONTENTTYPE);
  if (frame != NULL)
  {
    sGenre = ID3_GetString(frame, ID3FN_TEXT);
  }

  return sGenre;
}

size_t ID3_GetGenreNum(const ID3_Tag *tag)
{
  char *sGenre = ID3_GetGenre(tag);
  size_t ulGenre = 0xFF;
  if (NULL == sGenre)
  {
    return ulGenre;
  }

  // If the genre string begins with "(ddd)", where "ddd" is a number, then
  // "ddd" is the genre number---get it
  if (sGenre[0] == '(')
  {
    char *pCur = &sGenre[1];
    while (isdigit(*pCur))
    {
      pCur++;
    }
    if (*pCur == ')')
    {
      // if the genre number is greater than 255, its invalid.
      ulGenre = dami::min(0xFF, atoi(&sGenre[1]));
    }
  }

  delete [] sGenre;
  return ulGenre;
}

size_t ID3_GetV1GenreNum(char* sGenre)
{
  dami::String tmpgenre;
  size_t ulGenre = 0xFF;
  uint32 iStart = 0;
  uint32 digits;

  if (strlen(sGenre) < 3)
    return ulGenre;

  // If the genre string begins with "(ddd)", where "ddd" is a number, then
  // "ddd" is the genre number---get it
  while (1)
  {
    digits = 0;
    if (sGenre[iStart] == '(')
    {
      char *pCur = &sGenre[iStart + 1];
      if (strlen(pCur) >= 3 && strncmp(pCur, "RX)", 3) == 0)
      {
        iStart += 4;
        tmpgenre.append("(RX)", 4);
        continue;
      }
      else if (strlen(pCur) >= 3 && strncmp(pCur, "CR)", 3) == 0)
      {
        iStart += 4;
        tmpgenre.append("(CR)", 4);
        continue;
      }
      if (*pCur == '\0')
        break; //allready at end of text, only text has "(\0"
      if (*pCur == '(')
      { // starts with "((" so rest is text
        sGenre += iStart;
        if (strlen(sGenre) > 0)
          tmpgenre.append(sGenre, strlen(sGenre));
        sGenre -= iStart;
        break;
      }
      if (*pCur == ')')//empty "()"
      {
        iStart += 2;
        continue; //ignore them, doesn't get copied in tmpgenre
      }
      if (isdigit(*pCur))
      {
        while (isdigit(*pCur))
        {
          pCur++;
          ++digits;
        }
        if (*pCur == ')')
        {
          ulGenre = atoi(&sGenre[iStart + 1]); //should be made to only check nr of digits long
          // it's invalid when bigger than the actual number of v1 genre's
          if (ulGenre >= ID3_NR_OF_V1_GENRES)
          {
            ulGenre = 0xFF;
            iStart += 2 + digits; //two for "(" and ")"
            continue; //try to find a better one
          }
          else
          { //copy the remainder of the text, we found a valid genre number
            sGenre += iStart + 2 + digits; //two for "(" and ")"
            if (strlen(sGenre) > 0)
              tmpgenre.append(sGenre, strlen(sGenre));
            sGenre -= iStart + 2 + digits;
            break;
          }
        }
        else //some gibberish like "(123BLAH
          ++iStart;
      }
      else //some gibberish like "(BLAH")
        ++iStart;
    }
    else if (sGenre[iStart] == '\0')
      break;
    else
    {
      tmpgenre.append(sGenre[iStart], 1);
      ++iStart;
    }
  }
  if (tmpgenre.size() > 0)
  {
     sprintf(sGenre, tmpgenre.c_str());
  }

  return ulGenre;
}

bool ID3_HasChars(char* text, char* chars)
{
  dami::String tmptext;
//  char* newtext = text;
  uint32 iStart = 0;

  while (strlen(text) >= strlen(chars))
  {
    if (strncmp(text, chars, strlen(chars)) == 0)
    { //found, copy 0 to iStart and iStart to remainder
      if (iStart > 0)
      {
        text -= iStart; //get the original string back
        tmptext.append(text, iStart);
      }
      if (strlen(text) > (iStart + strlen(chars)))
      { //copy the remainder
        text += strlen(chars) + iStart;
        tmptext.append(text, strlen(text));
        text -= strlen(chars) + iStart;
      }
      if (tmptext.size() > 0)
      {
        sprintf(text, tmptext.c_str());
      }
      else
        text[0] = '\0';
      return true;
    }
    else
    {
      ++text;
      ++iStart;
    }
  } ;
  //not found
  return false;
}

char* chartest(char** input)
{
//_CrtMemState* test1 = &s1;
//_CrtMemState** test2 = &test1;
//s1 = *test1;
//test1 = *test2;
  int iSize;
//  ID3_Frame* bla = LEAKTESTNEW(ID3_Frame, 0);
//  ID3_Frame* bli = LEAKTESTNEW(ID3_Frame, 0);
  char* tmpchars = "ba";
  iSize = sizeof(char);
  iSize = sizeof(ID3_Frame); // TODO reassigning value to iSize (???)
  char* genre = *input;
  input = &tmpchars;
  return genre;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddGenre(ID3_Tag* tag, size_t genreNum, char* genre, bool add_v1_genre_number, bool add_v1_genre_description, bool addRXorCR, bool replace)
{
/* This routine should work for the following (all examples):
 * genrenumbers can come within char* genre using (number), by genreNum, or if char* genre matches a v1 genrenumber
 * "(20)InvalidDescription" --> "(20)Alternative"
 * "(RX)(CR)(20)" --> "(RX)(CR)(20)Alternative"
 * "(24)(40)(20)" --> "(24)(40)(20)Soundtrack, AlternRock, Alternative"
 * "(RX)(CR)Alternative" --> "(RX)(CR)(20)Alternative"
 */
  size_t newGenreNum1 = 0xFF; //this is the one found in the text as (number)
  size_t newGenreNum2 = 0xFF; //this is the one found in the text by matching the string
  bool writeRX = false;
  bool writeCR = false;
  char* tmpgenre1 = LEAKTESTNEW(char[1024]);// = NULL;
  const char* tmpgenre = NULL;
  const char* remainder = NULL;
  dami::String* newgenre = LEAKTESTNEW(dami::String);
  int iCompare;

  if (add_v1_genre_number == false && add_v1_genre_description == false)
  { // you don't want me to do anything? Fine by me
    delete newgenre;
    delete [] tmpgenre1;
    return NULL;
  }
  if (genre != NULL && strlen(genre) == 0)
    genre = NULL;

  if (genre != NULL)
  { //see if there are references to v1 genre numbers in it, e.g. "(" + number + ")" + description
    if (strlen(genre) > 1023)
    {
      delete newgenre;
      delete [] tmpgenre1;
      return NULL;
    }
    sprintf(tmpgenre1, "%s", genre);

    newGenreNum1 = ID3_GetV1GenreNum(tmpgenre1); //get's the first and strips it
    if (strlen(tmpgenre1) != 0)
    { //see if there are references to v1 genre numbers in it, e.g. "(" + number + ")" + description
      writeRX = ID3_HasChars(tmpgenre1, "(RX)");
    }
    if (strlen(tmpgenre1) != 0)
    { //see if there are references to v1 genre numbers in it, e.g. "(" + number + ")" + description
      writeCR = ID3_HasChars(tmpgenre1, "(CR)");
    }
    // search for an additional genre number, from a match
    if (strlen(tmpgenre1) != 0) //try to find an additional number from the remaining genre
    {
      for (newGenreNum2 = 0; newGenreNum2 < ID3_NR_OF_V1_GENRES; ++newGenreNum2)
      {
        tmpgenre = ID3_V1GENRE2DESCRIPTION(newGenreNum2);
        // if tmpgenre = "Blues" and tmpgenre1 = "Blues, HardRock" they should match i accept a space, a comma and a ; as de;imters
        // if tmpgenre = "Pop" and tmpgenre1 = "Pop-Folk" they shouldn't match
        // if tmpgenre = "Jazz" and tmpgenre1 = "Jazz+Funk" they shouldn't match
        iCompare = ID3_strncasecmp(tmpgenre, tmpgenre1, 23); //biggest genre is "Contemporary Christian" which is 22 chars
        if (iCompare == 0) //perfect match, nothing remains
        { //strip it off
          tmpgenre1[0] = '\0';
          break;
        }
        else if (iCompare == -2 && (tmpgenre1[strlen(tmpgenre)] == ',' || tmpgenre1[strlen(tmpgenre)] == ' ' || tmpgenre1[strlen(tmpgenre)] == ';' )) //tmpgenre was earlier null terminated
        {
          iCompare = strlen(tmpgenre);
          tmpgenre1 += iCompare;
          remainder = tmpgenre1; //remainder now holds the remainder of the string
          tmpgenre1 -= iCompare; //reset for delete
          break;
        }
      } //for
      if ( newGenreNum2 == ID3_NR_OF_V1_GENRES) //no match
        remainder = tmpgenre1;
    }
  }
  // after breaking, now trying to rebuild things
  if (add_v1_genre_number)
  { //they want a genrenumber
    char* sGenre = LEAKTESTNEW(char[6]);
    size_t size;
    if (genreNum < ID3_NR_OF_V1_GENRES)
    {
      size = sprintf(sGenre, "(%lu)", (luint) genreNum);
      newgenre->append(sGenre, size);
    }
    if (newGenreNum1 < ID3_NR_OF_V1_GENRES && newGenreNum1 != genreNum)
    {
      size = sprintf(sGenre, "(%lu)", (luint) newGenreNum1);
      newgenre->append(sGenre, size);
    }
    if (newGenreNum2 < ID3_NR_OF_V1_GENRES && newGenreNum2 != genreNum && newGenreNum2 != newGenreNum1)
    {
      size = sprintf(sGenre, "(%lu)", (luint) newGenreNum2);
      newgenre->append(sGenre, size);
    }
    delete [] sGenre;
  }
  if (addRXorCR)
  { // they want CR or RX o be added, if there is
    if (writeRX)
      newgenre->append("(RX)", 4);
    if (writeCR)
      newgenre->append("(CR)", 4);
  }
  if (add_v1_genre_description)
  {
    if (genreNum < ID3_NR_OF_V1_GENRES)
    {
      tmpgenre = ID3_V1GENRE2DESCRIPTION(genreNum);
      newgenre->append(tmpgenre, strlen(tmpgenre));
    }
    if (newGenreNum1 < ID3_NR_OF_V1_GENRES && newGenreNum1 != genreNum)
    {
      if (genreNum < ID3_NR_OF_V1_GENRES) //also valid
        newgenre->append(", ", 2);
      tmpgenre = ID3_V1GENRE2DESCRIPTION(newGenreNum1);
      newgenre->append(tmpgenre, strlen(tmpgenre));
    }
    if (newGenreNum2 < ID3_NR_OF_V1_GENRES && newGenreNum2 != genreNum && newGenreNum2 != newGenreNum1)
    {
      if (genreNum < ID3_NR_OF_V1_GENRES || newGenreNum1 < ID3_NR_OF_V1_GENRES)
        newgenre->append(", ", 2);
      tmpgenre = ID3_V1GENRE2DESCRIPTION(newGenreNum2);
      newgenre->append(tmpgenre, strlen(tmpgenre));
    }
    // add any remaining text
    if (remainder != NULL)
      newgenre->append(remainder, strlen(remainder));
  }
  //clean memory
  if (newgenre->size() == 0)
  {
    delete newgenre;
    delete [] tmpgenre1;
    return NULL;
  }

  sprintf(tmpgenre1, newgenre->c_str());
  delete newgenre;
  ID3_Frame* newframe = ID3_AddGenre(tag, tmpgenre1, replace);
  delete [] tmpgenre1;
  return newframe;
}

//following routine courtesy of John George
ID3_Frame* ID3_AddGenre(ID3_Tag* tag, const char* genre, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != genre && strlen(genre) > 0)
  {
    if (replace)
    {
      ID3_RemoveGenres(tag);
    }
    if (replace || NULL == tag->Find(ID3FID_CONTENTTYPE))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_CONTENTTYPE));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(genre);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

ID3_Frame* ID3_AddGenre(ID3_Tag *tag, size_t genreNum, bool replace)
{
  if(0xFF != genreNum)
  {
    char sGenre[6];
    sprintf(sGenre, "(%lu)", (luint) genreNum);
    return(ID3_AddGenre(tag, sGenre, replace));
  }
  else
  {
    return(NULL);
  }
}

size_t ID3_RemoveGenres(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_CONTENTTYPE)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetLyrics(const ID3_Tag *tag)
{
  char *sLyrics = NULL;
  if (NULL == tag)
  {
    return sLyrics;
  }

  ID3_Frame *frame = tag->Find(ID3FID_UNSYNCEDLYRICS);
  if (frame != NULL)
  {
    sLyrics = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sLyrics;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddLyrics(ID3_Tag *tag, const char *text, bool replace)
{
  return ID3_AddLyrics(tag, text, "", replace);
}

ID3_Frame* ID3_AddLyrics(ID3_Tag *tag, const char *text, const char* desc,
                         bool replace)
{
  return ID3_AddLyrics(tag, text, desc, "XXX", replace);
}

ID3_Frame* ID3_AddLyrics(ID3_Tag *tag, const char *text, const char* desc,
                         const char* lang, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveLyrics(tag);
    }
    if (replace || tag->Find(ID3FID_UNSYNCEDLYRICS) == NULL)
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_UNSYNCEDLYRICS));
      if (NULL != frame)
      {
        frame->GetField(ID3FN_LANGUAGE)->Set(lang);
        frame->GetField(ID3FN_DESCRIPTION)->Set(desc);
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

size_t ID3_RemoveLyrics(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_UNSYNCEDLYRICS)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}
#endif
char *ID3_GetLyricist(const ID3_Tag *tag)
{
  char *sLyricist = NULL;
  if (NULL == tag)
  {
    return sLyricist;
  }

  ID3_Frame *frame = tag->Find(ID3FID_LYRICIST);
  if (frame != NULL)
  {
    sLyricist = ID3_GetString(frame, ID3FN_TEXT);
  }
  return sLyricist;
}
#ifdef ID3LIB_WRITE
ID3_Frame* ID3_AddLyricist(ID3_Tag *tag, const char *text, bool replace)
{
  ID3_Frame* frame = NULL;
  if (NULL != tag && NULL != text && strlen(text) > 0)
  {
    if (replace)
    {
      ID3_RemoveLyricist(tag);
    }
    if (replace || (tag->Find(ID3FID_LYRICIST) == NULL))
    {
      frame = LEAKTESTNEW( ID3_Frame(ID3FID_LYRICIST));
      if (frame)
      {
        frame->GetField(ID3FN_TEXT)->Set(text);
        tag->AttachFrame(frame);
      }
    }
  }

  return frame;
}

size_t ID3_RemoveLyricist(ID3_Tag *tag)
{
  size_t num_removed = 0;
  ID3_Frame *frame = NULL;

  if (NULL == tag)
  {
    return num_removed;
  }

  while ((frame = tag->Find(ID3FID_LYRICIST)))
  {
    frame = tag->RemoveFrame(frame);
    delete frame;
    num_removed++;
  }

  return num_removed;
}

ID3_Frame* ID3_AddSyncLyrics(ID3_Tag *tag, const uchar *data, size_t datasize,
                             ID3_TimeStampFormat format, bool replace)
{
  return ID3_AddSyncLyrics(tag, data, datasize, format, "", replace);
}

ID3_Frame* ID3_AddSyncLyrics(ID3_Tag *tag, const uchar *data, size_t datasize,
                             ID3_TimeStampFormat format, const char *desc,
                             bool replace)
{
  return ID3_AddSyncLyrics(tag, data, datasize, format, desc, "XXX", replace);
}

ID3_Frame* ID3_AddSyncLyrics(ID3_Tag *tag, const uchar *data, size_t datasize,
                             ID3_TimeStampFormat format, const char *desc,
                             const char *lang, bool replace)
{
  return ID3_AddSyncLyrics(tag, data, datasize, format, desc, lang,
                           ID3CT_LYRICS, replace);
}

ID3_Frame* ID3_AddSyncLyrics(ID3_Tag *tag, const uchar *data, size_t datasize,
                             ID3_TimeStampFormat format, const char *desc,
                             const char *lang, ID3_ContentType type,
                             bool replace)
{
  ID3_Frame* frame = NULL;
  // language and descriptor should be mandatory
  if ((NULL == lang) || (NULL == desc))
  {
    return NULL;
  }

  // check if a SYLT frame of this language or descriptor already exists
  ID3_Frame* frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang);
  if (!frmExist)
  {
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc);
  }

  if (NULL != tag && NULL != data)
  {
    if (replace && frmExist)
    {
      frmExist = tag->RemoveFrame (frmExist);
      delete frmExist;
      frmExist = NULL;
    }

    // if the frame still exist, cannot continue
    if (frmExist)
    {
      return NULL;
    }

    ID3_Frame* frame = LEAKTESTNEW( ID3_Frame(ID3FID_SYNCEDLYRICS));

    frame->GetField(ID3FN_LANGUAGE)->Set(lang);
    frame->GetField(ID3FN_DESCRIPTION)->Set(desc);
    frame->GetField(ID3FN_TIMESTAMPFORMAT)->Set(format);
    frame->GetField(ID3FN_CONTENTTYPE)->Set(type);
    frame->GetField(ID3FN_DATA)->Set(data, datasize);
    tag->AttachFrame(frame);
  }

  return frame;
}
#endif
ID3_Frame *ID3_GetSyncLyricsInfo(const ID3_Tag *tag, const char *desc,
                                 const char *lang,
                                 ID3_TimeStampFormat& format,
                                 ID3_ContentType& type, size_t& size)
{
  // check if a SYLT frame of this language or descriptor exists
  ID3_Frame* frmExist = NULL;
  if (NULL != lang)
  {
    // search through language
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang);
  }
  else if (NULL != desc)
  {
    // search through descriptor
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc);
  }
  else
  {
    // both language and description not specified, search the first SYLT frame
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS);
  }

  if (!frmExist)
  {
    return NULL;
  }

  // get the lyrics time stamp format
  format = static_cast<ID3_TimeStampFormat>(frmExist->GetField(ID3FN_TIMESTAMPFORMAT)->Get ());

  // get the lyrics content type
  type = static_cast<ID3_ContentType>(frmExist->GetField(ID3FN_CONTENTTYPE)->Get ());

  // get the lyrics size
  size = frmExist->GetField (ID3FN_DATA)->Size ();

  // return the frame pointer for further uses
  return frmExist;
}

ID3_Frame *ID3_GetSyncLyrics(const ID3_Tag* tag, const char* lang,
                             const char* desc, const uchar* &pData, size_t& size)
{
  // check if a SYLT frame of this language or descriptor exists
  ID3_Frame* frmExist = NULL;
  if (NULL != lang)
  {
    // search through language
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_LANGUAGE, lang);
  }
  else if (NULL != desc)
  {
    // search through descriptor
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS, ID3FN_DESCRIPTION, desc);
  }
  else
  {
    // both language and description not specified, search the first SYLT frame
    frmExist = tag->Find(ID3FID_SYNCEDLYRICS);
  }

  if (NULL == frmExist)
  {
    return NULL;
  }

  // get the lyrics size
  size = dami::min(size, frmExist->GetField(ID3FN_DATA)->Size());

  // get the lyrics data
  pData = frmExist->GetField (ID3FN_DATA)->GetRawBinary();

  // return the frame pointer for further uses
  return frmExist;
}

