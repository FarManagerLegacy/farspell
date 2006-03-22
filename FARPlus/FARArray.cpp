/* $Header: /cvsroot/farplus/FARPlus/FARArray.cpp,v 1.2 2002/04/14 10:39:29 yole Exp $
   FAR+Plus: Lightweight typesafe dynamic array class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include <string.h>
#include "FARArray.h"
#include "FARPlus.h"

void BaseFarArray::BaseAdd (const void *item)
{
	if (fCount + 1 > fAllocCount)
	{
		fAllocCount += fResizeDelta;
		fItems = (char *) realloc (fItems, fAllocCount * fItemSize);
		memset (fItems + (fCount + 1) * fItemSize, 0, (fAllocCount - fCount - 1) * fItemSize);
	}
	memcpy (fItems + fCount * fItemSize, item, fItemSize);
	fCount++;
}

void BaseFarArray::BaseRemove (int index)
{
	far_assert(index >= 0);
	far_assert(index < fCount);
	memmove(fItems + index * fItemSize, 
	        fItems + (index+1) * fItemSize, 
	        (fCount - index - 1) * fItemSize);
	fCount--;
}

void BaseFarArray::InternalSort (BaseCmpFunc cmpFunc)
{
	FarSF::qsort (fItems, fCount, fItemSize, cmpFunc);
}
