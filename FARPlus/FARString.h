/* $Header: $
   FAR+Plus: A lightweight string class to be used in FAR plugins.
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARSTRING_H
#define __FARSTRING_H

#include <string.h>
#include "FARMemory.h"

#if _MSC_VER >= 1000
#pragma once
#endif

class FarString
{
private:
	class FarStringData
	{
		friend class FarString;

		char *fText;
		size_t fLength;
		int fRefCount;

		FarStringData (const char *text)
		{
			fLength = strlen (text);
			fText = new char [fLength + 1];
			if (text)
				strcpy (fText, text);
			else
				*fText = '\0';
			fRefCount = 1;
		}

		FarStringData (const char *text, int length)
		{
			fLength = length;
			fText = new char [length+1];
			strncpy (fText, text, length);
			fText [length] = '\0';
			fRefCount = 1;
		}

		~FarStringData()
		{
			delete [] fText;
		}

		void AddRef()
		{
			fRefCount++;
		}

		void DecRef()
		{
			fRefCount--;
			if (!fRefCount)
				delete this;
		}
	};


	FarStringData *fData;
	static FarStringData *fEmptyStringData;

	void UniqueString()
	{
		if (fData->fRefCount > 1)
		{
			FarStringData *newData = new FarStringData (fData->fText, fData->fLength);
			fData->DecRef();
			fData = newData;
		}
	}

	FarStringData *GetEmptyStringData()
	{
		if (fEmptyStringData == NULL)
			fEmptyStringData = new FarStringData ("");
		fEmptyStringData->AddRef();
		return fEmptyStringData;
	}

public:
	FarString()
	{
		fData = GetEmptyStringData();
	}
	
	FarString (const char *text)
	{
		if (text == NULL)
			fData = GetEmptyStringData();
		else
			fData = new FarStringData (text);
	}

	FarString (const char *text, int length)
	{
		if (length == 0)
			fData = GetEmptyStringData();
		else
			fData = new FarStringData (text, length);
	}

	FarString (const FarString &str)
		: fData (str.fData)
	{
		fData->AddRef();
	}

	~FarString()
	{
		fData->DecRef();
	}

	operator const char*() const
	{
		return fData->fText;
	}

	char *GetBuffer()
	{
		UniqueString();
		return fData->fText;
	}

	char operator[] (int index) const
	{
		return fData->fText [index];
	}

	const FarString &operator= (const FarString &rhs)
	{
		if (&rhs != this)
		{
			rhs.fData->AddRef();
			fData->DecRef();
			fData = rhs.fData;
		}
		return *this;
	}
	
	const FarString &operator= (const char *text)
	{
		if (text != fData->fText)
		{
			FarStringData *newData = new FarStringData (text);
			fData->DecRef();
			fData = newData;
		}
		return *this;
	}

	bool operator== (const FarString &rhs)
	{
		if (fData == rhs.fData)
			return true;
		if (fData->fLength != rhs.fData->fLength)
			return false;
		return memcmp (fData->fText, rhs.fData->fText, fData->fLength) == 0;
	}
		
	void SetText (const char *text, int length)
	{
		fData->DecRef();
		if (length == 0)
			fData = GetEmptyStringData();
		else
		{
			FarStringData *newData = new FarStringData (text, length);
			fData = newData;
		}
	}

	int Length() const
	{
		return fData->fLength;
	}

	bool IsEmpty() const
	{
		return Length() == 0;
	}

	void SetLength (size_t newLength)
	{
		if (newLength != fData->fLength)
		{
			FarStringData *newData;
			if (newLength == 0)
				newData = GetEmptyStringData();
			else
				newData = new FarStringData (fData->fText, newLength);
			fData->DecRef();
			fData = newData;
		}
	}

	FarString &operator += (const char *s)
	{
		if (s)
		{
			int addLen = strlen (s);
			FarStringData *newData = new FarStringData (fData->fText, fData->fLength + addLen);
			strcpy (newData->fText + fData->fLength, s);
			fData->DecRef();
			fData = newData;
		}
		return *this;
	}

	FarString GetToken (char separator);

	int IndexOf (char c) const
	{
		for (unsigned int i=0; i < fData->fLength; i++)
			if (fData->fText [i] == c)
				return i;
		return -1;
	}

	static void FreeEmptyString()
	{
		if (fEmptyStringData)
		{
			int oldRefCount = fEmptyStringData->fRefCount;
			fEmptyStringData->DecRef();
			if (oldRefCount == 1)
			   fEmptyStringData = NULL;
		}
	}

	FarString ToOEM() const;
	FarString ToANSI() const;
};

inline const FarString operator+ (const FarString &lhs, const FarString &rhs)
{
	FarString result = lhs;
	result += rhs;
	return result;
}

inline const FarString operator+ (const char *lhs, const FarString &rhs)
{
	FarString result = lhs;
	result += rhs;
	return result;
}

inline const FarString operator+ (const FarString &lhs, const char *rhs)
{
	FarString result = lhs;
	result += rhs;
	return result;
}

#endif
