/* $Header: /cvsroot/farplus/FARPlus/FARPlus.h,v 1.7 2002/08/24 14:55:43 yole Exp $
   FAR+Plus: A FAR Plugin C++ class library: header file
   (C) 1998-2002 Dmitry Jemerov <yole@yole.ru>
   This file is heavily based on sources by Eugene Roshal
*/

#ifndef __FARPLUS_H
#define __FARPLUS_H

#if _MSC_VER >= 1000
#pragma once
#endif

#include "plugin.hpp"
#include "FARString.h"
#include "FARArray.h"

// If this is defined, new features of FAR 1.70 are used.
// If this is not defined, the plugin will be compatible with FAR 1.65,
// but its code may be larger.
// #define USE_FAR_170

// -- Service functions ------------------------------------------------------

namespace FarSF {

    // FSF functions
    int AddEndSlash (char *Path);
    char* PointToName(const char *Path);
    void RecursiveSearch (char *InitDir, char *Mask, FRSUSERFUNC Func,
        DWORD Flags, void *param);
#ifndef USE_FAR_170
    int CmpNameList (const char *MaskList, const char *Path, bool skipPath = false);
#endif
    char *RTrim (char *Str);
    char *LTrim (char *Str);
    char *Trim (char *Str);
    wchar_t *LTrimW (wchar_t *Str);
    wchar_t *RTrimW (wchar_t *Str);
    wchar_t *TrimW (wchar_t *Str);

    void Unquote (char *Str);
    void qsort (void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
    void *bsearch (const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *));
    int atoi (const char *string);
	char *itoa (int value, char *string, int radix);
	int GetFileOwner (const char *Computer, const char *Name, char *Owner);
    int GetNumberOfLinks (const char *Name);
	char *QuoteSpaceOnly (char *Str);
	char *MkTemp (char *Dest, const char *Prefix);

	int LIsAlpha (unsigned Ch);
	int LIsAlphanum (unsigned Ch);
	int LIsLower (unsigned Ch);
    int LIsUpper (unsigned Ch);

	char *TruncStr (char *Str, int MaxLength);
	char *TruncPathStr (char *Str, int MaxLength);

	int CopyToClipboard (const char *Data);
	FarString PasteFromClipboard();

    // Useful functions not in FSF
    char *QuoteText (char *Str);
    const char *GetCommaWord(const char *Src, char *Word, char Separator=',');
    void ConvertNameToShort(const char *Src,char *Dest);
    int CheckForEsc();
    void InsertCommas (unsigned long Number, char *Dest);
    void Bytes2Str (unsigned long bytes, char *buf);

    void Trace (const char *fmt, ...);
    int Execute(HANDLE hPlugin,const char *CmdStr,bool HideOutput,
        bool Silent,bool ShowTitle,int MWaitForExternalProgram,
		bool SeparateWindow = false);

#ifdef USE_FAR_170

    extern FarStandardFunctions m_FSF;

    inline int AddEndSlash (char *Path)
        { return m_FSF.AddEndSlash (Path); }
    inline char *PointToName (const char *Path)
        { return m_FSF.PointToName (Path); }
    inline void RecursiveSearch (char *InitDir, char *Mask, FRSUSERFUNC Func,
        DWORD Flags, void *param)
        { m_FSF.FarRecursiveSearch (InitDir, Mask, Func, Flags, param); }
    inline int CmpNameList (const char *MaskList, const char *Path, bool skipPath = false)
		{ return m_FSF.ProcessName (MaskList, const_cast<char *> (Path),
			PN_CMPNAMELIST | (skipPath ? PN_SKIPPATH : 0)); }
    inline char *LTrim (char *Str)
        { return m_FSF.LTrim (Str); }
    inline char *RTrim (char *Str)
        { return m_FSF.RTrim (Str); }
    inline char *Trim (char *Str)
        { return m_FSF.Trim (Str); }
    inline void Unquote (char *Str)
        { m_FSF.Unquote (Str);  }
    inline int GetFileOwner (const char *Computer, const char *Name, char *Owner)
        { return m_FSF.GetFileOwner (Computer, Name, Owner);  }
    inline int GetNumberOfLinks (const char *Name)
        { return m_FSF.GetNumberOfLinks (Name); }
    inline char *QuoteSpaceOnly (char *Str)
        { return m_FSF.QuoteSpaceOnly (Str); }
	inline char *MkTemp (char *Dest, const char *Prefix)
		{ return m_FSF.MkTemp (Dest, Prefix); }
	inline char *TruncStr (char *Str, int MaxLength)
		{ return m_FSF.TruncStr (Str, MaxLength); }
	inline char *TruncPathStr (char *Str, int MaxLength)
		{ return m_FSF.TruncPathStr (Str, MaxLength); }
	inline int atoi (const char *string)
		{ return m_FSF.atoi (string); }
	inline char *itoa (int value, char *string, int radix)
		{ return m_FSF.itoa (value, string, radix); }
	inline int LIsAlpha (unsigned Ch)
		{ return m_FSF.LIsAlpha (Ch); }
	inline int LIsAlphanum (unsigned Ch)
		{ return m_FSF.LIsAlphanum (Ch); }
	inline int LIsLower (unsigned Ch)
		{ return m_FSF.LIsLower (Ch); }
	inline int LIsUpper (unsigned Ch)
		{ return m_FSF.LIsUpper (Ch); }
	inline int CopyToClipboard (const char *Data)
		{ return m_FSF.CopyToClipboard (Data); }
	inline FarString PasteFromClipboard()
	{
        char *buf = m_FSF.PasteFromClipboard();
		FarString str = buf;
		m_FSF.DeleteBuffer (buf);
		return str;
	}
#endif

    inline void qsort (void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
    {
#ifdef USE_FAR_170
        m_FSF.qsort (base, nelem, width, fcmp);
#else
        ::qsort (base, nelem, width, fcmp);
#endif
    }
    inline void *bsearch (const void *key, const void *base, size_t nelem, size_t width, int (__cdecl *fcmp)(const void *, const void *))
    {
#ifdef USE_FAR_170
        return m_FSF.bsearch (key, base, nelem, width, fcmp);
#else
        return ::bsearch (key, base, nelem, width, fcmp);
#endif
    }
};

// ProcessEditorEvent flags not documented in FAR 1.60

#ifndef FE_READ
    #define FE_READ 0
    #define FE_SAVE 1
#endif

// -- Far --------------------------------------------------------------------
class Far
{
protected:
    friend class FarCtrl;
    friend class FarEd;
    friend class FarDialog;
    friend class FarMessage;
    friend class FarDirList;

    static PluginStartupInfo m_Info;
    static char m_RootKey [256];
	static FarStringArray *fDiskMenuStrings;
	static FarIntArray *fDiskMenuNumbers;
	static FarStringArray *fPluginMenuStrings;
	static FarStringArray *fPluginConfigStrings;
    static DWORD  m_PluginFlags;
    static char  *m_CommandPrefix;

public:
    // Construction
    static void Init (const PluginStartupInfo *Info, DWORD PluginFlags = 0);
    static void Done();
    static int GetVersion();
    static bool RequireVersion (int MinVersion);

    static const char *GetModuleName();
	static const char *GetRootKey();

    // FAR wrappers
    static const char *GetMsg(int MsgId);
    static int Editor (const char *FileName, const char *Title=NULL, int X1=0, int Y1=0,
                int X2=-1, int Y2=-1, int StartLine=0, int StartChar=1);
    static HANDLE SaveScreen (int X1=0, int Y1=0, int X2=-1, int Y2=-1);
    static void RestoreScreen (HANDLE hScreen);
    static void Text (int X, int Y, int Color, const char *Text);
    static void Text (int X, int Y, int Color, int LngIndex);
    static void FlushText();
    static int CmpName (const char *Pattern, const char *String, bool SkipPath);
	static int GetCharTable (int Index, CharTableSet *pTable);
	static int DetectCharTable (const char *Buffer, int BufferSize);

#ifdef USE_FAR_170
	static int AdvControl (int Command, void *Param);
	static int DialogEx(int X1, int Y1, int X2, int Y2,
                     const char *HelpTopic, struct FarDialogItem *Item,
                     int ItemsNumber, DWORD Reserved, DWORD Flags,
                     FARWINDOWPROC DlgProc, long Param);
	static long SendDlgMessage(HANDLE hDlg, int Msg, int Param1, long Param2);
	static long WINAPI DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2);
#endif
	static int GetBuildNumber();

    // GetPluginInfo helpers
    static void ClearDiskMenu();
    static void ClearPluginMenu();
    static void ClearPluginConfig();
    static void AddDiskMenu (const char *Text, int Number=0);
    static void AddDiskMenu (int LngIndex, int Number=0);
    static void AddPluginMenu (const char *Text);
    static void AddPluginMenu (int LngIndex);
    static void AddPluginConfig (const char *Text);
    static void AddPluginConfig (int LngIndex);
    static void SetPluginFlags (DWORD Flags);
    static void SetCommandPrefix (const char *CommandPrefix);
    static void GetPluginInfo (struct PluginInfo *Info);

    // Miscellaneous helpers

    // Note: this function CANNOT be used when PanelItem.UserData is a class.
    // Its destructor will NOT be called.
    static void FreePanelItems (PluginPanelItem *PanelItem, int ItemsNumber);
};

// -- Inline functions -------------------------------------------------------

inline const char *Far::GetModuleName()
{
    return m_Info.ModuleName;
}

inline const char *Far::GetMsg(int MsgId)
{
    return m_Info.GetMsg (m_Info.ModuleNumber, MsgId);
}

inline int Far::Editor (const char *FileName, const char *Title, int X1, int Y1,
                       int X2, int Y2, int StartLine, int StartChar)
{
    return m_Info.Editor (FileName, Title, X1, Y1, X2, Y2, 0, StartLine, StartChar);
}

inline HANDLE Far::SaveScreen (int X1, int Y1, int X2, int Y2)
{
    return m_Info.SaveScreen (X1, Y1, X2, Y2);
}

inline void Far::RestoreScreen (HANDLE hScreen)
{
    m_Info.RestoreScreen (hScreen);
}

inline void Far::Text (int X, int Y, int Color, const char *Text)
{
    m_Info.Text (X, Y, Color, Text);
}

inline void Far::Text (int X, int Y, int Color, int LngIndex)
{
    m_Info.Text (X, Y, Color, GetMsg (LngIndex));
}

inline void Far::FlushText()
{
    m_Info.Text (0, 0, 0, NULL);
}

inline int Far::CmpName (const char *Pattern, const char *String, bool SkipPath)
{
    return m_Info.CmpName (Pattern, String, SkipPath);
}

inline int Far::GetCharTable (int Index, CharTableSet *pTable)
{
	return m_Info.CharTable (Index, reinterpret_cast<char *> (pTable), sizeof (CharTableSet));
}

inline int Far::DetectCharTable (const char *Buffer, int BufferSize)
{
	return m_Info.CharTable (FCT_DETECT, const_cast<char *> (Buffer), BufferSize);
}

inline void Far::SetPluginFlags (DWORD Flags)
{
    m_PluginFlags = Flags;
}

#ifdef USE_FAR_170

inline int Far::AdvControl (int Command, void *Param)
{
	return m_Info.AdvControl (m_Info.ModuleNumber, Command, Param);
}

inline int Far::DialogEx(int X1, int Y1, int X2, int Y2,
                     const char *HelpTopic, struct FarDialogItem *Item,
                     int ItemsNumber, DWORD Reserved, DWORD Flags,
                     FARWINDOWPROC DlgProc, long Param)
{
	if (!DlgProc) DlgProc = m_Info.DefDlgProc;
	return m_Info.DialogEx (m_Info.ModuleNumber, X1, Y1, X2, Y2,
	   HelpTopic, Item, ItemsNumber, Reserved, Flags, DlgProc, Param);
}

inline long Far::SendDlgMessage(HANDLE hDlg, int Msg, int Param1, long Param2)
{
	return m_Info.SendDlgMessage(hDlg, Msg, Param1, Param2);
}

inline long WINAPI Far::DefDlgProc(HANDLE hDlg, int Msg, int Param1, long Param2)
{
	return m_Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}
#endif

inline int Far::GetBuildNumber()
{
	if (GetVersion() >= 0x170)
		return HIWORD (m_Info.AdvControl (m_Info.ModuleNumber, ACTL_GETFARVERSION, NULL));
	return 0;
}

// -- FarCtrl ----------------------------------------------------------------

// encapsulates the Control API

class FarCtrl
{
private:
    HANDLE m_hPlugin;

	int FCTL (int Command, void *Param);

public:
    FarCtrl()
        : m_hPlugin (INVALID_HANDLE_VALUE) {}
    FarCtrl (HANDLE hPlugin)
        : m_hPlugin (hPlugin) {}
    void SetHandle (HANDLE hPlugin)
        { m_hPlugin = hPlugin; }

    int ClosePlugin (const char *DestPath);
    int GetPanelInfo (PanelInfo *pi);
	PanelInfo GetPanelInfo();
    int GetAnotherPanelInfo (PanelInfo *pi);
	PanelInfo GetAnotherPanelInfo();
    int UpdatePanel (bool clearSelection = false);
    int UpdateAnotherPanel (bool clearSelection = false);
    int RedrawPanel();
    int RedrawAnotherPanel();
	int SetPanelDir (const char *dir);
	int SetAnotherPanelDir (const char *dir);
	int GetCmdLine (char *buf);
	FarString GetCmdLine();
	int SetCmdLine (const char *buf);
	int InsertCmdLine (const char *buf);
	int GetCmdLinePos();
	int SetCmdLinePos (int newPos);
    int SetSelection (PanelInfo *pi);
	int SetAnotherSelection (PanelInfo *pi);
    int SetUserScreen();
	int SetViewMode (int viewMode);
	int SetAnotherViewMode (int viewMode);
};

inline int FarCtrl::FCTL (int Command, void *Param)
{
	return Far::m_Info.Control (m_hPlugin, Command, Param);
}

inline int FarCtrl::ClosePlugin (const char *DestPath)
{
    return FCTL (FCTL_CLOSEPLUGIN, (void *) DestPath);
}

inline int FarCtrl::GetPanelInfo (PanelInfo *pi)
{
    return FCTL (FCTL_GETPANELINFO, pi);
}

inline PanelInfo FarCtrl::GetPanelInfo()
{
	PanelInfo pi;
	GetPanelInfo (&pi);
	return pi;
}

inline PanelInfo FarCtrl::GetAnotherPanelInfo()
{
	PanelInfo pi;
	GetAnotherPanelInfo (&pi);
	return pi;
}

inline int FarCtrl::GetAnotherPanelInfo (PanelInfo *pi)
{
    return FCTL (FCTL_GETANOTHERPANELINFO, pi);
}

inline int FarCtrl::UpdatePanel (bool clearSelection)
{
    return FCTL (FCTL_UPDATEPANEL, clearSelection ? NULL : (void *) 1);
}

inline int FarCtrl::UpdateAnotherPanel (bool clearSelection)
{
    return FCTL (FCTL_UPDATEANOTHERPANEL, clearSelection ? NULL : (void *) 1);
}

inline int FarCtrl::RedrawPanel()
{
    return FCTL (FCTL_REDRAWPANEL, NULL);
}

inline int FarCtrl::RedrawAnotherPanel()
{
    return FCTL (FCTL_REDRAWANOTHERPANEL, NULL);
}

inline int FarCtrl::SetSelection (PanelInfo *pi)
{
    return FCTL (FCTL_SETSELECTION, pi);
}

inline int FarCtrl::SetAnotherSelection (PanelInfo *pi)
{
    return FCTL (FCTL_SETANOTHERSELECTION, pi);
}

inline int FarCtrl::SetUserScreen()
{
    return FCTL (FCTL_SETUSERSCREEN, NULL);
}

inline int FarCtrl::SetPanelDir (const char *dir)
{
	return FCTL (FCTL_SETPANELDIR, (void *) dir);
}

inline int FarCtrl::SetAnotherPanelDir (const char *dir)
{
	return FCTL (FCTL_SETANOTHERPANELDIR, (void *) dir);
}

inline int FarCtrl::GetCmdLine (char *buf)
{
	return FCTL (FCTL_GETCMDLINE, buf);
}

inline FarString FarCtrl::GetCmdLine()
{
	char buf [1024];
	GetCmdLine (buf);
	return FarString (buf);
}

inline int FarCtrl::SetCmdLine (const char *buf)
{
	return FCTL (FCTL_SETCMDLINE, (void *) buf);
}

inline int FarCtrl::InsertCmdLine (const char *buf)
{
	return FCTL (FCTL_INSERTCMDLINE, (void *) buf);
}

inline int FarCtrl::GetCmdLinePos()
{
	int pos;
    FCTL (FCTL_GETCMDLINEPOS, &pos);
	return pos;
}

inline int FarCtrl::SetCmdLinePos (int newPos)
{
	return FCTL (FCTL_SETCMDLINEPOS, &newPos);
}

inline int FarCtrl::SetViewMode (int viewMode)
{
	return FCTL (FCTL_SETVIEWMODE, &viewMode);
}

inline int FarCtrl::SetAnotherViewMode (int viewMode)
{
	return FCTL (FCTL_SETANOTHERVIEWMODE, &viewMode);
}

// -- FarMessage -------------------------------------------------------------

class FarMessage
{
protected:
    unsigned int m_Flags;
    FarString m_HelpTopic;
    char **m_Items;
    int m_ItemsNumber;
    int m_ButtonsNumber;

public:
    FarMessage (unsigned int Flags=0, const char *HelpTopic=NULL);
    ~FarMessage();
    // The first line is the message title
    void AddLine (const char *Text);
    void AddLine (int LngIndex);
    void AddFmt (const char *FmtText, ...);
    void AddFmt (int FmtLngIndex, ...);
    void AddSeparator();
    int AddButton (const char *Text);
    int AddButton (int LngIndex);
    int Show();
    int SimpleMsg (unsigned int Flags, ...);
    int ErrMsg (const char *Text, int ExtraFlags = 0);
    int ErrMsg (int LngIndex, int ExtraFlags = 0);
};

// -- FarSaveScreen ----------------------------------------------------------

class FarSaveScreen
{
private:
    HANDLE m_handle;

public:
    FarSaveScreen()
        { m_handle = Far::SaveScreen(); }
    ~FarSaveScreen()
        { Far::RestoreScreen (m_handle); }
};

// -- FarDirList -------------------------------------------------------------

class FarDirList
{
private:
    PluginPanelItem *m_pItems;
    int m_itemsNumber;
    bool m_status;

public:
    FarDirList (const char *Dir)
        { m_status = Far::m_Info.GetDirList (Dir, &m_pItems, &m_itemsNumber) ? true : false; }
    ~FarDirList()
        { if (m_status) Far::m_Info.FreeDirList (m_pItems); }

    bool Success()
        { return m_status; }
    int Count()
        { return m_itemsNumber; }
    PluginPanelItem &operator[] (int index)
        { return m_pItems [index]; }
};

#endif
