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
#pragma hdrstop
#include "VclHelper.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// ascii settings format:
// <MainColumn>,<SortColumn>,<SortDirection>|<ColumnIndex>,<Visible>,<Position>,<Width>|...

string VTGetColumnSettings(TVirtualStringTree* Tree)
{
    string Settings;
    list<string> List;

    // add general header settings
    List.push_back(int_to_string(Tree->Header->MainColumn));
    List.push_back(int_to_string(Tree->Header->SortColumn));
    List.push_back(int_to_string(Tree->Header->SortDirection));
    Settings += string_join(List,",");

    // add settings of each column
    for(int i=0; i<Tree->Header->Columns->Count; i++) {
        TVirtualTreeColumn* Column = Tree->Header->Columns->Items[i];
        List.clear();
        List.push_back(int_to_string(i));
        List.push_back(int_to_string(Column->Options.Contains(coVisible) ? 1 : 0));
        List.push_back(int_to_string(Column->Position));
        List.push_back(int_to_string(Column->Width));
        Settings += string("|") + string_join(List,",");
    }

    return Settings;
}

bool VTSetColumnSettings(TVirtualStringTree* Tree, string Settings)
{
    list<string> Sections;
    list<string> List;
    list<string>::iterator sitr;
    list<string>::iterator litr;

    Sections = string_split(Settings,"|");

    if(Sections.empty())
        return false;

    // read general header settings
    sitr = Sections.begin();
    if(string_trim(*sitr) == "")
        return false;

    List = string_split(*sitr,",");
    if(List.size() < 3)
        return false;

    litr = List.begin();
    Tree->Header->MainColumn = string_to_int(*litr); ++litr;
    Tree->Header->SortColumn = string_to_int(*litr); ++litr;
    Tree->Header->SortDirection = (TSortDirection)string_to_int(*litr); ++litr;

    // read column settings
    for(++sitr; sitr != Sections.end(); ++sitr) {
        if(string_trim(*sitr) == "")
            continue;

        List = string_split(*sitr,",");
        if(List.size() < 4)
            continue;

        litr = List.begin();

        // column index
        if(!Tree->Header->Columns->IsValidColumn(string_to_int(*litr)))
            continue;
        TVirtualTreeColumn* Column = Tree->Header->Columns->Items[string_to_int(*litr)];
        ++litr;

        // visible
        if(string_to_int(*litr))
            Column->Options << coVisible;
        else
           Column->Options >> coVisible;
        ++litr;

        // position
        Column->Position = string_to_int(*litr);
        ++litr;

        // width
        Column->Width = string_to_int(*litr);
        ++litr;
    }

    // Temporarily change width of one column to force VT to recalculate everything
    Tree->Header->Columns->Items[0]->Width++;
    Tree->Header->Columns->Items[0]->Width--;

    return true;
}

#if 0
// convert TColor to string
string ColorToString(TColor Color)
{
    string ColorStr;
    unsigned int Col = Color;

    ColorStr = int_to_string(Col & 0xFF) + ",";
    ColorStr += int_to_string((Col >> 8) & 0xFF) + ",";
    ColorStr += int_to_string((Col >> 16) & 0xFF) + ",";
    ColorStr += int_to_string((Col >> 24) & 0xFF);

    return ColorStr;
}

// convert string to TColor value
TColor StringToColor(string ColorStr)
{
    unsigned int Col = 0;
    list<string> ColList = string_split(ColorStr,",");
    list<string>::iterator itr = ColList.begin();

    for(int i = 0; i < 4 && itr != ColList.end(); ++itr, i++)
        Col |= ((string_to_int(*itr) & 0xFF) << (i * 8));

    return (TColor)Col;
}
#endif

// convert TFont attributes to string (does not save color)
string FontToString(const TFont* Font)
{
    string FontStr;
    list<string> StyleList;

    if(Font->Style.Contains(fsBold)) StyleList.push_back("Bold");
    if(Font->Style.Contains(fsItalic)) StyleList.push_back("Italic");
    if(Font->Style.Contains(fsUnderline)) StyleList.push_back("Underline");
    if(Font->Style.Contains(fsStrikeOut)) StyleList.push_back("StrikeOut");

    FontStr = string(Font->Name.c_str()) + ",";
    FontStr += int_to_string(Font->Size) + ",";
    FontStr += string_join(StyleList,"|");

    return FontStr;
}

// creates TFont object from string, caller frees.
TFont* StringToFont(string FontStr)
{
    TFont* Font = new TFont();
    list<string> AttrList = string_split(FontStr,",");
    list<string>::iterator itr = AttrList.begin();

    if(itr == AttrList.end())
        return Font;
    Font->Name = string_trim(*itr).c_str();

    if((++itr) == AttrList.end())
        return Font;
    Font->Size = string_to_int(*itr);

    if((++itr) == AttrList.end())
        return Font;

    // decode styles
    list<string> StyleList = string_split(*itr,"|");
    TFontStyles Styles;
    for(list<string>::iterator si = StyleList.begin(); si != StyleList.end(); ++si) {
        if(*si == "Bold") Styles << fsBold;
        if(*si == "Italic") Styles << fsItalic;
        if(*si == "Underline") Styles << fsUnderline;
        if(*si == "StrikeOut") Styles << fsStrikeOut;
    }
    Font->Style = Styles;

    return Font;
}


