// Примитивы из FarPlus.

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
