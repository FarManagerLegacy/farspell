/* 
   FAR+Plus: Menu template
   (C) 1998-2002 Dmitry Jemerov <yole@yole.ru>
   (C) 2006 Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
   This file is heavily based on sources by Eugene Roshal
*/

#ifndef __FARMENU_H
#define __FARMENU_H
#if _MSC_VER >= 1000
#pragma once
#endif

#include "plugin.hpp"
#include "FARString.h"
#include "FARArray.h"
#include "FARPlus.h"

// -- FarMenu[Ex] -------------------------------------------------------------

template <typename TItems>
class FarMenuT: Far
{
private:
	FarMenuT (const FarMenuT &rhs);
	const FarMenuT &operator= (const FarMenuT &rhs);

protected:
    FarString fTitle;
    unsigned int fFlags;
    FarString fHelpTopic;
    FarString fBottom;
    int *fBreakKeys;
    int fBreakCode;
    int fX, fY;
	int fMaxHeight;
    TItems *fItems;
    int fItemsNumber;
    bool fOwnsBreakKeys;
    int InternalAddItem (const char *Text, int Selected, int Checked, int Separator);

public:
    FarMenuT (const char *TitleText, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    FarMenuT (int TitleLngIndex, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    ~FarMenuT();

    void SetLocation (int X, int Y)
		{ fX = X; fY = Y; }
    void SetMaxHeight (int MaxHeight)
		{ fMaxHeight = MaxHeight; }
    void SetBottomLine (const FarString &Text)
		{ fBottom = Text; }
    void SetBottomLine (int LngIndex)
		{ fBottom = Far::GetMsg (LngIndex); }
    void SetBreakKeys (int *BreakKeys)
		{ fOwnsBreakKeys = false; fBreakKeys = BreakKeys; }
    void SetBreakKeys (int aFirstKey, ...);

    // returns index of new item
    int AddItem (const char *Text, bool Selected=false, int Checked=0);
    int AddItem (int LngIndex, bool Selected=false, int Checked=0);
    int AddItem (char Letter, const char *Text, bool Selected=false, int Checked=0);
    int AddItem (char Letter, int LngIndex, bool Selected=false, int Checked=0);
    int AddSeparator();
    void DisableItem (int index);

    void ClearItems();
    void SelectItem (int index);
    // returns index of selected item
    int Show();
    int GetBreakCode() const
		{ return fBreakCode; }
    int Count() const { return fItemsNumber; }
    const char* GetItemText(int index) const;
};

typedef FarMenuT<FarMenuItem> FarMenu;
typedef FarMenuT<FarMenuItemEx> FarMenuEx;

#ifdef __GNUC_
// -- FarMenu[Ex] -------------------------------------------------------------

template class FarMenuT<FarMenuItem>;
template class FarMenuT<FarMenuItemEx>;
#endif
/*class FarMenuEx
: public FarMenuT<FarMenuItemEx>
{
  public:
    void DisableItem (int index);
};*/
#endif
