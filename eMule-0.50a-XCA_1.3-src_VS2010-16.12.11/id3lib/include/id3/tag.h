// -*- C++ -*-
// $Id: tag.h,v 1.64 2009/09/04 14:21:18 nagilo Exp $

// id3lib: a software library for creating and manipulating id3v1/v2 tags
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

#ifndef _ID3LIB_TAG_H_
#define _ID3LIB_TAG_H_

#if defined(__BORLANDC__)
// due to a bug in borland it sometimes still wants mfc compatibility even when you disable it
#  if defined(_MSC_VER)
#    undef _MSC_VER
#  endif
#  if defined(__MFC_COMPAT__)
#    undef __MFC_COMPAT__
#  endif
#endif

#include <id3/id3lib_frame.h>
#include <id3/field.h>

class ID3_Reader;
#ifdef ID3LIB_WRITE
class ID3_Writer;
#endif
class ID3_TagImpl;
class ID3_Tag;

class ID3_CPP_EXPORT ID3_Tag
{
  ID3_TagImpl* _impl;
public:

  class Iterator
  {
  public:
    virtual ID3_Frame*       GetNext()       = 0;
    virtual ~Iterator() {};
  };

  class ConstIterator
  {
  public:
    virtual const ID3_Frame* GetNext()       = 0;
    virtual ~ConstIterator() {};
  };

public:

  ID3_Tag(const char *name = NULL, flags_t = (flags_t) ID3TT_ALL);
  ID3_Tag(const ID3_Tag &tag);
  virtual ~ID3_Tag();

  void       Clear();
#ifdef ID3LIB_WRITE
  bool       HasChanged() const;
#endif
  size_t     Size() const;

#ifdef ID3LIB_WRITE
  bool       SetUnsync(bool);
  bool       SetExtendedHeader(bool);
  bool       SetExperimental(bool);
#else
  void       SetUnsync(bool);
  void       SetExtendedHeader(bool);
  void       SetExperimental(bool);
#endif

  bool       GetUnsync() const;
  bool       GetExtendedHeader() const;
  bool       GetExperimental() const;

#ifdef ID3LIB_WRITE
  bool       SetPadding(bool);

  void       AddFrame(const ID3_Frame&);
  void       AddFrame(const ID3_Frame*);
#else
  void       SetPadding(bool);
#endif
  bool       AttachFrame(ID3_Frame*);
  ID3_Frame* RemoveFrame(const ID3_Frame *);

  size_t     Parse(const uchar*, size_t);
  bool       Parse(ID3_Reader& reader);
#ifdef ID3LIB_WRITE
  size_t     Render(uchar*, ID3_TagType = ID3TT_ID3V2) const;
  size_t     Render(ID3_Writer&, ID3_TagType = ID3TT_ID3V2) const;
#endif
  size_t     Link(const char *fileInfo, flags_t = (flags_t) ID3TT_ALL);
  size_t     Link(ID3_Reader &reader, flags_t = (flags_t) ID3TT_ALL);

#ifdef ID3LIB_WRITE
  flags_t    Update(flags_t = (flags_t) ID3TT_ALL);
  flags_t    Strip(flags_t = (flags_t) ID3TT_ALL);
#endif

  size_t     GetPrependedBytes() const;
  size_t     GetAppendedBytes() const;
  size_t     GetFileSize() const;
  const char* GetFileName() const;
  ID3_Err    GetLastError();

  ID3_Frame* Find(ID3_FrameID) const;
  ID3_Frame* Find(ID3_FrameID, ID3_FieldID, uint32) const;
  ID3_Frame* Find(ID3_FrameID, ID3_FieldID, const char*) const;
  ID3_Frame* Find(ID3_FrameID, ID3_FieldID, const unicode_t*) const;

  size_t     NumFrames() const;

  const Mp3_Headerinfo* GetMp3HeaderInfo() const;

  Iterator*  CreateIterator();
  ConstIterator* CreateIterator() const;

  ID3_Tag&   operator=( const ID3_Tag & );

  bool       HasTagType(ID3_TagType tt) const;
  ID3_V2Spec GetSpec() const;
#ifdef ID3LIB_WRITE
  bool       SetSpec(ID3_V2Spec);
#else
  void       SetSpec(ID3_V2Spec);
#endif

  static size_t IsV2Tag(const uchar*);
  static size_t IsV2Tag(ID3_Reader&);

  /* Deprecated! */
  //void       AddNewFrame(ID3_Frame* f);
  size_t     Link(const char *fileInfo, bool parseID3v1, bool parseLyrics3);
  void       SetCompression(bool);
#ifdef ID3LIB_WRITE
  void       AddFrames(const ID3_Frame *, size_t);
#endif
  bool       HasLyrics() const;
  bool       HasV2Tag()  const;
  bool       HasV1Tag()  const;
  size_t     Parse(const uchar header[ID3_TAGHEADERSIZE], const uchar *buffer);
  //ID3_Frame* operator[](size_t) const;
  //ID3_Frame* GetFrameNum(size_t) const;

#ifdef ID3LIB_WRITE
  ID3_Tag&   operator<<(const ID3_Frame &);
  ID3_Tag&   operator<<(const ID3_Frame *);
#endif
};

// deprecated!
//off_t ID3_C_EXPORT ID3_IsTagHeader(const uchar header[ID3_TAGHEADERSIZE]);


#endif /* _ID3LIB_TAG_H_ */

