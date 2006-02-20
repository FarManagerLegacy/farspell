/* $Header: $
   Lightweight typesafe dynamic array class for use in FAR plugins
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARLIST_H
#define __FARLIST_H

#include <stdlib.h>
#include "FARMemory.h"

#if _MSC_VER >= 1000
#pragma once
#endif

class BaseFarArray
{
protected:
	void **fItems;
	int fCount;
	int fAllocCount;
	bool fOwnsItems;
	int fResizeDelta;

	typedef int (__cdecl *BaseCmpFunc) (const void *, const void *);

	BaseFarArray (int resizeDelta)
		: fItems (NULL), fCount (0), fAllocCount (0), fOwnsItems (true),
		  fResizeDelta (resizeDelta) {};

	void Add (void *item);

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

	int IndexOf (const void *item)
	{
		for (int i=0; i<fCount; i++)
			if (fItems [i] == item) return i;
		return -1;
	}

	void InternalSort (BaseCmpFunc cmpFunc);

public:
	int Count()
	{
		return fCount;
	}
};

template <class T>
class FarArray: public BaseFarArray
{
private:
	typedef T *TPtr;
	typedef int (__cdecl *CmpFunc) (const T **, const T **);

public:
	FarArray (int resizeDelta = 16)
		: BaseFarArray (resizeDelta) {};
	~FarArray()
	{
		Clear();
	}

	void Clear()
	{
		if (fOwnsItems)
			for (int i=fCount-1; i >= 0; i--)
				delete (T *) fItems [i];
		InternalClear();
	}

	void Add (T *item)
	{
		BaseFarArray::Add (item);
	}

	int IndexOf (const T *item)
	{
		return BaseFarArray::IndexOf (item);
	}

	TPtr &operator[] (int index)
	{
		return (TPtr &) fItems [index];
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
		InternalSort ((BaseCmpFunc) cmpFunc);
	}
};

class FarStringArray: public BaseFarArray
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
		: BaseFarArray (resizeDelta) {};
	~FarStringArray()
	{
		Clear();
	}

	void Clear()
	{
		for (int i=0; i < fCount; i++)
			delete (char *) fItems [i];
		InternalClear();
	}

	void Add (const char *item)
	{
		BaseFarArray::Add (Copy (item));
	}

	char *operator[] (int index)
	{
		return (char *) fItems [index];
	}

	const char **GetItems()
	{
		return (const char **) fItems;
	}
};


class FarIntArray: public BaseFarArray
{
public:
	FarIntArray (int resizeDelta = 16)
		: BaseFarArray (resizeDelta)
	{
		fOwnsItems = false;
	}

	~FarIntArray()
	{
		Clear();
	}

	void Add (int item)
	{
		BaseFarArray::Add ((void *) item);
	}

	void Clear()
	{
		InternalClear();
	}

	int operator[] (int index)
	{
		return (int) fItems [index];
	}

	int *GetItems()
	{
		return (int *) fItems;
	}
};

#endif
