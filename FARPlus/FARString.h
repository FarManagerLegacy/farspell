/* $Header: /cvsroot/farplus/FARPlus/FARString.h,v 1.8 2002/09/03 06:33:01 yole Exp $
   FAR+Plus: A lightweight string class to be used in FAR plugins.
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   Portions copyright (C) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
*/

#ifndef __FARSTRING_H
#define __FARSTRING_H

#include <string.h>
#include "FARMemory.h"
#include "FARDbg.h"

#if _MSC_VER >= 1000
#pragma once
#endif

// strncpy template
template <class TChar> TChar *t_strncpy(TChar *strDest, const TChar *strSource, size_t count);

template <> inline char *t_strncpy<char>(char *strDest, const char *strSource, size_t count)
{ return strncpy(strDest, strSource, count); }

template <> inline wchar_t *t_strncpy<wchar_t>(wchar_t *strDest, const wchar_t *strSource, size_t count)
{ return wcsncpy(strDest, strSource, count); }

// strchr template
template <class TChar> TChar *t_strchr(const TChar *string, int c);

template <> inline char *t_strchr<char>(const char *string, int c)
{ return strchr(string, c);  }

template <> inline wchar_t *t_strchr<wchar_t>(const wchar_t *string, int c)
{ return wcschr(string, c); }

// strlen template
template <class TChar> size_t t_strlen(const TChar *string);

template <> inline size_t t_strlen<char>(const char *string)
{ return strlen(string); }

template <> inline size_t t_strlen<wchar_t>(const wchar_t *string)
{ return wcslen(string); }

template <class TChar>
class FarStringT
{
private:
	class FarStringData
	{
		friend class FarStringT;

		TChar *fText;
		size_t fLength;
		size_t fCapacity;
		int fRefCount;

		FarStringData (const TChar *text, int length = -1);

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
		
		void SetCapacity (size_t capacity);
		void SetLength (size_t newLength)
		{
			far_assert (newLength < fCapacity);
			fLength = newLength;
			fText [fLength] = '\0';
		}
	};

				   
	FarStringData *fData;
	static FarStringData *fEmptyStringData;

	bool IsUnique() const
	{
		return fData->fRefCount == 1;
	}
	
	void UniqueString()
	{
		if (fData->fRefCount > 1)
		{
			FarStringData *newData = new FarStringData (fData->fText, fData->fLength);
			fData->DecRef();
			fData = newData;
		}
	}

	FarStringData *GetEmptyStringData();

	FarStringData *GetStringData (const TChar *text, size_t length = -1)
	{
		if ((text == NULL || *text == '\0') && length == -1)
			return GetEmptyStringData();
		if (length == 0)
			return GetEmptyStringData();
		return new FarStringData (text, length);
	}

	const FarStringT &Append (const TChar *text, int addLen);

public:
	FarStringT()
	{
		fData = GetEmptyStringData();
	}
	
	FarStringT (const TChar *text)
	{
		fData = GetStringData (text);
	}

	FarStringT (const TChar *text, int length)
	{
		fData = GetStringData (text, length);
	}

	FarStringT (const FarStringT &str)
		: fData (str.fData)
	{
		fData->AddRef();
	}

	FarStringT (TChar c, int nCount)
	{
		fData = GetStringData (NULL, nCount);
		for (int i=0; i<nCount; i++)
			fData->fText [i] = c;
	}

	~FarStringT()
	{
		fData->DecRef();
	}

	operator const TChar*() const
	{
		return fData->fText;
	}

	const TChar *c_str() const
	{
		return fData->fText;
	}

	const TChar *data() const
	{
		return fData->fLength == 0 ? NULL : fData->fText;
	}

	TChar *GetBuffer (int nLength = -1)
	{
		UniqueString();
		if (nLength > 0)
			fData->SetCapacity (nLength);
		return fData->fText;
	}

	void ReleaseBuffer (int newLength = -1)
	{
		UniqueString();
		
		if (newLength == -1)
			newLength = (fData->fText) ? t_strlen<TChar> (fData->fText) : 0;
		
		fData->SetLength (newLength);
	}

	TChar operator[] (int index) const
	{
		return fData->fText [index];
	}

	TChar& operator[] (int index)
	{
		return fData->fText [index];
	}

	const FarStringT &operator= (TChar ch)
	{
		SetLength (1);
		fData->fText [0] = ch;
		return *this;
	}
	
	const FarStringT &operator= (const FarStringT &rhs)
	{
		if (&rhs != this)
		{
			rhs.fData->AddRef();
			fData->DecRef();
			fData = rhs.fData;
		}
		return *this;
	}
	
	const FarStringT &operator= (const TChar *text)
	{
		if (text != fData->fText)
		{
			FarStringData *newData = GetStringData (text);
			fData->DecRef();
			fData = newData;
		}
		return *this;
	}

	bool operator== (const FarStringT &rhs) const	
	{
		if (fData == rhs.fData)
			return true;
		if (fData->fLength != rhs.fData->fLength)
			return false;
		return memcmp (fData->fText, rhs.fData->fText, fData->fLength*sizeof(TChar)) == 0;
	}
		
	int Compare(const TChar *Str) const
	{
		if (Str == NULL)
			return 1;   // we're greater than an empty string

		return memcmp (fData->fText, Str, (fData->fLength+1)*sizeof(TChar));
	}

	int Compare (const TChar *Str, size_t nLength) const
	{
		if (Str == NULL)
			return 1;   // we're greater than an empty string

		return memcmp (fData->fText, Str, nLength*sizeof(TChar));
	}

	int CompareNoCase (const TChar *Str) const;
	int CompareNoCase (const TChar *Str, size_t nLength) const;

	void SetText (const TChar *text, int length)
	{
		fData->DecRef();
		fData = GetStringData (text, length);
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
			if (newLength < fData->fCapacity && IsUnique())
				fData->SetLength (newLength);
			else
			{
				FarStringData *newData = GetStringData (fData->fText, newLength);
				fData->DecRef();
				fData = newData;
			}
		}
	}

	void Empty()
	{
		SetLength (0);
	}

	const FarStringT &operator += (const TChar *s)
	{
		return Append (s, -1);
	}

	const FarStringT &operator += (const FarStringT &str)
	{
		return Append (str.fData->fText, str.fData->fLength);
	}

	const FarStringT &operator += (TChar c)
	{
		return Append (&c, 1);
	}

	int Insert (int nIndex, const TChar *Str, size_t nLength);
	
	int Insert (int nIndex, const TChar *Str)
	{
		if (Str && *Str)
			return Insert (nIndex, Str, t_strlen<TChar> (Str));
		return 0;
	}

	int Insert (int nIndex, const FarStringT &Str)
	{
		return Insert (nIndex, Str.fData->fText, Str.fData->fLength);
	}

	int Delete (int nIndex, int nCount = 1);
	
	FarStringT Mid (int nFirst, int nCount) const;
	FarStringT Mid (int nFirst) const
	{
		return Mid (nFirst, fData->fLength - nFirst);
	}

	FarStringT Left(int nCount) const
	{
		return Mid (0, nCount);
	}

	FarStringT Right (size_t nCount) const
	{
		if (nCount > fData->fLength)
			nCount = fData->fLength;
		return Mid (fData->fLength - nCount, nCount);
	}

	int IndexOf (TChar c, int startChar = 0) const
	{
		for (unsigned int i=startChar; i < fData->fLength; i++)
			if (fData->fText [i] == c)
				return i;
		return -1;
	}

	int LastIndexOf (TChar c) const
	{
		for (int i=fData->fLength; i >= 0; i--)
			if (fData->fText [i] == c)
				return i;
		return -1;
	}

	FarStringT<char> ToOEM() const;
	FarStringT<char> ToANSI() const;
	void MakeUpper();
	void MakeLower();
	void Trim();
	void TrimLeft();
	void TrimRight();

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
};

typedef FarStringT<char> FarString;
typedef FarStringT<char> FarStringA;
typedef FarStringT<wchar_t> FarStringW;

// -- FarStringTokenizer -----------------------------------------------------

template <class TChar>
class FarStringTokenizerT
{
private:
	const TChar *fText;
	char fSeparator;
	const TChar *fCurPos;
	int fCurIndex;
	bool fIgnoreWhitespace;

	const TChar *GetTokenEnd (const TChar *tokenStart) const;
	const TChar *GetTokenStart (const TChar *tokenEnd) const;

	FarStringTokenizerT (const FarStringTokenizerT &rhs); // not implemented
	void operator= (const FarStringTokenizerT &rhs);     // not implemented

public:
	FarStringTokenizerT()
		: fText (NULL), fSeparator (','), fCurPos (NULL), fCurIndex (-1), fIgnoreWhitespace (true)
	{
	}

	FarStringTokenizerT (const TChar *text, TChar separator = ',', bool ignoreWhitespace = true)
		: fText (NULL)
	{
		Attach (text, separator, ignoreWhitespace);
	}

	void Attach (const TChar *text, TChar separator = ',', bool ignoreWhitespace = true);
	bool HasNext() const
	{ 
		if (fText == NULL) return false;
		return *fCurPos != '\0'; 
	}

	int GetCurIndex() const
		{ return fCurIndex; }
	FarStringT<TChar> NextToken();
	FarStringT<TChar> GetToken (int index) const;
};

typedef FarStringTokenizerT<char> FarStringTokenizer;
typedef FarStringTokenizerT<char> FarStringTokenizerA;
typedef FarStringTokenizerT<wchar_t> FarStringTokenizerW;

// -- FarFileName ------------------------------------------------------------

class FarFileName: public FarString
{
public:
	FarFileName()
		: FarString() {};
	
	FarFileName (const char *text)
		: FarString (text) {};

	FarFileName (const char *text, int length)
		: FarString (text, length) {};

	FarFileName (const FarString &str)
		: FarString (str) {};

	void AddEndSlash();
	void QuoteSpaceOnly();
	FarFileName GetNameExt() const;
	FarFileName GetExt() const;
	FarFileName GetPath() const;
	FarFileName GetFullName() const;
	FarFileName GetShortName() const;

	void SetPath (const char * path);
	void SetNameExt (const char * name);
	void SetExt (const char * ext);

	static FarFileName MakeName (const FarString &path, const FarString &name);
};

// -- FarString implementation -----------------------------------------------

template <class TChar>
inline FarStringT<TChar> operator+ (const FarStringT<TChar> &lhs, TChar c)
{
	FarStringT<TChar> result = lhs;
	result += c;
	return result;
}

template <class TChar>
inline FarStringT<TChar> operator+ (TChar c, const FarStringT<TChar> &rhs)
{
	FarStringT<TChar> result (&c, 1);
	result += rhs;
	return result;
}

template <class TChar>
inline FarStringT<TChar> operator+ (const FarStringT<TChar> &lhs, const FarStringT<TChar> &rhs)
{
	FarStringT<TChar> result = lhs; 
	result += rhs;
	return result;
}

template <class TChar>
inline FarStringT<TChar> operator+ (const TChar *lhs, const FarStringT<TChar> &rhs)
{
	FarStringT<TChar> result = lhs;
	result += rhs;
	return result;
}

template <class TChar>
inline FarStringT<TChar> operator+ (const FarStringT<TChar> &lhs, const TChar *rhs)
{
	FarStringT<TChar> result = lhs;
	result += rhs;
	return result;
}

template <class TChar>
inline bool operator==( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) == 0; }
template <class TChar>
inline bool operator==( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) == 0; }
template <class TChar>
inline bool operator!=( const FarStringT<TChar>& s1, const FarStringT<TChar>& s2 ) { return s1.Compare( s2 ) != 0; }
template <class TChar>
inline bool operator!=( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) != 0; }
template <class TChar>
inline bool operator!=( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) != 0; }
template <class TChar>
inline bool operator<( const FarStringT<TChar>& s1, const FarStringT<TChar>& s2 ) { return s1.Compare( s2 ) < 0; }
template <class TChar>
inline bool operator<( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) < 0; }
template <class TChar>
inline bool operator<( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) > 0; }
template <class TChar>
inline bool operator>( const FarStringT<TChar>& s1, const FarStringT<TChar>& s2 ) { return s1.Compare( s2 ) > 0; }
template <class TChar>
inline bool operator>( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) > 0; }
template <class TChar>
inline bool operator>( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) < 0; }
template <class TChar>
inline bool operator<=( const FarStringT<TChar>& s1, const FarStringT<TChar>& s2 ){ return s1.Compare( s2 ) <= 0; }
template <class TChar>
inline bool operator<=( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) <= 0; }
template <class TChar>
inline bool operator<=( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) >= 0; }
template <class TChar>
inline bool operator>=( const FarStringT<TChar>& s1, const FarStringT<TChar>& s2 ) { return s1.Compare( s2 ) >= 0; }
template <class TChar>
inline bool operator>=( const FarStringT<TChar>& s1, const TChar * s2 ) { return s1.Compare( s2 ) >= 0; }
template <class TChar>
inline bool operator>=( const TChar * s1, const FarStringT<TChar>& s2 ) { return s2.Compare( s1 ) <= 0; }

#endif
