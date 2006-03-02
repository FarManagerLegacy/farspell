// -- FarMenu[Ex] -------------------------------------------------------------

template <typename TItems>
class FarMenuT
{
private:
	FarMenuT (const FarMenuT &rhs);
	const FarMenuT &operator= (const FarMenuT &rhs);

protected:
    FarString fTitle;
    unsigned int fFlags;
    FarString fHelpTopic;
    FarString fBottom;
    int *fBreakKeys;
    int fBreakCode;
    int fX, fY;
	int fMaxHeight;
    TItems *fItems;
    int fItemsNumber;
    bool fOwnsBreakKeys;
    int InternalAddItem (const char *Text, int Selected, int Checked, int Separator);

public:
    FarMenuT (const char *TitleText, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    FarMenuT (int TitleLngIndex, unsigned int Flags=FMENU_WRAPMODE,
        const char *HelpTopic=NULL);
    ~FarMenuT();

    void SetLocation (int X, int Y)
		{ fX = X; fY = Y; }
    void SetMaxHeight (int MaxHeight)
		{ fMaxHeight = MaxHeight; }
    void SetBottomLine (const FarString &Text)
		{ fBottom = Text; }
    void SetBottomLine (int LngIndex)
		{ fBottom = Far::GetMsg (LngIndex); }
    void SetBreakKeys (int *BreakKeys)
		{ fOwnsBreakKeys = false; fBreakKeys = BreakKeys; }
    void SetBreakKeys (int aFirstKey, ...);

    // returns index of new item
    int AddItem (const char *Text, bool Selected=false, int Checked=0);
    int AddItem (int LngIndex, bool Selected=false, int Checked=0);
    int AddSeparator();
    void DisableItem (int index);

    void ClearItems();
    void SelectItem (int index);
    // returns index of selected item
    int Show();
    int GetBreakCode() const
		{ return fBreakCode; }
};

typedef FarMenuT<FarMenuItem> FarMenu;
typedef FarMenuT<FarMenuItemEx> FarMenuEx;

/*class FarMenuEx
: public FarMenuT<FarMenuItemEx>
{
  public:
    void DisableItem (int index);
};*/
