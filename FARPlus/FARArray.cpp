/* $Header: $
   FAR+Plus: Lightweight typesafe dynamic array class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include <string.h>
#include "FARArray.h"
#include "FARPlus.h"

void BaseFarArray::Add (void *item)
{
	if (fCount + 1 > fAllocCount)
	{
		fAllocCount += fResizeDelta;
		fItems = (void **) realloc (fItems, fAllocCount * sizeof (void *));
		for (int i=fCount; i<fAllocCount; i++)
			fItems [i] = NULL;
	}
	fItems [fCount++] = item;
}

void BaseFarArray::InternalSort (BaseCmpFunc cmpFunc)
{
	FarSF::qsort (fItems, fCount, sizeof (void *), cmpFunc);
}
