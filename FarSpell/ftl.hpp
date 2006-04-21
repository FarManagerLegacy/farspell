/*
    Public primitives from FARPlus.
    Copyright (c) Sergey Shishmintzev, Kiev 2005-2006

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
      FARPlus http://farplus.sf.net/
      Dmitry Jemerov <yole@yole.ru>
      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
*/

namespace ftl
{
// Base class for combo boxes and list boxes items
class BaseListItems
{
protected:
	FarDialogItem *fItem;
	FarDataArray<FarListItem> fListItems;
	FarList fFarList;
	BaseListItems(FarDialogItem *item): fItem(item) 
	{
		if (fItem)
			fItem->ListItems = &fFarList;
	};
public:
        void AttachDialogItem(FarDialogItem *item)
        {
        	far_assert(item);
        	item->ListItems = &fFarList;
        }
	void AddItem (const char *Text, int Flags = 0)
	{
		FarListItem newItem;
		memset (&newItem, 0, sizeof (newItem));
		strncpy (newItem.Text, Text, sizeof (newItem.Text)-1);
		newItem.Flags = Flags;
		fListItems.Add (newItem);
	}
	int GetListPos() const
	{
		far_assert(fItem);
		return fItem->ListPos;
	}
	void BeforeShow()
	{
		fFarList.ItemsNumber = fListItems.Count();
		fFarList.Items = fListItems.GetItems();
	}
	int Count() const
	{
	  return fListItems.Count();
	}
};

class ListboxItems: public BaseListItems
{
public:
	ListboxItems(FarDialogItem *item)
		: BaseListItems(item) {};
};

class ComboboxItems: public BaseListItems
{
public:
	ComboboxItems(FarDialogItem *item)
		: BaseListItems(item) {};
	void SetText (const char *pText)
	{
		far_assert(fItem);
		strncpy (fItem->Data, pText, sizeof (fItem->Data)-1);
	}
	void SetText (FarString &String)
	{
		far_assert(fItem);
		strncpy (fItem->Data, String.c_str(), sizeof (fItem->Data)-1);
	}
	void GetText (char *Text, int MaxLength) const
	{
		lstrcpyn (Text, fItem->Data, MaxLength-1);
	}
	FarString GetText() const
	{
		far_assert(fItem);
		return fItem->Data;
	}
	void GetText (int Index, char *Text, int MaxLength) const
	{
		far_assert(Index>=0);
		far_assert(Index<fListItems.Count());
		lstrcpyn (Text, fListItems[Index].Text, MaxLength-1);
	}
	FarString GetText(int Index) const
	{
		far_assert(Index>=0);
		far_assert(Index<fListItems.Count());
		return fListItems[Index].Text;
	}
};

int GetRadioStatus(const struct FarDialogItem* pItems, int nItems, int nItem)
{
  int nSelected = 0;
  for (pItems+=nItem; pItems->Type == DI_RADIOBUTTON; pItems++, nSelected++)
  {
    if (pItems->Selected) return nSelected; 
  }
  return -1;
}

};
