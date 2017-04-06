

#include "stdafx.h"
#include "uiCommon.h"
#include "MsWinHelper.h"
#include "Resource.h"


HCURSOR WINAPI fnLoader(HCURSOR, LPCWSTR, DWORD, DWORD*, DWORD*);

typedef HCURSOR(WINAPI* GET_CURSOR_FRAME_INFO)(HCURSOR, LPCWSTR, DWORD, DWORD*, DWORD*);
GET_CURSOR_FRAME_INFO fnGetCursorFrameInfo = fnLoader;


HCURSOR WINAPI fnLoader(HCURSOR hCursor, LPCWSTR str, DWORD len, DWORD* displayRate, DWORD* totalFrames)
{
	HCURSOR hRet = NULL;
	HMODULE libUser32 = LoadLibraryA("user32.dll");
	if (!libUser32)
		return NULL;

	fnGetCursorFrameInfo = reinterpret_cast<GET_CURSOR_FRAME_INFO>(GetProcAddress(libUser32, "GetCursorFrameInfo"));
	ASSERT(fnGetCursorFrameInfo != NULL);
	if (fnGetCursorFrameInfo != NULL)
	{
		return fnGetCursorFrameInfo(hCursor, str, len, displayRate, totalFrames);
	}

//	FreeLibrary(libUser32); // This's necessary lib for windows app.
	return hRet;
}

BOOL GetCursorFrameInfo(HCURSOR hCursor, DWORD& DispRate, DWORD& TotalFrame)
{
	return fnGetCursorFrameInfo(hCursor, _T(""), 0, &DispRate, &TotalFrame) != NULL;
}


BOOL SaveFile(const TCHAR* pFileName, void *pData, UINT nSize)
{
	DWORD dwSizeWritten;
	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL);
	if (hFile != NULL)
	{
		::WriteFile(hFile, pData, nSize, &dwSizeWritten, nullptr);
		::CloseHandle(hFile);
		ASSERT(dwSizeWritten == nSize);
		return TRUE;
	}
	return FALSE;
}

BOOL SaveResourceToTempFile(uiString& FullPath, const TCHAR* SaveName, UINT ResID, const TCHAR* ResType, HINSTANCE hModule)
{
	ASSERT(ResType != nullptr);
	ASSERT(hModule != NULL);

	HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(ResID), ResType); // No need to close handle of HRSRC and fake HGLOBAL.
	if (hRes == NULL)
		printx("FindResource failed! ec: %d\n", GetLastError());
	else
	{
		HGLOBAL hMem = LoadResource(hModule, hRes);
		if (hMem == NULL)
			printx("LoadResource failed! ec: %d\n", GetLastError());
		else
		{
			PBYTE pAddr = (PBYTE)LockResource(hMem);
			DWORD dwSize = SizeofResource(hModule, hRes);

			TCHAR buf[MAX_PATH + 1];
			INT len = GetTempPath(_countof(buf), buf);
			if (buf[len - 1] != '\\')
			{
				buf[len - 1] = '\\';
				buf[len] = '\0';
			}
			_tcscat_s(buf, _countof(buf), SaveName);

			if (SaveFile(buf, pAddr, dwSize))
			{
				FullPath = buf;
				return TRUE;
			}
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		printx("---> DialogProc:: WM_INITDIALOG (ID: %p)\n", hDlg);
		return FALSE;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
			EndDialog(hDlg, 0);
		break;
	}

	/*
	stHandlerRetInfo rInfo;
	uiMsgProc(rInfo, hWnd, message, wParam, lParam);
	if (rInfo.bProcessed)
	return rInfo.ret;
	*/

	return FALSE;
}

LPWORD lpwAlign(LPWORD lpIn)
{
	ULONG ul;
	ul = (ULONG)lpIn;
	ul++;
	ul >>= 1;
	ul <<= 1;
	return (LPWORD)ul;
}

#define ID_HELP   150
#define ID_TEXT   200




LRESULT CreateDialogHelper(HINSTANCE hIns, HWND hParent)
{
	HGLOBAL hMem;
	LPDLGTEMPLATE lpdt;
	LPDLGITEMTEMPLATE lpdit;
	LPWORD lpw;
	LPWSTR lpwsz;
	LRESULT ret;
//	HWND hRet;
	int nchar;

	hMem = GlobalAlloc(GMEM_ZEROINIT, 1024);
	if (!hMem)
		return -1;

	lpdt = (LPDLGTEMPLATE)GlobalLock(hMem);

	// Define a dialog box.

	lpdt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION;
	lpdt->cdit = 3;         // Number of controls
	lpdt->x = 10;  lpdt->y = 10;
	lpdt->cx = 100; lpdt->cy = 100;

	lpw = (LPWORD)(lpdt + 1);
	*lpw++ = 0;             // No menu
	*lpw++ = 0;             // Predefined dialog box class (by default)

	lpwsz = (LPWSTR)lpw;
	nchar = 1 + MultiByteToWideChar(CP_ACP, 0, "My Dialog", -1, lpwsz, 50);
	lpw += nchar;

	//-----------------------
	// Define an OK button.
	//-----------------------
	lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
	lpdit = (LPDLGITEMTEMPLATE)lpw;
	lpdit->x = 10; lpdit->y = 70;
	lpdit->cx = 80; lpdit->cy = 20;
	lpdit->id = IDOK;       // OK button identifier
	lpdit->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;

	lpw = (LPWORD)(lpdit + 1);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0080;        // Button class

	lpwsz = (LPWSTR)lpw;
	nchar = 1 + MultiByteToWideChar(CP_ACP, 0, "OK", -1, lpwsz, 50);
	lpw += nchar;
	*lpw++ = 0;             // No creation data

	//-----------------------
	// Define a Help button.
	//-----------------------
	lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
	lpdit = (LPDLGITEMTEMPLATE)lpw;
	lpdit->x = 55; lpdit->y = 10;
	lpdit->cx = 40; lpdit->cy = 20;
	lpdit->id = ID_HELP;    // Help button identifier
	lpdit->style = WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON;

	lpw = (LPWORD)(lpdit + 1);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0080;        // Button class atom

	lpwsz = (LPWSTR)lpw;
	nchar = 1 + MultiByteToWideChar(CP_ACP, 0, "Help", -1, lpwsz, 50);
	lpw += nchar;
	*lpw++ = 0;             // No creation data

	//-----------------------
	// Define a static text control.
	//-----------------------
	lpw = lpwAlign(lpw);    // Align DLGITEMTEMPLATE on DWORD boundary
	lpdit = (LPDLGITEMTEMPLATE)lpw;
	lpdit->x = 10; lpdit->y = 10;
	lpdit->cx = 40; lpdit->cy = 20;
	lpdit->id = ID_TEXT;    // Text identifier
	lpdit->style = WS_CHILD | WS_VISIBLE | SS_LEFT;

	lpw = (LPWORD)(lpdit + 1);
	*lpw++ = 0xFFFF;
	*lpw++ = 0x0082;        // Static class

	WCHAR buff[] = L"message!";
	WCHAR *lpszMessage = buff;
	for (lpwsz = (LPWSTR)lpw; *lpwsz++ = (WCHAR)*lpszMessage++;);
	lpw = (LPWORD)lpwsz;
	*lpw++ = 0;             // No creation data

	GlobalUnlock(hMem);
	ret = DialogBoxIndirect(hIns, (LPDLGTEMPLATE)hMem, hParent, (DLGPROC)DialogProc); // Modal
	GlobalFree(hMem);

	return ret;
}


