/* $Header: $
   FAR+Plus: FAR editor API wrappers implementation
   (C) 2001 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#include "FAREd.h"

// -- FarEd ------------------------------------------------------------------

int FarEd::GetStringText (int StringNumber, char *StringText, int MaxLength)
{
    EditorGetString egs;
    egs.StringNumber = StringNumber;
    if (!ECTL (ECTL_GETSTRING, &egs)) return FALSE;
    int Len=(MaxLength-1 < egs.StringLength) ? MaxLength-1 : egs.StringLength;
    memcpy (StringText, egs.StringText, Len);
    StringText [MaxLength-1] = 0;
    return TRUE;
}

int FarEd::GetStringLength (int StringNumber)
{
    EditorGetString egs;
    egs.StringNumber = StringNumber;
    if (!ECTL (ECTL_GETSTRING, &egs)) return -1;
    return egs.StringLength-1;
}

int FarEd::GetStringSel (int StringNumber, int *SelStart, int *SelEnd)
{
    EditorGetString egs;
    egs.StringNumber = StringNumber;
    if (!ECTL (ECTL_GETSTRING, &egs)) return FALSE;
    if (SelStart) *SelStart = egs.SelStart;
    if (SelEnd) *SelEnd = egs.SelEnd;
    return TRUE;
}

int FarEd::SetPos (int CurLine, int CurPos)
{
    EditorSetPosition esp;
    memset (&esp, 0xFF, sizeof (esp));   // initialize members with -1
    esp.CurLine = CurLine;
    esp.CurPos = CurPos;
    return ECTL (ECTL_SETPOSITION, &esp);
}

int FarEd::SetTabPos (int CurLine, int CurTabPos)
{
    EditorSetPosition esp;
    memset (&esp, 0xFF, sizeof (esp));   // initialize members with -1
    esp.CurLine = CurLine;
    esp.CurTabPos = CurTabPos;
    return ECTL (ECTL_SETPOSITION, &esp);
}

int FarEd::SetViewPos (int TopScreenLine, int LeftPos)
{
    EditorSetPosition esp;
    memset (&esp, 0xFF, sizeof (esp));   // initialize members with -1
    esp.TopScreenLine = TopScreenLine;
    esp.LeftPos = LeftPos;
    return ECTL (ECTL_SETPOSITION, &esp);
}

int FarEd::SetOvertype (BOOL Overtype)
{
    EditorSetPosition esp;
    memset (&esp, 0xFF, sizeof (esp));   // initialize members with -1
    esp.Overtype = Overtype;
    return ECTL (ECTL_SETPOSITION, &esp);
}

int FarEd::SelectBlock (int BlockType, int BlockStartLine, int BlockStartPos,
                        int BlockWidth, int BlockHeight)
{
    EditorSelect es;
    es.BlockType = BlockType;
    es.BlockStartLine = BlockStartLine;
    es.BlockStartPos = BlockStartPos;
    es.BlockWidth = BlockWidth;
    es.BlockHeight = BlockHeight;
    return ECTL (ECTL_SELECT, &es);
}

int FarEd::UnselectBlock()
{
    EditorSelect es;
    es.BlockType = BTYPE_NONE;
    return ECTL (ECTL_SELECT, &es);
}

int FarEd::InsertString (bool Indent)
{
    int LocalIndent=Indent ? 1 : 0;
    return ECTL (ECTL_INSERTSTRING, &LocalIndent);
}

int FarEd::InsertTextString (char *Text, bool Indent)
{
    if (!InsertString (Indent))
        return FALSE;
    return InsertText (Text);
}

int FarEd::EditorToOem (char *Text, int TextLength)
{
    EditorConvertText ect;
    ect.Text = Text;
    ect.TextLength = TextLength;
    return ECTL (ECTL_EDITORTOOEM, &ect);
}

int FarEd::OemToEditor (char *Text, int TextLength)
{
    EditorConvertText ect;
    ect.Text = Text;
    ect.TextLength = TextLength;
    return ECTL (ECTL_OEMTOEDITOR, &ect);
}

int FarEd::TabToReal (int StringNumber, int SrcPos)
{
    EditorConvertPos ecp;
    ecp.StringNumber = StringNumber;
    ecp.SrcPos = SrcPos;
    if (!ECTL (ECTL_TABTOREAL, &ecp)) return -1;
    return ecp.DestPos;
}

int FarEd::RealToTab (int StringNumber, int SrcPos)
{
    EditorConvertPos ecp;
    ecp.StringNumber = StringNumber;
    ecp.SrcPos = SrcPos;
    if (!ECTL (ECTL_REALTOTAB, &ecp)) return -1;
    return ecp.DestPos;
}

int FarEd::AddColor (int StringNumber, int StartPos, int EndPos, int Color)
{
    EditorColor ec;
    ec.StringNumber = StringNumber;
    ec.StartPos = StartPos;
    ec.EndPos = EndPos;
    ec.Color = Color;
    return ECTL (ECTL_ADDCOLOR, &ec);
}

int FarEd::DeleteColor (int StringNumber, int StartPos)
{
    EditorColor ec;
    ec.StringNumber = StringNumber;
    ec.StartPos = StartPos;
    ec.Color = 0;
    return ECTL (ECTL_ADDCOLOR, &ec);
}

int FarEd::GetColor (int StringNumber, int ColorItem, 
                     int *StartPos, int *EndPos, int *Color)
{
    EditorColor ec;
    ec.StringNumber = StringNumber;
    ec.ColorItem = ColorItem;
    if (!ECTL (ECTL_GETCOLOR, &ec)) return 0;
    if (StartPos) *StartPos=ec.StartPos;
    if (EndPos) *EndPos=ec.EndPos;
    if (Color) *Color=ec.Color;
    return 1;
}

// -- FarEdInfo --------------------------------------------------------------

FarEdInfo::FarEdInfo()
{
    EditorInfo ei;
    if (FarEd::ECTL (ECTL_GETINFO, &ei)) 
	{
		EditorID       = ei.EditorID;
		FileName       = ei.FileName;
		WindowSizeX    = ei.WindowSizeX;
		WindowSizeY    = ei.WindowSizeY;
		TotalLines     = ei.TotalLines;
		CurLine        = ei.CurLine;
		CurPos         = ei.CurPos;
		CurTabPos      = ei.CurTabPos;
		TopScreenLine  = ei.TopScreenLine;
		LeftPos        = ei.LeftPos;
		Overtype       = ei.Overtype;
		BlockType      = ei.BlockType;
		BlockStartLine = ei.BlockStartLine;
		AnsiMode       = ei.AnsiMode;
		TableNum       = ei.TableNum;
		Options        = ei.Options;
		TabSize        = ei.TabSize;
	}
}

