//////////////////////////////////////////////////////////////////////////
//
// FarLog.h
//
// Copyright (c) 2002 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
//

#ifndef ___FarLog_H___
#define ___FarLog_H___

#include <stdarg.h>

class FarFile;

class FarLog
{
private:
	int       m_Level;
	FarFile * m_File;
	char    * m_Tmp;
	
	void Log( int Level, char const * Fmt, va_list argPtr );
	
public:
	//   0 - no logging
	//   1 - log errors only
	//   2 - log errors and warnings
	//   3 - log all events up to Level
	FarLog( const char * FileName, int Level = 3 );
	~FarLog();
	
	void Error( char const * Fmt, ... );
	void Warning( char const * Fmt, ... );
	void Message( char const * Fmt, ... );
	void Message( int Level, char const * Fmt, ... );
	void Write( char const * Text, int nLen = -1);
};

#endif //!defined(___FarLog_H___)
