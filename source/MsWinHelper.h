


#pragma once


BOOL GetCursorFrameInfo(HCURSOR hCursor, DWORD& DispRate, DWORD& TotalFrame);
BOOL SaveResourceToTempFile(uiString& FullPath, const TCHAR* SaveName, UINT ResID, const TCHAR* ResType, HINSTANCE hModule);


