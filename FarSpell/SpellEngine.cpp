/*
    Spell-checker plugin for FAR: abstract spell engine.
    Copyright (c) Sergey Shishmintzev, Kiev 2006

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
*/

#include <limits.h>
#include "FarSpell.hpp"

SimpleSpellEngine::SimpleSpellEngine(int _document_encoding, int _spell_color)
: SpellEngine(_document_encoding)
{
  spell_color = _spell_color;
  _dict_instance = NULL;
  _parser_instance = NULL;
  dictionary_encoding = -1;
}

SimpleSpellEngine::~SimpleSpellEngine()
{
  DropEngine();
}

bool SimpleSpellEngine::CheckDictionary()
{
  return FarSpellEditor::editors->spell_factory.DictionaryExists(dict);
}

void SimpleSpellEngine::RecreateEngine(int what)
{
  if (what&(RS_DICT))
  {
    DropEngine(RS_DICT);
    if (CheckDictionary())
    {
      _dict_instance = FarSpellEditor::editors->spell_factory.GetDictInstance(dict);
      if (_dict_instance)
      {
        FarString cs = _dict_instance->get_dic_encoding();
        dictionary_encoding = FarSpellEditor::editors->GetCharsetEncoding(cs);
      }
    }
  }
  if (what&(RS_PARSER) && document_encoding!=-1 && GetDict())
  {
    DropEngine(RS_PARSER);
    _parser_instance = ParserFactory::CreateParserInstance(
       parser_id.c_str(), 
       document_encoding, _dict_instance->GetWordChars(), 
       file_name.c_str()); 
  }
}

void SimpleSpellEngine::DropEngine(int what)
{
  if (what&(RS_DICT))
    _dict_instance = NULL;

  if ((what&(RS_PARSER) && _parser_instance) || !_dict_instance)
  {
    delete _parser_instance;
    _parser_instance = NULL;
  }
}


SpellInstance* SimpleSpellEngine::GetDict()
{
  if (!_dict_instance) RecreateEngine(RS_DICT);
  return _dict_instance;
}

ParserInstance* SimpleSpellEngine::GetParser()
{
  if (!_parser_instance) RecreateEngine(RS_PARSER);
  return _parser_instance;
}


void SimpleSpellEngine::SetDocumentEncoding(int new_document_encoding)
{
  if (document_encoding != new_document_encoding)
  {
    DropEngine(RS_PARSER);
    document_encoding = new_document_encoding;
  }
}

void SimpleSpellEngine::SetDocumentFileSource(const FarString &new_file_name)
{
  DropEngine(RS_PARSER);
  file_name = new_file_name;
}

void SimpleSpellEngine::SetDictionary(const FarString &new_dict)
{
  if (new_dict != dict)
  {
    DropEngine(RS_DICT);
    dict = new_dict;
  }
}

void SimpleSpellEngine::PutLine(const FarString &line)
{
  ParserInstance *parser = GetParser();
  if (!parser) 
    return;
  if (line.Length()<ParserInstance::MaxLineLength) 
    parser->put_line(const_cast<char *>(line.c_str()));
  else {
    FarString truncated = line.Left(ParserInstance::MaxLineLength);
    parser->put_line(const_cast<char *>(truncated.c_str()));
  }
}

bool SimpleSpellEngine::NextToken(Token *token)
{
  ParserInstance *parser = GetParser();
  SpellInstance *dict = GetDict();
  if (!parser || !dict) 
    return false;
  char *doc_token = parser->next_token();
  if (!doc_token)
    return false;
  if (!*doc_token)
  { // sanity check
    hunspell_free(doc_token);
    return false;
  }
  const size_t tok_len = strlen(doc_token);
  FarStringW token_wide;
  ToUnicode(document_encoding, doc_token, token_wide);
  token->begin = parser->get_tokenpos();
  token->end = token->begin + tok_len;
  token->color = dict->Check(token_wide)? default_color: spell_color;
  token->A.SetText(doc_token, tok_len);
  token->W = token_wide;
  hunspell_free(doc_token);
  return true;
}

bool SimpleSpellEngine::TokenAt(int pos, Token *token)
{
  ParserInstance *parser = GetParser();
  SpellInstance *dict = GetDict();
  if (!parser || !dict) 
    return false;
  char *doc_token = NULL;
  FarStringW token_wide;
  int tpos, tlen;
  while (doc_token = parser->next_token())
  {
    ToUnicode(document_encoding, doc_token, token_wide);
    tpos = parser->get_tokenpos();
    tlen = strlen(doc_token);
    if (pos>=tpos && pos<(tpos+tlen))
      break;
    hunspell_free(doc_token);
    doc_token = NULL;
  }
  if (doc_token)
  {
    token->begin = tpos;
    token->end = token->begin + tlen;
    token->color = dict->Check(token_wide)? default_color: spell_color;
    token->A.SetText(doc_token, tlen);
    token->W = token_wide;
    hunspell_free(doc_token);
    return true;
  } else
    return false;
}

SpellEngine::WordList *SimpleSpellEngine::Suggestion(const Token &token)
{
  SpellInstance *dict = GetDict();
  if (!dict) 
    return NULL;
  return new SimpleWordList(dict->Suggest(token.W));
}

SpellEngine::WordList *SimpleSpellEngine::Suggestion(const FarStringW &word)
{
  SpellInstance *dict = GetDict();
  if (!dict) 
    return NULL;
  return new SimpleWordList(dict->Suggest(word));
}

void SimpleSpellEngine::FreeLotOfMemory()
{
  DropEngine();
}


VirtalSpellEngine::VirtalSpellEngine(int _document_encoding)
: SpellEngine(_document_encoding)
, _parser_instances()
{
  //spell_color = _spell_color;
  _dict_view_instance = NULL;
}

VirtalSpellEngine::~VirtalSpellEngine()
{
  DropEngine();
}

void VirtalSpellEngine::RecreateEngine(int what)
{
  if (what&RS_DICT)
  {
    DropEngine(RS_DICT);
    _dict_view_instance = FarSpellEditor::editors->dict_view_factory.GetDictView(dict_view);
    if (_dict_view_instance && !CheckDictionary())
    {
      delete _dict_view_instance;
      _dict_view_instance = NULL;
    }
  }
  if (what&RS_PARSER && document_encoding!=-1 && GetDict())
  {
    DropEngine(RS_PARSER);
    for (int i = 0; i<_dict_view_instance->DictCount(); i++)
    {
      FarStringW wc;
      _dict_view_instance->GetDict(i)->GetWordChars(wc);
      ParserInstance *_parser_instance = ParserFactory::CreateParserInstance(
        parser_id.c_str(), 
        document_encoding, wc, 
        file_name.c_str()); 
      _parser_instances.Add(_parser_instance);
      far_assert(_parser_instances.Count() == i + 1);
      far_assert(_parser_instances[i] == _parser_instance);
    }
  }
}

void VirtalSpellEngine::DropEngine(int what)
{
  if (what&RS_DICT && _dict_view_instance)
  {
    delete _dict_view_instance;
    _dict_view_instance = NULL;
  }

  if (what&RS_PARSER || !_dict_view_instance)
  {
    _parser_instances.Clear();
  }
}


DictViewInstance* VirtalSpellEngine::GetDict()
{
  if (!_dict_view_instance) 
    RecreateEngine(RS_DICT);
  return _dict_view_instance;
}

VirtalSpellEngine::parser_instances_t &VirtalSpellEngine::GetParsers()
{
  if (!_parser_instances.Count())
    RecreateEngine(RS_PARSER);
  return _parser_instances;
}

void VirtalSpellEngine::SetDocumentFileSource(const FarString &_file_name)
{
  file_name = _file_name;
}

void VirtalSpellEngine::SetDocumentEncoding(int new_document_encoding)
{
  if (document_encoding != new_document_encoding)
  {
    DropEngine(RS_PARSER);
    document_encoding = new_document_encoding;
  }
}

void VirtalSpellEngine::SetDictionary(const FarString &_dict_view)
{
  if (_dict_view != dict_view)
  {
    DropEngine(RS_DICT);
    dict_view = _dict_view;
  }
}

void VirtalSpellEngine::PutLine(const FarString &_line)
{
  parser_instances_t &parsers = GetParsers();
  if (!parsers.Count()) return;
  FarString line(_line);
  if (line.Length()>ParserInstance::MaxLineLength) 
    line.Delete(ParserInstance::MaxLineLength, INT_MAX);
  for (int i = 0; i<parsers.Count(); i++)
  {
    ParserInstance *parser = parsers[i];
    far_assert(parser);
    parser->put_line(const_cast<char *>(line.c_str()));
  }
}

bool VirtalSpellEngine::NextToken(Token *token)
{
  DictViewInstance *dict_view_instance = GetDict();
  if (!dict_view_instance) 
    return false;
  bool result = false;
  parser_instances_t &parsers = GetParsers();
  FarDataArray<char *> tokens;
  FarDataArray<int> tokens_at;
  for (int i = 0; i<dict_view_instance->DictCount(); i++)
  {
    ParserInstance *parser = parsers[i];
    far_assert(parser);
    char *token = parser->next_token();
    tokens.Add(token);
    int at = parser->get_tokenpos();
    tokens_at.Add(at);
  }
  far_assert(tokens.Count()==dict_view_instance->DictCount());
  far_assert(tokens_at.Count()==tokens.Count());
  for (int i = 0; i<tokens.Count(); i++)
  {
    char *doc_token = tokens[i];
    if (!doc_token)
      continue;
    if (!*doc_token)
    { // sanity check
      hunspell_free(doc_token);
      continue;
    }
    const size_t tok_len = strlen(doc_token);
    FarStringW token_wide;
    ToUnicode(document_encoding, doc_token, token_wide);
    token->begin = tokens_at[i];
    token->end = token->begin + tok_len;
    token->color = dict_view_instance->WordColor(token_wide, default_color);
    token->A.SetText(doc_token, tok_len);
    token->W = token_wide;
    hunspell_free(doc_token);
    result = true;
  }
  return result;
}

bool VirtalSpellEngine::TokenAt(int pos, Token *token)
{
  far_assert(false, "NYI");
  return false;
}

SpellEngine::WordList *VirtalSpellEngine::Suggestion(const Token &token)
{
  far_assert(false, "NYI");
  return NULL;
}
 
SpellEngine::WordList *VirtalSpellEngine::Suggestion(const FarStringW &word)
{
  far_assert(false, "NYI");
  return NULL;
}

bool VirtalSpellEngine::CheckDictionary()
{
  DictViewInstance *dict_view_instance = GetDict();
  if (!dict_view_instance) 
    return false;
  for (int i = 0; i<dict_view_instance->DictCount(); i++)
  { 
    FarString dict(dict_view_instance->GetDict(i)->dict);
    if (!FarSpellEditor::editors->spell_factory.DictionaryExists(dict))
      return false;
  }
  return true;
}

void VirtalSpellEngine::FreeLotOfMemory()
{
  far_assert(false, "NYI");
  DropEngine();
}
