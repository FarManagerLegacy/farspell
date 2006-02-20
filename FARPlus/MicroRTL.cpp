/* $Header: $
   FAR+Plus: reduced RTL startup functions for Visual C++
   (C) 2001-02 Dmitry Jemerov <yole@spb.cityline.ru>
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
