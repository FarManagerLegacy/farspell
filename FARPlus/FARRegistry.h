/* $Header: $
   FAR+Plus: registry access class interface
   (C) 2001-02 Dmitry Jemerov <yole@yole.ru>
*/

#ifndef __FARREGISTRY_H
#define __FARREGISTRY_H

#include <windows.h>
#include "FARString.h"

#if _MSC_VER >= 1000
#pragma once
#endif

class FarRegistry
{
protected:
	FarString fRootKey;

    HKEY CreateRegKey (HKEY hRoot, const char *Key);
    HKEY OpenRegKey (HKEY hRoot, const char *Key, bool readOnly);

public:
	class KeyIterator
	{
	private:
		friend class FarRegistry;
		HKEY fEnumKey;
		int fEnumIndex;
		KeyIterator (HKEY enumKey)
			: fEnumKey (enumKey), fEnumIndex (0) {};

	public:
		KeyIterator (KeyIterator &other)
		{
			fEnumKey = other.fEnumKey;
			fEnumIndex = other.fEnumIndex;
			other.fEnumKey = NULL;
		}
		~KeyIterator();

		bool NextKey (FarString &keyName);
	};

	FarRegistry (const char *rootKey)
		: fRootKey (rootKey) {};
	FarRegistry (const char *rootKeyStart, const char *rootKeyBody)
		: fRootKey (rootKeyStart)
	{
		fRootKey += rootKeyBody;
	}

    bool SetRegKey(const char *Key,const char *ValueName,
        const char *ValueData,HKEY hRoot=HKEY_CURRENT_USER);
    bool SetRegKey(const char *Key,const char *ValueName,
        DWORD ValueData,HKEY hRoot=HKEY_CURRENT_USER);

    int GetRegKey (const char *Key, const char *ValueName,
        char *ValueData,const char *Default,DWORD DataSize,HKEY hRoot=HKEY_CURRENT_USER);
    int GetRegKey (const char *Key, const char *ValueName,
        int &ValueData,DWORD Default,HKEY hRoot=HKEY_CURRENT_USER);
    int GetRegKey (const char *Key, const char *ValueName,
        DWORD Default,HKEY hRoot=HKEY_CURRENT_USER);
	bool GetRegBool (const char *Key, const char *ValueName,
		bool Default, HKEY hRoot=HKEY_CURRENT_USER);
	FarString GetRegStr (const char *Key, const char *ValueName,
		const char *Default, HKEY hRoot=HKEY_CURRENT_USER);

    int DeleteRegKey (const char *RootKey, const char *KeyName, 
        HKEY hRoot=HKEY_CURRENT_USER);

	bool KeyExists (const char *key, const char *valueName,
		HKEY hRoot = HKEY_CURRENT_USER);

	KeyIterator EnumKeys (const char *Key, HKEY hRoot = HKEY_CURRENT_USER);
};

inline bool FarRegistry::GetRegBool (const char *Key, const char *ValueName, 
									 bool Default, HKEY hRoot/* =HKEY_CURRENT_USER */)
{
	return GetRegKey (Key, ValueName, Default, hRoot) ? true : false;
}

#endif
