/* $Header: /cvsroot/farplus/FARPlus/FARString.cpp,v 1.8 2002/09/03 06:33:01 yole Exp $
   FAR+Plus: lightweight string class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
   Portions copyright (C) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
*/

#include "FARString.h"
#include "FARPlus.h"
#include <windows.h>

// -- FarStringData ----------------------------------------------------------

FarString::FarStringData *FarString::fEmptyStringData = NULL;

FarString::FarStringData::FarStringData (const char *text, int length /*= -1*/)
	: fText (NULL), fCapacity (0), fRefCount (1)
{
	if (length == -1)
		fLength = strlen (text);
	else
		fLength = length;
	SetCapacity (fLength + 1);
	if (text)
	{
		memcpy (fText, text, fLength);
		fText[fLength]='\0'; // ss20051211
	}
	else
		*fText = '\0';
}

void FarString::FarStringData::SetCapacity (size_t capacity)
{
	const int memDelta = 16;
	
	if (capacity < fCapacity)
		return;

	size_t newCapacity = capacity + memDelta;
	char *newText = new char [newCapacity];
	
	if (fText)
	{
		memcpy (newText, fText, fLength + 1);
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

// -- FarString --------------------------------------------------------------

const FarString &FarString::Append (const char *s, int addLen)
{
	if (s != NULL && *s != '\0' && addLen != 0)
	{
		if (addLen == -1)
			addLen = strlen (s);
		int newLength = fData->fLength + addLen;
		if (IsUnique() && fData->fCapacity > newLength)
		{
			strncpy (fData->fText + fData->fLength, s, addLen);	
			fData->SetLength (newLength);
		}
		else
		{
			FarStringData *newData = new FarStringData (fData->fText, newLength);
			strncpy (newData->fText + fData->fLength, s, addLen);
            newData->fText [newLength] = '\0';
			fData->DecRef();
			fData = newData;
		}
	}
	return *this;
}

int FarString::Insert (int nIndex, const char * Str, size_t nLength)
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
			newLength - nIndex - nLength + 1 );
		
		memmove (fData->fText + nIndex, Str, nLength);
		
		fData->fLength = newLength;
	}
	
	return nLength;	
}

int FarString::Delete (int nIndex, int nCount /* = 1 */)
{
	far_assert (nIndex >= 0);	
	
	int nNewLength = fData->fLength;
	if (nIndex + nCount > nNewLength)
		nCount = nNewLength - nIndex;

	if (nCount > 0 && nIndex < nNewLength)
	{
		UniqueString();
		
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;
		memmove (fData->fText + nIndex, fData->fText + nIndex + nCount, nBytesToCopy);

		fData->fLength = nNewLength - nCount;
	}
	
	return fData->fLength;	
}

FarString FarString::Mid (int nFirst, int nCount) const
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
	
	return FarString (fData->fText + nFirst, nCount);
}

FarString FarString::ToOEM() const
{
	FarString result (fData->fText, Length());
	CharToOemBuff (fData->fText, result.GetBuffer(), Length());
	return result;
}

FarString FarString::ToANSI() const
{
	FarString result (fData->fText, Length());
	OemToCharBuff (fData->fText, result.GetBuffer(), Length());
	return result;
}

void FarString::MakeUpper()
{
	UniqueString();
	CharUpperBuff (fData->fText, fData->fLength);
}

void FarString::MakeLower()
{
	UniqueString();
	CharLowerBuff (fData->fText, fData->fLength);
}

void FarString::Trim()
{
	FarSF::Trim (GetBuffer());
	ReleaseBuffer();
}

void FarString::TrimLeft()
{
	FarSF::LTrim (GetBuffer());
	ReleaseBuffer();
}

void FarString::TrimRight()
{
	FarSF::RTrim (GetBuffer());
	ReleaseBuffer();
}

int FarString::CompareNoCase (const char *Str) const
{
	return 2-CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, fData->fLength,
		Str, -1);
}

int FarString::CompareNoCase (const char * Str, size_t nLength) const
{
	if (nLength > fData->fLength+1)
		nLength = fData->fLength+1;
	return 2-CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, fData->fText, nLength,
		Str, nLength);
}

// -- FarStringTokenizer -----------------------------------------------------

void FarStringTokenizer::Attach (const char *text, char separator, bool ignoreWhitespace)
{
	fText = text;
	fSeparator = separator;
	fCurIndex = 0;
	fIgnoreWhitespace = ignoreWhitespace;
	fCurPos = GetTokenStart (fText);
}

const char *FarStringTokenizer::GetTokenEnd (const char *tokenStart) const
{
	if (*tokenStart == '\"')
	{
		const char *closeQuote = strchr (tokenStart+1, '\"');
		if (!closeQuote)
			return tokenStart + strlen (tokenStart);
		return closeQuote+1;
	}
	else 
	{
		const char *separator = strchr (tokenStart+1, fSeparator);
		if (!separator)
			return tokenStart + strlen (tokenStart);
		return separator;
	}
}

const char *FarStringTokenizer::GetTokenStart (const char *tokenEnd) const
{
	while (*tokenEnd == fSeparator || (fIgnoreWhitespace && (*tokenEnd == ' ' || *tokenEnd == '\t')))
		tokenEnd++;
	return tokenEnd;
}

FarString FarStringTokenizer::NextToken()
{
	// it is not allowed to call NextToken() for a non-attached tokenizer
	far_assert (fText != NULL);  
	const char *tokenEnd = GetTokenEnd (fCurPos);
	FarString token (fCurPos, tokenEnd-fCurPos);
	fCurPos = GetTokenStart (tokenEnd);
	fCurIndex++;
	return token;
}

FarString FarStringTokenizer::GetToken (int index) const
{
	// it is not allowed to call GetToken() for a non-attached tokenizer
	far_assert (fText != NULL);
	far_assert (index >= 0);
	const char *curPos = GetTokenStart (fText);
	for (int i=0; i<index; i++)
	{
		curPos = GetTokenEnd (curPos);
		curPos = GetTokenStart (curPos);
		if (*curPos == '\0')
			return FarString();
	}
	const char *tokenEnd = GetTokenEnd (curPos);
	
	FarString result (curPos, tokenEnd-curPos);
	if (fIgnoreWhitespace)
		result.TrimRight();
	return result;
}

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
