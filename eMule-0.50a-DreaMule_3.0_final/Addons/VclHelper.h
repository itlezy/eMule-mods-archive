/*
This file is part of KCeasy (http://www.kceasy.com)
Copyright (C) 2002-2004 Markus Kern <mkern@kceasy.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
//---------------------------------------------------------------------------
#ifndef VclHelperH
#define VclHelperH
//---------------------------------------------------------------------------
#include <Graphics.hpp>

#include "VirtualTrees.hpp"
#include "istring.h"
//---------------------------------------------------------------------------

using namespace KCeasyEngine;

// get virtual tree column settings as text string
string VTGetColumnSettings(TVirtualStringTree* Tree);
// set virtual tree column settings from text string
bool VTSetColumnSettings(TVirtualStringTree* Tree, string Settings);

#if 0
// convert TColor to string
string ColorToString(TColor Color);
// convert string to TColor value
TColor StringToColor(string ColorStr);
#endif

// convert TFont attributes to string (does not save color)
string FontToString(const TFont* Font);
// creates TFont object from string, caller frees.
TFont* StringToFont(string FontStr);

#endif
