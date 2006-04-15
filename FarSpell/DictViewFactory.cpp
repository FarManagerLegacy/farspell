/*
    Factory of dictionary view for FarSpell plugin.
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
#include <malloc.h>
#include <limits.h>
#include "FARPlus/FARString.h"
#include "FARPlus/FARArray.h"
#include "FARPlus/FARRegistry.h"

#include "DecisionTable.hpp"
#include "SpellFactory.hpp"

class DictViewInstance: protected DecisionTable::Context
{
  friend class DictViewFactory;
  public:
    class DictParams: protected DecisionTable::Condition {
      friend class DictViewInstance;
      public:
        FarString name;
        FarString dict;
        void SetReplaceChars(const FarStringW &from, const FarStringW &to);
        bool GetReplaceChars(FarStringW &out_from, FarStringW &out_to);
      protected:
        DictParams(DictViewInstance *owner);
        DictParams(DictViewInstance *owner, int uid);
        void PreprocessWord(/*in,out*/FarStringW &word);
      protected:
        DictViewInstance *owner;
        SpellFactory *spell_factory;
        FarString reg_key;
        bool need_preprocess_word;
        bool replace_chars;
        FarStringW replace_from;
        FarStringW replace_to;
        bool Execute(const FarStringW &word);
      protected: // Condition
        void OnSave();
        bool Execute();
    };
    class Color: protected DecisionTable::Action {
      friend class DictViewInstance;
      protected:
        Color(DictViewInstance *owner);
        Color(DictViewInstance *owner, int uid);
        DictViewInstance *owner;
        FarString reg_key;
      public:
        bool enabled;          // produce spell error?
        int color;             // native highlight color
        FarString suggestions; // list of dictionaries separated by semicolon ";"
      protected: // Action
        void OnSave();
        void Execute();
    };
    const FarString reg_key;
    FarString name;
    DecisionTable logic;
    int WordColor(FarStringW &word, int default_color);
    bool Save();
    int DictCount() const;
    DictParams *AddDict(const FarString &dict);
    DictParams *GetDict(int index);
    void RemoveDict(int index);
    void RemoveDict(DictParams *);
  protected:
    DictViewInstance(FarRegistry &init_reg, const FarString &init_reg_key,
      SpellFactory *init_spell_factory );
  protected:
    FarRegistry &reg;
    SpellFactory *spell_factory;
    FarStringW _current_word;
    int _current_color;
  protected:
    int NextUID();
    FarString NextUIDStr();
    FarString GetRegKeyName(const char* Class, int uid);
    DecisionTable::condition_inst_t GetCondition(int uid);
    DecisionTable::condition_inst_t CreateCondition();
    DecisionTable::action_inst_t GetAction(int uid);
    DecisionTable::action_inst_t CreateAction();
};

class DictViewFactory
{
  public:
    typedef int (*DictViewFactoryEnumProc)(const FarString& id, const FarString& name, void* client_context);
    DictViewFactory(const FarString &init_reg_root, 
      SpellFactory *init_spell_factory);
    void EnumDictViews(DictViewFactoryEnumProc client, void* client_context);
    DictViewInstance *GetDictViewByName(const FarString &name);
    void DeleteDictView(const FarString &name);
    DictViewInstance *CreateDictView();
  protected:
    int NextUID();
    int GetDictViewId(const FarString &name);
    FarString GetDictViewIdStr(const FarString &name);
      static int FindDictViewId(const FarString& id, const FarString& name, void* client_context);

    FarRegistry reg;
    SpellFactory *spell_factory;
};

#ifndef _INTERFACE
static const char* const dict_view_uid_key = "UID";
static const char* const dict_view_name_key = "Name";
static const char* const dict_view_logic_key = "Logic";
static const char* const dict_view_last_uid_key = "LastUID";

static const char* const dict_params_uid_key = "UID";
static const char* const dict_params_name_key = "Name";
static const char* const dict_params_dict_key = "Dict";
static const char* const dict_params_replace_chars_key = "ReplaceChars";
static const char* const dict_params_replace_chars_from_key = "ReplaceCharsFrom";
static const char* const dict_params_replace_chars_to_key = "ReplaceCharsTo";

static FarString itoa(int i, int radix)
{
  FarString s;
  itoa(i, s.GetBuffer(32), radix);
  s.ReleaseBuffer();
  return s;
}

DictViewFactory::DictViewFactory(const FarString &init_reg_root,
 SpellFactory *init_spell_factory)
: reg(init_reg_root+"\\DictView\\")
{
  far_assert(init_spell_factory);
  spell_factory = init_spell_factory;
}

void DictViewFactory::EnumDictViews(DictViewFactoryEnumProc client, void* client_context)
{
  FarString key, name;
  for (FarRegistry::KeyIterator i(reg.EnumKeys("")); i.NextKey(key);) {
    name = reg.GetRegStr(key.c_str(), dict_view_name_key, "");
    if (!name.IsEmpty())
      client(key, name, client_context);
  }
}

DictViewInstance *DictViewFactory::GetDictViewByName(const FarString &name)
{
  FarString id(GetDictViewIdStr(name));
  if (!id.IsEmpty())
    return new DictViewInstance(reg, id, spell_factory);
  else
    return CreateDictView();
}

DictViewInstance *DictViewFactory::CreateDictView()
{
  return new DictViewInstance(reg, itoa(NextUID(), 10), spell_factory);
}

void DictViewFactory::DeleteDictView(const FarString &name)
{
  FarString id(GetDictViewIdStr(name));
  if (!id.IsEmpty())
    reg.DeleteRegKey("", id.c_str());
}

int DictViewFactory::NextUID()
{
  static const char *last_uid_key = "LastUID";
  int uid = reg.GetRegKey("", last_uid_key, 0) + 1;
  reg.SetRegKey("", last_uid_key, uid);
  return uid;
}

struct FindDictViewId_Ctx {
  const FarString &name; // in
  FarRegistry &reg;
  int uid; // out
};

int DictViewFactory::FindDictViewId(const FarString& id, 
  const FarString& name, void* client_context)
{
  FindDictViewId_Ctx* ctx = (FindDictViewId_Ctx *)client_context;
  if (ctx->name == name) {
    ctx->uid = atoi(id.c_str());
    return 0;
  } else 
    return 1;
}

int DictViewFactory::GetDictViewId(const FarString &name)
{
  FindDictViewId_Ctx ctx = { name, reg, -1 };
  EnumDictViews(FindDictViewId, &ctx);
  return ctx.uid;
}

FarString DictViewFactory::GetDictViewIdStr(const FarString &name)
{
  int id = GetDictViewId(name);
  if (id!=-1) {
    return itoa(id, 10);
  } else
    return FarString("");
}

DictViewInstance::DictParams::DictParams(DictViewInstance *init_owner)
{
  owner = init_owner;
  uid = owner->NextUID();
  reg_key = owner->GetRegKeyName("DictParams", uid);
  spell_factory = owner->spell_factory;
  name = "<noname>";
  dict = "en_US";
  replace_chars = false;
  need_preprocess_word = false;
}

DictViewInstance::DictParams::DictParams(DictViewInstance *init_owner, 
 int init_uid)
{
  owner = init_owner;
  spell_factory = owner->spell_factory;
  uid = init_uid;
  reg_key = owner->GetRegKeyName("DictParams", uid);
  name = owner->reg.GetRegStr(reg_key.c_str(), dict_params_name_key, "<noname>");
  dict = owner->reg.GetRegStr(reg_key.c_str(), dict_params_dict_key, "en_US");
  replace_chars = owner->reg.GetRegKey(reg_key.c_str(), dict_params_replace_chars_key, false);
  replace_from = owner->reg.GetFarString(reg_key.c_str(), dict_params_replace_chars_from_key, 
    FarStringW(L""));
  replace_to = owner->reg.GetFarString(reg_key.c_str(), dict_params_replace_chars_to_key, 
    FarStringW(L""));
  need_preprocess_word = replace_chars;
}

void DictViewInstance::DictParams::OnSave()
{
  owner->reg.SetRegKey(reg_key.c_str(), dict_params_uid_key, uid);
  owner->reg.SetRegKey(reg_key.c_str(), dict_params_name_key, name);
  owner->reg.SetRegKey(reg_key.c_str(), dict_params_dict_key, dict);
  owner->reg.SetRegKey(reg_key.c_str(), dict_params_replace_chars_key, replace_chars);
  if (replace_chars) {
    owner->reg.SetFarString(reg_key.c_str(), dict_params_replace_chars_from_key, replace_from);
    owner->reg.SetFarString(reg_key.c_str(), dict_params_replace_chars_to_key, replace_to);
  }
}

void DictViewInstance::DictParams::SetReplaceChars(const FarStringW &from, const FarStringW &to)
{
  replace_chars = from.Length() && to.Length() 
                  && from.Length() == to.Length()? true : false;
  if (replace_chars) {
    replace_from = from;
    replace_to = to;
  } else {
    replace_from.Empty();
    replace_to.Empty();
  }
  need_preprocess_word = replace_chars;
}

bool DictViewInstance::DictParams::GetReplaceChars(FarStringW &out_from, 
  FarStringW &out_to)
{
  if (replace_chars)
  {
    out_from = replace_from;
    out_to = replace_to;
    return true;
  } else {
    out_from.Empty();
    out_to.Empty();
    return false;
  }
}

void DictViewInstance::DictParams::PreprocessWord(/*in,out*/FarStringW &word)
{
  if (replace_chars) {
    for (int i = 0; i<replace_from.Length() && i<replace_to.Length(); i++)
      for (int j = 0; j<word.Length(); j++)
        if (word[j] == replace_from[i]) 
          word[j] = replace_to[i];
  }
}

bool DictViewInstance::DictParams::Execute(const FarStringW &in_word)
{
  far_assert(spell_factory);
  SpellInstance *dict_inst = spell_factory->GetDictInstance(dict);
  far_assert(dict_inst);
  if (need_preprocess_word) {
    FarStringW word = in_word;
    PreprocessWord(word);
    return dict_inst->Check(word);
  } else 
    return dict_inst->Check(in_word);
}

bool DictViewInstance::DictParams::Execute()
{
  return Execute(owner->_current_word);
}

DictViewInstance::Color::Color(DictViewInstance *init_owner)
{
  owner = init_owner;
  uid = owner->NextUID();
  reg_key = owner->GetRegKeyName("Color", uid);
  color = 0xF0;
}

static const char* const dict_color_enabled_key = "Enabled";
static const char* const dict_color_color_key = "Color";
static const char* const dict_color_suggestions_key = "Suggestions";

DictViewInstance::Color::Color(DictViewInstance *init_owner, int init_uid)
{
  owner = init_owner;
  uid = init_uid;
  reg_key = owner->GetRegKeyName("Color", uid);
  color = owner->reg.GetRegKey(reg_key.c_str(), dict_color_color_key, 0xF0);
  enabled = owner->reg.GetRegKey(reg_key.c_str(), dict_color_enabled_key, true);
  suggestions = owner->reg.GetRegStr(reg_key.c_str(), dict_color_suggestions_key, ""); 
}

void DictViewInstance::Color::OnSave()
{
  owner->reg.SetRegKey(reg_key.c_str(), dict_color_color_key, color);
  owner->reg.SetRegKey(reg_key.c_str(), dict_color_enabled_key, enabled);
  owner->reg.SetRegKey(reg_key.c_str(), dict_color_suggestions_key, suggestions.c_str());
}

void DictViewInstance::Color::Execute()
{
  owner->_current_color = color;
}

DictViewInstance::DictViewInstance(FarRegistry &init_reg, const FarString &init_reg_key,
  SpellFactory *init_spell_factory )
: reg(init_reg)
, reg_key(init_reg_key)
{
  far_assert(init_spell_factory);
  spell_factory = init_spell_factory;
  name = reg.GetRegStr(reg_key.c_str(), dict_view_name_key, 
   FarString(reg_key + ": <noname>").c_str() );
  logic.SetContext(this);
  FarString dt(reg.GetFarString(reg_key.c_str(), dict_view_logic_key, 
    FarString("")));
  if (!dt.IsEmpty())
    logic.FromString(dt);
}

bool DictViewInstance::Save()
{
  reg.SetRegKey(reg_key.c_str(), dict_view_uid_key, reg_key.c_str());
  reg.SetRegKey(reg_key.c_str(), dict_view_name_key, name.c_str());
  reg.SetFarString(reg_key.c_str(), dict_view_logic_key, logic.AsString());
  return true;
}

int DictViewInstance::NextUID()
{
  int uid = reg.GetRegKey(reg_key.c_str(), dict_view_last_uid_key, 0) + 1;
  reg.SetRegKey(reg_key.c_str(), dict_view_last_uid_key, uid);
  return uid;
}

FarString DictViewInstance::GetRegKeyName(const char* Class, int uid)
{
  char buf[32];
  return reg_key+"\\"+Class+"\\"+itoa(uid, buf, 10)+"\\";
}

DecisionTable::condition_inst_t DictViewInstance::GetCondition(int uid)
{
  DictParams *cond = new DictParams(this, uid);
  cond->client_context = cond;
  return cond;
}

DecisionTable::condition_inst_t DictViewInstance::CreateCondition()
{
  DictParams *cond = new DictParams(this);
  cond->client_context = cond;
  return cond;
}

DecisionTable::action_inst_t DictViewInstance::GetAction(int uid)
{
  Color *act = new Color(this, uid);
  act->client_context = act;
  return act;
}

DecisionTable::action_inst_t DictViewInstance::CreateAction()
{
  Color *act = new Color(this);
  act->client_context = act;
  return act;
}

int DictViewInstance::DictCount() const
{
  return logic.GetConditionsCount();
}

DictViewInstance::DictParams *DictViewInstance::GetDict(int index)
{
  DecisionTable::condition_inst_t cond_inst;

  far_assert(index >= 0);
  far_assert(index < logic.GetConditionsCount());
  cond_inst = logic.GetConditionInst(index);
  far_assert(cond_inst);
  DictViewInstance::DictParams *params = 
    static_cast<DictParams *>(cond_inst->client_context);
  far_assert(params);
  return params;
}

DictViewInstance::DictParams *DictViewInstance::AddDict(const FarString &dict)
{
  DecisionTable::condition_inst_t inst =
    logic.InsertCondition(logic.GetConditionsCount());
  DictParams *params = reinterpret_cast<DictParams *>(inst->client_context);
  params->dict = dict;
  return params;
}

void DictViewInstance::RemoveDict(int index)
{
  far_assert(index >= 0);
  far_assert(index < logic.GetConditionsCount());
  logic.DeleteCondition(index);
}

void DictViewInstance::RemoveDict(DictParams *instance)
{
  int index = logic.GetConditionNumber(instance);
  far_assert(index >= 0);
  far_assert(index < logic.GetConditionsCount());
  RemoveDict(index);
}

int DictViewInstance::WordColor(FarStringW &word, int default_color)
{
  _current_color = -1;
  _current_word = word;
  DecisionTable::action_inst_t act = logic.Execute();
  if (act) {
    far_assert(_current_color!=-1);
    return _current_color;
  } else
    return default_color;
}
#endif //_INTERFACE