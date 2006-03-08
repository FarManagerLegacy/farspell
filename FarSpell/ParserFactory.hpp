#pragma once

// TODO: MAXLNLEN in class.
typedef TextParser ParserInstance;

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
  return newHunspellParser(wordchars_ascii.c_str(), parser_id, file_name.GetExt().c_str());
}

ParserInstance * ParserFactory::newHunspellParser(const char *wordchars, int format, const char *extension)
// from hunspell-1.1.2/src/tools/hunspell.cxx
{
    ParserInstance * p = NULL;
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
    return p;
}
