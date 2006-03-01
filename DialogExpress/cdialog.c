/*
    Sample dialog resource to C converter
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
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dialogres.h"

#ifdef _DEBUG
extern size_t dialogresMemory;
#endif

static char* DialogItemTypes[] = {
  "DI_TEXT,        ",
  "DI_VTEXT,       ",
  "DI_SINGLEBOX,   ",
  "DI_DOUBLEBOX,   ",
  "DI_EDIT,        ",
  "DI_PSWEDIT,     ",
  "DI_FIXEDIT,     ",
  "DI_BUTTON,      ",
  "DI_CHECKBOX,    ",
  "DI_RADIOBUTTON, ",
  "DI_COMBOBOX,    ",
  "DI_LISTBOX,     ",
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

void dialogres_to_c(FILE* pFile, dialogres* dr, const char* zPrefix)
{
  dialogtemplate* pDialog;
  dialogitem *pDialogItem;
  enum dialogres_error rc;
  struct FarDialogItem *pItem, *pItems;
  for (rc=dialogres_last_dialog(dr, &pDialog); pDialog; rc=dialogres_prev_dialog(dr, &pDialog))
  {
    assert(rc==dialogres_Ok);
    fprintf( pFile, "struct FarDialogItem* %s%s(const struct PluginStartupInfo *pInfo)\n{\n"
           , zPrefix, dialogtemplate_name(pDialog));
    fprintf(pFile, "  struct FarDialogItem Items[] = { \n");
    dialogtemplate_create_items(pDialog, 0, &pItems);
    pItem = pItems;
    for (rc=dialogtemplate_first_item(pDialog, &pDialogItem); 
         pDialogItem; 
         rc=dialogtemplate_next_item(pDialog, &pDialogItem))
    {
      assert(rc==dialogres_Ok);
      pItem = pItems + dialogitem_index(pDialogItem);
      fprintf(pFile, "    { ");
      if (pItem->Type == DI_USERCONTROL) fprintf(pFile, "DI_USERCONTROL, ");
      else fprintf(pFile, DialogItemTypes[pItem->Type]);
      fprintf(pFile, "%2d, %2d, %2d, %2d, %2d, ", 
         pItem->X1, pItem->Y1, pItem->X2, pItem->Y2, 
         pItem->Focus);
      if (pItem->Flags&DIF_HISTORY)
        fprintf(pFile, "(int)(char*)\"%s\", ", dialogitem_history(pDialogItem));
      else
        fprintf(pFile, "%2d, ", pItem->Selected);
      FarDialogItemFlags_to_C(pFile, pItem->Type, pItem->Flags, 0);
      fprintf(pFile, ", %d", pItem->DefaultButton);
      //if ()
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
      fprintf(pFile, " },\n");
    }
    fprintf(pFile, "    { 0 },\n");
    fprintf(pFile, "  };\n");
    fprintf(pFile, "  struct FarDialogItem *pItems =  (struct FarDialogItem *)malloc(sizeof(Items));\n");
    fprintf(pFile, "  memcpy(pItems, Items, sizeof(Items));\n");
    for (rc=dialogtemplate_first_item(pDialog, &pDialogItem); 
         pDialogItem; 
         rc=dialogtemplate_next_item(pDialog, &pDialogItem))
    {
      assert(rc==dialogres_Ok);
      switch (dialogitem_datasource_type(pDialogItem))
      {
        case DI_DATASOURCE_MSGID: 
          fprintf(pFile,
    "  strcpy((pItems+%2d)->Data, pInfo->GetMsg(pInfo->ModuleNumber, %d));\n",
                  dialogitem_index(pDialogItem), 
                  dialogitem_datasource_msgid(pDialogItem));
          break;
        case DI_DATASOURCE_REGISTRY: 
          break;
      }
    }
    fprintf(pFile, "  return pItems;\n");
    fprintf(pFile, "}\n");
    fprintf( pFile, "int __inline %s%sIndex(int nId)\n{\n"
           , zPrefix, dialogtemplate_name(pDialog));
    fprintf(pFile, "  switch(nId)\n");
    fprintf(pFile, "  {\n");
    for (rc=dialogtemplate_first_item(pDialog, &pDialogItem); 
         pDialogItem; 
         rc=dialogtemplate_next_item(pDialog, &pDialogItem))
    {
      assert(rc==dialogres_Ok);
      if (dialogitem_id(pDialogItem))
        fprintf(pFile, "    case %d: return %d;\n", 
                dialogitem_id(pDialogItem),
                dialogitem_index(pDialogItem));
      
    }
    fprintf(pFile, "  }\n");
    fprintf(pFile, "  return 0;\n");
    fprintf(pFile, "}\n");
    fprintf( pFile, "int __inline %s%sId(int nIndex)\n{\n"
           , zPrefix, dialogtemplate_name(pDialog));
    fprintf(pFile, "  switch(nIndex)\n");
    fprintf(pFile, "  {\n");
    for (rc=dialogtemplate_first_item(pDialog, &pDialogItem); 
         pDialogItem; 
         rc=dialogtemplate_next_item(pDialog, &pDialogItem))
    {
      assert(rc==dialogres_Ok);
      fprintf(pFile, "    case %d: return %d;\n", 
              dialogitem_index(pDialogItem),
              dialogitem_id(pDialogItem));
    }
    fprintf(pFile, "  }\n");
    fprintf(pFile, "  return 0;\n");
    fprintf(pFile, "}\n");
    for (rc=dialogtemplate_first_item(pDialog, &pDialogItem); 
         pDialogItem; 
         rc=dialogtemplate_next_item(pDialog, &pDialogItem))
    {
      char *sid;
      assert(rc==dialogres_Ok);
      sid = make_c_ident(dialogitem_sid(pDialogItem));
      assert(sid);
      if (dialogitem_id(pDialogItem))
        fprintf(pFile, "#define  %s%sIndex_%s %d\n", zPrefix,
                dialogtemplate_name(pDialog),
                sid,
                dialogitem_index(pDialogItem));
      fprintf(pFile, "#define  %s%sId_%d %s\n", zPrefix,
              dialogtemplate_name(pDialog),
              dialogitem_index(pDialogItem),
              dialogitem_sid(pDialogItem));
      free(sid);      
    }
    dialogres_free(pItems);
  }
}

int process(const char *zFilename, const char *zPrefix)
{
  dialogres* dr;
  int rc = dialogres_open(zFilename, &dr);
  printf("// rc=%d\n", rc);
  switch(rc){
    case dialogres_SyntaxError:
    case dialogres_UnknownIdentifier:
    case dialogres_IllegalToken:
    case dialogres_DivideByZero:
      printf("SyntaxError at %d:%d\n", dialogres_error_line(dr), dialogres_error_pos(dr));
  }
#ifdef _DEBUG
  printf("// Memory use: %d\n", dialogresMemory);
#endif
  if (rc==dialogres_Ok)
  {
     dialogres_to_c(stdout, dr, zPrefix);
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
  char *zPrefix = "dlg";
  char *zFilename;
  if (argc!=2 && argc!=3)
  {
    printf("Usage: cdialog.exe filename.dlg [prefix]");
    exit(1);
  }
  zFilename = *++argv;
  strcpy(buf, zFilename);
  strcat(buf, ".log");
#ifndef NDEBUG
  log = fopen(buf, "w");
  dialogresParserTrace(log, "parse> ");
#endif 
  if (argc==3) zPrefix = *++argv;
  rc = process(zFilename, zPrefix);
#ifndef NDEBUG
  fclose(log);
#endif 
  return rc;
}
