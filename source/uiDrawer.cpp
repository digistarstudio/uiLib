

#include "stdafx.h"
#include "uiDrawer.h"


std::unordered_map<UINT, std::weak_ptr<stHandleWrapper<winCursorIconHandleType>>> winCursorIconHandleType::HandleMap;
//std::unordered_map<UINT, std::weak_ptr<stHandleWrapper<winFontHandleType>>> winFontHandleType::HandleMap;
std::unordered_map<UINT, std::weak_ptr<stHandleWrapper<winIconHandleType>>> winIconHandleType::HandleMap;

IMPLEMENT_WIN_HANDLE_TYPE(winFontHandleType)


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


