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

#pragma once
#define _INTERFACE
#include <boost/noncopyable.hpp>
#include <FARPlus/FARString.h>
#include <FARPlus/FARArray.h>
//#include "FarSpell.hpp"

class FarSpellEditor;
// I hate standart "virtual foo() = 0;"
#define abstract = 0
class SpellEngine
: private boost::noncopyable
{
  protected:
    int document_encoding;
    int default_color;
  public:
    class WordList
    : private boost::noncopyable
    {
      public:
        virtual ~WordList() {};
        virtual int Count() const abstract; 
        virtual FarStringW& operator[](int index) abstract;
      protected:
        WordList() {};
    };
    class Token 
    : private boost::noncopyable
    {
      protected:
        void *data;
      public:
        FarStringA A; // in document encoding
        FarStringW W;
        int color;
        int begin, end;
    };
    SpellEngine(int _document_encoding) 
    { 
      document_encoding = _document_encoding;
      default_color = -1;
    };
    virtual ~SpellEngine() { };
    virtual void SetDocumentFileSource(const FarString &file_name) abstract;
    virtual void SetDocumentEncoding(int new_document_encoding) abstract;
    virtual void SetDictionary(const FarString &dict) abstract;
    virtual void PutLine(const FarString &line) abstract;
    virtual bool NextToken(Token *token) abstract;
    virtual bool TokenAt(int pos, Token *token) abstract;
    virtual WordList *Suggestion(const Token &token) abstract;
    virtual WordList *Suggestion(const FarStringW &word) abstract;
  public:
    virtual bool CheckDictionary() abstract;
    virtual void FreeLotOfMemory() abstract;
};
// avoid use of "abstract" "keyword" in successors
#undef abstract


class SpellInstance;
class ParserInstance;

class SimpleSpellEngine
: public SpellEngine 
{
  protected:
    class SimpleWordList
    : WordList
    {
        friend class SimpleSpellEngine;
        SpellInstance::WordList *word_list;
        SimpleWordList(SpellInstance::WordList *_word_list) { word_list = _word_list; };
        ~SimpleWordList() { delete word_list; };
        int Count() const { return word_list->Count(); }; 
        FarStringW& operator[](int index) { return (*word_list)[index]; };
    };
  protected:
    int dictionary_encoding;
    int spell_color;
    SpellInstance* _dict_instance;
    FarString dict;
    ParserInstance* _parser_instance;
    FarString parser_id;
    FarString file_name;
    enum { RS_DICT = 0x1, RS_PARSER = 0x2,
           RS_ALL = RS_DICT|RS_PARSER };
    void RecreateEngine(int what = RS_ALL);
    void DropEngine(int what = RS_ALL);
    SpellInstance* GetDict();
    ParserInstance* GetParser();
  public:
    SimpleSpellEngine(int _document_encoding, int _spell_color);
    ~SimpleSpellEngine();
    void SetDocumentFileSource(const FarString &file_name);
    void SetDocumentEncoding(int new_document_encoding);
    void SetDictionary(const FarString &dict);
    void PutLine(const FarString &line);
    bool NextToken(Token *token);
    bool TokenAt(int pos, Token *token);
    WordList *Suggestion(const Token &token);
    WordList *Suggestion(const FarStringW &word);
  public:
    bool CheckDictionary();
    void FreeLotOfMemory();
};

class VirtalSpellEngine
: public SpellEngine 
{
  typedef FarArray<ParserInstance> parser_instances_t;
  protected:
    class VirtualWordList
    : WordList
    {
        friend class VirtalSpellEngine;
        DictViewInstance::WordList *word_list;
        VirtualWordList(DictViewInstance::WordList *_word_list) { word_list = _word_list; };
        ~VirtualWordList() { delete word_list; };
        int Count() const { return word_list->Count(); }; 
        FarStringW& operator[](int index) { return (*word_list)[index]; };
    };
  protected:
    int spell_color;
    parser_instances_t _parser_instances;
    DictViewInstance *_dict_view_instance;
    FarString dict_view;
    FarString parser_id;
    FarString file_name;
    enum { RS_DICT = 0x1, RS_PARSER = 0x2,
           RS_ALL = RS_DICT|RS_PARSER };
    void RecreateEngine(int what = RS_ALL);
    void DropEngine(int what = RS_ALL);
    DictViewInstance* GetDict();
    parser_instances_t &GetParsers();
  public:
    VirtalSpellEngine(int _document_encoding);
    ~VirtalSpellEngine();
    void SetDocumentFileSource(const FarString &file_name);
    void SetDocumentEncoding(int new_document_encoding);
    void SetDictionary(const FarString &dict);
    void PutLine(const FarString &line);
    bool NextToken(Token *token);
    bool TokenAt(int pos, Token *token);
    WordList *Suggestion(const Token &token);
    WordList *Suggestion(const FarStringW &word);
  public:
    bool CheckDictionary();
    void FreeLotOfMemory();
};
