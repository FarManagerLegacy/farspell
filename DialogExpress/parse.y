/*
    Dialog resource parser
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

// All token codes are small integers with #defines that begin with "TK_"
%token_prefix TK_

%token_type {Token} // The type of the data attached to each token is Token.  
//%default_type {Token} This is also the default type for non-terminals.

// The generated parser function takes a 4th argument as follows:
%extra_argument {Parse *pParse}

// This code runs whenever there is a syntax error
//
%syntax_error {
  if( pParse->rc == dialogres_Ok ){
    if( TOKEN.z && TOKEN.z[0] ){
      pParse->rc = dialogres_SyntaxError;
      pParse->sErrToken = TOKEN;
    }else{
      pParse->rc = dialogres_IncompleteStatement;
    }
  }
}
%stack_overflow {
  if( pParse->rc == dialogres_Ok )
    pParse->rc = dialogres_StackOverflow;
}

// The name of the generated procedure that implements the parser
// is as follows:
%name dialogresParser

// The following text is included near the beginning of the C source
// code file that implements the parser.
//
%include {
#include <assert.h>
#include <string.h>
#include <plugin.hpp>
#include "../dialogresInt.h"


static Token null_token =     { "",  1, 0, 0, 0};
static Token contents_token = { "Contents", 1, 8, 0, 0};
static Token version_token =  { "Dex.2006",  1, 8, 0, 0};

/*char* strFromToken(Token sTok)
{
  char* str = dialogres_malloc(sTok.n+1);
  strncpy(str, sTok.z, sTok.n);
  str[sTok.n] = '\0';
  return str;
}*/
static __inline Token unquote(Token s)
{
  Token ns = s;
  ns.z++;
  ns.n--;
  ns.n--;
  return ns;
}
}

%nonassoc END_OF_FILE ILLEGAL SPACE UNCLOSED_STRING COMMENT 
VARIABLE FLOAT LT LE GT GE NE SEMI.

// Define operator precedence early so that this is the first occurance
// of the operator tokens in the grammer.  Keeping the operators together
// causes them to be assigned integer values that are close together,
// which keeps parser tables smaller.
//
%left OR.
%left AND.
%right NOT.
%left BITAND BITOR LSHIFT RSHIFT.
%left PLUS MINUS.
%left STAR SLASH REM.
%right UMINUS UPLUS BITNOT.

input ::= cmdlist.
cmdlist ::= .
cmdlist ::= cmdlist cmd.


cmd ::= ENUM LB idents RB.
idents ::= . { pParse->nEnumIndex = 0; } 
idents ::= idents ident COMMA.
idents ::= idents ident. 
ident ::= id(I). { AddIntBind(pParse, I, pParse->nEnumIndex++); }
ident ::= id(I) EQ cexpr(V). { AddIntBind(pParse, I, pParse->nEnumIndex=V); pParse->nEnumIndex++; }


%type id {Token}
id(A) ::= ID(X).         {A = X;}

%fallback ID NAME ENUM T F 
HELP 
NULL TXT LNG REG HISTORY
HKCU HKLM HKCR HKCC VER DEF.

%type cexpr {int}
cexpr(R) ::= INTEGER(N).               { R = toint(N); }
cexpr(R) ::= LP cexpr(A) RP.           { R = A; }
cexpr(R) ::= MINUS cexpr(A). [UMINUS]  { R = -A; }
cexpr(R) ::= PLUS cexpr(A). [UPLUS]    { R = +A; }
cexpr(R) ::= cexpr(A) PLUS cexpr(B).   { R = A+B; }
cexpr(R) ::= cexpr(A) MINUS cexpr(B).  { R = A-B; }
cexpr(R) ::= cexpr(A) REM cexpr(B).    { 
  if (!B) { 
    pParse->rc = dialogres_DivideByZero; 
    pParse->sErrToken = pParse->sLastToken;
  }
  R = A%B; 
}
cexpr(R) ::= cexpr(A) SLASH cexpr(B).  { 
  if (!B) { 
    pParse->rc = dialogres_DivideByZero; 
    pParse->sErrToken = pParse->sLastToken;
  }
  else R = A/B; 
}
cexpr(R) ::= cexpr(A) STAR cexpr(B).   { R = A*B; }
cexpr(R) ::= NOT cexpr(A).             { R = !A; }
cexpr(R) ::= cexpr(A) OR cexpr(B).     { R = A||B; }
cexpr(R) ::= cexpr(A) AND cexpr(B).    { R = A&&B; }
cexpr(R) ::= BITNOT cexpr(A).          { R = ~A; }
cexpr(R) ::= cexpr(A) BITOR cexpr(B).  { R = A|B; }
cexpr(R) ::= cexpr(A) BITAND cexpr(B). { R = A&B; }
cexpr(R) ::= cexpr(A) LSHIFT cexpr(B). { R = A<<B; }
cexpr(R) ::= cexpr(A) RSHIFT cexpr(B). { R = A>>B; }
cexpr(R) ::= DIF_COLORMASK.            { R = DIF_COLORMASK; }
cexpr(R) ::= DIF_SETCOLOR.             { R = DIF_SETCOLOR; }
cexpr(R) ::= DIF_BOXCOLOR.             { R = DIF_BOXCOLOR; }
cexpr(R) ::= DIF_GROUP.                { R = DIF_GROUP; }
cexpr(R) ::= DIF_LEFTTEXT.             { R = DIF_LEFTTEXT; }
cexpr(R) ::= DIF_MOVESELECT.           { R = DIF_MOVESELECT; }
cexpr(R) ::= DIF_SHOWAMPERSAND.        { R = DIF_SHOWAMPERSAND; }
cexpr(R) ::= DIF_CENTERGROUP.          { R = DIF_CENTERGROUP; }
cexpr(R) ::= DIF_NOBRACKETS.           { R = DIF_NOBRACKETS; }
cexpr(R) ::= DIF_MANUALADDHISTORY.     { R = DIF_MANUALADDHISTORY; }
cexpr(R) ::= DIF_SEPARATOR.            { R = DIF_SEPARATOR; }
cexpr(R) ::= DIF_VAREDIT.              { R = DIF_VAREDIT; }
cexpr(R) ::= DIF_SEPARATOR2.           { R = DIF_SEPARATOR2; }
cexpr(R) ::= DIF_EDITOR .              { R = DIF_EDITOR; }
cexpr(R) ::= DIF_LISTNOAMPERSAND.      { R = DIF_LISTNOAMPERSAND; }
cexpr(R) ::= DIF_LISTNOBOX.            { R = DIF_LISTNOBOX; }
cexpr(R) ::= DIF_HISTORY.              { R = DIF_HISTORY; }
cexpr(R) ::= DIF_BTNNOCLOSE.           { R = DIF_BTNNOCLOSE; }
cexpr(R) ::= DIF_CENTERTEXT.           { R = DIF_CENTERTEXT; }
cexpr(R) ::= DIF_EDITEXPAND.           { R = DIF_EDITEXPAND; }
cexpr(R) ::= DIF_DROPDOWNLIST.         { R = DIF_DROPDOWNLIST; }
cexpr(R) ::= DIF_USELASTHISTORY.       { R = DIF_USELASTHISTORY; }
cexpr(R) ::= DIF_MASKEDIT.             { R = DIF_MASKEDIT; }
cexpr(R) ::= DIF_SELECTONENTRY.        { R = DIF_SELECTONENTRY; }
cexpr(R) ::= DIF_3STATE.               { R = DIF_3STATE; }
cexpr(R) ::= DIF_LISTWRAPMODE.         { R = DIF_LISTWRAPMODE; }
cexpr(R) ::= DIF_LISTAUTOHIGHLIGHT.    { R = DIF_LISTAUTOHIGHLIGHT; }
cexpr(R) ::= DIF_LISTNOCLOSE.          { R = DIF_LISTNOCLOSE; }
cexpr(R) ::= DIF_HIDDEN.               { R = DIF_HIDDEN; }
cexpr(R) ::= DIF_READONLY.             { R = DIF_READONLY; }
cexpr(R) ::= DIF_NOFOCUS.              { R = DIF_NOFOCUS; }
cexpr(R) ::= DIF_DISABLE.              { R = DIF_DISABLE; }
cexpr(R) ::= WHITE.                    { R = DIF_SETCOLOR|0x0F; }
cexpr(R) ::= YELLOW.                   { R = DIF_SETCOLOR|0x0E; }
cexpr(R) ::= LIGHTMAGENTA.             { R = DIF_SETCOLOR|0x0D; }
cexpr(R) ::= LIGHTRED.                 { R = DIF_SETCOLOR|0x0C; }
cexpr(R) ::= LIGHTCYAN.                { R = DIF_SETCOLOR|0x0B; }
cexpr(R) ::= LIGHTGREEN.               { R = DIF_SETCOLOR|0x0A; }
cexpr(R) ::= LIGHTBLUE.                { R = DIF_SETCOLOR|0x09; }
cexpr(R) ::= DARKGRAY.                 { R = DIF_SETCOLOR|0x08; }
cexpr(R) ::= LIGHTGRAY.                { R = DIF_SETCOLOR|0x07; }
cexpr(R) ::= BROWN.                    { R = DIF_SETCOLOR|0x06; }
cexpr(R) ::= MAGENTA.                  { R = DIF_SETCOLOR|0x05; }
cexpr(R) ::= RED.                      { R = DIF_SETCOLOR|0x04; }
cexpr(R) ::= CYAN.                     { R = DIF_SETCOLOR|0x03; }
cexpr(R) ::= GREEN.                    { R = DIF_SETCOLOR|0x02; }
cexpr(R) ::= BLUE.                     { R = DIF_SETCOLOR|0x01; }
cexpr(R) ::= BLACK.                    { R = DIF_SETCOLOR|0x00; }
cexpr(R) ::= BK_WHITE.                 { R = DIF_SETCOLOR|0xF0; }
cexpr(R) ::= BK_YELLOW.                { R = DIF_SETCOLOR|0xE0; }
cexpr(R) ::= BK_LIGHTMAGENTA.          { R = DIF_SETCOLOR|0xD0; }
cexpr(R) ::= BK_LIGHTRED.              { R = DIF_SETCOLOR|0xC0; }
cexpr(R) ::= BK_LIGHTCYAN.             { R = DIF_SETCOLOR|0xB0; }
cexpr(R) ::= BK_LIGHTGREEN.            { R = DIF_SETCOLOR|0xA0; }
cexpr(R) ::= BK_LIGHTBLUE.             { R = DIF_SETCOLOR|0x90; }
cexpr(R) ::= BK_DARKGRAY.              { R = DIF_SETCOLOR|0x80; }
cexpr(R) ::= BK_LIGHTGRAY.             { R = DIF_SETCOLOR|0x70; }
cexpr(R) ::= BK_BROWN.                 { R = DIF_SETCOLOR|0x60; }
cexpr(R) ::= BK_MAGENTA.               { R = DIF_SETCOLOR|0x50; }
cexpr(R) ::= BK_RED.                   { R = DIF_SETCOLOR|0x40; }
cexpr(R) ::= BK_CYAN.                  { R = DIF_SETCOLOR|0x30; }
cexpr(R) ::= BK_GREEN.                 { R = DIF_SETCOLOR|0x20; }
cexpr(R) ::= BK_BLUE.                  { R = DIF_SETCOLOR|0x10; }
cexpr(R) ::= BK_BLACK.                 { R = DIF_SETCOLOR|0x00; }
cexpr(R) ::= id(I).                    { R = LookupIntBind(pParse, I); }
//cexpr(R) ::= VARIABLE(V).              { R = LookupIntBind(pParse, V); }
                                      

cmd ::= NAME STRING(N) help(H) items(I). { 
  struct dialogitem *item;
  struct dialogtemplate *pDt = ALLOC_STRUCT(dialogtemplate);
  if (!pDt) 
  {
    pParse->rc = dialogres_NoMemory;
    return;
  }
  memset(pDt, 0, sizeof(struct dialogtemplate));
  pDt->sName = unquote(N);
  pDt->sHelp = H;
  pDt->pItems = pParse->pFistDialogItem;
  for (item = pDt->pItems; item; item=item->next)  
    pDt->nItems++;
  assert(!pDt->pItems || pDt->nItems == I->nIndex+1);
  pDt->prev = pParse->pDr->pDialogs;
  pParse->pDr->pDialogs = pDt;
  pParse->pDr->nDialogs++;
  pParse->nPoolBytes += N.n-1 + sizeof(struct dialogtemplate)
                     + ( sizeof(struct dialogitem)
                       + sizeof(struct DataSource)
                       + SIZEOF_FAR_DIALOG_ITEM_TEMPLATE 
                       ) * (pDt->nItems);
  pParse->pFistDialogItem = NULL;
}

help(H) ::= .               { H = contents_token; }
help(H) ::= HELP STRING(X). { H = unquote(X); pParse->nPoolBytes += H.n+1; }

%type items {struct dialogitem *}
%destructor items { 
  dialogitemFree(pParse->pFistDialogItem, 1); 
  pParse->pFistDialogItem = NULL;
}
items(L) ::= .                    { L = NULL; }
items(L) ::= items(IS) item(I).   { 
  dialogitem *prev = IS;
  L = I; 
  if (prev) {
    assert(pParse->pFistDialogItem);
    prev->next = L;
    L->nIndex = prev->nIndex + 1;
  } else { 
    pParse->pFistDialogItem = L;
  }
}

%type item {struct dialogitem *}
%destructor item { dialogitemFree($$, 1); }

item(I) ::= TEXT         item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_TEXT,        ID, D, S); }
item(I) ::= VTEXT        item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_VTEXT,       ID, D, S); }
item(I) ::= SINGLEBOX    item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_SINGLEBOX,   ID, D, S); }
item(I) ::= DOUBLEBOX    item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_DOUBLEBOX,   ID, D, S); }

item(I) ::= EDIT         item_id(ID) item_data(D) data_source(S) history(H). 
            { I = dialogitemAlloc(pParse, DI_EDIT,        ID, D, S); 
              I->sHistory = H; 
              if (H.n) I->pFarDialogItem->Flags |= DIF_HISTORY; }

item(I) ::= FIXEDIT      item_id(ID) item_data(D) data_source(S) history(H). 
            { I = dialogitemAlloc(pParse, DI_FIXEDIT,     ID, D, S); 
              I->sHistory = H; 
              if (H.n) I->pFarDialogItem->Flags |= DIF_HISTORY; }

item(I) ::= PSWEDIT      item_id(ID) item_data(D) data_source(S).  
            { I = dialogitemAlloc(pParse, DI_PSWEDIT,     ID, D, S); }
item(I) ::= BUTTON       item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_BUTTON,      ID, D, S); }
item(I) ::= CHECKBOX     item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_CHECKBOX,    ID, D, S); }
item(I) ::= RADIOBUTTON  item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_RADIOBUTTON, ID, D, S); }
item(I) ::= COMBOBOX     item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_COMBOBOX,    ID, D, S); }
item(I) ::= LISTBOX      item_id(ID) item_data(D) data_source(S).  
            { I = dialogitemAlloc(pParse, DI_LISTBOX,     ID, D, S); }
item(I) ::= USERCONTROL  item_id(ID) item_data(D) data_source(S). 
            { I = dialogitemAlloc(pParse, DI_USERCONTROL, ID, D, S); }

//%type snum {int}
//snum(N) ::= INTEGER(I).       { N = toint(I); }
//snum(N) ::= MINUS INTEGER(I). { N = -toint(I); }

%type item_data {struct FarDialogItem*}
%destructor item_data { dialogres_free($$); }
item_data(ITEM) ::= INTEGER(TX1) INTEGER(TY1) INTEGER(TX2) INTEGER(TY2) 
bool(F) selected(S) cexpr(FLAGS) bool(DEF).
{
  ITEM = (struct FarDialogItem*)dialogres_malloc(SIZEOF_FAR_DIALOG_ITEM_TEMPLATE);
  assert(ITEM);
  memset(ITEM, 0, SIZEOF_FAR_DIALOG_ITEM_TEMPLATE);
  ITEM->X1 = toint(TX1); ITEM->X2 = toint(TX2);
  ITEM->Y1 = toint(TY1); ITEM->Y2 = toint(TY2);
  ITEM->Selected = S;
  ITEM->Focus = F;
  ITEM->Focus = F;
  ITEM->Flags = FLAGS;
  ITEM->DefaultButton = DEF;
}

%type item_id{Token}
item_id(ID) ::= INTEGER(N).          { ID = N; }
item_id(ID) ::= MINUS(M) INTEGER(N). { ID = M; ID.n+=N.z-ID.z+N.n; }
item_id(ID) ::= id(I).            { ID = I; }

%type bool{int}
//focus(F) ::= 0.  { F = 0; }
//focus(F) ::= 1.  { F = 1; }
bool(F) ::= INTEGER(N). { F = toint(N); }
bool(F) ::= F.          { F = 0; } 
bool(F) ::= T.          { F = -1; }

%type selected{int}
selected(S) ::= bool(F).  { S = F; }
//selected(S) ::= ID(I).    { S = LookupIntBind(pParse, I); }

%type data_source{struct DataSource*}
%destructor data_source { dialogitemDataSourceFree($$); } 
data_source(D) ::= NULL.                    { D = dialogitemDataSourceAlloc(DI_DATASOURCE_NULL); }
data_source(D) ::= label_ver.               { D = dialogitemDataSourceAlloc(DI_DATASOURCE_TEXT); D->txt_data = version_token;  }
data_source(D) ::= label_txt_def STRING(S). { D = dialogitemDataSourceAlloc(DI_DATASOURCE_TEXT); D->txt_data = unquote(S); pParse->nPoolBytes += S.n-1; }
data_source(D) ::= label_lng ID(I). { 
  D = dialogitemDataSourceAlloc(DI_DATASOURCE_MSGID); 
  D->msg_data.nMsgId = LookupIntBind(pParse, I); 
  D->msg_data.sMsgId = I; 
  pParse->nPoolBytes += I.n+1;
}
data_source(D) ::= label_reg reg_root_def(R) STRING(S) reg_default(DEF). {
  D = dialogitemDataSourceAlloc(DI_DATASOURCE_REGISTRY); 
  D->reg_data.root = R; 
  D->reg_data.key = unquote(S); 
  D->reg_data.def = DEF;
  pParse->nPoolBytes += S.n-1;
}


%type reg_root_def{HKEY}
reg_root_def(K) ::= .           { K = HKEY_CURRENT_USER; } // default
reg_root_def(K) ::= label_hkcu. { K = HKEY_CURRENT_USER; }
reg_root_def(K) ::= label_hklm. { K = HKEY_LOCAL_MACHINE; }
reg_root_def(K) ::= label_hkcr. { K = HKEY_CLASSES_ROOT; }
reg_root_def(K) ::= label_hkcc. { K = HKEY_CURRENT_CONFIG; }

%type reg_default{struct RegDefault}
reg_default(D) ::= label_def STRING(S). { D.type = bytes; D.str = unquote(S); pParse->nPoolBytes += S.n-1; }
reg_default(D) ::= label_def cexpr(I). { D.type = dword; D.dw = I; }

%type history{Token}
history(H) ::= .                        { H = null_token; }
history(H) ::= label_history STRING(S). { H = unquote(S); pParse->nPoolBytes += H.n+1; }
history(H) ::= label_history ID(S).     { H = S;          pParse->nPoolBytes += H.n+1; }

label_txt_def ::= .            // label_txt_opt is default
label_txt_def ::= REM TXT REM. // TODO: strong check %XXX% tokens, and produce warnings
label_lng ::= REM LNG REM.
label_reg ::= REM REG REM.
label_hklm ::= REM HKLM REM.
label_hkcu ::= REM HKCU REM.
label_hkcr ::= REM HKCR REM.
label_hkcc ::= REM HKCC REM.
label_def ::= .
label_def ::= REM DEF REM.
label_ver ::= REM VER REM.
label_history ::= HISTORY EQ. 


