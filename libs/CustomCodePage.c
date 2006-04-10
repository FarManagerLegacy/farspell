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
 * The Original Code is the Custom CodePage Library.
 *
 * The Initial Developer of the Original Code is
 * Sergey Shishmintzev <sergey.shihshmintzev@gmail.com>.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#include <malloc.h>

#ifdef _MT
#define MULTITHREAD
#else
#undef MULTITHREAD
#endif

typedef int (__stdcall *MultiByteToWideChar_t)(
  UINT CodePage,         // code page
  DWORD dwFlags,         // character-type options
  LPCSTR lpMultiByteStr, // string to map
  int cbMultiByte,       // number of bytes in string
  LPWSTR lpWideCharStr,  // wide-character buffer
  int cchWideChar        // size of buffer
);
typedef int (__stdcall *WideCharToMultiByte_t)(
  UINT CodePage,            // code page
  DWORD dwFlags,            // performance and mapping flags
  LPCWSTR lpWideCharStr,    // wide-character string
  int cchWideChar,          // number of chars in string
  LPSTR lpMultiByteStr,     // buffer for new string
  int cbMultiByte,          // size of buffer
  LPCSTR lpDefaultChar,     // default for unmappable chars
  LPBOOL lpUsedDefaultChar  // set when default char used
);

static BOOL g_CustomCodePagesInitialized = FALSE;
MultiByteToWideChar_t KERNEL_MultiByteToWideChar;
WideCharToMultiByte_t KERNEL_WideCharToMultiByte;
       
typedef wchar_t ascii_to_wide_t[0x100];
typedef unsigned char wide_to_ascii_t[0x10000];
typedef struct codepage_t { 
  ascii_to_wide_t ascii_to_wide;
  wide_to_ascii_t wide_to_ascii;
  UINT nCodePage;
  CHAR zName[MAX_PATH];
  CHAR zFilename[MAX_PATH];
} codepage_t;

static codepage_t *pCodePages = NULL;
static UINT nCodePages = 0;
#ifdef MULTITHREAD
static CRITICAL_SECTION g_CodePageCacheCS;
#endif //MULTITHREAD

static codepage_t *FindCodePage(UINT nCodePage);

int __stdcall CCP_MultiByteToWideChar(
  UINT CodePage,         // code page
  DWORD dwFlags,         // character-type options
  LPCSTR lpMultiByteStr, // string to map
  int cbMultiByte,       // number of bytes in string
  LPWSTR lpWideCharStr,  // wide-character buffer
  int cchWideChar        // size of buffer
) 
{
  codepage_t *pCodePage = FindCodePage(CodePage);
  if (pCodePage) {
    int cchOut = 0;
    if (cbMultiByte<0)
      cbMultiByte = strlen(lpMultiByteStr) + 1;
    if (!lpWideCharStr || !cchWideChar)  
      return cbMultiByte;
    for (;cchWideChar>0 && cbMultiByte>0; cchWideChar--, cbMultiByte--, cchOut++) 
      *(lpWideCharStr++) = pCodePage->ascii_to_wide[*((unsigned char *)lpMultiByteStr++)];
    return cchOut;
  }
  return KERNEL_MultiByteToWideChar(CodePage, dwFlags, 
    lpMultiByteStr, cbMultiByte, 
    lpWideCharStr, cchWideChar);
}

int __stdcall CCP_WideCharToMultiByte(
  UINT CodePage,            // code page
  DWORD dwFlags,            // performance and mapping flags
  LPCWSTR lpWideCharStr,    // wide-character string
  int cchWideChar,          // number of chars in string
  LPSTR lpMultiByteStr,     // buffer for new string
  int cbMultiByte,          // size of buffer
  LPCSTR lpDefaultChar,     // default for unmappable chars
  LPBOOL lpUsedDefaultChar  // set when default char used
)
{
  codepage_t *pCodePage = FindCodePage(CodePage);
  if (pCodePage) {
    int cbOut = 0;
    if (cchWideChar<0)
      cchWideChar = wcslen(lpWideCharStr) + 1;
    if (!lpMultiByteStr || !cbMultiByte)  
      return cchWideChar;
    for (;cchWideChar>0 && cbMultiByte>0; cchWideChar--, cbMultiByte--, cbOut++) {
      *(lpMultiByteStr++) = pCodePage->wide_to_ascii[*((unsigned short*)lpWideCharStr++)];
    }
    return cbOut;
  }
  return KERNEL_WideCharToMultiByte(CodePage, dwFlags, 
    lpWideCharStr, cchWideChar,     
    lpMultiByteStr, cbMultiByte,
    lpDefaultChar, lpUsedDefaultChar);
}


BOOL InitializeCustomCodePages()
{
  HMODULE hKernel32 = LoadLibrary("kernel32.dll");
  if (g_CustomCodePagesInitialized) {
    SetLastError(ERROR_ALREADY_EXISTS);
    return TRUE;
  }
  SetLastError(NO_ERROR);
  pCodePages = NULL;
  nCodePages = 0;
  KERNEL_MultiByteToWideChar = 
    (MultiByteToWideChar_t)GetProcAddress(hKernel32, "MultiByteToWideChar");
  KERNEL_WideCharToMultiByte = 
    (WideCharToMultiByte_t)GetProcAddress(hKernel32, "WideCharToMultiByte");
  g_CustomCodePagesInitialized = KERNEL_MultiByteToWideChar && KERNEL_WideCharToMultiByte;
  if (g_CustomCodePagesInitialized) {
#ifdef MULTITHREAD
    InitializeCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
    return TRUE;
  } else
    return FALSE;
}

VOID FinalizeCustomCodePages()
{
  if (!g_CustomCodePagesInitialized) {
    SetLastError(ERROR_NOT_READY);
    return;
  }
  SetLastError(NO_ERROR);
#ifdef MULTITHREAD
  EnterCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
  free(pCodePages);
#ifdef MULTITHREAD
  //LeaveCriticalSection(&g_CodePageCacheCS);
  DeleteCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
}

static VOID InitAsciiToWide(ascii_to_wide_t *pTable)
{
  int i;
  for (i = 0; i < sizeof(ascii_to_wide_t); i++) 
    (*pTable)[i] = (wchar_t)i;
}

static VOID InitWideToAscii(wide_to_ascii_t *pTable, char cUnknown)
{
  FillMemory(pTable, sizeof(wide_to_ascii_t), cUnknown);
}

static VOID ReverseTable(ascii_to_wide_t *pSrc, wide_to_ascii_t *pDest)
{
  int i;
  for (i = 0; i < sizeof(ascii_to_wide_t); i++) 
    (*pDest)[(unsigned short)((*pSrc)[i])] = i;
}

static BOOL LoadAsciiToWide(HINSTANCE hInstance, LPCSTR zFilename, ascii_to_wide_t *pTable)
{
  enum { nBufSize = 0x8000 };
  HANDLE hFile;
  DWORD nSize;
  LPSTR pBuf;
  DWORD nRead;
  BOOL bStatus = FALSE;
  char zPath[MAX_PATH], *zPathChar;

  SetLastError(NO_ERROR);

  nRead = GetModuleFileName(hInstance, zPath, sizeof(zPath)/sizeof(zPath[0]));
  if (nRead) {
    for (zPathChar = zPath + nRead - 1; 
         zPathChar>zPath && *zPathChar!='\\';
         *(zPathChar--) = '\0');
    strcat(zPathChar, zFilename);
    zFilename = zPath;
  } 

  hFile = CreateFile(zFilename, GENERIC_READ, FILE_SHARE_READ, 
                     NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (hFile == INVALID_HANDLE_VALUE) 
    return FALSE;
  nSize = GetFileSize(hFile, NULL);
  if (nSize == INVALID_FILE_SIZE)
    goto close;
  if (nSize == 0 || nSize > nBufSize) {
    SetLastError(ERROR_BAD_LENGTH);
    goto close;
  }

  pBuf = (LPSTR)alloca(nBufSize);

  if (!ReadFile(hFile, pBuf, nBufSize, &nRead, NULL) || !nRead)
    goto close;

  { // 0x00    0x0000  #NULL
    LPSTR zLine = pBuf, zBegin;
    LPSTR zNextLine = pBuf;
    UCHAR c;
    WCHAR w;
    LONG l;
    BOOL bOneAsciiZeroReceived = FALSE;
    do {
      for (zLine = zNextLine; *zLine == ' ' || *zLine == '\t'; zLine++);

      zNextLine = strchr(zLine, 0x0D);
      if (!zNextLine)
        zNextLine = strchr(zLine, 0x0A);
      if (zNextLine) 
        while (*zNextLine == 0x0D || *zNextLine == 0x0A) 
          *(zNextLine++) = '\0';
      if (*zLine != '0') 
        continue;

      l = strtol(zBegin = zLine, &zLine, 16);
      if (zBegin == zLine || l > 0xFF || l < 0) 
        continue; // TODO? Warning
      if (l == 0) {
        if (bOneAsciiZeroReceived) 
          continue;
        else
          bOneAsciiZeroReceived = TRUE;
      }
      c = l;
      
      for (; *zLine == ' ' || *zLine == '\t'; zLine++);

      l = strtol(zBegin = zLine, &zLine, 16);
      if (zBegin == zLine || l > 0xFFFF || l < 0) 
        continue; // TODO? Warning
      w = l;

      (*pTable)[c] = w;

    } while (zNextLine);
  }
  bStatus = TRUE;
close:
  CloseHandle(hFile);
  return bStatus;
}

BOOL LoadCustomCodePage(HINSTANCE hInstance, UINT nCodePage, LPCSTR zFilename, LPCSTR zName)
{
  codepage_t sCodePage;
  if (!g_CustomCodePagesInitialized) {
    SetLastError(ERROR_NOT_READY);
    return FALSE;
  }
  InitAsciiToWide(&sCodePage.ascii_to_wide);
  InitWideToAscii(&sCodePage.wide_to_ascii, '?');
  if (!LoadAsciiToWide(hInstance, zFilename, &sCodePage.ascii_to_wide))
    return FALSE;
  ReverseTable(&sCodePage.ascii_to_wide, &sCodePage.wide_to_ascii);
  sCodePage.nCodePage = nCodePage;
  strncpy(sCodePage.zFilename, zFilename, sizeof(sCodePage.zFilename));
  strncpy(sCodePage.zName, zName? zName:zFilename, sizeof(sCodePage.zFilename));
#ifdef MULTITHREAD
  EnterCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
  nCodePages++;
  pCodePages = (codepage_t*)realloc(pCodePages, nCodePages*sizeof(codepage_t));
  pCodePages[nCodePages-1] = sCodePage;
#ifdef MULTITHREAD
  LeaveCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
  return TRUE;
}

static codepage_t *FindCodePage(UINT nCodePage)
{
  codepage_t *pCodePage = NULL, *p;
  int cnt;
#ifdef MULTITHREAD
  EnterCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
  if (!pCodePages) 
    goto leave;
  for ((cnt = nCodePages), (p = pCodePages); cnt; cnt--, p++) 
  {
    if (p->nCodePage == nCodePage) {
      pCodePage = p;
      goto leave;
    }
  }
leave:
#ifdef MULTITHREAD
  LeaveCriticalSection(&g_CodePageCacheCS);
#endif //MULTITHREAD
  return pCodePage;
}

