/*
    Spell-checker plugin for FAR: header file.
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

/* -- Language strings -------------------------------------------------------*/

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
  MSpellcheckDirection, 
  MSpellcheckAction,
  MOnlySetCursor,
  MSpellcheck,
  MForwardSpellcheck,
  MBackwardSpellcheck,
  MSpellcheckArea, 
  MSpellcheckEntireText,
  MSpellcheckFromCursor, 
  MSpellcheckSelection,
  MReplace,
  MSkip,
  MStop,
  MSpellcheckDone,
  MPreferences,
  MEditorGeneralConfig,
  MUnloadDictionaries,

  MHighlight,
  MDictionary0,
  MDictionary,
  MParser,

  MGeneralConfig,
  MPluginEnabled,
  MDefaultDict,
  MSpellExts,
  MDictPath,
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

  MEditDictViews,
  MSelectDictView,
  MDictViewsEditor,
  MDictViewName,
  MCondition,
  MAction,
  MDictParams,
  MSave,

  MActionEditor,
  MActionEnabled,
  MShowSuggestion,
  MUpDown,

  MDictionaryProperties,
  MAlphabet,
  MWordPuncts,
  MTransliterationEnabled,
  MTransliterateFrom,
  MTransliterateTo,
  MTransliterationIsError,


  MReturnToMenu,

  M_FMT_AUTO_TEXT,
  M_FMT_TEXT,
  M_FMT_LATEX,
  M_FMT_HTML,
  M_FMT_MAN,
  M_FMT_FIRST,
};

enum {
  ID_GC_SpellExts=300,
  ID_GC_DictPath,
  ID_GC_DefaultDict,
  ID_S_Word,
  ID_S_WordList,
  ID_Name,
  ID_CB1, ID_CB2, ID_CB3, ID_CB4,
  ID_BTN1, ID_BTN2, ID_BTN3, ID_BTN4,
};
