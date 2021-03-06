

#pragma once


#include "uiCommon.h"
#include "uiForm.h"
#include "uiControl.h"


#define DEFAULT_BACKBUFFER_COUNT 0

#define WM_CUSTOM   (WM_USER + 0x0001)
#define WM_CTRL_MSG (WM_USER + 0x0002)


BOOL uiMessageLookUp(UINT message);


INLINE uiWindow* uiWindowGet(HWND hWnd)
{
	return (uiWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
}


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
	uiFormBase::TimerCallback tcb;

};

struct stMsgProcRetInfo
{
	stMsgProcRetInfo() :bProcessed(FALSE), ret(0) {}
	BOOL    bProcessed;
	LRESULT ret;
};

struct stWindowCreateParam
{
	stWindowCreateParam(INT xIn, INT yIn, UINT cxIn, UINT cyIn, BOOL bVisibleIn)
	:x(xIn), y(yIn), cx(cxIn), cy(cyIn), bVisible(bVisibleIn)
	{
	}

	INT  x, y;
	UINT cx, cy;
	BOOL bVisible;
};

struct stDialogCreateParam
{
	stDialogCreateParam(uiFormBase* pFormIn, uiFormBase* pParentIn)
	:pForm(pFormIn), pParent(pParentIn)
	{
	}

	INLINE void Set(INT xIn, INT yIn, UINT cxIn, UINT cyIn, FORM_CREATION_FLAG fcfIn)
	{
		x = xIn; y = yIn; cx = cxIn; cy = cyIn;
		fcf = fcfIn;
	}

	uiFormBase* pForm;
	uiFormBase* pParent;
	INT x, y;
	UINT cx, cy;
	FORM_CREATION_FLAG fcf;
};


class uiWindow
{
public:

	enum
	{
		MOUSE_CLICK_INTERVAL = 300,
	};

	enum WINDOW_CLASS_NAME
	{
		WCN_NORMAL,
		WCN_MENU,

		WCN_TOTAL
	};


	uiWindow(uiFormBase *pFormIn);
	virtual ~uiWindow();

	BOOL DockForm(uiFormBase* pForm);

	void OnFormPreClose(uiFormBase* pForm);
	void OnFormPostClose(uiFormBase* pForm);
	void OnFormDestroy(uiFormBase* pForm);
	void OnFormHide(uiFormBase* pForm);

	void CloseImp();
	BOOL CloseDialogImp(INT_PTR ret);
	BOOL MoveImp(INT scX, INT scY);
	BOOL MoveByOffsetImp(INT x, INT y);
	void MoveToCenter();
	BOOL UpdateAT(uiFormBase* pActiveForm, uiFormBase*& pPrevForm);
	void SetActiveImp(uiFormBase* pActiveForm, uiFormBase*& pPrevForm);
	void ShowImp(FORM_SHOW_MODE sm);
	BOOL SizeImp(UINT nWidth, UINT nHeight);
	BOOL StartDraggingImp(uiFormBase *pForm, MOUSE_KEY_TYPE mkt, INT x, INT y, uiRect MouseMoveRect);
	void RedrawImp(const uiRect* pRect);
	void ResizeImp(INT x, INT y);
	void PostMsgHandler(UINT msg);
	void FormSizingCheck(uiFormBase* pForm, UINT nSide, uiRect* pRect);
	void RetrackMouseCheck(uiFormBase* pFormBase);
	void UpdateCursor(uiFormBase* pForm, INT csX, INT csY);


	void PopupMenuImp(uiMenu* pRootMenu, INT wsX, INT wsY);


	uiFormBase* CaptureMouseFocus(uiFormBase* pForm);
	BOOL ReleaseMouseFocus(uiFormBase* pForm);

	uiFormBase* SetKBFocusImp(uiFormBase* pForm);

	BOOL CaretShowImp(uiFormBase* pFormBase, INT x, INT y, INT width, INT height);
	BOOL CaretHideImp(uiFormBase* pFormBase);
	BOOL CaretMoveImp(uiFormBase* pFormBase, INT x, INT y);
	BOOL CaretMoveByOffset(uiFormBase* pFormBase, INT OffsetX, INT OffsetY);

	UINT TimerAdd(uiFormBase* pFormBase, UINT id, UINT msElapsedTime, INT nRunCount, void* pCtx);
	BOOL TimerClose(uiFormBase* pFormBase, UINT key, BOOL bByID);
	void TimerRemoveAll(uiFormBase* const pFormBase);

	void OnActivate(BOOL& bProcessed, WPARAM wParam, LPARAM lParam);
	void OnNCActivate(WPARAM wParam, LPARAM lParam);
	void OnMouseActivate(LPARAM& ret, WPARAM wParam, LPARAM lParam);
	BOOL OnClose();
	BOOL OnCreate();
	void OnDestroy();
	void OnKeyDown(LRESULT& lRet, WPARAM wParam, LPARAM lParam);
	void OnKeyUp(LRESULT& lRet, WPARAM wParam, LPARAM lParam);
	void OnSysKeyDown(LRESULT& lRet, WPARAM wParam, LPARAM lParam);
	void OnSysKeyUp(LRESULT& lRet, WPARAM wParam, LPARAM lParam);
	void OnMove(INT scx, INT scy);
	void OnPaint();
	void OnKBSetFocus(HWND hOldFocusWnd);
	void OnKBKillFocus(HWND hNewFocusWnd);
	void OnSize(UINT_PTR nType, UINT nNewWidth, UINT nNewHeight);
	void OnSizing(INT_PTR fwSide, RECT* pRect);
	void OnTimer(const UINT_PTR TimerID, LPARAM lParam);

	LRESULT OnNCHitTest(INT scX, INT scY);

	BOOL DragSizingEventCheck(INT x, INT y);
	BOOL DragEventForMouseBtnUp(INT wcX, INT wcY);
	void OnMouseCaptureLost(HWND hNewWnd);
	void OnDragging(INT x, INT y);

	void OnMouseLeave();
	void OnMouseMove(UINT_PTR nType, const INT x, const INT y);
	void OnMouseBtnDown(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);
	void OnMouseBtnUp(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);
	void OnMouseBtnDbClk(const MOUSE_KEY_TYPE KeyType, const INT x, const INT y);
	void OnMouseWheel(SHORT scX, SHORT scY, INT z, UINT_PTR kf);

	void MouseEnterForm(uiFormBase *pForm, INT x, INT y);
	void MouseLeaveForm(uiFormBase *pForm);


	INLINE void RetrackMouse() { if (!m_bDragging) bRetrackMouse = true; }
	INLINE BOOL IsDialog() const { return bIsDialog; }
	INLINE void SetAsDialog() { ASSERT(!bIsDialog); bIsDialog = true; }

	INLINE HWND GetHandle() const { return m_Handle; }
	INLINE void SetHandle(HWND hWnd) { m_Handle = hWnd; }
	INLINE uiPoint ClientToScreen(uiPoint& pt) const { return (pt += uiPoint(m_ScreenCoordinateX, m_ScreenCoordinateY)); }
	INLINE void ClientToScreen(INT& x, INT& y) const { x += m_ScreenCoordinateX; y += m_ScreenCoordinateY; }
	INLINE void ScreenToClient(INT& x, INT& y) const { x -= m_ScreenCoordinateX; y -= m_ScreenCoordinateY; }
//	INLINE void ScreenToClient(INT& x, INT& y) const { uiPoint pt(x, y); ::ScreenToClient(m_Handle, (POINT*)&pt); x = pt.x; y = pt.y; }
	INLINE BOOL PostMessage(UINT msg, WPARAM wParam, LPARAM lParam) const { return ::PostMessage(m_Handle, msg, wParam, lParam); }
	INLINE BOOL UpdateWindow() const { return ::UpdateWindow(m_Handle); }
	INLINE BOOL GetWindowRect(uiRect &rect) const { return ::GetWindowRect(m_Handle, (LPRECT)&rect); }
	INLINE BOOL GetClientRect(uiRect &rect) const { return ::GetClientRect(m_Handle, (LPRECT)&rect); }
	INLINE BOOL ShowWindow(INT nCmdShow) const { return ::ShowWindow(m_Handle, nCmdShow); }
	INLINE HWND SetCapture() { ASSERT(!bMouseFocusCaptured); bMouseFocusCaptured = true; return ::SetCapture(m_Handle); }
	INLINE BOOL ReleaseCapture() const { return ::ReleaseCapture(); }
	INLINE BOOL SetWindowText(const uiString& str) const { return ::SetWindowText(m_Handle, str); }
	INLINE BOOL IsActive() const { return m_bActive; }
	INLINE BOOL HasKBFocus() const { return bKBFocusCaptured; }

	INLINE static BOOL GetDesktopWorkArea(uiRect& rect) { return ::SystemParametersInfo(SPI_GETWORKAREA, 0, (RECT*)&rect, 0); }

	INLINE static HGLOBAL GetDialogTemplate() { return hGDialogTemplate; }
	INLINE static void SetDialogTemplate(HGLOBAL hMem) { hGDialogTemplate = hMem; }

	static uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase* pForm, uiFormBase* ParentForm, const stWindowCreateParam& wcp);
	static INT_PTR CreateModalDialog(const stDialogCreateParam* pDCP);

	static const TCHAR* GetClassName(WINDOW_CLASS_NAME wcn) { ASSERT(wcn < WCN_TOTAL); return WndClassName[wcn]; }
	static void RegisterWindowClass(BOOL bReg);

	void Dbg() const { if (uiMessageLookUp(WM_KILLFOCUS)) _CrtDbgBreak(); }

	INLINE void SetActiveWindow(uiFormBase*& pPrevForm) const
	{
		DEBUG_CHECK(Dbg());
		ASSERT(!m_bActive);
		ASSERT(::GetActiveWindow() != m_Handle);
		HWND hWndPrev = ::SetActiveWindow(m_Handle);
		pPrevForm = (hWndPrev == NULL) ? nullptr : uiWindowGet(hWndPrev)->m_pActiveForm;
	}

	INLINE BOOL AddRef() { return ++m_RefCount; }
	INLINE INT ReleaseRef()
	{
		INT ref = --m_RefCount;
		if (ref == 0)
			delete this;
		return ref;
	}


	// For debugging.
	// Don't use SetWindowPos to show windows.
//	INLINE LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam) { return ::SendMessage(m_Handle, Msg, wParam, lParam); }
	//INLINE HWND SetActiveWindow()
	//{
	//	ASSERT(!uiMessageLookUp(WM_KILLFOCUS));
	//	return ::SetActiveWindow(m_Handle);
	//}


protected:

	enum MOUSE_CLICKING_KEY_TRACK
	{
		MCKT_LEFT   = 0x01,
		MCKT_MIDDLE = 0x01 << 1,
		MCKT_RIGHT  = 0x01 << 2,
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
			VERIFY(::TrackMouseEvent(&tme));
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
			VERIFY(::TrackMouseEvent(&tme));
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


	static HGLOBAL hGDialogTemplate;
	static const TCHAR* WndClassName[];


	HWND m_Handle;

	uiFormBase* m_pForm;
	uiFormBase* m_pActiveForm;
	uiFormBase* m_pHoverForm;
	uiFormBase* m_pGrayForm;  // Mouse enters the form but doesn't leave its sizeable border region.
	uiFormBase* m_pDraggingForm;
	uiFormBase* m_pMouseFocusForm;
	uiFormBase* m_pKeyboardFocusForm;

	uiPoint m_LastMousePos; // window client spcae
	UINT8   m_TrackMouseClick;
	INT8    m_RefCount;

	UINT m_AreaType, m_SizingHitSide;
	INT  m_ScreenCoordinateX, m_ScreenCoordinateY;
	INT  m_MDPosX, m_MDPosY;

	MOUSE_KEY_TYPE m_MouseDragKey;
	UINT64 m_MouseKeyDownTime[MKT_TOTAL];
	uiFormBase* m_pFirstClickedForm[MKT_TOTAL];

	UINT m_TotalWorkingTimer;
	std::vector<stWndTimerInfo> m_TimerTable;

	uiDrawerInsType m_Drawer;

	bool m_bTrackMouseLeave = false;
	bool m_bDragging = false;
	bool m_bSizing = false;
	bool bRetrackMouse = false;
	bool bMouseFocusCaptured = false;
	bool bKBFocusCaptured = false;
	bool bIsDialog = false;
//	bool bWindowMove = false;
	bool m_bActive = false;


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

	enum DEFAULT_CURSOR_TYPE
	{
		DCT_NORMAL,
		DCT_SIZE_NS,
		DCT_SIZE_EW,
		DCT_SIZE_NESW,
		DCT_SIZE_NWSE,

		DCT_TOTAL,
	};

	uiWinCursor();
	~uiWinCursor();

	BOOL Set(uiImage&& img);
	void Update(DEFAULT_CURSOR_TYPE dct);

	INLINE void Reset() { m_CurrentType = DCT_TOTAL; }
	INLINE BOOL GetPos(uiPoint& pt) const { return ::GetCursorPos((POINT*)&pt); }


protected:

	INT     m_CurrentType;
	HCURSOR m_hArray[DCT_TOTAL];
	uiImage m_CustomCursor;


};


INLINE BOOL WndClientToScreen(uiWindow* pWnd, INT& x, INT& y)
{
	pWnd->ClientToScreen(x, y);
	return TRUE;
}

INLINE BOOL WndCreateMessage(uiWindow* pWnd, uiFormBase* pSrc, UINT_PTR id)
{
	BOOL bResult = (pWnd->PostMessage(WM_CTRL_MSG, (WPARAM)pSrc, id) != 0);
	VERIFY(bResult);
	return bResult;
}


BOOL InitWindowSystem();
BOOL CloseWindowSystem();


