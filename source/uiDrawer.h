

#pragma once


#include "uiCommon.h"


enum SYS_COLOR_NAME
{
	SCN_CAPTAIN,
	SCN_FRAME,

};


UINT32 uiGetSysColor(INT index);


class uiImage
{
public:

	uiImage() = default;
	~uiImage() = default;




};


class uiDrawer
{
public:

	uiDrawer();
	virtual ~uiDrawer();


	virtual BOOL Begin(void *pCtx) = 0;
	virtual void End(void *pCtx) = 0;

	virtual void ResizeBackBuffer(UINT nWidth, UINT nHeight) {}

	virtual void FillRect(uiRect rect, UINT32 color) = 0;
	virtual void RoundRect(uiRect rect, UINT32 color, INT width, INT height) = 0;
	virtual void DrawEdge(uiRect &rect, UINT color) = 0;
	virtual void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) = 0;
	virtual void DrawText(const TCHAR *pText, const uiRect &rect, UINT flag) = 0;

	BOOL PushDestRect(uiRect rect); // Return true if rectangle region is visible.
	void PopDestRect();

//	INLINE const uiRect& GetDestRect() { return m_RenderDestRect; }


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
		ASSERT(hDC != NULL);
		ASSERT(m_hDC == NULL);
		m_hDC = hDC;
		m_hOldPen = (HPEN)::GetCurrentObject(m_hDC, OBJ_PEN);
		m_hOldBrush = (HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH);
		m_hOldFont = (HFONT)::GetCurrentObject(m_hDC, OBJ_FONT);
		return TRUE;
	}
	BOOL Detach()
	{
		if (m_hDC != NULL)
		{
			HPEN hPen = (HPEN)::SelectObject(m_hDC, m_hOldPen);
			HBRUSH hBrush = (HBRUSH)::SelectObject(m_hDC, m_hOldBrush);
			HFONT hFont = (HFONT)::SelectObject(m_hDC, m_hOldFont);

			if (hPen != m_hOldPen)
				::DeleteObject(hPen);
			if (hBrush != m_hOldBrush)
				::DeleteObject(hBrush);
			if (hFont != m_hOldFont)
				::DeleteObject(hFont);

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

	INLINE HDC GetDC() const { return m_hDC; }


protected:

	HDC    m_hDC;
	HPEN   m_hOldPen;
	HBRUSH m_hOldBrush;
	HFONT  m_hOldFont;


};


class uiWndBackBuffer
{
public:

	enum { MAX_WIDTH = 65536, MAX_HEIGHT = 65536, };

	uiWndBackBuffer()
	:m_MemDC(NULL), m_hOldBmp(NULL), m_hBmp(NULL)
	{
	}
	~uiWndBackBuffer()
	{
		if (m_MemDC != NULL)
		{
#ifdef _DEBUG
			ASSERT(m_hOldPen == (HPEN)::GetCurrentObject(m_MemDC, OBJ_PEN));
			ASSERT(m_hOldBrush == (HBRUSH)::GetCurrentObject(m_MemDC, OBJ_BRUSH));
			ASSERT(m_hOldFont = (HFONT)::GetCurrentObject(m_MemDC, OBJ_FONT));
#endif
			::SelectObject(m_MemDC, m_hOldBmp); // This may not be necessary.
			::DeleteDC(m_MemDC); // Delete memory dc first then delete bitmap.
		}
		if (m_hBmp != NULL)
			::DeleteObject(m_hBmp);
	}

	BOOL Init(HDC hDC, UINT NewWidth, UINT NewHeight)
	{
		ASSERT(NewWidth < MAX_WIDTH && NewHeight < MAX_HEIGHT);

		m_MemDC = CreateCompatibleDC(hDC);
		m_hBmp = CreateCompatibleBitmap(hDC, NewWidth, NewHeight);
		ASSERT(m_MemDC != NULL && m_hBmp != NULL);
		if (m_MemDC == NULL || m_hBmp == NULL)
			return FALSE;
		m_hOldBmp = (HBITMAP)SelectObject(m_MemDC, m_hBmp);

#ifdef _DEBUG
		m_hOldPen = (HPEN)::GetCurrentObject(m_MemDC, OBJ_PEN);
		m_hOldBrush = (HBRUSH)::GetCurrentObject(m_MemDC, OBJ_BRUSH);
		m_hOldFont = (HFONT)::GetCurrentObject(m_MemDC, OBJ_FONT);
#endif

		return TRUE;
	}
	BOOL Resize(HDC hDC, UINT NewWidth, UINT NewHeight)
	{
		if (m_hBmp != NULL)
		{
			SelectObject(m_MemDC, m_hOldBmp);
			::DeleteObject(m_hBmp);

			m_hBmp = CreateCompatibleBitmap(hDC, NewWidth, NewHeight);
			m_hOldBmp = (HBITMAP)SelectObject(m_MemDC, m_hBmp);
			return (m_hBmp != NULL);
		}
		return TRUE;
	}

	INLINE HDC GetMemDC() const { return m_MemDC; }


protected:

	HDC     m_MemDC;
	HBITMAP m_hOldBmp, m_hBmp;

#ifdef _DEBUG
	HPEN   m_hOldPen;
	HBRUSH m_hOldBrush;
	HFONT  m_hOldFont;
#endif


};


class uiWndDrawer : public uiDrawer
{
public:

	enum { MAX_BACKBUFFER_COUNT = 3, };

	uiWndDrawer()
	:m_hWnd(NULL), m_PaintDC(NULL)
	{
		m_hRgn = NULL;
		m_TotalBackBuffer = m_CurrentBackBufferIndex = 0;
		m_nWidth = m_nHeight = 0;
	}
	~uiWndDrawer()
	{
		ASSERT(m_WndDrawDC.GetDC() == NULL);
		ASSERT(m_PaintDC == NULL);
		ASSERT(m_hRgn == NULL);
	}

	BOOL Begin(void *pCtx) override;
	void End(void *pCtx) override;

	BOOL InitBackBuffer(UINT nCount, HWND hWnd, UINT nWidth, UINT nHeight);
	void ResizeBackBuffer(UINT nWidth, UINT nHeight) override;

	void FillRect(uiRect rect, UINT32 color) override;
	void RoundRect(uiRect rect, UINT32 color, INT width, INT height) override;
	void DrawEdge(uiRect &rect, UINT color) override;
	void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) override;
	void DrawText(const TCHAR *pText, const uiRect &rect, UINT flag) override;


protected:

	void OnDestRectChanged(BOOL bRestore) override
	{
		HDC hMemDC = m_WndDrawDC.GetDC();
		if (m_hRgn != NULL)
		{
			SelectClipRgn(hMemDC, NULL);
			DeleteObject(m_hRgn);
		}
		m_hRgn = CreateRectRgn(m_RenderDestRect.Left, m_RenderDestRect.Top, m_RenderDestRect.Right, m_RenderDestRect.Bottom);
		INT i = SelectClipRgn(hMemDC, m_hRgn);
		ASSERT(i != ERROR);
	}

	HWND m_hWnd;
	HDC m_PaintDC;

	uiWndDC m_WndDrawDC;
	HRGN m_hRgn;

	INT m_TotalBackBuffer, m_CurrentBackBufferIndex;
	uiWndBackBuffer m_BackBuffer[MAX_BACKBUFFER_COUNT];
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


