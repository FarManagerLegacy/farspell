/*
    Factory of spell engines for FarSpell plugin: implementation.
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

#include "FarSpell.hpp"
#include "SpellFactory.hpp"

SpellInstance::WordList::WordList(int _encoding, char** _wlst, int _count)
{
  wlst = _wlst;
  count = _count;
  encoding = _encoding;
}

SpellInstance::WordList::~WordList()
{
  if (wlst)
  {
    for (int j = 0; j < count; j++)
      hunspell_free(wlst[j]);
    hunspell_free(wlst);
    wlst = NULL;
  }
}

FarStringW& SpellInstance::WordList::operator[](int index) 
{
  ToUnicode(encoding,  FarString(wlst[index]), _word_cache);
  return _word_cache;
}

SpellInstance::WordList *SpellInstance::Suggest(const FarStringW &word)
{
  FarString aword;
  char **wlst;
  ToAscii(encoding, word, aword);
  int count = suggest(&wlst, aword);
  return new WordList(encoding, wlst, count);
}

SpellInstance::SpellInstance( const FarString &reg_root, 
  const FarFileName &dict )
: Hunspell((dict+".aff").c_str(), (dict+".dic").c_str())
, reg(reg_root, FarSF::PointToName(dict.c_str()))
{
  encoding = search_codepage(FarString(get_dic_encoding()));
  FarStringW orig_word_chars(GetWordChars_FromDict());
  DWORD str_size = reg.GetValueSize("", "WordChars");

  if ((int)str_size>0 && (str_size&1)==0) {
    if (reg.GetRegKey("", "WordChars", 
                      word_chars.GetBuffer(str_size/sizeof(wchar_t)), str_size, 
                      orig_word_chars.data(), 
                      orig_word_chars.Length()*sizeof(wchar_t)))
      word_chars.ReleaseBuffer(str_size/sizeof(wchar_t));
    else
      word_chars.ReleaseBuffer(orig_word_chars.Length());
  }
  else 
    word_chars = orig_word_chars;
}

void SpellInstance::SetWordChars(const FarStringW &new_wc)
{
  word_chars = new_wc;
  reg.SetRegKey("", "WordChars", (char*)word_chars.c_str(), word_chars.Length()*sizeof(wchar_t));
}

bool SpellInstance::Check(const FarStringW &word)
{
  FarString aword;
  ToAscii(encoding, word, aword);
  return spell(aword.c_str());
}

FarStringW SpellInstance::GetWordChars_FromDict()
{
  FarStringW word_chars;
  int wcs_len;
  if (unsigned short * wcs = get_wordchars_utf16(&wcs_len))
  {
    wcsncpy(word_chars.GetBuffer(wcs_len), wcs, wcs_len);
    return word_chars;
  } else {
    ToUnicode(encoding, FarString(get_wordchars()), word_chars);
    return word_chars;
  }
}

SpellInstance* SpellFactory::GetDictInstance(const FarString& dict)
{
  for (int i = 0; i < cache_engine_langs.Count(); i++)
  {
     if ((*cache_engine_langs[i]) == dict)
       return cache_engine_instances[i];
  }
  FarFileName dict_path(DictionaryExists(dict)); // without extension

  if (dict_path.IsEmpty()) return NULL;

  HANDLE screen = Far::SaveScreen();
  FarMessage wait(0); // TODO: intercept stderr from Hunspell
  wait.AddLine(MFarSpell);
  wait.AddFmt(MLoadingDictionary, dict.c_str());
  wait.AddLine(dict_path.c_str());
  wait.Show();

  SpellInstance *engine = new SpellInstance(reg_root, dict_path);
  cache_engine_langs.Add(new FarString(dict));
  cache_engine_instances.Add(engine);
  Far::RestoreScreen(screen);
  return engine;
}


bool SpellFactory::AnyDictionaryExists()
{
  int dict_count = 0;
  EnumDictionaries("*", GetDictCountCb, &dict_count);
  return dict_count>0;
}

FarFileName SpellFactory::DictionaryExists(const char *dict)
{
  FarFileName result;
  EnumDictionaries(dict, SearchDictionary, &result);
  return result;
}

int WINAPI SpellFactory::SearchDictionary(
  const WIN32_FIND_DATA *FData,
  const char *FullName,
  void *Param
)
{
  FarFileName *out_name = reinterpret_cast<FarFileName *>(Param);
  FarFileName name(FullName, strlen(FullName)-4); // *.aff only
  if ( FarFileInfo::FileExists(name+".dic")) {
    *out_name = name; // return full file name without extension.
    return 0;
  } else
    return 1;
}

void SpellFactory::EnumDictionaries(const FarString &mask, FRSUSERFUNC Func, void *param)
{
  FarStringTokenizer folders(dict_root.c_str(), ';');
  while (folders.HasNext()) 
    FarSF::RecursiveSearch(const_cast<char *>(folders.NextToken().c_str()),
       const_cast<char *>((mask+".aff").c_str()), Func, 0, param);
}

int WINAPI SpellFactory::GetDictCountCb(
  const WIN32_FIND_DATA *FData,
  const char *FullName,
  void *Param
)
{
  FarFileName name(FullName, strlen(FullName)-4); // *.aff only
  if (FarFileInfo::FileExists(name+".dic"))
     (*(int*)Param)++;
  return 1;
}

struct EnumProcInstance
{
  SpellFactory::SpellFactoryEnumProc proc;
  void* param;
};

void SpellFactory::EnumDictionaries(const FarString &mask, SpellFactoryEnumProc enum_proc, void* param)
{
  EnumProcInstance inst = {enum_proc, param};
  EnumDictionaries(mask, EnumDictionariesCb, &inst);
}

int WINAPI SpellFactory::EnumDictionariesCb(
  const WIN32_FIND_DATA *FData,
  const char *FullName,
  void *Param
)
{
  EnumProcInstance *inst = (EnumProcInstance *)Param;
  FarFileName name(FullName, strlen(FullName)-4); // *.aff only
  if (FarFileInfo::FileExists(name+".dic"))
     return inst->proc(name.GetNameExt(), inst->param);
  return 1;
}
