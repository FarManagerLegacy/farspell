/* $Header: /cvsroot/farplus/FARPlus/FARArray.h,v 1.3 2002/09/05 06:33:05 yole Exp $
   Lightweight typesafe dynamic array class for use in FAR plugins
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARLIST_H
#define __FARLIST_H

#include <stdlib.h>
#include <string.h>
#include "FARMemory.h"

#if _MSC_VER >= 1000
#pragma once
#endif

class BaseFarArray
{
private:
	BaseFarArray (const BaseFarArray &rhs);              // not implemented (for now)
    BaseFarArray &operator= (const BaseFarArray &rhs);   // not implemented (for now)

protected:
	char *fItems;
	int fCount;
	int fAllocCount;
	int fResizeDelta;
	int fItemSize;

	typedef int (__cdecl *BaseCmpFunc) (const void *, const void *);

	BaseFarArray (int itemSize, int resizeDelta)
		: fItems (NULL), fCount (0), fAllocCount (0),
		  fResizeDelta (resizeDelta), fItemSize (itemSize) {};

	void BaseAdd (const void *item);

	void InternalClear()
	{
		fCount = 0;
		fAllocCount = 0;
		if (fItems)
		{
			free (fItems);
			fItems = NULL;
		}
	}

	int BaseIndexOf (const void *item) const
	{
		for (int i=0; i<fCount; i++)
		{
			if (memcmp (fItems + i * fItemSize, item, fItemSize) == 0) 
				return i;
		}
		return -1;
	}

	void InternalSort (BaseCmpFunc cmpFunc);

public:
	int Count() const
	{
		return fCount;
	}
};

template <class T>
class FarDataArray: public BaseFarArray
{
public:
	FarDataArray (int resizeDelta = 16)
		: BaseFarArray (sizeof (T), resizeDelta) {};
	
	~FarDataArray()
	{
		Clear();
	}

	void Clear()
	{
		InternalClear();
	}

	void Add (T &item)
	{
		BaseAdd (&item);
	}

	int IndexOf (T &item) const
	{
		return BaseIndexOf (&item);
	}

	T &At (int index)
	{
		return reinterpret_cast<T *> (fItems) [index];
	}

	T &operator[] (int index)
	{
		return At (index);
	}

	const T &operator[] (int index) const
	{
		return reinterpret_cast<const T *> (fItems) [index];
	}

	T *GetItems() const
	{
		return reinterpret_cast<T *> (fItems);
	}
};

template <class T>
class FarArray: public FarDataArray<T *>
{
private:
	typedef int (__cdecl *CmpFunc) (const T **, const T **);
	typedef FarDataArray<T *> Parent;
protected:
	bool fOwnsItems;

public:
	FarArray (int resizeDelta = 16)
		: FarDataArray<T *> (resizeDelta), fOwnsItems (true) {};
	~FarArray()
	{
		Clear();
	}

	void Clear()
	{
		if (fOwnsItems)
			for (int i = Parent::fCount-1; i >= 0; i--)
				delete Parent::At(i);
		Parent::InternalClear();
	}

	void Add (const T *item)
	{
		T *addItem = const_cast<T *> (item);
		Parent::Add (addItem);
	}

	bool OwnsItems() const
	{
		return fOwnsItems;
	}

	void SetOwnsItems (bool ownsItems)
	{
		fOwnsItems = ownsItems;
	}

	void Sort (CmpFunc cmpFunc)
	{
		Parent::InternalSort (reinterpret_cast<BaseFarArray::BaseCmpFunc>(cmpFunc));
	}
};

class FarStringArray: public FarArray<char>
{
private:
	char *Copy (const char *item)
	{
		int len = (item == NULL) ? 0 : strlen (item);
		char *itemCopy = new char [len+1];
		if (len > 0)
			strcpy (itemCopy, item);
		else
			*itemCopy = '\0';
		return itemCopy;
	}

public:
	FarStringArray (int resizeDelta = 16)
		: FarArray<char> (resizeDelta) {};
	~FarStringArray()
	{
		Clear();
	}

	void Add (const char *item)
	{
		if (fOwnsItems)
			FarArray<char>::Add (Copy (item));
		else
			FarArray<char>::Add (const_cast<char *> (item));
	}
};

class FarIntArray: public FarDataArray<int>
{
public:
	FarIntArray (int resizeDelta = 16)
		: FarDataArray<int> (16)
	{
	};

	void Add (int i)
	{
		FarDataArray<int>::Add (i);
	}
};

#endif
