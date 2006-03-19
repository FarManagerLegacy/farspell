/*
    Template parser for MetaDialog
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
#include <stdlib.h>
#include "metaparse.h"

static char* PredicateStr[] = {
  "x1?",
  "y1?",
  "x2?",
  "y2?",
  "focused?",
  "selected?",
  "flags?",
  "msgid?", 
  "id?", 
  "history?",
  "text?",
  "color.index?",
  "foreground.index?",
  "background.index?",
  0
};

static char* VariableStr[] = {
  "dialog.id", 
  "dialog.cid", 
  "dialog.helptopic", 
  "dialog.width",
  "dialog.height",  
  "items.count",  
  "index",  
  "id",  
  "sid",
  "cid",
  "type",  
  "x1",  
  "y1",  
  "x2",  
  "y2",  
  "focused",  
  "selected",  
  "flags",
  "msgid",
  "smsgid",
  "text",
  "cdata",
  "history",
  "color.index",
  "foreground.index",
  "background.index",
  0,
};

struct Caret 
{
  const char* z;
  unsigned nLine;
  unsigned nPos;
};

struct Parse
{
  enum { stLiteral=0, stWaitLP, stCommand, stCommandArgs, stComment } nState;
  TemplateToken *pFirstToken;
  TemplateToken *pLastToken;
  TemplateToken *pItemLoopToken;
  TemplateToken *pDialogLoopToken;
  struct Caret sCaret;
  struct Caret sLastCaret;
  int nErrors;
  const char *zFilename;
};

static void PrintCaret(FILE* pFile, struct Parse *pParse)
{
  fprintf(pFile, "%s(%d,%d): ", 
    pParse->zFilename, 
    pParse->sCaret.nLine,
    pParse->sCaret.nPos);
}

static void PrintLastCaret(FILE* pFile, struct Parse *pParse)
{
  fprintf(pFile, "%s(%d,%d): ", 
    pParse->zFilename, 
    pParse->sLastCaret.nLine,
    pParse->sLastCaret.nPos);
}

static void SwitchState(int nState, struct Parse *pParse)
{
  pParse->nState = nState; 
  pParse->sLastCaret = pParse->sCaret;
  pParse->sLastCaret.z++;
}

static void AddToken(TemplateToken *pToken, struct Parse *pParse)
{
  pToken->next = NULL;
  pToken->nLine = pParse->sLastCaret.nLine;
  pToken->nPos = pParse->sLastCaret.nPos;
  if (pParse->pLastToken) {
    pParse->pLastToken->next = pToken;
    pToken->nIndex = pParse->pLastToken->nIndex + 1;
  }
  if (!pParse->pFirstToken) {
    pToken->nIndex = 0;
    pParse->pFirstToken = pToken;
  }
  pParse->pLastToken = pToken;
}

static void AddLiteral(const char* zLit, int nChars, struct Parse *pParse)
{
  TemplateToken *pToken;
  if (!nChars) return;
  pToken = (TemplateToken*)malloc(sizeof(TemplateToken) + (nChars+1)*sizeof(char));  
  assert(pToken);
  memset(pToken, 0, sizeof(TemplateToken));
  if ( (nChars==1 && *zLit=='\n') || (nChars==2 && *zLit=='\n' && *zLit=='\r'))
    pToken->nType = Newline;
  else
  {
    if (nChars>2 && *zLit=='\n' && *zLit=='\r')
    {
      AddLiteral(zLit, 2, pParse); // break tokens
      zLit+=2;
      nChars-=2;
    } else if (nChars>1 && *zLit=='\n') {
      AddLiteral(zLit, 1, pParse); // break tokens
      zLit++;
      nChars--;
    }
    pToken->nType = Literal;
  }
  pToken->nChars = nChars;
  AddToken(pToken, pParse);
  pToken++;
  strncpy((char*)pToken, zLit, nChars);
  ((char*)pToken)[nChars]='\0';
}

static void AddCommand(const char *zCmd, struct Parse *pParse)
{
  TemplateToken *pToken;
  int nChars = strlen(zCmd);
  char **pzVar;
  if (!nChars) return;
  pToken = (TemplateToken*)malloc(sizeof(TemplateToken)+(nChars+1)*sizeof(char));
  assert(pToken);
  memset(pToken, 0, sizeof(TemplateToken));
  if ( strcmp(zCmd, "other")==0 )
    pToken->nType = Alternative;
  else if (strcmp(zCmd, "common")==0)
    pToken->nType = Common;
  else {
    pToken->nType = Variable;
    pToken->nChars = nChars;
    for (pzVar = VariableStr; *pzVar; pzVar++)
      if (strcmp(*pzVar, zCmd)==0)
      {
        pToken->nType = InternalVariable;
        pToken->nVariable = pzVar-VariableStr;
      }
  }
  AddToken(pToken, pParse);
  pToken++;
  strncpy((char*)pToken, zCmd, nChars);
  ((char*)pToken)[nChars]='\0';
}

static int LoopType(char *z, struct Parse *pParse)
{
  if (strcmp(z, "item")==0) return 0;
  if (strcmp(z, "dialog")==0) return 1;
  PrintLastCaret(stderr, pParse);
  fprintf(stderr, "Unknown loop '%s'\n", z);
  pParse->nErrors++;
  return -1;
}

static void AddCommandArg(char *zCmd, char *zArg, struct Parse *pParse)
{
  TemplateToken *pToken, **ppLoopToken;
  char **pzPred;
  int nChars = strlen(zCmd);
  if (!*zCmd) return;
  pToken = (TemplateToken*)malloc(sizeof(TemplateToken)+(nChars+1)*sizeof(char));
  assert(pToken);
  memset(pToken, 0, sizeof(TemplateToken));
  if ( strcmp(zCmd, "has")==0 )
    pToken->nType = HasCondition;
  else if ( strcmp(zCmd, "hasn't")==0 )
    pToken->nType = HasNotCondition;
  else if ( strcmp(zCmd, "hasnot")==0 )
    pToken->nType = HasNotCondition;
  else if (strcmp(zCmd, "foreach")==0)
    pToken->nType = LoopType(zArg, pParse) ? ForeachDialog : ForeachItem;
  else if (strcmp(zCmd, "next")==0)
    pToken->nType = LoopType(zArg, pParse) ? NextDialog : NextItem;
  else {
    PrintLastCaret(stderr, pParse);
    fprintf(stderr, "Unknown command '%s'\n", zCmd);
    pParse->nErrors++;
    pToken->nType = Variable;
  }
  switch (pToken->nType)
  {
    case HasCondition:
    case HasNotCondition:
      pToken->nPredicate = -1;
      for (pzPred = PredicateStr; *pzPred; pzPred++)
        if (strcmp(*pzPred, zArg)==0) 
        {
          pToken->nPredicate = pzPred-PredicateStr;
          break;
        }
      if (pToken->nPredicate==-1) { 
        PrintLastCaret(stderr, pParse);
        fprintf(stderr, "Unknown predicate '%s'\n", zArg);
        pParse->nErrors++;
      }
      break;
    case ForeachItem:
      if (!pParse->pDialogLoopToken) {
        PrintLastCaret(stderr, pParse);
        fprintf(stderr, "Only in dialog loop\n");
        pParse->nErrors++;
      }
    case ForeachDialog:
      if (pToken->nType == ForeachDialog) {
        pParse->pItemLoopToken = NULL;
        ppLoopToken = &pParse->pDialogLoopToken;
      } else
        ppLoopToken = &pParse->pItemLoopToken;
      if (*ppLoopToken) {
        PrintLastCaret(stderr, pParse);
        fprintf(stderr, "Nested loops not supported\n");
        pParse->nErrors++;
      }
      *ppLoopToken = pToken;
      break;
    case NextDialog: 
    case NextItem:
      ppLoopToken = ( pToken->nType == NextDialog )
                    ? &pParse->pDialogLoopToken
                    : &pParse->pItemLoopToken;
      if (!*ppLoopToken) {
        PrintLastCaret(stderr, pParse);
        fprintf(stderr, "Begin of loop not declared\n");
        pParse->nErrors++;
      }
      pToken->pLoop = (*ppLoopToken)->next;
      *ppLoopToken = NULL;
      break;
    case Variable:
      break;
    default:
      assert("Unexpected pToken->nType");
  }
  AddToken(pToken, pParse);
}

int meta_load_template(const char* zFilename, TemplateToken **ppTemplate)
{
  char cQuote = '%';
  char cLP = '[';
  char cRP = ']';
  FILE* pFile = fopen(zFilename, "r");
  char buffer[1024];
  char command[32];
  char arg[32];
  char *zEnd, *zCmd = NULL, *zArg = NULL;
  char c;
  size_t nChars;
  struct Parse sParse;
  struct Caret *pCaret = &sParse.sCaret;
  struct Caret *pLastCaret = &sParse.sLastCaret;
  if (!pFile) 
  {
    fprintf(stderr, "Error opening %s", zFilename);
    exit(errno);
  }
  memset(&sParse, 0, sizeof(sParse));
  sParse.zFilename = zFilename;
  while (fgets(buffer, sizeof(buffer), pFile))
  {
    sParse.sCaret.nLine++;
    sParse.sCaret.nPos = 1;
    if (strncmp(buffer, "metaquote", 9)==0)
    {
      for (pCaret->z = &buffer[9]; c=*(pCaret->z)==' '||c=='\t'; pCaret->z++);
      cQuote = *(pCaret->z)++;
      cLP = *(pCaret->z)++;
      cRP = *(pCaret->z)++;
      fprintf(stderr, "Using metachars %c%c%c\n", cQuote, cLP, cRP);
    }
    else if (strncmp(buffer, "begin", 5)==0)
    {  
      sParse.sCaret.nLine++;
      sParse.sCaret.nPos = 1;
      sParse.sLastCaret = sParse.sCaret;
      //fprintf(stderr, "Parsing template...\n");
      while (nChars = fread(buffer, sizeof(char), sizeof(buffer), pFile))
      {
        //printf("Parsing chunk: %d\n", nChars);
        zEnd = buffer+nChars;
        sParse.sLastCaret.z = buffer;
        sParse.sCaret.z = buffer;
        for(pCaret->z = buffer; pCaret->z < zEnd; pCaret->z++) 
        {
          if ( *(pCaret->z)=='\n' ) {
            pCaret->nLine++;
            pCaret->nPos = 1;
          } else if ( *(pCaret->z)!='\r' ) 
            pCaret->nPos++;
          switch (sParse.nState) {
             case stLiteral:
               if ( *(pCaret->z)==cQuote )
               {
                  AddLiteral(pLastCaret->z, pCaret->z-pLastCaret->z, &sParse);
                  SwitchState(stWaitLP, &sParse);
               } 
               break;
             case stWaitLP:
               if ( *(pCaret->z)!=cLP )
               {
                 PrintCaret(stderr, &sParse);
                 fprintf(stderr, "'%c' wanted\n", cQuote);
                 return ++sParse.nErrors;
               }
               SwitchState(stCommand, &sParse);
               zCmd = command;
               break;
             case stCommand:
               //for (; (p<pEnd-1) && (p[1]=='\t' || p[1]==' '); p++);
               if (zCmd >= command+sizeof(command))
               {
                 PrintLastCaret(stderr, &sParse);
                 fprintf(stderr, "[Fatal] command too big\n");
                 return ++sParse.nErrors;
               }
               if (c=*(pCaret->z)==cRP || c=='*') {
                 *zCmd = '\0';
                 AddCommand(command, &sParse);
                 //printf("Command %d:%d: %s\n", sParse.sLastCaret.nLine, sParse.sLastCaret.nPos, command);
                 SwitchState(c=='*' ? stComment : stLiteral, &sParse);
               } else if( c=*(pCaret->z)==' '||c=='\t' ) {
                 *zCmd = '\0';
                 //printf("Command with args %d:%d: %s\n", sParse.sLastCaret.nLine, sParse.sLastCaret.nPos, command);
                 for (; c=*(pCaret->z) && (c=='\t' || c==' '); pCaret->z++);
                 SwitchState(stCommandArgs, &sParse);
                 zArg = arg;
               } else {
                 *zCmd = *(pCaret->z); zCmd++;
               }
               break;
             case stCommandArgs:
               if (zArg >= arg+sizeof(arg))
               {
                 PrintLastCaret(stderr, &sParse);
                 fprintf(stderr, "[Fatal] Command argument too big\n");
                 return ++sParse.nErrors;
               }
               if (*(pCaret->z)==cRP) {
                 *zArg = '\0';
                 AddCommandArg(command, arg, &sParse);
                 SwitchState(stLiteral, &sParse);
               } else {
                 *zArg = *(pCaret->z); zArg++;
               }
               break;
             case stComment:
               if (*(pCaret->z)=='*') {
                 SwitchState(stCommand, &sParse);
                 zCmd = command;
               }
               break;
          }
        }
        switch(sParse.nState) {
          case stLiteral:
            AddLiteral(pLastCaret->z, zEnd-pLastCaret->z, &sParse);
            break;
        }
      }
      break;
    }
  }
  fclose(pFile);
  if (sParse.pItemLoopToken)
  {
    PrintCaret(stderr, &sParse);
    fprintf(stderr, "Unclosed item loop\n");
    sParse.nErrors++;
    
  }
  if (sParse.pDialogLoopToken)
  {
    PrintCaret(stderr, &sParse);
    fprintf(stderr, "Unclosed dialog loop\n");
    sParse.nErrors++;
  }
  fprintf(stderr, "%d tokens created.\n", sParse.pLastToken->nIndex+1);
  assert(ppTemplate);
  *ppTemplate = sParse.pFirstToken;
  return sParse.nErrors;
}

void meta_print_template(FILE* pFile, TemplateToken *pTemplate)
{
  TemplateToken *pToken;
  assert(pFile);
  assert(pTemplate);
  for (pToken = pTemplate; pToken; pToken = pToken->next)
  {
    switch (pToken->nType)
    {
      case Literal:
        fprintf(pFile, "%s", (char*)(pToken+1));
        break;
      case Newline:
        fprintf(pFile, "\n");
        break;
      case Common:
        fprintf(pFile, "%%[common]");
        break;
      case Alternative:
        fprintf(pFile, "%%[other]");
        break;
      case ForeachDialog:
        fprintf(pFile, "%%[foreach dialog]");
        break;
      case NextDialog:
        fprintf(pFile, "%%[next dialog]");
        break;
      case ForeachItem:
        fprintf(pFile, "%%[foreach item]");
        break;
      case NextItem:
        fprintf(pFile, "%%[next item]");
        break;
      case HasCondition:
        fprintf(pFile, "%%[has %s]", PredicateStr[pToken->nPredicate]);
        break;
      case HasNotCondition:
        fprintf(pFile, "%%[hasn't %s]", PredicateStr[pToken->nPredicate]);
        break;
      case Variable:
        //fprintf(pFile, "%%[%s]", (char*)(pToken+1));
        break;
      case InternalVariable:
        fprintf(pFile, "%%[%s]", VariableStr[pToken->nVariable]);
        break;
    }
  }
}
