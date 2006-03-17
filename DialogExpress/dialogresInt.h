/*
    Dialog resource parser internal header
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

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "dialogres.h"

#if FARMANAGERVERSION < MAKEFARVERSION(1,70,1975)
#define DIF_LISTNOCLOSE 0x04000000UL
#endif

#ifdef _DEBUG
void* _dialogres_malloc(size_t nBytes, const char* zFile, unsigned nLine);
void* __dialogres_malloc(size_t nBytes);
void _dialogres_free(void **ppChunk, const char* zFile, unsigned nLine);
void __dialogres_free(void *pChunk);
#define dialogres_malloc(s) _dialogres_malloc(s, __FILE__, __LINE__)
#define dialogres_free(s) { assert(s); _dialogres_free((void**)&s, __FILE__, __LINE__); }
#else
#define __dialogres_free dialogres_free
#define __dialogres_malloc dialogres_malloc
void* dialogres_malloc(size_t nBytes);
void dialogres_free(void *pChunk);
#endif

#define SIZEOF_FAR_DIALOG_ITEM_TEMPLATE (sizeof(struct FarDialogItem) - sizeof(char[512]))


typedef struct
{
  const char *z;          /* Text of the token.  Not NULL-terminated! */
  unsigned s    : 1;      /* True if static / no malloc'ed / always live  */
  unsigned n    : 31;     /* Number of characters in this token */
  unsigned line : 22;
  unsigned pos  : 10;
} Token;

typedef struct _TokenIntBind
{
  Token name;
  unsigned value;
  struct _TokenIntBind *prev;
} TokenIntBind;

typedef struct
{
  dialogres *pDr;
  //char *ErrMsg;
  struct InputPoolList { 
    struct InputPoolList* prev; 
  } *pInputPoolList;
  const char *Stmt;
  //const char *Tail;
  enum dialogres_error rc;
  int nEnumIndex;
  Token sLastToken;
  Token sErrToken;
  unsigned nLineNo;
  const char* zLastLine;
  TokenIntBind *binds;
  size_t nPoolBytes;
  struct dialogitem *pFistDialogItem;
} Parse;


enum dialogres_error dialogresParseInclude(Parse *pParse, const char* zFilename);
Parse* dialogresParseAlloc(dialogres* pDr);
void dialogresParseFree(Parse* pParse);
void AddIntBind(Parse *pParse, Token sName, unsigned nValue);
unsigned LookupIntBind(Parse *pParse, Token sName);

void dialogresInitEnvironment(Parse *pParse);
int dialogresLookupColorId(Parse *pParse, Token sId);

void *dialogresParserAlloc(void *(*mallocProc)(size_t));
void dialogresParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
);
void dialogresParser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  Token yyminor,               /* The value for the token */
  Parse *pParse                /* Optional %extra_argument parameter */
);


void dialogresParserTrace(FILE *TraceFILE, char *zTracePrompt);

struct RegDefault {
  enum { bytes, dword } type;
  union
  {
    Token str;
    DWORD dw;
  };
};


struct DataSource {
  int nType;
  union {
    Token txt_data;
    struct {
      unsigned nMsgId;
      Token sMsgId;
    } msg_data;
    struct {
      HKEY root;
      Token key;
      struct RegDefault def;
    } reg_data;
  };
};

struct DataSource* dialogitemDataSourceAlloc(int nType);
void dialogitemDataSourceFree(struct DataSource* pData);

struct dialogitem
{
  int nId;
  Token sId;
  int nIndex;
  struct FarDialogItem *pFarDialogItem;
  struct DataSource *pDataSource;
  Token sHistory;
  struct dialogitem *next;
};

struct dialogitem* dialogitemAlloc(Parse* pParse, int nType, Token sId, struct FarDialogItem *tmpl, struct DataSource *data);
void dialogitemFree(struct dialogitem* pItem, int AllRest);

struct dialogtemplate
{
  Token sName;
  Token sHelp;
  unsigned nItems;
  struct dialogitem *pItems;
  struct dialogtemplate *prev;
};

void DialogTemplateFree(struct dialogtemplate *p, int AllRest);

struct dialogres
{
  enum dialogres_error rc;
  char* zFilename;
  struct dialogtemplate *pDialogs;
  unsigned nDialogs;
  Token sErrToken;
  char *pPool;
};

#define ALLOC_STRUCT(TYPE) (struct TYPE*)dialogres_malloc(sizeof(struct TYPE))
#define ERASE_STRUCT(VAR, TYPE) memset(VAR, 0, sizeof(struct TYPE))
#define ALLOC_STRUCT_VAR(VAR, TYPE) struct TYPE* VAR = ALLOC_STRUCT(TYPE);\
  assert(VAR); \
  ERASE_STRUCT(VAR, TYPE)

static __inline int toint(Token s)
{
  if (s.n>2 && s.z[1]=='x')
    return strtol(&s.z[2], NULL, 16);
  else if (s.n>3 && s.z[0]=='-' && s.z[2]=='x')
    return -strtol(&s.z[3], NULL, 16);
  else
    return atol(s.z);
}

static __inline void clearToken(Token *s)
{
  s->n=0;
  s->z=NULL;
}
