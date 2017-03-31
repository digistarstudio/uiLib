

#pragma once


#include "uiCommon.h"
#include <memory>
#include <unordered_map>


// If don't use this, you must calculate text length and draw from first visible character by yourself.
// RoundRect has side effect too if you didn't recalculate the visible portion and decided if should call it.
#define USE_GDI_CLIPPING


#define DISABLE_DYNAMIC_ALLOCATION \
void* operator new(std::size_t size) = delete; \
void* operator new(std::size_t size, void* addr) = delete; // Disable placement new.


#define DECLARE_WIN_HANDLE_TYPE(ClassType, Deleter, winType) \
class ClassType \
{ \
public: \
	static BOOL Del(HANDLE hHandle) { return ::Deleter((winType)hHandle); } \
	static std::unordered_map<std::size_t, std::weak_ptr<stHandleWrapper<ClassType>>> HandleMap; \
	static CHAR* GetName() { return #ClassType; } \
};

#define IMPLEMENT_WIN_HANDLE_TYPE(ClassType) \
std::unordered_map<std::size_t, std::weak_ptr<stHandleWrapper<ClassType>>> ClassType::HandleMap;


template <typename HandleType>
struct stHandleWrapper
{
	INLINE stHandleWrapper::stHandleWrapper(HANDLE hHandleIn, size_t KeyIn)
	:hHandle(hHandleIn), Key(KeyIn)
	{
	}
	stHandleWrapper::~stHandleWrapper()
	{
		printx("---> stHandleWrapper::~stHandleWrapper type: %s\n", HandleType::GetName());
		HandleType::HandleMap.erase(Key);
		BOOL bRet = HandleType::Del(hHandle);
		if (!bRet)
		{
			DWORD ec = GetLastError();
			ASSERT(0);
		}
	}

	HANDLE hHandle;
	size_t Key;

};


DECLARE_WIN_HANDLE_TYPE(winFontHandleType, DeleteObject, HGDIOBJ)


// These two functions are "CP" from DX12 mini engine. Hope c standard has the same things.
INLINE size_t HashRange(const uint32_t* const Begin, const uint32_t* const End, size_t Hash)
{
	for (const uint32_t* Iter = Begin; Iter < End; ++Iter)
		Hash = 16777619U * Hash ^ *Iter;
	return Hash;
}
template <typename T>
INLINE size_t HashState(const T* StateDesc, size_t Count = 1, size_t Hash = 2166136261U)
{
	static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
	return HashRange((uint32_t*)StateDesc, (uint32_t*)(StateDesc + Count), Hash);
}


typedef stHandleWrapper<winFontHandleType> FontHandleW;


class uiFont
{
public:

	DISABLE_DYNAMIC_ALLOCATION;

	uiFont() {}
	uiFont(const uiFont& src) { m_ptrHandle = src.m_ptrHandle; }
	~uiFont() {}


	BOOL Create(const TCHAR* pName, INT height, INT width);


	INLINE HANDLE GetHandle() const { return (m_ptrHandle) ? m_ptrHandle->hHandle : NULL; }
	INLINE BOOL   IsValid() const { return (bool)m_ptrHandle; }
	INLINE void   Release() { m_ptrHandle.reset(); }

	INLINE uiFont& operator=(uiFont&& rhs) { m_ptrHandle = std::move(rhs.m_ptrHandle); return *this; }
	INLINE uiFont& operator=(const uiFont& rhs) { m_ptrHandle = rhs.m_ptrHandle; return *this; }
	INLINE bool    operator==(const uiFont& in) const { return m_ptrHandle == in.m_ptrHandle; }


protected:

	std::shared_ptr<FontHandleW> m_ptrHandle;


};


enum WIN_IMAGE_TYPE
{
	WIT_BMP,
	WIT_CURSOR,
	WIT_ICON,

	WIT_TOTAL,
	WIT_NONE,

	WIT_TYPE_MASK = 0x03,
	WIT_SHARED_RESOURCE = 0x01 << 6,
	WIT_LOAD_RESOURCE   = 0x01 << 7,
};


IMPLEMENT_ENUM_FLAG(WIN_IMAGE_TYPE);


struct stImgHandleWrapper
{
	struct Functor
	{
		union
		{
			BOOL(WINAPI *Deleter)(HANDLE);
			BOOL(WINAPI *g)(HGDIOBJ);
			BOOL(WINAPI *c)(HCURSOR);
		//	BOOL(WINAPI *i)(HICON);
		};
		INLINE BOOL operator()(HANDLE hHandle) const { return Deleter(hHandle); }
		Functor(BOOL(WINAPI *img)(HGDIOBJ)) { g = img; }
		Functor(BOOL(WINAPI *cur)(HCURSOR)) { c = cur; }
	//	Functor(BOOL(WINAPI *ico)(HICON)) { i = ico; }
	};

	typedef std::map<const TCHAR*, std::weak_ptr<stImgHandleWrapper>, cmp_str> PathMap;
	typedef std::unordered_map<std::size_t, std::weak_ptr<stImgHandleWrapper>> KeyMap;

	INLINE stImgHandleWrapper::stImgHandleWrapper(WIN_IMAGE_TYPE typeIn, HANDLE hHandleIn, size_t KeyIn)
	:Type(typeIn), hHandle(hHandleIn), Key(KeyIn)
	{
		Type |= WIT_LOAD_RESOURCE;
	}
	INLINE stImgHandleWrapper::stImgHandleWrapper(WIN_IMAGE_TYPE typeIn, HANDLE hHandleIn)
	:Type(typeIn), hHandle(hHandleIn)
	{
	}

	stImgHandleWrapper::~stImgHandleWrapper()
	{
	//	printx("---> stImgHandleWrapper::~stImgHandleWrapper type: %d\n", Type & WIT_TYPE_MASK);

		DEBUG_CHECK(DebugCheckFunc);

		if (Type & WIT_LOAD_RESOURCE)
			KeyHandleMap.erase(Key);
		else
		{
			PathHandleMap.erase(it);
			it.PathMap::iterator::~iterator();
		}

		BOOL bRet = m_Del[Type & WIT_TYPE_MASK](hHandle);
		if (!bRet)
		{
			printx("Failed to close image handle! ec: %d\n", GetLastError()); // if ec is zero, there might be some win32 api using the handle.
			ASSERT(0);
		}
	}

	INLINE WIN_IMAGE_TYPE GetType() const { return (WIN_IMAGE_TYPE)(Type & WIT_TYPE_MASK); }
	void Set(PathMap::iterator itIn, uiString &strIn)
	{
		ASSERT(!(Type & WIT_LOAD_RESOURCE));
		strPath = std::move(strIn); // Take over the string that store in the map.
		new (&it) PathMap::iterator(itIn);
	}

	void DebugCheckFunc()
	{
		if (Type & WIT_LOAD_RESOURCE)
		{
			ASSERT(KeyHandleMap[Key].expired());
		}
		else
		{
			ASSERT(PathHandleMap.find(strPath) == it);
		}
	}


	WIN_IMAGE_TYPE Type;
	HANDLE hHandle;
	uiString strPath; // No need to save space of this member. (28/56)
	union
	{
		PathMap::iterator it;
		size_t Key;
	};


	static Functor m_Del[WIT_TOTAL];
	static PathMap PathHandleMap;
	static KeyMap  KeyHandleMap;


};


class uiImage
{
public:

	DISABLE_DYNAMIC_ALLOCATION;

	uiImage() {}
	uiImage(const uiImage& src) { m_pImg = src.m_pImg; }
	~uiImage() {}


	BOOL LoadCursorRes(UINT ResID, const TCHAR* ResType = nullptr, HINSTANCE hModule = NULL);
	BOOL LoadIconRes(UINT ResID, HINSTANCE hModule = NULL);

	BOOL LoadFromFile(uiString str);


	INLINE HANDLE GetHandle() const { return (m_pImg) ? m_pImg->hHandle : NULL; }
	INLINE BOOL   IsValid() const { return (bool)m_pImg; }
	INLINE void   Release() { m_pImg.reset(); }

	INLINE uiImage& operator=(uiImage&& rhs) { m_pImg = std::move(rhs.m_pImg); return *this; }
	INLINE uiImage& operator=(const uiImage& rhs) { m_pImg = rhs.m_pImg; return *this; }
	INLINE bool     operator==(const uiImage& in) const { return m_pImg == in.m_pImg; }
	INLINE bool     operator!=(const uiImage& in) const { return m_pImg != in.m_pImg; }

	INLINE WIN_IMAGE_TYPE GetType() const { return (m_pImg) ? m_pImg->GetType() : WIT_NONE; }


protected:

	struct stImageSourceInfo
	{
		HINSTANCE hModule;
		UINT      ResID;
	};

	static WIN_IMAGE_TYPE GetType(uiString& str);


	std::shared_ptr<stImgHandleWrapper> m_pImg;


};


#define RANGE(var, min, max) ( ((var) < (min)) ? (min) : (((var) > (max)) ? (max) : (var)) )


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


	virtual BOOL Begin(void *pCtx) = 0;
	virtual void End(void *pCtx) = 0;

	virtual void ResizeBackBuffer(UINT nWidth, UINT nHeight) {}

	virtual void FillRect(uiRect rect, UINT32 color) = 0;
	virtual void RoundRect(uiRect rect, UINT32 color, INT width, INT height) = 0;
	virtual void DrawEdge(uiRect &rect, UINT color) = 0;
	virtual void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) = 0;
	virtual void DrawText(const TCHAR *pText, const uiRect &rect, UINT flag) = 0;
	virtual BOOL DrawImage(const uiImage& img, INT x, INT y, INT width, INT height, UINT32 flags) = 0;


	BOOL PushDestRect(uiRect rect); // Return true if rectangle region is visible.
	void PopDestRect();


//	INLINE const uiRect& GetDestRect() { return m_RenderDestRect; }

	INLINE void UpdateCoordinate(uiRect &rect)
	{
		rect.Move(m_OriginX, m_OriginY);
	}
	INLINE void UpdateCoordinate(INT &x, INT &y)
	{
		x += m_OriginX;
		y += m_OriginY;
	}


protected:

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
		m_hDC = m_hBmpDC = NULL;
		m_hOldBmp = NULL;
		m_hOldPen = NULL;
		m_hOldBrush = NULL;
		m_hOldFont = NULL;
	}
	~uiWndDC()
	{
		ASSERT(m_hDC == NULL);
		ASSERT(m_hBmpDC == NULL);
	}

	BOOL Attach(HDC hDC)
	{
		ASSERT(hDC != NULL);
		ASSERT(m_hDC == NULL);
		ASSERT(m_hBmpDC == NULL);
		m_hDC = hDC;
		m_hOldPen = (HPEN)::GetCurrentObject(m_hDC, OBJ_PEN);
		m_hOldBrush = (HBRUSH)::GetCurrentObject(m_hDC, OBJ_BRUSH);
		m_hOldFont = (HFONT)::GetCurrentObject(m_hDC, OBJ_FONT);
		return TRUE;
	}
	BOOL Detach()
	{
		if (m_hBmpDC != NULL)
		{
			::SelectObject(m_hBmpDC, m_hOldBmp);
			VERIFY(DeleteDC(m_hBmpDC));
			m_hBmpDC = NULL;
		}
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

	BOOL SelectBmp(HDC hPaintDC, HBITMAP hBmp)
	{
		if (m_hBmpDC == NULL)
		{
			if ((m_hBmpDC = ::CreateCompatibleDC(hPaintDC)) == NULL)
				return FALSE;
			m_hOldBmp = (HBITMAP)::GetCurrentObject(m_hBmpDC, OBJ_BITMAP);
		}
		return (::SelectObject(m_hBmpDC, hBmp) != HGDI_ERROR);
	}

	BOOL BitBlt(INT x, INT y, INT cx, INT cy, INT x1, INT y1, DWORD rop = SRCCOPY)
	{
		return ::BitBlt(m_hDC, x, y, cx, cy, m_hBmpDC, x1, y1, rop);
	}

	INLINE HDC GetDC() const { return m_hDC; }


protected:

	HDC     m_hDC, m_hBmpDC;
	HBITMAP m_hOldBmp;
	HPEN    m_hOldPen;
	HBRUSH  m_hOldBrush;
	HFONT   m_hOldFont;


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
		if (m_MemDC != NULL) // Delete memory dc first then delete bitmap.
		{
#ifdef _DEBUG
			ASSERT(m_hOldPen == (HPEN)::GetCurrentObject(m_MemDC, OBJ_PEN));
			ASSERT(m_hOldBrush == (HBRUSH)::GetCurrentObject(m_MemDC, OBJ_BRUSH));
			ASSERT(m_hOldFont = (HFONT)::GetCurrentObject(m_MemDC, OBJ_FONT));
#endif
			::SelectObject(m_MemDC, m_hOldBmp); // This may not be necessary.
			::DeleteDC(m_MemDC);
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
	BOOL DrawImage(const uiImage& img, INT x, INT y, INT width, INT height, UINT32 flags) override;


protected:

	void OnDestRectChanged(BOOL bRestore) override
	{
		if (bRestore)
			return;
#ifdef USE_GDI_CLIPPING
		HDC hMemDC = m_WndDrawDC.GetDC();
		if (m_hRgn != NULL)
		{
			SelectClipRgn(hMemDC, NULL);
			DeleteObject(m_hRgn);
		}
		m_hRgn = CreateRectRgn(m_RenderDestRect.Left, m_RenderDestRect.Top, m_RenderDestRect.Right, m_RenderDestRect.Bottom);
		INT i = SelectClipRgn(hMemDC, m_hRgn);
		ASSERT(i != ERROR);
#endif
	}

	HWND m_hWnd;
	HDC m_PaintDC;

	uiWndDC m_WndDrawDC;
	HRGN m_hRgn;

	INT m_TotalBackBuffer, m_CurrentBackBufferIndex;
	uiWndBackBuffer m_BackBuffer[MAX_BACKBUFFER_COUNT];
	UINT m_nWidth, m_nHeight;


};


