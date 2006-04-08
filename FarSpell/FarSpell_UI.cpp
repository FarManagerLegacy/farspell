/*
    Spell-checker plugin for FAR: User interface.
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

// Dialog primitives:
#include "ftl.hpp"

// Generated dialogs:
#include "dialogs.cpp"

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
  FarTextCtrl dlg_languages_label(&dialog, MDictionary, -1);
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
      document.Delete(ParserInstance::MaxLineLength, document.Length());
      FarStringW token_wide;

      parser_inst->put_line((char*)document.c_str());
      while ((token = parser_inst->next_token()))
      {
        ToUnicode(ascii_cp, token, token_wide);
        //if (!dict_inst->Check(token_wide)) TODO? configurable.
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
    FarEditorSuggestList(const FarEdInfo &fei, FarSpellEditor* _editor, const char *_token, int _len)
    : editor(_editor)
    {
      ascii_cp = editor->doc_enc;
      token = NULL;
      word_list = NULL;
      line = fei.CurLine;
      tpos = fei.CurPos;
      tlen = _len;
      SpellInstance *dict_inst = editor->GetDict();
      if (!dict_inst) return;
      ToUnicode(ascii_cp, _token, _len, current_word_wide);
      word_list = dict_inst->Suggest(current_word_wide);
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

class SuggestionDialog: public SuggestionSkel
{
  private:
    FarEditorSuggestList &sl;
    ftl::ListboxItems lbi;
    int edit_unchanged_flag;
  public: 
    enum dlg { skip=-1, stop=-2, /* positive value is new word length */ };
    private: static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
    {
      SuggestionDialog *pThis = (SuggestionDialog *)Far::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
      switch (Msg) 
      {
        case DN_INITDIALOG:
           Far::SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
           break;
        case DN_CLOSE:
           pThis->edit_unchanged_flag = 
             Far::SendDlgMessage(hDlg, DM_EDITUNCHANGEDFLAG, Index_ID_S_Word, -1);
           break;
        case DN_LISTCHANGE:
           if (pThis && Param2>=0)
             Far::SendDlgMessage(hDlg, DM_SETTEXTPTR, Index_ID_S_Word, 
               (long)pThis->sl[Param2].c_str());
      }
      return Far::DefDlgProc(hDlg, Msg, Param1, Param2);
    }
    public: SuggestionDialog(FarSpellEditor *fse, FarEditorSuggestList &_sl, 
                             bool stop_button)
    : sl(_sl), lbi(&sItems[Index_ID_S_WordList])
    {
      strncpy(sItems[Index_ID_S_Word].Data, sl.GetWord().c_str(), sizeof(sItems[0].Data));
      strncpy(sItems[Index_MDictionary].Data, fse->dict.c_str(), sizeof(sItems[0].Data));
      for (int i = 0; i < sl.Count(); i++)
        lbi.AddItem(sl[i].c_str());
      if (!stop_button)
        sItems[Index_MStop].Flags |= DIF_DISABLE;
    }
    private: int AfterShow(int item_index)
    {
      switch (ItemId(item_index)) 
      {
        case MSkip: 
          return skip;
        case ID_S_Word:
          if (edit_unchanged_flag)
            return skip;
          else
            return sl.Apply(FarString((char*)&sItems[Index_ID_S_Word].Data));
        case ID_S_WordList:
        case MReplace: 
          int idx;
          if ((idx=lbi.GetListPos()) >= 0)
            return sl.Apply(idx);
          else
            return skip;
        case MStop: 
        default:
          return stop;
      }
    }
    public: int Execute()
    {
      lbi.BeforeShow();
      int idx = ShowEx(0, DlgProc, (long)this);
      return AfterShow(idx);
    }
    public: int Execute(int X, int Y)
    {
      lbi.BeforeShow();
      int idx = ShowEx(X, Y, 0, DlgProc, (long)this);
      return AfterShow(idx);
    }
};

void FarSpellEditor::ShowSuggestion(FarEdInfo &fei)
{
  FarEditorSuggestList sl(fei, this);
  SuggestionDialog sd(this, sl, false);
  sd.Execute();
}

class SpellcheckDialog: SpellcheckSkel
{
  public:
    static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
    {
      static const struct { char c; int idx; } HotKeyMap[] = {
        { 'F', Index_MForwardSpellcheck },
        { 'B', Index_MBackwardSpellcheck },
        { 'S', Index_MSuggestion },
        { 'C', Index_MOnlySetCursor },
        { 'T', Index_MSpellcheckEntireText },
        { 'R', Index_MSpellcheckFromCursor },
        { 'L', Index_MSpellcheckSelection },
      };
      switch (Msg) 
      {
        case DN_BTNCLICK: {
            BOOL value;
            bool enable_changed = false;
            if (Param1==Index_MSpellcheckSelection) {
              enable_changed = true;
              value = FALSE;
            } else if (Param1==Index_MSpellcheckEntireText 
                     || Param1==Index_MSpellcheckFromCursor) {
              enable_changed = true;
              value = TRUE;
            }
            if (enable_changed) {
              Far::SendDlgMessage(hDlg, DM_ENABLE, Index_MSpellcheckDirection, value);
              Far::SendDlgMessage(hDlg, DM_ENABLE, Index_MForwardSpellcheck, value);
              Far::SendDlgMessage(hDlg, DM_ENABLE, Index_MBackwardSpellcheck, value);
              Far::SendDlgMessage(hDlg, DM_ENABLE, Index_Hot_F, value);
              Far::SendDlgMessage(hDlg, DM_ENABLE, Index_Hot_B, value);
            }
          }
          break;
        case DN_HOTKEY:
          char const c = Param2&0xFF;
          for (int i = 0; i<sizeof(HotKeyMap)/sizeof(HotKeyMap[0]); i++)
            if (HotKeyMap[i].c == c) {
              Far::SendDlgMessage(hDlg, DM_SETFOCUS, HotKeyMap[i].idx, 0);
              Far::SendDlgMessage(hDlg, DM_SETCHECK, HotKeyMap[i].idx, BSTATE_CHECKED);
              return FALSE;
            }
          break;
      }
      return Far::DefDlgProc(hDlg, Msg, Param1, Param2);
    }
    bool Execute(FarEdInfo &fei, FarSpellEditor* e)
    {
      if (e->editors->spellcheck_forward)
        sItems[Index_MForwardSpellcheck].Selected = 1;
      else
        sItems[Index_MBackwardSpellcheck].Selected = 1;
      if (e->editors->spellcheck_suggestion)
        sItems[Index_MSuggestion].Selected = 1;
      else
        sItems[Index_MOnlySetCursor].Selected = 1;
      if (fei.BlockType==BTYPE_NONE) {
        sItems[Index_MSpellcheckSelection].Flags |= DIF_DISABLE;
        //sItems[Index_Hot_L].Flags |= DIF_DISABLE; // не срабатывает Alt+R
      }
      switch (e->editors->spellcheck_area) {      
        case sa_selection:
          if (fei.BlockType!=BTYPE_NONE) {
            sItems[Index_MSpellcheckSelection].Selected = 1;
            sItems[Index_MSpellcheckDirection].Flags |= DIF_DISABLE;
            sItems[Index_MForwardSpellcheck].Flags |= DIF_DISABLE;
            sItems[Index_MBackwardSpellcheck].Flags |= DIF_DISABLE;
            sItems[Index_Hot_F].Flags |= DIF_DISABLE;
            sItems[Index_Hot_B].Flags |= DIF_DISABLE;
            break;
          }  // fall throught
        case sa_entire_text:
          sItems[Index_MSpellcheckEntireText].Selected = 1;
          break;
        case sa_from_cursor:
          sItems[Index_MSpellcheckFromCursor].Selected = 1;
          break;
      }
      if (ShowEx(0, DlgProc, 0)>=0) {
        e->editors->spellcheck_forward = sItems[Index_MForwardSpellcheck].Selected;
        e->editors->spellcheck_suggestion = sItems[Index_MSuggestion].Selected;
        e->editors->spellcheck_area = ftl::GetRadioStatus(sItems, NItems, Index_MSpellcheckEntireText);
        far_assert(e->editors->spellcheck_area>=0);
        far_assert(e->editors->spellcheck_area<=sa_last);
        if (fei.BlockType==BTYPE_NONE && e->editors->spellcheck_area == sa_selection) {
          FarMessage msg;
          msg.AddLine(MFarSpell);
          msg.AddLine(MSpellcheckDone);
          msg.AddButton(MOk);
          msg.Show();
          return false;
        }
        return true;
      }
      return false;
    }
};

struct Token 
{ 
  // слово в строке задаётся интервалом [begin, end=begin+len) :
  const char* begin; // указатель на подстроку, не оканчивется нулём.
                     // == full_str+pos
  const char* end;   // указатель конца подстроки, в слово не включется.
  unsigned len;     // длина слова.
  unsigned pos;     // положение слова в строке.
  //int level;        // код для мультисловарей.
};

void FarSpellEditor::Spellcheck(FarEdInfo &fei)
{
  SpellcheckDialog sc;
  if (!sc.Execute(fei, this)) return;
#if 0
  {
    FarMessage msg(0);
    msg.AddLine("Sorry, not yet implemented.");
    msg.AddButton(MOk);
    msg.Show();
    return;
  }
#endif
  SpellInstance *const dict_inst = GetDict();
  ParserInstance *const parser_inst = GetParser(fei);
  if (!dict_inst || !parser_inst) return;
  // Направление поиска ошибки:
  int dir_step = editors->spellcheck_forward? +1 : -1; 
  int top_line, bottom_line;
  FarDataArray<Token> tokens; // массив проверяемых слов в строке.
  FarString document;
  FarStringW token_wide;
  bool loop = true;  // проверять ещё?
  bool text_terminated = false; // достигли границы текста
  bool line_block = false; // ищем во многострочном блоке?
  bool rect_block = false; // ищем в прямоугольном блоке?
  bool selection = false; // ищем только в блоке?
  FarEdString fes;
  
  far_assert(dir_step);

  switch (editors->spellcheck_area) {
    case sa_entire_text:
      top_line = 0;
      bottom_line = fei.TotalLines-1;
      if (dir_step>0) {
        FarEd::SetPos(fei.CurLine = top_line, fei.CurPos = 0);
        fes.Update();
      } else {
        FarEd::SetPos(fei.CurLine = bottom_line, -1);
        fes.Update();
        FarEd::SetPos(-1, fei.CurPos = fes.StringLength);
      }
      break;
    case sa_from_cursor:
      if (dir_step>0) {
        top_line = fei.CurLine;
        bottom_line = fei.TotalLines-1;
      } else {
        top_line = 0;
        bottom_line = fei.CurLine;
      }
      break;
    case sa_selection:
      far_assert(fei.BlockType!=BTYPE_NONE);
      selection = true;
      dir_step = +1;
      if (fei.BlockType==BTYPE_STREAM) line_block = true;
      else rect_block = true;
      top_line = fei.BlockStartLine;
      bottom_line = fei.TotalLines-1;
      FarEd::SetPos(fei.CurLine = top_line, -1);
      fes.Update();
      FarEd::SetPos(-1, fei.CurPos = fes.SelStart);
      break;
  }

  document.SetText(fes.StringText, min(fes.StringLength, ParserInstance::MaxLineLength));
  // skip_left - символ до которого проверка не происходит. 
  // skip_right символ после которого проверка не происходит.
  const char *skip_left = document.c_str()+
    ((selection && fes.SelStart>=0) ? fes.SelStart : 0);
  const char *skip_right = document.c_str()+
    ((selection && fes.SelEnd>=0) ? fes.SelEnd : document.Length()-1);
  const char *skip_begin = editors->spellcheck_area==sa_from_cursor?
    document.c_str()+fei.CurPos : NULL; 
  // skip_begin задаёт начало слова, которое уже не нужно проверять.

  for(;;) /* будем выходить по средине цикла. */ { 
    parser_inst->put_line((char*)document.c_str());
    // Соберём все слова текущей строки в массив tokens:
    tokens.Clear(); 
    while (char *token = parser_inst->next_token()) 
    {
      struct Token s;
      s.pos = parser_inst->get_tokenpos();
      s.begin = s.end = document.c_str() + s.pos;
      s.end += s.len = strlen(token);
      tokens.Add(s);
      hunspell_free(token);
    } // while (token = ...
    // token_begin --- начало массива слов.
    // token_end --- конец массива слов, который не входит в массив.
    Token *const token_begin = tokens.GetItems(); 
    Token *const token_end = token_begin+tokens.Count(); 
    // пройдёмся по всем словам:
    for (Token *token = (dir_step>0) ? token_begin : token_end-1; 
         loop && token>=token_begin && token<token_end; // условие, проверяется
         token += dir_step)                                      // всегда.
     if ( ( !skip_begin )
       || ( dir_step>0 && token->begin > skip_begin ) 
       || ( dir_step<0 && token->end < skip_begin ) 
        )
     if ( ( token->begin >= skip_left ) 
       && ( token->begin <= skip_right ) 
        )
    {
      ToUnicode(doc_enc, token->begin, token->len, token_wide);
      if (!dict_inst->Check(token_wide))
      {
        FarEd::SetPos(-1, fei.CurPos=token->pos);
        if (editors->spellcheck_suggestion)
        {
          bool show_upper;
          if (fei.CurLine<(fei.TotalLines-fei.WindowSizeY-1)) {
            FarEd::SetViewPos(fei.CurLine-1-3, -1);
            show_upper = false;
          } else {
            FarEd::SetViewPos(fei.CurLine-fei.WindowSizeY+2+3, -1);
            show_upper = true;
          }
          FarEd::Redraw(); // без этого не отображается FarEd::InsertText, SetViewPos
          FarEditorSuggestList sl(fei, this, token->begin, token->len);
          SuggestionDialog sd(this, sl, true);
          int result = sd.Execute(-1, 
            show_upper ? 1 : fei.WindowSizeY-SuggestionDialog::Height);
          if (result==SuggestionDialog::stop) 
            loop = false; // сказали прекратить проверку.
          else if (result>=0 && !(fei.CurState&ECSTATE_LOCKED))  {
            if (int const fixup = result - token->len) 
            { // размер слова изменился:
              token->len = result;
              // пересчитаем положение всех следующих слов:
              for (Token *t = token+1; t<token_end; t++) 
                t->pos += fixup;
              // а указатели пересчитаем позже.
            } // if (fixup)...
            // содержимое строки изменили средствами FarEd, перечитаем:
            {
              fes.Update();
              int const left = skip_left - document.c_str();
              document.SetText(fes.StringText, min(fes.StringLength, ParserInstance::MaxLineLength));
              skip_left = document.c_str() + left;
              skip_right = document.c_str() +
              ((selection && fes.SelEnd>=0) ? fes.SelEnd : document.Length()-1);
            }
            // указатель document.c_str() мог изменится,
            // а указатели следующих слов сдвинулись на fixup символов, 
            // пересчитаем:
            const char* doc = document.c_str();
            for (Token *t = token_begin; t<token_end; t++) {
              t->begin = t->end = doc + t->pos;
              t->end += t->len;
            }
          } // if result>=0...
        } else 
          loop = false; // диалог с советом не показывали, значит прекращаем.
      } // if ...
      skip_begin = token->begin; 
    } // for (Token *token =...
    if (!loop) break;
    // достигли края документа?
    if (dir_step>0) {
       if (fei.CurLine>=bottom_line)
        text_terminated = true;
    } else { 
      if (fei.CurLine<=top_line)
        text_terminated = true;
    }  
    if (text_terminated) break;
    // переход на следующую строку:
    FarEd::SetPos(fei.CurLine += dir_step, fei.CurPos = 0);
    fes.Update();
    if (selection && fes.SelStart==-1) {
      text_terminated = true;
      break;
    }
    document.SetText(fes.StringText, min(fes.StringLength, ParserInstance::MaxLineLength));
    skip_left = document.c_str()+
      ((selection && fes.SelStart>=0) ? fes.SelStart : 0);
    skip_right = document.c_str()+
      ((selection && fes.SelEnd>=0) ? fes.SelEnd : document.Length()-1);
    skip_begin = NULL; // ничего не будем игнорировать.
  } // for (;;)
  if (text_terminated)
  { // достигли края документа.
    FarEd::Redraw(); // без этого не отображается FarEd::InsertText, SetViewPos
    FarMessage msg;
    msg.AddLine(MFarSpell);
    msg.AddLine(MSpellcheckDone);
    msg.AddButton(MOk);
    msg.Show();
  }
}

void FarSpellEditor::DoMenu(FarEdInfo &fei, bool insert_suggestions)
{
  FarEditorSuggestList *sl 
   = insert_suggestions && editors->plugin_enabled 
     ? new FarEditorSuggestList(fei, this) : NULL;
  FarMenuT<FarMenuItemEx> menu(MFarSpell, FMENU_WRAPMODE, "Contents");
  int i, static_part = 0;
  static const char Numbers[] = "`1234567890-=~!@#$%^&*()_+";
  if (sl && sl->Count())
  {
    for (int j = 0; j < sl->Count(); j++) 
      menu.AddItem((j<sizeof(Numbers)-1)? Numbers[j] : 'x', (*sl)[j]);
    menu.AddSeparator();
    static_part = sl->Count()+1;
  }
  menu.AddItem('S', MSuggestion);
  menu.AddItem('C', MSpellcheck);
  i = menu.AddItem('P', MPreferences);
  if (!editors->plugin_enabled)
    menu.DisableItem(i);

  menu.AddItem('G', MEditorGeneralConfig);
  menu.SetBottomLine(dict);

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
      case 1: Spellcheck(fei); break;
      case 2: ShowPreferences(fei); break;
      case 3: editors->GeneralConfig(true); break;
    }
  }
  if (sl)
  {
    delete sl;
    sl = NULL;
  }
}

class ColorDialog: ColorSelectSkel
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



class GeneralConfigDialog: GeneralConfigSkel
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
      res = ItemId(Show(0));
      if (res<0) cont = 0; // cancel
      else switch (res)
      {
        case MColorSelectBtn:
          m->ColorSelectDialog();
          break;
        case MEditDictViews:
          m->DictionaryViewSelect(/*edit_by_enter=*/true);
          break;
        case MUnloadDictionaries:
          m->spell_factory.UnloadDictionaries();
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

class DictViewEditorDialog: public DictViewEditorSkel
{
  DictViewInstance *dict_view;
  DecisionTable *dt;
  ftl::ComboboxItems conditions;
  int n_conditions;
  enum { MaxConditions = 4, MaxRules = 8, RuleMask = 0xFF };
  static inline int CellIndex(int r, int c) { return ItemIndex(0x2000|(c<<8)|r); }
  static inline int RuleN(int id) { return id&0xFF; }
  static inline int CondN(int id) { return (id>>8)&0xF; }
  static long CondToBstate(DecisionTable::condition_t cond)
  {
    switch(cond) {
      case DecisionTable::any: return BSTATE_3STATE; 
      case DecisionTable::tt: return BSTATE_CHECKED; 
      case DecisionTable::ff: return BSTATE_UNCHECKED; 
      default: far_assert("Invalid 'condition_t cond'");
               return 0; // avoid compiler warning
    }  
  }
  static DecisionTable::condition_t BstateToCond(long bstate)
  {
    switch(bstate) {
      case BSTATE_3STATE: return DecisionTable::any; 
      case BSTATE_CHECKED: return DecisionTable::tt; 
      case BSTATE_UNCHECKED: return DecisionTable::ff; 
      default: far_assert("Invalid 'long bstate'");
               return DecisionTable::any; // avoid compiler warning
    }  
  }
  void RedrawRules(HANDLE hDlg)
  {
    const unsigned n_conds = dt->GetConditionsCount();
    unsigned n_rules = dt->GetRulesCount();
    bool b_disabled_tail = false; // запрещаем менять после безусловного правила.
    for (unsigned r = 0; r<MaxRules; r++) {
      bool b_all_any = true; // безусловное правило?
      bool b_rule_enabled = false; // изменяемое правило?
      for (unsigned c = 0; c<MaxConditions; c++) {
        long state = BSTATE_3STATE;
        const DecisionTable::condition_t cond = dt->GetCell(r, c);
        b_all_any = b_all_any && cond == DecisionTable::any;
        bool b_enabled = !b_disabled_tail && c<n_conds && r<n_rules;
        if (b_enabled) {
          b_rule_enabled = true;
          state = CondToBstate(cond);
          far_assert(dt->LastError == DecisionTable::Ok);
        }
        const int cell_index =  CellIndex(r, c);
        Far::SendDlgMessage(hDlg, DM_SETCHECK, cell_index, state);
        Far::SendDlgMessage(hDlg, DM_ENABLE, cell_index, b_enabled? TRUE : FALSE);
      }
      b_disabled_tail = b_disabled_tail || b_all_any;
      if (!b_all_any && r == n_rules-1 && r+1<MaxRules) { 
        // подготовим следующее правило.
        dt->InsertRule(n_rules);
        n_rules++;
      }
      Far::SendDlgMessage(hDlg, DM_ENABLE, Index_0x1000+r, b_rule_enabled? TRUE : FALSE);
      Far::SendDlgMessage(hDlg, DM_ENABLE, Index_0x3000+r, b_rule_enabled? TRUE : FALSE);
      for (unsigned c = 0; c<MaxConditions; c++) {
        // обновим кнопки со свойствами словаря.
        Far::SendDlgMessage(hDlg, DM_ENABLE, ItemIndex(ID_BTN1+c), 
          c<n_conds? TRUE : FALSE);
      }
    }
  }
  static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
  {
    int id = 0;
    DictViewEditorDialog *pThis = NULL;
    switch (Msg) 
    {
      case DN_INITDIALOG:
        pThis = (DictViewEditorDialog *)Param2;
        far_assert(pThis);
        Far::SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
        pThis->RedrawRules(hDlg);
        //FarListPos = {x,-1};DM_LISTSETCURPOS
        break;
      case DN_EDITCHANGE:
        pThis = (DictViewEditorDialog *)Far::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
        far_assert(pThis);
        switch(Param1) {
          case Index_ID_CB1: id = 0; break;
          case Index_ID_CB2: id = 1; break;
          case Index_ID_CB3: id = 2; break;
          case Index_ID_CB4: id = 3; break;
          default: id = -1;
        }
        if (id==-1) 
          break;
        {
           const unsigned last_n_conds = pThis->dt->GetConditionsCount();
           DecisionTable::condition_inst_t cond_inst = NULL;
           if (id>last_n_conds) return FALSE;
           if (id==last_n_conds) 
             cond_inst = pThis->dt->InsertCondition(id);
           else          
             cond_inst = pThis->dt->GetConditionInst(id);
           far_assert(cond_inst);
           DictViewInstance::DictParams *params = 
             static_cast<DictViewInstance::DictParams *>(cond_inst->client_context);
           far_assert(params);
           far_assert(!IsBadReadPtr(reinterpret_cast<void *>(Param2), sizeof(FarDialogItem)));
           params->dict = reinterpret_cast<FarDialogItem *>(Param2)->Data;//TODO: SetDict
           if (!last_n_conds)
             pThis->dt->InsertRule(0);
           pThis->RedrawRules(hDlg);
        }
        break;
      case DN_BTNCLICK:
        pThis = (DictViewEditorDialog *)Far::SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
        far_assert(pThis);
        id = ItemId(Param1);
        if ((id&0xF000) == 0x1000) { // reset rule
          const unsigned rule = RuleN(id);
          if (rule<pThis->dt->GetRulesCount()) {
            const unsigned n_conds = pThis->dt->GetConditionsCount();
            for (unsigned c = 0; c<n_conds; c++)
              pThis->dt->SetCell(rule, c, DecisionTable::any);
            pThis->RedrawRules(hDlg);
          }
          return TRUE;
        } else if ((id&0xF000) == 0x2000) { // cell check-box
          const unsigned rule = RuleN(id);
          const unsigned cond = CondN(id);
          static DecisionTable::condition_t click_to_condition[] = { 
            DecisionTable::ff,
            DecisionTable::tt,
            DecisionTable::any
          };
          far_assert(rule<MaxRules);
          far_assert(cond<MaxConditions);
          far_assert(Param2>=0);
          far_assert(Param2<sizeof(click_to_condition)/sizeof(click_to_condition[0]));
          pThis->dt->SetCell(rule, cond, click_to_condition[Param2]);
          if (pThis->dt->LastError != DecisionTable::Ok)
            return FALSE;
          pThis->RedrawRules(hDlg);
        } else if ((id&0xF000) == 0x3000) { // action button
          const unsigned rule = RuleN(id);
          far_assert(rule<MaxRules);
          pThis->RedrawRules(hDlg);
          return TRUE;
        } else switch(id) {
          case ID_BTN1:
          case ID_BTN2:
          case ID_BTN3:
          case ID_BTN4:
            return TRUE;
        }
        break;
    }
    return Far::DefDlgProc(hDlg, Msg, Param1, Param2);
  }
  static int ScanDicts(const FarString& name, void* context)
  {
    DictViewEditorDialog *pThis = (DictViewEditorDialog *)context;
    pThis->conditions.AddItem(name.c_str());
    pThis->n_conditions++;
    return 1;
  }
  public: DictViewEditorDialog()
  : conditions(NULL)
  {
    n_conditions = 0;
    conditions.AttachDialogItem(sItems+Index_ID_CB1);
    conditions.AttachDialogItem(sItems+Index_ID_CB2);
    conditions.AttachDialogItem(sItems+Index_ID_CB3);
    conditions.AttachDialogItem(sItems+Index_ID_CB4);
    //conditions.AddItem("");
    FarSpellEditor::editors->spell_factory.EnumDictionaries(ScanDicts, this);
    conditions.BeforeShow();
  }
  public: void Execute(DictViewInstance *init_dict_view)
  { 
    far_assert(init_dict_view);
    dict_view = init_dict_view;
    dt = &dict_view->logic;
    strncpy(sItems[Index_ID_Name].Data, dict_view->name.c_str(), sizeof(sItems[0].Data));
    const unsigned n_conds = dt->GetConditionsCount();
    for (unsigned c = 0; c<n_conds; c++) {
      DecisionTable::condition_inst_t cond_inst = dt->GetConditionInst(c);
      far_assert(cond_inst);
      DictViewInstance::DictParams *params = 
        static_cast<DictViewInstance::DictParams *>(cond_inst->client_context);
      far_assert(params);
      strncpy(sItems[ItemIndex(ID_CB1+c)].Data, params->dict.c_str(), sizeof(sItems[0].Data));
    }
    int idx = ShowEx(0, DlgProc, (long)this);
    if (idx>=0) switch (idx) {
      case Index_MCancel:
        break;
      default:
        dict_view->name = sItems[Index_ID_Name].Data;
        dict_view->Save();
        break;
    }
  }
};

void FarSpellEditor::Manager::EditDictionaryView(DictViewInstance *dict_view)
{
  DictViewEditorDialog().Execute(dict_view);
}

int ScanDictViews(const FarString& id, const FarString& name, 
  void* client_context)
{
  FarMenu *menu = (FarMenu *)client_context;
  menu->AddItem(name);
  return 1;
}

void FarSpellEditor::Manager::DictionaryViewSelect(bool edit_by_enter)
{
  static int brk_keys[] = { VK_F4, VK_INSERT, VK_DELETE, 0 };
  DictViewInstance *dict_view = NULL;
  int selected_item = 0;
  for (bool loop = true; loop;) {
    loop = false;
    FarMenu menu(MSelectDictView, FMENU_WRAPMODE, "Contents");
    dict_view_factory.EnumDictViews(ScanDictViews, &menu);
    menu.AddItem("");
    menu.SetBottomLine("Enter, Esc, F4, Ins, Del");
    menu.SetBreakKeys(brk_keys); 
    menu.SelectItem(selected_item);
    selected_item = menu.Show();
    if (selected_item>=0) {
      switch (menu.GetBreakCode()) {
        case -1:
          if (!edit_by_enter) break;
        case 0: // F4
          if (selected_item>=0) {
            FarString name(menu.GetItemText(selected_item));
            if (!name.IsEmpty()) {
              dict_view = dict_view_factory.GetDictViewByName(name);
              EditDictionaryView(dict_view);
              delete dict_view;
              dict_view = NULL;
            }
          }
          loop = true;
          break;
        case 1: // Ins
          dict_view = dict_view_factory.CreateDictView();
          EditDictionaryView(dict_view);
          delete dict_view;
          dict_view = NULL;
          loop = true;
          break;
        case 2: // Del
          if (selected_item>=0)
            dict_view_factory.DeleteDictView(menu.GetItemText(selected_item));
          loop = true;
          break;
      }
    }
  }
}

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
