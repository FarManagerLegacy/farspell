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
