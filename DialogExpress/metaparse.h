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
#pragma once

enum Predicate { 
  ItemHasX1,
  ItemHasY1,
  ItemHasX2,
  ItemHasY2,
  ItemHasFocused,
  ItemHasSelected,
  ItemHasFlags,
  ItemHasDefaultButton,
  ItemHasMsgidP, 
  ItemHasIdP, 
  ItemHasHistoryP,
  ItemHasTextP,
  ItemHasColorIndexP,
  ItemHasFgColorIndexP,
  ItemHasBgColorIndexP,
};
enum Variable { 
  vDialogId, 
  vDialogProgId, 
  vDialogHelp, 
  vDialogWidth,  
  vDialogHeight,  
  vItemsCount,  
  vItemIndex,  
  vItemId,  
  vItemIdLiteral,  
  vItemProgId,  
  vItemType,  
  vItemX1,  
  vItemY1,  
  vItemX2,  
  vItemY2,  
  vItemFocused,
  vItemSelected,
  vItemFlags,
  vItemDefaultButton,
  vItemMsgid,
  vItemMsgidLiteral,
  vItemText,
  vItemCData,
  vItemHistory,
  vItemColorIndex,
  vItemFgColorIndex,
  vItemBgColorIndex,
};

typedef struct _TemplateToken
{
  enum { 
    Literal, 
    Newline,
    ForeachDialog, 
    NextDialog,
    ForeachItem,
    NextItem,
    HasNotCondition,
    HasCondition,
    Alternative,
    Common,
    Variable,
    InternalVariable,
  } nType;
  int nIndex;
  unsigned nLine;
  unsigned nPos;
  union
  {
    size_t nChars;
    int nPredicate;
    int nVariable;
    struct _TemplateToken *pLoop;
  };
  struct _TemplateToken *next;
} TemplateToken;

int meta_load_template(const char* zFilename, TemplateToken **ppTemplate);
void meta_print_template(FILE* pFile, TemplateToken *pTemplate);
