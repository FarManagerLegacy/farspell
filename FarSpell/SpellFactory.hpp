#pragma once

typedef Hunspell SpellInstance;

class SpellFactory
{
  public:
    typedef int (*SpellFactoryEnumProc)(const FarString& name, void* param);
    SpellFactory(FarFileName &_dict_root)
    : dict_root(_dict_root)
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
  SpellInstance* engine = new SpellInstance(
         FarFileName::MakeName(dict_root, dict+".aff").c_str(),
         FarFileName::MakeName(dict_root, dict+".dic").c_str());
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
