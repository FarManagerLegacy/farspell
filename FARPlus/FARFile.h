/* $Header: $
   FAR+Plus: lightweight file class
   (C) 2001 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#ifndef __FARFILE_H
#define __FARFILE_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include <windows.h>
#include "FARString.h"

class FarFile
{
protected:
	HANDLE fFile;
	HANDLE fMapping;
	char *fMapPtr;
	char *fCurPtr;
	DWORD fSize;
	DWORD fLastError;

public:
	FarFile (const char *fileName, DWORD access, DWORD shareMode, DWORD createDisp);
	~FarFile();

	DWORD GetLastError() const
		{ return fLastError; }
	DWORD GetPos()
		{ return fCurPtr - fMapPtr; }
	bool IsEOF()
		{ return GetPos() >= fSize; }

	FarString ReadLine();
};

#endif
