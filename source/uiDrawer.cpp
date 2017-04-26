

#include "stdafx.h"
#include "uiDrawer.h"
#include "MsWinHelper.h"
#include "uiApp.h"


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

		case SCN_INACTIVECAPTION:
			color = ::GetSysColor(COLOR_INACTIVECAPTION);
			break;
		case SCN_FRAME:
		//	color = RGB();
			color = ::GetSysColor(COLOR_ACTIVECAPTION);
			color = ::GetSysColor(COLOR_3DHILIGHT);
			color = ::GetSysColor(COLOR_3DLIGHT);
			break;
	}

	return color;
}


uiFont GSystemFont[SFT_TOTAL];

void InitSystemFont()
{
	NONCLIENTMETRICS ncm;
	ZeroMemory(&ncm, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0) == 0)
	{
		printx("ec: %d\n", GetLastError());
		return;
	}

	VERIFY(GSystemFont[SFT_CAPTION].Create(ncm.lfCaptionFont));
	VERIFY(GSystemFont[SFT_SM_CAPTION].Create(ncm.lfSmCaptionFont));
	VERIFY(GSystemFont[SFT_MENU].Create(ncm.lfMenuFont));
	VERIFY(GSystemFont[SFT_STATUS].Create(ncm.lfStatusFont));
	VERIFY(GSystemFont[SFT_MESSAGE].Create(ncm.lfMessageFont));
}

void ReleaseSystemFont()
{
	for (INT i = 0; i < SFT_TOTAL; ++i)
	{
		GSystemFont[i].Release();
		ASSERT(!GSystemFont[i].IsValid());
	}
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

	LOGFONT lf = { 0 };
	lf.lfWidth = width;
	lf.lfHeight = height;
	str.CopyTo(lf.lfFaceName, _countof(lf.lfFaceName));

	return Create(lf);
}

BOOL uiFont::Create(const LOGFONT& logfont)
{
	size_t key = HashState(&logfont);
	auto it = winFontHandleType::HandleMap.find(key);
	if (it != winFontHandleType::HandleMap.end())
	{
		if ((m_ptrHandle = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HANDLE hFont = CreateFontIndirect(&logfont);
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


BOOL uiImage::LoadCursorRes(UINT ResID, const TCHAR* ResType, HINSTANCE hModule)
{
	stImageSourceInfo isi = { (hModule == NULL) ? uiGetAppIns() : hModule, ResID };
	const size_t Key = HashState(&isi);

	auto it = stImgHandleWrapper::KeyHandleMap.find(Key);
	if (it != stImgHandleWrapper::KeyHandleMap.end())
	{
		if ((m_pImg = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HCURSOR hCursor = NULL;
	if (ResType == nullptr)
		hCursor = (HCURSOR)LoadImage(isi.hModule, MAKEINTRESOURCE(ResID), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR);
	else
	{
		// It looks like these two old win32 functions are not compatiable with new animation format.
		// HICON hCursor = CreateIconFromResourceEx(pAddr, dwSize, FALSE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
		// HCURSOR hCursor = ::LoadCursor(isi.hModule, MAKEINTRESOURCE(ResID));
		uiString TempPath;
		if (SaveResourceToTempFile(TempPath, _T("uiTempCursor"), ResID, ResType, isi.hModule))
		{
			hCursor = (HCURSOR)LoadImage(NULL, TempPath, IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
			::DeleteFile(TempPath);
		}
	}

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

BOOL uiImage::LoadIconRes(UINT ResID, HINSTANCE hModule)
{
	stImageSourceInfo isi = { (hModule == NULL) ? uiGetAppIns() : hModule, ResID };
	const size_t Key = HashState(&isi);

	auto it = stImgHandleWrapper::KeyHandleMap.find(Key);
	if (it != stImgHandleWrapper::KeyHandleMap.end())
	{
		if ((m_pImg = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HCURSOR hIcon = (HICON)LoadImage(isi.hModule, MAKEINTRESOURCE(ResID), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
	if (hIcon == NULL)
	{
		printx("LoadImage (ICON) failed! ec: %d\n", GetLastError());
		return FALSE;
	}

	m_pImg = std::shared_ptr<stImgHandleWrapper>(new stImgHandleWrapper(WIT_ICON, hIcon, Key));
	if (m_pImg.get() == nullptr)
	{
		VERIFY(stImgHandleWrapper::m_Del[WIT_ICON](hIcon));
		return FALSE;
	}

	stImgHandleWrapper::KeyHandleMap.insert({ Key, m_pImg });

	return TRUE;
}

static INLINE UINT GetLoadImageType(WIN_IMAGE_TYPE wit)
{
	ASSERT(wit < WIT_TOTAL);
	ASSERT(WIT_BMP == 0 && WIT_CURSOR == 1 && WIT_ICON == 2);
	static UINT type[] = { IMAGE_BITMAP, IMAGE_CURSOR, IMAGE_ICON };
	return type[wit];
}

BOOL uiImage::LoadFromFile(uiString str)
{
	WIN_IMAGE_TYPE wit = GetType(str.MakeLower());
	if (wit == WIT_NONE)
		return FALSE;

	auto it = stImgHandleWrapper::PathHandleMap.find(str);
	if (it != stImgHandleWrapper::PathHandleMap.end())
	{
		if ((m_pImg = it->second.lock()).get() != nullptr)
			return TRUE;
		ASSERT(0); // Only single thread is supported.
	}

	HANDLE hHandle = LoadImage(NULL, str, GetLoadImageType(wit), 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE);
	if (hHandle == NULL)
	{
		printx("LoadImage (Type: %d) failed! ec: %d\n", wit, GetLastError());
		return FALSE;
	}

	m_pImg = std::shared_ptr<stImgHandleWrapper>(new stImgHandleWrapper(wit, hHandle));
	if (m_pImg.get() == nullptr)
	{
		VERIFY(stImgHandleWrapper::m_Del[wit](hHandle));
		return FALSE;
	}

	it = stImgHandleWrapper::PathHandleMap.insert({ str, m_pImg }).first;
	m_pImg->Set(it, std::move(str));

	return TRUE;
}

WIN_IMAGE_TYPE uiImage::GetType(uiString& str)
{
	const INT len = 3;
	if (str.CmpRight(len, _T("bmp")))
		return WIT_BMP;
	if (str.CmpRight(len, _T("cur")) || str.CmpRight(len, _T("ani")))
		return WIT_CURSOR;
	if (str.CmpRight(len, _T("ico")))
		return WIT_ICON;
	return WIT_NONE;
}

BOOL uiImage::GetInfo(stImageInfo& ImgInfo)
{
	if (!IsValid())
		return FALSE;

	WIN_IMAGE_TYPE wit = GetType();

	if (wit == WIT_BMP)
	{
		BITMAP bm = { 0 };
		VERIFY(::GetObject((HBITMAP)GetHandle(), sizeof(bm), &bm));
		ImgInfo.Width = bm.bmWidth;
		ImgInfo.Height = bm.bmHeight;
		return TRUE;
	}

	ICONINFO ii;
	HICON hIcon = (HICON)GetHandle();
	if (::GetIconInfo(hIcon, &ii))
	{
		if (!ii.fIcon) // is cursor
		{
			GetCursorFrameInfo(hIcon, (DWORD&)ImgInfo.DispRate, (DWORD&)ImgInfo.TotalFrame);
			ImgInfo.HotSpotX = ii.xHotspot;
			ImgInfo.HotSpotY = ii.yHotspot;
		}

		BITMAP bm = { 0 };
		GetObject(ii.hbmColor, sizeof(bm), &bm);
		ImgInfo.Width = bm.bmWidth;
		ImgInfo.Height = bm.bmHeight;

		::DeleteObject(ii.hbmColor);
		::DeleteObject(ii.hbmMask);
	}

	return FALSE;
}


uiGDIObjCacher::~uiGDIObjCacher()
{
	ASSERT(HandleMap.size() == 0);
	ASSERT(CurCacheCount == 0);
}

HGDIOBJ uiGDIObjCacher::Find(const stGDIObjectName& ObjName)
{
	const size_t key = HashState(&ObjName);

	auto it = HandleMap.find(key);
	if (it != HandleMap.end())
	{
		it->second->RemoveToHead(ListHead);
		return it->second->pData1;
	}

	stSimpleListNode* pListNode;
	if (CurCacheCount < MaxCacheCount)
	{
		pListNode = stSimpleListNode::alloc();
		++CurCacheCount;
	}
	else
	{
		pListNode = (stSimpleListNode*)LIST_GET_TAIL(ListHead);
		pListNode->Detach();
		HandleMap.erase((size_t)pListNode->pData2);
		VERIFY(::DeleteObject(pListNode->pData1));
	}

	HGDIOBJ hObj;
	switch (ObjName.ot)
	{
	case TYPE_BRUSH:
		hObj = ::CreateSolidBrush(ObjName.BrushColor);
		break;

	case TYPE_PEN:
		hObj = ::CreatePen(ObjName.Style, ObjName.Width, ObjName.PenColor);
		break;

	case TYPE_REGION:
		ASSERT(0);
		break;
	}

	if (hObj == NULL)
	{
		printx("Failed to creat GDI object(%d)! ec: %d\n", ObjName.ot, GetLastError());
		stSimpleListNode::mfree(pListNode);
		--CurCacheCount;
		return NULL;
	}

	pListNode->pData1 = hObj;
	pListNode->pData2 = (void*)key;
	list_add(*pListNode, &ListHead);

	HandleMap.insert({ key, pListNode });

	return hObj;
}

void uiGDIObjCacher::Release()
{
	for (auto it = HandleMap.begin(); it != HandleMap.end(); ++it)
	{
		VERIFY(::DeleteObject(it->second->pData1));
		stSimpleListNode::mfree(it->second);
		--CurCacheCount;
	}

	HandleMap.clear();
}


UINT uiGDIObjCacher::MaxCacheCount = 0;
UINT uiGDIObjCacher::CurCacheCount = 0;
list_head uiGDIObjCacher::ListHead;
std::unordered_map<std::size_t, stSimpleListNode*> uiGDIObjCacher::HandleMap;


uiGDIObjCacher GGDIObjCacher;


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

	m_hMemDC = ::CreateCompatibleDC(NULL);
	m_hOldBmp = (HBITMAP)::GetCurrentObject(m_hMemDC, OBJ_BITMAP);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	return TRUE;
}

BOOL uiWndDrawer::Begin(void* pCtx)
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

	PAINTSTRUCT* ps = (PAINTSTRUCT*)pCtx;
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

void uiWndDrawer::End(void* pCtx)
{
	ASSERT(m_OriginX == 0 && m_OriginY == 0);
	ASSERT(m_hRgn == NULL);
	ASSERT(m_CurDepth == 1);

	if (m_TotalBackBuffer > 0)
	{
		VERIFY(::BitBlt(m_PaintDC, 0, 0, m_nWidth, m_nHeight, m_WndDrawDC.GetDC(), 0, 0, SRCCOPY));
		if (++m_CurrentBackBufferIndex == m_TotalBackBuffer)
			m_CurrentBackBufferIndex = 0;
	}

	m_CurFont.Release();
	m_WndDrawDC.Detach();
	VERIFY(::SelectObject(m_hMemDC, m_hOldBmp) != NULL);

	if (pCtx != nullptr)
		EndPaint(m_hWnd, (PAINTSTRUCT*)pCtx);

	m_PaintDC = NULL;
}

BOOL uiWndDrawer::BeginCache(const uiRect& rect)
{
	ASSERT(m_RectList.GetCounts() == 0);
	ASSERT(m_PaintDC == NULL);
	ASSERT(rect.IsValidRect());

	m_RenderDestRect = rect;

	HDC hDC = ::GetDC(m_hWnd);
	VERIFY(hDC != NULL);

	if (m_TotalBackBuffer == 0)
		m_WndDrawDC.Attach(hDC);
	else
	{
		if (--m_CurrentBackBufferIndex < 0)
			m_CurrentBackBufferIndex = m_TotalBackBuffer - 1;

		m_WndDrawDC.Attach(m_BackBuffer[m_CurrentBackBufferIndex].GetMemDC());
	}

	m_PaintDC = hDC;

	return TRUE;
}

BOOL uiWndDrawer::EndCache()
{
	ASSERT(m_OriginX == 0 && m_OriginY == 0);
	ASSERT(m_hRgn == NULL);
	ASSERT(m_CurDepth == 1);

	if (m_TotalBackBuffer > 0)
	{
		VERIFY(::BitBlt(m_PaintDC, 0, 0, m_nWidth, m_nHeight, m_WndDrawDC.GetDC(), 0, 0, SRCCOPY));
		if (++m_CurrentBackBufferIndex == m_TotalBackBuffer)
			m_CurrentBackBufferIndex = 0;
	}

	m_CurFont.Release();
	m_WndDrawDC.Detach();
	VERIFY(::SelectObject(m_hMemDC, m_hOldBmp) != NULL);
	VERIFY(::ReleaseDC(m_hWnd, m_PaintDC) == 1);

	m_PaintDC = NULL;

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
	UpdateCoordinate(rect);

	m_WndDrawDC.SetBrush(color);
	m_WndDrawDC.FillRect((RECT*)&rect);
}

void uiWndDrawer::RoundRect(uiRect rect, UINT32 color, INT width, INT height)
{
	UpdateCoordinate(rect);

	m_WndDrawDC.SetPen(1, color);
	m_WndDrawDC.SetHollowBrush();
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
	UpdateCoordinate(x, y);
	UpdateCoordinate(x2, y2);

	m_WndDrawDC.SetPen(LineWidth, color, PS_SOLID);
	::MoveToEx(m_WndDrawDC.GetDC(), x, y, nullptr);
	::LineTo(m_WndDrawDC.GetDC(), x2, y2);
}

BOOL uiWndDrawer::Text(const uiString& str, uiRect rect, const uiFont& Font, const stTextParam* pParam)
{
	if (!Font.IsValid())
		return FALSE;

	UpdateCoordinate(rect);

	HDC hDestDC = m_WndDrawDC.GetDC();
	::SetBkMode(hDestDC, TRANSPARENT);
	::SetTextColor(hDestDC, pParam == nullptr ? RGB(0, 0, 0) : pParam->color);

	if (Font != m_CurFont)
	{
		m_CurFont = Font;
		VERIFY(::SelectObject(hDestDC, Font.GetHandle()) != NULL);
	}

	//BOOL bRet = ::DrawText(hDestDC, str, str.Length(), (LPRECT)&rect, flag);
	BOOL bRet = ::TextOut(hDestDC, rect.Left, rect.Top, str, str.Length());

	return bRet;
}

BOOL uiWndDrawer::DrawImage(const uiImage& img, const stDrawImageParam& param)
{
	BOOL bRet;
	HDC hDestDC = m_WndDrawDC.GetDC();
	WIN_IMAGE_TYPE wit = img.GetType();

	INT x = param.x, y = param.y;
	UpdateCoordinate(x, y);

	switch (wit)
	{
	case WIT_BMP:
		if(bRet = (::SelectObject(m_hMemDC, img.GetHandle()) != NULL))
			bRet = m_WndDrawDC.BitBlt(m_hMemDC, x, y, param.width, param.height, 0, 0, NOTSRCCOPY);
		break;

	case WIT_CURSOR:
	case WIT_ICON:
	//	bRet = DrawIcon(hDestDC, x, y, (HICON)img.GetHandle());
		bRet = DrawIconEx(hDestDC, x, y, (HICON)img.GetHandle(), param.width, param.height, param.aniIndex, NULL, DI_NORMAL | DI_COMPAT | DI_MASK);
		break;

	default:
		ASSERT(0);
	}

	if (!bRet)
	{
		printx("Warning! uiWndDrawer::DrawImage failed! type: %d, ec: %d\n", wit, GetLastError());
	//	ASSERT(0);
	}

	return bRet;
}


