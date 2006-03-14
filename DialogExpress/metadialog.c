/*
    MetaDialog: Dialog resource to <any> converter
    Copyright (C) 2006 Sergey Shishmintzev

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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dialogres.h"
#include "metaparse.h"


#ifdef _DEBUG
extern size_t dialogresMemory;
#endif

static char* DialogItemTypes[] = {
  "DI_TEXT       ",
  "DI_VTEXT      ",
  "DI_SINGLEBOX  ",
  "DI_DOUBLEBOX  ",
  "DI_EDIT       ",
  "DI_PSWEDIT    ",
  "DI_FIXEDIT    ",
  "DI_BUTTON     ",
  "DI_CHECKBOX   ",
  "DI_RADIOBUTTON",
  "DI_COMBOBOX   ",
  "DI_LISTBOX    ",
};

void FarDialogItemFlags_to_C(FILE* pFile, 
  int nType, unsigned long nFlag, 
  int bColorConstants)
{
  enum {
    DIM_ALL         = 0xFFFFFFFF,
    DIM_TEXT        = 1<<0,
    DIM_VTEXT       = 1<<1,
    DIM_SINGLEBOX   = 1<<2,
    DIM_DOUBLEBOX   = 1<<3,
    DIM_EDIT        = 1<<4,
    DIM_PSWEDIT     = 1<<5,
    DIM_FIXEDIT     = 1<<6,
    DIM_BUTTON      = 1<<7,
    DIM_CHECKBOX    = 1<<8,
    DIM_RADIOBUTTON = 1<<9,
    DIM_COMBOBOX    = 1<<10,
    DIM_LISTBOX     = 1<<11,
    DIM_USERCONTROL = 1<<12,
  };
  struct FlagId { unsigned long nFlag; char* zId; unsigned nMask; };
  struct FlagId FlagIds[] = {
    { 0x00000400UL, "DIF_GROUP",             DIM_RADIOBUTTON },
    { 0x00000800UL, "DIF_LEFTTEXT",          DIM_DOUBLEBOX|DIM_SINGLEBOX },
    { 0x00001000UL, "DIF_MOVESELECT",        DIM_RADIOBUTTON },
    { 0x00002000UL, "DIF_SHOWAMPERSAND",     DIM_DOUBLEBOX|DIM_SINGLEBOX|DIM_TEXT|DIM_BUTTON|DIM_CHECKBOX|DIM_RADIOBUTTON },
    { 0x00004000UL, "DIF_CENTERGROUP",       DIM_BUTTON|DIM_CHECKBOX|DIM_RADIOBUTTON|DIM_TEXT },
    { 0x00008000UL, "DIF_NOBRACKETS",        DIM_BUTTON },
    { 0x00010000UL, "DIF_SEPARATOR",         DIM_TEXT },
    { 0x00010000UL, "DIF_VAREDIT",           DIM_EDIT|DIM_COMBOBOX  },
    { 0x00020000UL, "DIF_SEPARATOR2",        DIM_TEXT },
    { 0x00020000UL, "DIF_EDITOR",            DIM_EDIT|DIM_FIXEDIT|DIM_PSWEDIT },
    { 0x00020000UL, "DIF_LISTNOAMPERSAND",   DIM_LISTBOX },
    { 0x00040000UL, "DIF_LISTNOBOX",         DIM_LISTBOX },
    { 0x00040000UL, "DIF_HISTORY",           DIM_EDIT|DIM_FIXEDIT },
    { 0x00200000UL, "DIF_USELASTHISTORY",    DIM_EDIT|DIM_FIXEDIT },
    { 0x00008000UL, "DIF_MANUALADDHISTORY",  DIM_EDIT|DIM_FIXEDIT },
    { 0x00040000UL, "DIF_BTNNOCLOSE",        DIM_BUTTON },
    { 0x00040000UL, "DIF_CENTERTEXT",        DIM_TEXT|DIM_VTEXT },
    { 0x00080000UL, "DIF_EDITEXPAND",        DIM_EDIT },
    { 0x00100000UL, "DIF_DROPDOWNLIST",      DIM_COMBOBOX },
    { 0x00400000UL, "DIF_MASKEDIT",          DIM_FIXEDIT },
    { 0x00800000UL, "DIF_SELECTONENTRY",     DIM_EDIT|DIM_FIXEDIT|DIM_PSWEDIT },
    { 0x00800000UL, "DIF_3STATE",            DIM_CHECKBOX },
    { 0x01000000UL, "DIF_LISTWRAPMODE",      DIM_LISTBOX|DIM_COMBOBOX },
    { 0x02000000UL, "DIF_LISTAUTOHIGHLIGHT", DIM_LISTBOX|DIM_COMBOBOX },
    { 0x04000000UL, "DIF_LISTNOCLOSE",       DIM_COMBOBOX }, // ???
    { 0x10000000UL, "DIF_HIDDEN",            DIM_ALL },
    { 0x20000000UL, "DIF_READONLY",          DIM_EDIT|DIM_FIXEDIT|DIM_PSWEDIT },
    { 0x40000000UL, "DIF_NOFOCUS",           DIM_ALL },
    { 0x80000000UL, "DIF_DISABLE",           DIM_ALL },
    { 0x00000100UL, "DIF_SETCOLOR",          DIM_ALL }, 
    { 0x00000200UL, "DIF_BOXCOLOR",          DIM_ALL },
  };
  char *ColorIds[16] = {
    "BLACK", 
    "BLUE",         "GREEN",      "CYAN",      "RED", 
    "MAGENTA",      "BROWN",      "LIGHTGRAY", "DARKGRAY",
    "LIGHTBLUE",    "LIGHTGREEN", "LIGHTCYAN", "LIGHTRED",
    "LIGHTMAGENTA", "YELLOW",
    "WHITE",
   };
  int i;
  unsigned nMask = (nType == DI_USERCONTROL) ? DIM_USERCONTROL : 1 << nType;
  bColorConstants = bColorConstants && (nFlag&DIF_SETCOLOR);
  for (i = 0; i < sizeof(FlagIds)/sizeof(struct FlagId); i++)
  {
    if (FlagIds[i].nMask&nMask && FlagIds[i].nFlag&nFlag)
    {
      nFlag &= ~FlagIds[i].nFlag;
      fprintf(pFile, FlagIds[i].zId);
      fprintf(pFile, "|");
    }
  }
  if (bColorConstants)
  {
    fprintf(pFile, "%s|BK_%s|", ColorIds[nFlag&0x0F], ColorIds[(nFlag&0xF0)>>4]);
    nFlag &= ~(DIF_SETCOLOR|DIF_COLORMASK);
  }
  fprintf(pFile, "0x%X", nFlag);
}

char* make_c_ident(const char* s) 
{
  char *i, *r;
  r = strdup(s);
  for(i = r; *i; i++) {
    switch (*i) {
      case '-': *i='m'; break;
      case ' ': *i='_'; break;
    }
  }
  return r;
}

void dialogres_to_any(FILE* pFile, dialogres* dr, TemplateToken *pTemplate)
{
  dialogtemplate* pDialog = NULL;
  dialogitem *pDialogItem;
  enum dialogres_error rc;
  struct FarDialogItem *pItem, *pItems = NULL;
  int bSkip, bCondition = 0, bSkipNewline, bNot;
  TemplateToken *pToken;
  char *sid;
  for (pToken = pTemplate; pToken;)
  {
    bSkipNewline = 0;
    bNot = 0;
    switch (pToken->nType)
    {
      case Literal:
        fprintf(pFile, "%s", (char*)(pToken+1));
        pToken = pToken->next;
        break;
      case Newline:
        fprintf(pFile, "\n");
        pToken = pToken->next;
        break;
      case ForeachDialog:
        rc = dialogres_last_dialog(dr, &pDialog);
        if (pDialog) {
          pToken = pToken->next;
          dialogtemplate_create_items(pDialog, 0, &pItems);
        } else {
          for (; pToken->nType!=NextDialog; pToken->next)
          pToken = pToken->next;
        }
        bSkipNewline = 1;
        break;
      case NextDialog:
        dialogres_free(pItems);
        pItems = NULL;
        rc = dialogres_prev_dialog(dr, &pDialog);
        pToken = pDialog ? pToken->pLoop : pToken->next;
        if (pDialog)
          dialogtemplate_create_items(pDialog, 0, &pItems);
        bSkipNewline = 1;
        break;
      case ForeachItem:
        assert(pDialog);
        assert(pItems);
        bCondition = 0;
        rc = dialogtemplate_first_item(pDialog, &pDialogItem);
        if (pDialogItem) {
          pToken = pToken->next;
          pItem = pItems + dialogitem_index(pDialogItem);
        } else {
          for (; pToken->nType!=NextItem; pToken->next)
          pToken = pToken->next;
        }
        bSkipNewline = 1;
        break;
      case NextItem:
        assert(pDialog);
        assert(pDialogItem);
        assert(pItems);
        bCondition = 0;
        rc = dialogtemplate_next_item(pDialog, &pDialogItem);
        pToken = pDialogItem ? pToken->pLoop : pToken->next;
        if (pDialogItem)  
          pItem = pItems + dialogitem_index(pDialogItem);
        bSkipNewline = 1;
        break;
      case HasNotCondition:
        bNot = 1;
      case HasCondition:
        bCondition = 1;
        switch (pToken->nPredicate) {
         case ItemHasX1:
           assert(pItem);
           bSkip = pItem->X1==0;
           break;
         case ItemHasY1:
           assert(pItem);
           bSkip = pItem->Y1==0;
           break;
         case ItemHasX2:
           assert(pItem);
           bSkip = pItem->X2==0;
           break;
         case ItemHasY2:
           assert(pItem);
           bSkip = pItem->Y2==0;
           break;
         case ItemHasFocused:
           assert(pItem);
           bSkip = pItem->Focus==0;
           break;
         case ItemHasSelected:
           assert(pItem);
           bSkip = pItem->Selected==0;
           break;
         case ItemHasFlags:
           assert(pItem);
           bSkip = pItem->Flags==0;
           break;
         case ItemHasMsgidP:
           assert(pDialogItem);
           bSkip = (dialogitem_datasource_type(pDialogItem)!=DI_DATASOURCE_MSGID);
           break;
         case ItemHasIdP:
           assert(pDialogItem);
           bSkip = !dialogitem_id(pDialogItem);
           break;
         case ItemHasHistoryP:
           assert(pDialogItem);
           bSkip = !*dialogitem_history(pDialogItem);
           break;
         case ItemHasTextP:
           assert(pDialogItem);
           bSkip = dialogitem_datasource_type(pDialogItem)!=DI_DATASOURCE_TEXT;
           break;
         default:
           bSkip = 1;
           assert(0);
        }
        pToken = pToken->next;
        if (bNot) bSkip = !bSkip;
        if (bSkip) {
          for (;pToken; pToken = pToken->next) 
          {
            if (pToken->nType == HasCondition) break;
            if (pToken->nType == HasNotCondition) break;
            if (bCondition && pToken->nType == Alternative) break;
            if (pToken->nType == NextItem) break;
            if (pToken->nType == Common) break;
          }
        }
        bSkipNewline = 1;
        break;
      case Common:
        assert(bCondition);
        pToken = pToken->next;
        bSkipNewline = 1;
        break;
      case Alternative:
        assert(bCondition);
        pToken = pToken->next;
        bSkipNewline = 1;
        break;
      case Variable:
        //fprintf(pFile, "%%[%s]", (char*)(pToken+1));
        pToken = pToken->next;
        break;
      case InternalVariable:
        switch (pToken->nVariable)
        {
          case vDialogId: 
            assert(pDialog);
            fprintf(pFile, "%s", dialogtemplate_name(pDialog));
            break;
          case vDialogProgId: 
            assert(pDialog);
            sid = make_c_ident(dialogtemplate_name(pDialog));
            assert(sid);
            fprintf(pFile, "%s", sid);
            free(sid);
            break;
          case vDialogHelp: 
            assert(pDialog);
            fprintf(pFile, "%s", dialogtemplate_help(pDialog));
            break;
          case vDialogWidth:  
            assert(pDialog);
            assert(dialogtemplate_items_count(pDialog)>0);
            assert(pItems);
            fprintf(pFile, "%d", pItems->X1+pItems->X2+1);
            break;
          case vDialogHeight:  
            assert(pDialog);
            assert(dialogtemplate_items_count(pDialog)>0);
            assert(pItems);
            fprintf(pFile, "%d", pItems->Y1+pItems->Y2+1);
            break;
          case vItemsCount:  
            assert(pDialog);
            fprintf(pFile, "%d", dialogtemplate_items_count(pDialog));
            break;
          case vItemIndex:  
            assert(pDialogItem);
            fprintf(pFile, "%d", dialogitem_index(pDialogItem));
            break;
          case vItemId:  
            assert(pDialogItem);
            fprintf(pFile, "%d", dialogitem_id(pDialogItem));
            break;
          case vItemIdLiteral:  
            assert(pDialogItem);
            fprintf(pFile, "%s", dialogitem_sid(pDialogItem));
            break;
          case vItemProgId:  
            assert(pDialogItem);
            sid = make_c_ident(dialogitem_sid(pDialogItem));
            assert(sid);
            fprintf(pFile, "%s", sid);
            free(sid);
            break;
          case vItemType:
            assert(pItem);
            if (pItem->Type == DI_USERCONTROL) 
              fprintf(pFile, "DI_USERCONTROL");
            else
              fprintf(pFile, "%s", DialogItemTypes[pItem->Type]);
            break;
          case vItemX1:  
            assert(pItem);
            fprintf(pFile, "%2d", pItem->X1);
            break;
          case vItemY1:  
            assert(pItem);
            fprintf(pFile, "%2d", pItem->Y1);
            break;
          case vItemX2:  
            assert(pItem);
            fprintf(pFile, "%2d", pItem->X2);
            break;
          case vItemY2:  
            assert(pItem);
            fprintf(pFile, "%2d", pItem->Y2);
            break;
          case vItemFocused:
            assert(pItem);
            fprintf(pFile, "%d", pItem->Focus);
            break;
          case vItemSelected:
            assert(pItem);
            fprintf(pFile, "%d", pItem->Selected);
            break;
          case vItemFlags:
            assert(pItem);
            FarDialogItemFlags_to_C(pFile, pItem->Type, pItem->Flags, 0);
            break;
          case vItemMsgid:
            assert(pDialogItem);
            assert(dialogitem_datasource_type(pDialogItem)==DI_DATASOURCE_MSGID);
            fprintf(pFile, "%3d", dialogitem_datasource_msgid(pDialogItem));
            break;
          case vItemMsgidLiteral:
            assert(pDialogItem);
            assert(dialogitem_datasource_type(pDialogItem)==DI_DATASOURCE_MSGID);
            fprintf(pFile, "%s", dialogitem_datasource_smsgid(pDialogItem));
            break;
          case vItemText:
            assert(pDialogItem);
            assert(dialogitem_datasource_type(pDialogItem)==DI_DATASOURCE_TEXT);
            fprintf(pFile, "%s", dialogitem_datasource_text(pDialogItem));
            break;
          case vItemHistory:
            assert(pDialogItem);
            assert(dialogitem_history(pDialogItem));
            assert(*dialogitem_history(pDialogItem));
            fprintf(pFile, "%s", dialogitem_history(pDialogItem));
            break;
          case vItemCData:
            assert(pDialogItem);
            switch (dialogitem_datasource_type(pDialogItem))
            {
              case DI_DATASOURCE_NULL: 
                fprintf(pFile, " /* NULL */");
                break;
              case DI_DATASOURCE_TEXT: 
                fprintf(pFile, ", \"%s\"", dialogitem_datasource_text(pDialogItem));
                break;
              case DI_DATASOURCE_MSGID: 
                fprintf(pFile, " /* msgid=%d */", dialogitem_datasource_msgid(pDialogItem));
                break;
              case DI_DATASOURCE_REGISTRY: 
                fprintf(pFile, " /* reg */");
                break;
            }
            break;
          default: 
            assert("Unknown internal variable");
        }
        pToken = pToken->next;
        break;
      default:
        assert("Unknown template token");
    }
    if (bSkipNewline)
      if (pToken && pToken->nType == Newline)
        pToken = pToken->next;
  }
  if (pItems) dialogres_free(pItems);
}

int process(const char *zFilename, TemplateToken *pTemplate)
{
  dialogres* dr;
  int rc = dialogres_open(zFilename, &dr);
  if (rc&dialogres_ErrToken) {
      printf("// SyntaxError at %d:%d\n", dialogres_error_line(dr), dialogres_error_pos(dr));
      rc^=dialogres_ErrToken;
  }
  printf("// rc=%d\n", rc);
  if (rc==dialogres_OSError)
    printf("// GetLastError(): %d\n", GetLastError());
#ifdef _DEBUG
  printf("// Memory use: %d\n", dialogresMemory);
#endif
  if (rc==dialogres_Ok)
  {
     dialogres_to_any(stdout, dr, pTemplate);
  }
  dialogres_close(dr);
#ifdef _DEBUG
  printf("// Memory leaks: %d\n", dialogresMemory);
  dialogresTraceMemory(stdout, "// memory> ");
#endif
  return rc;
}

int main(int argc, char **argv)
{
  int rc;
  char buf[MAX_PATH];
  FILE *log;
  char *zFilename;
  char *zTemplate;
  TemplateToken *pTemplate;
  if (argc!=3)
  {
    printf("Usage: metadialog.exe template filename.dlg [var=value [var=value ...]]\n");
    printf("Note: variables not yet supported :(\n");
    exit(1);
  }
  zTemplate = *++argv;
  rc = meta_load_template(zTemplate, &pTemplate);
  if (rc)
  {
    printf("%d error(s)\n", rc);
    exit(1);
  }
  //meta_print_template(stdout, pTemplate);
  zFilename = *++argv;
  strcpy(buf, zFilename);
  strcat(buf, ".log");
#ifndef NDEBUG
  log = fopen(buf, "w");
  dialogresParserTrace(log, "parse> ");
#endif 
  rc = process(zFilename, pTemplate);
#ifndef NDEBUG
  fclose(log);
#endif 
  return rc;
}
