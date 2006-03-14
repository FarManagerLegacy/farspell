/*
    Dialog resource parser implementation 
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
#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "dialogresInt.h"

#define STATIC 

#ifdef _DEBUG
// Simple, thread unsafe(!) memory leak and corruption detector
struct MemDbgChunkList
{
  size_t nBytes;
  char* xInfoGuard1;
  const char* zFile;
  unsigned  nLine;
  const char* zFreeFile;
  unsigned  nFreeLine;
  char* xInfoGuard2;
  struct MemDbgChunkList* prev;
  struct MemDbgChunkList* next;
  char* xTag;
};

size_t dialogresMemory = 0;
struct MemDbgChunkList * dialogresMemoryList;

// Makes useless pointer, but unique id:
#define SET_XTAG(xTag) (xTag) = (char*)(&(xTag))+1 
#define SET_PTAG(xTag) (pTag) = *(char**)(pTag) = (char*)(pTag)+1
#define CHECK_PTAG(pTag) ( *(char**)(pTag) == (char*)(pTag)+1 )

int dialogresTraceMemory(FILE *pFile, char* zPrefix)
{
  struct MemDbgChunkList *chunk;
  int nBytes = 0;
  for (chunk = dialogresMemoryList; chunk; chunk = chunk->prev)
  {
    fprintf(pFile, "%s%d bytes by %s:%d\n", zPrefix, chunk->nBytes, 
                                            chunk->zFile, chunk->nLine);
    nBytes += chunk->nBytes;
  }
  //printf("%d==%d", nBytes, dialogresMemory);
  assert(nBytes == dialogresMemory);
  return dialogresMemory;
}

void _dialogres_free(void **ppChunk, const char* zFile, unsigned nLine)
{
  void *pTag;
  struct MemDbgChunkList *chunk = (struct MemDbgChunkList *)(*ppChunk);
  if (!chunk)  {
    fprintf(stderr, "Freeing of NULL pointer in %s:%d\n", zFile, nLine); 
    exit(STATUS_ACCESS_VIOLATION);                       
  }
  chunk--;
  assert(!IsBadWritePtr(chunk, sizeof(struct MemDbgChunkList)));
  if (!CHECK_PTAG(&chunk->xTag))
  {
    // Freeing of Invalid chunk or corrupted block      
    fprintf(stderr, "Double free or corrupted block in %s:%d\n", zFile, nLine); 
    if (CHECK_PTAG(&chunk->xInfoGuard1) && CHECK_PTAG(&chunk->xInfoGuard2))
    {
      fprintf(stderr, "\tIt was allocated in %s:%d\n", chunk->zFile, chunk->nLine); 
      fprintf(stderr, "\tAnd freed in %s:%d\n", chunk->zFreeFile, chunk->nFreeLine); 
    }
    exit(STATUS_ACCESS_VIOLATION);                       
  }
  chunk->xTag = NULL;
  chunk->zFreeFile = zFile;
  chunk->nFreeLine = nLine;
  pTag = (char*)chunk+sizeof(struct MemDbgChunkList)+chunk->nBytes;
  if (!CHECK_PTAG(pTag))
  {
    fprintf(stderr, "Write beyond of the chunk (allocated in %s:%d, freeing in %s:%d)", chunk->zFile, chunk->nLine, zFile, nLine);
    exit(STATUS_ACCESS_VIOLATION);
  }
  dialogresMemory -= chunk->nBytes;
  if (chunk->prev)
  {
    pTag = &chunk->prev->xTag;
    assert(CHECK_PTAG(pTag)); // Leak detector corrupted
    chunk->prev->next = chunk->next;
  }
  if (chunk->next)
  {
    pTag = &chunk->next->xTag;
    assert(CHECK_PTAG(pTag)); // Leak detector corrupted
    chunk->next->prev = chunk->prev;
  }
  if (chunk == dialogresMemoryList)
    dialogresMemoryList = chunk->prev;
  //fprintf(stderr, ">%x\n", chunk);
  //fflush(stderr);
  free(chunk);
  *ppChunk = NULL;
}

void* _dialogres_malloc(size_t nBytes, const char* zFile, unsigned  nLine)
{
  char* pTag;
  struct MemDbgChunkList *chunk = 
 (struct MemDbgChunkList*)malloc(sizeof(struct MemDbgChunkList) + nBytes + sizeof(char*));
  //fprintf(stderr, "<%x %s:%d\n", chunk, file, line);
  //fflush(stderr);
  dialogresMemory += nBytes;
  SET_XTAG(chunk->xInfoGuard1);
  SET_XTAG(chunk->xInfoGuard2);
  SET_XTAG(chunk->xTag);
  chunk->nBytes = nBytes;
  chunk->zFile = zFile;
  chunk->nLine = nLine;
  chunk->next = NULL;
  chunk->prev = dialogresMemoryList;
  if (dialogresMemoryList)
  {
    assert(!dialogresMemoryList->next);
    dialogresMemoryList->next = chunk; 
  }
  dialogresMemoryList = chunk;
  chunk++;
  pTag=(char*)chunk+nBytes;
  SET_PTAG(pTag);
  return (void*)chunk;
}

void __dialogres_free(void *pChunk)
{
  _dialogres_free(&pChunk, __FILE__, __LINE__);
}

void* __dialogres_malloc(size_t nBytes)
{
  return _dialogres_malloc(nBytes, __FILE__, __LINE__);
}
#else

void* dialogres_malloc(size_t nBytes)
{
  return HeapAlloc(GetProcessHeap(), 0, nBytes);
  //return malloc(nBytes);
}
void dialogres_free(void* pChunk)
{
  //free(pChunk);
  HeapFree(GetProcessHeap(), 0, pChunk);
}

#endif

STATIC char* Collect(dialogres* pDr, char *pPool);
STATIC void Collect_Chunk(void** ppChunk, unsigned nBytes, char **ppPool, int release);
STATIC void Collect_Token(Token *s, char **ppPool);

enum dialogres_error dialogresParseInclude(Parse *pParse, const char* zFilename)
{
  enum dialogres_error nErr;
  char *pBuf;
  HANDLE hFile;
  DWORD nSize, nRead;
  BOOL bRead;
  struct InputPoolList *pPool;
  //
  assert(pParse);
  assert(zFilename);
  //
  hFile = CreateFile(zFilename, FILE_READ_DATA, FILE_SHARE_READ, 
                     NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return dialogres_OSError;
  //
  nSize = GetFileSize(hFile, NULL);
  if (nSize == INVALID_FILE_SIZE)
  {
    CloseHandle(hFile);
    return dialogres_OSError;
  }
  if (nSize>MAX_DIALOG_RESOURCE_SIZE)
  {
    CloseHandle(hFile);
    return dialogres_TooLarge;
  }
  //
  pPool = (struct InputPoolList *)dialogres_malloc(nSize+1 + sizeof(struct InputPoolList));
  assert(pPool);
  pPool->prev = pParse->pInputPoolList;
  pParse->pInputPoolList =  pPool;
  pBuf = (char*)++pPool;
  while ( bRead = ReadFile(hFile, pBuf, nSize, &nRead, NULL) && nRead)
  {
    assert(nRead <= nSize);
    pBuf[nRead] = '\0';
    dialogresRunParser(pParse, pBuf, 0);
    break;
  }
  CloseHandle(hFile);
  hFile = INVALID_HANDLE_VALUE;
  return pParse->rc;
error:
  if (hFile != INVALID_HANDLE_VALUE)
    CloseHandle(hFile);
  return pParse->rc;
}

enum dialogres_error dialogres_open(const char* zFilename, struct dialogres** ppDr)
{
  struct dialogres* pDr;
  Parse *pParse;
  enum dialogres_error nErr;
  char *pPoolEnd;
  size_t nSize;
  //
  assert(ppDr);
  assert(zFilename);
  *ppDr = NULL;
  //
  pDr = *ppDr = (dialogres *)dialogres_malloc(sizeof(dialogres));
  if (!pDr) 
  {
    nErr = dialogres_NoMemory;
    goto error;
  }
  memset(pDr, 0, sizeof(dialogres));
  pParse = dialogresParseAlloc(pDr);
  assert(pParse);
  pDr->rc = dialogresParseInclude(pParse, zFilename);
  if (pDr->rc != dialogres_Ok)
  {
    pDr->sErrToken = pParse->sErrToken;
    nSize = strlen(zFilename)+1;
    pParse->nPoolBytes = nSize + pDr->sErrToken.n+1; 
    pParse->nPoolBytes += nSize + pDr->sErrToken.n+1;
    pPoolEnd = pDr->pPool = (char*)dialogres_malloc(pParse->nPoolBytes);
    assert(pPoolEnd);
    Collect_Token(&pDr->sErrToken, &pPoolEnd);
    strncpy(pPoolEnd, zFilename, nSize);
    pPoolEnd += nSize;
    DialogTemplateFree(pDr->pDialogs, 1);
    dialogitemFree(pParse->pFistDialogItem, 1);
  }
  else
  {
    assert(!pParse->pFistDialogItem);
    nSize = strlen(zFilename)+1;
    pParse->nPoolBytes += nSize + pDr->sErrToken.n+1;
    pDr->pPool = (char*)dialogres_malloc(pParse->nPoolBytes);
    assert(pDr->pPool);
    pPoolEnd = Collect(pDr, pDr->pPool);
    strncpy(pPoolEnd, zFilename, nSize);
    pPoolEnd += nSize;
  }
  dialogresParseFree(pParse);
#ifdef _DEBUG
  printf("// pool size=%d unused pool %d\n", pParse->nPoolBytes, (pDr->pPool+pParse->nPoolBytes) - pPoolEnd);
#endif _DEBUG
  assert(pPoolEnd <= (pDr->pPool + pParse->nPoolBytes));
  return pDr->rc;
fatal_error:
  if (pDr)
    dialogres_close(pDr);
  *ppDr = NULL;
error:
  if (pDr) pDr->rc = nErr;
  return nErr;
}

void dialogres_close(struct dialogres* pDr)
{
  if (!pDr) return;
  if (pDr->pPool) dialogres_free(pDr->pPool);
  dialogres_free(pDr);
}


enum dialogres_error dialogres_last_error(dialogres* pDr)
{
  assert(pDr);
  return pDr->rc;
}

int dialogres_error_pos(dialogres* pDr)
{
  assert(pDr);
  return pDr->sErrToken.pos;
}

int dialogres_error_line(dialogres* pDr)
{
  assert(pDr);
  return pDr->sErrToken.line;
}

int dialogres_dialogs_count(dialogres* pDr)
{
  assert(pDr);
  return pDr->nDialogs;
}

enum dialogres_error dialogres_get_dialog(dialogres* pDr, char *zName, dialogtemplate** ppDt)
{
  assert(pDr);
  assert(ppDt);
  for (*ppDt = pDr->pDialogs; *ppDt; *ppDt = (*ppDt)->prev)
  {
    if (strcmp((*ppDt)->sName.z, zName) == 0) break;
  }
  return *ppDt ? dialogres_Ok : dialogres_NotFound;
}

enum dialogres_error dialogres_last_dialog(dialogres* pDr, dialogtemplate** ppDt)
{
  assert(pDr);
  assert(ppDt);
  return (*ppDt = pDr->pDialogs) ? dialogres_Ok : dialogres_NotFound;
}

enum dialogres_error dialogres_prev_dialog(dialogres* pDr, dialogtemplate** ppDt)
{
  assert(pDr);
  assert(ppDt);
  return (*ppDt = (*ppDt)->prev) ? dialogres_Ok : dialogres_NotFound;
}

enum dialogres_error dialogtemplate_create_items(dialogtemplate* pDt, int untemplate, struct FarDialogItem **ppFarItems)
{
  struct dialogitem *pItem;
  unsigned nBytes;
  struct FarDialogItem *pFarItem, *pFarItems;
  assert(pDt);
  assert(ppFarItems);
  nBytes = pDt->nItems * sizeof(struct FarDialogItem); // full size
  pFarItems = *ppFarItems = (struct FarDialogItem *)dialogres_malloc(nBytes);
  if (!pFarItems) return dialogres_NoMemory;
  memset(pFarItems, 0, nBytes);
  pFarItem = pFarItems;
  for (pItem = pDt->pItems; pItem; (pItem = pItem->next), pFarItem++)
  {
    memcpy(pFarItem, pItem->pFarDialogItem, SIZEOF_FAR_DIALOG_ITEM_TEMPLATE);
    if (untemplate)
    {
    }
  }
  assert(pFarItem == pFarItems+pDt->nItems);
  return dialogres_Ok;
}

int dialogtemplate_index_by_id(dialogtemplate* pDt, int nId)
{
  struct dialogitem *pItem;
  int idx = 0;
  assert(pDt);
  for (pItem = pDt->pItems; pItem; pItem = pItem->next)
  {
    if (pItem->nId == nId) return idx;
    idx++;
  }
  return -1;
}

int dialogtemplate_items_count(dialogtemplate* pDt)
{
  assert(pDt);
  return pDt->nItems;
}

const char* dialogtemplate_name(dialogtemplate* pDt)
{
  assert(pDt);
  return pDt->sName.z;
}

const char* dialogtemplate_help(dialogtemplate* pDt)
{
  assert(pDt);
  return pDt->sHelp.z;
}

enum dialogres_error dialogtemplate_first_item(dialogtemplate* pDt, dialogitem** ppDi)
{
  assert(pDt);
  assert(ppDi);
  return (*ppDi = pDt->pItems) ? dialogres_Ok : dialogres_NotFound;
}

enum dialogres_error dialogtemplate_next_item(dialogtemplate* pDt, dialogitem** ppDi)
{
  assert(pDt);
  assert(ppDi);
  return (*ppDi = (*ppDi)->next) ? dialogres_Ok : dialogres_NotFound;
}

enum dialogres_error dialogtemplate_get_item(dialogtemplate* pDt, int nId, int nIndex, dialogitem** ppDi)
{
  struct dialogitem *pItem;
  int i = 0;
  assert(pDt);
  assert(ppDi);
  for (pItem = pDt->pItems; pItem; pItem = pItem->next)
  {
    if ((nIndex == i) || (nIndex<0 && pItem->nId==nId)) 
    {
      *ppDi = pItem;
      return dialogres_Ok;
    }
    i++;
  }
  assert(i == pDt->nItems);
  *ppDi = NULL;
  return dialogres_NotFound;
}

int dialogitem_id(dialogitem *pDi)
{
  assert(pDi);
  return pDi->nId;
}

const char* dialogitem_sid(dialogitem *pDi)
{
  assert(pDi);
  return pDi->sId.z;
}

int dialogitem_index(dialogitem *pDi)
{
  assert(pDi);
  return pDi->nIndex;
}

int dialogitem_datasource_type(dialogitem *pDi)
{
  assert(pDi);
  assert(pDi->pDataSource);
  return pDi->pDataSource->nType;
}

const char *dialogitem_datasource_text(dialogitem *pDi)
{
  assert(pDi);
  assert(pDi->pDataSource);
  assert(pDi->pDataSource->nType==DI_DATASOURCE_TEXT);
  return pDi->pDataSource->txt_data.z;
}

int dialogitem_datasource_msgid(dialogitem *pDi)
{
  assert(pDi);
  assert(pDi->pDataSource);
  assert(pDi->pDataSource->nType==DI_DATASOURCE_MSGID);
  return pDi->pDataSource->msg_data.nMsgId;
}

const char* dialogitem_datasource_smsgid(dialogitem *pDi)
{
  assert(pDi);
  assert(pDi->pDataSource);
  assert(pDi->pDataSource->nType==DI_DATASOURCE_MSGID);
  return pDi->pDataSource->msg_data.sMsgId.z;
}

const char *dialogitem_history(dialogitem *pDi)
{
  assert(pDi);
  assert(pDi->sHistory.z);
  return pDi->sHistory.z;
}

void AddIntBind(Parse *pParse, Token sName, unsigned nValue)
{
  TokenIntBind *bind = (TokenIntBind *)dialogres_malloc(sizeof(TokenIntBind));
  if (!bind)
  {
    pParse->rc = dialogres_NoMemory;
    return;
  }
  bind->name = sName;
  bind->value = nValue;
  bind->prev = pParse->binds;
  pParse->binds = bind;
}

STATIC void Collect_Chunk(void** ppChunk, unsigned nBytes, char **ppPool, int release)
{
  void *pOld = *ppChunk;
  *ppChunk = *ppPool;
  memcpy(*ppChunk, pOld, nBytes);
  (*ppPool) += nBytes;
  if (release)
    dialogres_free(pOld);
}

STATIC void Collect_Token(Token *s, char **ppPool)
{
  const char *o;
  char *m;
  if (s->s) return; // true STATIC
  o = s->z;
  s->z = (*ppPool);
  (*ppPool) += s->n+1;
  strncpy(m=(char*)s->z, o, s->n);
  m[s->n] = '\0';
}

STATIC void Collect_dialogitem(struct dialogitem *pItem, char **ppPool) 
{
  for (; pItem; pItem = pItem->next)
  {  
    Collect_Token(&pItem->sId, ppPool); 
    assert(pItem->pDataSource);
    switch (pItem->pDataSource->nType)
    {
       case DI_DATASOURCE_NULL: 
         break;
       case DI_DATASOURCE_MSGID: 
         Collect_Token(&pItem->pDataSource->msg_data.sMsgId, ppPool); 
         break;
       case DI_DATASOURCE_TEXT: 
         Collect_Token(&pItem->pDataSource->txt_data, ppPool); 
         break;
       case DI_DATASOURCE_REGISTRY: 
         Collect_Token(&pItem->pDataSource->reg_data.key, ppPool);  
         if (pItem->pDataSource->reg_data.def.type == bytes)
           Collect_Token(&pItem->pDataSource->reg_data.def.str, ppPool);  
         break;
       default:
         assert(0);
    }
    Collect_Chunk((void**)&pItem->pDataSource, sizeof(struct DataSource), ppPool, 1); 
    assert(pItem->pFarDialogItem);
    Collect_Chunk((void**)&pItem->pFarDialogItem, SIZEOF_FAR_DIALOG_ITEM_TEMPLATE, ppPool, 1);
    Collect_Token(&pItem->sHistory, ppPool); 
    if (pItem->next)
      Collect_Chunk((void**)&pItem->next, sizeof(struct dialogitem), ppPool, 1);
  }
}

STATIC void Collect_dialogtemplate(struct dialogtemplate *pDt, char **ppPool) 
{
  for (; pDt; pDt = pDt->prev)
  {  
    Collect_Token(&pDt->sName, ppPool);
    Collect_Token(&pDt->sHelp, ppPool);
    if (pDt->pItems)
    {
      Collect_Chunk((void**)&pDt->pItems, sizeof(struct dialogitem), ppPool, 1);
      Collect_dialogitem(pDt->pItems, ppPool);
    }
    if (pDt->prev)
      Collect_Chunk((void**)&pDt->prev, sizeof(struct dialogtemplate), ppPool, 1);
  }
}

STATIC char* Collect(dialogres* pDr, char *pPool)
{
  if (pDr->pDialogs)
  {
    Collect_Chunk((void**)&pDr->pDialogs, sizeof(struct dialogtemplate), &pPool, 1);
    Collect_dialogtemplate(pDr->pDialogs, &pPool);
  }
  Collect_Token(&pDr->sErrToken, &pPool);
  return pPool;
}

unsigned LookupIntBind(Parse *pParse, Token sName)
{
  TokenIntBind *bind;   
  for (bind = pParse->binds; bind; bind = bind->prev)
  {
    if ((sName.n==bind->name.n) && (strncmp(sName.z, bind->name.z, sName.n)==0))
      return bind->value;
  }
  pParse->rc = dialogres_UnknownIdentifier;
  pParse->sErrToken = sName;
  return 0;
}

void DialogTemplateFree(struct dialogtemplate *pItem, int AllRest)
{
  struct dialogtemplate *tmp;
  while (pItem)
  {
     dialogitemFree(pItem->pItems, 1);
     assert(pItem != pItem->prev);
     tmp = pItem;
     pItem = AllRest ? pItem->prev : NULL;
     dialogres_free(tmp);
  }
}

Parse* dialogresParseAlloc(dialogres* pDr)
{
  Parse* pParse = (Parse*)dialogres_malloc(sizeof(Parse));
  if (pParse)
  {
    memset(pParse, 0, sizeof(Parse));
    pParse->pDr = pDr;
  }
  return pParse;
}

void dialogresParseFree(Parse* pParse)
{
  TokenIntBind *bind, *tmp;   
  struct InputPoolList *pList, *pTmp;
  if (!pParse) return;
  for (bind = pParse->binds; bind; )
  {
    tmp = bind;
    bind = bind->prev;
    dialogres_free(tmp);
  }
  for (pList = pParse->pInputPoolList; pList;) {
    pTmp = pList;
    pList = pList->prev;
    dialogres_free(pTmp);
  }
  dialogres_free(pParse);
}

struct DataSource* dialogitemDataSourceAlloc(int nType)
{
  ALLOC_STRUCT_VAR(pDataSource, DataSource);
  pDataSource->nType = nType;
  return pDataSource;
}

void dialogitemDataSourceFree(struct DataSource* pData)
{
  dialogres_free(pData);
}

//extern FILE *yyTraceFILE;
struct dialogitem* dialogitemAlloc(Parse* pParse, int nType, Token sId, struct FarDialogItem *pFarDialogItem, struct DataSource *pDataSource)
{
  extern Token dialogres_itemid_is_msgid_token;
  static Token null_token =     { "",  1, 0, 0, 0 };
  struct dialogitem* pDialogItem = ALLOC_STRUCT(dialogitem);
  if (!pDialogItem)
  {
    pParse->rc = dialogres_NoMemory;
    return NULL;
  }
  ERASE_STRUCT(pDialogItem, dialogitem);
  //fflush(yyTraceFILE);
  assert(sId.n);
  assert(sId.z);
  if (sId.z == dialogres_itemid_is_msgid_token.z) {
    assert(pDataSource);
    assert(pDataSource->nType == DI_DATASOURCE_MSGID);  // FIXME: produce parser error.
    pDialogItem->sId = pDataSource->msg_data.sMsgId;
  }  else
    pDialogItem->sId = sId;
  pDialogItem->nId = LookupIntBind(pParse, pDialogItem->sId);
  if (pParse->rc == dialogres_UnknownIdentifier)
  {
    pDialogItem->nId = toint(sId);
    clearToken(&pParse->sErrToken);
    pParse->rc = dialogres_Ok;
  }
  pParse->nPoolBytes += pDialogItem->sId.n+1;
  pFarDialogItem->Type = nType;
  pDialogItem->pFarDialogItem = pFarDialogItem;    
  pDialogItem->pDataSource = pDataSource;
  pDialogItem->sHistory = null_token;
  pDialogItem->next = NULL;
  return pDialogItem;
}

void dialogitemFree(struct dialogitem* pItem, int AllRest)
{
  struct dialogitem* tmp;
  while (pItem)
  {
    if (pItem->pFarDialogItem) 
      dialogres_free(pItem->pFarDialogItem);
    dialogitemDataSourceFree(pItem->pDataSource);
    tmp = pItem;
    assert(pItem != pItem->next);
    pItem = AllRest ? pItem->next : NULL;
    dialogres_free(tmp);
  }
}
