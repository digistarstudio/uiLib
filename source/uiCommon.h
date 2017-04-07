

#pragma once


enum DEFAULT_COMMAND_ID
{
	uiID_ANY,

	uiID_CLOSE,
	uiID_MINIMIZE,
	uiID_MAXIMIZE,
	uiID_RESTORE,

	uiID_SYSTEM = 0x00000F00,
	uiID_INVALID = 0xFFFFFFFF,
};


struct uiSize
{
	uiSize():iWidth(0), iHeight(0) {}
	uiSize(INT width, INT height):iWidth(width), iHeight(height) {}

	INT iWidth;
	INT iHeight;
};

struct uiPoint
{
	uiPoint() = default;
	uiPoint(INT ix, INT iy):x(ix), y(iy) {}

	INLINE uiPoint& operator+=(const uiPoint& pt)
	{
		x += pt.x;
		y += pt.y;
		return *this;
	}
	INLINE uiPoint& operator-=(const uiPoint& pt)
	{
		x -= pt.x;
		y -= pt.y;
		return *this;
	}

	friend uiPoint operator-(uiPoint lhs, const uiPoint& rhs)
	{
		lhs.x -= rhs.x;
		lhs.y -= rhs.y;
		return lhs;
	}
	friend uiPoint operator+(uiPoint lhs, const uiPoint& rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		return lhs;
	}

	INT x, y;
};

struct uiRect
{
	uiRect() { Left = Top = Right = Bottom = 0; }
	uiRect(INT left, INT top, INT right, INT bottom)
	{
		Left = left;
		Top = top;
		Right = right;
		Bottom = bottom;
	}
	uiRect(INT width, INT height)
	{
		Left = Top = 0;
		Right = width;
		Bottom = height;
	}
	uiRect(const uiRect& rectIn) = default;


	INLINE BOOL IsValidRect() const { return (Left < Right && Top < Bottom); }

	INLINE void Inflate(INT x, INT y) { Left -= x; Right += x; Top -= y; Bottom += y; }
	INLINE void Inflate(INT l, INT t, INT r, INT b) { Left -= l; Right += r; Top -= t; Bottom += b; }
	INLINE uiRect InflateRV(INT l, INT t, INT r, INT b) const { return uiRect(Left - l, Top - t, Right + r, Bottom + b); }

	INLINE void GetPos(INT &x, INT &y) const { x = Left; y = Top; }
	INLINE void SetPos(INT x, INT y) { Left = x; Top = y; }
	INLINE uiSize GetSize() const { return uiSize(Right - Left, Bottom - Top); }
	INLINE void SetSize(INT iWidth, INT iHeight) { Right = Left + iWidth; Bottom = Top + iHeight; }
	INLINE void Resize(INT nNewWidth, INT nNewHeight) { Right = Left + nNewWidth;  Bottom = Top + nNewHeight; }
	INLINE void Reset() { Left = Top = Right = Bottom = 0; }

	INLINE INT Width() const { return Right - Left; }
	INLINE INT Height() const { return Bottom - Top; }

	INLINE void SetWidth(INT NewWidth) { Right = Left + NewWidth; }
	INLINE void SetHeight(INT NewHeight) { Bottom = Top + NewHeight; }

	INLINE void Init(INT iWidth, INT iHeight) { Left = Top = 0; Right = iWidth; Bottom = iHeight; }
	INLINE void Init(INT left, INT top, INT right, INT bottom) { Left = left; Top = top; Right = right; Bottom = bottom; }
	INLINE void InitForUnion() { Left = Top = MAX_INT_VALUE; Right = Bottom = MIN_INT_VALUE; }
	INLINE void Move(INT x, INT y) { Left += x; Right += x; Top += y; Bottom += y; }
	INLINE void Move(uiPoint& pt) { Left += pt.x; Right += pt.x; Top += pt.y; Bottom += pt.y; }

	INLINE BOOL IsPointIn(INT x, INT y) const { return (Left <= x && x < Right) && (Top <= y && y < Bottom); }
	INLINE BOOL IsPointIn(const uiPoint& pt) const { return (Left <= pt.x && pt.x < Right) && (Top <= pt.y && pt.y < Bottom); }
	INLINE BOOL IsEmpty() const { return (!Left && !Right && !Top && !Bottom); }

	INLINE uiPoint GetLeftTop() const { return uiPoint(Left, Top); }
	INLINE uiPoint GetRightBottom() const { return uiPoint(Right, Bottom); }

	INLINE uiPoint VCenter(INT heightIn) const
	{
		INT offset = (Height() - heightIn) / 2;
		return uiPoint(Top + offset, Bottom - offset);
	}
	INLINE uiPoint HCenter(INT WidthIn) const
	{
		INT offset = (Width() - WidthIn) / 2;
		return uiPoint(Left + offset, Right - offset);
	}

	INLINE void IntersectWith(const uiRect &rectIn) // The rectangle will be invalid if there is no intersection.
	{
		if (Left < rectIn.Left)
			Left = rectIn.Left;
		if (Right > rectIn.Right)
			Right = rectIn.Right;
		if (Top < rectIn.Top)
			Top = rectIn.Top;
		if (Bottom > rectIn.Bottom)
			Bottom = rectIn.Bottom;
	}

	INLINE void UnionWith(const uiRect &rectIn)
	{
		if (Left > rectIn.Left)
			Left = rectIn.Left;
		if (Right < rectIn.Right)
			Right = rectIn.Right;
		if (Top > rectIn.Top)
			Top = rectIn.Top;
		if (Bottom < rectIn.Bottom)
			Bottom = rectIn.Bottom;
	}

	INLINE bool operator==(const uiRect &rectIn) const
	{
		return (Left == rectIn.Left) && (Top == rectIn.Top) && (Right == rectIn.Right) && (Bottom == rectIn.Bottom);
	}
	INLINE bool operator!=(const uiRect &rectIn) const
	{
		return !(*this == rectIn);
	}

	INT Left, Top;
	INT Right, Bottom;

};


enum MENU_ITEM_TYPE
{
	ITEM_SEPARATOR,
	ITEM_MENU,
};


class uiMenu;


struct uiMenuItem
{
	TCHAR *m_String;
	UINT  ID, flag;
	uiMenu *pNext, *reserved;
};


class uiMenu
{
public:

	uiMenu();
	~uiMenu();

	BOOL AddItem(INT index, TCHAR* pStr, UINT type);
	BOOL AppendItem(TCHAR* pStr, UINT type);

	void Release();

	INT GetCount(BOOL bCountSeparator);


protected:

	INT m_iCount;
	std::vector<uiMenuItem> m_ItemArray;

	HANDLE hMenu;


};


class uiString
{
public:

	uiString() :m_pStr(nullptr), m_len(1) {}
	uiString(const TCHAR* pstrIn) :m_pStr(nullptr) { StoreString(pstrIn, 0); }
	uiString(const uiString& src) :m_pStr(nullptr) { StoreString(src.m_pStr, src.Length()); }

	uiString(uiString&& SrcStr)
	:m_pStr(SrcStr.m_pStr), m_len(SrcStr.m_len)
	{
		SrcStr.m_pStr = nullptr;
		SrcStr.m_len = 1;
	}
	~uiString()
	{
		Release();
		ASSERT(m_len == 1);
	}

	INT Format()
	{
	}

	uiString& MakeLower()
	{
		for (UINT i = 0, tail = m_len - 1; i < tail; ++i)
			if ('A' <= m_pStr[i] && m_pStr[i] <= 'Z')
				m_pStr[i] += ('a' - 'A');
		return *this;
	}
	UINT CopyTo(TCHAR* pBuf, UINT nMaxChar) const
	{
		UINT len = m_len;
		if (len > nMaxChar)
		{
			len = nMaxChar - 1;
			memcpy(pBuf, m_pStr, sizeof(TCHAR) * len);
			pBuf[len] = '\0';
		}
		else
		{
			memcpy(pBuf, m_pStr, sizeof(TCHAR) * len);
			--len;
		}
		return len;
	}

	void StoreString(const TCHAR* pStrIn, UINT lenIn)
	{
		if (pStrIn == nullptr || *pStrIn == '\0')
		{
			Release();
			return;
		}

		if (lenIn == 0)
			lenIn = (UINT)_tcslen(pStrIn);

		if (m_pStr == nullptr)
			m_pStr = (TCHAR*)malloc(sizeof(TCHAR) * (lenIn + 1));
		else if (m_len != lenIn + 1)
			m_pStr = (TCHAR*)realloc(m_pStr, sizeof(TCHAR) * (lenIn + 1));

		m_len = lenIn + 1;

		ASSERT(m_pStr != nullptr);
		memcpy(m_pStr, pStrIn, sizeof(TCHAR) * lenIn);
		m_pStr[lenIn] = '\0';
	}

	BOOL CmpRight(UINT len, const TCHAR* strSrc)
	{
		ASSERT(_tcslen(strSrc) >= len);
		if (m_pStr == nullptr || m_len <= len)
			return FALSE;
		TCHAR* Dest = &m_pStr[m_len - 1 - len];
		for (UINT i = 0; i < len; ++i)
			if (Dest[i] != strSrc[i])
				return FALSE;
		return TRUE;
	}

	uiString& operator=(const TCHAR* pStrIn)
	{
		StoreString(pStrIn, 0);
		return *this;
	}
	uiString& operator=(const uiString& src)
	{
		StoreString(src.m_pStr, src.Length());
		return *this;
	}
	uiString& operator=(uiString&& rstr) noexcept
	{
		/*
		uiString& temp = uiString(std::move(rstr));
		std::swap(m_pStr, temp.m_pStr);
		std::swap(m_len, temp.m_len);
		/*/
		std::swap(m_pStr, rstr.m_pStr);
		std::swap(m_len, rstr.m_len);
		rstr.~uiString();
		//*/
		return *this;
	}

	INLINE UINT Length() const { return m_len - 1; }
	INLINE BOOL IsEmpty() const { return (m_len == 1); }
	operator const TCHAR*() const { return (m_pStr == nullptr) ? _T("") : m_pStr; }


	static INT Format(WCHAR* pBuf, INT nMaxCharCount, const TCHAR* format, ...) // Return length donesn't include null terminator.
	{
		ASSERT(pBuf != nullptr);

		va_list vlist;
		INT nCharacters;
		va_start(vlist, format);
		nCharacters = vswprintf_s(pBuf, nMaxCharCount, format, vlist);
		va_end(vlist);
		ASSERT(nCharacters <= nMaxCharCount);
		return nCharacters;
	}
	static INT Format(CHAR* pBuf, INT nMaxCharCount, const CHAR* format, ...) // Return length donesn't include null terminator.
	{
		ASSERT(pBuf != nullptr);

		va_list vlist;
		INT nCharacters;
		va_start(vlist, format);
		nCharacters = vsprintf_s(pBuf, nMaxCharCount, format, vlist);
		va_end(vlist);
		ASSERT(nCharacters <= nMaxCharCount);
		return nCharacters;
	}
	static INT GetBufferLength(const WCHAR* format, ...)  // Return length includes null terminator.
	{
		INT iLen;
		va_list vlist;
		va_start(vlist, format);
		iLen = _vscwprintf(format, vlist) + 1; // _vscprintf doesn't count terminating '\0'
		va_end(vlist);
		return iLen;
	}
	static INT GetBufferLength(const CHAR* format, ...) // Return length includes null terminator.
	{
		INT iLen;
		va_list vlist;
		va_start(vlist, format);
		iLen = _vscprintf(format, vlist) + 1; // _vscprintf doesn't count terminating '\0'
		va_end(vlist);
		return iLen;
	}


protected:

	INLINE void Release()
	{
		if (m_pStr != nullptr)
		{
			free(m_pStr);
			m_pStr = nullptr; // Must reset now.
			m_len = 1;
		}
	}


	TCHAR *m_pStr;
	UINT  m_len; // Null-terminator is always included.


};


UINT uiGetID(INT count = 1);
void uiReleaseID(UINT id);


