

#pragma once


#include "uiCommon.h"


enum SYS_COLOR_NAME
{
	SCN_CAPTAIN,
	SCN_FRAME,

};


UINT32 uiGetSysColor(INT index);


class uiDrawer
{
public:

	uiDrawer();
	virtual ~uiDrawer();


	virtual void Begin(void *pCtx) = 0;
	virtual void End(void *pCtx) = 0;

	virtual void ResizeBackBuffer(UINT nWidth, UINT nHeight) {}

	virtual void FillRect(uiRect rect, UINT32 color) = 0;
	virtual void RoundRect(uiRect rect, UINT32 color, INT width, INT height) = 0;
	virtual void DrawEdge(uiRect &rect, UINT color) = 0;
	virtual void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) = 0;
	virtual void DrawText(const TCHAR *pText, const uiRect &rect, UINT flag) = 0;

	BOOL PushDestRect(uiRect rect); // Return true if rectangle region is visible.
	void PopDestRect();

	INLINE const uiRect& GetDestRect() { return m_RenderDestRect; }


protected:

	friend class uiFormBase;

	virtual void OnDestRectChanged(BOOL bRestore) {}

	struct stRenderDestInfo
	{
		INT OriginX, OriginY;
		uiRect rect;
	};

	uiRect m_RenderDestRect;
	INT m_OriginX, m_OriginY;
	TList<stRenderDestInfo> m_RectList;


};


class uiWndDC
{
public:

	uiWndDC()
	{
		m_hDC = NULL;
		m_hOldPen = NULL;
		m_hOldBrush = NULL;
		m_hOldFont = NULL;
	}
	~uiWndDC()
	{
		ASSERT(m_hDC == NULL);
	}

	BOOL Attach(HDC hDC)
	{
		ASSERT(m_hDC == NULL);
		m_hDC = hDC;
		m_hOldPen = (HPEN)::SelectObject(m_hDC, ::GetStockObject(WHITE_PEN));
		m_hOldBrush = (HBRUSH)::SelectObject(m_hDC, ::GetStockObject(WHITE_BRUSH));
		m_hOldFont = (HFONT)::GetCurrentObject(m_hDC, OBJ_FONT);

		return TRUE;
	}
	BOOL Detach()
	{
		if (m_hDC != NULL)
		{
			::SelectObject(m_hDC, m_hOldPen);
			::SelectObject(m_hDC, m_hOldBrush);
			::SelectObject(m_hDC, m_hOldFont);
			m_hDC = NULL;
			return TRUE;
		}
		return FALSE;
	}


	BOOL SetPen(INT width, UINT32 color, INT style = PS_SOLID)
	{
		HPEN hPen = ::CreatePen(style, width, color), hOldPen;
		if (hPen == NULL)
			return FALSE;
		if ((hOldPen = (HPEN)::SelectObject(m_hDC, hPen)) != m_hOldPen)
			::DeleteObject(hOldPen);
		return TRUE;
	}
	BOOL SetBrush(UINT32 color, BOOL bHollow)
	{
		HBRUSH hBrush, hOldBrush;

		if (bHollow)
			hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		else
			hBrush = CreateSolidBrush(color);
		if (hBrush == NULL)
			return FALSE;
		if ((hOldBrush = (HBRUSH)::SelectObject(m_hDC, hBrush)) != m_hOldBrush)
			::DeleteObject(hOldBrush);
		return TRUE;
	}
	BOOL SetFont(HFONT hFont)
	{
		ASSERT(m_hDC != NULL);
		::SelectObject(m_hDC, (hFont == NULL) ? m_hOldFont : hFont);
	}

	BOOL FillRect(const RECT* pRect)
	{
		HBRUSH hBrush = (HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH);
		::FillRect(m_hDC, pRect, hBrush);
		return TRUE;
	}
	BOOL RoundRect(const RECT* pRect, INT width, INT height)
	{
		::RoundRect(m_hDC, pRect->left, pRect->top, pRect->right, pRect->bottom, width, height);
		return TRUE;
	}


protected:

	HDC    m_hDC;
	HPEN   m_hOldPen;
	HBRUSH m_hOldBrush;
	HFONT  m_hOldFont;


};


class uiWndDrawer : public uiDrawer
{
public:

	uiWndDrawer()
	:m_hWnd(NULL), m_PaintDC(NULL)
	{
		m_MemDC = NULL;
		m_hBmp = NULL;
		m_hOldBmp = NULL;
		m_hRgn = NULL;

		m_nWidth = m_nHeight = 0;
	}
	~uiWndDrawer()
	{
		m_WndDC.Detach();
		if (m_MemDC != NULL)
			::DeleteDC(m_MemDC); // Delete memory dc first then delete bitmap.
		if (m_hBmp != NULL)
			::DeleteObject(m_hBmp);

		ASSERT(m_PaintDC == NULL);
	}

	void Begin(void *pCtx);
	void End(void *pCtx);

	BOOL InitBackBuffer(HWND hWnd, UINT nWidth, UINT nHeight);
	void ResizeBackBuffer(UINT nWidth, UINT nHeight);

	void FillRect(uiRect rect, UINT32 color);
	void RoundRect(uiRect rect, UINT32 color, INT width, INT height);
	void DrawEdge(uiRect &rect, UINT color);
	void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth);
	void DrawText(const TCHAR *pText, const uiRect &rect, UINT flag);


protected:

	virtual void OnDestRectChanged(BOOL bRestore)
	{
		if (m_hRgn != NULL)
		{
			SelectClipRgn(m_MemDC, NULL);
			DeleteObject(m_hRgn);
		}
		m_hRgn = CreateRectRgn(m_RenderDestRect.Left, m_RenderDestRect.Top, m_RenderDestRect.Right, m_RenderDestRect.Bottom);
		INT i = SelectClipRgn(m_MemDC, m_hRgn);
		ASSERT(i != ERROR);
	}

	HWND m_hWnd;
	HDC m_PaintDC;

	HDC m_MemDC;
	uiWndDC m_WndDC;
	HBITMAP m_hBmp, m_hOldBmp;
	HRGN m_hRgn;

	UINT m_nWidth, m_nHeight;


};


class uiWndFont
{
public:

	uiWndFont(HFONT hFont)
	:m_hFont(hFont)
	{
	}
	uiWndFont()
	:m_hFont(NULL)
	{
	}
	~uiWndFont()
	{
	}

	BOOL GetLogFont(LOGFONT *pLogFont)
	{
		ASSERT(IS_ALIGNED(pLogFont, 4));
		return ::GetObject(m_hFont, sizeof(LOGFONT), pLogFont) == sizeof(LOGFONT);
	}


protected:

	HFONT m_hFont;


};


