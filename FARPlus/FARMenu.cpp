/* 
   FAR+Plus: Menu template
   (C) 1998-2002 Dmitry Jemerov <yole@yole.ru>
   (C) 2006 Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
*/

#include "FARMenu.h"
#include "FARPlus.h"

// -- FarMenu ----------------------------------------------------------------

template <typename TItems>
FarMenuT<TItems>::FarMenuT(const char *TitleText, unsigned int Flags,
                  const char *HelpTopic)
  : fTitle         (TitleText),
    fFlags         (Flags),
	fHelpTopic     (HelpTopic),
    fBottom        (NULL),
    fBreakKeys     (NULL),
    fBreakCode     (-1),
    fX             (-1),
    fY             (-1),
    fMaxHeight     (0),
    fItems         (NULL),
    fItemsNumber   (0),
    fOwnsBreakKeys (false)
{
}

template <typename TItems>
FarMenuT<TItems>::FarMenuT (int TitleLngIndex, unsigned int Flags,
                  const char *HelpTopic)
  : fTitle         (Far::GetMsg (TitleLngIndex)),
    fFlags         (Flags),
	fHelpTopic     (HelpTopic),
    fBottom        (NULL),
    fBreakKeys     (NULL),
    fBreakCode     (-1),
    fX             (-1),
    fY             (-1),
    fMaxHeight     (0),
    fItems         (NULL),
    fItemsNumber   (0),
    fOwnsBreakKeys (false)
{
}

template <typename TItems>
FarMenuT<TItems>::~FarMenuT()
{
    if (fItems) free (fItems);
    if (fOwnsBreakKeys && fBreakKeys) delete [] fBreakKeys;
}

int FarMenuT<FarMenuItem>::InternalAddItem (const char *Text, int Selected, int Checked, int Separator)
{
    fItems = (FarMenuItem *) realloc (fItems,
        ++fItemsNumber * sizeof (FarMenuItem));
    lstrcpyn (fItems [fItemsNumber-1].Text, Text, sizeof (fItems [fItemsNumber-1].Text)-1);
    fItems [fItemsNumber-1].Selected = Selected;
    fItems [fItemsNumber-1].Checked = Checked;
    fItems [fItemsNumber-1].Separator = Separator;
    return fItemsNumber-1;
}

int FarMenuT<FarMenuItemEx>::InternalAddItem (const char *Text, int Selected, int Checked, int Separator)
{
    fItems = (FarMenuItemEx *) realloc (fItems,
        ++fItemsNumber * sizeof (FarMenuItemEx));
    FarMenuItemEx *fItem = fItems+fItemsNumber-1;
    lstrcpyn (fItem->Text.Text, Text, sizeof (fItem->Text.Text)-1);
    fItem->Flags = (Selected?MIF_SELECTED:0) 
                 | (Checked?MIF_CHECKED:0) 
                 | (Separator?MIF_SEPARATOR:0);
    return fItemsNumber-1;
}

template <typename TItems>
int FarMenuT<TItems>::AddItem (const char *Text, bool Selected, int Checked)
{
    return InternalAddItem (Text, Selected, Checked, 0);
}

template <typename TItems>
int FarMenuT<TItems>::AddItem (int LngIndex, bool Selected, int Checked)
{
    return InternalAddItem (Far::GetMsg (LngIndex), Selected, Checked, 0);
}

template <typename TItems>
int FarMenuT<TItems>::AddItem (char Letter, const char *Text, bool Selected, int Checked)
{
  char buffer[128] = {'&', Letter, '.', ' '};
  lstrcpyn(&buffer[4], Text, sizeof(buffer)-4);
  return InternalAddItem (buffer, Selected, Checked, 0);
}

template <typename TItems>
int FarMenuT<TItems>::AddItem (char Letter, int LngIndex, bool Selected, int Checked)
{
  char buffer[128] = {'&', Letter, '.', ' '};
  lstrcpyn(&buffer[4], Far::GetMsg (LngIndex), sizeof(buffer)-4);
  return InternalAddItem (buffer, Selected, Checked, 0);
}

template <typename TItems>
int FarMenuT<TItems>::AddSeparator()
{
    return InternalAddItem ("", 0, 0, 1);
}

template <typename TItems>
void FarMenuT<TItems>::ClearItems()
{
    delete [] fItems;
    fItems = NULL;
    fItemsNumber = 0;
}

void FarMenuT<FarMenuItem>::SelectItem (int index)
{
    if (index >= 0 && index < fItemsNumber)
        for (int i=0; i < fItemsNumber; i++)
            fItems [i].Selected = (i == index);
}

void FarMenuT<FarMenuItemEx>::SelectItem (int index)
{
    if (index >= 0 && index < fItemsNumber)
    {
        for (int i = 0; i < fItemsNumber; i++)
        {
            fItems [i].Flags &= ~MIF_SELECTED;
        }
        (fItems + index)->Flags |= MIF_SELECTED;
    }
}

void FarMenuT<FarMenuItemEx>::DisableItem (int index)
{
  if (index >= 0 && index < fItemsNumber)
  {
    (fItems + index)->Flags |= MIF_DISABLE;
  }
}

template <typename TItems>
void FarMenuT<TItems>::SetBreakKeys (int aFirstKey, ...)
{
    int keyCount = 0;
    int curKey = aFirstKey;
    va_list va;
    va_start (va, aFirstKey);
    while (curKey != 0)
    {
        keyCount++;
        curKey = va_arg (va, int);
    }
    va_end (va);

    if (fOwnsBreakKeys && fBreakKeys)
        delete fBreakKeys;
    fOwnsBreakKeys = true;
    fBreakKeys = new int [keyCount+1];

    int *pKey = fBreakKeys;        
    curKey = aFirstKey;
    va_start (va, aFirstKey);
    while (curKey != 0)
    {
        *pKey++ = curKey;
        curKey = va_arg (va, int);
    }
    va_end (va);
    
    *pKey = curKey;  // put terminating NULL
}

int FarMenuT<FarMenuItem>::Show()
{
    return Far::m_Info.Menu (Far::m_Info.ModuleNumber,
        fX, fY, fMaxHeight, fFlags,
        fTitle, fBottom, fHelpTopic,
        fBreakKeys, &fBreakCode,
        fItems, fItemsNumber);
}

int FarMenuT<FarMenuItemEx>::Show()
{
    return Far::m_Info.Menu (Far::m_Info.ModuleNumber,
        fX, fY, fMaxHeight, fFlags | FMENU_USEEXT,
        fTitle, fBottom, fHelpTopic,
        fBreakKeys, &fBreakCode,
        (FarMenuItem*)fItems, fItemsNumber);
}

#ifdef _MSC_VER
// -- FarMenu[Ex] -------------------------------------------------------------

template class FarMenuT<FarMenuItem>;
template class FarMenuT<FarMenuItemEx>;
#endif //_MSC_VER
