/*
    Spell-checker plugin for FAR
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
*/

#define HARDCODED_MLDATA

#include <malloc.h>
#include <limits.h>
#include <stdio.h>
#define _WINCON_
#include <windows.h>
#undef _WINCON_
#pragma pack(push,8)
#include <wincon.h>
#pragma pack(pop)

#ifndef HARDCODED_MLDATA
#include <MLang.h>
#endif HARDCODED_MLDATA

#include "FARPlus/FARMemory.h"
#include "FARPlus/FARLog.h"
#include "FARPlus/FARPlus.h"
#include "FARPlus/FARRegistry.h"
#include "FARPlus/FARDialog.h"
#include "FARPlus/FAREd.h"
//#include "FARPlus/FARXml.h"
//#include "FARPlus/FARDbg.h"
#include "FARPlus/FARFile.h"
#include "ftl.hpp"

//#include "hunspell/hunspell.h"
#include "hunspell/hunspell.hxx"
#define hunspell_free free
#define hunspell_malloc malloc
#include "parsers/textparser.hxx"
#include "parsers/textparser.hxx"
#include "parsers/htmlparser.hxx"
#include "parsers/latexparser.hxx"
#include "parsers/manparser.hxx"
#include "parsers/firstparser.hxx"

#ifdef HARDCODED_MLDATA
#include "codepages.cpp"
#endif

// Registry keys
char *const plugin_enabled_key = "PluginEnabled";
char *const editor_files_key = "EditorFiles";
char *const highlight_mask_key = "HighlightMask";
char *const highlight_color_key = "HighlightColor";
char *const highlight_deletecolor_key = "HighlightDeleteColor";
char *const suggestions_in_menu_key = "AreSuggesionsInMenu";
char *const enable_file_settings_key = "FileSettings";
char *const default_dict_key = "DefaultDict";
char *const oem_cp_source_key = "OEMCodepageSource";

// -- Language strings -------------------------------------------------------
enum {
  MDbgPleaseWait,
  MDbgContinue,
  MNoDbgMessages,
  MUnloadPlugin,
  MExitFAR,
  MDbgSeeMore,
  MExceptionIn,

  MError,
  MOk,
  MCancel,

  MFarSpell,
  MLoadingDictionary,
  MEngineNotInstalled,
  MNoDictionaries,
  MPluginWillBeDisabled,
  MNoDictionary,
  MHighlightingWillBeDisabled,

  MSuggestion,
  MReplace,
  MSkip,
  MStop,
  MPreferences,
  MEditorGeneralConfig,
  MUnloadDictionaries,

  MHighlight,
  MDictianary,
  MParser,

  MGeneralConfig,
  MPluginEnabled,
  MDefaultDict,
  MSpellExts,
  MSuggMenu,
  MFileSettings,
  MAnotherColoring,
  MColorSelectBtn,

  MSelectColor,
  MForeground,
  MBackground,
  MTest,

  MFileSettingWasDisabled,
  MWillDeleteFileSetting,
  MFileSettingsRemoved,

  M_FMT_AUTO_TEXT,
  M_FMT_TEXT,
  M_FMT_LATEX,
  M_FMT_HTML,
  M_FMT_MAN,
  M_FMT_FIRST,
};

// Dialog ID's
enum {
  ID_GC_SpellExts=300,
  ID_GC_DefaultDict,
  ID_S_Word,
  ID_S_WordList,
};

// Generated dialogs:
#include "dialogs.cpp"

// class SpellFactory:
#include "SpellFactory.hpp"

// class ParserFactory:
#include "ParserFactory.hpp"

FarString HRString(HRESULT hr)
{
  char *msg;
  FormatMessage (
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    0,
    hr,
    MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<char *> (&msg),
    0,
    0);
  FarString res(msg);
  LocalFree(msg);
  return res;
}

void ConvertEncoding(FarString &src, int src_enc, FarString &dst, int dst_enc)
{
  wchar_t *w;
  int wc, ac;
  if (src_enc==dst_enc)
  {
    if (&src!=&dst) dst=src;
    return;
  }
  wc = MultiByteToWideChar(src_enc, 0, src.c_str(), src.Length(), NULL, 0);
  w = (wchar_t*)alloca((wc+1)*sizeof(wchar_t));
  wc = MultiByteToWideChar(src_enc, 0, src.c_str(), src.Length(), w, wc);
  ac = WideCharToMultiByte(dst_enc, 0, w, wc, NULL, 0, NULL, NULL);
  ac = WideCharToMultiByte(dst_enc, 0, w, wc, dst.GetBuffer(ac+1), ac, NULL, NULL);
  dst.ReleaseBuffer(ac);
}

class FarEditorSuggestList;
class FarSpellEditor
{
  friend class FarEditorSuggestList;
  public:
    int EditorId;
    FarSpellEditor *next, *prev;
    class Manager
    {
      public:
        FarSpellEditor *last;
        FarFileName plugin_root;
        SpellFactory spell_factory;
        FarString highlight_list;
        bool plugin_enabled;
        bool enable_file_settings;
        int highlight_color;
        bool suggestions_in_menu;
        bool highlight_deletecolor; 
        int oem_cp_source; enum { OEM_from_GetOEMCP,  OEM_from_GetConsoleOutputCP };
        FarString default_dict;
        class FarRegistry1: public FarRegistry
        {  public:
             FarRegistry1 (const char *rootKeyStart, const char *rootKeyBody)
             : FarRegistry(rootKeyStart, rootKeyBody) {}
           int DeleteRegValue (const char *RootKey, const char *ValName, HKEY hRoot = HKEY_CURRENT_USER)
           {
             HKEY hKey = OpenRegKey (hRoot, RootKey, false);
             int ExitCode = RegDeleteValue (hKey, ValName);
             RegCloseKey(hKey);
             return ExitCode;
           }
        };
        FarRegistry1 reg;
#       ifndef HARDCODED_MLDATA
        IMultiLanguage2 *ml;
#       endif HARDCODED_MLDATA
        Manager():
          reg(Far::GetRootKey(), "\\FarSpell"),
          plugin_root(FarFileName(Far::GetModuleName()).GetPath()),
          spell_factory(FarFileName(plugin_root+"dict\\"), FarString(Far::GetRootKey())+"\\FarSpell")
        {
          last = NULL;
          plugin_enabled = reg.GetRegKey("", plugin_enabled_key, true);
          enable_file_settings = reg.GetRegKey("", enable_file_settings_key, true);
          highlight_list = reg.GetRegStr("", highlight_mask_key, "*.txt,*.,*.tex,*.htm,*.html,*.docbook");
          highlight_color = reg.GetRegKey("", highlight_color_key, 0x84);
          suggestions_in_menu = reg.GetRegKey("", suggestions_in_menu_key, 0x1);
          highlight_deletecolor = reg.GetRegKey("", highlight_deletecolor_key, false);
          default_dict = reg.GetRegStr("", default_dict_key, "en_US");
          FarString s(reg.GetRegStr("", oem_cp_source_key, "GetOEMCP"));
          if (s == "GetOEMCP") oem_cp_source = OEM_from_GetOEMCP;
          else if (s == "GetConsoleOutputCP")  oem_cp_source = OEM_from_GetConsoleOutputCP;
          else oem_cp_source = atoi(s.c_str());
          CheckDictionaries();
#         ifndef HARDCODED_MLDATA
          HRESULT hr;
          ml = NULL;
          hr = CoInitialize(NULL);
          if (FAILED(hr))
            GetLog().Message("CoInitialize=0x%x %s", hr, HRString(hr).c_str());
          else
          {
            hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_SERVER,
                                IID_IMultiLanguage2, (void**)&ml);
            if (FAILED(hr))
              GetLog().Message("CoCreateInstance(IMultiLanguage2)=0x%x %s", hr, HRString(hr).c_str());
          }
#         endif  HARDCODED_MLDATA
        }
        ~Manager()
        {
          reg.SetRegKey("", plugin_enabled_key, plugin_enabled);
          reg.SetRegKey("", enable_file_settings_key, enable_file_settings);
          reg.SetRegKey("", highlight_mask_key, highlight_list);
          reg.SetRegKey("", highlight_color_key, highlight_color);
          reg.SetRegKey("", suggestions_in_menu_key, suggestions_in_menu);
          reg.SetRegKey("", highlight_deletecolor_key, highlight_deletecolor);
          reg.SetRegKey("", default_dict_key, default_dict);
          switch (oem_cp_source)
          {
            case OEM_from_GetOEMCP:
              reg.SetRegKey("", oem_cp_source_key, "GetOEMCP");
              break;
            case OEM_from_GetConsoleOutputCP:
              reg.SetRegKey("", oem_cp_source_key, "GetConsoleOutputCP");
              break;
            default: {
                char buf[32];
                reg.SetRegKey("", oem_cp_source_key, itoa(oem_cp_source, buf, 10));
              }
              break;
          }
          while (last) delete last;
#         ifndef HARDCODED_MLDATA
          if (ml)
          {
            ml->Release();
            ml = NULL;
          }
          CoUninitialize();
#         endif  HARDCODED_MLDATA
          editors = NULL;
        }
        FarSpellEditor* Lookup(int id)
        {
          FarSpellEditor *editor;
          editor = last;
          while (editor)
          {
            if (id == editor->EditorId)
              return editor;
            editor = editor->prev;
          }
          return NULL;
        }
        FarLog GetLog()
        {
          return FarLog(plugin_root+"farspell.log");
        }
        int OnConfigure(int ItemNumber);
          int GeneralConfig(bool from_editor);
          int ColorSelectDialog();
          void ClearFileSettings();
        int GetCharsetEncoding(FarString &name);
        int GetOEMCP();
        int OnEvent(int Event, int Param);
        void OnMenu()
        {
           FarEdInfo fei;
           FarSpellEditor *editor = Lookup(fei.EditorID);
           if (editor)
              editor->DoMenu(fei, suggestions_in_menu);
        }
        void CheckDictionaries();
    };
    static Manager *editors;
    static void Init()
    {
      editors = new Manager();
    }
  private:
    char *default_config_string;
    int highlight;
    FarFileName file_name;
    SpellInstance* _dict_instance;
    FarString dict;
    ParserInstance* _parser_instance;
    FarString parser_id;
    int spell_enc, doc_enc, doc_tablenum, doc_ansi;
    int colored_begin, colored_end;
    enum { RS_DICT = 0x1, RS_PARSER = 0x2,
           RS_ALL = RS_DICT|RS_PARSER };
    void RecreateEngine(int what = RS_ALL);
    void DropEngine(int what = RS_ALL);
    SpellInstance* GetDict();
    ParserInstance* GetParser(FarEdInfo &fei);
  public:
    FarSpellEditor();
    ~FarSpellEditor();
    bool CheckDictionary();
    void DoMenu(FarEdInfo &fei, bool insert_suggestions);
    void Save(FarEdInfo *fei);
    void Redraw(FarEdInfo &fei, int What);
    void HighlightRange(FarEdInfo &fei, int top_line, int bottom_line);
    void ClearAndRedraw(FarEdInfo &fei);
    void ShowPreferences(FarEdInfo &fei);
    void ShowSuggestion(FarEdInfo &fei);
    void UpdateDocumentCharset(FarEdInfo &fei);
    virtual void ReportError (int line, int col, const char *expected)
    {
       editors->GetLog().Error("%s:%d:%d: expected %s", file_name.c_str(), line, col, expected);
    }
};

FarSpellEditor::Manager *FarSpellEditor::editors = NULL;

int FarSpellEditor::Manager::GetCharsetEncoding(FarString &name)
{
# ifdef HARDCODED_MLDATA
  int res = search_codepage(name);
  if (res != -1) return res;
  GetLog().Error("GetCharsetInfo: %s - not found", name.c_str());
  // TODO: notify user
# else
  wchar_t *caption; int caption_len;
  BSTR bcaption;
  MIMECSETINFO mcsi;
  HRESULT hr;
  //
  caption_len = mbstowcs(NULL, name.c_str(), name.Length());
  caption = (wchar_t *)alloca((caption_len+1)*sizeof(wchar_t));
  mbstowcs(caption, name.c_str(), name.Length());
  bcaption = SysAllocStringLen(caption, caption_len);
  hr = ml->GetCharsetInfo(bcaption, &mcsi);
  SysFreeString(bcaption);
  if (SUCCEEDED(hr))
    return mcsi.uiInternetEncoding;
  GetLog().Error("GetCharsetInfo %s: %x %s", name.c_str(), hr, HRString(hr).c_str());
  // TODO: notify user
# endif HARDCODED_MLDATA
  return GetACP();
}

int FarSpellEditor::Manager::GetOEMCP()
{
  switch (oem_cp_source)
  {
    case OEM_from_GetOEMCP:
      return ::GetOEMCP();
      break;
    case OEM_from_GetConsoleOutputCP:
      return ::GetConsoleOutputCP();
    default:
      return oem_cp_source;
  }
}

void FarSpellEditor::Manager::ClearFileSettings()
{
  reg.DeleteRegKey("", editor_files_key);
}

void FarSpellEditor::Manager::CheckDictionaries()
{
  if (!plugin_enabled) return;
  if (!spell_factory.AnyDictionaryExists())
  {
    FarMessage msg(FMSG_MB_OK, "Contents");
    msg.AddLine(MFarSpell);
    msg.AddLine(MNoDictionaries);
    msg.AddLine(MPluginWillBeDisabled);
    msg.Show();
    plugin_enabled = false;
  }
}

// config format: int highlight | string dictinoary | int parser_id
char *const config_template = "%d|%s||%s";
char *const default_config_hl = "1|en_US||*";
char *const default_config_nhl = "0|en_US||*";

FarSpellEditor::FarSpellEditor():
  dict(editors->default_dict)
{
  //============================
  next = NULL;
  if (prev = editors->last)
    editors->last->next = this;
  editors->last = this;
  //============================
  FarEdInfo fei;
  EditorId = fei.EditorID,
  //editors->GetLog().Message("Creatig 0x%X (%d)", this, EditorId);
  file_name = fei.FileName;
  highlight = 1;
  default_config_string = default_config_hl;
  parser_id = "*";
  _dict_instance = NULL;
  _parser_instance = NULL;
  colored_begin = INT_MAX;
  colored_end = 0;
  doc_tablenum = 0xCCCCCCCC;
  doc_ansi = 0xCCCCCCCC;
  doc_enc = -1;

  FarString config = editors->reg.GetRegStr(editor_files_key, file_name, "");
  if (config.Length())
  {
    //editors->GetLog().Message("%s=%s", file_name.c_str(), config.c_str());
    FarStringTokenizer tokenizer (config, '|');
    for (int i = 0; i<4 && tokenizer.HasNext(); i++) 
      switch (i)
      {
        case 0: highlight = atoi(tokenizer.NextToken()); break;
        case 1: dict = tokenizer.NextToken(); break;
        case 2: tokenizer.NextToken(); break;
        case 3: parser_id = tokenizer.NextToken(); break;
      }
  }
  else
  {
    highlight = FarSF::CmpNameList(editors->highlight_list, file_name, true);
    default_config_string = highlight ? default_config_hl : default_config_nhl;
  }
  //RecreateEngine(RS_ALL);
  if (parser_id.IsEmpty())
    parser_id = "*";
  CheckDictionary();
}

FarSpellEditor::~FarSpellEditor()
{
  Save(NULL);
  DropEngine(RS_ALL);
  //===============================
  if (this == editors->last)
  {
    editors->last = prev;
    if (next) editors->GetLog().Error("Assertion failed: 20051220 EditorId=%d next.EditorId=%d",
      EditorId, next->EditorId);
  }
  if (next)
  {
    next->prev = prev;
    next = NULL;
  }
  if (prev)
  {
    prev->next = next;
    prev = NULL;
  }
  //===============================
}

bool FarSpellEditor::CheckDictionary()
{
  bool exists = editors->spell_factory.DictionaryExists(dict);
  if (highlight && !exists)
  {
    FarMessage msg(FMSG_MB_OK, "Contents");
    msg.AddLine(MFarSpell);
    msg.AddLine(MNoDictionary);
    msg.AddLine(dict.c_str());
    msg.AddLine(MHighlightingWillBeDisabled);
    msg.Show();
    highlight = false;
  }
  return exists;
}

bool IsLegalFullPath(FarFileName &name)
{
  if (name.Length()<3) return false;
  const char c = name[0];
  if (c=='\\' && name[1]=='\\' ) 
    return true;
  if ((c>='a' && c <='z') || (c>='A' && c<='Z') 
    && name[1]==':' && name[2]=='\\')
    return true;
  return false;
}

void FarSpellEditor::Save(FarEdInfo *fei)
{
  if (!editors->enable_file_settings) return;
  if (fei) {
    if (file_name.Compare(fei->FileName)!=0) Save(NULL);
    file_name = fei->FileName;
  }
  if (!IsLegalFullPath(file_name)) {
    editors->GetLog().Message("Saving to ill file name '%s'", file_name.c_str());
#ifdef _DEBUG
    FarMessage msg(FMSG_WARNING);
    msg.AddLine("FarSpell: Debug");
    msg.AddLine("Saving to ill file name:");
    msg.AddLine(file_name.c_str());
    msg.AddButton(MOk);
    msg.Show();
#endif
    return;
  }
  int size = sizeof(config_template)+dict.Length()+parser_id.Length()+32*2;
  char *config_text = (char*)alloca(size);
  _snprintf(config_text, size-sizeof(char), config_template,
     highlight,
     dict.c_str(),
     parser_id.c_str());
  if (strcmp(default_config_string, config_text))
    editors->reg.SetRegKey(editor_files_key, file_name, config_text);
  else
    editors->reg.DeleteRegValue(editor_files_key, file_name); // reconstruction possible
}

void FarSpellEditor::DropEngine(int what)
{
  if (what&(RS_DICT))
    _dict_instance = NULL;

  if ((what&(RS_PARSER) && _parser_instance) || !_dict_instance)
  {
    delete _parser_instance;
    _parser_instance = NULL;
  }
}

void FarSpellEditor::RecreateEngine(int what)
{
  if (what&(RS_DICT))
  {
    if (CheckDictionary())
    {
      _dict_instance = editors->spell_factory.GetDictInstance(dict);
      spell_enc = editors->GetCharsetEncoding(FarString(_dict_instance->get_dic_encoding()));
    }
  }
  if (what&(RS_PARSER) && doc_enc!=-1 && GetDict())
  {
    _parser_instance = ParserFactory::CreateParserInstance(
       parser_id.c_str(), 
       doc_enc, _dict_instance->GetWordChars(), 
       file_name);
  }
}

SpellInstance* FarSpellEditor::GetDict()
{
  if (!_dict_instance) RecreateEngine(RS_DICT);
  return _dict_instance;
}

ParserInstance* FarSpellEditor::GetParser(FarEdInfo &fei)
{
  UpdateDocumentCharset(fei);
  if (!_parser_instance) RecreateEngine(RS_PARSER);
  return _parser_instance;
}

int FarSpellEditor::Manager::OnEvent(int Event, int Param)
{
  FarSpellEditor *editor;
  int i, id=-1;
  switch (Event)
   {
    case EE_READ: // Param==NULL
      new FarSpellEditor();
      break;
    case EE_SAVE: // Param==NULL
      if (plugin_enabled) {
        FarEdInfo fei;
        editor=Lookup(id=fei.EditorID);
        if (editor) editor->Save(&fei);
        else goto error;
      }
      break;
    case EE_CLOSE: // *Param==EditorId || Param==NULL;
      if (Param)
      {
        editor=Lookup(id=*(int*)Param);
        //GetLog().Message("OnEvent(%d,%d): Closing 0x%X(%d)", Event, Param, editor, id);
        if (editor) delete editor;
        else goto error;
      }
      break;
    case EE_REDRAW:
      if (plugin_enabled) {
        FarEdInfo fei;
        editor=Lookup(id=fei.EditorID);
        if (editor) editor->Redraw(fei, Param);
        else goto error;
      }
      break;
  }
  return 0;
error:
  GetLog().Error("OnEvent(%d,%d): FarSpellEditor[%d] instance not found", Event, Param, id);
  return 0;
}

void FarSpellEditor::Redraw(FarEdInfo &fei, int What)
{
#ifdef _DEBUG
  if (highlight) {
    if(!GetDict()) editors->GetLog().Message("Redraw: GetDict(`%s')==NULL");
    if(!GetParser(fei)) editors->GetLog().Message("Redraw: GetParser(id=`%s',enc=%d)==NULL", parser_id.c_str(), doc_enc);
  }
#endif  
  if (!highlight || !GetDict() || !GetParser(fei)) return;
  switch (What)
  {
     case EEREDRAW_ALL:
     case EEREDRAW_CHANGE:
       HighlightRange(fei, fei.TopScreenLine, fei.TopScreenLine+fei.WindowSizeX-1);
       break;
     case EEREDRAW_LINE:
       HighlightRange(fei, fei.CurLine, fei.CurLine);
       break;
  }
}


void FarSpellEditor::UpdateDocumentCharset(FarEdInfo &fei)
{
  if (doc_tablenum == fei.TableNum && doc_ansi == fei.AnsiMode) return;
  if (fei.TableNum != -1)
  {
    FarString doc_cs;
    CharTableSet table;
    Far::GetCharTable(fei.TableNum, &table);
    FarString keyName, far_tn(table.TableName);
    FarRegistry table_registry ("Software\\Far\\CodeTables");
    FarRegistry::KeyIterator keyIter = table_registry.EnumKeys("", HKEY_CURRENT_USER);
    while (keyIter.NextKey (keyName))
    {
      FarString tn(table_registry.GetRegStr (keyName, "TableName", "", HKEY_CURRENT_USER));
      //editors->GetLog().Message("key=%s %s", keyName.c_str(), tn.c_str());
      if (tn==far_tn)
      {
        //editors->GetLog().Message("hit!");
        doc_cs = table_registry.GetRegStr (keyName, "RFCCharset", "", HKEY_CURRENT_USER);
        break;
      }
    }
    doc_enc = editors->GetCharsetEncoding(doc_cs);
  }
  else
    doc_enc = fei.AnsiMode ? GetACP() : editors->GetOEMCP();
  doc_tablenum = fei.TableNum;
  doc_ansi = fei.AnsiMode;
  // charset changed: current parser charset incorrect.
  DropEngine(RS_PARSER);
  //editors->GetLog().Message("tn=%d doc_cs=%s spell_cs=%s spell_enc=%d doc_enc=%d", fei.TableNum,  doc_cs.c_str(), spell_cs.c_str(), spell_enc, doc_enc);
}

void FarSpellEditor::HighlightRange(FarEdInfo &fei, int top_line, int bottom_line)
{
  FarString document;
  char *token;
  FarStringW token_wide;
  
  SpellInstance *dict_inst = GetDict();
  ParserInstance *parser_inst = GetParser(fei);
  if (!dict_inst || !parser_inst) return;

  colored_begin = min(colored_begin, top_line);
  colored_end = max(colored_end, bottom_line);
  //GetLog().Message("top=%d bot=%d", top_line, bottom_line);
  for (int line_no=top_line; line_no<bottom_line; line_no++)
  {
    if (editors->highlight_deletecolor)
      FarEd::DeleteColor(line_no);
    document = FarEd::GetStringText(line_no);
    document.Delete(ParserInstance::MaxLineLength, document.Length());
    parser_inst->put_line((char*)document.c_str());
    while ((token = parser_inst->next_token()))
    {
      ToUnicode(doc_enc, token, token_wide);
      if (!dict_inst->Check(token_wide))
      {
        int pos = parser_inst->get_tokenpos();
        FarEd::AddColor(line_no, pos, pos+strlen(token)-1, editors->highlight_color);
      }
      hunspell_free(token);
    }
  }
}

void FarSpellEditor::ClearAndRedraw(FarEdInfo &fei)
{
  if (!highlight) return;
  if (editors->highlight_deletecolor && colored_begin!=INT_MAX && colored_end!=0)
    for (int i=colored_begin; i<min(colored_end, fei.TotalLines); i++)
      FarEd::DeleteColor(i);
  colored_begin=INT_MAX;
  colored_end=0;
  Redraw(fei, (int)EEREDRAW_ALL);
}

int ScanDicts(const FarString& name, void* param)
{
  FarComboBox *dlg_languages = (FarComboBox*)param;
  dlg_languages->AddItem(name);
  return 1;
}

class ParserEnumerator {
  public:
    FarComboBox *dlg_parsers;
    FarStringArray parser_ids;
};

int ScanParsers(const char* id, const char* name, void* param)
{
  ParserEnumerator *pe = (ParserEnumerator*)param;
  pe->dlg_parsers->AddItem(name);
  pe->parser_ids.Add(id);
  return 1;
}

void FarSpellEditor::ShowPreferences(FarEdInfo &fei)
{
  FarDialog dialog(MPreferences, "Contents");
  FarCheckCtrl dlg_highlight(&dialog, MHighlight, highlight);
  FarTextCtrl dlg_languages_label(&dialog, MDictianary, -1);
  FarComboBox dlg_languages(&dialog, &dict, -1, 32);
  FarString s(ParserFactory::GetParserName(parser_id));
  FarTextCtrl dlg_parser_label(&dialog, MParser, -1);
  FarComboBox dlg_parser(&dialog, &s, -1, 32);
  FarButtonCtrl dlg_ok(&dialog, MOk, DIF_CENTERGROUP);
  FarButtonCtrl dlg_cancel(&dialog, MCancel, DIF_CENTERGROUP);
  FarControl *res;
  ParserEnumerator parser_enumumerator;

  editors->spell_factory.EnumDictionaries(ScanDicts, &dlg_languages);
  parser_enumumerator.dlg_parsers = &dlg_parser;
  ParserFactory::EnumParsers(ScanParsers, &parser_enumumerator);
  dlg_languages.SetFlags(DIF_DROPDOWNLIST);
  dlg_parser.SetFlags(DIF_DROPDOWNLIST);
  dialog.SetDefaultControl(&dlg_ok);

  res=dialog.Show();

  if (res == &dlg_ok)
  {
    FarString new_dict = dlg_languages.GetText();
    const char *new_parser_id;
    if (dlg_parser.GetListPos()>=0)
      new_parser_id = parser_enumumerator.parser_ids[dlg_parser.GetListPos()];
    else
      new_parser_id = NULL;
    highlight = dlg_highlight.GetSelected();
    if (new_dict.Length() && dict != new_dict)
    {
      dict = new_dict;
      if (new_parser_id) parser_id = new_parser_id;
      DropEngine(RS_ALL);
      ClearAndRedraw(fei);
    }
    else if (new_parser_id && strcmp(new_parser_id, parser_id.c_str()) != 0)
    {
      parser_id = new_parser_id;
      DropEngine(RS_PARSER);
      ClearAndRedraw(fei);
    }
  }
}

class FarEditorSuggestList
{
    int ascii_cp;
    int tpos, tlen;
    int line;
    char *token;
    SpellInstance::WordList *word_list;
    FarSpellEditor* editor;
    FarString _word_cache;
    FarStringW current_word_wide;
    FarString current_word_oem;
  public:
    FarEditorSuggestList(FarEdInfo &fei, FarSpellEditor* _editor)
    : editor(_editor)
    {
      ascii_cp = editor->doc_enc;
      token = NULL;
      word_list = NULL;
      SpellInstance *dict_inst = editor->GetDict();
      ParserInstance *parser_inst = editor->GetParser(fei);
      if (!dict_inst || !parser_inst) return;
      int cpos = fei.CurPos;
      line = fei.CurLine;
      FarString document(FarEd::GetStringText(line));
      FarStringW token_wide;

      parser_inst->put_line((char*)document.c_str());
      while ((token = parser_inst->next_token()))
      {
        ToUnicode(ascii_cp, token, token_wide);
        if (!dict_inst->Check(token_wide))
        {
          tpos = parser_inst->get_tokenpos();
          tlen = strlen(token);
          if (cpos>=tpos && cpos<(tpos+tlen))
            break;
        }
        hunspell_free(token);
        token = NULL;
      }
      if (token)
      {
        word_list = dict_inst->Suggest(token_wide);
        hunspell_free(token);
        token = NULL;
        current_word_wide = token_wide;
      }
    }
    ~FarEditorSuggestList()
    {
      if (word_list)
      {
        delete word_list;
        word_list = NULL;
      }
      if (token)
      {
        hunspell_free(token);
        token = NULL;
      }
    }
    inline int Count() const
    {
      return word_list ? word_list->Count() : 0;
    }
    const FarString &GetWord() 
    {
      if (!current_word_oem.IsEmpty())
        return current_word_oem; 
      ToAscii(ascii_cp, current_word_wide, current_word_oem);
      FarEd::EditorToOem(current_word_oem);
      return current_word_oem;
    }
    FarString& operator[](int index) 
    {
      far_assert(word_list);
      far_assert(word_list->Count());
      far_assert(index < word_list->Count());
      far_assert(index >= 0);
      ToAscii(ascii_cp, (*word_list)[index], _word_cache);
      FarEd::EditorToOem(_word_cache);
      return _word_cache;
    }
    int Apply(FarString &word_oem)
    {
      FarEd::SetPos(line, tpos);
      for (int i=0; i<tlen; i++)
        FarEd::DeleteChar();
      FarEd::InsertText(word_oem);
      return word_oem.Length();
    }
    int Apply(int index) 
    {
      far_assert(word_list);
      far_assert(word_list->Count());
      far_assert(index < word_list->Count());
      far_assert(index >= 0);
      FarString s;
      ToAscii(ascii_cp, (*word_list)[index], s);
      FarEd::EditorToOem(s);
      return Apply(s);
    }
};

class SuggestionDialog: Suggestion
{
  private:
    FarEditorSuggestList &sl;
    ftl::ListboxItems lbi;
  public:
    static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
    {
      SuggestionDialog *pThis = (SuggestionDialog *)Far::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
      switch (Msg) 
      {
        case DN_INITDIALOG:
           Far::SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
           break;
        case DN_LISTCHANGE:
           if (pThis && Param2>=0)
             Far::SendDlgMessage(hDlg, DM_SETTEXTPTR, Index_ID_S_Word, 
               (long)pThis->sl[Param2].c_str());
      }
      return Far::DefDlgProc(hDlg, Msg, Param1, Param2);
    }
    SuggestionDialog(FarEditorSuggestList &_sl, bool stop_button)
    : sl(_sl), lbi(&sItems[Index_ID_S_WordList])
    {
      strncpy(sItems[Index_ID_S_Word].Data, sl.GetWord().c_str(), sizeof(sItems[0].Data));
      for (int i = 0; i < sl.Count(); i++)
        lbi.AddItem(sl[i].c_str());
      if (!stop_button)
        sItems[Index_MStop].Flags |= DIF_DISABLE;

      lbi.BeforeShow();
      int idx = ShowEx(0, DlgProc, (long)this);
      switch (ItemId(idx)) 
      {
        case MSkip: break;
        case ID_S_Word:
          sl.Apply(FarString((char*)&sItems[Index_ID_S_Word].Data));
          break;
        case ID_S_WordList:
        case MReplace: 
          if ((idx=lbi.GetListPos()) >= 0)
            sl.Apply(idx);
          break;
        case MStop: break;
      }
    }
};

void FarSpellEditor::ShowSuggestion(FarEdInfo &fei)
{
  FarEditorSuggestList sl(fei, this);
  SuggestionDialog sd(sl, false);
}

void FarSpellEditor::DoMenu(FarEdInfo &fei, bool insert_suggestions)
{
  FarEditorSuggestList *sl 
   = insert_suggestions && editors->plugin_enabled 
     ? new FarEditorSuggestList(fei, this) : NULL;
  FarMenuEx menu(MFarSpell, FMENU_WRAPMODE, "Contents");
  int i, static_part = 0;

  if (sl && sl->Count())
  {
    for (int j = 0; j < sl->Count(); j++) 
      menu.AddItem((*sl)[j]);
    menu.AddSeparator();
    static_part = sl->Count()+1;
  }
  menu.AddItem(MSuggestion);
  i = menu.AddItem(MPreferences);
  if (!editors->plugin_enabled)
    menu.DisableItem(i);

  menu.AddItem(MEditorGeneralConfig);

  int res = menu.Show();

  if (res==-1)
  {
    //pass
  }
  else if (sl && sl->Count() && res<sl->Count())
  {
    sl->Apply(res);
  }
  else
  {
    switch (res-static_part)
    {
      case 0: ShowSuggestion(fei); break;
      case 1: ShowPreferences(fei); break;
      case 2: editors->GeneralConfig(true); break;
    }
  }
  if (sl)
  {
    delete sl;
    sl = NULL;
  }
}

int GetRadioStatus(struct FarDialogItem* pItems, int nItems, int nItem)
{
  int nSelected = 0;
  for (pItems+=nItem; pItems->Type == DI_RADIOBUTTON; pItems++, nSelected++)
  {
    if (pItems->Selected) return nSelected; 
  }
  return -1;
}

class ColorDialog: ColorSelect
{
  int nCurrentColor;
  static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
  {
    int id;
    ColorDialog *pThis = (ColorDialog *)Far::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
    switch (Msg) 
    {
      case DN_INITDIALOG:
         Far::SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
         break;
      case DN_CTLCOLORDLGITEM: 
         if (Param1 == Index_243) 
            return pThis->nCurrentColor;
         break;
      case DN_BTNCLICK: 
         id = ItemId(Param1);
         if (id&0x100) 
         {
           pThis->nCurrentColor = pThis->nCurrentColor&0xF0 | id&0x0F;
         }
         else if (id&0x200) 
         {
           pThis->nCurrentColor = pThis->nCurrentColor&0x0F | id&0xF0;
         }
         break;
    }
    return Far::DefDlgProc(hDlg, Msg, Param1, Param2);
  }
  public: void Select(int &nColor)
  {
    int col1 = (nColor&0x0F);
    int col2 = (nColor&0xF0)>>4;
    FarDialogItem* pItem;
    // Установить текущий цвет 
    pItem = &sItems[Index_0x100+col1];
    pItem->Selected = 1;
    pItem->Focus = 1;
    sItems[Index_0x200+col2].Selected = 1;
    // Установить цвет для "образца"
    pItem = &sItems[Index_243];
    pItem->Flags = (pItem->Flags&~DIF_COLORMASK) | nColor;
    nCurrentColor = nColor;
    if (ShowEx(0, DlgProc, (long)this) != -1)
      // Если диалог не был отвергнут
      // то применить новые цвета.
      nColor = nCurrentColor;
  }
};

int FarSpellEditor::Manager::ColorSelectDialog()
{
  ColorDialog().Select(highlight_color);
  return FALSE;
}



class GeneralConfigDialog: GeneralConfig
{
  static int ScanDicts(const FarString &name, void* param)
  {
    ftl::ComboboxItems *dicts = (ftl::ComboboxItems*)param;
    dicts->AddItem(name);
    return 1;
  }
  public: void Execute(FarSpellEditor::Manager *m, bool from_editor)
  {
    FarDialogItem* pItem;
    int res;
    char *p =  sItems[Index_ID_GC_SpellExts].Data;
    strcpy(p, m->highlight_list.c_str());
    sItems[Index_MPluginEnabled].Selected = m->plugin_enabled;
    sItems[Index_MSuggMenu].Selected = m->suggestions_in_menu;
    sItems[Index_MAnotherColoring].Selected = !m->highlight_deletecolor;
    sItems[Index_MFileSettings].Selected = m->enable_file_settings;
    if (from_editor)
      sItems[Index_MUnloadDictionaries].Flags |= DIF_DISABLE;
    ftl::ComboboxItems default_dict_items(&sItems[Index_ID_GC_DefaultDict]);
    default_dict_items.SetText(m->default_dict);
    m->spell_factory.EnumDictionaries(ScanDicts, &default_dict_items);
    for (int cont=1; cont;) 
    {
      default_dict_items.BeforeShow();
      res = Show(0);
      switch (ItemId(res))
      {
        case MColorSelectBtn:
          m->ColorSelectDialog();
          break;
        case MUnloadDictionaries:
          m->spell_factory.UnloadDictionaries();
          break;
        case -1: // cancel
          cont = 0;
          break;
        case MOk:
        default:
          {
            int l = strlen(p);
            char* news = m->highlight_list.GetBuffer(l);
            strncpy(news, p, l);
            m->highlight_list.ReleaseBuffer(l);
            m->suggestions_in_menu = sItems[Index_MSuggMenu].Selected; 
            m->highlight_deletecolor = !sItems[Index_MAnotherColoring].Selected;
            m->plugin_enabled = sItems[Index_MPluginEnabled].Selected;
            m->enable_file_settings = sItems[Index_MFileSettings].Selected;
            m->default_dict = default_dict_items.GetText();
          }
          cont = 0;
      }
    }
  }
};

int FarSpellEditor::Manager::GeneralConfig(bool from_editor)
{
  bool last_enable_file_settings = enable_file_settings;
  GeneralConfigDialog().Execute(this, from_editor);
  CheckDictionaries();
  if (!enable_file_settings && last_enable_file_settings)
  {
    FarMessage msg(FMSG_MB_YESNO);
    msg.AddLine(MGeneralConfig);
    msg.AddLine(MFileSettingWasDisabled);
    msg.AddLine(MWillDeleteFileSetting);
    if (msg.Show()==0)
    {
      ClearFileSettings();
      FarMessage().SimpleMsg(FMSG_MB_OK, MGeneralConfig, MFileSettingsRemoved, -1);
    }
  }
  return FALSE;
}

int FarSpellEditor::Manager::OnConfigure(int ItemNumber)
{
  return GeneralConfig(false);
}

// -- Exported plugin functions ----------------------------------------------
#ifdef _DEBUG
class ExcDump
{
  public:
    int code;
    static bool no_more;
    ExcDump(PEXCEPTION_POINTERS pExInfo)
    {
      try
      {
        {
          if (!no_more)
          {
            FarMessage wait( FMSG_WARNING );
            wait.AddFmt( MExceptionIn, FarFileName(Far::GetModuleName()).GetNameExt().c_str());
            wait.AddLine( MDbgPleaseWait );
            wait.Show();
          }
          FarDumpStackCb(pExInfo, sDump, this);
          ToLog();
        }
        ToMessage();
      }
      catch(...)
      {
      }
      code = EXCEPTION_EXECUTE_HANDLER;
    }
    void ToLog()
    {
      FarLog log(FarString(Far::GetModuleName())+".err!");
      log.Error("exception");
      log.Write(dump.c_str(), dump.Length());
    }
    void ToMessage()
    {
      if (no_more) return;
      FarMessage Msg( FMSG_WARNING|FMSG_LEFTALIGN, "Exception" );
      FarStringTokenizer lines(dump, '\r');
      int limit = 15, skip_lf=0;
      Msg.AddFmt( MExceptionIn, FarFileName(Far::GetModuleName()).GetNameExt().c_str());
      // lines.NextToken(); // skip
      while (limit-- && lines.HasNext())
      {
        Msg.AddLine( &lines.NextToken().c_str()[skip_lf] );
        skip_lf=1;
      }
      Msg.AddFmt( MDbgSeeMore, Far::GetModuleName());
      Msg.AddSeparator();
      Msg.AddButton( MDbgContinue );
      Msg.AddButton( MNoDbgMessages );
      Msg.AddButton( MUnloadPlugin );
      Msg.AddButton( MExitFAR );

      switch (Msg.Show())
      {
        case -1:
        case 0:  // Continue
          break;
        case 1:  // Don''t disturb me
          no_more = 1;
          break;
        case 2: // Unload plugin
          //FarCtrl().ClosePlugin(NULL);
          delete FarSpellEditor::editors;
          Far::Done();
          break;
        case 3: // Exit FAR
          ExitThread(0);
          break;
      }
    }
  private:
    FarString dump;
    void Dump(void* param, const char* Fmt, va_list Args)
    {
      char tmp[1024];
      wvsprintf( tmp, Fmt, Args );
      dump+=tmp;
      /*if (str.Length() && str[str.Length()-1] == '\n')
      {
        log.Error("%s", str.c_str());
        str.Empty();
      }*/
    }
    static void sDump(void* const param, const char* Fmt, ...)
    {
      ExcDump *const pThis = (ExcDump*)param;
      va_list argPtr;
      va_start( argPtr, Fmt );
      pThis->Dump(param, Fmt, argPtr);
      va_end( argPtr );
    }
};
bool ExcDump::no_more = false;

int far_exc_dump(PEXCEPTION_POINTERS pExInfo)
{
  try
  {
    return ExcDump(pExInfo).code;
  }
  catch(...)
  {
    return EXCEPTION_EXECUTE_HANDLER;
  }
}
#endif _DEBUG

int WINAPI _export SetLngMode() //Exported for DialogGenerator
{
  return 0;
}

void WINAPI _export SetStartupInfo (const struct PluginStartupInfo *Info)
{
#ifdef _DEBUG
  __try
  {
#endif _DEBUG
    Far::Init (Info, PF_EDITOR|PF_DISABLEPANELS);
    Far::AddPluginMenu (MFarSpell);
    Far::AddPluginConfig (MFarSpell);
    FarSpellEditor::Init();
#ifdef _DEBUG
  }
  __except(far_exc_dump(GetExceptionInformation()))
  {
  }
#endif _DEBUG
}

HANDLE WINAPI _export OpenPlugin (int /*OpenFrom*/, int /*Item*/)
{
  if (FarSpellEditor::editors) 
#ifdef _DEBUG
  __try
  {
#endif _DEBUG
    FarSpellEditor::editors->OnMenu();
#ifdef _DEBUG
  }
  __except(far_exc_dump(GetExceptionInformation()))
  {
  }
#endif _DEBUG
  return INVALID_HANDLE_VALUE;
}

void WINAPI _export ExitFAR()
{
  if (FarSpellEditor::editors)
  {
    delete FarSpellEditor::editors;
    Far::Done();
  }
}

void WINAPI _export GetPluginInfo (struct PluginInfo *Info)
{
  Far::GetPluginInfo (Info);
}

int WINAPI _export Configure(int ItemNumber)
{
#ifdef _DEBUG
  __try
  {
#endif _DEBUG
    return FarSpellEditor::editors->OnConfigure(ItemNumber);
#ifdef _DEBUG
  }
  __except(far_exc_dump(GetExceptionInformation()))
  {
  }
  return 0;
#endif _DEBUG
}


int WINAPI _export ProcessEditorEvent(int Event, int Param)
{
#ifdef _DEBUG
  __try
  {
#endif _DEBUG
    return FarSpellEditor::editors?FarSpellEditor::editors->OnEvent(Event, Param):0;
#ifdef _DEBUG
  }
  __except(far_exc_dump(GetExceptionInformation()))
  {
  }
  return 0;
#endif _DEBUG
}
