

#include "stdafx.h"
#include "uiMsWin.h"

#include <Windowsx.h>


#define WM_CUSTOM   (WM_USER + 0x0001)
#define WM_CTRL_MSG (WM_USER + 0x0002)


std::map<void*, uiWindow*> GWindowsHandleMap;


namespace UICore
{
	list_head RootList;
	BOOL bSilentMode = FALSE; // Don't create the window using os api if this is true.

	uiWindow *pGAppBaseWindow = nullptr;

	struct INIT_UI
	{
		INIT_UI()
		{
			INIT_LIST_HEAD(&RootList);
		}
		~INIT_UI()
		{
			ASSERT(GWindowsHandleMap.size() == 0);
		}

	};

	INIT_UI init_ui;
	uiWinCursor GCursor;

	uiFormStyle GFormStyle = { 4, 0, 0, 0, 0 };
}

static uiWinCursor& uiGetCursor() { return UICore::GCursor; }

static uiWindow* WndFindByHandle(void* handle)
{
	auto it = GWindowsHandleMap.find(handle);
	ASSERT(it != GWindowsHandleMap.end());
	return it->second;
}

static void WndRemoveMap(void* handle)
{
	auto it = GWindowsHandleMap.find(handle);
	ASSERT(it != GWindowsHandleMap.end());
	GWindowsHandleMap.erase(it);
}

static void WndAddMap(void* handle, uiWindow* pWnd)
{
#ifdef _DEBUG
	ASSERT(GWindowsHandleMap.find(handle) == GWindowsHandleMap.end());
#endif
	GWindowsHandleMap[handle] = pWnd;
}


#define LogWndMsg printx


static INLINE uiWindow* uiWindowGet(HWND hWnd)
{
	return (uiWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}

#define MOUSE_KEY_DOWN(mkt) { pWnd = uiWindowGet(hWnd); pWnd->OnMouseBtnDown(mkt, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); bProcessed = TRUE; break; }
#define MOUSE_KEY_UP(mkt) { pWnd = uiWindowGet(hWnd); pWnd->OnMouseBtnUp(mkt, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); bProcessed = TRUE; break; }
#define MOUSE_KEY_DBCLK(mkt) { pWnd = uiWindowGet(hWnd); pWnd->OnMouseBtnDbClk(mkt, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); bProcessed = TRUE; break; }

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	LRESULT lRet = 0;
	BOOL bProcessed = FALSE;
	uiWindow *pWnd = nullptr;
	uiFormBase *pForm;
	//	RECT rect;

	switch (message)
	{
	case WM_CREATE:
		pWnd = (uiWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
		pWnd->SetHandle(hWnd);
		WndAddMap(hWnd, pWnd);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
		pWnd->OnCreate();
		bProcessed = TRUE;
		break;

	case WM_DESTROY:
		LogWndMsg("Msg: WM_DESTROY wParam:%p lParam:%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		if (pWnd == UICore::pGAppBaseWindow)
		{
			PostQuitMessage(0);
			UICore::pGAppBaseWindow = nullptr;
		}
		WndRemoveMap(hWnd);
		pWnd->SetHandle(NULL);
		delete pWnd;
		return 0;

	case WM_MOVE:
	//	LogWndMsg("Msg: WM_MOVE Type:%d X:%d Y:%d\n", wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_SIZE: // This is sent after size has changed.
	//	LogWndMsg("Msg: WM_SIZE Type:%d Width:%d Height:%d\n", wParam, LOWORD(lParam), HIWORD(lParam));
		pWnd = uiWindowGet(hWnd);
		pWnd->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		bProcessed = TRUE;
		break;

	case WM_ACTIVATE:
		LogWndMsg("Msg: WM_ACTIVATE 0x%p 0x%p\n", wParam, lParam);
		break;

	case WM_SETFOCUS:
	//	LogWndMsg("Msg: WM_SETFOCUS 0x%p 0x%p\n", wParam, lParam);
		break;

	case WM_KILLFOCUS:
	//	LogWndMsg("Msg: WM_KILLFOCUS 0x%p 0x%p\n", wParam, lParam);
		break;

	case WM_PAINT:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnPaint();
		break;

	case WM_CLOSE:
		LogWndMsg("Msg: WM_CLOSE\n");
	//	pWnd = uiWindowGet(hWnd);
	//	if (pWnd->OnClose())
			DestroyWindow(hWnd);
		break;

	case WM_ACTIVATEAPP:
		LogWndMsg("Msg: WM_ACTIVATEAPP 0x%p 0x%p\n", wParam, lParam);
		break;

	case WM_SETCURSOR: // If an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
		pWnd = uiWindowGet(hWnd);
		lRet = pWnd->OnSetCursor();
		bProcessed = TRUE;
		break;
//*
	case WM_NCHITTEST:
	//	LogWndMsg("Msg: WM_NCHITTEST x:%d y:%d\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = uiWindowGet(hWnd);
		pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		ScreenToClient(hWnd, &pt);
	//	LogWndMsg("Msg: WM_NCHITTEST x:%d y:%d\n", pt.x, pt.y);
		lRet = (LRESULT)pWnd->OnNCHitTest(pt.x, pt.y);
		bProcessed = TRUE;
		break;
		//*/
/*
	case WM_NCPAINT:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnNCPaint(hWnd, (HRGN)wParam);
		bProcessed = TRUE;
		break; //*/

	case WM_KEYDOWN:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnKeyDown(0);
		break;

	case WM_KEYUP:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnKeyUp(0);
		break;

	case WM_COMMAND:
	//	INT src = HIWORD(wParam);
		switch (wParam)
		{
		case 0:
			printx("menu command!\n");
			break;
		}
		break;

	case WM_MENUCOMMAND:
		printx("menu handle: %p, index\n", lParam, wParam);
		break;

	case WM_MOUSEMOVE:
	//	LogWndMsg("Msg: WM_MOUSEMOVE x:%d y:%d\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;

	case WM_LBUTTONDOWN:
		MOUSE_KEY_DOWN(MKT_LEFT);
	case WM_LBUTTONUP:
		MOUSE_KEY_UP(MKT_LEFT);
	case WM_LBUTTONDBLCLK:
		MOUSE_KEY_DBCLK(MKT_LEFT);
	case WM_RBUTTONDOWN:
		MOUSE_KEY_DOWN(MKT_RIGHT);
	case WM_RBUTTONUP:
		MOUSE_KEY_UP(MKT_RIGHT);
	case WM_RBUTTONDBLCLK:
		MOUSE_KEY_DBCLK(MKT_RIGHT);
	case WM_MBUTTONDOWN:
		MOUSE_KEY_DOWN(MKT_MIDDLE);
	case WM_MBUTTONUP:
		MOUSE_KEY_UP(MKT_MIDDLE);
	case WM_MBUTTONDBLCLK:
		MOUSE_KEY_DBCLK(MKT_MIDDLE);

	case WM_SIZING:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnSizing(wParam, (RECT*)lParam);
		bProcessed = TRUE;
		lRet = TRUE;
		break;

	case WM_CAPTURECHANGED:
		LogWndMsg("Msg: WM_CAPTURECHANGED HWND: %p\n", lParam);
		break;

	case WM_MOUSELEAVE:
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		LogWndMsg("Msg: WM_MOUSELEAVE x:%d y:%d\n", pt.x, pt.y);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseLeave();
		bProcessed = TRUE;
		break;

	case WM_CUSTOM:
		LogWndMsg("Msg: WM_CUSTOM\n");
		break;

	case WM_CTRL_MSG:
		pForm = (uiFormBase*)wParam;
		pForm->EntryOnCommand(lParam);
		break;
	}

	if (pWnd != nullptr)
		pWnd->PostMsgHandler(message);
	if (bProcessed)
		return lRet;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase *pForm, uiFormBase *ParentForm, INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight)
{
	static BOOL bRegistered = FALSE;
	const TCHAR *pWndClass = _T("WndClass");
	HINSTANCE hInstance = GetModuleHandle(NULL);

	if (!bRegistered)
	{
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = NULL; //(HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = pWndClass;
		wcex.hIconSm = NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		RegisterClassExW(&wcex);
		bRegistered = TRUE;
	}

	HWND hParent = (ParentForm == nullptr) ? NULL : (HWND)ParentForm->GetBaseWnd()->GetHandle();
	RECT r = { x, y, (LONG)(x + nWidth), (LONG)(x + nHeight) };
	DWORD ExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, Style = WS_POPUP /*| WS_SIZEBOX | WS_MAXIMIZEBOX*/;
	//	DWORD ExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, Style = WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_SYSMENU;

	switch (uwt)
	{
	case UWT_NORMAL:
		break;
	case UWT_TOOL:
		ASSERT(hParent != NULL);
	//	Style = WS_POPUPWINDOW | WS_CAPTION;
		ExStyle = WS_EX_TOOLWINDOW;
		break;
	case UWT_MENU:
		break;
	}

	AdjustWindowRectEx(&r, Style, FALSE, ExStyle);
	nWidth = r.right - r.left;
	nHeight = r.bottom - r.top;

	uiWindow *pWnd = new uiWindow(pForm);
	if (pWnd != nullptr)
	{
		if (UICore::pGAppBaseWindow == nullptr)
		{
			UICore::pGAppBaseWindow = pWnd;
			pForm->SetAsBase();
		}

		pForm->SetWindow(pWnd);

		HWND hWnd = CreateWindowEx(ExStyle, pWndClass, nullptr, Style, r.left, r.top, nWidth, nHeight, hParent, NULL, hInstance, pWnd);
		if (hWnd != NULL)
			return pWnd;

		delete pWnd;
	}

	return FALSE;
}


BOOL WndClientToScreen(uiWindow *pWnd, INT &x, INT &y)
{
	pWnd->ClientToScreent(x, y);
	return TRUE;
}

BOOL WndCreateMessage(uiWindow *pWnd, uiFormBase *pSrc, UINT id)
{
	BOOL bResult = (pWnd->PostMessage(WM_CTRL_MSG, (WPARAM)pSrc, id) != 0);
	VERIFY(bResult);
	return bResult;
}


uiWindow::uiWindow(uiFormBase *pFormIn)
{
	m_Handle = NULL;
	m_pForm = pFormIn;
	m_pHoverForm = nullptr;
	m_pDraggingForm = nullptr;

	m_TrackMouseClick = 0;
	m_NonClientArea = 0;
	m_LastMousePos.x = -1;
	m_LastMousePos.y = -1;

	ZeroMemory(m_MouseKeyDownTime, sizeof(m_MouseKeyDownTime));
	ZeroMemory(m_MouseClickTime, sizeof(m_MouseClickTime));
	ZeroMemory(m_pFirstClickedForm, sizeof(m_pFirstClickedForm));

	m_bTrackMouseLeave = false;
	m_bDragging = false;
	m_bClosed = false;
}

uiWindow::~uiWindow()
{
	ASSERT(m_Handle == NULL);
	ASSERT(m_pForm == nullptr);
}

BOOL uiWindow::GetUiRect(uiRect &rect)
{
	RECT wrect;
	if (GetClientRect((HWND)m_Handle, &wrect) != 0)
	{
		rect.Left = wrect.left;
		rect.Top = wrect.top;
		rect.Right = wrect.right;
		rect.Bottom = wrect.bottom;
		return TRUE;
	}
	return FALSE;
}

void uiWindow::OnFormDestroy(uiFormBase *pForm)
{
	if (m_pHoverForm == pForm)
	{
		m_pHoverForm = nullptr;
		bRetrackMouse = true;
	}
	if (m_pForm == pForm)
	{
		m_pForm = nullptr;
		bRetrackMouse = false;
	}
}

void uiWindow::OnFormHide(uiFormBase *pForm)
{
}

void uiWindow::CloseImp()
{
//	TrackMouseLeave(FALSE);
//	::DestroyWindow((HWND)m_Handle); // Don't call this here.
	::PostMessage((HWND)m_Handle, WM_CLOSE, NULL, NULL);
//	PostMessage((HWND)m_Handle, WM_DESTROY, NULL, NULL); // This won't work for tool windows.
}

BOOL uiWindow::MoveImp(INT x, INT y)
{
	return (SetWindowPos((HWND)m_Handle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER) != 0);
}

BOOL uiWindow::MoveByOffsetImp(INT x, INT y)
{
	POINT pt = { 0, 0 };
	::ClientToScreen((HWND)m_Handle, &pt);
	return (SetWindowPos((HWND)m_Handle, NULL, pt.x + x, pt.y + y, 0, 0, SWP_NOSIZE | SWP_NOZORDER) != 0);
}

void uiWindow::ShowImp(FORM_SHOW_MODE sm)
{
	INT iCmdShow = SW_SHOW;

	switch (sm)
	{
	case FSM_HIDE:
		iCmdShow = SW_HIDE;
		break;
	case FSM_SHOW:
		break;
	case FSM_RESTORE:
		iCmdShow = SW_RESTORE;
		break;
	case FSM_MINIMIZE:
		iCmdShow = SW_MINIMIZE;
		break;
	case FSM_MAXIMIZE:
		iCmdShow = SW_MAXIMIZE;
		break;
	}

	ShowWindow((HWND)m_Handle, iCmdShow);
}

BOOL uiWindow::SizeImp(UINT nWidth, UINT nHeight)
{
	//	UINT Flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE;
	return (SetWindowPos((HWND)m_Handle, NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER) != 0);
}

void uiWindow::StartDraggingImp(uiFormBase *pForm, INT x, INT y, uiRect MouseMoveRect)
{
	if (pForm == m_pForm)
		::PostMessage((HWND)m_Handle, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(x, y));
	else
	{
		m_bDragging = true;
		m_pDraggingForm = pForm;
		m_MDPosX = x;
		m_MDPosY = y;

		POINT pt = { 0, 0 };
		::ClientToScreen((HWND)m_Handle, &pt);
		MouseMoveRect.Move(pt.x, pt.y);
		ClipCursor((RECT*)&MouseMoveRect);
	}
}

void uiWindow::RedrawImp(const uiRect* pRect)
{
	InvalidateRect((HWND)m_Handle, (const RECT*)pRect, FALSE);
}

void uiWindow::ResizeImp(INT x, INT y)
{
	INT iRegion;

	switch (m_NonClientArea)
	{
//	case uiFormBase::NCHT_CLIENT:
//		iRegion = HTCLIENT;
//		break;
	case uiFormBase::NCHT_TOP:
		iRegion = HTTOP;
		break;
	case uiFormBase::NCHT_BOTTOM:
		iRegion = HTBOTTOM;
		break;
	case uiFormBase::NCHT_LEFT:
		iRegion = HTLEFT;
		break;
	case uiFormBase::NCHT_RIGHT:
		iRegion = HTRIGHT;
		break;
	case uiFormBase::NCHT_TOP | uiFormBase::NCHT_LEFT:
		iRegion = HTTOPLEFT;
		break;
	case uiFormBase::NCHT_TOP | uiFormBase::NCHT_RIGHT:
		iRegion = HTTOPRIGHT;
		break;
	case uiFormBase::NCHT_BOTTOM | uiFormBase::NCHT_LEFT:
		iRegion = HTBOTTOMLEFT;
		break;
	case uiFormBase::NCHT_BOTTOM | uiFormBase::NCHT_RIGHT:
		iRegion = HTBOTTOMRIGHT;
		break;
	}

	::PostMessage((HWND)m_Handle, WM_NCLBUTTONDOWN, iRegion, MAKELPARAM(x, y));
}

void uiWindow::PostMsgHandler(UINT msg)
{
	if (bRetrackMouse)
	{
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient((HWND)m_Handle, &pt);
		INT DestX, DestY;

		do
		{
			bRetrackMouse = false;
			m_pHoverForm = m_pForm->FindByPos(pt.x, pt.y, &DestX, &DestY);
			MouseEnterForm(m_pHoverForm);
		} while (bRetrackMouse);
	}

#ifdef DEBUG
#endif
}

BOOL uiWindow::OnClose()
{
	if (m_pForm->OnClose())
	{
		if (m_pForm->m_pParent != nullptr)
			m_pForm->DetachChild(m_pForm);
		m_pForm->EntryOnDestroy(this); // m_pForm will delete itself.
		m_pForm = nullptr;
		return TRUE;
	}
	return FALSE;
}

void uiWindow::OnCreate()
{
	//	printx("---> uiWindow::OnCreate\n");

	uiRect rect;
	GetUiRect(rect);
	m_Drawer.InitBackBuffer((HWND)m_Handle, rect.Width(), rect.Height());

	//	TrackMouseLeave(); // Not necessary here.
}

void uiWindow::OnDestroy()
{
	delete this;
}

void uiWindow::OnKeyDown(INT iKey)
{
	//	printx("---> uiWindow::OnKeyDown\n");
}

void uiWindow::OnKeyUp(INT iKey)
{
	//	printx("---> uiWindow::OnKeyUp\n");
}

void uiWindow::OnMove()
{
}

void uiWindow::OnNCPaint(HWND hWnd, HRGN hRgn)
{
	HDC hdc;
	RECT rect;
	RGNDATA rdata;

	//	VERIFY(GetRgnBox(hRgn, &rect) != 0);
	//	GetRgnBox(hRgn, &rect);
	//	DWORD ec = GetLastError();

	//VERIFY(GetRegionData(hRgn, sizeof(rdata), &rdata) == 0);

	GetWindowRect(hWnd, &rect);
	//	hdc = GetDCEx(hWnd, 0, DCX_WINDOW);
	hdc = GetWindowDC(hWnd);


	//HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, );

	FillRect(hdc, &rect, (HBRUSH)::GetStockObject(WHITE_BRUSH));

	ReleaseDC(hWnd, hdc);
}

void uiWindow::OnPaint()
{
	uiRect rect;
	GetUiRect(rect);
	ASSERT(rect.Left == 0 && rect.Top == 0);

	PAINTSTRUCT ps;
	m_Drawer.Begin(&ps);
	if (m_Drawer.PushDestRect(rect))
	{
		m_pForm->EntryOnPaint(&m_Drawer, 1);
		m_Drawer.PopDestRect();
	}
	m_Drawer.End(&ps);
}

BOOL uiWindow::OnSetCursor()
{
	//	printx("---> uiWindow::OnSetCursor.\n");

	if (bChangeCursor)
	{
		uiGetCursor().Update();
		bChangeCursor = false;
		return TRUE; // return TRUE to halt further processing.
	}
	return FALSE;
}

void uiWindow::OnSize(UINT nType, UINT nNewWidth, UINT nNewHeight)
{
	printx("---> uiWindow::OnSize. New width:%d New height:%d\n", nNewWidth, nNewHeight);

	switch (nType)
	{
	case SIZE_RESTORED:
		break;
	case SIZE_MINIMIZED:
		break;
	case SIZE_MAXIMIZED:
		break;
	case SIZE_MAXSHOW:
		break;
	case SIZE_MAXHIDE:
		break;
	}

	m_Drawer.ResizeBackBuffer(nNewWidth, nNewHeight);
	m_pForm->EntryOnSize(nNewWidth, nNewHeight);
}

void uiWindow::OnSizing(INT fwSide, RECT *pRect)
{
	switch (fwSide)
	{
	case WMSZ_LEFT:
		printx("uiWindow::OnSizing. Left edge!\n");
		break;
	case WMSZ_RIGHT:
		printx("uiWindow::OnSizing. Right edge!\n");
		break;
	case WMSZ_TOP:
		printx("uiWindow::OnSizing. Top edge!\n");
		break;
	case WMSZ_TOPLEFT:
		printx("uiWindow::OnSizing. Top Left edge!\n");
		break;
	case WMSZ_TOPRIGHT:
		printx("uiWindow::OnSizing. Top Right edge!\n");
		break;
	case WMSZ_BOTTOM:
		printx("uiWindow::OnSizing. Bottom edge!\n");
		break;
	case WMSZ_BOTTOMLEFT:
		printx("uiWindow::OnSizing. Bottom Left edge!\n");
		break;
	case WMSZ_BOTTOMRIGHT:
		printx("uiWindow::OnSizing. Bottom Right edge!\n");
		break;
	}

	uiSize si = m_pForm->GetMinSize();
	if (si.iHeight > 0 && pRect->bottom - pRect->top < si.iHeight)
	{
		if (fwSide == WMSZ_TOP || fwSide == WMSZ_TOPLEFT || fwSide == WMSZ_TOPRIGHT)
			pRect->top = pRect->bottom - si.iHeight;
		else
			pRect->bottom = pRect->top + si.iHeight;
	}
	if (si.iWidth > 0 && pRect->right - pRect->left < si.iWidth)
	{
		if (fwSide == WMSZ_BOTTOMLEFT || fwSide == WMSZ_LEFT || fwSide == WMSZ_TOPLEFT)
			pRect->left = pRect->right - si.iWidth;
		else
			pRect->right = pRect->left + si.iWidth;
	}

	//	m_pForm->OnSizing((uiRect*)pRect);
}

INT64 uiWindow::OnNCHitTest(INT x, INT y)
{
	//	printx("---> uiWindow::OnNCHitTest. X:%d Y:%d\n", x, y);

	if (m_bDragging && !m_bSizing)
		return HTCLIENT;

	INT64 iRet = HTCLIENT;
	INT iPos, destX, destY, fx = 0, fy = 0;
	uiFormBase *pForm = m_pForm->FindByPos(x, y, &destX, &destY);
	uiWinCursor &cursor = uiGetCursor();
	ASSERT(pForm != nullptr);

	pForm->FrameToClientSpace(fx, fy);
	destX -= fx;
	destY -= fy;
	m_NonClientArea = iPos = pForm->OnNCHitTest(destX, destY);

	switch (iPos)
	{
	case uiFormBase::NCHT_CLIENT:
		cursor.Set(uiWinCursor::CT_NORMAL);
		break;
	case uiFormBase::NCHT_TOP:
	case uiFormBase::NCHT_BOTTOM:
		cursor.Set(uiWinCursor::CT_SIZE_NS);
		break;
	case uiFormBase::NCHT_LEFT:
	case uiFormBase::NCHT_RIGHT:
		cursor.Set(uiWinCursor::CT_SIZE_EW);
		break;
	case uiFormBase::NCHT_TOP | uiFormBase::NCHT_LEFT:
	case uiFormBase::NCHT_BOTTOM | uiFormBase::NCHT_RIGHT:
		cursor.Set(uiWinCursor::CT_SIZE_NWSE);
		break;
	case uiFormBase::NCHT_TOP | uiFormBase::NCHT_RIGHT:
	case uiFormBase::NCHT_BOTTOM | uiFormBase::NCHT_LEFT:
		cursor.Set(uiWinCursor::CT_SIZE_NESW);
		break;
	}

	bChangeCursor = true;

	return iRet;
}

BOOL uiWindow::OnLButtonDown(INT x, INT y)
{
	m_MDPosX = x;
	m_MDPosY = y;

	if (m_NonClientArea != uiForm::NCHT_CLIENT)
	{
		m_pDraggingForm = m_pHoverForm;

		if (m_pDraggingForm == m_pForm)
		{
			ResizeImp(x, y);
			return TRUE;
		}

		m_bSizing = m_bDragging = true;
		m_SizingHitSide = m_NonClientArea;
		uiGetCursor().StartSizing(FALSE);

		uiRect MouseMoveRect;
		m_pDraggingForm->GetPlate()->ClientToWindow(MouseMoveRect);
		POINT pt = { 0, 0 };
		::ClientToScreen((HWND)m_Handle, &pt);
		MouseMoveRect.Move(pt.x, pt.y);
		ClipCursor((RECT*)&MouseMoveRect);

		return TRUE;
	}

	return FALSE;
}

BOOL uiWindow::OnLButtonUp(INT x, INT y)
{
	// x, y: cient space
	if (m_bDragging)
	{
		if (m_pDraggingForm != m_pForm)
			ClipCursor(NULL);
		m_bDragging = false;

		if (m_bSizing)
		{
			m_bSizing = false;
			uiGetCursor().StartSizing(TRUE);
			m_LastMousePos.x = -1; // Mod this to prevent early return in OnMouseMove.
			OnMouseMove(0, x, y);
		}
	}

	return FALSE;
}

void uiWindow::FormSizing(uiFormBase *pForm, UINT nSide, uiRect *pRect)
{
	uiSize si = pForm->GetMinSize();
	if (si.iHeight > 0 && pRect->Height() < si.iHeight)
	{
		if (nSide & uiForm::NCHT_TOP)
			pRect->Top = pRect->Bottom - si.iHeight;
		else
			pRect->Bottom = pRect->Top + si.iHeight;
	}
	if (si.iWidth > 0 && pRect->Width() < si.iWidth)
	{
		if (nSide & uiForm::NCHT_LEFT)
			pRect->Left = pRect->Right - si.iWidth;
		else
			pRect->Right = pRect->Left + si.iWidth;
	}
}

void uiWindow::OnDragging(INT x, INT y)
{
	ASSERT(m_pDraggingForm != m_pForm);

	INT OffsetX = x - m_MDPosX;
	INT OffsetY = y - m_MDPosY;

	if (!m_bSizing)
	{
		m_pDraggingForm->RedrawForm();
		m_pDraggingForm->MoveByOffset(OffsetX, OffsetY);
		m_pDraggingForm->RedrawForm();
	}
	else
	{
		uiRect OldRectWS, NewRectWS, DestFrameRect = m_pDraggingForm->m_FrameRect;
		m_pDraggingForm->FrameToWindow(OldRectWS);

		if (m_SizingHitSide & uiForm::NCHT_TOP)
		{
			if (y > OldRectWS.Top && OffsetY < 0)
				OffsetY = 0;
			DestFrameRect.Top += OffsetY;
		}
		else if (m_SizingHitSide & uiForm::NCHT_BOTTOM)
		{
			if (y < OldRectWS.Bottom && OffsetY > 0)
				OffsetY = 0;
			DestFrameRect.Bottom += OffsetY;
		}

		if (m_SizingHitSide & uiForm::NCHT_LEFT)
		{
			if (x > OldRectWS.Left && OffsetX < 0)
				OffsetX = 0;
			DestFrameRect.Left += OffsetX;
		}
		else if (m_SizingHitSide & uiForm::NCHT_RIGHT)
		{
			if (x < OldRectWS.Right && OffsetX > 0)
				OffsetX = 0;
			DestFrameRect.Right += OffsetX;
		}

		if (OffsetX || OffsetY)
		{
			FormSizing(m_pDraggingForm, m_SizingHitSide, &DestFrameRect);
			m_pDraggingForm->m_FrameRect = DestFrameRect; // Must update the form first.
			m_pDraggingForm->FrameToWindow(NewRectWS);
			m_pDraggingForm->OnFrameSize(DestFrameRect.Width(), DestFrameRect.Height());

			NewRectWS.UnionWith(OldRectWS);
			RedrawImp(&NewRectWS);
		}
	}

	m_MDPosX = x;
	m_MDPosY = y;
}

void uiWindow::OnMouseLeave()
{
	m_bTrackMouseLeave = false;
	m_bDragging = false;

	m_LastMousePos.x = m_LastMousePos.y = -1;

	if (m_pHoverForm != nullptr)
	{
		MouseLeaveForm(m_pHoverForm);
		m_pHoverForm = nullptr;
	}
}

void uiWindow::OnMouseMove(UINT nType, INT x, INT y)
{
	if (x == m_LastMousePos.x && y == m_LastMousePos.y)
		return;

	UINT MmdFlags = 0;
	if (y != m_LastMousePos.y)
		MmdFlags |= (y - m_LastMousePos.y > 0) ? MMD_DOWN : MMD_UP;
	if (x != m_LastMousePos.x)
		MmdFlags |= (x - m_LastMousePos.x > 0) ? MMD_RIGHT : MMD_LEFT;
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
	m_TrackMouseClick = 0;
	TrackMouseLeave(TRUE);

	if (m_bDragging)
	{
		OnDragging(x, y);
		return;
	}

	INT destX, destY, FrameX = x, FrameY = y;
	uiFormBase *pForm = m_pForm->FindByPos(x, y, &destX, &destY);
	if (m_pHoverForm != pForm)
	{
		ASSERT(pForm != nullptr);
		if(m_pHoverForm != nullptr)
			MouseLeaveForm(m_pHoverForm);
		m_pHoverForm = pForm;
		MouseEnterForm(m_pHoverForm);
	}

	m_pHoverForm->OnMouseMove(destX, destY, MmdFlags);
}

void uiWindow::OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	//	printx("---> uiWindow::OnMouseBtnDown Type:%d x:%d y:%d\n", KeyType, x, y);

	switch (KeyType)
	{
	case MKT_LEFT:
		if (OnLButtonDown(x, y))
			return;
		m_TrackMouseClick |= MCKT_LEFT;
		m_MouseKeyDownTime[MKT_LEFT] = GetTimeStamp();
		break;

	case MKT_MIDDLE:
		m_TrackMouseClick |= MCKT_MIDDLE;
		m_MouseKeyDownTime[MKT_MIDDLE] = GetTimeStamp();
		break;

	case MKT_RIGHT:
		m_TrackMouseClick |= MCKT_RIGHT;
		m_MouseKeyDownTime[MKT_RIGHT] = GetTimeStamp();
		break;

	default:
		ASSERT(0);
	}

	uiRect rect;
	m_pHoverForm->ClientToWindow(rect);
	m_pHoverForm->OnMouseBtnDown(KeyType, x - rect.Left, y - rect.Top);
}

void uiWindow::OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	//	printx("---> uiWindow::OnMouseBtnUp Type:%d x:%d y:%d\n", KeyType, x, y);
	m_pHoverForm->OnMouseBtnUp(KeyType, x, y);
	if (m_pHoverForm == nullptr)
		return;

	UINT64 ctime = GetTimeStamp();
	switch (KeyType)
	{
	case MKT_LEFT:
		if (OnLButtonUp(x, y))
			break;
		if (m_TrackMouseClick & MCKT_LEFT)
		{
			if (ctime - m_MouseKeyDownTime[MKT_LEFT] <= MOUSE_CLICK_INTERVAL)
			{
				m_pHoverForm->OnMouseBtnClk(MKT_LEFT, x, y);
				m_pFirstClickedForm[MKT_LEFT] = m_pHoverForm;
			}
			m_TrackMouseClick &= ~MCKT_LEFT;
		}
		break;

	case MKT_MIDDLE:
		if (m_TrackMouseClick & MCKT_MIDDLE)
		{
			if (ctime - m_MouseKeyDownTime[MKT_MIDDLE] <= MOUSE_CLICK_INTERVAL)
			{
				m_pHoverForm->OnMouseBtnClk(MKT_MIDDLE, x, y);
				m_pFirstClickedForm[MKT_MIDDLE] = m_pHoverForm;
			}
			m_TrackMouseClick &= ~MCKT_MIDDLE;
		}
		break;

	case MKT_RIGHT:
		if (m_TrackMouseClick & MCKT_RIGHT)
		{
			if (ctime - m_MouseKeyDownTime[MKT_RIGHT] <= MOUSE_CLICK_INTERVAL)
			{
				m_pHoverForm->OnMouseBtnClk(MKT_RIGHT, x, y);
				m_pFirstClickedForm[MKT_RIGHT] = m_pHoverForm;
			}
			m_TrackMouseClick &= ~MCKT_RIGHT;
		}
		break;

	default:
		ASSERT(0);
	}
}

void uiWindow::OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	if (m_pHoverForm == nullptr)
		return;

	printx("---> uiWindow::OnMouseBtnDbClk Type:%d x:%d y:%d\n", KeyType, x, y);

	BOOL bDBClick = TRUE;
	switch (KeyType)
	{
	case MKT_LEFT:
		if (m_pFirstClickedForm[MKT_LEFT] != m_pHoverForm)
			bDBClick = FALSE;
		break;
	case MKT_MIDDLE:
		if (m_pFirstClickedForm[MKT_MIDDLE] != m_pHoverForm)
			bDBClick = FALSE;
		break;
	case MKT_RIGHT:
		if (m_pFirstClickedForm[MKT_RIGHT] != m_pHoverForm)
			bDBClick = FALSE;
		break;
	}

	if (bDBClick)
		m_pHoverForm->OnMouseBtnDbClk(KeyType, x, y);
	else
		m_pHoverForm->OnMouseBtnClk(KeyType, x, y);
}

void uiWindow::MouseEnterForm(uiFormBase *pForm)
{
	pForm->OnMouseEnter();
}

void uiWindow::MouseLeaveForm(uiFormBase *pForm)
{
	pForm->OnMouseLeave();
}

void uiWindow::OnDragging()
{

}


BOOL uiWinMenu::CreateMenu()
{
	ASSERT(m_hMenu == NULL);
	return (m_hMenu = ::CreateMenu()) != NULL;
}

BOOL uiWinMenu::CreatePopupMenu()
{
	ASSERT(m_hMenu == NULL);
	return (m_hMenu = ::CreatePopupMenu()) != NULL;
}

void uiWinMenu::DestroyMenu()
{
	if (m_hMenu != NULL)
	{
		::DestroyMenu(m_hMenu);
		m_hMenu = NULL;
	}
}

BOOL uiWinMenu::InsertItem(TCHAR* pText, UINT id, INT index)
{
	MENUITEMINFO mii;
	ZeroMemory(&mii, sizeof(mii));
	mii.cbSize = sizeof(mii);
	// MIIM_BITMAP MIIM_CHECKMARKS MIIM_DATA MIIM_FTYPE MIIM_ID MIIM_STATE MIIM_STRING MIIM_SUBMENU MIIM_TYPE
	mii.fMask = MIIM_STRING;
	// MFT_BITMAP MFT_MENUBARBREAK MFT_MENUBREAK MFT_OWNERDRAW MFT_RADIOCHECK MFT_RIGHTJUSTIFY MFT_RIGHTORDER MFT_SEPARATOR MFT_STRING
	mii.fType = MFT_STRING;
	// MFS_CHECKED MFS_DEFAULT MFS_DISABLED MFS_ENABLED MFS_GRAYED MFS_HILITE MFS_UNCHECKED MFS_UNHILITE
	mii.fState = MFS_DEFAULT;
	mii.wID = id;
	mii.hSubMenu;
	mii.hbmpChecked;
	mii.hbmpUnchecked;
	mii.dwItemData;
	mii.dwTypeData = pText;
	mii.cch = _tcslen(pText);
	mii.hbmpItem;

	BOOL bResult = ::InsertMenuItem(m_hMenu, 1, FALSE, &mii);

	return bResult;
}

UINT uiWinMenu::GetMenuItemCount() const
{
	ASSERT(m_hMenu != NULL);
	return ::GetMenuItemCount(m_hMenu);
}

BOOL uiWinMenu::Popup(HWND hWnd, INT x, INT y)
{
	ASSERT(m_hMenu != NULL);
	BOOL bResult = ::TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, hWnd, NULL);
	return (bResult != 0);
}

void uiWinMenu::ChangeStyle()
{
	ASSERT(m_hMenu != NULL);

	MENUINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	VERIFY(GetMenuInfo(m_hMenu, &mi));

	mi.dwStyle |= MNS_MODELESS;
	VERIFY(SetMenuInfo(m_hMenu, &mi));
}


uiWinCursor::uiWinCursor()
{
	m_CurrentType = CT_NORMAL;

	m_hArray[CT_NORMAL] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[CT_SIZE_NS] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENS), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[CT_SIZE_EW] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZEWE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[CT_SIZE_NESW] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENESW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[CT_SIZE_NWSE] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENWSE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	m_bSizing = false;
}

uiWinCursor::~uiWinCursor()
{
	/*
	for (INT i = 0; i < CT_TOTAL; ++i)
	{
		if (m_hArray[i] != NULL)
		{
		}
	} 
	//*/
}

void uiWinCursor::Set(CURSOR_TYPE type)
{
	m_CurrentType = type;
}

void uiWinCursor::Update()
{
	HANDLE hOldCursor = SetCursor(m_hArray[m_bSizing ? m_SizingType : m_CurrentType]);
}

void uiWinCursor::StartSizing(BOOL bComplete)
{
	if (bComplete)
	{
		ASSERT(m_bSizing);
		HANDLE hOldCursor = SetCursor(m_hArray[m_CurrentType]);
		m_bSizing = false;
	}
	else
	{
		ASSERT(!m_bSizing);
		m_bSizing = true;
		m_SizingType = m_CurrentType;
	}
}

