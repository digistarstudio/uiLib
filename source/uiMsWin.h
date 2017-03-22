

#pragma once


#include <Windows.h>
#include <WinUser.h>
#include "uiCommon.h"
#include "uiForm.h"
#include "uiControl.h"


#define DEFAULT_BACKBUFFER_COUNT 0


BOOL uiMessageLookUp(UINT message);


struct stWndTimerInfo
{
	stWndTimerInfo()
	:pFormBase(nullptr), TimerHandle(0)
	{
		id = 0;
		msElapsedTime = 0;
		pCtx = nullptr;
		nRunCount = 0;
	}

	// Data matchs uiFormBase::stTimerInfo.
	UINT  id;
	UINT  msElapsedTime;
	void* pCtx;
	INT   nRunCount;

	uiFormBase* pFormBase;
	UINT_PTR    TimerHandle;

};

class uiWindow
{
public:

	enum
	{
		MOUSE_CLICK_INTERVAL = 300,
	};

	uiWindow(uiFormBase *pFormIn);
	virtual ~uiWindow();

	BOOL DockForm(uiFormBase *pForm);
	void OnFormDestroy(uiFormBase *pForm);
	void OnFormHide(uiFormBase *pForm);

	void CloseImp();
	BOOL MoveImp(INT scX, INT scY);
	BOOL MoveByOffsetImp(INT x, INT y);
	void MoveToCenter();
	void ShowImp(FORM_SHOW_MODE sm);
	BOOL SizeImp(UINT nWidth, UINT nHeight);
	BOOL StartDraggingImp(uiFormBase *pForm, MOUSE_KEY_TYPE mkt, INT x, INT y, uiRect MouseMoveRect);
	void RedrawImp(const uiRect* pRect);
	void ResizeImp(INT x, INT y);
	void PostMsgHandler(UINT msg);
	void FormSizingCheck(uiFormBase *pForm, UINT nSide, uiRect *pRect);
	void RetrackMouseCheck(uiFormBase *pFormBase);

	uiFormBase* CaptureMouseFocus(uiFormBase* pForm);
	BOOL ReleaseMouseFocus(uiFormBase* pForm);

	BOOL CaretShowImp(uiFormBase *pFormBase, INT x, INT y, INT width, INT height);
	BOOL CaretHideImp(uiFormBase *pFormBase);
	BOOL CaretMoveImp(uiFormBase *pFormBase, INT x, INT y);
	BOOL CaretMoveByOffset(uiFormBase *pFormBase, INT OffsetX, INT OffsetY);

	UINT TimerAdd(uiFormBase *pFormBase, UINT id, UINT msElapsedTime, INT nRunCount, void* pCtx);
	BOOL TimerClose(uiFormBase *pFormBase, UINT key, BOOL bByID);
	void TimerRemoveAll(uiFormBase* const pFormBase);

	void OnActivate(WPARAM wParam, LPARAM LParam);
	BOOL OnClose();
	void OnCreate();
	void OnDestroy();
	void OnKeyDown(INT iKey);
	void OnKeyUp(INT iKey);
	void OnMove(INT scx, INT scy);
	void OnNCPaint(HWND hWnd, HRGN hRgn);
	void OnPaint();
//	BOOL OnSetCursor();
	void OnGetKBFocus(HWND hOldFocusWnd);
	void OnLoseKBFocus();
	void OnSize(UINT nType, UINT nNewWidth, UINT nNewHeight);
	void OnSizing(INT fwSide, RECT *pRect);
	void OnTimer(const UINT_PTR TimerID, LPARAM lParam);

	LRESULT OnNCHitTest(INT x, INT y);

	BOOL DragSizingEventCheck(INT x, INT y);
	void DragEventForMouseBtnUp(INT wcX, INT wcY);
	void OnMouseCaptureLost();
	void OnDragging(INT x, INT y);

	void OnMouseLeave();
	void OnMouseMove(UINT nType, const INT x, const INT y);
	void OnMouseBtnDown(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);
	void OnMouseBtnUp(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);
	void OnMouseBtnDbClk(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);

	void MouseEnterForm(uiFormBase *pForm, INT x, INT y);
	void MouseLeaveForm(uiFormBase *pForm);


	INLINE void RetrackMouse() { if (!m_bDragging) bRetrackMouse = true; }

	INLINE HWND GetHandle() const { return m_Handle; }
	INLINE void SetHandle(HWND hWnd) { m_Handle = hWnd; }
	INLINE void ClientToScreen(INT& x, INT& y) { x += m_ScreenCoordinateX; y += m_ScreenCoordinateY; }
	INLINE void ScreenToClient(INT& x, INT& y) { x -= m_ScreenCoordinateX; y -= m_ScreenCoordinateY; }
	INLINE BOOL PostMessage(UINT msg, WPARAM wParam, LPARAM lParam) const { return ::PostMessage(m_Handle, msg, wParam, lParam); }
	INLINE BOOL UpdateWindow() { return ::UpdateWindow(m_Handle); }
	INLINE BOOL GetWindowRect(uiRect &rect) { return ::GetWindowRect(m_Handle, (LPRECT)&rect); }
	INLINE BOOL GetClientRect(uiRect &rect) { return ::GetClientRect(m_Handle, (LPRECT)&rect); }
	INLINE BOOL ShowWindow(INT nCmdShow) { return ::ShowWindow(m_Handle, nCmdShow); }
	INLINE HWND SetCapture() { return ::SetCapture(m_Handle); }
	INLINE BOOL ReleaseCapture() { return ::ReleaseCapture(); }

	// For debugging.
	// Don't use SetWindowPos to show windows.
	INLINE LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) { return ::SendMessage(m_Handle, Msg, wParam, lParam); }
	INLINE HWND SetActiveWindow()
	{
		ASSERT(!uiMessageLookUp(WM_KILLFOCUS));
		return ::SetActiveWindow(m_Handle);
	}


protected:

	enum MOUSE_CLICKING_KEY_TRACK
	{
		MCKT_LEFT = 0x01,
		MCKT_MIDDLE = 0x01 << 1,
		MCKT_RIGHT = 0x01 << 2,
	};

	void TrackMouseLeave(BOOL bOn)
	{
		//	printx("---> TrackMouseLeave\n");
		if (bOn && !m_bTrackMouseLeave)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE; // TME_HOVER | TME_LEAVE
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = m_Handle;
			VERIFY(::TrackMouseEvent(&tme) != 0);
			m_bTrackMouseLeave = true;
			return;
		}
		if (!bOn && m_bTrackMouseLeave)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.dwFlags = TME_LEAVE | TME_CANCEL;
			tme.dwHoverTime = HOVER_DEFAULT;
			tme.hwndTrack = m_Handle;
			VERIFY(::TrackMouseEvent(&tme) != 0);
			m_bTrackMouseLeave = true;
		}
	}

	UINT64 GetTimeStamp()
	{
		//return timeGetTime();
		FILETIME filetime;
		GetSystemTimeAsFileTime(&filetime);
		return ((((UINT64)filetime.dwHighDateTime) << 32) + filetime.dwLowDateTime) / 10000; // Convert to millisecond.
	}


	HWND m_Handle;

	uiFormBase *m_pForm;
	uiFormBase *m_pHoverForm;
	uiFormBase *m_pGrayForm;  // Mouse enters the form but doesn't leave its sizeable border region.
	uiFormBase *m_pDraggingForm;
	uiFormBase *m_pMouseFocusForm;
	uiFormBase *m_pKeyboardFocusForm;

	POINT m_LastMousePos;
	UINT8 m_TrackMouseClick;

	INT m_NonClientArea, m_SizingHitSide;
	INT m_ScreenCoordinateX, m_ScreenCoordinateY;
	INT m_MDPosX, m_MDPosY;

	MOUSE_KEY_TYPE m_MouseDragKey;
	UINT64 m_MouseKeyDownTime[MKT_TOTAL];
	uiFormBase *m_pFirstClickedForm[MKT_TOTAL];

	UINT m_TotalWorkingTimer;
	std::vector<stWndTimerInfo> m_TimerTable;

	uiWndDrawer m_Drawer;

	bool m_bTrackMouseLeave = false;
	bool m_bDragging = false;
	bool m_bSizing = false;
	bool bChangeCursor = false;
	bool bRetrackMouse = false;
	bool bMouseFocusCaptured = false;
	bool bKBFocusCaptured = false;


};


class uiWinMenu
{
public:

	uiWinMenu()
	:m_hMenu(NULL)
	{
	}
	~uiWinMenu()
	{
	}


	BOOL CreateMenu();
	BOOL CreatePopupMenu();
	void DestroyMenu();

	BOOL InsertItem(TCHAR* pText, UINT id, INT index);
	UINT GetMenuItemCount() const;

	BOOL Popup(HWND hWnd, INT x, INT y);

	void ChangeStyle();


protected:

	HMENU m_hMenu;


};


class uiWinCaret
{
public:

	uiWinCaret()
	:m_x(-1), m_y(-1)
	{
	}
	~uiWinCaret() = default;

	INLINE BOOL Destroy()
	{
		return ::DestroyCaret();
	}
	/*
	INLINE BOOL Hide(HWND hWnd)
	{
		return ::HideCaret(hWnd);
	}//*/
	INLINE BOOL SetCaret(HWND hWnd, HBITMAP hBmp, INT width, INT height)
	{
		return ::CreateCaret(hWnd, hBmp, width, height);
	}
	INLINE BOOL SetPos(INT x, INT y)
	{
		if (::SetCaretPos(x, y))
		{
			m_x = x; m_y = y;
			return TRUE;
		}
		return FALSE;
	}
	INLINE BOOL MoveByOffset(INT OffsetX, INT OffsetY)
	{
		return SetPos(OffsetX + m_x, OffsetY + m_y);
	}
	INLINE BOOL Show(HWND hWnd, INT x, INT y)
	{
		if (::SetCaretPos(x, y) && ::ShowCaret(hWnd))
		{
			m_x = x; m_y = y;
			return TRUE;
		}
		return FALSE;
	}

protected:

	INT m_x, m_y;

};


class uiWinCursor
{
public:

	enum CURSOR_TYPE
	{
		CT_NORMAL,
		CT_SIZE_NS,
		CT_SIZE_EW,
		CT_SIZE_NESW,
		CT_SIZE_NWSE,

		CT_TOTAL,
	};

	uiWinCursor();
	~uiWinCursor();


	void Set(CURSOR_TYPE type);
//	void Update();
	void Update(uiFormBase::NON_CLIENT_HIT_TEST nca);
	void StartSizing(BOOL bComplete);

	INLINE BOOL GetPos(uiPoint& pt) const { return ::GetCursorPos((POINT*)&pt); }


protected:

	INT m_CurrentType, m_SizingType;
	HCURSOR m_hArray[CT_TOTAL];
	bool m_bSizing;


};


uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase *pForm, uiFormBase *ParentForm, INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight, BOOL bVisible);


