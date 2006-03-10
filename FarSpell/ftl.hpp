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
		fItem->ListItems = &fFarList;
	};
public:
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
		return fItem->ListPos;
	}
	void BeforeShow()
	{
		fFarList.ItemsNumber = fListItems.Count();
		fFarList.Items = fListItems.GetItems();
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
	void SetText (char *pText)
	{
		strncpy (fItem->Data, pText, sizeof (fItem->Data)-1);
	}
	void SetText (FarString &String)
	{
		strncpy (fItem->Data, String.c_str(), sizeof (fItem->Data)-1);
	}
	void GetText (char *Text, int MaxLength)
	{
		lstrcpyn (Text, fItem->Data, MaxLength-1);
	}
	FarString GetText()
	{
		return fItem->Data;
	}
};

};
