/* $Header: $
   FAR+Plus: FAR editor API wrappers interface
   (C) 2001 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#ifndef __FARED_H
#define __FARED_H

#include "FARPlus.h"

// -- FarEd ------------------------------------------------------------------

// encapsulates the EditorControl API

class FarEd
{
private:
	friend class FarEdInfo;

	FarEd();
	FarEd (const FarEd &rhs);
	const FarEd &operator= (const FarEd &rhs);
	
	static int ECTL (int Command, void *Param);
  
public:
    // 'EditorControl' API wrappers
    
    // ECTL_GETSTRING
    static int GetString (int StringNumber, EditorGetString *egs);
    static int GetStringText (int StringNumber, char *StringText, int MaxLength);
    static int GetCurStringText (char *StringText, int MaxLength);
    static int GetStringLength (int StringNumber);
    static int GetCurStringLength();
    static int GetStringSel (int StringNumber, int *SelStart, int *SelEnd);
    
    // ECTL_SETPOSITION
    static int SetPos (int CurLine, int CurPos);
    static int SetTabPos (int CurLine, int CurTabPos);
    static int SetViewPos (int TopScreenLine, int LeftPos);
    static int SetOvertype (BOOL Overtype);

    // ECTL_SELECT
    static int SelectBlock (int BlockType, int BlockStartLine, int BlockStartPos,
      int BlockWidth, int BlockHeight);
    static int UnselectBlock();

    static int InsertString (bool Indent=false);         // ECTL_INSERTSTRING
    static int InsertText (char *Text);                  // ECTL_INSERTTEXT
    static int InsertTextString (char *Text, bool Indent = false);
    static int DeleteString();                           // ECTL_DELETESTRING
    static int DeleteChar();                             // ECTL_DELETECHAR
    static int Redraw();                                 // ECTL_REDRAW
    static int EditorToOem (char *Text, int TextLength); // ECTL_EDITORTOOEM
    static int OemToEditor (char *Text, int TextLength); // ECTL_OEMTOEDITOR
    static int TabToReal (int StringNumber, int SrcPos); // ECTL_TABTOREAL
    static int RealToTab (int StringNumber, int SrcPos); // ECTL_REALTOTAB
    static int ExpandTabs (int StringNumber);            // ECTL_EXPANDTABS
    static int SetTitle (char *Title);                   // ECTL_SETTITLE
    static int ReadInput (INPUT_RECORD *pRec);           // ECTL_READINPUT
    static int ProcessInput (INPUT_RECORD *pRec);        // ECTL_PROCESSINPUT

    // ECTL_ADDCOLOR
    static int AddColor (int StringNumber, int StartPos, int EndPos, int Color);
    static int DeleteColor (int StringNumber, int StartPos=-1);

    // ECTL_GETCOLOR
    static int GetColor (int StringNumber, int ColorItem,
      int *StartPos, int *EndPos, int *Color);
};

// -- FarEdInfo --------------------------------------------------------------

class FarEdInfo
{
private:
	FarEdInfo (const FarEdInfo &rhs);
	const FarEdInfo &operator= (const FarEdInfo &rhs);

public:
	FarEdInfo();

    int EditorID;
    const char *FileName;
    int WindowSizeX;
    int WindowSizeY;
    int TotalLines;
    int CurLine;
    int CurPos;
    int CurTabPos;
    int TopScreenLine;
    int LeftPos;
    int Overtype;
    int BlockType;
    int BlockStartLine;
    int AnsiMode;
    int TableNum;
    DWORD Options;
    int TabSize;
};

// -- inline functions -------------------------------------------------------

inline int FarEd::ECTL (int Command, void *Param)
{
    return Far::m_Info.EditorControl (Command, Param);
}

inline int FarEd::InsertText (char *Text)
{
    return ECTL (ECTL_INSERTTEXT, Text);
}

inline int FarEd::DeleteString()
{
    return ECTL (ECTL_DELETESTRING, NULL);
}

inline int FarEd::DeleteChar()
{
    return ECTL (ECTL_DELETECHAR, NULL);
}

inline int FarEd::Redraw()
{
    return ECTL (ECTL_REDRAW, NULL);
}

inline int FarEd::ExpandTabs (int StringNumber)
{
    return ECTL (ECTL_EXPANDTABS, (void *) StringNumber);
}

inline int FarEd::SetTitle (char *Title)
{
    return ECTL (ECTL_SETTITLE, (void *) Title);
}

inline int FarEd::ReadInput (INPUT_RECORD *pRec)
{
    return ECTL (ECTL_PROCESSINPUT, (void *) pRec);
}

inline int FarEd::ProcessInput (INPUT_RECORD *pRec)
{
    return ECTL (ECTL_PROCESSINPUT, (void *) pRec);
}

inline int FarEd::GetString (int StringNumber, EditorGetString *egs)
{
    egs->StringNumber = StringNumber;
    return ECTL (ECTL_GETSTRING, (void *) &egs);
}

inline int FarEd::GetCurStringText (char *StringText, int MaxLength)
{
    return GetStringText (-1, StringText, MaxLength);
}

inline int FarEd::GetCurStringLength()
{
    return GetStringLength (-1);
}

#endif
