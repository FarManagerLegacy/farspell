/*
    Spell-checker plugin for FAR: main header.
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

#include <farcolor.hpp>

#include "FARPlus/FARMenu.h"
#include "FARPlus/FARMemory.h"
#include "FARPlus/FARLog.h"
#include "FARPlus/FARPlus.h"
#include "FARPlus/FARRegistry.h"
#include "FARPlus/FARDialog.h"
#include "FARPlus/FAREd.h"
//#include "FARPlus/FARXml.h"
//#include "FARPlus/FARDbg.h"
#include "FARPlus/FARFile.h"

// message ids:
#include "FarSpell.h"

int search_codepage(FarString &name);

// class SpellFactory:
#include "SpellFactory.hpp"

// class ParserFactory:
#include "ParserFactory.hpp"

#define _INTERFACE
#include "DictViewFactory.cpp"

enum { // for spellcheck_area
  sa_entire_text, sa_from_cursor, sa_selection, 
  sa_last = sa_selection 
}; 

class FarEditorSuggestList;
class FarSpellEditor
{
  friend class CurrentSettingsDialog;
  friend class FarEditorSuggestList;
  friend class SuggestionDialog;
  public:
    int EditorId;
    FarSpellEditor *next, *prev;
    class Manager
    {
      public:
        FarSpellEditor *last;
        FarFileName plugin_root;      // keep order 1
        FarFileName dictionary_path;  // keep order 2
        SpellFactory spell_factory;   // keep order 3
        FarString highlight_list;
        bool plugin_enabled;
        bool enable_file_settings;
        int highlight_color;
        bool suggestions_in_menu;
        bool highlight_deletecolor; 
        int oem_cp_source; enum { OEM_from_GetOEMCP,  OEM_from_GetConsoleOutputCP };
        bool spellcheck_forward;
        bool spellcheck_suggestion;
        int spellcheck_area; 
        FarString default_dict;
        bool return_from_dictionary_menu;
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
        DictViewFactory dict_view_factory;
#       ifndef HARDCODED_MLDATA
        IMultiLanguage2 *ml;
#       endif HARDCODED_MLDATA
        Manager();
        ~Manager();
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
          void DictionaryViewSelect(bool edit_by_enter);
          void EditDictionaryView(DictViewInstance *dict_view);
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
    void Spellcheck(FarEdInfo &fei);
    void UpdateDocumentCharset(FarEdInfo &fei);
    virtual void ReportError (int line, int col, const char *expected)
    {
       editors->GetLog().Error("%s:%d:%d: expected %s", file_name.c_str(), line, col, expected);
    }
};

/* translate unicode string to ascii string:
     use ranges (e.g. a-zA-Z etc.)
     escape unicode chars outside specified code_page (e.g. U+2019 etc)
     escape special chars ('-', ',', 'U', '\' <-> "\-", "\,", "\U", "\\")
  FIXME: not compatible with Custom CodePage library, when 
  FIXME: standart code page was customized.
*/
FarString EscapeUnicode(const FarStringW &w, UINT code_page);
FarStringW UnescapeUnicode(const FarString &e, UINT code_page);
