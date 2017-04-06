

#include "stdafx.h"
#include "uiMsWin.h"
#include "uiApp.h"
#include "MsWinHelper.h"
#include <Windowsx.h>


#ifdef _DEBUG
	#define MSG_TRACE(a, b, c, d) UICore::MsgRecorder wmr(a, b, c, d)
#else
	#define MSG_TRACE(a, b, c, d)
#endif


#define WM_CUSTOM   (WM_USER + 0x0001)
#define WM_CTRL_MSG (WM_USER + 0x0002)

#define WM_NOP      (WM_USER + 0x7FFF)


std::map<void*, uiWindow*> GWindowsHandleMap;


BEGIN_NAMESPACE(UICore)


struct INIT_UI
{
	INIT_UI()
	{
	}
	~INIT_UI()
	{
		ASSERT(GWindowsHandleMap.size() == 0);
	}

};

INIT_UI init_ui;
uiWinCursor GCursor;
uiWinCaret GCaret;

class MsgRecorder
{
public:

	struct msWinMessage
	{
		msWinMessage(HWND hWndIn, UINT MsgIn, WPARAM wParamIn, LPARAM lParamIn)
		:hWnd(hWndIn), Msg(MsgIn), wParam(wParamIn), lParam(lParamIn)
		{
		}

		HWND hWnd;
		UINT Msg;
		WPARAM wParam;
		LPARAM lParam;
	};

	MsgRecorder(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		MsgStack.push_back(msWinMessage(hWnd, Msg, wParam, lParam));
	}
	~MsgRecorder()
	{
		MsgStack.pop_back();
	}

	static std::vector<msWinMessage> MsgStack;

};

std::vector<MsgRecorder::msWinMessage> MsgRecorder::MsgStack;


END_NAMESPACE


BOOL uiMessageLookUp(UINT message)
{
	for (auto it = UICore::MsgRecorder::MsgStack.begin(); it != UICore::MsgRecorder::MsgStack.end(); ++it)
		if (it->Msg == message)
			return TRUE;
	return FALSE;
}


static uiWinCursor& uiGetCursor() { return UICore::GCursor; }

static uiWindow* WndFindByHandle(void* handle)
{
	auto it = GWindowsHandleMap.find(handle);
	ASSERT(it != GWindowsHandleMap.end());
	return it->second;
}

static size_t WndRemoveMap(void* handle)
{
	auto it = GWindowsHandleMap.find(handle);
	ASSERT(it != GWindowsHandleMap.end());
	GWindowsHandleMap.erase(it);
	return GWindowsHandleMap.size();
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


void uiMsgProc(stMsgProcRetInfo& ret, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL& bProcessed = ret.bProcessed;
	LRESULT& lRet = ret.ret;
	uiWindow *pWnd = nullptr;
	uiFormBase *pForm;

	MSG_TRACE(hWnd, message, wParam, lParam);
	ASSERT(!bProcessed && lRet == 0);

	switch (message)
	{
	case WM_CREATE:
		pWnd = (uiWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
		pWnd->SetHandle(hWnd);
		WndAddMap(hWnd, pWnd);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWnd);
		RECT rect;
		::GetWindowRect(hWnd, &rect);
		lRet = pWnd->OnCreate((uiRect*)&rect);
		bProcessed = TRUE;
		break;

	case WM_DESTROY:
		LogWndMsg("Msg: WM_DESTROY wParam:%p lParam:%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		if (WndRemoveMap(hWnd) == 0)
			PostQuitMessage(0);
		pWnd->SetHandle(NULL);
		delete pWnd;
		bProcessed = TRUE;
		return;

	case WM_MOVE:
	//	LogWndMsg("Msg: WM_MOVE Type:%d X:%d Y:%d\n", wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		bProcessed = TRUE;
		break;

	case WM_SIZE: // This is sent after size has changed.
	//	LogWndMsg("Msg: WM_SIZE Type:%d Width:%d Height:%d\n", wParam, LOWORD(lParam), HIWORD(lParam));
		pWnd = uiWindowGet(hWnd);
		pWnd->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
		bProcessed = TRUE;
		break;

	case WM_ACTIVATE:
		LogWndMsg("Msg: WM_ACTIVATE 0x%p 0x%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnActivate(wParam, lParam);
		bProcessed = TRUE;
		break;

	case WM_SETFOCUS:
	//	LogWndMsg("Msg: WM_SETFOCUS 0x%p 0x%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnGetKBFocus((HWND)wParam);
		bProcessed = TRUE;
		break;

	case WM_KILLFOCUS:
	//	LogWndMsg("Msg: WM_KILLFOCUS 0x%p 0x%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnLoseKBFocus();
		bProcessed = TRUE;
		break;

	case WM_PAINT:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnPaint();
		bProcessed = TRUE;
		break;

	case WM_CLOSE:
		//LogWndMsg("Msg: WM_CLOSE\n");
		DestroyWindow(hWnd);
		bProcessed = TRUE;
		break;

	case WM_ACTIVATEAPP:
		LogWndMsg("Msg: WM_ACTIVATEAPP 0x%p 0x%p\n", wParam, lParam);
		break;

	case WM_SETCURSOR: // Just set hCursor in struct WNDCLASSEXW to NULL.
	//	LogWndMsg("Msg: WM_SETCURSOR\n");
	//	lRet = bProcessed = TRUE; // Must let DefWindowProc handle this, or cause problem when creating modal dialog.
		break;

	case WM_NCHITTEST:
	//	LogWndMsg("Msg: WM_NCHITTEST x:%d y:%d\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = uiWindowGet(hWnd);
		lRet = pWnd->OnNCHitTest(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = nullptr;
		bProcessed = TRUE;
		break;

/*	case WM_NCPAINT:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnNCPaint(hWnd, (HRGN)wParam);
		bProcessed = TRUE;
		break; //*/

	case WM_KEYDOWN:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnKeyDown(0);
		bProcessed = TRUE;
		break;

	case WM_KEYUP:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnKeyUp(0);
		bProcessed = TRUE;
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

	case WM_TIMER:
	//	LogWndMsg("Msg: WM_TIMER ID:%d Callback: 0x%p\n", wParam, lParam);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnTimer(wParam, lParam);
		bProcessed = TRUE;
		break;

	case WM_MENUCOMMAND:
		printx("menu handle: %p, index\n", lParam, wParam);
		break;

	case WM_MOUSEMOVE:
	//	LogWndMsg("Msg: WM_MOUSEMOVE x:%d y:%d\n", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		bProcessed = TRUE;
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

	case WM_MOUSEWHEEL:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseWheel(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), GET_WHEEL_DELTA_WPARAM(wParam), GET_KEYSTATE_WPARAM(wParam));
		bProcessed = TRUE;
		break;

	case WM_SIZING:
		pWnd = uiWindowGet(hWnd);
		pWnd->OnSizing(wParam, (RECT*)lParam);
		bProcessed = TRUE;
		lRet = TRUE;
		break;

	case WM_CAPTURECHANGED:
		LogWndMsg("Msg: WM_CAPTURECHANGED HWND: %p\n", lParam);
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseCaptureLost();
		bProcessed = TRUE;
		break;

	case WM_MOUSELEAVE:
		LogWndMsg("Msg: WM_MOUSELEAVE\n");
		pWnd = uiWindowGet(hWnd);
		pWnd->OnMouseLeave();
		bProcessed = TRUE;
		break;

	case WM_CUSTOM:
		LogWndMsg("Msg: WM_CUSTOM\n");
		break;

	case WM_CTRL_MSG:
		pForm = (uiFormBase*)wParam;
		pWnd = pForm->GetBaseWnd();
		pForm->EntryOnCommand(lParam); // pForm might be destroyed after calling this method.
		bProcessed = TRUE;
		break;
	}

	if (pWnd != nullptr)
		pWnd->PostMsgHandler(message);
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	stMsgProcRetInfo rInfo;
	uiMsgProc(rInfo, hWnd, message, wParam, lParam);
	if (rInfo.bProcessed)
		return rInfo.ret;

	return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL OnInitDialog(stDialogCreateParam* pDCP, HWND hDlg)
{
	uiFormBase* pForm = pDCP->pForm;
	uiWindow* pWnd = new uiWindow(pForm);
	if (pWnd == nullptr)
		return FALSE;

	pForm->SetWindow(pWnd);
	pWnd->SetAsDialog();

	pWnd->SetHandle(hDlg);
	WndAddMap(hDlg, pWnd);
	::SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pWnd);

	uiRect rect;
	pWnd->GetWindowRect(rect);
	if (pWnd->OnCreate(&rect) == -1)
		return FALSE;

	// Simulate normal window.
	pWnd->OnSize(0, rect.Width(), rect.Height());
	pWnd->OnMove(rect.Left, rect.Top);

	pForm->EntryOnCreate(TRUE, rect.Width(), rect.Height());

	return TRUE;
}

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		printx("---> DialogProc() Msg: WM_INITDIALOG, HWND: %p\n", hDlg);
		if (!OnInitDialog((stDialogCreateParam*)lParam, hDlg))
			EndDialog(hDlg, -1);
		return FALSE;
	}

	stMsgProcRetInfo rInfo;
	uiMsgProc(rInfo, hDlg, uMsg, wParam, lParam);

	if (rInfo.bProcessed)
	{
		SetWindowLong(hDlg, DWL_MSGRESULT, rInfo.ret);
		return TRUE;
	}

	return FALSE;
}

uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase* pForm, uiFormBase* ParentForm, INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight, BOOL bVisible)
{
	static BOOL bRegistered = FALSE;
	const TCHAR *pWndClass = _T("WndClass");
	HINSTANCE hInstance = uiGetAppIns();

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
		wcex.hCursor = NULL; // LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = NULL; //(HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = pWndClass;
		wcex.hIconSm = NULL; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

		RegisterClassExW(&wcex);
		bRegistered = TRUE;
	}

	HWND hParent = (ParentForm == nullptr) ? NULL : (HWND)ParentForm->GetBaseWnd()->GetHandle();
	RECT r = { x, y, (LONG)(x + nWidth), (LONG)(y + nHeight) };
	DWORD ExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, Style = (bVisible) ? WS_POPUP| WS_VISIBLE : WS_POPUP /*| WS_SIZEBOX | WS_MAXIMIZEBOX*/;
//	DWORD ExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, Style = WS_CAPTION | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE;

	switch (uwt)
	{
	case UWT_NORMAL:
		break;
	case UWT_TOOL:
		ASSERT(hParent != NULL);
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
		pForm->SetWindow(pWnd);

		HWND hWnd = CreateWindowEx(ExStyle, pWndClass, nullptr, Style, r.left, r.top, nWidth, nHeight, hParent, NULL, hInstance, pWnd);
		if (hWnd != NULL) // pWnd will be deleted if the creation fails.
			return pWnd;
	}

	return nullptr;
}

INT_PTR CreateModalDialog(const stDialogCreateParam* pDCP)
{
//*
	HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, sizeof(DLGTEMPLATE)); // Must use protected system memory.
	if (!hMem)
		return -1;

	LPDLGTEMPLATE pDlgTemp = (LPDLGTEMPLATE)GlobalLock(hMem);
	//pDlgTemp->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION; // Define a dialog box.
	pDlgTemp->style = WS_POPUP;
	pDlgTemp->dwExtendedStyle = WS_EX_LTRREADING;
	pDlgTemp->cdit = 0; // Number of controls
	pDlgTemp->x = 100;
	pDlgTemp->y = 100;
	pDlgTemp->cx = 250;
	pDlgTemp->cy = 100;
	GlobalUnlock(hMem);

	HWND hParentWnd = (pDCP->pParent != nullptr) ? pDCP->pParent->GetBaseWnd()->GetHandle() : NULL;
	LPARAM lParam = (LPARAM)const_cast<stDialogCreateParam*>(pDCP);
	INT_PTR ret = DialogBoxIndirectParam(uiGetAppIns(), (LPDLGTEMPLATE)hMem, hParentWnd, DialogProc, lParam);
	//printx("ec: %d\n", GetLastError());
	GlobalFree(hMem);
/*/

	HGLOBAL hMem = GlobalAlloc(GMEM_ZEROINIT, sizeof(DLGTEMPLATEEX2)); // Must use protected system memory.
	if (!hMem)
		return -1;

	DLGTEMPLATEEX2* pDlgTemp = (DLGTEMPLATEEX2*)GlobalLock(hMem);
	//pDlgTemp->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION; // Define a dialog box.
	pDlgTemp->style = WS_POPUP;
	pDlgTemp->cDlgItems = 0; // Number of controls
	pDlgTemp->x = 100;
	pDlgTemp->y = 100;
	pDlgTemp->cx = 250;
	pDlgTemp->cy = 100;
	GlobalUnlock(hMem);

	HWND hParentWnd = (pDCP->pParent != nullptr) ? pDCP->pParent->GetBaseWnd()->GetHandle() : NULL;
	LPARAM lParam = (LPARAM)const_cast<stDialogCreateParam*>(pDCP);
	INT_PTR ret = DialogBoxIndirectParam(uiGetAppIns(), (LPDLGTEMPLATE)hMem, hParentWnd, DialogProc, lParam);
	printx("ec: %d\n", GetLastError());
	GlobalFree(hMem);

//*/
	return ret;
}


BOOL WndClientToScreen(uiWindow* pWnd, INT& x, INT& y)
{
	pWnd->ClientToScreen(x, y);
	return TRUE;
}

BOOL WndCreateMessage(uiWindow* pWnd, uiFormBase* pSrc, UINT id)
{
	BOOL bResult = (pWnd->PostMessage(WM_CTRL_MSG, (WPARAM)pSrc, id) != 0);
	VERIFY(bResult);
	return bResult;
}


uiWindow::uiWindow(uiFormBase *pFormIn)
:m_pFirstClickedForm()
{
	m_Handle = NULL;
	m_pForm = pFormIn;
	m_pHoverForm = nullptr;
	m_pGrayForm = nullptr;
	m_pDraggingForm = nullptr;
	m_pMouseFocusForm = nullptr;
	m_pKeyboardFocusForm = nullptr;

	m_TrackMouseClick = 0;
	m_AreaType = 0;
	m_LastMousePos.x = -1;
	m_LastMousePos.y = -1;

	m_MouseDragKey = MKT_NONE;
	ZeroMemory(m_MouseKeyDownTime, sizeof(m_MouseKeyDownTime));

	m_TotalWorkingTimer = 0;
}

uiWindow::~uiWindow()
{
	ASSERT(m_Handle == NULL);
	ASSERT(m_pForm == nullptr);
	ASSERT(m_TotalWorkingTimer == 0);
}


void uiWindow::OnFormDestroy(uiFormBase *pForm)
{
	if (pForm->GetTimerCount() != 0)
		TimerRemoveAll(pForm);

	if (m_pHoverForm == pForm)
	{
		m_pHoverForm = nullptr;
		bRetrackMouse = true;
	}
	if (m_pMouseFocusForm == pForm)
	{
		m_pMouseFocusForm = nullptr;
		bRetrackMouse = true;
	}
	if (m_pKeyboardFocusForm == pForm)
		m_pKeyboardFocusForm = nullptr;
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
//	::DestroyWindow(m_Handle); // Don't call this here.

	if (IsDialog())
		::EndDialog(m_Handle, 0);
	else
		::PostMessage(m_Handle, WM_CLOSE, NULL, NULL);

//	::PostMessage(m_Handle, WM_DESTROY, NULL, NULL); // This won't work for tool windows.
}

BOOL uiWindow::CloseDialogImp(INT_PTR ret)
{
	ASSERT(IsDialog());
	return ::EndDialog(m_Handle, ret);
}

BOOL uiWindow::MoveImp(INT scX, INT scY)
{
	return ::SetWindowPos(m_Handle, NULL, scX, scY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

BOOL uiWindow::MoveByOffsetImp(INT x, INT y)
{
	if (x == 0 && y == 0)
		return FALSE;
	return MoveImp(m_ScreenCoordinateX + x, m_ScreenCoordinateY + y);
}

void uiWindow::MoveToCenter()
{
	RECT rect;
	VERIFY(::SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0) != 0);

	uiRect ur;
	GetWindowRect(ur);
	INT mx = (rect.left + rect.right - ur.Width()) / 2;
	INT my = (rect.top + rect.bottom - ur.Height()) / 2;
	MoveImp(mx, my);
}

void uiWindow::ShowImp(FORM_SHOW_MODE sm)
{
	INT iCmdShow = SW_SHOW;
	if (sm != FSM_HIDE && uiMessageLookUp(WM_KILLFOCUS))
	{
		ASSERT(0);
		return;
	}

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

	ShowWindow(iCmdShow);
}

BOOL uiWindow::SizeImp(UINT nWidth, UINT nHeight)
{
	//	UINT Flags = SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE;
	return (SetWindowPos(m_Handle, NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER) != 0);
}

BOOL uiWindow::StartDraggingImp(uiFormBase *pForm, MOUSE_KEY_TYPE mkt, INT x, INT y, uiRect MouseMoveRect)
{
#ifdef _DEBUG
	switch (mkt)
	{
	case MKT_LEFT:
		ASSERT(GetKeyState(VK_LBUTTON) < 0);
		break;
	case MKT_MIDDLE:
		ASSERT(GetKeyState(VK_MBUTTON) < 0);
		break;
	case MKT_RIGHT:
		ASSERT(GetKeyState(VK_RBUTTON) < 0);
		break;
	}
#endif

	if (pForm == m_pForm)
		return ::PostMessage(m_Handle, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(x, y));
	else
	{
		if (m_bDragging || m_MouseDragKey != MKT_NONE)
		{
			ASSERT(0);
			return FALSE;
		}

		m_MouseDragKey = mkt;
		m_bDragging = true;
		m_pDraggingForm = pForm;
		m_MDPosX = x;
		m_MDPosY = y;

		MouseMoveRect.Move(m_ScreenCoordinateX, m_ScreenCoordinateY);
		ClipCursor((RECT*)&MouseMoveRect);
		SetCapture();
		return TRUE;
	}
}

void uiWindow::RedrawImp(const uiRect* pRect)
{
	InvalidateRect(m_Handle, (const RECT*)pRect, FALSE);
}

void uiWindow::ResizeImp(INT x, INT y)
{
	INT iRegion = -1;
	const UINT mask = uiFormBase::CAT_ALL_DRF;

	switch (m_AreaType & mask)
	{
	case uiFormBase::CAT_TOP:
		iRegion = HTTOP;
		break;
	case uiFormBase::CAT_BOTTOM:
		iRegion = HTBOTTOM;
		break;
	case uiFormBase::CAT_LEFT:
		iRegion = HTLEFT;
		break;
	case uiFormBase::CAT_RIGHT:
		iRegion = HTRIGHT;
		break;
	case (uiFormBase::CAT_TOP | uiFormBase::CAT_LEFT):
		iRegion = HTTOPLEFT;
		break;
	case uiFormBase::CAT_TOP | uiFormBase::CAT_RIGHT:
		iRegion = HTTOPRIGHT;
		break;
	case uiFormBase::CAT_BOTTOM | uiFormBase::CAT_LEFT:
		iRegion = HTBOTTOMLEFT;
		break;
	case uiFormBase::CAT_BOTTOM | uiFormBase::CAT_RIGHT:
		iRegion = HTBOTTOMRIGHT;
		break;
	}

	ASSERT(iRegion != -1);
	::PostMessage(m_Handle, WM_NCLBUTTONDOWN, iRegion, MAKELPARAM(x, y));
}

void uiWindow::PostMsgHandler(UINT msg)
{
	BOOL bUpdateCursor = FALSE;
	uiPoint ptWS, ptSS, ptCS;

	if (bRetrackMouse)
	{
		UICore::GCursor.GetPos(ptSS);
		ptWS = ptSS;
		ScreenToClient(ptWS.x, ptWS.y);

		uiFormBase *pOldHoverForm;
		//	INT TrackCount = 0;

		do
		{
			bRetrackMouse = false;
			bUpdateCursor = TRUE;
			//	printx("Start tracking: %d\n", ++TrackCount);

			if ((m_AreaType = m_pForm->FindByPos(&m_pGrayForm, ptWS.x, ptWS.y, &ptCS)) != uiFormBase::CAT_CLIENT)
			{
				if (m_pHoverForm != nullptr)
				{
					MouseLeaveForm(m_pHoverForm);
					m_pHoverForm = nullptr;
				}
			}
			else
			{
				pOldHoverForm = m_pHoverForm;
				m_pHoverForm = m_pGrayForm;

				if (m_pHoverForm == pOldHoverForm)
					break;
				if (pOldHoverForm != nullptr)
					MouseLeaveForm(pOldHoverForm);
				if (bRetrackMouse)
				{
					m_pHoverForm = nullptr;
					continue;
				}

				ASSERT(m_pHoverForm != nullptr);
				MouseEnterForm(m_pHoverForm, ptCS.x, ptCS.y);
			}

			//	printx("Leave tracking: %d\n", TrackCount);
		} while (bRetrackMouse);

		ASSERT(msg != WM_PAINT); // Can't send redraw command when dealing with this message.
	}

	if (bUpdateCursor)
		UpdateCursor(m_pGrayForm, ptCS.x, ptCS.y);
}

void uiWindow::FormSizingCheck(uiFormBase* pForm, UINT nSide, uiRect *pRect)
{
	uiSize si = pForm->GetMinSize();

	if (si.iHeight > 0 && pRect->Height() < si.iHeight)
	{
		if (nSide & uiForm::CAT_TOP)
			pRect->Top = pRect->Bottom - si.iHeight;
		else
			pRect->Bottom = pRect->Top + si.iHeight;
	}
	if (si.iWidth > 0 && pRect->Width() < si.iWidth)
	{
		if (nSide & uiForm::CAT_LEFT)
			pRect->Left = pRect->Right - si.iWidth;
		else
			pRect->Right = pRect->Left + si.iWidth;
	}
}

void uiWindow::RetrackMouseCheck(uiFormBase* pFormBase) // For form size and move function.
{
	if (bRetrackMouse || m_bDragging || m_pHoverForm == nullptr)
		return;

	if (pFormBase->FBTestFlag(uiFormBase::FBF_MOUSE_HOVER))
		bRetrackMouse = true;
	else
		for (uiFormBase *pBase = m_pHoverForm; pBase != nullptr; pBase = pBase->GetPlate())
			if (pFormBase == pBase)
			{
				bRetrackMouse = true;
				break;
			}
}

void uiWindow::UpdateCursor(uiFormBase *pForm, INT csX, INT csY)
{
	if (m_bSizing)
		return;

	uiImage NewCursor;
	IAreaCursor *pIAC = pForm->GetIAreaCursor();
	if (pIAC == nullptr)
	{
		const UINT mask = uiFormBase::CAT_ALL_DRF | uiFormBase::CAT_DRAG_BAR_H | uiFormBase::CAT_DRAG_BAR_V;
		uiWinCursor::DEFAULT_CURSOR_TYPE dct = uiWinCursor::DCT_NORMAL;

		switch (m_AreaType & mask)
		{
		case uiFormBase::CAT_CLIENT:
			dct = uiWinCursor::DCT_NORMAL;
			break;
		case uiFormBase::CAT_TOP:
		case uiFormBase::CAT_BOTTOM:
			dct = uiWinCursor::DCT_SIZE_NS;
			break;
		case uiFormBase::CAT_LEFT:
		case uiFormBase::CAT_RIGHT:
			dct = uiWinCursor::DCT_SIZE_EW;
			break;
		case uiFormBase::CAT_TOP | uiFormBase::CAT_LEFT:
		case uiFormBase::CAT_BOTTOM | uiFormBase::CAT_RIGHT:
			dct = uiWinCursor::DCT_SIZE_NWSE;
			break;
		case uiFormBase::CAT_TOP | uiFormBase::CAT_RIGHT:
		case uiFormBase::CAT_BOTTOM | uiFormBase::CAT_LEFT:
			dct = uiWinCursor::DCT_SIZE_NESW;
			break;
		}

		uiGetCursor().Update(dct);
	}
	else
	{
		uiImage& NewCursor = pIAC->GetCursorImage(pForm, csX, csY, (uiFormBase::CLIENT_AREA_TYPE)m_AreaType);
		uiGetCursor().Set(std::move(NewCursor));

	//	uiGetCursor().Set(pIAC->GetCursorImage(pForm, csX, csY, (uiFormBase::CLIENT_AREA_TYPE)m_AreaType));
	}
}

uiFormBase* uiWindow::CaptureMouseFocus(uiFormBase* pForm)
{
	uiFormBase *pOriginalFocusForm = nullptr;

	if (bMouseFocusCaptured)
	{
		if (pForm == m_pMouseFocusForm)
			return m_pMouseFocusForm;

		pOriginalFocusForm = m_pMouseFocusForm;
		m_pMouseFocusForm->OnMouseFocusLost();
		m_pMouseFocusForm = pForm;
		return pOriginalFocusForm;
	}

	HWND hWnd = SetCapture();
	if (hWnd != NULL) // Windows won't return the handle of the window created in other process.
	{
		uiWindow *pWnd = uiWindowGet(hWnd);
		pOriginalFocusForm = pWnd->m_pMouseFocusForm;
	}
	m_pMouseFocusForm = pForm;
	bMouseFocusCaptured = true;

	return pOriginalFocusForm;
}

BOOL uiWindow::ReleaseMouseFocus(uiFormBase* pForm)
{
	if (!bMouseFocusCaptured)
		return FALSE;

	if (m_pMouseFocusForm == pForm)
	{
		ReleaseCapture();
		bMouseFocusCaptured = false;
		return TRUE;
	}
	return FALSE;
}

BOOL uiWindow::CaretShowImp(uiFormBase *pFormBase, INT x, INT y, INT width, INT height)
{
	if (!bKBFocusCaptured || pFormBase != m_pKeyboardFocusForm)
	{
		//	ASSERT(0);
		//	return FALSE;
	}
	if (UICore::GCaret.SetCaret(m_Handle, NULL, width, height))
		return UICore::GCaret.Show(m_Handle, x, y);
	return FALSE;
}

BOOL uiWindow::CaretHideImp(uiFormBase *pFormBase)
{
	if (!bKBFocusCaptured || pFormBase != m_pKeyboardFocusForm)
	{
		ASSERT(0);
		return FALSE;
	}
	return UICore::GCaret.Destroy();
}

BOOL uiWindow::CaretMoveImp(uiFormBase *pFormBase, INT x, INT y)
{
	if (!bKBFocusCaptured || pFormBase != m_pKeyboardFocusForm)
	{
		ASSERT(0);
		return FALSE;
	}
	return UICore::GCaret.SetPos(x, y);
}

BOOL uiWindow::CaretMoveByOffset(uiFormBase *pFormBase, INT OffsetX, INT OffsetY)
{
	if (!bKBFocusCaptured || pFormBase != m_pKeyboardFocusForm)
	{
		//	ASSERT(0);
		//	return FALSE;
	}
	return UICore::GCaret.MoveByOffset(OffsetX, OffsetY);
}

void DbgTimerCheck(UINT id, UINT msElapsedTime, INT nRunCount, void* pCtx, stWndTimerInfo &ExistedTimer)
{
	if (ExistedTimer.msElapsedTime != msElapsedTime)
		return;
	if (ExistedTimer.nRunCount != nRunCount)
		return;
	if (ExistedTimer.pCtx != pCtx)
		return;
	printx("Warning! TimerStart with existed timer id and the same parameters! ID: %d\n", id);
}

UINT uiWindow::TimerAdd(uiFormBase *pFormBase, UINT id, UINT msElapsedTime, INT nRunCount, void* pCtx)
{
	ASSERT(m_TotalWorkingTimer <= m_TimerTable.size());

	for(UINT i = 0; i < m_TotalWorkingTimer; ++i)
		if (m_TimerTable[i].TimerHandle && m_TimerTable[i].pFormBase == pFormBase && m_TimerTable[i].id == id)
		{
			DEBUG_CHECK(DbgTimerCheck(id, msElapsedTime, nRunCount, pCtx, m_TimerTable[i]));

			UINT_PTR NewTimerHandle;
			if ((NewTimerHandle = ::SetTimer(m_Handle, i + 1, msElapsedTime, nullptr)) == 0)
				return 0;

			m_TimerTable[i].msElapsedTime = msElapsedTime;
			m_TimerTable[i].nRunCount = nRunCount;
			m_TimerTable[i].pCtx = pCtx;
			m_TimerTable[i].TimerHandle = NewTimerHandle;
			return i + 1;
		}

	INT index = 0;
	if (m_TotalWorkingTimer == m_TimerTable.size())
		index = m_TimerTable.size();
	else
	{
		INT ArraySize = (INT)m_TimerTable.size();
		for (; index < ArraySize; ++index)
			if (m_TimerTable[index].TimerHandle == 0)
				break;
		ASSERT(index < ArraySize);
	}

	UINT_PTR TimerHandle;
	if ((TimerHandle = ::SetTimer(m_Handle, index + 1, msElapsedTime, nullptr)) == 0)
		return 0;

	stWndTimerInfo wti;
	wti.id = id;
	wti.msElapsedTime = msElapsedTime;
	wti.nRunCount = nRunCount;
	wti.pCtx = pCtx;
	wti.pFormBase = pFormBase;
	wti.TimerHandle = TimerHandle;

	if (index == m_TimerTable.size())
		m_TimerTable.push_back(wti);
	else
		m_TimerTable[index] = wti;

	++m_TotalWorkingTimer;
	pFormBase->SetTimerCount(TRUE);

	return index + 1;
}

BOOL uiWindow::TimerClose(uiFormBase *pFormBase, UINT key, BOOL bByID)
{
	UINT nIndex;

	if (!bByID)
	{
		nIndex = key - 1;
		if (nIndex >= m_TimerTable.size() || m_TimerTable[nIndex].TimerHandle == 0 || m_TimerTable[nIndex].pFormBase != pFormBase)
		{
			printx("Warning: TimerClose failed! Handle: %d\n", key);
			return FALSE;
		}
	}
	else
	{
		for (nIndex = 0; nIndex < m_TimerTable.size(); ++nIndex)
			if (m_TimerTable[nIndex].TimerHandle == 0 || m_TimerTable[nIndex].pFormBase != pFormBase || m_TimerTable[nIndex].id != key)
				continue;
			else
				break;

		if (nIndex == m_TimerTable.size())
		{
			printx("Warning: TimerClose failed! ID: %d\n", key);
			return FALSE;
		}
	}

	stWndTimerInfo &wti = m_TimerTable[nIndex];
	VERIFY(::KillTimer(m_Handle, nIndex + 1));
	wti.TimerHandle = 0; // Lazy clean.
	wti.pFormBase->SetTimerCount(FALSE);
	--m_TotalWorkingTimer;

	return TRUE;
}

void uiWindow::TimerRemoveAll(uiFormBase* const pFormBase)
{
	INT iTableSize = m_TimerTable.size();
	for (INT i = 0; i < iTableSize; ++i)
	{
		stWndTimerInfo &wti = m_TimerTable[i];
		if (wti.TimerHandle == 0 || wti.pFormBase != pFormBase)
			continue;

		VERIFY(::KillTimer(m_Handle, i + 1));

		wti.TimerHandle = 0; // Lazy clean.
		wti.pFormBase->SetTimerCount(FALSE);
		--m_TotalWorkingTimer;
	}

	ASSERT(pFormBase->GetTimerCount() == 0);
}

void uiWindow::OnActivate(WPARAM wParam, LPARAM lParam)
{
	HWND hActivated = NULL, hDeactivated = NULL;

	switch (LOWORD(wParam))
	{
	case WA_INACTIVE:
		hActivated = (HWND)lParam;
		break;
	case WA_ACTIVE:
		hDeactivated = (HWND)lParam;
		break;
	case WA_CLICKACTIVE:
		hDeactivated = (HWND)lParam;
		break;
	}
//	BOOL bMinimized = HIWORD(wParam);

	if (m_pKeyboardFocusForm == nullptr)
		m_pKeyboardFocusForm = m_pForm;
}

BOOL uiWindow::OnClose()
{
	if (m_pForm->OnClose())
	{
		if (m_pForm->GetParent() != nullptr)
			m_pForm->GetParent()->DetachChild(m_pForm);
		m_pForm->EntryOnDestroy(this); // m_pForm will delete itself.
		m_pForm = nullptr;
		return TRUE;
	}
	return FALSE;
}

BOOL uiWindow::OnCreate(const uiRect* pRect)
{
	//printx("---> uiWindow::OnCreate\n");

	if (m_Drawer.InitBackBuffer(DEFAULT_BACKBUFFER_COUNT, m_Handle, pRect->Width(), pRect->Height()))
		return 0;

	m_pForm = nullptr;
	return -1;
}

void uiWindow::OnDestroy()
{
	delete this;
}

void uiWindow::OnKeyDown(INT iKey)
{
	//printx("---> uiWindow::OnKeyDown\n");
}

void uiWindow::OnKeyUp(INT iKey)
{
	//printx("---> uiWindow::OnKeyUp\n");
}

void uiWindow::OnMove(INT scx, INT scy)
{
	//printx("---> uiWindow::OnMove scX: %d, scY: %d\n", scx, scy);

	if (m_pForm->IsCreated())
	{
		uiFormBase::stFormMoveInfo fmi;
		fmi.XOffset = scx - m_ScreenCoordinateX;
		fmi.YOffset = scy - m_ScreenCoordinateY;
		if (fmi.XOffset)
			fmi.MDFlag |= (fmi.XOffset > 0) ? MOVE_RIGHT : MOVE_LEFT;
		if (fmi.YOffset)
			fmi.MDFlag |= (fmi.YOffset > 0) ? MOVE_DOWN : MOVE_UP;
		m_pForm->EntryOnMove(scx, scy, &fmi);
	}

	m_ScreenCoordinateX = scx;
	m_ScreenCoordinateY = scy;
}

void uiWindow::OnNCPaint(HWND hWnd, HRGN hRgn)
{
	// This doesn't work totally.
}

void uiWindow::OnPaint()
{
	PAINTSTRUCT ps;
	if (m_Drawer.Begin(&ps))
	{
		if (GetKeyState(VK_F2) < 0)
		{
			uiRect rect;
			memcpy(&rect, &ps.rcPaint, sizeof(rect));
			m_Drawer.FillRect(rect, RGB(rand() % 256, rand() % 256, rand() % 256));
			//	printx("---> uiWindow::OnPaint\n");
		}
		else
		{
			m_pForm->EntryOnPaint(&m_Drawer, 1);
		}
		m_Drawer.End(&ps);
	}
	else
	{
		uiRect rect;
		if (GetClientRect(rect) && rect.IsEmpty()) // This happens when tool window is minimized.
			return;
		printx("Warning! ---> uiWindow::OnPaint. Failed to draw!\n");
		ASSERT(0);
	}
}

void uiWindow::OnGetKBFocus(HWND hOldFocusWnd)
{
	printx("---> uiWindow::OnGetKBFocus. Old focus wnd: %p\n", hOldFocusWnd);
	ASSERT(!bKBFocusCaptured);

	if (m_pKeyboardFocusForm != nullptr)
		m_pKeyboardFocusForm->EntryOnKBGetFocus();

	bKBFocusCaptured = true;
}

void uiWindow::OnLoseKBFocus()
{
	printx("---> uiWindow::OnLoseKBFocus.\n");
	ASSERT(bKBFocusCaptured);

	if (m_pForm != nullptr && m_pKeyboardFocusForm != nullptr)
		m_pKeyboardFocusForm->EntryOnKBLoseFocus();

	bKBFocusCaptured = false;
}

void uiWindow::OnSize(UINT nType, UINT nNewWidth, UINT nNewHeight)
{
	//printx("---> uiWindow::OnSize. New width:%d New height:%d\n", nNewWidth, nNewHeight);

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
	if (m_pForm->IsCreated())
		m_pForm->EntryOnSize(nNewWidth, nNewHeight);
}

void uiWindow::OnSizing(INT fwSide, RECT *pRect)
{
/*
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
	} //*/

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

void uiWindow::OnTimer(const UINT_PTR TimerID, LPARAM lParam)
{
	//printx("---> uiWindow::OnTimer. ID: %p Callback: 0x%p\n", TimerID, lParam);
	UINT index = TimerID - 1;
	ASSERT(index < m_TimerTable.size());

	stWndTimerInfo &wti = m_TimerTable[index];

	if (wti.TimerHandle == 0)
	{
	//	printx("Old Timer triggered!\n");
		return;
	}
	if (wti.nRunCount > 0)
		--wti.nRunCount;
	wti.pFormBase->OnTimer((uiFormBase::stTimerInfo*)&wti);

	if (wti.nRunCount == 0)
	{
		VERIFY(::KillTimer(m_Handle, TimerID));
		wti.TimerHandle = 0;
		wti.pFormBase->SetTimerCount(FALSE);
		--m_TotalWorkingTimer;
	}
}

LRESULT uiWindow::OnNCHitTest(INT scX, INT scY) // Windows stops sending the message if the window captured the mouse focus.
{
	//	printx("---> uiWindow::OnNCHitTest. X:%d Y:%d\n", x, y);
//	ASSERT(!m_bDragging);
//	ASSERT(!m_bSizing);

	// Just determine the area type. Don't do much.
	uiPoint ptCS;
	ScreenToClient(scX, scY);
	m_AreaType = m_pForm->FindByPos(&m_pGrayForm, scX, scY, &ptCS);
//	uiFormBase::CLIENT_AREA_TYPE cat = (uiFormBase::CLIENT_AREA_TYPE)m_AreaType;
	UpdateCursor(m_pGrayForm, ptCS.x, ptCS.y);

	return HTCLIENT;
}

BOOL uiWindow::DragSizingEventCheck(INT x, INT y)
{
	m_MDPosX = x;
	m_MDPosY = y;

	if (m_AreaType != uiForm::CAT_CLIENT)
	{
		m_pDraggingForm = m_pGrayForm;

		if (m_pDraggingForm == m_pForm)
		{
			ResizeImp(x, y);
			return TRUE;
		}

		m_MouseDragKey = MKT_LEFT;
		m_bSizing = m_bDragging = true;
		m_SizingHitSide = m_AreaType;
//		uiGetCursor().StartSizing(FALSE);
		SetCapture();

		uiRect MouseMoveRect;
		m_pDraggingForm->GetPlate()->GetClientRectWS(MouseMoveRect);
		MouseMoveRect.Move(m_ScreenCoordinateX, m_ScreenCoordinateY);
		ClipCursor((RECT*)&MouseMoveRect);

		return TRUE;
	}

	return FALSE;
}

BOOL uiWindow::DragEventForMouseBtnUp(INT wcX, INT wcY)
{
	if (m_bDragging)
	{
		if (m_pDraggingForm != m_pForm)
		{
			ReleaseCapture();
			ClipCursor(NULL);
		}

		m_MouseDragKey = MKT_NONE;
		m_bDragging = false;

		if (m_bSizing)
		{
			m_bSizing = false;
//			uiGetCursor().StartSizing(TRUE);
		}
		return TRUE;
	}
	return FALSE;
}

void uiWindow::OnMouseCaptureLost()
{
	uiPoint cpt;
	uiGetCursor().GetPos(cpt);
	ScreenToClient(cpt.x, cpt.y);

	if (m_bDragging)
	{
		ASSERT(!bMouseFocusCaptured);

		if (m_pDraggingForm != m_pForm)
			ClipCursor(NULL);
		m_MouseDragKey = MKT_NONE;
		m_bDragging = false;

		if (m_bSizing)
		{
			m_bSizing = false;
//			uiGetCursor().StartSizing(TRUE);
		}
	}
	else if (m_pMouseFocusForm != nullptr)
	{
		ASSERT(bMouseFocusCaptured);
		m_pMouseFocusForm->OnMouseFocusLost();
		m_pMouseFocusForm = nullptr;
		bMouseFocusCaptured = false;
	}

	if (m_pForm != nullptr && m_pForm->m_FrameRect.IsPointIn(cpt)) // Make sure mouse cursor is inside the window.
		bRetrackMouse = true;
}

void uiWindow::OnDragging(INT x, INT y)
{
	ASSERT(m_pDraggingForm != m_pForm);

	INT OffsetX = x - m_MDPosX;
	INT OffsetY = y - m_MDPosY;
	uiRect OriginalFrameRect;

	if (!m_bSizing)
	{
		m_pDraggingForm->MoveByOffset(OffsetX, OffsetY, TRUE);
	}
	else
	{
		uiRect OldRectWS, DestFrameRect = m_pDraggingForm->m_FrameRect;
		m_pDraggingForm->GetFrameRectWS(OldRectWS);

		if (m_SizingHitSide & uiForm::CAT_TOP)
		{
			if (y > OldRectWS.Top && OffsetY < 0)
				OffsetY = 0;
			DestFrameRect.Top += OffsetY;
		}
		else if (m_SizingHitSide & uiForm::CAT_BOTTOM)
		{
			if (y < OldRectWS.Bottom && OffsetY > 0)
				OffsetY = 0;
			DestFrameRect.Bottom += OffsetY;
		}

		if (m_SizingHitSide & uiForm::CAT_LEFT)
		{
			if (x > OldRectWS.Left && OffsetX < 0)
				OffsetX = 0;
			DestFrameRect.Left += OffsetX;
		}
		else if (m_SizingHitSide & uiForm::CAT_RIGHT)
		{
			if (x < OldRectWS.Right && OffsetX > 0)
				OffsetX = 0;
			DestFrameRect.Right += OffsetX;
		}

		if (OffsetX || OffsetY)
		{
			FormSizingCheck(m_pDraggingForm, m_SizingHitSide, &DestFrameRect);
			if (m_pDraggingForm->m_FrameRect != DestFrameRect)
			{
				OriginalFrameRect = m_pDraggingForm->m_FrameRect;
				m_pDraggingForm->m_FrameRect = DestFrameRect; // Must update the form first.
				m_pDraggingForm->OnFrameSize(DestFrameRect.Width(), DestFrameRect.Height());
				DestFrameRect.UnionWith(OriginalFrameRect);

				m_pDraggingForm->RedrawFrame(&DestFrameRect);
			}
		}
	}

	m_MDPosX = x;
	m_MDPosY = y;
}

void uiWindow::OnMouseLeave()
{
	uiGetCursor().Reset();
	m_bTrackMouseLeave = false;
	m_bDragging = false;
	m_MouseDragKey = MKT_NONE;

	m_LastMousePos.x = m_LastMousePos.y = -1;

	if (m_pHoverForm != nullptr)
	{
		MouseLeaveForm(m_pHoverForm);
		m_pHoverForm = nullptr;
	}
}

void uiWindow::OnMouseMove(UINT nType, const INT x, const INT y)
{
//	printx("---> uiWindow::OnMouseMove. m_AreaType: %d\n", m_AreaType);

	if (x == m_LastMousePos.x && y == m_LastMousePos.y)
		return;

	MOVE_DIRECTION MmdFlags = MOVE_NONE;
	if (y != m_LastMousePos.y)
		MmdFlags |= (y - m_LastMousePos.y > 0) ? MOVE_DOWN : MOVE_UP;
	if (x != m_LastMousePos.x)
		MmdFlags |= (x - m_LastMousePos.x > 0) ? MOVE_RIGHT : MOVE_LEFT;
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
	m_TrackMouseClick = 0;
	TrackMouseLeave(TRUE);

	if (m_bDragging)
	{
		OnDragging(x, y);
		return;
	}
	if (m_pMouseFocusForm != nullptr)
	{
		uiPoint pt(x, y);
		m_pMouseFocusForm->WindowToClient(pt);
		m_pMouseFocusForm->OnMouseMove(pt.x, pt.y, MmdFlags);
		return;
	}
	if (m_AreaType != uiFormBase::CAT_CLIENT)
	{
		if (m_pHoverForm != nullptr)
		{
			MouseLeaveForm(m_pHoverForm);
			m_pHoverForm = nullptr;
		}
		return;
	}

	uiPoint ptCS;
	uiFormBase *pForm = nullptr;
	INT iAreaType = m_pForm->FindByPos(&pForm, x, y, &ptCS);
	ASSERT(!bRetrackMouse && iAreaType == uiForm::CAT_CLIENT);
	if (m_pHoverForm != pForm)
	{
		bRetrackMouse = true;
		return;
	}

	if (GetKeyState(VK_F4) < 0 && m_pHoverForm != nullptr)
		m_pHoverForm->RedrawForm();

	if (m_pHoverForm != nullptr)
		m_pHoverForm->OnMouseMove(ptCS.x, ptCS.y, MmdFlags);
}

void uiWindow::OnMouseBtnDown(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y)
{
	printx("---> uiWindow::OnMouseBtnDown Type:%d x:%d y:%d\n", KeyType, x, y);

	switch (KeyType)
	{
	case MKT_LEFT:
		if (DragSizingEventCheck(x, y))
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

	uiFormBase *pDestForm = (m_pMouseFocusForm != nullptr) ? m_pMouseFocusForm : m_pHoverForm;
	if (pDestForm == nullptr)
		return;

	uiPoint ptCS = pDestForm->WindowToClient(uiPoint(x, y));
	pDestForm->OnMouseBtnDown(KeyType, ptCS.x, ptCS.y);
}

void uiWindow::OnMouseBtnUp(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y)
{
	printx("---> uiWindow::OnMouseBtnUp Type:%d x:%d y:%d\n", KeyType, x, y);

	uiFormBase *pDestForm = (m_pMouseFocusForm != nullptr) ? m_pMouseFocusForm : m_pHoverForm;
	if (pDestForm != nullptr)
	{
		uiPoint ptCS = pDestForm->WindowToClient(uiPoint(x, y));
		pDestForm->OnMouseBtnUp(KeyType, ptCS.x, ptCS.y);
	}

	if (KeyType == m_MouseDragKey && DragEventForMouseBtnUp(x, y)) // Check special event.
		return;

	pDestForm = (m_pMouseFocusForm != nullptr) ? m_pMouseFocusForm : m_pHoverForm;
	if (pDestForm == nullptr) // Re-check to avoid accessing null pointer. Hover form might be deleted after key up event is called.
		return;

	UINT64 ctime = GetTimeStamp();
	MOUSE_KEY_TYPE ClickedKey = MKT_NONE;

	switch (KeyType)
	{
	case MKT_LEFT:
		if (m_TrackMouseClick & MCKT_LEFT)
		{
			if (ctime - m_MouseKeyDownTime[MKT_LEFT] <= MOUSE_CLICK_INTERVAL)
				ClickedKey = MKT_LEFT;
			m_TrackMouseClick &= ~MCKT_LEFT;
		}
		break;

	case MKT_MIDDLE:
		if (m_TrackMouseClick & MCKT_MIDDLE)
		{
			if (ctime - m_MouseKeyDownTime[MKT_MIDDLE] <= MOUSE_CLICK_INTERVAL)
				ClickedKey = MKT_MIDDLE;
			m_TrackMouseClick &= ~MCKT_MIDDLE;
		}
		break;

	case MKT_RIGHT:
		if (m_TrackMouseClick & MCKT_RIGHT)
		{
			if (ctime - m_MouseKeyDownTime[MKT_RIGHT] <= MOUSE_CLICK_INTERVAL)
				ClickedKey = MKT_RIGHT;
			m_TrackMouseClick &= ~MCKT_RIGHT;
		}
		break;

	default:
		ASSERT(0);
	}

	if (ClickedKey != MKT_NONE)
	{
		uiPoint ptCS = pDestForm->WindowToClient(uiPoint(x, y));
		pDestForm->OnMouseBtnClk(ClickedKey, ptCS.x, ptCS.y);
		m_pFirstClickedForm[ClickedKey] = pDestForm;
	}
}

void uiWindow::OnMouseBtnDbClk(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y)
{
	printx("---> uiWindow::OnMouseBtnDbClk Type:%d x:%d y:%d\n", KeyType, x, y);

	uiFormBase *pDestForm = (m_pMouseFocusForm != nullptr) ? m_pMouseFocusForm : m_pHoverForm;
	if (pDestForm == nullptr)
		return;

	BOOL bDBClick = TRUE;
	switch (KeyType)
	{
	case MKT_LEFT:
		if (m_pFirstClickedForm[MKT_LEFT] != pDestForm)
			bDBClick = FALSE;
		break;
	case MKT_MIDDLE:
		if (m_pFirstClickedForm[MKT_MIDDLE] != pDestForm)
			bDBClick = FALSE;
		break;
	case MKT_RIGHT:
		if (m_pFirstClickedForm[MKT_RIGHT] != pDestForm)
			bDBClick = FALSE;
		break;
	}

	uiPoint ptCS = pDestForm->WindowToClient(uiPoint(x, y));
	if (bDBClick)
		pDestForm->OnMouseBtnDbClk(KeyType, ptCS.x, ptCS.y);
	else
		pDestForm->OnMouseBtnDown(KeyType, ptCS.x, ptCS.y); // Windows replaces button down message with double click message if it happened.
}

void uiWindow::OnMouseWheel(SHORT scX, SHORT scY, INT z, UINT_PTR kf)
{
	printx("---> uiWindow::OnMouseWheel. x: %d, y: %d, z: %d\n", scX, scY, z);
}

void uiWindow::MouseEnterForm(uiFormBase *pForm, INT x, INT y)
{
	ASSERT(!pForm->IsMouseHovering());
	pForm->FBSetFlag(uiFormBase::FBF_MOUSE_HOVER); // Must set this flag first.
	pForm->OnMouseEnter(x, y);
}

void uiWindow::MouseLeaveForm(uiFormBase *pForm)
{
	ASSERT(pForm->IsMouseHovering());
	pForm->FBCleanFlag(uiFormBase::FBF_MOUSE_HOVER); // Must set this flag first.
	pForm->OnMouseLeave();
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
	m_CurrentType = DCT_TOTAL; // Specify an invalid value to force updating cursor when mouse goes inside the window first time.

	m_hArray[DCT_NORMAL] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_NORMAL), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[DCT_SIZE_NS] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENS), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[DCT_SIZE_EW] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZEWE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[DCT_SIZE_NESW] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENESW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	m_hArray[DCT_SIZE_NWSE] = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_SIZENWSE), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
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
	::SetCursor(NULL); // Must call this to remove the using cursor.
}

BOOL uiWinCursor::Set(uiImage&& img)
{
	if (!img.IsValid() || img.GetType() != WIT_CURSOR)
		return FALSE;
	if (img == m_CustomCursor)
		return TRUE;

	HANDLE hPrev = ::SetCursor((HCURSOR)img.GetHandle()); // Set fisrt, then assign the image.
	m_CustomCursor = std::move(img);
	return TRUE;
}

void uiWinCursor::Update(DEFAULT_CURSOR_TYPE dct)
{
	ASSERT(dct < DCT_TOTAL);

	if (dct != m_CurrentType || m_CustomCursor.IsValid())
	{
		::SetCursor(m_hArray[dct]); // Muse set first to release internal reference count to the image.
		m_CustomCursor.Release();
		m_CurrentType = dct;
	}
}


