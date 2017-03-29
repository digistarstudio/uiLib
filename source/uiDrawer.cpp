

#include "stdafx.h"
#include "uiDrawer.h"


IMPLEMENT_WIN_HANDLE_TYPE(winFontHandleType)


stImgHandleWrapper::Functor stImgHandleWrapper::m_Del[WIT_TOTAL] = { DeleteObject, DestroyCursor, DestroyIcon };
stImgHandleWrapper::PathMap stImgHandleWrapper::PathHandleMap;
stImgHandleWrapper::KeyMap  stImgHandleWrapper::KeyHandleMap;


UINT32 uiGetSysColor(INT index)
{
	UINT32 color = GetSysColor(COLOR_ACTIVECAPTION);

	switch (index)
	{
		case SCN_CAPTAIN:
		//	color = RGB();
			break;

		case SCN_FRAME:
		//	color = RGB();
			break;
	}

	return color;
}


BOOL uiFont::Create(const TCHAR* pName, INT height, INT width)
{
	uiString str(pName);
	str.MakeLower();
	if (str.Length() >= _countof(LOGFONT::lfFaceName)) // LF_FACESIZE
	{
		ASSERT(0);
		return FALSE;
	}

	LOGFONT lg = { 0 };
	lg.lfWidth = width;
	lg.lfHeight = height;
	str.CopyTo(lg.lfFaceName, _countof(lg.lfFaceName));

	size_t key = HashState(&lg);
	auto it = winFontHandleType::HandleMap.find(key);
	if (it != winFontHandleType::HandleMap.end())
	{
		if ((m_ptrHandle = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HANDLE hFont = CreateFontIndirect(&lg);
	if (hFont == NULL)
	{
		printx("CreateFontIndirect failed! ec: %d\n", GetLastError());
		ASSERT(0);
		return FALSE;
	}

	m_ptrHandle = std::shared_ptr<FontHandleW>(new FontHandleW(hFont, key));
	if (m_ptrHandle.get() == nullptr)
	{
		VERIFY(DeleteObject(hFont));
		return FALSE;
	}

	winFontHandleType::HandleMap.insert({ key, m_ptrHandle }); // Don't save returned iterator.

	return TRUE;
}


BOOL uiImage::LoadCursor(uiString str)
{
	str.MakeLower();

	auto it = stImgHandleWrapper::PathHandleMap.find(str);
	if (it != stImgHandleWrapper::PathHandleMap.end())
	{
		if ((m_pImg = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HCURSOR hCursor = (HCURSOR)LoadImage(NULL, str, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	if (hCursor == NULL)
	{
		printx("LoadImage (Cursor) failed! ec: %d\n", GetLastError());
		return FALSE;
	}

	m_pImg = std::shared_ptr<stImgHandleWrapper>(new stImgHandleWrapper(WIT_CURSOR, hCursor));
	if (m_pImg.get() == nullptr)
	{
		VERIFY(stImgHandleWrapper::m_Del[WIT_CURSOR](hCursor));
		return FALSE;
	}

	it = stImgHandleWrapper::PathHandleMap.insert({ str, m_pImg }).first;
	m_pImg->Set(it, str);

	return TRUE;
}

BOOL uiImage::LoadCursor(UINT ResID, const TCHAR* ResType, HINSTANCE hModule)
{
	struct stImageSourceInfo
	{
		HINSTANCE hModule;
		UINT      ResID;
	};

	stImageSourceInfo isi = { (hModule == NULL) ? GetModuleHandle(NULL) : hModule, ResID };
	const size_t Key = HashState(&isi);

	auto it = stImgHandleWrapper::KeyHandleMap.find(Key);
	if (it != stImgHandleWrapper::KeyHandleMap.end())
	{
		if ((m_pImg = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	// No need to close handle of HRSRC and fake HGLOBAL.
	HCURSOR hCursor;
	if (ResType != nullptr)
	{
		TCHAR path[MAX_PATH + 1];
		HRSRC hRes = FindResource(isi.hModule, MAKEINTRESOURCE(ResID), ResType);
		if (hRes != NULL)
		{
			HGLOBAL hMem = LoadResource(isi.hModule, hRes);
			if (hMem != NULL)
			{
				PBYTE pAddr = (PBYTE)LockResource(hMem);
				DWORD dwSize = SizeofResource(isi.hModule, hRes);
				if (SaveToTempFile(_T("uiCursorTempFile"), path, _countof(path), pAddr, dwSize))
				{
					hCursor = (HCURSOR)LoadImage(NULL, path, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
					::DeleteFile(path);
				}
			}
			else
			{
				printx("LoadResource failed! ec: %d\n", GetLastError());
				return FALSE;
			}
		}
		else
		{
			printx("FindResource failed! ec: %d\n", GetLastError());
			return FALSE;
		}
	}
	else
	{
		hCursor = (HCURSOR)LoadImage(isi.hModule, MAKEINTRESOURCE(ResID), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR);
	}
	// It looks like these two old win32 functions are not compatiable with new animation format.
	//	HICON hCursor = CreateIconFromResourceEx(pAddr, dwSize, FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
	//	HCURSOR hCursor = ::LoadCursor(isi.hModule, MAKEINTRESOURCE(ResID));

	if (hCursor == NULL)
	{
		printx("LoadImage (Cursor) failed! ec: %d\n", GetLastError());
		return FALSE;
	}

	m_pImg = std::shared_ptr<stImgHandleWrapper>(new stImgHandleWrapper(WIT_CURSOR, hCursor, Key));
	if (m_pImg.get() == nullptr)
	{
		VERIFY(stImgHandleWrapper::m_Del[WIT_CURSOR](hCursor));
		return FALSE;
	}

	stImgHandleWrapper::KeyHandleMap.insert({ Key, m_pImg });

	return TRUE;
}

BOOL uiImage::SaveToTempFile(const TCHAR* pFileName, TCHAR buf[], UINT nMaxCharCount, void *pData, UINT nSize)
{
	INT len = GetTempPath(nMaxCharCount, buf);
	if (buf[len - 1] != '\\')
	{
		buf[len - 1] = '\\';
		buf[len] = '\0';
	}
	_tcscat_s(buf, nMaxCharCount, pFileName);
	HANDLE hFile = ::CreateFile(buf, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, NULL);
	DWORD dwSizeWritten;
	if (hFile != NULL)
	{
		::WriteFile(hFile, pData, nSize, &dwSizeWritten, nullptr);
		::CloseHandle(hFile);
		return TRUE;
	}
	return FALSE;
}


uiDrawer::uiDrawer()
:m_OriginX(0), m_OriginY(0)
{
}

uiDrawer::~uiDrawer()
{
}

BOOL uiDrawer::PushDestRect(uiRect rect)
{
	uiPoint pt = rect.GetLeftTop();
	stRenderDestInfo RDInfo;
	RDInfo.rect = m_RenderDestRect;

	rect.Move(m_OriginX, m_OriginY);
	m_RenderDestRect.IntersectWith(rect);

	if (m_RenderDestRect.IsValidRect())
	{
		RDInfo.OriginX = m_OriginX;
		RDInfo.OriginY = m_OriginY;
		m_RectList.AddDataHead(RDInfo);

		m_OriginX += pt.x;
		m_OriginY += pt.y;
		OnDestRectChanged(FALSE);

		return TRUE;
	}

	m_RenderDestRect = RDInfo.rect;

	return FALSE;
}

void uiDrawer::PopDestRect()
{
	stRenderDestInfo RDInfo;
	ListIndex index = m_RectList.GetHeadIndex();
	m_RectList.GetAndRemove(index, RDInfo);

	m_OriginX = RDInfo.OriginX;
	m_OriginY = RDInfo.OriginY;
	m_RenderDestRect = RDInfo.rect;
	OnDestRectChanged(TRUE);
}


BOOL uiWndDrawer::Begin(void *pCtx)
{
#ifdef _DEBUG
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	//ASSERT(m_nWidth == (rect.right - rect.left) && m_nHeight == (rect.bottom - rect.top));

	if (m_nWidth != rect.right - rect.left)
		printx("Warning: client rect width mismatched! %d - %d\n", m_nWidth, rect.right - rect.left);
	if (m_nHeight != rect.bottom - rect.top)
		printx("Warning: client rect height mismatched! %d - %d\n", m_nHeight, rect.bottom - rect.top);
#endif

	ASSERT(m_RectList.GetCounts() == 0);
	ASSERT(m_PaintDC == NULL);

	PAINTSTRUCT *ps = (PAINTSTRUCT*)pCtx;
	if ((m_PaintDC = BeginPaint(m_hWnd, ps)) == NULL)
		return FALSE;

	m_RenderDestRect = *(uiRect*)(&ps->rcPaint);
	if (!m_RenderDestRect.IsValidRect())
	{
		EndPaint(m_hWnd, (PAINTSTRUCT*)pCtx);
		m_PaintDC = NULL;
		return FALSE;
	}

	m_WndDrawDC.Attach((m_TotalBackBuffer == 0) ? m_PaintDC : m_BackBuffer[m_CurrentBackBufferIndex].GetMemDC());

	return TRUE;
}

void uiWndDrawer::End(void *pCtx)
{
	ASSERT(m_OriginX == 0 && m_OriginY == 0);

	if (m_hRgn != NULL)
	{
		SelectClipRgn(m_WndDrawDC.GetDC(), NULL);
		DeleteObject(m_hRgn);
		m_hRgn = NULL;
	}

	if (m_TotalBackBuffer > 0)
	{
		BitBlt(m_PaintDC, 0, 0, m_nWidth, m_nHeight, m_WndDrawDC.GetDC(), 0, 0, SRCCOPY);
		if (++m_CurrentBackBufferIndex == m_TotalBackBuffer)
			m_CurrentBackBufferIndex = 0;
	}

	m_WndDrawDC.Detach();
	EndPaint(m_hWnd, (PAINTSTRUCT*)pCtx);
	m_PaintDC = NULL;
}

BOOL uiWndDrawer::InitBackBuffer(UINT nCount, HWND hWnd, UINT nWidth, UINT nHeight)
{
	ASSERT(nCount <= MAX_BACKBUFFER_COUNT);
	ASSERT(m_hWnd == NULL);

	m_hWnd = hWnd;
	HDC hDC = GetDC(hWnd);
	ASSERT(hDC != NULL);

	m_TotalBackBuffer = nCount;
	for (; nCount; --nCount)
		m_BackBuffer[nCount - 1].Init(hDC, nWidth, nHeight);

	VERIFY(ReleaseDC(hWnd, hDC) == 1);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	return TRUE;
}

void uiWndDrawer::ResizeBackBuffer(UINT nWidth, UINT nHeight)
{
	if (m_TotalBackBuffer > 0)
	{
		HDC hDC = GetDC(m_hWnd);
		for (INT i = 0; i < m_TotalBackBuffer; ++i)
			m_BackBuffer[i].Resize(hDC, nWidth, nHeight);
		ReleaseDC(m_hWnd, hDC);
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	ASSERT(m_hRgn == NULL);
}

void uiWndDrawer::FillRect(uiRect rect, UINT32 color)
{
	//rect.Move(m_OriginX, m_OriginY);
	UpdateCoordinate(rect);

	m_WndDrawDC.SetBrush(color, FALSE);
	m_WndDrawDC.FillRect((RECT*)&rect);
}

void uiWndDrawer::RoundRect(uiRect rect, UINT32 color, INT width, INT height)
{
	//rect.Move(m_OriginX, m_OriginY);
	UpdateCoordinate(rect);

	m_WndDrawDC.SetPen(1, color);
	m_WndDrawDC.SetBrush(0, TRUE);
	m_WndDrawDC.RoundRect((RECT*)&rect, width, height);
}

void uiWndDrawer::DrawEdge(uiRect &rect, UINT color)
{
//	HPEN hPen = CreatePen(PS_SOLID, LineWidth, color);
//	HPEN *hOldPen = (HPEN*)SelectObject(m_WndDrawDC.GetDC(), hPen);

//	::DrawEdge(m_WndDrawDC.GetDC(), (RECT*)&rect, EDGE_ETCHED, BF_RECT | BF_ADJUST);
}

void uiWndDrawer::DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth)
{
//	HPEN hPen = CreatePen(PS_SOLID, LineWidth, color);
//	HPEN *hOldPen = (HPEN*)SelectObject(m_WndDrawDC.GetDC(), hPen);

//	::DrawLine(x, y,


}

void uiWndDrawer::DrawText(const TCHAR *pText, const uiRect &rect, UINT flag)
{
	uiRect temp = rect;
	UpdateCoordinate(temp);

	HDC hMemDC = m_WndDrawDC.GetDC();
	SetBkMode(hMemDC, TRANSPARENT);
	SetTextColor(hMemDC, RGB(128, 128, 128));

	HFONT hFont, hOldFont;
	// Retrieve a handle to the variable stock font.  
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // DEFAULT_GUI_FONT SYSTEM_FONT
/*
	if (hOldFont = (HFONT)SelectObject(hMemDC, hFont))
	{
		uiWndFont ftOld(hOldFont), ftSystem(hFont);
		LOGFONT old, sys;
		VERIFY(ftOld.GetLogFont(&old));
		VERIFY(ftSystem.GetLogFont(&sys));


	//	::DrawText(hMemDC, pText, _tcslen(pText), (LPRECT)&temp, flag);
		::TextOut(hMemDC, temp.Left, temp.Top, pText, _tcslen(pText));
		// Restore the original font.        
		SelectObject(hMemDC, hOldFont);
	} //*/

}


