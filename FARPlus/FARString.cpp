/* $Header: /cvsroot/farplus/FARPlus/FARString.cpp,v 1.8 2002/09/03 06:33:01 yole Exp $
   FAR+Plus: lightweight string class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   Portions copyright (C) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
*/

#include "FARString.h"
#include "FARPlus.h"
#include <windows.h>

// -- FarStringData ----------------------------------------------------------

FarStringT<char>::FarStringData *FarStringT<char>::fEmptyStringData = NULL;
FarStringT<wchar_t>::FarStringData *FarStringT<wchar_t>::fEmptyStringData = NULL;

template <class TChar>
FarStringT<TChar>::FarStringData::FarStringData (const TChar *text, int length /*= -1*/)
	: fText (NULL), fCapacity (0), fRefCount (1)
{
	if (length == -1)
		fLength = t_strlen<TChar>(text);
	else
		fLength = length;
	SetCapacity (fLength + 1);
	if (text)
	{
		memcpy (fText, text, fLength*sizeof(TChar));
		fText[fLength]='\0'; // ss20051211
	}
	else
		*fText = '\0';
}

template <class TChar>
void FarStringT<TChar>::FarStringData::SetCapacity (size_t capacity)
{
	const int memDelta = 16;
	
	if (capacity < fCapacity)
		return;

	size_t newCapacity = capacity + memDelta;
	TChar *newText = new TChar [newCapacity];
	
	if (fText)
	{
		memcpy (newText, fText, (fLength + 1) * sizeof(TChar));
		delete [] fText;
		fText = newText;
	}
	else
	{
		fText   = newText;
		*fText  = '\0';
	}
	
	fCapacity = newCapacity;
}


template <>
FarStringT<char>::FarStringData *FarStringT<char>::GetEmptyStringData()
{
	if (fEmptyStringData == NULL)
		fEmptyStringData = new FarStringData ("");
	fEmptyStringData->AddRef();
	return fEmptyStringData;
}

template <>
FarStringT<wchar_t>::FarStringData *FarStringT<wchar_t>::GetEmptyStringData()
{
	if (fEmptyStringData == NULL)
		fEmptyStringData = new FarStringData (L"");
	fEmptyStringData->AddRef();
	return fEmptyStringData;
}

// -- FarString --------------------------------------------------------------

template <class TChar>
const FarStringT<TChar> &FarStringT<TChar>::Append (const TChar *s, int addLen)
{
	if (s != NULL && *s != '\0' && addLen != 0)
	{
		if (addLen == -1)
			addLen = t_strlen<TChar> (s);
		int newLength = fData->fLength + addLen;
		if (IsUnique() && fData->fCapacity > newLength)
		{
			t_strncpy<TChar> (fData->fText + fData->fLength, s, addLen);	
			fData->SetLength (newLength);
		}
		else
		{
			FarStringData *newData = new FarStringData (fData->fText, newLength);
			t_strncpy<TChar> (newData->fText + fData->fLength, s, addLen);
			newData->fText [newLength] = '\0';
			fData->DecRef();
			fData = newData;
		}
	}
	return *this;
}

template <class TChar>
int FarStringT<TChar>::Insert (int nIndex, const TChar * Str, size_t nLength)
{
	far_assert (nIndex >= 0);
	
	if (nLength > 0)
	{
		int newLength = fData->fLength;
		UniqueString();
		
		if (nIndex > newLength)
			nIndex = newLength;

		newLength += nLength;
		
		fData->SetCapacity (newLength);
		
		memmove (fData->fText + nIndex + nLength,
			fData->fText + nIndex,
			(newLength - nIndex - nLength + 1) * sizeof(TChar) );
		
		memmove (fData->fText + nIndex, Str, nLength*sizeof(TChar));
		
		fData->fLength = newLength;
	}
	
	return nLength;	
}

template <class TChar>
int FarStringT<TChar>::Delete (int nIndex, int nCount /* = 1 */)
{
	far_assert (nIndex >= 0);	
	
	int nNewLength = fData->fLength;
	if (nIndex + nCount > nNewLength)
		nCount = nNewLength - nIndex;

	if (nCount > 0 && nIndex < nNewLength)
	{
		UniqueString();
		
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;
		memmove (fData->fText + nIndex, fData->fText + nIndex + nCount, 
		  nBytesToCopy*sizeof(TChar));

		fData->fLength = nNewLength - nCount;
	}
	
	return fData->fLength;	
}

template <class TChar>
FarStringT<TChar> FarStringT<TChar>::Mid (int nFirst, int nCount) const
{
	if (nFirst < 0)
		nFirst = 0;

	if (nCount < 0)
		nCount = 0;
	
	if (nFirst + nCount > fData->fLength)
	{
		nCount = fData->fLength - nFirst;
	}
	if (nFirst > fData->fLength)
	{
		nCount = 0;
	}
	
	far_assert (nFirst >= 0 );
	far_assert (nFirst + nCount <= fData->fLength);
	
	if (nFirst == 0 && nFirst + nCount == fData->fLength)
		return *this;
	
	return FarStringT<TChar> (fData->fText + nFirst, nCount);
}

template <>
FarStringA FarStringT<char>::ToOEM() const
{
	FarStringA result (fData->fText, Length());
	CharToOemBuff (fData->fText, result.GetBuffer(), Length());
	return result;
}

template <>
FarStringA FarStringT<wchar_t>::ToOEM() const
{
	FarStringA result;
        int ac = WideCharToMultiByte(CP_OEMCP, 0, fData->fText, Length(), 
                   result.GetBuffer(Length()), Length(), NULL, NULL);
        result.ReleaseBuffer(ac);
	return result;
}


template <>
FarStringA FarStringT<char>::ToANSI() const
{
	FarStringA result (fData->fText, Length());
	OemToCharBuff (fData->fText, result.GetBuffer(), Length());
	return result;
}

template <>
FarStringA FarStringT<wchar_t>::ToANSI() const
{
	FarStringA result;
        int ac = WideCharToMultiByte(CP_ACP, 0, fData->fText, Length(), 
                   result.GetBuffer(Length()), Length(), NULL, NULL);
        result.ReleaseBuffer(ac);
	return result;
}

template <>
void FarStringT<char>::MakeUpper()
{
	UniqueString();
	CharUpperBuffA (fData->fText, fData->fLength);
}

template <>
void FarStringT<wchar_t>::MakeUpper()
{
	UniqueString();
	CharUpperBuffW (fData->fText, fData->fLength);
}

template <>
void FarStringT<char>::MakeLower()
{
	UniqueString();
	CharLowerBuffA (fData->fText, fData->fLength);
}

template <>
void FarStringT<wchar_t>::MakeLower()
{
	UniqueString();
	CharLowerBuffW (fData->fText, fData->fLength);
}

template <>
void FarStringT<char>::Trim()
{
	FarSF::Trim (GetBuffer());
	ReleaseBuffer();
}

template <>
void FarStringT<wchar_t>::Trim()
{
	FarSF::TrimW (GetBuffer());
	ReleaseBuffer();
}

template <>
void FarStringT<char>::TrimLeft()
{
	FarSF::LTrim (GetBuffer());
	ReleaseBuffer();
}

template <>
void FarStringT<wchar_t>::TrimLeft()
{
	FarSF::LTrimW (GetBuffer());
	ReleaseBuffer();
}

template <>
void FarStringT<char>::TrimRight()
{
	FarSF::RTrim (GetBuffer());
	ReleaseBuffer();
}

template <>
void FarStringT<wchar_t>::TrimRight()
{
	FarSF::RTrimW (GetBuffer());
	ReleaseBuffer();
}


template <>
int FarStringT<char>::CompareNoCase (const char *Str) const
{
	return 2-CompareStringA (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, fData->fLength,
		Str, -1);
}

template <>
int FarStringT<wchar_t>::CompareNoCase (const wchar_t *Str) const
{
	return 2-CompareStringW (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, fData->fLength,
		Str, -1);
}

template <>
int FarStringT<char>::CompareNoCase (const char * Str, size_t nLength) const
{
	if (nLength > fData->fLength+1)
		nLength = fData->fLength+1;
	return 2-CompareStringA (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, nLength,
		Str, nLength);
}

template <>
int FarStringT<wchar_t>::CompareNoCase (const wchar_t * Str, size_t nLength) const
{
	if (nLength > fData->fLength+1)
		nLength = fData->fLength+1;
	return 2-CompareStringW (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, nLength,
		Str, nLength);
}

// -- FarStringTokenizer -----------------------------------------------------

template <class TChar>
void FarStringTokenizerT<TChar>::Attach (const TChar *text, TChar separator, bool ignoreWhitespace)
{
	fText = text;
	fSeparator = separator;
	fCurIndex = 0;
	fIgnoreWhitespace = ignoreWhitespace;
	fCurPos = GetTokenStart (fText);
}

template <class TChar>
const TChar *FarStringTokenizerT<TChar>::GetTokenEnd (const TChar *tokenStart) const
{
	if (*tokenStart == '\"')
	{
		const TChar *closeQuote = t_strchr<TChar> (tokenStart+1, '\"');
		if (!closeQuote)
			return tokenStart + t_strlen<TChar> (tokenStart);
		return closeQuote+1;
	}
	else 
	{
		const TChar *separator = t_strchr<TChar> (tokenStart+1, fSeparator);
		if (!separator)
			return tokenStart + t_strlen<TChar> (tokenStart);
		return separator;
	}
}

template <class TChar>
const TChar *FarStringTokenizerT<TChar>::GetTokenStart (const TChar *tokenEnd) const
{
	while (*tokenEnd == fSeparator || (fIgnoreWhitespace && (*tokenEnd == ' ' || *tokenEnd == '\t')))
		tokenEnd++;
	return tokenEnd;
}

template <class TChar>
FarStringT<TChar> FarStringTokenizerT<TChar>::NextToken()
{
	// it is not allowed to call NextToken() for a non-attached tokenizer
	far_assert (fText != NULL);  
	const TChar *tokenEnd = GetTokenEnd (fCurPos);
	FarStringT<TChar> token (fCurPos, tokenEnd-fCurPos);
	fCurPos = GetTokenStart (tokenEnd);
	fCurIndex++;
	return token;
}

template <class TChar>
FarStringT<TChar> FarStringTokenizerT<TChar>::GetToken (int index) const
{
	// it is not allowed to call GetToken() for a non-attached tokenizer
	far_assert (fText != NULL);
	far_assert (index >= 0);
	const TChar *curPos = GetTokenStart (fText);
	for (int i=0; i<index; i++)
	{
		curPos = GetTokenEnd (curPos);
		curPos = GetTokenStart (curPos);
		if (*curPos == '\0')
			return FarStringT<TChar>();
	}
	const TChar *tokenEnd = GetTokenEnd (curPos);
	
	FarStringT<TChar> result (curPos, tokenEnd-curPos);
	if (fIgnoreWhitespace)
		result.TrimRight();
	return result;
}

void ToUnicode(int codepage, const FarStringA &src, FarStringW &dest)
{
  int l = src.Length();
  l = MultiByteToWideChar(codepage, 0, src.c_str(), l, NULL, 0);
  l = MultiByteToWideChar(codepage, 0, src.c_str(), l, 
    dest.GetBuffer(l), l);
  dest.ReleaseBuffer(l);
}
void ToAscii(int codepage, const FarStringW &src, FarStringA &dest)
{
  int l = src.Length();
  l = WideCharToMultiByte(codepage, 0, src.c_str(), l, NULL, 0, NULL, NULL);
  l = WideCharToMultiByte(codepage, 0, src.c_str(), l, dest.GetBuffer(l), l,
    NULL, NULL);
  dest.ReleaseBuffer(l);
}

// Compile template instances 

template class FarStringT<char>;
template class FarStringT<wchar_t>;

template class FarStringTokenizerT<char>;
template class FarStringTokenizerT<wchar_t>;

// -- FarFileName ------------------------------------------------------------

void FarFileName::AddEndSlash()
{
	if (IsEmpty() || c_str() [Length()-1] != '\\')
        Insert (Length(), "\\");
}

void FarFileName::QuoteSpaceOnly()
{
	if (IndexOf (' ') != -1)
	{
		SetLength (Length()+2);
		FarSF::QuoteSpaceOnly (GetBuffer());
		ReleaseBuffer();
	}
}

FarFileName FarFileName::GetNameExt() const
{
	return FarFileName (FarSF::PointToName (c_str()));
}

FarFileName FarFileName::GetExt() const
{
	int p = LastIndexOf ('.');
	if (p == -1)
		return FarFileName (".");
	return FarFileName (Mid (p));
}

FarFileName FarFileName::GetPath() const
{
	const char *text = c_str();
	char *pName = FarSF::PointToName (text);
	return FarFileName (text, pName - text);
}

void FarFileName::SetPath (const char * path)
{
	*this = FarFileName::MakeName (FarFileName (path).GetPath(), GetNameExt());
}

void FarFileName::SetNameExt (const char *name)
{
	*this = FarFileName::MakeName (GetPath(), FarFileName (name).GetNameExt());
}

void FarFileName::SetExt (const char * ext)
{
	FarFileName ffn = FarFileName (ext).GetExt();
 
 	int nPos = LastIndexOf( '.' );
 	if ( nPos != -1 )
 		Delete( nPos, Length() - nPos );
 
 	operator+=( ffn );
}

FarFileName FarFileName::GetFullName() const
{
	char buf [260];
	char *pName;
	DWORD dwSize = ::GetFullPathName (c_str(), sizeof (buf)-1, buf, &pName);
	if (dwSize <= sizeof (buf)-1)
		return FarFileName (buf);

	FarFileName result;
	result.SetLength (dwSize);
	::GetFullPathName (c_str(), dwSize, result.GetBuffer(), &pName);
	return result;
}

FarFileName FarFileName::GetShortName() const
{
	char shortName [260];
	FarSF::ConvertNameToShort (c_str(), shortName);
	return shortName;
}

FarFileName FarFileName::MakeName (const FarString &path, const FarString &name)
{
	FarFileName fullName = path;
	fullName.AddEndSlash();
	fullName += name;
	return fullName;
}
