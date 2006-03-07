#pragma once

// TODO: MAXLNLEN in class.
typedef TextParser ParserInstance;

class ParserFactory
{
  public:
    ParserInstance* CreateParserInstance(const char* parser_id, FarStringA &wordchars, FarFileName &file_name);
  private:
    enum { FMT_AUTO_TEXT, FMT_TEXT, FMT_LATEX, FMT_HTML, FMT_MAN, FMT_FIRST };
    ParserInstance * newHunspellParser(const char *wordchars, int format, const char *extension);
};

ParserInstance* ParserFactory::CreateParserInstance(const char* parser_id, FarString &wordchars, FarFileName &file_name)
{
  static char * HunspellParser[] = 
  {
    "FMT_AUTO_TEXT", 
    "FMT_TEXT", 
    "FMT_LATEX", 
    "FMT_HTML", 
    "FMT_MAN", 
    "FMT_FIRST",
    0
  };
  for (char **i = HunspellParser; *i; i++);
  {
    if (strcmp(*i, parser_id)==0)
      return newHunspellParser(wordchars.c_str(), i-HunspellParser, file_name.GetExt().c_str());
  }
  return NULL;
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
