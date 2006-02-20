/* $Header: $
   FAR+Plus: lightweight file class implementation
   (C) 2001 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#include "FARFile.h"

FarFile::FarFile (const char *fileName, DWORD access, DWORD shareMode, DWORD createDisp)
	: fFile      (NULL), 
	  fMapping   (NULL),
	  fMapPtr    (NULL),
	  fCurPtr    (NULL),
	  fSize      (0),
	  fLastError (ERROR_SUCCESS)
{
	fFile = CreateFile (fileName, access, shareMode, NULL, createDisp, 
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (!fFile)
	{
		fLastError = GetLastError();
		return;
	}
	
	fMapping = CreateFileMapping (fFile, NULL, 
		(access & GENERIC_WRITE) ? PAGE_READWRITE : PAGE_READONLY,
		0, 0, NULL);
	if (!fMapping)
	{
		fLastError = GetLastError();
		CloseHandle (fFile);
		fFile = NULL;
		return;
	}

	fMapPtr = (char *) MapViewOfFile (fMapping, 
		(access & GENERIC_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ,
		0, 0, 0);
	if (!fMapPtr)
	{
		fLastError = GetLastError();
		CloseHandle (fMapping);
		CloseHandle (fFile);
		fMapping = NULL;
		fFile = NULL;
		return;
	}

	fCurPtr = fMapPtr;
	fSize = GetFileSize (fFile, NULL);
}

FarFile::~FarFile()
{
	if (fMapPtr)
		UnmapViewOfFile (fMapPtr);
	if (fMapping)
		CloseHandle (fMapping);
	if (fFile)
		CloseHandle (fFile);
}

FarString FarFile::ReadLine()
{
	DWORD maxLen = fMapPtr + fSize - fCurPtr;
	if (!maxLen)
		return FarString();

	DWORD len = 0;
	char *pStart = fCurPtr;
	if (*fCurPtr == 0x0A) fCurPtr++;
	else 
	{
		while (*fCurPtr != 0x0D && len < maxLen)
		{
			fCurPtr++;
			len++;
		}
		fCurPtr++;
		if (*fCurPtr == 0x0A) fCurPtr++;
	}
	return FarString (pStart, len);
}
