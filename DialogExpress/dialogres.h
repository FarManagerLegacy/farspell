/*
    Dialog resource parser header
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
#include <plugin.hpp>

enum dialogres_error
{
  dialogres_Ok                   = 0,
  dialogres_Error                = 1, 
  dialogres_OSError              = 2,
  dialogres_NoMemory             = 3,
  dialogres_ErrToken             = 0x1000, // see sErrToken
  dialogres_IncompleteStatement  = 4, // syntax
  dialogres_SyntaxError          = dialogres_ErrToken|5, // syntax 
  dialogres_UnknownIdentifier    = dialogres_ErrToken|6, // syntax 
  dialogres_IllegalToken         = dialogres_ErrToken|7, // syntax 
  dialogres_DivideByZero         = dialogres_ErrToken|8, // syntax 
  dialogres_StackOverflow        = 9, // syntax
  dialogres_NotFound             = 10,
  dialogres_TooLarge             = 11,
};

#define MAX_DIALOG_RESOURCE_SIZE 0x20000

typedef struct dialogres dialogres;
typedef struct dialogtemplate dialogtemplate;
typedef struct dialogitem dialogitem;

#ifdef _DEBUG
#define dialogres_free(s) _dialogres_free((void**)&s, __FILE__, __LINE__)
void _dialogres_free(void **ppChunk, const char* zFile, unsigned nLine);
#else
void dialogres_free(void*);
#endif


enum dialogres_error dialogres_open(const char* zFilename, dialogres** ppDr);
void dialogres_close(dialogres* pDr);


enum dialogres_error dialogres_last_error(dialogres* pDr);
int dialogres_error_pos(dialogres* pDr);
int dialogres_error_line(dialogres* pDr);


int dialogres_dialogs_count(dialogres* pDr);
enum dialogres_error dialogres_get_dialog(dialogres* pDr, char *zName, dialogtemplate** ppDt);
enum dialogres_error dialogres_last_dialog(dialogres* pDr, dialogtemplate** ppDt);
enum dialogres_error dialogres_prev_dialog(dialogres* pDr, dialogtemplate** ppDt);


enum dialogres_error dialogtemplate_create_items(dialogtemplate* pDt, int untemplate, struct FarDialogItem **);
int dialogtemplate_items_count(dialogtemplate* pDt);
int dialogtemplate_index_by_id(dialogtemplate* pDt, int nId);
const char* dialogtemplate_name(dialogtemplate* pDt);
const char* dialogtemplate_help(dialogtemplate* pDt);
enum dialogres_error dialogtemplate_get_item(dialogtemplate* pDt, int nId, int nIndex, dialogitem** ppDi);
enum dialogres_error dialogtemplate_first_item(dialogtemplate* pDt, dialogitem** ppDi);
enum dialogres_error dialogtemplate_next_item(dialogtemplate* pDt, dialogitem** ppDi);
enum dialogres_error dialogtemplate_last_item(dialogtemplate* pDt, dialogitem** ppDi);
enum dialogres_error dialogtemplate_prev_item(dialogtemplate* pDt, dialogitem** ppDi);

enum
{
  DI_DATASOURCE_NULL,
  DI_DATASOURCE_TEXT,
  DI_DATASOURCE_MSGID,
  DI_DATASOURCE_REGISTRY,
};

int dialogitem_datasource_type(dialogitem *pDi);
int dialogitem_id(dialogitem *pDi);
const char* dialogitem_sid(dialogitem *pDi);
int dialogitem_index(dialogitem *pDi);
const char *dialogitem_datasource_text(dialogitem *pDi);
int dialogitem_datasource_msgid(dialogitem *pDi);
const char *dialogitem_history(dialogitem *pDi);

