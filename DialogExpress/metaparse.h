#pragma once

enum Predicate { 
  ItemHasMsgidP, 
  ItemHasIdP, 
  ItemHasHistoryP,
  ItemHasTextP,
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
  vItemMsgid,
  vItemText,
  vItemCData,
  vItemHistory,
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
