

#pragma once


#include "uiDrawer.h"
#include "uiCommon.h"
#include "uiKeyCode.h"


class uiMenu;
class uiButton;
class uiFormBase;
class uiHeaderFormBase;
class uiWindow;
struct uiFormStyle;

class IAreaCursor;


BOOL WndClientToScreen(uiWindow *pWnd, INT &x, INT &y); // Declare this for inline function.
BOOL WndCreateMessage(uiWindow *pWnd, uiFormBase *pSrc, UINT_PTR id);


enum MOUSE_ACTIVATE_RESULT
{
	MAR_ACTIVATE,
	MAR_ACTIVATE_EAT,
	MAR_NO_ACTIVATE,
	MAR_NO_ACTIVATE_EAT,

	MAR_TOTAL,
};

enum FORM_SHOW_MODE
{
	FSM_HIDE,
	FSM_SHOW,
	FSM_NOACTIVATE,

	// For frame form only.
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

	FCF_NO_AT       = 0x01 << 5, // Active trail: used to track frame state for painting non-client area.
	FCF_NO_ACTIVATE = 0x01 << 6,
	FCF_ACCEPTFILES = 0x01 << 7,
	FCF_NO_DBCLICK  = 0x01 << 8,

	FCF_NO_HEADER = 0x01 << 24,
};

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

enum FORM_CLASS
{
	FC_BASE,

	FC_DOCKSITE,

	FC_TOOLBAR,
	FC_FORM,
	FC_HEADER_BAR,

	FC_DROPDOWN_FORM,

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

enum FORM_PAINT_MODE
{
	FPM_NORMAL  = 0x01,
	FPM_NCPAINT = 0x01 << 1,
	FPM_ACTIVE  = 0x01 << 2,
};


IMPLEMENT_ENUM_FLAG(FORM_CREATION_FLAG)
IMPLEMENT_ENUM_FLAG(MOVE_DIRECTION)
IMPLEMENT_ENUM_FLAG(FORM_DOCKING_FLAG)
IMPLEMENT_ENUM_FLAG(FORM_PAINT_MODE)


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


struct stKeyEventInfo
{
	VUI_KEY_CODE KeyCode;
	INT  iRepeatCount;
	bool bKeyDownPrev;
};


class uiFormBase
{
public:

	enum CLIENT_AREA_TYPE
	{
		CAT_CLIENT = 0, // Don't change this.

		// draggable region
		CAT_TOP    = 0x01,
		CAT_BOTTOM = 0x01 << 1,
		CAT_LEFT   = 0x01 << 2,
		CAT_RIGHT  = 0x01 << 3,

		CAT_DRAG_BAR_H = 0x01 << 4,
		CAT_DRAG_BAR_V = 0x01 << 5,

		// border
		CAT_B_TOP    = 0x01 << 6,
		CAT_B_BOTTOM = 0x01 << 7,
		CAT_B_LEFT   = 0x01 << 8,
		CAT_B_RIGHT  = 0x01 << 9,

		CAT_ALL_DRF = CAT_TOP | CAT_BOTTOM | CAT_LEFT | CAT_RIGHT,
	};

	enum FORM_BASE_FLAGS
	{
		FBF_NONE,

		FBF_ACTIVATED       = 0x01,
		FBF_CREATING        = 0x01 << 1,
		FBF_CREATED         = 0x01 << 2,
		FBF_IS_DESTROYING   = 0x01 << 3,
		FBF_SHOW            = 0x01 << 4,
		FBF_MINIMIZED       = 0x01 << 5,
		FBF_MAXIMIZED       = 0x01 << 6,
		FBF_MOUSE_HOVER     = 0x01 << 7,
		FBF_OWN_CARET       = 0x01 << 8,
		FBF_SIDE_DOCKED     = 0x01 << 9,
		FBF_MODAL_MODE      = 0x01 << 10,
		FBF_ACTIVE_TRAIL    = 0x01 << 11,
		FBF_NO_ACTIVE_TRAIL = 0x01 << 12,
		FBF_KB_FOCUS        = 0x01 << 13,
		FBF_MOUSE_FOCUS     = 0x01 << 14,
	};

	enum ACTIVATE_STATE
	{
		AS_ACTIVE,
		AS_INACTIVE,
		AS_NOTIFY,
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

	struct stTimerInfo
	{
		const UINT id;
		const UINT msElapsedTime;

		// Changeable for message handler.
		void* pCtx;
		INT   nRunCount;
	};

	typedef void(*TimerCallback)(uiFormBase* pForm, stTimerInfo* ti);


	uiFormBase();
	virtual ~uiFormBase();

//	BOOL Create(uiFormBase* parent, const RECT& rect, FORM_CREATION_FLAG cf);
	BOOL Create(uiFormBase* parent, INT x, INT y, UINT nWidth, UINT nHeight, FORM_CREATION_FLAG fcf = FCF_NONE);
	INT_PTR ModalDialog(uiFormBase* pParent, INT x, INT y, UINT nWidth, UINT nHeight, FORM_CREATION_FLAG fcf = FCF_CENTER);

	void Bind(UINT CmdID);
	void Close();
	BOOL CloseDialog(INT_PTR iResult);
	void DePlate();
	void Move(INT x, INT y);
	void MoveToCenter();
	void MoveByOffset(const INT OffsetX, const INT OffsetY, BOOL bForceRedraw = FALSE);
	void Show(FORM_SHOW_MODE sm);
	BOOL Size(INT NewWidth, INT NewHeight);
	UINT TimerStart(UINT id, UINT msElapsedTime, INT nRundCount, void* pCtx); // Return timer handle.
	UINT TimerStart(TimerCallback tcb, UINT msElapsedTime, INT nRundCount, void* pCtx); // Return timer handle.
	BOOL TimerStop(UINT key, BOOL bByID = TRUE);

	void RedrawForm(const uiRect* pUpdateRect = nullptr);
	void RedrawFrame(const uiRect* pUpdateRect = nullptr);

	uiFormBase* SetActive();

	uiFormBase* SetCapture();
	BOOL ReleaseCapture();

	uiFormBase* SetFocus();

	BOOL CaretShow(BOOL bShow, INT x = -1, INT y = -1, INT width = -1, INT height = -1);
	BOOL CaretMoveByOffset(INT OffsetX, INT OffsetY);
	BOOL CaretMove(INT x, INT y);

	void PopupMenu(INT x, INT y, uiMenu* pMenu);

	virtual FORM_CLASS GetClass() const { return FC_BASE; }

	virtual INT FindByPos(uiFormBase** pDest, INT fcX, INT fcY, uiPoint* ptCS); // fcX and fcY are in frame space.
	virtual CLIENT_AREA_TYPE GetAreaType(INT csX, INT csY) { return CAT_CLIENT; }

	virtual void ToPlateSpace(const uiFormBase* pForm, INT& x, INT& y) const;
	virtual void ToPlateSpace(const uiFormBase* pForm, uiRect& rect, BOOL bClip) const;
	virtual uiPoint FrameToClientSpace(uiPoint& pt) const { return pt; }
	virtual uiRect GetClientRectFS() const { return uiRect(m_FrameRect.Width(), m_FrameRect.Height()); }
	virtual uiRect GetClientRect() const { return uiRect(m_FrameRect.Width(), m_FrameRect.Height()); }

	void StartDragging(MOUSE_KEY_TYPE mkt, INT wcX, INT wcY);
	uiPoint GetCursorPos() const;

	void EntryOnUpdateAT(ACTIVATE_STATE as, uiFormBase* pSubNode);
	void EntryOnCreate(FORM_CREATION_FLAG fcf, UINT nWidth, UINT nHeight);
	BOOL EntryOnClose();
	void EntryOnDestroy(uiWindow* pWnd);
	void EntryOnMove(INT x, INT y, const stFormMoveInfo* pInfo);
	void EntryOnSize(UINT nNewWidth, UINT nNewHeight);

	void EntryOnKBGetFocus(uiFormBase* pPrevForm);
	void EntryOnKBKillFocus(uiFormBase* pNewForm);

	virtual void EntryOnCommand(UINT_PTR id);
	virtual void EntryOnPaint(uiDrawer* pDrawer);

	virtual uiSize GetMinSize() const { return uiSize(-1, -1); }
	virtual uiSize GetMaxSize() const { return uiSize(-1, -1); }

	virtual BOOL OnClose() { return TRUE; }
	virtual BOOL OnCreate(FORM_CREATION_FLAG fcf) { return TRUE; }
	virtual void OnDestroy() {}

	virtual uiFormBase* TryActivate();
	virtual void OnActivate(BOOL bActive);
	virtual void OnUpdateAT(ACTIVATE_STATE as, uiFormBase* pSubNode);

	virtual MOUSE_ACTIVATE_RESULT OnMouseActivate(); // TODO

	virtual void OnCommand(INT_PTR id, BOOL &bDone);

	virtual BOOL OnDeplate(INT iReason, uiFormBase *pDockingForm);
	virtual void OnSubnodeDestroy(uiFormBase* pSubnode) {}
	virtual void OnChildDestroy(uiFormBase* pSubnode) {}

	virtual void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y);
	virtual void OnMouseEnter(INT x, INT y);
	virtual void OnMouseLeave();
	virtual void OnMouseFocusLost(uiFormBase* pNewForm);
	virtual void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd);
	virtual void OnMove(INT x, INT y, const stFormMoveInfo* pInfo);
	virtual void OnPaint(uiDrawer* pDrawer);
	virtual void OnPreCreate(FORM_CREATION_FLAG& fcf);
	virtual void OnFrameSize(UINT nNewWidth, UINT nNewHeight);
	virtual void OnSize(UINT nNewWidth, UINT nNewHeight);
	virtual void OnTimer(stTimerInfo* ti);

	virtual void OnKBFocus(BOOL bGet, uiFormBase* pForm) {}

	virtual void OnKeyDown(const stKeyEventInfo* pKEI) {}
	virtual void OnKeyUp(const stKeyEventInfo* pKEI) {}
	virtual void OnSysKeyDown(const stKeyEventInfo* pKEI) {}
	virtual void OnSysKeyUp(const stKeyEventInfo* pKEI) {}


	struct stFindCtx
	{
		stFindCtx() { pos = nullptr; }
		list_entry* pos;
	};

	uiFormBase* FindChildByClass(FORM_CLASS fc, stFindCtx& ctx)
	{
		ctx.pos = (ctx.pos == nullptr) ? m_ListChildren.next : ctx.pos->next;

		for (; IS_VALID_ENTRY(ctx.pos, m_ListChildren); ctx.pos = ctx.pos->next)
		{
			uiFormBase *pForm = CONTAINING_RECORD(ctx.pos, uiFormBase, m_ListChildrenEntry);
			if (pForm->GetClass() == fc)
				return pForm;
		}
		return nullptr;
	}


	INLINE uiFormBase* NextSibling()
	{
		if (m_pParent != nullptr && IS_VALID_ENTRY(m_ListChildrenEntry.next, m_pParent->m_ListChildren))
			return CONTAINING_RECORD(m_ListChildrenEntry.next, uiFormBase, m_ListChildrenEntry);
		return nullptr;
	}
	INLINE uiFormBase* PrevSibling()
	{
		if (m_pParent != nullptr && IS_VALID_ENTRY(m_ListChildrenEntry.prev, m_pParent->m_ListChildren))
			return CONTAINING_RECORD(m_ListChildrenEntry.prev, uiFormBase, m_ListChildrenEntry);
	}
	//INLINE uiFormBase* GetFirstChild()
	//{
	//	if (!LIST_IS_EMPTY(m_ListChildren))
	//		return CONTAINING_RECORD(m_ListChildren.next, uiFormBase, m_ListChildrenEntry);
	//	return nullptr;
	//}

	INLINE void AddChild(uiFormBase *pChildForm)
	{
		ASSERT(GetClass() != FC_DOCKSITE);
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

	INLINE void ToWindowSpace(uiRect &rect, BOOL bClip) const // rect is in plate space.
	{
		for (const uiFormBase *pPlate = GetPlate(), *pForm = this; pPlate != nullptr; pForm = pPlate, pPlate = pPlate->GetPlate())
			pPlate->ToPlateSpace(pForm, rect, bClip);
	}
	INLINE void ToWindowSpace(INT &x, INT &y) const // x and y are in plate space.
	{
		for (const uiFormBase *pPlate = GetPlate(), *pForm = this; pPlate != nullptr; pForm = pPlate, pPlate = pPlate->GetPlate())
			pPlate->ToPlateSpace(pForm, x, y);
	}
	INLINE void GetFrameRectWS(uiRect &rect) const
	{
		rect = m_FrameRect;
		ToWindowSpace(rect, FALSE);
	}
	INLINE void GetClientRectWS(uiRect &rect, BOOL bClip) const
	{
		rect = GetClientRectFS();
		rect.Move(m_FrameRect.Left, m_FrameRect.Top);
		ToWindowSpace(rect, bClip);
	}
	INLINE uiPoint ClientToWindow(uiPoint& ptIn) const
	{
		ptIn = ClientToFrameSpace(ptIn) + m_FrameRect.GetLeftTop();
		ToWindowSpace(ptIn.x, ptIn.y);
		return ptIn;
	}
	INLINE uiPoint WindowToClient(uiPoint& pt) const
	{
		return pt -= ClientToWindow(uiPoint());
	}
	INLINE void ClientToScreen(uiPoint &ptCS) const
	{
		ClientToWindow(ptCS);
		WndClientToScreen(GetBaseWnd(), ptCS.x, ptCS.y);
	}
	INLINE uiPoint ClientToFrameSpace(uiPoint& pt) const { return pt -= FrameToClientSpace(uiPoint()); }


	INLINE void SetWindow(uiWindow *pWnd) { ASSERT(m_pWnd == nullptr); m_pWnd = pWnd; }
	INLINE BOOL IsRootForm() const { return (m_pWnd != nullptr); }

	INLINE BOOL HasChildrenForm() const { return IS_VALID_ENTRY(m_ListChildren.next, m_ListChildren); }
//	INLINE BOOL HasDockedForm() const { return (m_pFormSplitter != nullptr); }
	INLINE BOOL IsPointIn(INT x, INT y) const { return m_FrameRect.IsPointIn(x, y); } // Parent space.

	INLINE BOOL IsMouseHovering() const { return FBTestFlag(FBF_MOUSE_HOVER); }
	INLINE BOOL IsVisible() const { return FBTestFlag(FBF_SHOW); }
	INLINE BOOL IsCreated() const { return FBTestFlag(FBF_CREATED); }
	INLINE BOOL IsCreating() const { return FBTestFlag(FBF_CREATING); }
	INLINE BOOL IsSideDocked() const { return FBTestFlag(FBF_SIDE_DOCKED); }
	INLINE BOOL IsDestroying() const { return FBTestFlag(FBF_IS_DESTROYING); }

	INLINE uiFormBase* GetParent() const { return m_pParent; }
	INLINE uiFormBase* GetPlate() const { return m_pPlate; }

	INLINE uiRect GetFrameRect() const { return uiRect(m_FrameRect.Width(), m_FrameRect.Height()); }

	INLINE const uiString& GetName() const { return m_strName; }
	INLINE void SetName(const TCHAR* pStrName) { m_strName = pStrName; }

	BOOL IsDirectChildForm(const uiFormBase* pForm) const
	{
		for (list_entry *pNext = LIST_GET_HEAD(m_ListChildren); IS_VALID_ENTRY(pNext, m_ListChildren); pNext = pNext->next)
		{
			uiFormBase* pSubForm = CONTAINING_RECORD(pNext, uiFormBase, m_ListChildrenEntry);
			if (pSubForm == pForm)
				return TRUE;
		}
		return FALSE;
	}
	BOOL IsSubnode(const uiFormBase* pUpperNode) const
	{
		for (const uiFormBase* pSubForm = this; pSubForm != nullptr; pSubForm = pSubForm->GetPlate())
			if (pSubForm == pUpperNode)
				return TRUE;
		return FALSE;
	}

	BOOL CheckVisibility()
	{
		for (uiFormBase *pPlate = this; pPlate != nullptr; pPlate = pPlate->GetPlate())
			if (!pPlate->IsVisible())
				return FALSE;
		return TRUE;
	}

	typedef void(*OnFind)(uiFormBase* pDest, void* ctx);
	static UINT EnumChildByClass(FORM_CLASS fc, uiFormBase* pForm, OnFind Callback, void* CBCtx);

	static uiFormBase* FindTheParent(uiFormBase* pForm1, uiFormBase* pForm2);

	DECLARE_INTERFACE(IAreaCursor)


protected:

	uiWindow* GetBaseWnd() const
	{
		const uiFormBase* pPlate = this;
		for (; pPlate->GetPlate() != nullptr; pPlate = pPlate->GetPlate());
		return pPlate->m_pWnd;
	}

	INLINE INT GetDepth() const
	{
		INT i = 0;
		for (const uiFormBase* pPlate = this; pPlate->GetPlate() != nullptr; ++i, pPlate = pPlate->GetPlate());
		return i;
	}
	INLINE BOOL FBTestFlag(FORM_BASE_FLAGS flag) const;
	INLINE void FBSetFlag(FORM_BASE_FLAGS flag);
	INLINE void FBClearFlag(FORM_BASE_FLAGS flag);


private:

//	friend uiFormBase* FindFromSplitter(stFormSplitter *pSplitter, INT x, INT y);
//	friend uiFormBase* FindByPosImp(uiFormBase *pForm, INT x, INT y);
	friend void UpdateDockRect(stFormSplitter &splitter, FORM_DOCKING_FLAG df, UINT SizingBarWidth, uiRect &RectPlate, uiFormBase *pDocking, uiFormBase *pPlate);

	friend class uiForm;
	friend class uiHeaderForm;
	friend class IFrameImp;
	friend class uiSideDockableFrame;
	friend class uiWindow;

	INLINE uiWindow* GetWnd() const { return m_pWnd; }

	INLINE std::vector<UINT>& GetIDList() { return m_IDList; }
	INLINE UINT GetTimerCount() const { return m_TimerCount; }
	INLINE void SetTimerCount(BOOL bInc) { m_TimerCount = (bInc) ? m_TimerCount + 1 : m_TimerCount - 1; } // Don't use ++lvalue here.

	INLINE void SetPostCreateList(void *pList) { ASSERT(m_pStyle == nullptr); m_pStyle = (uiFormStyle*)pList; }
	INLINE void CleanPostCreateList() { m_pStyle = nullptr; }
	INLINE void AddPostCreateEvent(uiFormBase* pFormBase) { ((UTX::CSimpleList*)m_pStyle)->push_front(pFormBase); }


	uiWindow    *m_pWnd;
	uiFormBase  *m_pPlate;
	uiFormBase  *m_pParent;
	uiFormStyle *m_pStyle;

	list_head  m_ListChildren;
	list_entry m_ListChildrenEntry;

	uiRect m_FrameRect;

	std::vector<UINT> m_IDList; // Don't access this member directly.

	uiString m_strName;

	FORM_BASE_FLAGS m_Flag;

	BYTE m_DockFlag;
	BYTE m_TimerCount; // It's safe to change type of the data directly.


};


IMPLEMENT_ENUM_FLAG(uiFormBase::CLIENT_AREA_TYPE)
IMPLEMENT_ENUM_FLAG(uiFormBase::FORM_BASE_FLAGS)

INLINE BOOL uiFormBase::FBTestFlag(uiFormBase::FORM_BASE_FLAGS flag) const { return m_Flag & flag; }
INLINE void uiFormBase::FBSetFlag(uiFormBase::FORM_BASE_FLAGS flag) { m_Flag |= flag; }
INLINE void uiFormBase::FBClearFlag(uiFormBase::FORM_BASE_FLAGS flag) { m_Flag &= ~flag; }


class UI_INTERFACE IAreaCursor
{
public:

	virtual uiImage GetCursorImage(uiFormBase* pForm, INT csX, INT csY, uiFormBase::CLIENT_AREA_TYPE) = 0;

};


class IFrameImp
{
public:

	IFrameImp() :m_pActive(nullptr), m_pHeaderForm(nullptr) {}

	void OnNCPaintImp(uiFormBase* pForm, uiDrawer* pDrawer, FORM_PAINT_MODE mode);
	BOOL SetTitleImp(uiFormBase* pForm, const uiString& str);
	BOOL SetIconImp(uiFormBase* pForm, uiImage& img, BOOL bBig);

	INLINE uiImage& GetIcon(BOOL bSmall = TRUE) { return bSmall ? m_IconS : m_IconB; }
	INLINE void SetHeader(uiHeaderFormBase* pHBase) { m_pHeaderForm = pHBase; }
	INLINE uiHeaderFormBase* GetHeader() const { return m_pHeaderForm; }

protected:

	uiFormBase* m_pActive;
	uiHeaderFormBase* m_pHeaderForm;
	uiImage m_IconS, m_IconB;


};


class uiSideDockableFrame : virtual public uiFormBase, public IFrameImp
{
public:

	enum
	{
		DEFAULT_BORDER_THICKNESS = 3,
		DEFAULT_HEADER_BAR_HEIGHT = 26,
		MIN_HEADER_BAR_WIDTH = 150,
	};

	enum FORM_BORDER_FLAGS
	{
		FBF_NONE   = 0,

		// sizeable border side.
		FBF_TOP    = 0x01,
		FBF_BOTTOM = 0x01 << 1,
		FBF_LEFT   = 0x01 << 2,
		FBF_RIGHT  = 0x01 << 3,

		FBF_ALL = FBF_TOP | FBF_BOTTOM | FBF_LEFT | FBF_RIGHT,
	};


	uiSideDockableFrame()
	:m_BorderFlags(FBF_ALL) // FBF_NONE FBF_ALL
	{
		m_BTLeft = m_BTTop = m_BTRight = m_BTBottom = DEFAULT_BORDER_THICKNESS;
		m_DTLeft = m_DTTop = m_DTRight = m_DTBottom = DEFAULT_BORDER_THICKNESS;
	}

	void OnPreCreate(FORM_CREATION_FLAG& fcf) override;
	void OnUpdateAT(ACTIVATE_STATE as, uiFormBase* pSubNode) override;
	void EntryOnPaint(uiDrawer* pDrawer) override;

	INT FindByPos(uiFormBase **pDest, INT fcX, INT fcY, uiPoint *ptCS) override;

	void ToPlateSpace(const uiFormBase *pForm, INT& x, INT& y) const override;
	void ToPlateSpace(const uiFormBase *pForm, uiRect& rect, BOOL bClip) const override;
	uiPoint FrameToClientSpace(uiPoint& pt) const override;
	uiRect GetClientRectFS() const override;
	uiRect GetClientRect() const override;

	void OnFrameSize(UINT nNewWidth, UINT nNewHeight) override;

	virtual void OnFramePaint(uiDrawer* pDrawer);
	virtual void OnHeaderBarChanged(uiHeaderFormBase* pNewHeaderForm);

	BOOL DockForm(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf);
	BOOL SideDock(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf);

	BOOL SetHeaderBar(uiHeaderFormBase* pHeaderForm);
	BOOL SetMenuBar(uiMenu* pMenu);


	INLINE void SetBorder(BYTE Thickness, FORM_BORDER_FLAGS flags = FBF_ALL) { SetBorder(Thickness, Thickness, Thickness, Thickness, flags); }
	void SetBorder(INT left, INT top, INT right, INT bottom, FORM_BORDER_FLAGS flags = FBF_ALL);
	void SetDraggableThickness(BYTE left, BYTE top, BYTE right, BYTE bottom);

	INLINE void SetIcon(uiImage img, BOOL bBig = FALSE) { SetIconImp(this, img, bBig); }
	INLINE void SetTitle(const uiString& str) { m_strName = str; SetTitleImp(this, str); }


	uiFormBase* FindFirstChild()
	{
		for (list_entry* entry = LIST_GET_HEAD(m_ListChildren); IS_VALID_ENTRY(entry, m_ListChildren); entry = entry->next)
		{
			uiFormBase *pForm = CONTAINING_RECORD(entry, uiFormBase, m_ListChildrenEntry);
			if (pForm->IsSideDocked())
				continue;
			return pForm;
		}
		return nullptr;
	}
	uiFormBase* TryActivate() override
	{
		if (FBTestFlag(FBF_ACTIVATED))
			return nullptr;
		if (m_pActive != nullptr)
			return m_pActive->TryActivate();
		else
		{
			if ((m_pActive = FindFirstChild()) != nullptr)
				return m_pActive;
		}
		return this;
	}
	void OnSubnodeDestroy(uiFormBase* pSubnode)
	{
		if (m_pActive == pSubnode)
		{
		//	uiFormBase* pNextActive = pSubnode->Next; // TODO
			m_pActive = pSubnode->NextSibling();
		}
	}


protected:

	void SetClientRect(const uiRect& NewRect);
	void UpdataClientRect(); // This won't call any redraw methods.


	uiRect m_ClientRect;
	UTX::CSimpleList m_SideDockedFormList;

	FORM_BORDER_FLAGS m_BorderFlags;
	BYTE m_BTLeft, m_BTTop, m_BTRight, m_BTBottom; // Border thickness for drawing.
	BYTE m_DTLeft, m_DTTop, m_DTRight, m_DTBottom; // Draggable thickness, is always thicker than or equal to visible border.


};


IMPLEMENT_ENUM_FLAG(uiSideDockableFrame::FORM_BORDER_FLAGS)


class IMessageHandler : virtual public uiFormBase
{
public:

	void EntryOnCommand(UINT_PTR id) override
	{
	}



};


class uiMenuBar : virtual public uiFormBase
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


	void OnPaint(uiDrawer* pDrawer) override
	{
		uiRect rect = GetFrameRect();
		pDrawer->FillRect(rect, RGB(200, 200, 200));

		for (INT i = 0; i < m_Count; ++i)
		{
			if (m_MouseHover == i)
				pDrawer->FillRect(m_RectArray[i], RGB(210, 210, 210));
			else
				pDrawer->FillRect(m_RectArray[i], RGB(220, 220, 220));
		}
	}
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd) override
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


class uiHeaderFormBase : public uiFormBase
{
public:


	FORM_CLASS GetClass() const override final { return FC_HEADER_BAR; }


	void OnPaint(uiDrawer* pDrawer) override final;


	virtual void PaintBG(uiDrawer* pDrawer, const uiRect& rect) = 0;
	virtual void Paint(uiDrawer* pDrawer, const uiRect& rect) = 0;

	virtual void OnTitleChanged(const uiString& str) = 0;
	virtual void OnIconChanged(const uiImage& img, BOOL bBig) = 0;


};


class uiHeaderForm : public uiHeaderFormBase
{
public:

	uiHeaderForm() {}
	virtual ~uiHeaderForm() {}


	void EntryOnCommand(UINT_PTR id) override;
	BOOL ShowButton(BOOL bShowMin, BOOL bShowMax, BOOL bShowClose);

	void OnMouseEnter(INT x, INT y) override;
	void OnMouseLeave() override;

	BOOL OnCreate(FORM_CREATION_FLAG fcf) override;
	void OnPreCreate(FORM_CREATION_FLAG& fcf) override;
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd) override;
	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override;
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y) override;

	//void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	//{
	//	printx("---> uiHeaderForm::OnMouseBtnClk\n");
	//}
	//void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	//{
	//	printx("---> uiHeaderForm::OnMouseBtnDbClk\n");
	//}

	void PaintBG(uiDrawer* pDrawer, const uiRect& rect) override;
	void Paint(uiDrawer* pDrawer, const uiRect& rect) override;
	void OnSize(UINT nNewWidth, UINT nNewHeight) override;

	// framework notification.
	void OnTitleChanged(const uiString& str) override { printx("Title changed!\n"); }
	void OnIconChanged(const uiImage& img, BOOL bBig) override { printx("Icon changed!\n"); }


protected:

	void UpdateLayout(INT NewWidth, INT NewHeight);

	uiButton *m_pMinBtn = nullptr;
	uiButton *m_pMiddleBtn = nullptr;
	uiButton *m_pCloseBtn = nullptr;

	uiRect m_rectIcon, m_rectTitle;

	uiImage m_temp;


};


class uiForm : public uiSideDockableFrame
{
public:

	uiForm()
	:m_minSize(-1, -1), m_maxSize(-1, -1)
	{
		m_pFormSplitter = nullptr;
	}
	~uiForm() {}


	uiSize GetMinSize() const override { return m_minSize; }
	uiSize GetMaxSize() const override { return m_maxSize; }

	void OnHeaderBarChanged(uiHeaderFormBase* pNewHeaderForm) override
	{
		m_minSize.iWidth = m_BTLeft + m_BTRight + MIN_HEADER_BAR_WIDTH;
		m_minSize.iHeight = m_BTTop + m_BTBottom + DEFAULT_HEADER_BAR_HEIGHT;
	}


protected:

	stFormSplitter *m_pFormSplitter;
	uiSize m_minSize, m_maxSize;


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


	BOOL SetProperty(TAB_FORM_FLAGS tff);
	BOOL AddPane(uiFormBase *pForm, INT index, BOOL bActivate);

	BOOL DeletePane(INT index);
	BOOL DetachPane(INT index, uiForm *pForm = nullptr);

	BOOL ActivateTab(INT index);

	void Layout();
	void UpdateTabsRect();
	void RedrawTabs(INT index1, INT index2 = -1);

	void OnMouseEnter(INT x, INT y) override
	{
		printx("---> uiTabForm::OnMouseEnter\n");
	}
	void OnMouseLeave() override
	{
		printx("---> uiTabForm::OnMouseLeave\n");

		if (m_HighlightIndex != -1)
		{
			if (m_HighlightIndex != m_ActiveIndex)
				RedrawTabs(m_HighlightIndex);
			m_HighlightIndex = -1;
		}
	}
	void OnMouseFocusLost(uiFormBase* pNewForm) override
	{
		printx("---> uiTabForm::OnMouseFocusLost\n");
		m_bDraggingTab = false;
	}

	void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiTabForm::OnMouseBtnClk x: %d, y: %d\n", x, y);
	}

	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override;
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y) override;
	void GetBufferRect(const uiRect &OldRect, const uiRect &NewRect, uiRect &out);
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd) override;
	void OnSize(UINT nNewWidth, UINT nNewHeight) override;
	void OnPaint(uiDrawer* pDrawer) override;


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

	SHORT m_ActiveIndex, m_HighlightIndex, m_TotalPane;
	SHORT m_LeftMargin, m_TopMargin, m_RightMargin, m_BottomMargin;
	UINT  m_Flag;

	UINT   m_ColorActive, m_ColorHover, m_ColorDefault;
	UINT16 m_TabWidth, m_TabHeight;

	std::vector<stPaneInfo> m_PaneInfoArray;

	uiRect m_TabsRegion, m_PaneRegion, m_OldTabRect;

	bool m_bDraggingTab = false;


private:

	INT AddPaneInfo(stPaneInfo *pNewPaneInfo, INT index = -1)
	{
		if (index == -1)
			index = static_cast<SHORT>(m_PaneInfoArray.size());
		m_PaneInfoArray.insert(m_PaneInfoArray.begin() + index, 1, *pNewPaneInfo);
		m_TotalPane = static_cast<SHORT>(m_PaneInfoArray.size());
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


class uiDockSiteBase : public uiTabForm
{
public:

	uiDockSiteBase();
	~uiDockSiteBase();

	FORM_CLASS GetClass() const final { return FC_DOCKSITE; }


	BOOL OnDeplate(INT iReason, uiFormBase *pDockingForm);


protected:


};