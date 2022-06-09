// -*- C++ -*-
// $Id: header.h,v 1.4 2002/09/13 15:37:23 t1mpy Exp $

// id3lib: a C++ library for creating and manipulating id3v1/v2 tags
// Copyright 1999, 2000  Scott Thomas Haug
// Copyright 2002  Thijmen Klok (thijmen@id3lib.org)

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

#ifndef _ID3LIB_HEADER_H_
#define _ID3LIB_HEADER_H_

#include "id3/globals.h" //has <stdlib.h> "id3/sized_types.h"
#include "flags.h"

class ID3_Reader;
#ifdef ID3LIB_WRITE
class ID3_Writer;
#endif

class ID3_Header
{
public:
  struct Info
  {
    uchar      frame_bytes_id;
    uchar      frame_bytes_size;
    uchar      frame_bytes_flags;
    bool       is_extended;
    size_t     extended_bytes; //including the extended header, so everything!
    bool       is_experimental;
  };

  ID3_Header()
    : _spec (ID3V2_UNKNOWN),
      _data_size (0)
#ifdef ID3LIB_WRITE
	  ,_changed (false)
#endif
  {
    this->Clear();
  }
  virtual ~ID3_Header() { ; }

  virtual bool       SetSpec(ID3_V2Spec);
  /*   */ ID3_V2Spec GetSpec() const { return _spec; }

#ifdef ID3LIB_WRITE
  /*   */ bool       SetDataSize(size_t size)
  {
    bool changed = size != _data_size;
    _changed = _changed || changed;
    _data_size = size;
    return changed;
  }
#else
  /*   */ void       SetDataSize(size_t size)
  {
    _data_size = size;
  }
#endif
  /*   */ size_t     GetDataSize() const { return _data_size; }
#ifdef ID3LIB_WRITE
  virtual bool       Clear()
  {
    bool changed = this->SetDataSize(0);
    if (this->GetSpec() == ID3V2_UNKNOWN)
    {
      this->SetSpec(ID3V2_LATEST);
      changed = true;
    }
    changed = _flags.clear() || changed;
    _changed = changed || _changed;
    return changed;
  }
#else
  virtual void       Clear()
  {
	  this->SetDataSize(0);
	  if (this->GetSpec() == ID3V2_UNKNOWN)
	  {
		  this->SetSpec(ID3V2_LATEST);
	  }
	  _flags.clear();
  }
#endif
  virtual size_t     Size() const = 0;

#ifdef ID3LIB_WRITE
  virtual ID3_Err    Render(ID3_Writer&) const = 0;
#endif
  virtual bool       Parse(ID3_Reader&) = 0;

  ID3_Header &operator=( const ID3_Header &rhs)
  {
    if (this != &rhs)
    {
      this->SetSpec(rhs.GetSpec());
      this->SetDataSize(rhs.GetSpec());
      this->_flags = rhs._flags;
    }
    return *this;
  }

protected:
  ID3_V2Spec      _spec;             // which version of the spec
  size_t          _data_size;        // how big is the data?
  ID3_Flags       _flags;            // header flags
  Info*     _info;             // header info w.r.t. id3v2 spec
#ifdef ID3LIB_WRITE
  bool            _changed;          // has the header changed since parsing
#endif
}
;

#endif /* _ID3LIB_HEADER_H */
