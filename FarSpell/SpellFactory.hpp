#pragma once

class SpellInstance: public Hunspell
{
  public: 
    class WordList
    {
      friend class SpellInstance;
      public:
        ~WordList();
        inline int Count() const;
        FarStringW& operator[](int index);
      private:
        WordList(int _encoding, char** _wlst, int _count);
        int encoding;
        char **wlst;
        int count;
        FarStringW _word_cache;
    };
    SpellInstance( FarString &reg_root, FarFileName &dict_root, 
                   FarString &dict);
    WordList *Suggest(FarStringW &word);
    bool Check(FarStringW &word);
    const FarStringW &GetWordChars() {  return word_chars; }
    void SetWordChars(FarStringW &new_wc);
  private:
    FarStringW GetWordChars_FromDict();
    FarRegistry reg;
    FarStringW word_chars;
    int encoding;
};

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

inline int SpellInstance::WordList::Count() const
{
  return count;
}

FarStringW& SpellInstance::WordList::operator[](int index) 
{
  ToUnicode(encoding,  FarString(wlst[index]), _word_cache);
  return _word_cache;
}

SpellInstance::WordList *SpellInstance::Suggest(FarStringW &word)
{
  FarString aword;
  char **wlst;
  ToAscii(encoding, word, aword);
  int count = suggest(&wlst, aword);
  return new WordList(encoding, wlst, count);
}

SpellInstance::SpellInstance( FarString &reg_root, FarFileName &dict_root, FarString &dict)
: Hunspell(FarFileName::MakeName(dict_root, dict+".aff").c_str(),
           FarFileName::MakeName(dict_root, dict+".dic").c_str())
, reg(reg_root, dict)
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

void SpellInstance::SetWordChars(FarStringW &new_wc)
{
  word_chars = new_wc;
  reg.SetRegKey("", "WordChars", (char*)word_chars.c_str(), word_chars.Length()*sizeof(wchar_t));
}

bool SpellInstance::Check(FarStringW &word)
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

class SpellFactory
{
  public:
    typedef int (*SpellFactoryEnumProc)(const FarString& name, void* param);
    SpellFactory(FarFileName &_dict_root, FarString &_reg_root)
    : dict_root(_dict_root)
    , reg_root(_reg_root+"\\Hunspell\\")
    {
    }
    ~SpellFactory() {};
    SpellInstance* GetDictInstance(FarString& dict);
    bool AnyDictionaryExists();
    bool DictionaryExists(const char *dict);
    void UnloadDictionaries()
    {
      cache_engine_langs.Clear();
      cache_engine_instances.Clear();
    }
    void EnumDictionaries(SpellFactoryEnumProc enum_proc, void* param);
  private:
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

SpellInstance* SpellFactory::GetDictInstance(FarString& dict)
{
  for (int i = 0; i < cache_engine_langs.Count(); i++)
  {
     if ((*cache_engine_langs[i]) == dict)
       return cache_engine_instances[i];
  }
  if (!DictionaryExists(dict)) return NULL;
  HANDLE screen = Far::SaveScreen();
  FarMessage wait(0);
  wait.AddLine(MFarSpell);
  wait.AddFmt(MLoadingDictionary, dict.c_str());
  wait.Show();
  SpellInstance* engine = new SpellInstance(reg_root, dict_root, dict);
  cache_engine_langs.Add(new FarString(dict));
  cache_engine_instances.Add(engine);
  Far::RestoreScreen(screen);
  return engine;
}


bool SpellFactory::AnyDictionaryExists()
{
  int dict_count = 0;
  EnumDictionaries(GetDictCountCb, &dict_count);
  return dict_count>0;
}

bool SpellFactory::DictionaryExists(const char *dict)
{
  int count = 0;
  FarFileName name(dict);
  name.SetExt(".aff");
  FarSF::RecursiveSearch((char*)dict_root.c_str(), (char*)name.c_str(), 
    GetDictCountCb, 0, &count);
  return count>0;
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

void SpellFactory::EnumDictionaries(SpellFactoryEnumProc enum_proc, void* param)
{
  EnumProcInstance inst = {enum_proc, param};
  EnumDictionaries(EnumDictionariesCb, &inst);
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
