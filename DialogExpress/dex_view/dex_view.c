/*
    Dialog Resource Viewer and Generator (lately)
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

#include <malloc.h>
#include <windows.h>
#include <string.h>
#pragma pack(push,2)
#pragma pack(2)
#include <plugin.hpp>
#pragma pack()
#pragma pack(pop)
#include "../dialogres.h"
#define malloc(nBytes) HeapAlloc(GetProcessHeap(), 0, nBytes)
#define free(pChunk) HeapFree(GetProcessHeap(), 0, pChunk)

/****************************************************************************
 * Константы для извлечения строк из .lng файла
 ****************************************************************************/
enum DexLng {
  MOk,
  MCancel,
  MEdit,

  MDexViewer,
  MSelectDialog,

  MError,
  MOSError,
  MNoMemory,
  MIncompleteStatement,
  MSyntaxError,
  MUnknownIdentifier,
  MIllegalToken,
  MDivideByZero,
  MParserStackOverflow,
  MNotFound,
  MTooLarge,
  MUnknownError,
  MFilePosition,
};

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
static char PluginRootKey[255];

/****************************************************************************
 * Обёртка сервисной функции FAR: получение строки из .lng-файла
 ****************************************************************************/
static const char *GetMsg(enum DexLng MsgId) {
  return Info.GetMsg(Info.ModuleNumber, MsgId);
}

void ShowDialog(dialogtemplate* pDialog)
{
  dialogitem *pDialogItem;
  struct FarDialogItem *pItem, *pItems;
  char buf[32];

  dialogtemplate_create_items(pDialog, 0, &pItems);
  pItem = pItems;

  for (dialogtemplate_first_item(pDialog, &pDialogItem); 
       pDialogItem; 
       dialogtemplate_next_item(pDialog, &pDialogItem))
  {
    pItem = pItems + dialogitem_index(pDialogItem);
    if (pItem->Flags&DIF_HISTORY)
      pItem->History = dialogitem_history(pDialogItem);
    switch (dialogitem_datasource_type(pDialogItem))
    {
      case DI_DATASOURCE_TEXT: 
        strncpy(pItem->Data, dialogitem_datasource_text(pDialogItem), sizeof(pItem->Data));
        break;
      case DI_DATASOURCE_MSGID: 
        strncpy(pItem->Data, _itoa(dialogitem_datasource_msgid(pDialogItem), buf, 10), sizeof(pItem->Data));
        break;
    }
  }
  Info.Dialog(Info.ModuleNumber, 
    -1, -1, 
    pItems->X1+pItems->X2+1, 
    pItems->Y1+pItems->Y2+1,
    NULL, pItems, dialogtemplate_items_count(pDialog));
  dialogres_free(pItems);
}

BOOL TryViewDialogRes(char *zFilename)
{
  char *z, buf[1024];
  enum dialogres_error rc;
  int r, j, dialogs_count, known_token, skip_item;
  dialogres* dr;
  dialogtemplate* pDialog;
  const char *zMsgItems[5];
  struct FarMenuItem *pItems, *pItem;

  if (!zFilename || !*zFilename) return FALSE;
  if (!(z=strrchr(zFilename, '.')) || strcmp(z, ".dlg")) return FALSE;
load_again:
  rc = dialogres_open(zFilename, &dr);
  zMsgItems[1] = GetMsg(MError-1+(rc&~dialogres_ErrToken));
  if (rc)
  {
    zMsgItems[0] = GetMsg(MDexViewer);
    known_token = rc&dialogres_ErrToken && dr;
    if (known_token)  {
      FSF.sprintf(buf, GetMsg(MFilePosition), 
          FSF.PointToName(zFilename), 
          dialogres_error_line(dr), 
          dialogres_error_pos(dr));
      zMsgItems[2] = buf;
    }
    else if (rc == dialogres_OSError) {
      FSF.sprintf(buf, zMsgItems[1], GetLastError());
      zMsgItems[1] = buf;
      FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(),
        MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  buf+32, sizeof(buf)-32, 0);
      zMsgItems[2] = buf+32;
    }
    else 
      zMsgItems[2] = "...";
    // проверим, понадобится ли кнопка для вызова редактора для исправления.
    skip_item = ( (rc>=dialogres_IncompleteStatement)
               && (rc<=dialogres_DivideByZero)
                ) ? 0 : 1;

    if (skip_item == 0) {
      zMsgItems[3] = GetMsg(MEdit);
      zMsgItems[4] = GetMsg(MCancel);
    } else {
      zMsgItems[3] = GetMsg(MOk);
    }
    switch(Info.Message(Info.ModuleNumber, 0, NULL, 
      zMsgItems, -skip_item + sizeof(zMsgItems) / sizeof(zMsgItems[0]), 2-skip_item))
    {
      case 1: break;
      case 0: 
        if (skip_item == 1) break;
        if (Info.Editor(zFilename, NULL, -1, -1, -1, -1, 
          EF_NONMODAL|EF_ENABLE_F6, 
          known_token?dialogres_error_line(dr):0,
          known_token?dialogres_error_pos(dr):0) == EEC_MODIFIED) 
        {
          if (dr) dialogres_close(dr);
          goto load_again;
        }
    }
  }
  if (rc || !dr || !(dialogs_count=dialogres_dialogs_count(dr)))
  {
    if (dr) dialogres_close(dr);
    return FALSE;
  }
  // Следующий код основан на DialogViewer Lite by Griphon.
  // Вечная память DailogGenerator'у.
  pItems = (struct FarMenuItem*)alloca(dialogs_count*sizeof(struct FarMenuItem));
  pItem = pItems + dialogres_dialogs_count(dr);
  for (dialogres_last_dialog(dr, &pDialog); pDialog; dialogres_prev_dialog(dr, &pDialog))
  {
    pItem--;
    strncpy(pItem->Text, dialogtemplate_name(pDialog), sizeof(pItem->Text));
    pItem->Selected = 0;
    pItem->Checked = 0;
    pItem->Separator = 0;
  }
  do {
    r = Info.Menu(Info.ModuleNumber,-1,-1,0,FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT,
                  GetMsg(MSelectDialog), NULL, "Contents", NULL, NULL, 
                  pItems, dialogs_count);
    if (r>=0) { 
       rc = dialogres_get_dialog(dr, (pItems+r)->Text, &pDialog); 
       ShowDialog(pDialog);
       // prepare menu for next usage
       for(j=0; j<dialogs_count; j++) 
         (pItems+j)->Selected = 0; 
       (pItems+r)->Selected = 1;
    }
   } while (r>=0); // exit loop if ESC pressed

  return TRUE;
}

/***************************************************************************
 * Функция SetStartupInfo вызывается один раз, перед всеми
 * другими функциями. Она передается плагину информацию,
 * необходимую для дальнейшей работы.
 **************************************************************************/
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info = *psi;
  FSF = *psi->FSF;
  Info.FSF = &FSF;
  strcpy(PluginRootKey, psi->RootKey);
  strcat(PluginRootKey, "\\DexView");
}

/***************************************************************************
 * Функция GetPluginInfo вызывается для получения основной
 * (general) информации о плагине
 ***************************************************************************/
void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static char *PluginMenuStrings[1];
  pi->StructSize = sizeof(struct PluginInfo);
  PluginMenuStrings[0] = (char*)GetMsg(MDexViewer);
  pi->PluginMenuStrings = PluginMenuStrings;
  pi->PluginMenuStringsNumber = sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  pi->CommandPrefix = "dex";
}

/***************************************************************************
 * Функция OpenPlugin вызывается при создании новой копии плагина.
 ***************************************************************************/
HANDLE WINAPI _export OpenPlugin(int OpenFrom, int item)
{
  switch (OpenFrom)
  {
    case OPEN_COMMANDLINE:
      TryViewDialogRes(FSF.LTrim((char*)item));
      break;
    case OPEN_PLUGINSMENU:
    {
      struct PanelInfo PInfo;
      if (!Info.Control(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, &PInfo))
        return INVALID_HANDLE_VALUE;
      if (PInfo.PanelType != PTYPE_FILEPANEL) 
        return INVALID_HANDLE_VALUE;
      if (PInfo.CurrentItem < 0)
        return INVALID_HANDLE_VALUE;
      TryViewDialogRes(PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
    }
  }
  return INVALID_HANDLE_VALUE;
}

#if 0
// В место этого, лучше приделать ассоциацию по расширению на префикс dex:
HANDLE WINAPI _export OpenFilePlugin(char *zFilename, const unsigned char *Data, int DataSize)
{
  return TryViewDialogRes(zFilename)
         ? (HANDLE)-2
         : INVALID_HANDLE_VALUE;
}
#endif

int __cdecl atexit(void (__cdecl *rest)(void))
{
	return 1;
}

int __stdcall _DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	return 1;
}

int __stdcall _cygwin_dll_entry(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	return 1;
}

int main()
{
	return 1;
}
