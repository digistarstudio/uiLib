

#include "stdafx.h"
#include "uiDrawer.h"


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
	m_RenderDestRect.IntersectWith(m_RenderUpdateRect);

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


void uiWndDrawer::Begin(void *pCtx)
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

	m_RenderDestRect.Init(m_nWidth, m_nHeight);
	m_PaintDC = BeginPaint(m_hWnd, (PAINTSTRUCT*)pCtx);

/*
	// Test code.
	{
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = m_nWidth;
		rect.bottom = m_nHeight;
		::FillRect(m_MemDC, &rect, (HBRUSH)(COLOR_WINDOW + 1));

		rect.bottom = 30;
		if (DrawCaption(m_hWnd, m_MemDC, &rect, DC_ACTIVE | DC_TEXT) == 0)
			printx("DrawCaption failed!\n");
	}
	//*/
}

void uiWndDrawer::End(void *pCtx)
{
	if (m_hRgn != NULL)
	{
		SelectClipRgn(m_MemDC, NULL);
		DeleteObject(m_hRgn);
		m_hRgn = NULL;
	}

	BitBlt(m_PaintDC, 0, 0, m_nWidth, m_nHeight, m_MemDC, 0, 0, SRCCOPY);
	EndPaint(m_hWnd, (PAINTSTRUCT*)pCtx);
	m_PaintDC = NULL;
}

BOOL uiWndDrawer::InitBackBuffer(HWND hWnd, UINT nWidth, UINT nHeight)
{
	ASSERT(m_hWnd == NULL);
	ASSERT(m_MemDC == NULL && m_hBmp == NULL && m_hOldBmp == NULL);

	m_hWnd = hWnd;
	HDC hDC = GetDC(hWnd);
	ASSERT(hDC != NULL);

	m_MemDC = CreateCompatibleDC(hDC);
	m_hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
	ASSERT(m_MemDC != NULL && m_hBmp != NULL);
	m_hOldBmp = (HBITMAP)SelectObject(m_MemDC, m_hBmp);
	VERIFY(ReleaseDC(hWnd, hDC) == 1);

	m_WndDC.Attach(m_MemDC);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	return TRUE;
}

void uiWndDrawer::ResizeBackBuffer(UINT nWidth, UINT nHeight)
{
	if (m_hBmp != NULL)
	{
		SelectObject(m_MemDC, m_hOldBmp);
		::DeleteObject(m_hBmp);

		HDC hDC = GetDC(m_hWnd);
		m_hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);
		ReleaseDC(m_hWnd, hDC);

		m_hOldBmp = (HBITMAP)SelectObject(m_MemDC, m_hBmp);
	}

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	/*
	if (m_hRgn != NULL)
	{
		SelectClipRgn(m_MemDC, NULL);
		DeleteObject(m_hRgn);
	}
	m_hRgn = CreateRectRgn(0, 0, nWidth, nHeight);
	//*/
}

void uiWndDrawer::FillRect(uiRect rect, UINT32 color)
{
	rect.Move(m_OriginX, m_OriginY);

	m_WndDC.SetBrush(color, FALSE);
	m_WndDC.FillRect((RECT*)&rect);
}

void uiWndDrawer::RoundRect(uiRect rect, UINT32 color, INT width, INT height)
{
	rect.Move(m_OriginX, m_OriginY);

	m_WndDC.SetPen(1, color);
	m_WndDC.SetBrush(0, TRUE);
	m_WndDC.RoundRect((RECT*)&rect, width, height);
}

void uiWndDrawer::DrawEdge(uiRect &rect, UINT color)
{
//	HPEN hPen = CreatePen(PS_SOLID, LineWidth, color);
//	HPEN *hOldPen = (HPEN*)SelectObject(m_MemDC, hPen);

	::DrawEdge(m_MemDC, (RECT*)&rect, EDGE_ETCHED, BF_RECT | BF_ADJUST);
}

void uiWndDrawer::DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth)
{
	HPEN hPen = CreatePen(PS_SOLID, LineWidth, color);
	HPEN *hOldPen = (HPEN*)SelectObject(m_MemDC, hPen);

//	::DrawLine(x, y,


}

void uiWndDrawer::DrawText(const TCHAR *pText, const uiRect &rect, UINT flag)
{
	SetBkMode(m_MemDC, TRANSPARENT);
	SetTextColor(m_MemDC, RGB(128, 128, 128));

	HFONT hFont, hOldFont;
	// Retrieve a handle to the variable stock font.  
	hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT); // DEFAULT_GUI_FONT SYSTEM_FONT

	if (hOldFont = (HFONT)SelectObject(m_MemDC, hFont))
	{
		uiRect temp = rect;
		temp.Move(m_OriginX, m_OriginY);


		uiWndFont ftOld(hOldFont), ftSystem(hFont);
		LOGFONT old, sys;
		VERIFY(ftOld.GetLogFont(&old));
		VERIFY(ftSystem.GetLogFont(&sys));


	//	::DrawText(m_MemDC, pText, _tcslen(pText), (LPRECT)&temp, flag);
		::TextOut(m_MemDC, temp.Left, temp.Top, pText, _tcslen(pText));
		// Restore the original font.        
		SelectObject(m_MemDC, hOldFont);
	}

}


