/* $Header: /cvsroot/farplus/FARPlus/MicroRTL.cpp,v 1.2 2002/08/24 14:52:54 yole Exp $
   FAR+Plus: reduced RTL startup functions for Visual C++
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#include <windows.h>

extern "C" int __stdcall _DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	return 1;
}

extern "C" int main()
{
	return 1;
}

extern "C" int __cdecl _purecall (void)
{
	MessageBox (NULL, "FARPlus", "Pure virtual function called", MB_OK);
	return 0;
}
