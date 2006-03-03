// stripped HelloWorld.c
#include <windows.h>
#include <string.h>
#include "plugin.hpp"
#define malloc(nBytes) HeapAlloc(GetProcessHeap(), 0, nBytes)
#define free(pChunk) HeapFree(GetProcessHeap(), 0, pChunk)

static struct PluginStartupInfo Info;
/*
Функция SetStartupInfo вызывается один раз, перед всеми
другими функциями. Она передается плагину информацию,
необходимую для дальнейшей работы.
*/
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
}

/*
Функция GetPluginInfo вызывается для получения основной
  (general) информации о плагине
*/
void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static char *PluginMenuStrings[1];
  pi->StructSize = sizeof(struct PluginInfo);
  PluginMenuStrings[0] = "DialogExpress: Color select example";
  pi->PluginMenuStrings = PluginMenuStrings;
  pi->PluginMenuStringsNumber = sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}

/*
  Функция OpenPlugin вызывается при создании новой копии плагина.
*/
#include "color.c"
static long WINAPI DlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
{
  int id = ColorSelectId(Param1);
  int nColor;
  switch (Msg) 
  {
    case DN_INITDIALOG:
       Info.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, Param2);
       break;
    case DN_CTLCOLORDLGITEM: 
       if (Param1 == ColorSelectIndex_243) 
          return Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
       break;
    case DN_BTNCLICK: 
       id = ColorSelectId(Param1);
       if (id&0x100) 
       {
         nColor = Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
         nColor = nColor&0xF0 | id&0x0F;
         Info.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, nColor);
       }
       else if (id&0x200) 
       {
         nColor = Info.SendDlgMessage(hDlg, DM_GETDLGDATA, 0, 0);
         nColor = nColor&0x0F | id&0xF0;
         Info.SendDlgMessage(hDlg, DM_SETDLGDATA, 0, nColor);
       }
       break;
  }
  return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
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

static int nColor = 0x0F;

HANDLE WINAPI _export OpenPlugin(int OpenFrom, int item)
{
  int res;
  struct FarDialogItem* pItems = malloc(sizeof(struct FarDialogItem)*ColorSelect_NItems);
  struct FarDialogItem* pItem;
  FillColorSelect(&Info, pItems);
  // Установить текущий цвет 
  pItem = pItems+ColorSelectIndex_0x100+(nColor&0x0F);
  pItem->Selected = 1;
  pItem->Focus = 1;
  (pItems+ColorSelectIndex_0x200+((nColor&0xF0)>>4))->Selected = 1;
  // Установить цвет для "образца"
  pItem = pItems+ColorSelectIndex_243;
  pItem->Flags = (pItem->Flags&~DIF_COLORMASK) | nColor;
  res = Info.DialogEx(Info.ModuleNumber, 
           -1, -1, ColorSelect_Width, ColorSelect_Height, 
           "Content", pItems, ColorSelect_NItems, 0, 0, 
	   DlgProc, nColor);
  if (res != -1)
    nColor = GetRadioStatus(pItems, 45, ColorSelectIndex_0x100)
           |(GetRadioStatus(pItems, 45, ColorSelectIndex_0x200)<<4);
  free(pItems);
  return INVALID_HANDLE_VALUE;
}

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
