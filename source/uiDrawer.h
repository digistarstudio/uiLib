

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


enum SYSTEM_FONT_TYPE
{
	SFT_CAPTION,
	SFT_SM_CAPTION,
	SFT_MENU,
	SFT_STATUS,
	SFT_MESSAGE,

	SFT_TOTAL,
};


class uiFont
{
public:

	DISABLE_DYNAMIC_ALLOCATION;

	uiFont() {}
	uiFont(const uiFont& src) { m_ptrHandle = src.m_ptrHandle; }
	uiFont(uiFont&& src) :m_ptrHandle(std::move(src.m_ptrHandle)) {}
	~uiFont() {}


	BOOL Create(const TCHAR* pName, INT height, INT width);
	BOOL Create(const LOGFONT& logfont);


	INLINE HANDLE GetHandle() const { return (m_ptrHandle) ? m_ptrHandle->hHandle : NULL; }
	INLINE BOOL   IsValid() const { return (bool)m_ptrHandle; }
	INLINE void   Release() { m_ptrHandle.reset(); }

	INLINE uiFont& operator=(uiFont&& rhs) noexcept { m_ptrHandle = std::move(rhs.m_ptrHandle); return *this; }
	INLINE uiFont& operator=(const uiFont& rhs) { m_ptrHandle = rhs.m_ptrHandle; return *this; }
	INLINE bool    operator==(const uiFont& in) const { return m_ptrHandle == in.m_ptrHandle; }
	INLINE bool    operator!=(const uiFont& in) const { return m_ptrHandle != in.m_ptrHandle; }


protected:

	std::shared_ptr<FontHandleW> m_ptrHandle;


};


void InitSystemFont();
void ReleaseSystemFont();
extern uiFont GSystemFont[SFT_TOTAL];
INLINE const uiFont& uiGetSysFont(SYSTEM_FONT_TYPE sft)
{
	ASSERT(sft < _countof(GSystemFont));
	return GSystemFont[sft];
}


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

		DEBUG_CHECK(DebugCheckFunc());

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
	void Set(PathMap::iterator itIn, uiString&& strIn)
	{
		ASSERT(!(Type & WIT_LOAD_RESOURCE));
		strPath = std::move(strIn); // Take over the string that store in the map.
		new (&it) PathMap::iterator(itIn);
	}

	void DebugCheckFunc()
	{
		ASSERT((Type & WIT_TYPE_MASK) < WIT_TOTAL);
		if (Type & WIT_LOAD_RESOURCE)
			ASSERT(KeyHandleMap[Key].expired());
		else
			ASSERT(PathHandleMap.find(strPath) == it);
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
	uiImage(uiImage&& src) :m_pImg(std::move(src.m_pImg)) {}
	~uiImage() {}


	BOOL LoadCursorRes(UINT ResID, const TCHAR* ResType = nullptr, HINSTANCE hModule = NULL);
	BOOL LoadIconRes(UINT ResID, HINSTANCE hModule = NULL);

	BOOL LoadFromFile(uiString str);

	INLINE BOOL IsValid() const { return (bool)m_pImg; }
	INLINE void Release() { m_pImg.reset(); }

	INLINE uiImage& operator=(uiImage&& rhs) noexcept { m_pImg = std::move(rhs.m_pImg); return *this; }
	INLINE uiImage& operator=(const uiImage& rhs) { m_pImg = rhs.m_pImg; return *this; }
	INLINE bool     operator==(const uiImage& in) const { return m_pImg == in.m_pImg; }
	INLINE bool     operator!=(const uiImage& in) const { return m_pImg != in.m_pImg; }


	struct stImageInfo
	{
		stImageInfo() { ZeroMemory(this, sizeof(*this)); }

		INT  Width, Height;
		// Cursor info.
		INT  TotalFrame, DispRate;
		INT  HotSpotX, HotSpotY;
	};
	BOOL GetInfo(stImageInfo& ImgInfo);

	// TODO: move to protected scope
	INLINE HANDLE GetHandle() const { ASSERT(m_pImg); return m_pImg->hHandle; }
	INLINE WIN_IMAGE_TYPE GetType() const { ASSERT(m_pImg); return m_pImg->GetType(); }


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
	SCN_INACTIVECAPTION,
	SCN_FRAME,
};


UINT32 uiGetSysColor(INT index);


class uiDrawerBase
{
public:

	enum DEST_CHANGE_TYPE { DEST_RESTORE, DEST_PUSH, DEST_UPDATE };


	uiDrawerBase() { m_CurDepth = 1; m_ModeFlag = m_OriginX = m_OriginY = 0; }
	//~uiDrawerBase() {}


	INLINE const uiRect& GetDestRect() { return m_RenderDestRect; }
	INLINE void SetMode(INT mode) { m_ModeFlag = mode; }
	INLINE INT  GetMode() const { return m_ModeFlag; }
	INLINE int  GetCurrentDepth() const { return m_CurDepth; }
	INLINE void UpdateCoordinate(uiRect& rect)
	{
		rect.Move(m_OriginX, m_OriginY);
	}
	INLINE void UpdateCoordinate(INT& x, INT& y)
	{
		x += m_OriginX;
		y += m_OriginY;
		const int i = sizeof(m_RectList);
	}


protected:

	struct stRenderDestInfo
	{
		INT    OriginX, OriginY;
		uiRect rect;
		void*  ctx;
	};

	INT  m_CurDepth, m_ModeFlag;
	INT  m_OriginX, m_OriginY;

	uiRect m_RenderDestRect;
	TList<stRenderDestInfo> m_RectList; // x86: 24 Bytes


};


template<typename T>
class uiDrawerT : public T
{
public:

	BOOL PushDestRect(uiRect rect) // Return true if rectangle region is visible.
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
			OnDestRectChanged(DEST_PUSH, RDInfo.ctx);
			m_RectList.AddDataHead(RDInfo);

			m_OriginX += pt.x;
			m_OriginY += pt.y;
			++m_CurDepth;

			return TRUE;
		}

		m_RenderDestRect = RDInfo.rect;

		return FALSE;
	}
	void PopDestRect()
	{
		stRenderDestInfo RDInfo;
		ListIndex index = m_RectList.GetHeadIndex();
		m_RectList.GetAndRemove(index, RDInfo);

		m_OriginX = RDInfo.OriginX;
		m_OriginY = RDInfo.OriginY;
		m_RenderDestRect = RDInfo.rect;
		OnDestRectChanged(DEST_RESTORE, RDInfo.ctx);

		--m_CurDepth;
	}

//	INLINE void UpdateDestRect() { OnDestRectChanged(DEST_UPDATE, void*()); }


};


struct stTextParam
{
	UINT color;
	UINT flag;  //TODO
};

struct stDrawImageParam
{
	stDrawImageParam(INT xIn, INT yIn, INT widthIn, INT heightIn, UINT flagsIn = 0, UINT AniIndexIn = 0);

	INT  x, y, width, height;
	UINT flags;
	UINT aniIndex;
};

INLINE stDrawImageParam::stDrawImageParam(INT xIn, INT yIn, INT widthIn, INT heightIn, UINT flagsIn, UINT AniIndexIn)
:x(xIn), y(yIn), width(widthIn), height(heightIn), flags(flagsIn), aniIndex(AniIndexIn)
{
}


class uiDrawerVInterface : public uiDrawerBase
{
public:

	virtual ~uiDrawerVInterface() {}

	virtual BOOL Begin(void* pCtx) = 0;
	virtual void End(void* pCtx) = 0;

	virtual BOOL BeginCache(const uiRect& rect) = 0; // Directly paint on previous back buffer.
	virtual BOOL EndCache() = 0;

	virtual void ResizeBackBuffer(UINT nWidth, UINT nHeight) = 0;

	virtual void FillRect(uiRect rect, UINT32 color) = 0;
	virtual void RoundRect(uiRect rect, UINT32 color, INT width, INT height) = 0;
	virtual void DrawEdge(uiRect& rect, UINT color) = 0;
	virtual void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) = 0;
	virtual BOOL Text(const uiString& str, uiRect rect, const uiFont& Font, const stTextParam* pParam = nullptr) = 0;
	virtual BOOL DrawImage(const uiImage& img, const stDrawImageParam& param) = 0;


protected:

	virtual void OnDestRectChanged(DEST_CHANGE_TYPE dct, void*& ctx) {}


};


class uiGDIObjCacher // Save almost 80% time if cache hit.
{
public:

	enum { DEFAULT_MAX_CACHE_COUNT = 50, };
	enum OBJECT_TYPE { TYPE_BRUSH, TYPE_PEN, TYPE_REGION };

	struct stGDIObjectName
	{
		INLINE stGDIObjectName(OBJECT_TYPE otIn, uiRect& rectIn)
		:ot(otIn), rect(rectIn)
		{
		}
		INLINE stGDIObjectName(OBJECT_TYPE otIn, INT StyleIn, INT WidthIn, UINT PenColorIn)
		:ot(otIn), Style(StyleIn), Width(WidthIn), PenColor(PenColorIn), padding(0)
		{
		}
		INLINE stGDIObjectName(OBJECT_TYPE otIn, UINT BrushColorIn)
		:ot(otIn), rect()
		{
			BrushColor = BrushColorIn;
		}

		OBJECT_TYPE ot;
		union
		{
			UINT BrushColor;
			struct
			{
				UINT PenColor;
				INT  Style;
				INT  Width;
				INT  padding;
			};
			uiRect rect;
		};
	};


	uiGDIObjCacher()
	{
		ASSERT(MaxCacheCount == 0);
		MaxCacheCount = DEFAULT_MAX_CACHE_COUNT;
		INIT_LIST_HEAD(&ListHead);
	}
	~uiGDIObjCacher();

	static HGDIOBJ Find(const stGDIObjectName& ObjName);
	static void Release();


protected:

	static UINT MaxCacheCount;
	static UINT CurCacheCount;
	static list_head ListHead;
	static std::unordered_map<std::size_t, stSimpleListNode*> HandleMap;


};


class uiWndDC
{
public:

	uiWndDC()
	{
		m_hDC = NULL;
		m_hOldBmp = NULL;
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
		m_hOldPen = (HPEN)::GetCurrentObject(hDC, OBJ_PEN);
		m_hOldBrush = (HBRUSH)::GetCurrentObject(hDC, OBJ_BRUSH);
		m_hOldFont = (HFONT)::GetCurrentObject(hDC, OBJ_FONT);
		return TRUE;
	}
	BOOL Detach()
	{
		if (m_hDC != NULL)
		{
			HPEN hPen = (HPEN)::SelectObject(m_hDC, m_hOldPen);
			HBRUSH hBrush = (HBRUSH)::SelectObject(m_hDC, m_hOldBrush);
			HFONT hFont = (HFONT)::SelectObject(m_hDC, m_hOldFont);
			m_hDC = NULL;
			return TRUE;
		}
		return FALSE;
	}


	BOOL SetPen(INT width, UINT32 color, INT style = PS_SOLID)
	{
		uiGDIObjCacher::stGDIObjectName gon(uiGDIObjCacher::TYPE_PEN, style, width, color);
		return ::SelectObject(m_hDC, uiGDIObjCacher::Find(gon)) != NULL;
	}
	BOOL SetHollowBrush()
	{
		return ::SelectObject(m_hDC, ::GetStockObject(HOLLOW_BRUSH)) != NULL;
	}
	BOOL SetBrush(UINT32 color)
	{
		uiGDIObjCacher::stGDIObjectName gon(uiGDIObjCacher::TYPE_BRUSH, color);
		return ::SelectObject(m_hDC, uiGDIObjCacher::Find(gon)) != NULL;
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

	BOOL BitBlt(HDC MemDC, INT x, INT y, INT cx, INT cy, INT x1, INT y1, DWORD rop = SRCCOPY)
	{
		return ::BitBlt(m_hDC, x, y, cx, cy, MemDC, x1, y1, rop);
	}

	INLINE HDC GetDC() const { return m_hDC; }


protected:

	HDC     m_hDC;
	HBITMAP m_hOldBmp;
	HPEN    m_hOldPen;
	HBRUSH  m_hOldBrush;
	HFONT   m_hOldFont;


};


class uiWndBackBuffer
{
public:

	enum { MAX_WIDTH = 65536, MAX_HEIGHT = 65536, EXTRA_BUFFER_WIDTH = 50, EXTRA_BUFFER_HEIGHT = 50, };

	uiWndBackBuffer()
	:m_MemDC(NULL), m_hOldBmp(NULL), m_hBmp(NULL), m_CurWidth(0), m_CurHeight(0)
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
		NewWidth += EXTRA_BUFFER_WIDTH;
		NewHeight += EXTRA_BUFFER_HEIGHT;

		ASSERT(NewWidth < MAX_WIDTH && NewHeight < MAX_HEIGHT);

		m_MemDC = CreateCompatibleDC(hDC);
		m_hBmp = CreateCompatibleBitmap(hDC, NewWidth, NewHeight);
		ASSERT(m_MemDC != NULL && m_hBmp != NULL);
		if (m_MemDC == NULL || m_hBmp == NULL)
			return FALSE;
		m_hOldBmp = (HBITMAP)SelectObject(m_MemDC, m_hBmp);
		m_CurWidth = NewWidth;
		m_CurHeight = NewHeight;

#ifdef _DEBUG
		m_hOldPen = (HPEN)::GetCurrentObject(m_MemDC, OBJ_PEN);
		m_hOldBrush = (HBRUSH)::GetCurrentObject(m_MemDC, OBJ_BRUSH);
		m_hOldFont = (HFONT)::GetCurrentObject(m_MemDC, OBJ_FONT);
#endif

		return TRUE;
	}
	BOOL Resize(HDC hDC, UINT NewWidth, UINT NewHeight)
	{
	//	printx("---> Resize() Old Width: %d, New Width: %d, Old Height: %d, New Height: %d\n", m_CurWidth, NewWidth, m_CurHeight, NewHeight);

		INT  bXScaleUp = NewWidth - m_CurWidth, bYScaleUp = NewHeight - m_CurHeight;
		BOOL bXBuffered = -bXScaleUp <= EXTRA_BUFFER_WIDTH;
		BOOL bYBuffered = -bYScaleUp <= EXTRA_BUFFER_HEIGHT;

		if (bXScaleUp <= 0 && bXBuffered && bYScaleUp <= 0 && bYBuffered)
			return TRUE;

		if (bXScaleUp > 0)
			NewWidth += EXTRA_BUFFER_WIDTH;
		else if (bXBuffered)
			NewWidth = m_CurWidth;

		if (bYScaleUp > 0)
			NewHeight += EXTRA_BUFFER_HEIGHT;
		else if (bYBuffered)
			NewHeight = m_CurHeight;

		if (m_hBmp != NULL)
		{
			VERIFY(SelectObject(m_MemDC, m_hOldBmp) != NULL);
			VERIFY(::DeleteObject(m_hBmp));
		}
		if ((m_hBmp = CreateCompatibleBitmap(hDC, NewWidth, NewHeight)) != NULL)
		{
		//	printx("---> New Width: %d, New Height: %d\n", NewWidth, NewHeight);
			VERIFY(SelectObject(m_MemDC, m_hBmp) != NULL);
			m_CurWidth = NewWidth;
			m_CurHeight = NewHeight;
			return TRUE;
		}

		return FALSE;
	}

	INLINE HDC GetMemDC() const { return m_MemDC; }


protected:

	HDC     m_MemDC;
	HBITMAP m_hOldBmp, m_hBmp;
	UINT    m_CurWidth, m_CurHeight;

#ifdef _DEBUG
	HPEN   m_hOldPen;
	HBRUSH m_hOldBrush;
	HFONT  m_hOldFont;
#endif


};


//#define DRAWER_NO_VTABLE_LOOKUP

#ifdef DRAWER_NO_VTABLE_LOOKUP
	//#define DRAWER_BASE_TYPE uiDrawerT<uiDrawerBase>
	#define DRAWER_BASE_TYPE uiDrawerBase
	#define OVERRIDE
#else
	#define DRAWER_BASE_TYPE uiDrawerT<uiDrawerVInterface>
	typedef uiDrawerT<uiDrawerVInterface> uiDrawer;
	#define uiDrawerInsType uiWndDrawer
	#define OVERRIDE override
#endif


class uiWndDrawer : public DRAWER_BASE_TYPE
{
public:

	enum { MAX_BACKBUFFER_COUNT = 3, };

	uiWndDrawer()
	:m_hWnd(NULL), m_PaintDC(NULL), m_hMemDC(NULL)
	{
		m_hRgn = NULL;
		m_hOldBmp = NULL;
		m_TotalBackBuffer = m_CurrentBackBufferIndex = 0;
		m_nWidth = m_nHeight = 0;
	}
	~uiWndDrawer()
	{
		ASSERT(m_WndDrawDC.GetDC() == NULL);
		ASSERT(m_PaintDC == NULL);
		ASSERT(m_hRgn == NULL);

		if (m_hMemDC != NULL)
		{
			VERIFY(::SelectObject(m_hMemDC, m_hOldBmp) != NULL);
			VERIFY(::DeleteDC(m_hMemDC));
		}
	}

	BOOL InitBackBuffer(UINT nCount, HWND hWnd, UINT nWidth, UINT nHeight);

	BOOL Begin(void* pCtx) OVERRIDE;
	void End(void* pCtx) OVERRIDE;

	BOOL BeginCache(const uiRect& rect) OVERRIDE;
	BOOL EndCache() OVERRIDE;

	void ResizeBackBuffer(UINT nWidth, UINT nHeight) OVERRIDE;

	void FillRect(uiRect rect, UINT32 color) OVERRIDE;
	void RoundRect(uiRect rect, UINT32 color, INT width, INT height) OVERRIDE;
	void DrawEdge(uiRect& rect, UINT color) OVERRIDE;
	void DrawLine(INT x, INT y, INT x2, INT y2, UINT color, UINT LineWidth) OVERRIDE;
	BOOL Text(const uiString& str, uiRect rect, const uiFont& Font, const stTextParam* pParam = nullptr) OVERRIDE;
	BOOL DrawImage(const uiImage& img, const stDrawImageParam& param) OVERRIDE;


protected:

	void OnDestRectChanged(DEST_CHANGE_TYPE dct, void*& ctx) OVERRIDE
	{
#ifdef USE_GDI_CLIPPING
		HDC hMemDC = m_WndDrawDC.GetDC();
		if (dct == DEST_RESTORE)
		{
			VERIFY(::SelectClipRgn(hMemDC, (HRGN)ctx) != ERROR);
			VERIFY(DeleteObject(m_hRgn));
			m_hRgn = (HRGN)ctx;
		}
		else if (dct == DEST_PUSH)
		{
			ctx = m_hRgn;
			m_hRgn = CreateRectRgn(m_RenderDestRect.Left, m_RenderDestRect.Top, m_RenderDestRect.Right, m_RenderDestRect.Bottom);
			VERIFY(::SelectClipRgn(hMemDC, m_hRgn) != ERROR);
		}
#endif
	}

	HWND m_hWnd;
	HDC  m_PaintDC, m_hMemDC;

	uiWndDC m_WndDrawDC;
	HRGN    m_hRgn;
	HBITMAP m_hOldBmp;

	uiFont m_CurFont;

	INT m_TotalBackBuffer, m_CurrentBackBufferIndex;
	uiWndBackBuffer m_BackBuffer[MAX_BACKBUFFER_COUNT];
	UINT m_nWidth, m_nHeight;


};


#ifdef DRAWER_NO_VTABLE_LOOKUP
	typedef uiDrawerT<uiWndDrawer> uiDrawer;
	typedef uiDrawer uiDrawerInsType;
#endif


