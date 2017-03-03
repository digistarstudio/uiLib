

#pragma once


#include <Windows.h>
#include <WinUser.h>
#include "uiCommon.h"
#include "uiForm.h"


class uiFormBase;


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
	BOOL GetUiRect(uiRect &rect);
	void OnFormDestroy(uiFormBase *pForm);
	void OnFormHide(uiFormBase *pForm);

	void CloseImp();
	BOOL MoveImp(INT x, INT y);
	BOOL MoveByOffsetImp(INT x, INT y);
	void ShowImp(FORM_SHOW_MODE sm);
	BOOL SizeImp(UINT nWidth, UINT nHeight);
	void StartDraggingImp(uiFormBase *pForm, INT x, INT y, uiRect MouseMoveRect);
	void RedrawImp(const uiRect* pRect);
	void ResizeImp(INT x, INT y);
	void PostMsgHandler(UINT msg);
	void FormSizing(uiFormBase *pForm, UINT nSide, uiRect *pRect);
	void MoveToCenter();

	BOOL OnClose();
	void OnCreate();
	void OnDestroy();
	void OnKeyDown(INT iKey);
	void OnKeyUp(INT iKey);
	void OnMove();
	void OnNCPaint(HWND hWnd, HRGN hRgn);
	void OnPaint();
	BOOL OnSetCursor();
	void OnSize(UINT nType, UINT nNewWidth, UINT nNewHeight);
	void OnSizing(INT fwSide, RECT *pRect);

	INT64 OnNCHitTest(INT x, INT y);
	BOOL OnLButtonDown(INT x, INT y);
	BOOL OnLButtonUp(INT x, INT y);
	void OnDragging(INT x, INT y);

	void OnMouseLeave();
	void OnMouseMove(UINT nType, INT x, INT y);
	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y);

	void MouseEnterForm(uiFormBase *pForm, INT x, INT y);
	void MouseLeaveForm(uiFormBase *pForm);

	INLINE void ClientToScreent(INT& x, INT& y) { POINT pt = { x, y }; ::ClientToScreen((HWND)m_Handle, &pt); x = pt.x; y = pt.y; }
	INLINE void SetHandle(void *HandleIn) { m_Handle = HandleIn; }
	INLINE void* GetHandle() const { return m_Handle; }
	INLINE BOOL PostMessage(UINT msg, WPARAM wParam, LPARAM lParam) const { return ::PostMessage((HWND)m_Handle, msg, wParam, lParam); }
	INLINE BOOL UpdateWindow() { return ::UpdateWindow((HWND)m_Handle); }
	INLINE BOOL GetWindowRect(uiRect &rect) { return ::GetWindowRect((HWND)m_Handle, (LPRECT)&rect); }
	INLINE BOOL ShowWindow(INT nCmdShow) { return ::ShowWindow((HWND)m_Handle, nCmdShow); }


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
			tme.hwndTrack = (HWND)m_Handle;
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
			tme.hwndTrack = (HWND)m_Handle;
			VERIFY(::TrackMouseEvent(&tme) != 0);
			m_bTrackMouseLeave = true;
		}
	}

	UINT64 GetTimeStamp()
	{
		//	return timeGetTime();
		FILETIME filetime;
		GetSystemTimeAsFileTime(&filetime);
		return ((((UINT64)filetime.dwHighDateTime) << 32) + filetime.dwLowDateTime) / 10000; // Convert to millisecond.
	}

	void OnDragging();

	void *m_Handle;

	uiFormBase *m_pForm;
	uiFormBase *m_pHoverForm;
	uiFormBase *m_pDraggingForm;

	uiWndDrawer m_Drawer;

	POINT m_LastMousePos;
	UINT8 m_TrackMouseClick;

	INT m_NonClientArea, m_SizingHitSide;
	INT m_MDPosX, m_MDPosY;

	UINT64 m_MouseKeyDownTime[MKT_TOTAL], m_MouseClickTime[MKT_TOTAL];
	uiFormBase *m_pFirstClickedForm[MKT_TOTAL];

	bool m_bTrackMouseLeave;
	bool m_bDragging;
	bool m_bSizing = false;
	bool m_bClosed;
	bool bChangeCursor = false;
	bool bRetrackMouse = false;


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
	void Update();
	void StartSizing(BOOL bComplete);


protected:

	INT m_CurrentType, m_SizingType;
	HCURSOR m_hArray[CT_TOTAL];
	bool m_bSizing;


};


uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase *pForm, uiFormBase *ParentForm, INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight, BOOL bVisible);


