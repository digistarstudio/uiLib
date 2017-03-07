

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
	uiPoint(INT ix, INT iy):x(ix), y(iy) {}
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
	uiRect(const uiRect& rectIn) = default;


	INLINE void Inflate(INT x, INT y) { Left -= x; Right += x; Top -= y; Bottom += y; }
	INLINE void Inflate(INT l, INT t, INT r, INT b) { Left -= l; Right += r; Top -= t; Bottom += b; }

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
	INLINE void InitForUnion() { Left = Top = MAX_INT_VALUE; Right = Bottom = MIN_INT_VALUE; }
	INLINE void Move(INT x, INT y) { Left += x; Right += x; Top += y; Bottom += y; }

	INLINE BOOL IsPointIn(INT x, INT y) const { return (Left <= x && x < Right) && (Top <= y && y < Bottom); }
	INLINE BOOL IsEmpty() const { return (!Left && !Right && !Top && !Bottom); }

	INLINE uiPoint GetLeftTop() const { return uiPoint(Left, Top); }
	INLINE uiPoint GetRightBottom() const { return uiPoint(Right, Bottom); }

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
	INLINE BOOL IsValidRect() const { return (Left < Right && Top < Bottom); }

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

	BOOL AddItem(INT index, TCHAR *pStr, UINT type);
	BOOL AppendItem(TCHAR *pStr, UINT type);

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

	uiString():m_pStr(nullptr) {}
	uiString(const uiString &src) { StoreString(src.m_pStr, src.Length()); }

	uiString(uiString &&rvalue)
	{
		m_pStr = rvalue.m_pStr;
		m_len = rvalue.m_len;
		rvalue.m_pStr = nullptr;
		rvalue.m_len = 0;
	}
	~uiString()
	{
		if (m_pStr != nullptr)
			free(m_pStr);
	}

	INT Format()
	{



	}

	INLINE INT Length() const { return m_len - 1; }
	INLINE BOOL IsEmpty() const { return (m_len <= 1); }

	void StoreString(const TCHAR* pStrIn, UINT lenIn)
	{
		if (pStrIn != nullptr && lenIn == 0)
			lenIn = _tcslen(pStrIn);
		if (m_pStr == nullptr)
		{
			m_len = lenIn + 1;
			m_pStr = (TCHAR*)malloc(sizeof(TCHAR) * m_len);
		}
		else if (m_len != lenIn + 1)
		{
			m_pStr = (TCHAR*)realloc(m_pStr, sizeof(TCHAR) * (lenIn + 1));
			assert(m_pStr != nullptr);
			m_len = lenIn + 1;
		}
		memcpy(m_pStr, pStrIn, sizeof(TCHAR) * lenIn);
		m_pStr[lenIn] = '\0';
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
	uiString& operator=(uiString &&rstr)
	{
	//	printx("---> Move assignment called!\n");
		if (m_pStr != nullptr)
			free(m_pStr);
		m_pStr = rstr.m_pStr;
		m_len = rstr.m_len;
		rstr.m_pStr = nullptr; // Must clean the pointer. 
		rstr.m_len = 0;
		return *this;
	}

	operator const TCHAR*() const { return (m_pStr == nullptr) ? _T("") : m_pStr; }


protected:

	TCHAR *m_pStr;
	INT m_len; // Null-terminator is included.


};


UINT uiGetID(INT count = 1);
void uiReleaseID(UINT id);


