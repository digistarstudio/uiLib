


#pragma once


#pragma pack(push, 1) 


typedef struct _DLGTEMPLATEEX
{
	WORD   dlgVer;
	WORD   signature;
	DWORD  helpID;
	DWORD  exStyle;
	DWORD  style;
	WORD   cDlgItems;
	short  x;
	short  y;
	short  cx;
	short  cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;

typedef struct _DLGITEMTEMPLATEEX
{
	DWORD  helpID;
	DWORD  exStyle;
	DWORD  style;
	short  x;
	short  y;
	short  cx;
	short  cy;
	WORD   id;
} DLGITEMTEMPLATEEX, *LPDLGITEMTEMPLATEEX;


struct DLGTEMPLATEEX2
{
	DLGTEMPLATEEX2()
	{
		ZeroMemory(this, sizeof(*this));
		dlgVer = 1;
		signature = 0xFFFF;

	//	exStyle = ;
	}

	WORD   dlgVer;
	WORD   signature;
	DWORD  helpID;
	DWORD  exStyle;
	DWORD  style;
	WORD   cDlgItems;
	short  x;
	short  y;
	short  cx;
	short  cy;

	WCHAR menu;
	WCHAR windowClass;
	WCHAR title;
	WORD  pointsize;
	WORD  weight;
	BYTE  italic;
	BYTE  charset;
	WCHAR typeface;
};

#pragma pack(pop)


BOOL GetCursorFrameInfo(HCURSOR hCursor, DWORD& DispRate, DWORD& TotalFrame);
BOOL SaveResourceToTempFile(uiString& FullPath, const TCHAR* SaveName, UINT ResID, const TCHAR* ResType, HINSTANCE hModule);

LRESULT CreateDialogHelper(HINSTANCE hIns, HWND hParent);


class uiWndFont
{
public:

	uiWndFont(HFONT hFont) :m_hFont(hFont) {}
	~uiWndFont() {}

	BOOL GetLogFont(LOGFONT& lf) { return ::GetObject(m_hFont, sizeof(lf), &lf); }

protected:

	HFONT m_hFont;


};


