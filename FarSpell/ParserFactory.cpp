/*
    Factory of parser for FarSpell plugin: implemetation.
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

#include "ParserFactory.hpp"

#include "FARPlus/FARPlus.h"

#include "parsers/textparser.hxx"
#include "parsers/htmlparser.hxx"
#include "parsers/latexparser.hxx"
#include "parsers/manparser.hxx"
#include "parsers/firstparser.hxx"

// Message ids:
#include "FarSpell.h"


ParserInstance::ParserInstance(TextParser *hunspell_parser) 
{
  far_assert(hunspell_parser);
  parser = hunspell_parser;
  middle_chars = "-'";
  last_token_skip = 0;
}

ParserInstance::~ParserInstance() 
{
  if (parser) {
    delete parser;
    parser = NULL;
  }
}

void ParserInstance::put_line(char *line)
{
  far_assert(parser);
  parser->put_line(line);
}

char *ParserInstance::next_token()
{
  far_assert(parser);
  char *token = NULL; 
  char *skip_begin, *skip_end;
  last_token_skip = 0;
  for (;;) {
    if (token) hunspell_free(token);
    token = parser->next_token();     
    if (!token) return NULL;  // все слова кончились
    far_assert(*token);
    // пропустим все спец-символы в начале слова:
    for (skip_begin = token; 
         *skip_begin && middle_chars.IndexOf(*skip_begin)>=0; 
         skip_begin++);
    if (!*skip_begin) continue; // группа спец-символов - это не слово...

    // опустим все спец-символы в конце слова:
    for (skip_end = token+strlen(token)-1; 
         skip_end!=token && middle_chars.IndexOf(*skip_end)>=0; 
         skip_end--);
    *(skip_end+1) = '\0'; // обрежем спец-символы в конце слова.
    far_assert(skip_begin <= skip_end);
    // вначале слова ничего не пропустили, поэтому вернём адрес
    // который может и должен быть освобождён hunspell_free.
    if (skip_begin == token) 
      return token;
    // увы, придётся выделить память заново, чтобы вызывающая
    // процедура могла освободить слово с помощью hunspell_free.
    int len = skip_end-skip_begin+1;
    far_assert(len!=0); // это надо отловить и исправить.
    far_assert(len>0); 
    char *new_token = (char*)hunspell_malloc((len+1)*sizeof(char));
    memcpy(new_token, skip_begin, (len+1)*sizeof(char));
    far_assert(*(new_token+len) == '\0');  //*(new_token+len) = '\0';
    hunspell_free(token);
    last_token_skip = skip_begin-token; // TextParser не знает, 
                                        // что мы обрезали слово
    return new_token;
  }
}

int ParserInstance::get_tokenpos()
{
  far_assert(parser);
  far_assert(last_token_skip>=0);
  return parser->get_tokenpos() + last_token_skip;
}

/*****************************************************************************
 * ParserFactory                                                             
 *****************************************************************************/ 

char * ParserFactory::HunspellParser[] = 
{
  "*", 
  "hunspell:TEXT", 
  "hunspell:LATEX", 
  "hunspell:HTML", 
  "hunspell:MAN", 
  "hunspell:FIRST",
  0
};

int ParserFactory::MsgIds[] =
{
  M_FMT_AUTO_TEXT,
  M_FMT_TEXT,
  M_FMT_LATEX,
  M_FMT_HTML,
  M_FMT_MAN,
  M_FMT_FIRST,
};

void ParserFactory::EnumParsers(SpellFactoryEnumProc proc, void* param)
{
  for (char **i = HunspellParser; *i; i++)
    if (!proc(*i, Far::GetMsg(MsgIds[i-HunspellParser]), param))
      return;
}

const char* ParserFactory::GetParserName(const char *parser_id)
{
  for (char **i = HunspellParser; *i; i++)
    if (strcmp(*i, parser_id)==0)
      return Far::GetMsg(MsgIds[i-HunspellParser]);
  return Far::GetMsg(MsgIds[FMT_AUTO_TEXT]);
}

ParserInstance* ParserFactory::CreateParserInstance(const char *parser_id, int encoding, const FarStringW &wordchars, const FarFileName &file_name)
{
  for (char **i = HunspellParser; *i; i++)
    if (strcmp(*i, parser_id)==0)
      return CreateParserInstance(i-HunspellParser, encoding, wordchars, file_name);
  return CreateParserInstance(FMT_AUTO_TEXT, encoding, wordchars, file_name);
}

ParserInstance* ParserFactory::CreateParserInstance(int parser_id, int encoding, const FarStringW &wordchars, const FarFileName &file_name)
{
  far_assert(parser_id>=FMT_AUTO_TEXT);
  far_assert(parser_id<=FMT_FIRST);
  FarString wordchars_ascii;
  ToAscii(encoding, wordchars, wordchars_ascii);
  for(int i=wordchars_ascii.Length()-1;  i>=0; i--)
    if (wordchars_ascii[i]=='?') wordchars_ascii.Delete(i);
  return newHunspellParser(wordchars_ascii.c_str(), parser_id, file_name.GetExt().c_str());
}

ParserInstance * ParserFactory::newHunspellParser(const char *wordchars, int format, const char *extension)
// from hunspell-1.1.2/src/tools/hunspell.cxx
{
    TextParser *p = NULL;
    switch (format)
    {
      case FMT_TEXT: p = new TextParser(wordchars); break;
      case FMT_LATEX: p = new LaTeXParser(wordchars); break;
      case FMT_HTML: p = new HTMLParser(wordchars); break;
      case FMT_MAN: p = new ManParser(wordchars); break;
      case FMT_FIRST: p = new FirstParser(wordchars);
    }

    if ((!p) && (extension)) {
  if ((strcmp(extension, ".html") == 0) ||
          (strcmp(extension, ".htm") == 0) ||
          (strcmp(extension, ".xhtml") == 0) ||
          (strcmp(extension, ".docbook") == 0) ||
          (strcmp(extension, ".sgml") == 0) ||
      (strcmp(extension, ".xml") == 0)) {
          p = new HTMLParser(wordchars);
  } else if (((extension[0] > '0') && (extension[0] <= '9'))) {
          p = new ManParser(wordchars);
  } else if ((strcmp(extension, ".tex") == 0)) {
          p = new LaTeXParser(wordchars);
  }
    }

    if (!p)
      p = new TextParser(wordchars);
    return new ParserInstance(p);
}

const char* ParserFactory::GetDefaultParser()
{
  return HunspellParser[FMT_TEXT];
}