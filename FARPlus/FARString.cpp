/* $Header: $
   FAR+Plus: lightweight string class implementation
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include "FARString.h"
#include <windows.h>

FarString::FarStringData *FarString::fEmptyStringData = NULL;

FarString FarString::GetToken (char separator)
{
	char *tokenStart = fData->fText;
	while (*tokenStart == separator)
		tokenStart++;
	if (!*tokenStart)
	{
		SetLength (0);
		return FarString();
	}
	bool endOfString = false;
	char *tokenEnd = strchr (tokenStart, separator);
	if (!tokenEnd)
	{
		tokenEnd = tokenStart + strlen (tokenStart);
		endOfString = true;
	}
	
	FarString token (tokenStart, tokenEnd - tokenStart);
	if (endOfString)
		SetLength (0);
	else
		*this = tokenEnd;
	return token;
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
