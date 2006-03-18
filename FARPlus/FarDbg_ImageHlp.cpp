/*************************************************************************
  FarDbg.cpp

  (c) 2001-02 Dennis Trachuk <dennis.trachuk@nm.ru>||<dennis.trachuk@bk.ru>

  (was Revision: 1.20 [30/04/2002])
  
*************************************************************************/

#include <windows.h>
#include <imagehlp.h>
#include "FARPlus.h"

#ifdef _MSC_VER
#pragma comment( lib, "imagehlp.lib" )
#define __TRY __try
#define __CATCH_ALL __except( EXCEPTION_EXECUTE_HANDLER )
#endif

#ifdef __GNUC__
#define __TRY try
#define __CATCH_ALL catch(...)
#endif

/////////////////////////////////////////////////////////////////////////////
// Routine to produce stack dump

#define MODULE_NAME_LEN 64
#define SYMBOL_NAME_LEN 128

struct FARPLUS_SYMBOL_INFO
{
	DWORD dwAddress;
	DWORD dwOffset;
	CHAR  szModule[ MODULE_NAME_LEN ];
	CHAR  szSymbol[ SYMBOL_NAME_LEN ];
};

static DWORD __stdcall GetModuleBase( HANDLE hProcess, DWORD dwReturnAddress )
{
	IMAGEHLP_MODULE moduleInfo;

	if ( SymGetModuleInfo( hProcess, dwReturnAddress, &moduleInfo ) )
		return moduleInfo.BaseOfImage;

	MEMORY_BASIC_INFORMATION mbInfo;
	if ( VirtualQueryEx( hProcess, (LPCVOID)dwReturnAddress, &mbInfo, sizeof( mbInfo ) ) )
	{
		char szFile[ MAX_PATH ] = { 0 };
		DWORD cch = GetModuleFileName( (HINSTANCE)mbInfo.AllocationBase, szFile, MAX_PATH );
		
		// Ignore the return code since we can't do anything with it.
		if ( SymLoadModule( hProcess, NULL, cch ? szFile : NULL, NULL,
			(DWORD)mbInfo.AllocationBase, 0 ) )
	
		return (DWORD) mbInfo.AllocationBase;
	}

	return SymGetModuleBase( hProcess, dwReturnAddress );
}

#define _countof( array ) ( sizeof( array ) / sizeof( array[ 0 ] ) )

static void ResolveSymbol( HANDLE hProcess, DWORD dwAddress, FARPLUS_SYMBOL_INFO &siSymbol )
{
	union
	{
		char rgchSymbol[ sizeof( IMAGEHLP_SYMBOL ) + 255 ];
		IMAGEHLP_SYMBOL sym;
	};

	char  szUndec     [ 256 ];
	char  szWithOffset[ 256 ];

	LPSTR pszSymbol = NULL;
	
	IMAGEHLP_MODULE mi;
	mi.SizeOfStruct = sizeof( IMAGEHLP_MODULE );
	
	ZeroMemory( &siSymbol, sizeof( FARPLUS_SYMBOL_INFO ) );

	siSymbol.dwAddress = dwAddress;
	
	if ( SymGetModuleInfo( hProcess, dwAddress, &mi ) )
		strncpy( siSymbol.szModule, FarSF::PointToName( mi.ImageName ), _countof( siSymbol.szModule ) );
	else
		strcpy( siSymbol.szModule, "<no module>" );
	
	__TRY
	{
		sym.SizeOfStruct  = sizeof( IMAGEHLP_SYMBOL );
		sym.Address       = dwAddress;
		sym.MaxNameLength = 255;

		if ( SymGetSymFromAddr( hProcess, dwAddress, &(siSymbol.dwOffset), &sym ) )
		{
			pszSymbol = sym.Name;

			if ( UnDecorateSymbolName( sym.Name, szUndec, _countof( szUndec ),
				UNDNAME_NO_MS_KEYWORDS|UNDNAME_NO_ACCESS_SPECIFIERS ) )
				pszSymbol = szUndec;
			else if ( SymUnDName( &sym, szUndec, _countof( szUndec ) ) )
				pszSymbol = szUndec;
			
			if ( siSymbol.dwOffset != 0 )
			{
				wsprintfA( szWithOffset, "%s + %d bytes", pszSymbol, siSymbol.dwOffset );
				pszSymbol = szWithOffset;
			}
		}
		else
			pszSymbol = "<no symbol>";
	}
	__CATCH_ALL
	{
		pszSymbol = "<EX: no symbol>";
		siSymbol.dwOffset = dwAddress - mi.BaseOfImage;
	}
	
	strncpy( siSymbol.szSymbol, pszSymbol, _countof( siSymbol.szSymbol ) );
}

void FarDumpStackCb( PEXCEPTION_POINTERS pExcPtrs, FarDumpCallback callback, void * const param )
{
	callback(param, "===== begin FarDumpStack output =====\r\n" );

	FarIntArray adwAddress/*( 16 )*/;
	HANDLE hProcess = ::GetCurrentProcess();
	if ( SymInitialize( hProcess, NULL, FALSE ) )
	{
		// force undecorated names to get params
		DWORD dw = SymGetOptions();
		dw &= ~SYMOPT_UNDNAME;
		SymSetOptions(dw);

		HANDLE hThread = ::GetCurrentThread();
		CONTEXT threadContext;

		threadContext.ContextFlags = CONTEXT_FULL;

		if ( ::GetThreadContext(hThread, &threadContext ) )
		{
			STACKFRAME stackFrame;
			ZeroMemory( &stackFrame, sizeof( stackFrame ) );
			stackFrame.AddrPC.Mode = AddrModeFlat;

			stackFrame.AddrPC.Offset    = threadContext.Eip;
			stackFrame.AddrStack.Offset = threadContext.Esp;
			stackFrame.AddrStack.Mode   = AddrModeFlat;
			stackFrame.AddrFrame.Offset = threadContext.Ebp;
			stackFrame.AddrFrame.Mode   = AddrModeFlat;

			for ( int nFrame = 0; nFrame < 1024; nFrame++ )
			{
				if ( StackWalk( IMAGE_FILE_MACHINE_I386, hProcess, hProcess,
					&stackFrame, &threadContext, NULL, 
					SymFunctionTableAccess, GetModuleBase, NULL) == 0 )
					break;
				adwAddress.Add( stackFrame.AddrPC.Offset );
			}
		}
	}
	else
	{
		callback(param,  "FarDumpStack Error: IMAGEHLP.DLL wasn't found. "
			"GetLastError() returned 0x%8.8X\r\n", GetLastError() );
	}
	
	// dump it out now
	for ( int nAddress = 0; nAddress < adwAddress.Count(); nAddress++ )
	{
		FARPLUS_SYMBOL_INFO info;
		
		callback(param, "%8.8X: ", adwAddress[ nAddress ] );
		
		ResolveSymbol( hProcess, adwAddress[ nAddress ], info );
		
		callback(param, "%s!%s\r\n", info.szModule, info.szSymbol );
	}
	
	callback(param, "====== end FarDumpStack output ======\r\n" );
}
