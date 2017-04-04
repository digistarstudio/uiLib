


#pragma once


BOOL GetCursorFrameInfo(HCURSOR hCursor, DWORD& DispRate, DWORD& TotalFrame);
BOOL SaveResourceToTempFile(uiString& FullPath, const TCHAR* SaveName, UINT ResID, const TCHAR* ResType, HINSTANCE hModule);


class uiWndFont
{
public:

	uiWndFont(HFONT hFont) :m_hFont(hFont) {}
	~uiWndFont() {}

	BOOL GetLogFont(LOGFONT& lf) { return ::GetObject(m_hFont, sizeof(lf), &lf); }

protected:

	HFONT m_hFont;


};


