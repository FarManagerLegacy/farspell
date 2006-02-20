/* $Header: $
   FAR+Plus: dialog classes implementation
   (C) 2001-02 Dmitry Jemerov <yole@spb.cityline.ru>
*/

#include "FARDialog.h"

// -- FarDialog --------------------------------------------------------------

FarDialog::FarDialog (const char *Title, const char *HelpTopic)
  : m_Items          (NULL),
    fHelpTopic       (HelpTopic),
    m_DefaultControl (NULL),
    m_FocusControl   (NULL),
    m_ItemsNumber    (0),
    m_BorderX        (2),
    m_BorderY        (1)
{
	m_Controls.SetOwnsItems (false);
	fSpecialItems.SetOwnsItems (false);

    new FarBoxCtrl (this, TRUE, 0, 0, 0, 0, Title);
}

FarDialog::FarDialog (int TitleLngIndex, const char *HelpTopic)
  : m_Items          (NULL),
    fHelpTopic       (HelpTopic),
    m_DefaultControl (NULL),
    m_FocusControl   (NULL),
    m_ItemsNumber    (0),
    m_BorderX        (2),
    m_BorderY        (1)
{
	m_Controls.SetOwnsItems (false);
	fSpecialItems.SetOwnsItems (false);

    AddOwnedControl (new FarBoxCtrl (this, TRUE, 0, 0, 0, 0, TitleLngIndex));
}

FarDialog::~FarDialog()
{
    if (!m_Controls.OwnsItems())
	{
		for (int i=m_Controls.Count()-1; i >= 0; i--)
		{
			if (m_Controls [i]->IsOwned())
			{
				delete m_Controls [i];
				m_Controls [i] = NULL;
			}
		}
		for (int j=fSpecialItems.Count()-1; j >= 0; j--)
		{
			if (fSpecialItems [j]->IsOwned())
			{
				delete fSpecialItems [j];
				fSpecialItems [j] = NULL;
			}
		}
	}
	
	m_Controls.Clear();
	fSpecialItems.Clear();
	if (m_ItemsNumber > 0)
        free (m_Items);
}

void FarDialog::AddControl (FarControl *pCtrl)
{
    m_ItemsNumber++;
    m_Items=(FarDialogItem *) realloc (m_Items, sizeof (FarDialogItem) * m_ItemsNumber);
	m_Controls.Add (pCtrl);
    
    for (int i=0; i<m_ItemsNumber; i++)
        m_Controls [i]->fItem = &m_Items [i];
}

void FarDialog::AddSpecialItem (FarDlgItem *pItem)
{
	fSpecialItems.Add (pItem);
}

void FarDialog::AddOwnedControl (FarDlgItem *pCtrl)
{
	pCtrl->SetOwned();
}

int FarDialog::FindControl (FarControl *pCtrl)
{
	return m_Controls.IndexOf (pCtrl);
}

void FarDialog::AddText (const char *Text)
{
	AddOwnedControl (new FarTextCtrl (this, Text));
}

void FarDialog::AddText (int LngIndex)
{
	AddText (Far::GetMsg (LngIndex));
}

void FarDialog::AddSeparator()
{
	AddOwnedControl (new FarSeparatorCtrl (this));
}

void FarDialog::Layout()
{
    // The core loop. Automatic control layout, default/focus management
    // and so on.
    
    unsigned int LastY=m_BorderY;
    unsigned int MaxX2=0;
	unsigned int MaxY2 = 0;
    
    // m_Controls [0] is the dialog frame. It is handled specially.
    for (int i=1; i<m_ItemsNumber; i++)
    {
        FarDialogItem *pCurItem=m_Controls [i]->fItem;
        
        // Layout
        if (pCurItem->Y1 == -1)
        {
            if (pCurItem->X1 == -1) // next Y, default X
            {
                pCurItem->Y1 = ++LastY;
                if (!(pCurItem->Flags & DIF_SEPARATOR))
                {
                    pCurItem->X1 = m_BorderX+3;
                    if (pCurItem->X2)
                        pCurItem->X2 += pCurItem->X1-1;
                }
            } 
            else if (pCurItem->X1 == DIF_CENTERGROUP) // special case for buttons
            { 
                pCurItem->Flags |= DIF_CENTERGROUP;
                FarDialogItem *pPrevItem=m_Controls [i-1]->fItem;
                if (pPrevItem->Type == DI_BUTTON)
                {
                    pCurItem->Y1 = pPrevItem->Y1;
                    pPrevItem->Flags |= DIF_CENTERGROUP;
                }
                else {
                    pCurItem->Y1 = ++LastY;
                }
                pCurItem->X1 = 0; // not important for centered groups
            }
            else {
                if (m_Controls [i]->m_flagsPlus & FCF_NEXTY)
                    pCurItem->Y1 = ++LastY;
                else
                    pCurItem->Y1 = LastY;
                pCurItem->X1 += m_BorderX+3;
                if (pCurItem->X2)
                    pCurItem->X2 += pCurItem->X1-1;
            }
			if (pCurItem->Type == DI_SINGLEBOX || pCurItem->Type == DI_DOUBLEBOX)
			{
				// convert height to bottom Y
				pCurItem->Y2 += pCurItem->Y1;
				MaxY2 = pCurItem->Y2;
			}
        }
        
        // Max width determination
        if (pCurItem->X2 > (int) MaxX2)
            MaxX2 = pCurItem->X2;
        else
        {
            int Width=0;
            if (pCurItem->Type == DI_TEXT && pCurItem->Data != NULL)
            {
                Width = strlen (pCurItem->Data);
            }
            else if (pCurItem->Type == DI_CHECKBOX || pCurItem->Type == DI_RADIOBUTTON)
                Width = strlen (pCurItem->Data)+4;
            
            int curX = pCurItem->X1;
            if (curX == -1 && pCurItem->Flags & DIF_SEPARATOR && pCurItem->Data != NULL)
            {
                curX = 3;
                Width++;
            }
            if (Width + curX > (int) MaxX2)
                MaxX2 = Width + curX;
        }
        
        // Default
        if (m_Controls [i] == m_DefaultControl)
            pCurItem->DefaultButton = TRUE;
        else if (m_DefaultControl == NULL && pCurItem->Type == DI_BUTTON)
        {
            m_DefaultControl = m_Controls [i];
            pCurItem->DefaultButton = TRUE;
        }
        else
            pCurItem->DefaultButton = FALSE;
        
    }
	UpdateFocus();
    
    // Set the size of the dialog frame
	if (MaxY2 > LastY)
		LastY = MaxY2;
    m_Items [0].X1 = m_BorderX+1;
    m_Items [0].Y1 = m_BorderY;
    m_Items [0].X2 = m_Items [0].X1+MaxX2-1;
    m_Items [0].Y2 = LastY+1;
}

void FarDialog::UpdateFocus()
{
    for (int i=1; i<m_ItemsNumber; i++)
    {
		FarDialogItem *pCurItem=m_Controls [i]->fItem;
        if (m_Controls [i] == m_FocusControl)
            pCurItem->Focus = TRUE;
        else if (m_FocusControl == NULL &&
            pCurItem->Type != DI_SINGLEBOX && pCurItem->Type != DI_DOUBLEBOX &&
            pCurItem->Type != DI_TEXT && pCurItem->Type != DI_VTEXT)
        {
            m_FocusControl = m_Controls [i];
            pCurItem->Focus = TRUE;
        }
        else
            pCurItem->Focus = FALSE;
	}
}

FarControl *FarDialog::Show (bool SkipLayout)
{
    if (!SkipLayout)
        Layout();

    // Show dialog and validate data
    BOOL bValidData;
    int ExitCode;
    do {
        bValidData = TRUE;
        ExitCode=Far::m_Info.Dialog (Far::m_Info.ModuleNumber,
            -1, -1, m_Items [0].X2+m_BorderX+2, m_Items [0].Y2+m_BorderY+1,
            fHelpTopic,
            m_Items,
            m_ItemsNumber);
        if (ExitCode == -1)
            return NULL;
        if (!m_Controls [ExitCode]->m_flagsPlus & FCF_VALIDATE)
            break;

        for (int i=0; i<m_ItemsNumber; i++)
            if (!m_Controls [i]->Validate())
            {
				SetFocusControl (m_Controls [i]);
				UpdateFocus();
                bValidData = FALSE;
                break;
            }
    } while (!bValidData);

    return m_Controls [ExitCode];
}

// -- FarControl -------------------------------------------------------------

FarControl::FarControl (FarDialog *pDlg, unsigned char Type)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
}

FarControl::FarControl (FarDialog *pDlg, unsigned char Type, const char *Text)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
    lstrcpyn (fItem->Data, Text, sizeof (fItem->Data)-1);
}

FarControl::FarControl (FarDialog *pDlg, unsigned char Type, int LngIndex)
  : FarDlgItem  (pDlg),
    m_flagsPlus (0)
{
    InitControl (Type);
    lstrcpyn (fItem->Data, GetMsg (LngIndex), sizeof (fItem->Data)-1);
}

void FarControl::InitControl (unsigned char Type)
{
    fDlg->AddControl (this);
    memset (fItem, 0, sizeof (FarDialogItem));
    fItem->Type = Type;
    fItem->X1 = -1;
    fItem->Y1 = -1;
}

BOOL FarControl::Validate()
{
    return TRUE;
}

void FarControl::SetFlags (DWORD flags)
{
    fItem->Flags |= flags;
}

// -- FarTextCtrl ------------------------------------------------------------

FarTextCtrl::FarTextCtrl (FarDialog *pDlg, const char *Text, int X)
  : FarControl (pDlg, DI_TEXT, Text)
{
    fItem->X1 = X;
}

FarTextCtrl::FarTextCtrl (FarDialog *pDlg, int LngIndex, int X)
  : FarControl (pDlg, DI_TEXT, LngIndex)
{
    fItem->X1 = X;
}

// -- FarEditCtrl ------------------------------------------------------------

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width, 
						  const char *History)
  : FarControl    (pDlg, DI_EDIT, Text),
    fValidateFunc (NULL),
    fIntValuePtr  (NULL),
	fStringPtr    (NULL)
{
	fItem->X1 = X;
    fItem->X2 = Width;
    if (History)
    {
        fItem->Flags |= DIF_HISTORY;
        fItem->Selected = (int) History;
    }
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg,FarString *stringPtr, int X, int Width,
						  const char *History)
	: FarControl    (pDlg, DI_EDIT, *stringPtr),
	  fValidateFunc	(NULL),
	  fIntValuePtr  (NULL),
	  fStringPtr    (stringPtr)
{
    fItem->X1 = X;
    fItem->X2 = Width;
    if (History)
    {
        fItem->Flags |= DIF_HISTORY;
        fItem->Selected = (int) History;
    }
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, int iValue, int X, int Width)
    : FarControl    (pDlg, DI_EDIT, ""),
      fValidateFunc (NULL),
      fIntValuePtr  (NULL),
	  fStringPtr    (NULL)
{
    fItem->X1 = X;
    fItem->X2 = Width;

    itoa (iValue, fItem->Data, 10);
}

FarEditCtrl::FarEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width)
    : FarControl    (pDlg, DI_EDIT, ""),
      fValidateFunc (NULL),
      fIntValuePtr  (pIntValue),
	  fStringPtr    (NULL)
{
    fItem->X1 = X;
    fItem->X2 = Width;
    
    itoa (*pIntValue, fItem->Data, 10);
}

void FarEditCtrl::GetText (char *Text, int MaxLength)
{
    lstrcpyn (Text, fItem->Data, MaxLength-1);
}

FarString FarEditCtrl::GetText()
{
	return fItem->Data;
}

int FarEditCtrl::GetIntValue()
{
    return FarSF::atoi (fItem->Data);
}

void FarEditCtrl::SetText (const char *NewText)
{
    strcpy (fItem->Data, NewText);
}

BOOL FarEditCtrl::Validate()
{
    BOOL ret = TRUE;
    if (fValidateFunc != NULL)
        ret = fValidateFunc (fItem->Data, fValidateUserParam);

    if (ret)
	{
		if (fIntValuePtr)
			*fIntValuePtr = GetIntValue();
		if (fStringPtr)
			*fStringPtr = fItem->Data;
	}
        
    return ret;
}

// -- FarPswEditCtrl ---------------------------------------------------------

FarPswEditCtrl::FarPswEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width)
  : FarEditCtrl (pDlg, Text, X, Width, NULL)
{
    fItem->Type = DI_PSWEDIT;
}

// -- FarFixEditCtrl ---------------------------------------------------------

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, const char *Text, int X, int Width, 
                                const char *History /* = NULL */)
    :  FarEditCtrl (pDlg, Text, X, Width, History)
{
    fItem->Type = DI_FIXEDIT;
}

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, int iValue, int X, int Width)
    : FarEditCtrl (pDlg, iValue, X, Width)
{
    fItem->Type = DI_FIXEDIT;
}

FarFixEditCtrl::FarFixEditCtrl (FarDialog *pDlg, int *pIntValue, int X, int Width)
    : FarEditCtrl (pDlg, pIntValue, X, Width)
{
    fItem->Type = DI_FIXEDIT;
}

void FarFixEditCtrl::SetMask (const char *Mask)
{
    fItem->Mask = const_cast<char *> (Mask);
    fItem->Flags |= DIF_MASKEDIT;
}

// -- FarSeparatorCtrl -------------------------------------------------------

FarSeparatorCtrl::FarSeparatorCtrl (FarDialog *pDlg)
  : FarControl (pDlg, DI_TEXT)
{
    fItem->Flags |= DIF_SEPARATOR;
}

FarSeparatorCtrl::FarSeparatorCtrl (FarDialog *pDlg, const char *Text)
  : FarControl (pDlg, DI_TEXT, Text)
{
    fItem->Flags |= DIF_SEPARATOR;
}


FarSeparatorCtrl::FarSeparatorCtrl (FarDialog *pDlg, int LngIndex)
  : FarControl (pDlg, DI_TEXT, LngIndex)
{
    fItem->Flags |= DIF_SEPARATOR;
}


// -- FarCheckCtrl -----------------------------------------------------------

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, const char *Text, bool Selected)
    : FarControl  (pDlg, DI_CHECKBOX, Text), 
      m_pSelected (NULL)
{
    fItem->Selected = Selected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, int LngIndex, bool Selected)
    : FarControl  (pDlg, DI_CHECKBOX, LngIndex), 
      m_pSelected (NULL)
{
    fItem->Selected = Selected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, const char *Text, bool *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, Text),  
      m_pSelected (pSelected)
{
    fItem->Selected = *pSelected;
}

FarCheckCtrl::FarCheckCtrl (FarDialog *pDlg, int LngIndex, bool *pSelected)
    : FarControl  (pDlg, DI_CHECKBOX, LngIndex),
      m_pSelected (pSelected)
{
    fItem->Selected = *pSelected;
}

BOOL FarCheckCtrl::Validate()
{
    if (m_pSelected)
        *m_pSelected = GetSelected() ? true : false;
    return TRUE;
}

// -- FarRadioGroup ----------------------------------------------------------

FarRadioCtrl *FarRadioGroup::AddItem (const char *Text)
{
	int index = fItems.Count();
	FarRadioCtrl *pCtrl = new FarRadioCtrl (fDlg, Text);
	pCtrl->SetOwned();     // the dialog will own it
	fItems.Add (pCtrl);
	if (index == fSelectedIndex)
		pCtrl->fItem->Selected = 1;
	if (index == 0)
		pCtrl->fItem->Flags |= DIF_GROUP;
	return pCtrl;
}

FarRadioCtrl *FarRadioGroup::AddItem (int LngIndex)
{
	return AddItem (Far::GetMsg (LngIndex));
}

int FarRadioGroup::GetSelectedIndex()
{
	for (int i=0; i<fItems.Count(); i++)
		if (fItems [i]->fItem->Selected)
			return i;
	
	return -1;
}

// -- FarBoxCtrl -------------------------------------------------------------

FarBoxCtrl::FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1, 
                        int W, int H, const char *Title)
  : FarControl (pDlg, Double ? DI_DOUBLEBOX : DI_SINGLEBOX, Title)
{
    fItem->X1 = X1;
    fItem->Y1 = Y1;
    fItem->X2 = W;
    fItem->Y2 = H;
}

FarBoxCtrl::FarBoxCtrl (FarDialog *pDlg, BOOL Double, int X1, int Y1, 
                        int W, int H, int LngIndex)
  : FarControl (pDlg, Double ? DI_DOUBLEBOX : DI_SINGLEBOX, LngIndex)
{
    fItem->X1 = X1;
    fItem->Y1 = Y1;
    fItem->X2 = W;
    fItem->Y2 = H;
}

// -- FarButtonCtrl ----------------------------------------------------------

FarButtonCtrl::FarButtonCtrl (FarDialog *pDlg, const char *Text, int X)
  : FarControl (pDlg, DI_BUTTON, Text)
{
    fItem->X1 = X;
}

FarButtonCtrl::FarButtonCtrl (FarDialog *pDlg, int LngIndex, int X)
  : FarControl (pDlg, DI_BUTTON, LngIndex)
{
    fItem->X1 = X;
}

