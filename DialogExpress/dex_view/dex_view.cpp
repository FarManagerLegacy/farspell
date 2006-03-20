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
#include <farcolor.hpp>
#include "../dialogres.h"
#include "../../FarLNG/farlng.h"

#ifdef _DEBUG
#include <assert.h>
#include <stdio.h>
#include "../../FARPlus/FARDbg.h"
#ifdef _MSC_VER
extern "C" void __cdecl _assert(const char *zExp, const char *zFile, unsigned nLine)
{
  //far_assert_message(zFile, nLine, zExp, FALSE);
  fprintf(stderr, "Assertion `%s' failed at %s:%n\n", zFile, nLine, zExp);
  exit(1);
}
#endif

#define save_dialogres_free dialogres_free
#undef dialogres_free
extern "C"  void dialogres_free(void* pChunk)
{  
  _dialogres_free(&pChunk, __FILE__, __LINE__);
}
#define dialogres_free save_dialogres_free 

static void DumpToFile(void* const param, const char* Fmt, ...)
{
  FILE *pFile = (FILE *)param;
  va_list argPtr;
  va_start( argPtr, Fmt );
  vfprintf(pFile, Fmt, argPtr);
  va_end( argPtr );
}

int far_exc_dump(PEXCEPTION_POINTERS pExInfo)
{
  try
  {
    FarDumpStackCb(pExInfo, DumpToFile, stderr);
    return EXCEPTION_EXECUTE_HANDLER;
  }
  catch(...)
  {
    return EXCEPTION_EXECUTE_HANDLER;
  }
}
#endif

#define malloc(nBytes) HeapAlloc(GetProcessHeap(), 0, nBytes)
#define free(pChunk) HeapFree(GetProcessHeap(), 0, pChunk)

/****************************************************************************
 * ��������� ��� ���������� ����� �� .lng �����
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

  MAskLanguage,
  MSelectLanguage,
};

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
static char PluginRootKey[255];

/****************************************************************************
 * ������ ��������� ������� FAR: ��������� ������ �� .lng-�����
 ****************************************************************************/
static const char *GetMsg(enum DexLng MsgId) {
  return Info.GetMsg(Info.ModuleNumber, MsgId);
}

typedef struct DialogInfo {
  const char *zHelpTopic;
  const char *zHlfPath;
  int nShowHelpFlags;
} DialogInfo;

long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
{
  struct DialogInfo *pDlgInfo = (struct DialogInfo *)Dlg_GetDlgData(Info, hDlg);
  switch (Msg) 
  {
    case DN_INITDIALOG:
      Dlg_SetDlgData(Info, hDlg, Param2);
      break;
    case DN_HELP:
      if (pDlgInfo->zHlfPath)
        Info.ShowHelp(pDlgInfo->zHlfPath, pDlgInfo->zHelpTopic, FHELP_CUSTOMFILE);
      return NULL;
  }
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

BOOL FileExists(const char* Name) 
{ // from FAR/PlugDoc/Examples/HlfViewer/Mix.cpp
  return GetFileAttributes(Name)!=0xFFFFFFFF;
}

void ShowDialog(dialogtemplate* pDialog, const char* zPath, const char* zLngFile)
{
  dialogitem *pDialogItem;
  struct FarDialogItem *pItem, *pItems;
  char buf[32];
  char zHlfPath[MAX_PATH];
  struct DialogInfo sDlgInfo = { dialogtemplate_help(pDialog), zPath, FHELP_CUSTOMPATH };
  char **pStrings;
  int nStringsCount;
  unsigned nMsgId;
  int nColorIndex;
  const bool bLngOk = zLngFile
    ? LoadLanguageFile(zLngFile, NULL, pStrings, nStringsCount)
    : false;

  if (zLngFile) {
    size_t n = strlen(zLngFile)+strlen(zPath);
    strcpy(zHlfPath, zPath);
    strcat(zHlfPath, zLngFile);
    strcpy(&zHlfPath[n-3], "hlf");
    if (FileExists(zHlfPath)) {
      sDlgInfo.zHlfPath = zHlfPath;
      sDlgInfo.nShowHelpFlags = FHELP_CUSTOMFILE;
    } 
  }
  
  dialogtemplate_create_items(pDialog, 0, &pItems);
  pItem = pItems;

  for (dialogtemplate_first_item(pDialog, &pDialogItem); 
       pDialogItem; 
       dialogtemplate_next_item(pDialog, &pDialogItem))
  {
    pItem = pItems + dialogitem_index(pDialogItem);
    if (pItem->Flags&DIF_HISTORY)
      pItem->History = dialogitem_history(pDialogItem);
    if (dialogitem_color_index(pDialogItem, &nColorIndex)==dialogres_Ok) 
      pItem->Flags |= Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, 
                             (void*)nColorIndex)
                   | DIF_SETCOLOR;
    if (dialogitem_fgcolor_index(pDialogItem, &nColorIndex)==dialogres_Ok) 
      pItem->Flags |= 0x0F&Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, 
                             (void*)nColorIndex)
                   | DIF_SETCOLOR;
    if (dialogitem_bgcolor_index(pDialogItem, &nColorIndex)==dialogres_Ok) 
      pItem->Flags |= 0xF0&Info.AdvControl(Info.ModuleNumber, ACTL_GETCOLOR, 
                             (void*)nColorIndex)
                   | DIF_SETCOLOR;

    switch (dialogitem_datasource_type(pDialogItem))
    {
      case DI_DATASOURCE_TEXT: 
        strncpy(pItem->Data, dialogitem_datasource_text(pDialogItem), sizeof(pItem->Data));
        break;
      case DI_DATASOURCE_MSGID: 
        nMsgId = dialogitem_datasource_msgid(pDialogItem);
        strncpy(pItem->Data, 
          bLngOk && nMsgId<(unsigned)nStringsCount 
            ? pStrings[nMsgId]
            : _itoa(nMsgId, buf, 10), 
          sizeof(pItem->Data));
        break;
    }
  }

  Info.DialogEx(Info.ModuleNumber, 
    -1, -1, 
    pItems->X1+pItems->X2+1, 
    pItems->Y1+pItems->Y2+1,
    NULL, pItems, dialogtemplate_items_count(pDialog), 
    NULL, 0, DlgProc, (long)&sDlgInfo);
  dialogres_free(pItems);
  if (bLngOk) 
    FinalizeLanguageStrings(pStrings, nStringsCount);
}

int WINAPI GetLngCountCb(const WIN32_FIND_DATA *FData,
                         const char *FullName, void *Param) {
  (*(int*)Param)++;
  return TRUE;
}

int WINAPI GetLanguagesCb(const WIN32_FIND_DATA *FData,
                         const char *FullName, void *Param) {
  struct FarMenuItemEx **ppItem = (struct FarMenuItemEx **)Param;
  strncpy((*ppItem)->Text.Text, FSF.PointToName(FullName), sizeof((*ppItem)->Text.Text));
  (*ppItem)++;
  return TRUE;
}

BOOL TryViewDialogRes(char *zFilename)
{
  char *z, buf[1024];
  enum dialogres_error rc;
  int r, j, dialogs_count, known_token, skip_item;
  dialogres* dr;
  dialogtemplate* pDialog;
  const char *zMsgItems[5];
  struct FarMenuItemEx *pItems, *pItem, *pLngItems;
  int nCurrentLanguage = 0;
  static bool bAskLanguage = true;

  if (!zFilename || !*zFilename) return FALSE;
  if (!(z=strrchr(zFilename, '.')) || strcmp(z, ".dlg")) return FALSE;
load_again:
  rc = dialogres_open(zFilename, &dr);
  zMsgItems[1] = GetMsg((enum DexLng)(MError-1+(rc&~dialogres_ErrToken)));
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
    // ��������, ����������� �� ������ ��� ������ ��������� ��� �����������.
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

  const char *zPathEnd = FSF.PointToName(zFilename);
  char *zPath = (char *)alloca(zPathEnd-zFilename+1);
  int nLngFilesCount = 0;
  strncpy(zPath, zFilename, zPathEnd-zFilename);
  zPath[zPathEnd-zFilename] = '\0';
  FSF.FarRecursiveSearch (zPath, "*.lng", GetLngCountCb, 0, &nLngFilesCount);
  if (!nLngFilesCount)
    bAskLanguage = false;

  size_t nMenuBytes = (dialogs_count+nLngFilesCount+2)
                      *sizeof(struct FarMenuItemEx);
  pItems = (struct FarMenuItemEx*)alloca(nMenuBytes);
  memset(pItems, 0, nMenuBytes);
  
  pItem = pItems + dialogres_dialogs_count(dr);
  for (dialogres_last_dialog(dr, &pDialog); pDialog; dialogres_prev_dialog(dr, &pDialog))
  {
    pItem--;
    strncpy(pItem->Text.Text, dialogtemplate_name(pDialog), sizeof(pItem->Text.Text));
  }

  pItem = pItems + dialogs_count;
  pItem->Flags = MIF_SEPARATOR;
  pItem++;

  strncpy(pItem->Text.Text, GetMsg(MAskLanguage), sizeof(pItem->Text.Text));
  if (!nLngFilesCount) 
    pItem->Flags |= MIF_DISABLE;
  if (bAskLanguage)
    pItem->Flags = MIF_CHECKED;
  pItem++;

  pLngItems = pItem; 
  if (!bAskLanguage)
    (pLngItems+nCurrentLanguage)->Flags = MIF_CHECKED;
  FSF.FarRecursiveSearch (zPath, "*.lng", GetLanguagesCb, 0, &pItem);

  do {
    r = Info.Menu(Info.ModuleNumber, -1, -1, 0,
                  FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                  GetMsg(MSelectDialog), NULL, "Contents", NULL, NULL, 
                  (struct FarMenuItem*)pItems, dialogs_count+2+(bAskLanguage?0:nLngFilesCount));
    if (r>=0) { 
       if (r<dialogs_count) {
         if (bAskLanguage) 
           nCurrentLanguage = 0;
         do {
           if (bAskLanguage && nLngFilesCount) {
             pItem = (pLngItems+nCurrentLanguage);
             pItem->Flags |= MIF_SELECTED;
             int l = Info.Menu(Info.ModuleNumber, -1, -1, 0,
                       FMENU_WRAPMODE|FMENU_AUTOHIGHLIGHT|FMENU_USEEXT,
                       GetMsg(MSelectLanguage), NULL, "Contents", NULL, NULL, 
                       (struct FarMenuItem*)pLngItems, nLngFilesCount);
             if (l>=0) nCurrentLanguage = l;
             pItem->Flags &= ~MIF_SELECTED;
             if (l<0) break;
           }
           rc = dialogres_get_dialog(dr, (pItems+r)->Text.Text, &pDialog); 
           ShowDialog(pDialog, zPath, nLngFilesCount
                ? (pLngItems+nCurrentLanguage)->Text.Text
                : NULL);
         } while (bAskLanguage);
       } else if (r==dialogs_count+1) {
         bAskLanguage = !bAskLanguage;
         if (bAskLanguage) {
           (pItems+r)->Flags |= MIF_CHECKED;
           (pLngItems+nCurrentLanguage)->Flags &= ~MIF_CHECKED;
         } else {
           (pItems+r)->Flags &= ~MIF_CHECKED;
           (pLngItems+nCurrentLanguage)->Flags |= MIF_CHECKED;
         }
       } else { // language
         (pLngItems+nCurrentLanguage)->Flags &= ~MIF_CHECKED;
         nCurrentLanguage = (pItems+r) - pLngItems;
         (pLngItems+nCurrentLanguage)->Flags |= MIF_CHECKED;
       }
       // prepare menu for next usage
       for(j=0; j<dialogs_count+2+nLngFilesCount; j++) 
         (pItems+j)->Flags &= ~MIF_SELECTED; 
       (pItems+r)->Flags |= MIF_SELECTED;
    }
   } while (r>=0); // exit loop if ESC pressed

  return TRUE;
}

/***************************************************************************
 * ������� SetStartupInfo ���������� ���� ���, ����� �����
 * ������� ���������. ��� ���������� ������� ����������,
 * ����������� ��� ���������� ������.
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
 * ������� GetPluginInfo ���������� ��� ��������� ��������
 * (general) ���������� � �������
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
 * ������� OpenPlugin ���������� ��� �������� ����� ����� �������.
 ***************************************************************************/
HANDLE WINAPI _export OpenPlugin(int OpenFrom, int item)
{
#if defined(_MSC_VER) && defined(_DEBUG)
  __try {
#endif
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
#if defined(_MSC_VER) && defined(_DEBUG)
  } __except(far_exc_dump(GetExceptionInformation())) { }
#endif
  return INVALID_HANDLE_VALUE;
}

#if 0
// � ����� �����, ����� ��������� ���������� �� ���������� �� ������� dex:
HANDLE WINAPI _export OpenFilePlugin(char *zFilename, const unsigned char *Data, int DataSize)
{
  return TryViewDialogRes(zFilename)
         ? (HANDLE)-2
         : INVALID_HANDLE_VALUE;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus
#ifdef NDEBUG
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
#endif //NDEBUG
#ifdef __cplusplus
}
#endif //__cplusplus

#ifdef NDEBUG
int main()
{
	return 1;
}
#endif //NDEBUG

