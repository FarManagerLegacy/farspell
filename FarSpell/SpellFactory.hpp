/*
    Factory of spell engines for FarSpell plugin: header.
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
*/

#pragma once
#include <FARPlus/FARPlus.h>
#include <FARPlus/FARArray.h>
#include <FARPlus/FARString.h>
#include <FARPlus/FARRegistry.h>

#include "hunspell/hunspell.hxx"

#define hunspell_free free
#define hunspell_malloc malloc


class SpellInstance: public Hunspell
{
  public: 
    class WordList
    {
      friend class SpellInstance;
      public:
        ~WordList();
        int Count() const { return count; }
        FarStringW& operator[](int index);
      protected:
        WordList(int _encoding, char** _wlst, int _count);
        int encoding;
        char **wlst;
        int count;
        FarStringW _word_cache;
    };
    SpellInstance( const FarString &reg_root, const FarFileName &dict_root, 
                   const FarString &dict);
    WordList *Suggest(const FarStringW &word);
    bool Check(const FarStringW &word);
    const FarStringW &GetWordChars() {  return word_chars; }
    void SetWordChars(const FarStringW &new_wc);
  protected:
    FarStringW GetWordChars_FromDict();
    FarRegistry reg;
    FarStringW word_chars;
    int encoding;
};


class SpellFactory
{
  public:
    typedef int (*SpellFactoryEnumProc)(const FarString& name, void* param);
    SpellFactory(const FarFileName &_dict_root, const FarString &_reg_root)
    : dict_root(_dict_root)
    , reg_root(_reg_root+"\\Hunspell\\")
    {
    }
    ~SpellFactory() {};
    SpellInstance* GetDictInstance(const FarString& dict);
    bool AnyDictionaryExists();
    bool DictionaryExists(const char *dict);
    void UnloadDictionaries()
    {
      cache_engine_langs.Clear();
      cache_engine_instances.Clear();
    }
    void EnumDictionaries(SpellFactoryEnumProc enum_proc, void* param);
  protected:
    FarString reg_root;
    FarFileName dict_root;
    FarArray<FarString> cache_engine_langs;
    FarArray<SpellInstance> cache_engine_instances;
    void EnumDictionaries(FRSUSERFUNC Func, void *param)
    {
      FarSF::RecursiveSearch((char*)dict_root.c_str(), "*.aff", Func, 0, param);
    }
    static int WINAPI GetDictCountCb(const WIN32_FIND_DATA *FData, 
      const char *FullName, void *Param);
    static int WINAPI EnumDictionariesCb(const WIN32_FIND_DATA *FData, 
      const char *FullName, void *Param);
};

