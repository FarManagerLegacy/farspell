/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Contributor(s):
 *   Sergey Shishmintzev <sergey.shihshmintzev@gmail.com>
 *     Custom CodePage Libarary
 *   V. A. Tikhomirov <kmopevm@knastu.ru>
 *     http://rsdn.ru/article/baseserv/apicallsintercepting.xml  
 *     http://rsdn.ru/article/baseserv/IntercetionAPI.xml
 *   Jeffrey Richter
 *     Win64 Book / 22-LastMsgBoxInfoLib / APIHook.cpp.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <windows.h>
#include "CustomCodePage.h"

#include <ImageHlp.h>
#ifndef _NO_IMAGEHLP_LIB
#pragma comment(lib, "ImageHlp")
#else
#define ImageDirectoryEntryToData Simple_ImageDirectoryEntryToData
static PVOID ImageDirectoryEntryToData(
  PVOID Base,            
  BOOLEAN MappedAsImage, // always TRUE 
  USHORT DirectoryEntry,  
  PULONG Size            
)
{
  // ����������� ��������� �������� PE ���������
  LPBYTE pimage = (LPBYTE)Base;
  IMAGE_DOS_HEADER *idh;
  IMAGE_OPTIONAL_HEADER *ioh;
  IMAGE_IMPORT_DESCRIPTOR *iid;
  DWORD *isd;  //image_thunk_data dword
  int i;

  // �������� ��������� �� ����������� ��������� ������ PE ���������
  idh = (IMAGE_DOS_HEADER*)pimage;
  if (idh->e_magic != 0x5A4D) 
  {
    //���� �� ��������� ���������� ���, �� � ���� ��������� ��� PE ���������
    MessageBox(NULL, "Not exe hdr", "Error!", 0);
    return NULL;
  }
  ioh = (IMAGE_OPTIONAL_HEADER*)(pimage + idh->e_lfanew 
                                 + 4 + sizeof(IMAGE_FILE_HEADER));
  iid = (IMAGE_IMPORT_DESCRIPTOR*)(pimage +ioh->DataDirectory[1].VirtualAddress);
#ifdef _SEARCH_IDATA_SECTION // for weird compilers?
  if (!iid) {
    IMAGE_SECTION_HEADER *ish;
    ish = (IMAGE_SECTION_HEADER*)((BYTE*)ioh + sizeof(IMAGE_OPTIONAL_HEADER));
    //���� ������ .idata
    for (i = 0; i < 16; i++)
      if (lstrcmpiA((char*)((ish+i)->Name) , ".idata") == 0) break;
    if (i==16) 
    {
      MessageBox(NULL, "Unable to find import section", "Error!", 0);
      return NULL;
    }
    // �������� ����� ������ .idata(������� �������� IMAGE_IMPORT_DESCRIPTOR)
    iid = (IMAGE_IMPORT_DESCRIPTOR*)(pimage + (ish +i)->VirtualAddress );
  }
#endif _SEARCH_IDATA_SECTION
  return iid;
}
#endif

static PVOID sm_pvMaxAppAddr = NULL;
static const BYTE cPushOpCode = 0x68;   // The PUSH opcode on x86 platforms

static BOOL ReplaceIATEntryInOneMod(
  PCSTR pszCalleeModName, 
  PROC pfnCurrent, 
  PROC pfnNew, 
  HMODULE hmodCaller
)
// ��� ������� ���� � ������� ������� - .idata ������ ����� � ������ ��
// ����� ���������-�������� 
{
  // ������� ����� ������ �������
  ULONG ulSize;
  PIMAGE_IMPORT_DESCRIPTOR pImportDesc = 
    (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(hmodCaller, TRUE,
    IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
  if (pImportDesc == NULL)
    return FALSE;  // ����� � ���

  // ����� ������ ������
  for (; pImportDesc->Name; pImportDesc++) 
  {
    PSTR pszModName = (PSTR)((PBYTE) hmodCaller + pImportDesc->Name);
    if (lstrcmpiA(pszModName, pszCalleeModName) == 0) 
    {
      // ������� ����� ������� �������
      PIMAGE_THUNK_DATA pThunk 
        = (PIMAGE_THUNK_DATA)((PBYTE) hmodCaller + pImportDesc->FirstThunk);
      // �������� ��� ������������� �������
      for (; pThunk->u1.Function; pThunk++) 
      {
        PROC* ppfn = (PROC*) &pThunk->u1.Function; // ������� ����� �������
        BOOL fFound = (*ppfn == pfnCurrent);       // ��� ����?
        if (!fFound && (*ppfn > sm_pvMaxAppAddr)) 
        {
          // ���� �� �����, �� ������ ��������. 
          // ���� �� � Win98 ��� ����������, ��
          // ����� ����� ���� push � ������� ����� �������
          PBYTE pbInFunc = (PBYTE) *ppfn;
          if (pbInFunc[0] == cPushOpCode) 
          {
            //��, ����� PUSH
            ppfn = (PROC*) &pbInFunc[1];
            //��� �����?
            fFound = (*ppfn == pfnCurrent);
          }
        }
        if (fFound) 
        { // �����!
          DWORD dwOldProt;
          DWORD dwWritten;
          // ������ �������� � ���� ������� ���������� ��� ������
          // ������� ������������� ��������� ������
          VirtualProtect(ppfn, sizeof(ppfn), PAGE_EXECUTE_READWRITE, &dwOldProt);
          // ������ ����� �� ����
          WriteProcessMemory(GetCurrentProcess(), ppfn, &pfnNew, 
            sizeof(pfnNew), &dwWritten);
          // ��������������� �������������� ������ ������� �� ������
          VirtualProtect(ppfn, sizeof(ppfn), dwOldProt, &dwOldProt);
          // ������!
          // ���� �������� �� ������� - ���, ��� ����� ������:
          return dwWritten == sizeof(pfnNew); 
        }
      }
    }
  }
  // ����� ���� ������� �� �������
  return FALSE;
}

 
BOOL CustomCodePageInterceptAPI(HINSTANCE hInstance)
{
  extern LPVOID KERNEL_MultiByteToWideChar;
  extern LPVOID KERNEL_WideCharToMultiByte;
  static BOOL s_bAlreadyHooked = FALSE;
  BOOL bOk;
  if (s_bAlreadyHooked) {
    SetLastError(ERROR_ALREADY_EXISTS);
    return TRUE;
  }
  bOk = KERNEL_MultiByteToWideChar && KERNEL_WideCharToMultiByte;
  if (!bOk) { 
    SetLastError(ERROR_NOT_READY);
    return FALSE;
  }
  if (sm_pvMaxAppAddr == NULL) {
     // Functions with address above lpMaximumApplicationAddress require
     // special processing (Windows 98 only)
     SYSTEM_INFO si;
     GetSystemInfo(&si);
     sm_pvMaxAppAddr = si.lpMaximumApplicationAddress;
  }
  bOk = bOk && ReplaceIATEntryInOneMod("kernel32.dll", KERNEL_MultiByteToWideChar, CCP_MultiByteToWideChar, hInstance);
  bOk = bOk && ReplaceIATEntryInOneMod("kernel32.dll", KERNEL_WideCharToMultiByte, CCP_WideCharToMultiByte, hInstance);
  s_bAlreadyHooked = bOk;
  return bOk;
}
