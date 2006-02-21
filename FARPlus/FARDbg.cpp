/*************************************************************************
  FarDbg.cpp

  (c) 2001-02 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>
  
  Revision: 1.20 [30/04/2002]
  
*************************************************************************/

//#define _CRTDBG_MAP_ALLOC
#include <malloc.h>
#include <CrtDbg.h>
#ifndef _msize
#define _msize(p) _msize_dbg(p, _NORMAL_BLOCK)
#endif

#include "FarPlus.h"
#include "FarDbg.h"

#ifdef FARPLUS_TRACE

void far_ods( char const * const Fmt, ... )
{
	// здесь не используется create/delete, на случай, если 
	// захочется попользовать в CDbgReportInit::AllocHook(),
	// иначе, по понятным причинам, получим переполнение стека :)
	HANDLE Heap = GetProcessHeap();

	char * Tmp = (char*)HeapAlloc( Heap, 0, 0x1000 );

	va_list argPtr;
	va_start( argPtr, Fmt );

	wvsprintf( Tmp, Fmt, argPtr );

	va_end( argPtr );
	
	OutputDebugString( Tmp );
	
	HeapFree( Heap, 0, Tmp );
}

#endif

int show_assert_message( const char * FileName, const int Line, const char * Expression, bool bShowWinError, bool bDumpStack )
{
	FarString FN = FileName;
	FarMessage Msg( FMSG_LEFTALIGN|FMSG_WARNING|(bShowWinError?FMSG_ERRORTYPE:0) );
	Msg.AddLine( "Debug Assertion Failed!" );
	Msg.AddFmt( "Plugin    : %s", FarSF::PointToName( Far::GetModuleName() ) );
	Msg.AddFmt( "File      : %hs, Line %d", FarSF::TruncPathStr( FN.GetBuffer(), 40 ), Line );
	Msg.AddFmt( "Expression: %s", Expression );
	Msg.AddSeparator();
	Msg.AddButton( "&Abort" );
	Msg.AddButton( "&Debug" );
	Msg.AddButton( "&Ignore" );

	if ( bDumpStack )
		Msg.AddButton( "Dump &Stack" );
	
	return Msg.Show();
}

int far_assert_message( const char * FileName, const int Line, const char * Expression, bool bShowWinError )
{
	bool bDumpStack = true;

	while ( true )
	{
		int result = show_assert_message( FileName, Line, Expression, bShowWinError, bDumpStack );
		
		switch ( result ) // abort
		{
			case 0: // abort
				ExitThread( 0 );
			
			case 1: // debug
				return 1;
			
			case 2: // ignore
				return -1;
			
			case 3: // dump stack
				bDumpStack = false;
				FarDumpStack();
				break;
		}
	}

	return 1; // never
}

#ifdef _MSC_VER

inline bool dbg_isspace( char c )
{
	return c == '\n' || c == '\r' || c == '\x20';
}

int far_dbg_report( int reportType, char * userMessage, int * retVal )
{
	char * RptTypes[] = { "Warning", "Error", "Assertion Failed" };

	while ( dbg_isspace( *(userMessage + strlen( userMessage ) - 1) ) )
		*(userMessage + strlen( userMessage ) - 1) = '\0';
	
	if ( reportType == _CRT_ASSERT )
	{
		char * p = strchr( userMessage, '(' );
		if ( p )
		{
			*p = 0;
			int Line = strtol( ++p, &p, 10 );
			if ( p && *p == ')' )
			{
				p ++;
				
				p += strlen( RptTypes[ reportType ] ) + sizeof ( " : " );
				
				*p = 0;
				
				p ++;
				
				*retVal = far_assert_message( userMessage, Line, p, false );
				
				return *retVal;
			}
		}
	}
	
	FarMessage Msg( FMSG_WARNING );
	Msg.AddFmt( "Debug %s!", RptTypes[ reportType ] );
	Msg.AddLine( "" );

	char * p = strstr( userMessage, ") : " );
	if ( p && isdigit( *(p-1) ) )
	{
		p ++;
		
		*p = 0;
		
		p+= 3;

		Msg.AddLine( userMessage );
		Msg.AddLine( p );
	}
	else
	{
		Msg.AddLine( userMessage );
	}

	Msg.AddSeparator();
	
	Msg.AddButton( "&Abort" );
	Msg.AddButton( "&Debug" );
	Msg.AddButton( "&Ignore" );
	
	*retVal = Msg.Show();
	
	if ( *retVal == 0 ) // abort
	{
		ExitThread( 0 );
	}
	
	if ( *retVal == 2 ) // ignore
	{
		*retVal = -1;
	}
	
	return *retVal;
}

#ifdef _DEBUG
#ifdef HAVE_DEBUGCRT


class CDbgReportInit
{
private:
	static int m_MemDelta;
	static _CRT_ALLOC_HOOK m_OrigAllocHook;
	
	_CRT_REPORT_HOOK m_OrigReportHook;
public:
	static int __cdecl AllocHook( int nAllocType, void * pvData, size_t nSize,
		int nBlockUse, long lRequest, const BYTE * szFileName, int nLine )
	{
		switch ( nAllocType )
		{
		case _HOOK_ALLOC:
			m_MemDelta += nSize;
			break;

		case _HOOK_REALLOC:
			m_MemDelta -= _msize( pvData );
			m_MemDelta += nSize;
			break;

		case _HOOK_FREE:
			nSize = _msize( pvData );
			m_MemDelta -= nSize;
			break;

		default:
			__asm int 3; // !!! unknown memory operation !!!
			break;
		}
		
		if ( m_OrigAllocHook )
			return m_OrigAllocHook( nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine );

		return 1; // allow all allocs/reallocs/frees
	}
	
	CDbgReportInit()
	{
		// хочется считать память лично :)
		m_OrigAllocHook = _CrtSetAllocHook( CDbgReportInit::AllocHook );

		// и выводить всякую лабуду :)
		m_OrigReportHook = _CrtSetReportHook( far_dbg_report );

		// показывать все в диалоге
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG|_CRTDBG_MODE_WNDW );

		// проверка памяти
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF );
	}
	~CDbgReportInit()
	{
		// восстанавливаем стандартый обработчик сообщений
		_CrtSetReportHook( m_OrigReportHook );
		
		// переключаем вывод сообщений из диалога Far'а в ODS
		_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
		_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
		
		if ( m_MemDelta > 0 )
		{
			// где-то забыли почиститься, скажем это.
			far_ods( "-----------------------------------\n" );
			far_ods( "Detected memory leaks: %d bytes\n", m_MemDelta );
			far_ods( "-----------------------------------\n" );
		}

		_CrtSetAllocHook( m_OrigAllocHook );
	}
};

int CDbgReportInit::m_MemDelta = 0;
_CRT_ALLOC_HOOK CDbgReportInit::m_OrigAllocHook = NULL;

CDbgReportInit _DbgReportInit;

#endif HAVE_DEBUGCRT
#endif _DEBUG

class CTraceClipboardData
{
private:
	FarString m_Data;
	DWORD     m_dwTarget;
	
	char    * m_Tmp;

public:
	CTraceClipboardData( DWORD dwTarget ) : m_dwTarget( dwTarget )
	{
		m_Tmp = new char[ 0x1000 ];
	}
	~CTraceClipboardData()
	{
		delete [] m_Tmp;

		if ( m_Data.IsEmpty() ) return;
		
		// far_trace можно переопределить
		if ( m_dwTarget & FAR_STACK_DUMP_TARGET_TRACE )
			far_trace( m_Data.c_str() );
		
		// far_ods всегда выводит в ODS
		//if ( m_dwTarget & FAR_STACK_DUMP_TARGET_ODS )
		//	far_ods( m_Data.c_str() );
		
		if ( m_dwTarget & FAR_STACK_DUMP_TARGET_CLIPBOARD )
			FarSF::CopyToClipboard( m_Data );
		
	}
	void vSendOut( char const * const Fmt, va_list Args )
	{
        	wvsprintf( m_Tmp, Fmt, Args );
		m_Data += m_Tmp;
	}
	void SendOut( char const * const Fmt, ... )
	{
		va_list argPtr;
		va_start( argPtr, Fmt );
		vSendOut(Fmt,  argPtr);
		va_end( argPtr );
	}
};

void to_clipboard(void* const p, const char* text, ...)
{
  CTraceClipboardData *const clipboardData = (CTraceClipboardData*)p;
  va_list argPtr;
  va_start( argPtr, text );
  clipboardData->vSendOut(text, argPtr);
  va_end( argPtr );
}

void FarDumpStack( PEXCEPTION_POINTERS pExcPtrs, DWORD dwTarget )
{
  CTraceClipboardData clipboardData( dwTarget );
  FarDumpStackCb(pExcPtrs, to_clipboard, &clipboardData);
}

#endif // defined(_MSC_VER)
