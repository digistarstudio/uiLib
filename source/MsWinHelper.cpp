

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


