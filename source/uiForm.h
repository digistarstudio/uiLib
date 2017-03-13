

#pragma once


#include "uiDrawer.h"
#include "uiCommon.h"


#define START_DRAGGING 1


class uiMenu;
class uiFormBase;
class uiWindow;
struct uiFormStyle;


BOOL WndClientToScreen(uiWindow *pWnd, INT &x, INT &y); // Declare this for inline function.
BOOL WndCreateMessage(uiWindow *pWnd, uiFormBase *pSrc, UINT id);
void uiMonitorEvent();


enum FORM_SHOW_MODE
{
	FSM_HIDE,
	FSM_SHOW,

	// For base window only.
	FSM_RESTORE,
	FSM_MINIMIZE,
	FSM_MAXIMIZE,
};

enum UI_WINDOW_TYPE
{
	UWT_NORMAL,
	UWT_TOOL,
	UWT_MENU,
};

enum FORM_CREATION_FLAG
{
	FCF_NONE = 0x00,

	FCF_INVISIBLE   = 0x01,
	FCF_POPUP       = 0x01 << 1,
	FCF_TOOL        = 0x01 << 2,

	FCF_CENTER      = 0x01 << 3,
	FCF_CLIENT_RECT = 0x01 << 4, // Create form with specific client rectangle size.
};

IMPLEMENT_ENUM_FLAG(FORM_CREATION_FLAG)

enum MOUSE_KEY_TYPE
{
	MKT_LEFT = 0,
	MKT_MIDDLE,
	MKT_RIGHT,

	MKT_TOTAL,
	MKT_NONE,
};

enum MOVE_DIRECTION
{
	MOVE_NONE  = 0x00,
	MOVE_UP    = 0x01,
	MOVE_DOWN  = 0x01 << 1,
	MOVE_LEFT  = 0x01 << 2,
	MOVE_RIGHT = 0x01 << 3,
};

IMPLEMENT_ENUM_FLAG(MOVE_DIRECTION)

enum FORM_CLASS
{
	FC_BASE,

	FC_TOOLBAR,
	FC_FORM,
	FC_HEADER_BAR,

	FC_CONTROL,
	FC_BUTTON,
	FC_STATIC,

};

enum FORM_DOCKING_FLAG
{
	FDF_TOP    = 0x01,
	FDF_BOTTOM = 0x01 << 1,
	FDF_LEFT   = 0x01 << 2,
	FDF_RIGHT  = 0x01 << 3,

	FDF_FRAME  = 0x01 << 4,
	FDF_AUTO_SIZE = 0x01 << 5,
};

IMPLEMENT_ENUM_FLAG(FORM_DOCKING_FLAG)

struct stFormSplitter
{
	stFormSplitter()
	{
	}

	UINT nSplitterWidth;
	uiRect RectFull, RectTopLeft, RectBottomRight;
	stFormSplitter *pTopLeft = nullptr;
	stFormSplitter *pBottomRight = nullptr;
	uiFormBase *pTopLeftForm = nullptr; // This's valid only when pTopLeft is null pointer.
	uiFormBase *pBottomRightForm = nullptr; // This's valid only when pBottomRight is null pointer.
	bool bVertical = false;

};


class uiFormBase
{
public:

	enum SHOW_FLAG
	{
		SHOW_TITLE = 1,
		SHOW_CLOSE,

		SHOW_BORDER,
	};

	enum NON_CLIENT_HIT_TEST
	{
		NCHT_CLIENT = 0, // Don't change this.

		NCHT_TOP    = 0x01,
		NCHT_BOTTOM = 0x01 << 1,
		NCHT_LEFT   = 0x01 << 2,
		NCHT_RIGHT  = 0x01 << 3,
	};

	struct stFormMoveInfo
	{
		stFormMoveInfo()
		:XOffset(0), YOffset(0), MDFlag(MOVE_NONE)
		{
		}
		INT XOffset, YOffset;
		MOVE_DIRECTION MDFlag;
	};

	uiFormBase();
	virtual ~uiFormBase();

//	BOOL Create(uiFormBase *parent, const RECT& rect, FORM_CREATION_FLAG cf);
	BOOL Create(uiFormBase *parent, INT x, INT y, UINT nWidth, UINT nHeight, FORM_CREATION_FLAG fcf = FCF_NONE);

	void Bind(UINT CmdID);
	void Close();
	void DePlate();
	void Move(INT x, INT y);
	void MoveToCenter();
	void MoveByOffset(const INT OffsetX, const INT OffsetY, BOOL bForceRedraw = FALSE);
	void Show(FORM_SHOW_MODE sm);
	BOOL Size(INT NewWidth, INT NewHeight);
	void RedrawForm(const uiRect *pUpdateRect = nullptr);
	void RedrawFrame(const uiRect *pUpdateRect = nullptr);

	uiFormBase* SetCapture();
	BOOL ReleaseCapture();

	BOOL CaretShow(BOOL bShow, INT x = -1, INT y = -1, INT width = -1, INT height = -1);
	BOOL CaretMoveByOffset(INT OffsetX, INT OffsetY);
	BOOL CaretMove(INT x, INT y);

	void PopupMenu(INT x, INT y, uiMenu *pMenu);

	virtual FORM_CLASS GetClass() { return FC_BASE; }

	virtual BOOL DockForm(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf) { ASSERT(0); return FALSE; }
	virtual BOOL SideDock(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf) { ASSERT(0); return FALSE; }

	virtual UTX::CSimpleList* GetSideDockedFormList() { return nullptr; }
	virtual uiFormBase* FindByPos(INT x, INT y, INT *DestX, INT *DestY); // x and y are in frame space.
	virtual void ToWindowSpace(uiFormBase *pForm, uiRect &rect, BOOL bClip);
	virtual void ToWindowSpace(uiFormBase *pForm, INT &x, INT &y);

	void StartDragging(MOUSE_KEY_TYPE mkt, INT wcX, INT wcY);

	void EntryOnCreate(BOOL bShowIn, UINT nWidth, UINT nHeight);
	BOOL EntryOnClose();
	void EntryOnDestroy(uiWindow *pWnd);
	void EntryOnMove(INT x, INT y, const stFormMoveInfo *pInfo);
	void EntryOnPaint(uiDrawer* pDrawer, INT depth);
	void EntryOnSize(UINT nNewWidth, UINT nNewHeight);

	void EntryOnKBGetFocus();
	void EntryOnKBLoseFocus();


	virtual void EntryOnCommand(UINT id);

	virtual uiSize GetMinSize() { return uiSize(-1, -1); }
	virtual uiSize GetMaxSize() { return uiSize(-1, -1); }

	virtual BOOL OnClose() { return TRUE; }
	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnActivate(BOOL bActive) {}

	virtual void OnCommand(INT id, BOOL &bDone);

	virtual BOOL OnDeplate(INT iReason, uiFormBase *pDockingForm);

	virtual void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseEnter(INT x, INT y);
	virtual void OnMouseLeave();
	virtual void OnMouseFocusLost();
	virtual void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd);
	virtual void OnMove(INT x, INT y, const stFormMoveInfo *pInfo);
	virtual INT  OnNCHitTest(INT x, INT y);
	virtual void OnPaint(uiDrawer* pDrawer);
	virtual void OnFramePaint(uiDrawer* pDrawer);
	virtual void OnFrameSize(UINT nNewWidth, UINT nNewHeight);
	virtual void OnSize(UINT nNewWidth, UINT nNewHeight);

	virtual BOOL IsDockableForm() const { return FALSE; }

	virtual void OnKBGetFocus() {}
	virtual void OnKBLoseFocus() {}


	INLINE uiFormBase* NextSibling()
	{
		if (m_pParent != nullptr && IS_VALID_ENTRY(m_ListChildrenEntry.next, m_pParent->m_ListChildren))
			return CONTAINING_RECORD(m_ListChildrenEntry.next, uiFormBase, m_ListChildrenEntry);
	}
	INLINE uiFormBase* PrevSibling()
	{
		if (m_pParent != nullptr && IS_VALID_ENTRY(m_ListChildrenEntry.prev, m_pParent->m_ListChildren))
			return CONTAINING_RECORD(m_ListChildrenEntry.prev, uiFormBase, m_ListChildrenEntry);
	}
	INLINE uiFormBase* GetFirstChild()
	{
		if (!LIST_IS_EMPTY(m_ListChildren))
			return CONTAINING_RECORD(m_ListChildren.next, uiFormBase, m_ListChildrenEntry);
		return nullptr;
	}

	INLINE void AddChild(uiFormBase *pChildForm)
	{
		ASSERT(pChildForm->m_pParent == nullptr);
		ASSERT(pChildForm->m_ListChildrenEntry.next == nullptr && pChildForm->m_ListChildrenEntry.prev == nullptr);
		pChildForm->m_pParent = this;
		list_add_tail(&pChildForm->m_ListChildrenEntry, &m_ListChildren);
	}
	INLINE void DetachChild(uiFormBase *pChildForm)
	{
		ASSERT(IsDirectChildForm(pChildForm));
		list_remove(&pChildForm->m_ListChildrenEntry);
		INIT_LIST_ENTRY(&pChildForm->m_ListChildrenEntry);
		pChildForm->m_pParent = nullptr;
	}

	INLINE void FrameToWindow(uiRect &rect)
	{
		rect = m_FrameRect;
		if (GetPlate() != nullptr)
			GetPlate()->ToWindowSpace(this, rect, FALSE);
	}
	INLINE void ClientToWindow(uiRect &rect)
	{
		rect = m_ClientRect;
		rect.Move(m_FrameRect.Left, m_FrameRect.Top);
		if (GetPlate() != nullptr)
			GetPlate()->ToWindowSpace(this, rect, FALSE);
	}
	INLINE void ClientToWindow(INT& x, INT& y)
	{
		x += (m_ClientRect.Left + m_FrameRect.Left);
		y += (m_ClientRect.Top + m_FrameRect.Top);
		if (GetPlate() != nullptr)
			GetPlate()->ToWindowSpace(this, x, y);
	}
	INLINE void ClientToScreen(INT &x, INT &y)
	{
		ClientToWindow(x, y);
		WndClientToScreen(GetBaseWnd(), x, y);
	}
	INLINE void WindowToClient(INT& x, INT& y)
	{
		INT cx = 0, cy = 0;
		ClientToWindow(cx, cy);
		x -= cx;
		y -= cy;
	}


	INLINE void ParentToFrameSpace(INT &x, INT &y) { x -= m_FrameRect.Left; y -= m_FrameRect.Top; }
	INLINE void FrameToParentSpace(INT &x, INT &y) { x += m_FrameRect.Left; y += m_FrameRect.Top; }
	INLINE void FrameToClientSpace(INT &x, INT &y) { x -= m_ClientRect.Left; y -= m_ClientRect.Top; }

	INLINE BOOL IsRootForm() const { return (m_pWnd != nullptr); }
	INLINE BOOL IsBaseForm() const { return (this == pAppBaseForm); }

	INLINE void SetWindow(uiWindow *pWnd) { ASSERT(m_pWnd == nullptr); m_pWnd = pWnd; }
	INLINE void SetAsBase() { ASSERT(pAppBaseForm == nullptr); pAppBaseForm = this; }

	INLINE BOOL HasChildrenForm() const { return IS_VALID_ENTRY(m_ListChildren.next, m_ListChildren); }
//	INLINE BOOL HasDockedForm() const { return (m_pFormSplitter != nullptr); }
	INLINE BOOL IsPointIn(INT x, INT y) const { return m_FrameRect.IsPointIn(x, y); } // Parent space.
	INLINE BOOL IsMouseHovering() const { return bMouseHover; }
	INLINE BOOL IsVisible() const { return bShow; }
	INLINE BOOL IsCreated() const { return bCreated; }
	INLINE BOOL IsCreating() const { return bCreating; }

	INLINE uiFormBase* GetParent() const { return m_pParent; }
	INLINE uiFormBase* GetPlate() const { return m_pPlate; }

	INLINE uiRect GetFrameRect() const { return uiRect(0, 0, m_FrameRect.Width(), m_FrameRect.Height()); }
	INLINE uiRect GetClientRect() const { return uiRect(0, 0, m_ClientRect.Width(), m_ClientRect.Height()); }

	INLINE const uiString& GetTitle() const { return m_strTitle; }
	INLINE void SetTitle(const TCHAR* pStrName) { m_strTitle = pStrName; }

	BOOL IsDirectChildForm(uiFormBase *pForm)
	{
		for (list_entry *pNext = LIST_GET_HEAD(m_ListChildren); IS_VALID_ENTRY(pNext, m_ListChildren); pNext = pNext->next)
		{
			uiFormBase *pSubForm = CONTAINING_RECORD(pNext, uiFormBase, m_ListChildrenEntry);
			if (pSubForm == pForm)
				return TRUE;
		}
		return FALSE;
	}

	BOOL CheckVisibility()
	{
		for (uiFormBase *pPlate = this; pPlate != nullptr; pPlate = pPlate->GetPlate())
			if (!pPlate->IsVisible())
				return FALSE;
		return TRUE;
	}


	enum CAP_TYPE
	{
		CT_SIZEABLE,
		CT_ALLOWSIDEDOCK,
		CT_ALLOWCLIENTDOCK,
	};

	BOOL GetCapability(CAP_TYPE ct)
	{
		switch (CT_SIZEABLE)
		{
	//	case CT_SIZEABLE:
	//		return bSizeable;
		case CT_ALLOWSIDEDOCK:
			return bAllowSideDock;
		case CT_ALLOWCLIENTDOCK:
			return bAllowClientDock;
		}
		ASSERT(0);
		return FALSE;
	}


protected:

	uiWindow* GetBaseWnd()
	{
		uiFormBase *pPlate = GetPlate();
		if (pPlate == nullptr)
			return m_pWnd;
		for (; pPlate->GetPlate() != nullptr; pPlate = pPlate->GetPlate());
		return pPlate->m_pWnd;
	}
	const uiRect& GetClientRectFS() const { return m_ClientRect; }


private:

	static uiFormBase* pAppBaseForm;

//	friend uiFormBase* FindFromSplitter(stFormSplitter *pSplitter, INT x, INT y);
//	friend uiFormBase* FindByPosImp(uiFormBase *pForm, INT x, INT y);
	friend void UpdateDockRect(stFormSplitter &splitter, FORM_DOCKING_FLAG df, UINT SizingBarWidth, uiRect &RectPlate, uiFormBase *pDocking, uiFormBase *pPlate);
	friend uiWindow* CreateTemplateWindow(UI_WINDOW_TYPE uwt, uiFormBase *pForm, uiFormBase *ParentForm, INT32 x, INT32 y, UINT32 nWidth, UINT32 nHeight, BOOL bVisible);
	friend LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	friend class uiForm;
	friend class uiWindow;


	void SetClientRect(uiRect& NewRect)
	{
		if (NewRect != m_ClientRect)
		{
			if (NewRect.Width() != m_ClientRect.Width() || NewRect.Height() != m_ClientRect.Height())
				OnSize(NewRect.Width(), NewRect.Height());
			m_ClientRect = NewRect;
		}
	}
	INLINE std::vector<UINT>& GetIDList() { return m_IDList; }

	INLINE void SetPostCreateList(void *pList) { ASSERT(m_pStyle == nullptr); m_pStyle = (uiFormStyle*)pList; }
	INLINE void CleanPostCreateList() { m_pStyle = nullptr; }
	INLINE void AddPostCreateEvent(uiFormBase* pFormBase) { ((UTX::CSimpleList*)m_pStyle)->push_front(pFormBase); }


	uiWindow    *m_pWnd;
	uiFormBase  *m_pPlate;
	uiFormBase  *m_pParent;
	uiFormStyle *m_pStyle;

	list_head  m_ListChildren;
	list_entry m_ListChildrenEntry;

	uiRect m_FrameRect, m_ClientRect;
	INT m_minWidth, m_minHeight;

	std::vector<UINT> m_IDList; // Don't access this member directly.

	uiString m_strTitle;

	BYTE m_DockFlag;

	bool m_bActivated = false;
	bool bCreating = false;
	bool bCreated = false;
	bool bShow = false;
	bool bMouseHover = false;
	bool bOwnCaret = false;
	bool m_bSideDocked = false;

	// Capabilities.
//	bool bSizeable = false;
	bool bNoBorder = false;
	bool bAllowSideDock = false;
	bool bAllowClientDock = false;


};


class uiControl : public uiFormBase
{
public:

	uiControl()
	:m_ID(uiID_INVALID)
	{}
	~uiControl()
	{
		ASSERT(m_ID == uiID_INVALID);
	}


	void OnCreate()
	{
		if (m_ID == uiID_INVALID)
			m_ID = uiGetID();
	}
	void OnDestroy()
	{
		if (m_ID != uiID_INVALID)
		{
			if (m_ID > uiID_SYSTEM)
				uiReleaseID(m_ID);
			m_ID = uiID_INVALID;
		}
	}

	virtual FORM_CLASS GetClass() { return FC_CONTROL; }

	BOOL Enable(BOOL bEnable)
	{
		if ((!m_bDisabled && !bEnable) || (m_bDisabled && bEnable))
		{
			m_bDisabled = !bEnable;
			RedrawForm();
			return TRUE;
		}
		return FALSE;
	}

	INLINE void GenerateMessage()
	{
		ASSERT(IsCreated());
		WndCreateMessage(GetBaseWnd(), this, m_ID);
	}
	INLINE UINT GetID()
	{
		ASSERT(IsCreated());
		return m_ID;
	}
	INLINE void SetID(UINT id)
	{
		if (m_ID != uiID_INVALID)
			uiReleaseID(m_ID);
		m_ID = id;
	}


protected:

	UINT m_ID;
	bool m_bDisabled = false;


};


class uiButton : public uiControl
{
public:

	uiButton()
	{
	}
	~uiButton() {}


	virtual FORM_CLASS GetClass() { return FC_BUTTON; }

	void OnMouseEnter(INT x, INT y)
	{
		printx("---> uiButton::OnMouseEnter x:%d y:%d\n", x, y);
		RedrawForm();
	}
	void OnMouseLeave()
	{
		printx("---> uiButton::OnMouseLeave\n");
		RedrawForm();
	}
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
	{
		//	printx("OnMouseMove client pos x:%d, y:%d. Screen pos x:%d, y:%d\n", x, y);
	}

	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		printx("---> uiButton::OnMouseBtnDown. Client pos x:%d, y:%d\n", x, y);
		if (KeyType == MKT_LEFT)
			m_bMouseDown = true;
	}
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		if (KeyType == MKT_LEFT && m_bMouseDown)
		{
			m_bMouseDown = false;
			BOOL bDone = FALSE;
			GenerateMessage();
		}
	}

	virtual void EntryOnCommand(UINT id)
	{
		GetParent()->EntryOnCommand(id);
	}

	void OnPaint(uiDrawer* pDrawer)
	{
		//	printx("---> uiButton::OnPaint\n");
		uiRect rect = GetClientRect();

		if (IsMouseHovering())
			pDrawer->FillRect(rect, RGB(230, 200, 230));
		else
			pDrawer->FillRect(rect, RGB(155, 200, 155));

		rect.Inflate(-1, -1, -1, -1);
		pDrawer->FillRect(rect, RGB(30, 50, 30));
	}

	void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		printx("button clicked!\n");
	}
	void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		printx("button double clicked!\n");
	}


protected:

	bool m_bMouseDown = false;


};


class uiDockableToolForm : public uiFormBase
{
public:

	uiDockableToolForm();
	virtual ~uiDockableToolForm();


};




class uiMenuBar : public uiFormBase
{
public:

	enum
	{
		DEFAULT_BAR_HEIGHT = 19,
	};

	uiMenuBar()
	{
		m_MouseHover = -1;
		m_Count = 0;
		m_RectArray = nullptr;
	}
	~uiMenuBar()
	{
		SAFE_DELETE_ARRAY(m_RectArray);
	}

	BOOL Create(uiFormBase* parent, uiMenu *pMenu);

	uiMenu* GetRootMenu();


	virtual void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(200, 200, 200));

		for (INT i = 0; i < m_Count; ++i)
		{
			if (m_MouseHover == i)
				pDrawer->FillRect(m_RectArray[i], RGB(210, 210, 210));
			else
				pDrawer->FillRect(m_RectArray[i], RGB(220, 220, 220));
		}
	}
	virtual void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
	{
	//	printx("---> uiMenuBar::OnMouseMove\n");
		if (m_Count == 0)
			return;

		INT oldMouseHover = m_MouseHover;
		m_MouseHover = -1;

		for (INT i = 0; i < m_Count; ++i)
		{
			if (!m_RectArray[i].IsPointIn(x, y))
				continue;

			m_MouseHover = i;
			RedrawForm(nullptr);
			//RedrawForm(&m_RectArray[i]);
			break;
		}

		if (oldMouseHover != -1 && m_MouseHover == -1)
			RedrawForm(nullptr);
	}

	virtual void OnMouseLeave()
	{
	//	printx("---> uiMenuBar::OnMouseLeave\n");
		if (m_MouseHover == -1)
			return;

		RedrawForm(&m_RectArray[m_MouseHover]);
		m_MouseHover = -1;
	}


protected:

	INT m_MouseHover, m_Count;
	uiRect *m_RectArray;


};


class uiHeaderForm : public uiFormBase
{
public:

	uiHeaderForm()
	{
	}
	~uiHeaderForm() {}


	void EntryOnCommand(UINT id);
	BOOL ShowButton(BOOL bShowMin, BOOL bShowMax, BOOL bShowClose);

	void OnMouseEnter(INT x, INT y);
	void OnMouseLeave();

	void OnCreate();
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd);
	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y);

	//void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	//{
	//	printx("---> uiHeaderForm::OnMouseBtnClk\n");

	//}
	//void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	//{
	//	printx("---> uiHeaderForm::OnMouseBtnDbClk\n");
	//}


	void OnPaint(uiDrawer* pDrawer);
	void OnSize(UINT nNewWidth, UINT nNewHeight);


protected:

	void UpdateLayout(INT NewWidth, INT NewHeight);

	//POINT m_pt;

	uiButton *m_pMinBtn = nullptr;
	uiButton *m_pMiddleBtn = nullptr;
	uiButton *m_pCloseBtn = nullptr;

};


class uiForm : public uiFormBase
{
public:

	enum
	{
		FRAME_WIDTH = 3,
	};


	uiForm()
	:m_minSize(-1, -1), m_maxSize(-1, -1)
	{
		m_pDockingPlate = nullptr;
		m_pFormSplitter = nullptr;
	}
	~uiForm() {}


	BOOL DockForm(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf);
	BOOL SideDock(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf);

	virtual UTX::CSimpleList* GetSideDockedFormList() { return &m_SideDockedFormList; }


	virtual void OnCreate();
//	virtual uiFormBase* FindByPos(INT x, INT y);
	virtual BOOL SetHeaderBar(const TCHAR* pStr, uiHeaderForm *pHeaderForm = nullptr);
	virtual BOOL SetMenuBar(uiMenu* pMenu);

	void OnFramePaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetFrameRect();
		DWORD color = uiGetSysColor(SCN_FRAME);
		pDrawer->FillRect(rect, color);
	}

	virtual INT  OnNCHitTest(INT x, INT y);
	virtual void OnPaint(uiDrawer* pDrawer);
	virtual void OnFrameSize(UINT nNewWidth, UINT nNewHeight);
	virtual void OnSize(UINT nNewWidth, UINT nNewHeight);


	virtual uiSize GetMinSize() { return m_minSize; }
	virtual uiSize GetMaxSize() { return m_maxSize; }

	void UpdataClientRect();


protected:

	uiFormBase *m_pDockingPlate;
	stFormSplitter *m_pFormSplitter;

	UTX::CSimpleList m_SideDockedFormList;

	uiSize m_minSize, m_maxSize;


};


class uiFrame : public uiFormBase
{
public:

	uiFrame()
	{
	}
	~uiFrame() {}

	void OnCreate();
	void OnFramePaint(uiDrawer* pDrawer);
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd);
	void OnPaint(uiDrawer* pDrawer);


protected:

	uiRect m_Header;


};


class uiButton2 : public uiButton
{
public:

	uiButton2()
	{
	}
	~uiButton2()
	{
	}

	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
	{
		INT sx = x, sy = y;
		ClientToScreen(sx, sy);
		printx("OnMouseMove client pos x:%d, y:%d. Screen pos x:%d, y:%d\n", x, y, sx, sy);

		POINT p;
		if (GetCursorPos(&p))
			printx("Real screen pos - x:%d, y:%d\n", p.x, p.y);

	//	RedrawForm();
	}

	void OnMouseEnter(INT x, INT y)
	{
	//	printx("---> uiButton2::OnMouseEnter\n");
		RedrawForm();
	}
	void OnMouseLeave()
	{
	//	printx("---> uiButton2::OnMouseLeave\n");
		RedrawForm();
	}

	void OnPaint(uiDrawer* pDrawer)
	{
	//	printx("---> uiButton2::OnPaint\n");
		uiRect rect = GetClientRect();
		const INT size = 3;

		if (IsMouseHovering())
			pDrawer->RoundRect(rect, RGB(230, 200, 230), size, size);
		else
			pDrawer->RoundRect(rect, RGB(155, 200, 155), size, size);

		rect.Inflate(-1, -1, -1, -1);
	//	rect.Inflate(10, 10);
		pDrawer->FillRect(rect, RGB(30, 50, 30));
	}


protected:

	INT id;


};


class uiDockableForm : public uiForm
{
public:

	uiDockableForm();
	~uiDockableForm();

	BOOL OnDeplate(INT iReason, uiFormBase *pDockingForm);


protected:

	list_head m_ListDockingForm;


};


class uiTabForm : public uiForm
{
public:

	enum TAB_FORM_FLAGS
	{
		TFF_NONE           = 0x00,

		TFF_TAB_TOP        = 0x01,
		TFF_TAB_BOTTOM     = 0x01 << 1,
		TFF_TAB_LEFT       = 0x01 << 2,
		TFF_TAB_RIGHT      = 0x01 << 3,
		TFF_FULL_FORM      = 0x01 << 4,
		TFF_FORCE_SHOW_TAB = 0x01 << 5, // Show tab even if this form has only one pane.
		TFF_DRAGGABLE_TAB  = 0x01 << 6,
	};

	enum
	{
		DEFAULT_TAB_HEIGHT = 23, // For buttons in vertical position.
	};

	struct stPaneInfo
	{
		stPaneInfo()
		:pForm(nullptr), bTabForm(false)
		{
		}

		uiFormBase *pForm;
		uiRect Rect;       // visible rect only
		uiSize FullRect;   // size needed for showing complete text
		bool bTabForm;     // Form was created automatically.
	};

	uiTabForm::uiTabForm()
	:m_TotalPane(0), m_LeftMargin(1), m_TopMargin(1), m_RightMargin(1), m_BottomMargin(1)
	{
		m_ActiveIndex = -1;
		m_HighlightIndex = -1;

		m_ColorActive = RGB(255, 255, 255);
		m_ColorHover = RGB(230, 230, 230);
		m_ColorDefault = RGB(200, 200, 220);
	}
	virtual uiTabForm::~uiTabForm()
	{
	}


	BOOL Create(uiFormBase *pParent, TAB_FORM_FLAGS tff);
	BOOL AddPane(uiFormBase *pForm, INT index, BOOL bActivate);

	BOOL DeletePane(INT index);
	BOOL DetachPane(INT index, uiForm *pForm = nullptr);

	BOOL ActivateTab(INT index);

	void Layout();
	void UpdateTabsRect();
	void RedrawTabs(INT index1, INT index2 = -1);

	void OnMouseEnter(INT x, INT y)
	{
		printx("---> uiTabForm::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> uiTabForm::OnMouseLeave\n");

		if (m_HighlightIndex != -1)
		{
			if (m_HighlightIndex != m_ActiveIndex)
				RedrawTabs(m_HighlightIndex);
			m_HighlightIndex = -1;
		}
	}
	void OnMouseFocusLost()
	{
		printx("---> uiTabForm::OnMouseFocusLost\n");
		m_bDraggingTab = false;
	}

	void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		printx("---> uiTabForm::OnMouseBtnClk x: %d, y: %d\n", x, y);
	}

	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	void GetBufferRect(const uiRect &OldRect, const uiRect &NewRect, uiRect &out);
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd);
	void OnSize(UINT nNewWidth, UINT nNewHeight);
	void OnPaint(uiDrawer* pDrawer);


	INLINE BOOL TestFlag(TAB_FORM_FLAGS flag) { return m_Flag & flag; }
	INLINE void SetMargin(INT left, INT top = -1, INT right = -1, INT bottom = -1)
	{
		if (left != -1) m_LeftMargin = left;
		if (top != -1) m_TopMargin = top;
		if (right != -1) m_RightMargin = right;
		if (bottom != -1) m_BottomMargin = bottom;
	}
	uiSize GetFullTabsSize()
	{
		uiSize out;
		for (INT i = 0; i < m_TotalPane; ++i)
		{
			out.iWidth += GetPaneInfo(i)->FullRect.iWidth;
			out.iHeight += GetPaneInfo(i)->FullRect.iHeight;
		}
		return out;
	}


protected:

	INT m_ActiveIndex, m_HighlightIndex, m_TotalPane;
	INT m_LeftMargin, m_TopMargin, m_RightMargin, m_BottomMargin;
	UINT m_Flag;

	UINT m_ColorActive, m_ColorHover, m_ColorDefault;
	UINT16 m_TabWidth, m_TabHeight;

	std::vector<stPaneInfo> m_PaneInfoArray;

	uiRect m_TabsRegion, m_PaneRegion, m_OldTabRect;

	bool m_bDraggingTab = false;


private:

	INT AddPaneInfo(stPaneInfo *pNewPaneInfo, INT index = -1)
	{
		if (index == -1)
			index = m_PaneInfoArray.size();
		m_PaneInfoArray.insert(m_PaneInfoArray.begin() + index, 1, *pNewPaneInfo);
		m_TotalPane = m_PaneInfoArray.size();
		return index;
	}
	stPaneInfo* GetPaneInfo(INT index)
	{
		ASSERT(0 <= index && index < m_TotalPane);
		return &m_PaneInfoArray[index];
	}
	BOOL ReleasePaneInfo(INT index)
	{
		ASSERT(0 <= index && index < m_TotalPane);
		if (0 > index || index > m_TotalPane)
			return FALSE;

		m_PaneInfoArray.erase(m_PaneInfoArray.begin() + index);
		--m_TotalPane;

		return TRUE;
	}


};


IMPLEMENT_ENUM_FLAG(uiTabForm::TAB_FORM_FLAGS)


