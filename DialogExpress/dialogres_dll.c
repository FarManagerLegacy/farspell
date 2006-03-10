/*
    DLL helper for minimize size of dynamic dialogres library
    Copyright (C) 2006 Sergey Shishmintzev

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contributor(s):
      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
      Idea from FARPlus micrortl.cpp http://farplus.sf.net/

*/

#include "dialogresInt.h"
#ifdef _DEBUG
#undef dialogres_free
void dialogres_free(void* pChunk)
{  
  _dialogres_free(pChunk, __FILE__, __LINE__);
}
#endif

int __cdecl atexit(void (__cdecl *rest)(void))
{
	return 1;
}

int __stdcall _DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	return 1;
}

int __stdcall _cygwin_dll_entry(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	return 1;
}

int main()
{
	return 1;
}
