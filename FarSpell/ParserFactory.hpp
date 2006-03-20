/*
    Factory of parser for FarSpell plugin: header.
    Copyright (c) Sergey Shishmintzev, Kiev 2005-2006

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contributor(s):
      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
      Hunspell team http://hunspell.sf.net/ or http://OpenOffice.org/
*/

#pragma once
#include <FARPlus/FARString.h>
#include <parsers/textparser.hxx>

#define hunspell_free free
#define hunspell_malloc malloc

class ParserInstance
{
  friend class ParserFactory;
  public:
    enum {
      MaxLineLength = MAXLNLEN
    };
    void put_line(char *line);
    char *next_token();
    int get_tokenpos();
    ~ParserInstance();
  private:
    int last_token_skip;
    FarString middle_chars;
    ParserInstance(TextParser *hunspell_parser);
    TextParser *parser;
};

class ParserFactory
{
  public:
    typedef (*SpellFactoryEnumProc)(const char* id, const char* name, void* param);
    static void EnumParsers(SpellFactoryEnumProc proc, void* param);
    static const char* GetParserName(const char *parser_id);
    static ParserInstance* CreateParserInstance(const char *parser_id, int encoding, const FarStringW &wordchars, const FarFileName &file_name);
    static ParserInstance* CreateParserInstance(int parser_id, int encoding, const FarStringW &wordchars, const FarFileName &file_name);
  private:
    enum { FMT_AUTO_TEXT, FMT_TEXT, FMT_LATEX, FMT_HTML, FMT_MAN, FMT_FIRST };
    static char * HunspellParser[];
    static int MsgIds[];
    static ParserInstance * newHunspellParser(const char *wordchars, int format, const char *extension);
};

