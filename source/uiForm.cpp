


#include "stdafx.h"
#include "uiForm.h"
#include "uiMsWin.h"
#include "FormStyle.h"


void uiMonitorEvent()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0U, 0U))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


static void FreeSplitter(stFormSplitter *pSplitter)
{
	if (pSplitter->pTopLeft != nullptr)
		FreeSplitter(pSplitter->pTopLeft);
	if (pSplitter->pBottomRight != nullptr)
		FreeSplitter(pSplitter->pBottomRight);
	delete pSplitter;
}


uiFormBase* uiFormBase::pAppBaseForm = nullptr;


uiFormBase::uiFormBase()
{
	m_pWnd = nullptr;
	m_pPlate = nullptr;
	m_pParent = nullptr;
	m_pStyle = nullptr;

	INIT_LIST_HEAD(&m_ListChildren);
	INIT_LIST_ENTRY(&m_ListChildrenEntry);

	m_DockFlag = 0;
	m_TimerCount = 0;
}

uiFormBase::~uiFormBase()
{
	printx("---> uiFormBase::~uiFormBase\n");
	ASSERT(!m_TimerCount);
}


BOOL uiFormBase::Create(uiFormBase *parent, INT x, INT y, UINT nWidth, UINT nHeight, FORM_CREATION_FLAG fcf)
{
	BOOL bResult = TRUE;

	if (/*!UICore::bSilentMode &&*/ parent == nullptr)
	{
		uiWindow *pWnd = CreateTemplateWindow(UWT_NORMAL, this, nullptr, x, y, nWidth, nHeight, !(fcf & FCF_INVISIBLE));
	//	printx("CreateTemplateWindow completed!\n");

		bResult = (pWnd != nullptr);
		if (pAppBaseForm == nullptr)
			pAppBaseForm = this;
		if (fcf & FCF_CENTER)
			pWnd->MoveToCenter();
	}
	else if (fcf & FCF_TOOL)
	{
		ASSERT(parent != nullptr);
		uiWindow *pWnd = CreateTemplateWindow(UWT_TOOL, this, parent, x, y, nWidth, nHeight, !(fcf & FCF_INVISIBLE));

		parent->AddChild(this);
		if (fcf & FCF_CENTER)
			pWnd->MoveToCenter();
	}
	else if (parent != nullptr)
	{
		m_FrameRect.SetPos(x, y);
		m_FrameRect.SetSize(nWidth, nHeight);
		parent->AddChild(this);
		m_pPlate = parent;

		if (fcf & FCF_CENTER)
		{
			if (parent->IsCreating())
				parent->AddPostCreateEvent(this);
			else
				MoveToCenter();
		}
	}
	else
	{
		ASSERT(0);
	}

	EntryOnCreate(!(fcf & FCF_INVISIBLE), nWidth, nHeight);

	return bResult;
}

void uiFormBase::Bind(UINT CmdID)
{
#ifdef _DEBUG

	std::vector<UINT> &IDList = GetIDList();
	for (size_t i = 0, total = IDList.size(); i < total; ++i)
		if (IDList[i] == CmdID)
		{
			ASSERT(0);
		}

#endif

	GetIDList().push_back(CmdID);
}

void uiFormBase::Close()
{
	if (OnClose())
	{
		if (GetParent() != nullptr)
			GetParent()->DetachChild(this);
		if (GetPlate() != nullptr)
		{


			RedrawForm();
		}

		uiWindow *pWnd = m_pWnd; // Cache this first.

		EntryOnDestroy(GetBaseWnd()); // m_pForm will delete itself after this call.

		if (pWnd != nullptr)
			pWnd->CloseImp();
	}
}

void uiFormBase::DePlate()
{
}

void uiFormBase::Move(INT x, INT y)
{
	if (IsRootForm())
	{
		m_pWnd->MoveImp(x, y);
	}
	else
	{
		MoveByOffset(x - m_FrameRect.Left, y - m_FrameRect.Top);
	}
}

void uiFormBase::MoveToCenter()
{
	if (IsRootForm())
	{
		m_pWnd->MoveToCenter();
	}
	else if (!m_bSideDocked)
	{
		uiRect rect = m_pPlate->GetClientRect(), FrameRect = GetFrameRect();
		INT mx = (rect.Width() - FrameRect.Width()) / 2;
		INT my = (rect.Height() - FrameRect.Height()) / 2;
		MoveByOffset(mx - m_FrameRect.Left, my - m_FrameRect.Top);
	}
	else
	{
		ASSERT(0);
	}
}

void uiFormBase::MoveByOffset(const INT OffsetX, const INT OffsetY, BOOL bForceRedraw)
{
	if (IsRootForm())
	{
		m_pWnd->MoveByOffsetImp(OffsetX, OffsetY);
		return;
	}

	if (OffsetX == 0 && OffsetY == 0)
		return;

	uiRect oldFrame = m_FrameRect;
	m_FrameRect.Move(OffsetX, OffsetY);

	uiFormBase::stFormMoveInfo fmi;
	fmi.XOffset = OffsetX;
	fmi.YOffset = OffsetY;
	if (OffsetX)
		fmi.MDFlag |= (OffsetX > 0) ? MOVE_RIGHT : MOVE_LEFT;
	if (OffsetY)
		fmi.MDFlag |= (OffsetY > 0) ? MOVE_DOWN : MOVE_UP;
	EntryOnMove(m_FrameRect.Left, m_FrameRect.Top, &fmi);

	GetBaseWnd()->RetrackMouseCheck(this); // Check after rectangle was updated.

	if (bForceRedraw || CheckVisibility())
	{
	//	oldFrame.UnionWith(m_FrameRect); // Windows support multi-region clipping.
		RedrawFrame(&oldFrame);
		RedrawFrame(&m_FrameRect);
	}
}

void uiFormBase::Show(FORM_SHOW_MODE sm)
{
	if (m_pWnd != nullptr)
		m_pWnd->ShowImp(sm);
	else
	{
		switch (sm)
		{
		case FSM_HIDE:
			bShow = false;
			RedrawForm();
			break;

		case FSM_SHOW:
			bShow = true;
			RedrawForm();
			break;

		case FSM_RESTORE:
			break;

		case FSM_MINIMIZE:
			break;

		case FSM_MAXIMIZE:
			break;
		}
	}
}

BOOL uiFormBase::SetTimer(UINT id, UINT msElapsedTime, INT nRunCount, void* pCtx)
{
	if (m_TimerCount == 255)
	{
		ASSERT(0);
		return FALSE;
	}
	return GetBaseWnd()->TimerAdd(this, id, msElapsedTime, nRunCount, pCtx);
}

BOOL uiFormBase::Size(INT NewWidth, INT NewHeight)
{
	if (NewWidth < 0 && NewHeight < 0)
		return FALSE;

	if (NewWidth < 0)
		NewWidth = m_FrameRect.Width();
	if (NewHeight < 0)
		NewHeight = m_FrameRect.Height();

	if (m_pWnd != nullptr)
		m_pWnd->SizeImp(NewWidth, NewHeight);

	uiRect oldFrame = m_FrameRect;
	m_FrameRect.SetSize(NewWidth, NewHeight);
	OnFrameSize(NewWidth, NewHeight);

	GetBaseWnd()->RetrackMouseCheck(this); // Check after rectangle was updated.

	if (CheckVisibility())
	{
	//	oldFrame.UnionWith(m_FrameRect);
		RedrawFrame(&oldFrame);
		RedrawFrame(&m_FrameRect);
	}

	return TRUE;
}

void uiFormBase::RedrawForm(const uiRect *pUpdateRect)
{
	if (!IsVisible())
		return;

	uiRect dest, temp;

	if (pUpdateRect == nullptr)
	{
		dest = m_FrameRect;
		if (GetPlate() != nullptr)
			GetPlate()->ToWindowSpace(this, dest, TRUE);
	}
	else
	{
		temp = m_ClientRect;
		temp.Move(m_FrameRect.Left, m_FrameRect.Top);
		if (GetPlate() != nullptr)
			GetPlate()->ToWindowSpace(this, temp, TRUE);

		dest = *pUpdateRect;
		dest.Move(temp.Left, temp.Top);
		dest.IntersectWith(temp);
	}

	if (dest.IsValidRect())
		GetBaseWnd()->RedrawImp(&dest);
}

void uiFormBase::RedrawFrame(const uiRect *pUpdateRect)
{
	if (!IsVisible())
		return;

	uiRect dest = (pUpdateRect == nullptr) ? m_FrameRect : *pUpdateRect;
	if (GetPlate() != nullptr)
		GetPlate()->ToWindowSpace(this, dest, TRUE);

	if (dest.IsValidRect())
		GetBaseWnd()->RedrawImp(&dest);
}

uiFormBase* uiFormBase::SetCapture()
{
	return GetBaseWnd()->CaptureMouseFocus(this);
}

BOOL uiFormBase::ReleaseCapture()
{
	return GetBaseWnd()->ReleaseMouseFocus(this);
}

BOOL uiFormBase::CaretShow(BOOL bShow, INT x, INT y, INT width, INT height)
{
	if (!bShow)
	{
		GetBaseWnd()->CaretHideImp(this);
		bOwnCaret = false;
	}
	else
	{
		ASSERT(width != -1 && height != -1);
		uiRect rect;
		ClientToWindow(rect);
		GetBaseWnd()->CaretShowImp(this, x + rect.Left, y + rect.Top, width, height);
		bOwnCaret = true;
	}

	return FALSE;
}

BOOL uiFormBase::CaretMoveByOffset(INT OffsetX, INT OffsetY)
{
	if (!bOwnCaret)
		return FALSE;
	return GetBaseWnd()->CaretMoveByOffset(this, OffsetX, OffsetY);
}

BOOL uiFormBase::CaretMove(INT x, INT y)
{
	if (!bOwnCaret)
		return FALSE;

	uiRect rect;
	ClientToWindow(rect);
	return GetBaseWnd()->CaretMoveImp(this, x + rect.Left, y + rect.Top);
}

void uiFormBase::PopupMenu(INT x, INT y, uiMenu *pMenu)
{
	INT sx, sy;
	uiRect rect;
	ClientToScreen(sx, sy);

}

/*
uiFormBase* FindFromSplitter(stFormSplitter *pSplitter, INT x, INT y)
{
	if (pSplitter->RectTopLeft.IsPointIn(x, y))
	{
		if (pSplitter->pTopLeft != nullptr)
			return FindFromSplitter(pSplitter->pTopLeft, x, y);
		return pSplitter->pTopLeftForm;
	}
	if (pSplitter->RectBottomRight.IsPointIn(x, y))
	{
		if (pSplitter->pBottomRight != nullptr)
			return FindFromSplitter(pSplitter->pBottomRight, x, y);
		return pSplitter->pBottomRightForm;
	}
	return nullptr;
}

uiFormBase* FindByPosImp(uiFormBase *pForm, INT x, INT y)
{
	uiFormBase *pSubForm;
	list_entry *pNext;
	list_head &ListChildren = pForm->m_ListChildren;

	for (pNext = ListChildren.next; IS_VALID_ENTRY(pNext, ListChildren); pNext = pNext->next)
	{
		pSubForm = CONTAINING_RECORD(pNext, uiFormBase, m_ListChildrenEntry);
//		if (pSubForm->m_pDockingPlate != nullptr)
//			continue;
		if (pSubForm->IsPointIn(x, y))
			return FindByPosImp(pSubForm, x, y);
	}

	return pForm;
}
//*/

void UpdateDockRect(stFormSplitter &splitter, FORM_DOCKING_FLAG df, UINT SizingBarWidth, uiRect &CurrentRect, uiFormBase *pDocking, uiFormBase *pPlate)
{
	uiRect &rect = pDocking->m_FrameRect;
	INT NewWidth, NewHeight;

	if (df & FDF_TOP)
	{
		rect.Move(CurrentRect.Left - rect.Left, CurrentRect.Top - rect.Top);
		NewHeight = CurrentRect.Height() - rect.Height() - SizingBarWidth;

		CurrentRect.Move(0, rect.Height() + SizingBarWidth);
		CurrentRect.SetHeight(NewHeight);

		splitter.RectTopLeft = rect;
		splitter.RectBottomRight = CurrentRect;
		splitter.pTopLeftForm = pDocking;
		splitter.pBottomRightForm = pPlate;

		pPlate->m_ClientRect = CurrentRect;
	}
	else if (df & FDF_BOTTOM)
	{
	}
	else if (df & FDF_LEFT)
	{
		splitter.bVertical = true;
	}
	else if (df & FDF_RIGHT)
	{
		splitter.bVertical = true;
	}
}

uiFormBase* uiFormBase::FindByPos(INT x, INT y, INT *DestX, INT *DestY)
{
	UTX::CSimpleList *pSideDockedFormList = GetSideDockedFormList();
	if (pSideDockedFormList != nullptr)
	{
		const list_head &head = pSideDockedFormList->GetListHead();
		for (list_entry *pNext = head.next; IS_VALID_ENTRY(pNext, head); pNext = pNext->next)
		{
			uiFormBase *pForm = (uiFormBase*)pSideDockedFormList->GetAt(pNext);
			ASSERT(pForm->GetPlate() == this);
			if (!pForm->bShow)
				continue;
			if (pForm->IsPointIn(x, y))
				return pForm->FindByPos(x - pForm->m_FrameRect.Left, y - pForm->m_FrameRect.Top, DestX, DestY);
		}
	}

	if (!m_ClientRect.IsPointIn(x, y))
	{
		*DestX = x - m_ClientRect.Left;
		*DestY = y - m_ClientRect.Top;
		return this;
	}

	INT cx = x - m_ClientRect.Left, cy = y - m_ClientRect.Top;
	uiFormBase *pSubForm = this;
	for (list_entry *pEntry = LIST_GET_TAIL(m_ListChildren); IS_VALID_ENTRY(pEntry, m_ListChildren); pEntry = pEntry->prev)
	{
		pSubForm = CONTAINING_RECORD(pEntry, uiFormBase, m_ListChildrenEntry);
		if (!pSubForm->bShow || pSubForm->m_bSideDocked)
			continue;
		if (pSubForm->GetPlate() != this)
			continue;
		if (pSubForm->IsPointIn(cx, cy))
			return pSubForm->FindByPos(cx - pSubForm->m_FrameRect.Left, cy - pSubForm->m_FrameRect.Top, DestX, DestY);
	}

	*DestX = x - m_ClientRect.Left;
	*DestY = y - m_ClientRect.Top;
	return this;
}

void uiFormBase::ToWindowSpace(uiFormBase *pForm, uiRect &rect, BOOL bClip)
{
	if (pForm->m_bSideDocked)
	{
		rect.Move(m_FrameRect.Left, m_FrameRect.Top);

		if (bClip)
			rect.IntersectWith(m_FrameRect);
	}
	else
	{
		rect.Move(m_ClientRect.Left, m_ClientRect.Top);
		if (bClip)
			rect.IntersectWith(m_ClientRect);
		rect.Move(m_FrameRect.Left, m_FrameRect.Top);
	}

	if (GetPlate() != nullptr)
		GetPlate()->ToWindowSpace(this, rect, bClip);
}

void uiFormBase::ToWindowSpace(uiFormBase *pForm, INT &x, INT &y)
{
	if (pForm->m_bSideDocked)
	{
		x += m_FrameRect.Left;
		y += m_FrameRect.Top;
	}
	else
	{
		x = x + m_FrameRect.Left + m_ClientRect.Left;
		y = y + m_FrameRect.Top + m_ClientRect.Top;
	}

	if (GetPlate() != nullptr)
		GetPlate()->ToWindowSpace(this, x, y);
}

void uiFormBase::StartDragging(MOUSE_KEY_TYPE mkt, INT wcX, INT wcY)
{
	uiRect rect;
	if (GetPlate() != nullptr)
	{
		if (this->m_bSideDocked)
		{
		}
		else
		{
			GetPlate()->ClientToWindow(rect);
		}
	}

	uiWindow *pWnd = GetBaseWnd();
	pWnd->StartDraggingImp(this, mkt, wcX, wcY, rect);
}

void uiFormBase::EntryOnCreate(BOOL bShowIn, UINT nWidth, UINT nHeight)
{
	ASSERT(!bCreated && !bCreating);

	UTX::CSimpleList PostCreateList;
	SetPostCreateList(&PostCreateList);

	bCreating = true;
	m_FrameRect.SetSize(nWidth, nHeight);
	m_ClientRect.SetSize(nWidth, nHeight); // Must set the default size for the client rectangle.

	OnCreate();
	OnFrameSize(m_FrameRect.Width(), m_FrameRect.Height());
	EntryOnMove(m_FrameRect.Left, m_FrameRect.Top, &stFormMoveInfo());

	uiFormBase* pFormBase;
	for (; PostCreateList.size();)
	{
		pFormBase = (uiFormBase*)PostCreateList.pop_back();
		pFormBase->MoveToCenter();
	}
	ASSERT(PostCreateList.size() == 0);

	// CleanPostCreateList();
	m_pStyle = GetDefaultStyleObject(this);

	bCreated = true;
	bCreating = false;
	bShow = (bShowIn != 0); // Put this at last to prevent move and size functions calling RedrawForm.
}

BOOL uiFormBase::EntryOnClose()
{
	return TRUE;
}

void uiFormBase::EntryOnDestroy(uiWindow *pWnd)
{
	uiFormBase *pChild;
	list_entry *pCurrent, *pNext;
	for (pCurrent = m_ListChildren.next; IS_VALID_ENTRY(pCurrent, m_ListChildren); pCurrent = pNext)
	{
		pNext = pCurrent->next;
		pChild = CONTAINING_RECORD(pCurrent, uiFormBase, m_ListChildrenEntry);

		if (pChild->IsRootForm())
			pChild->EntryOnDestroy(pChild->m_pWnd);
		else
			pChild->EntryOnDestroy(pWnd);
	}

	OnDestroy();
	pWnd->OnFormDestroy(this);

	delete this;
}

void uiFormBase::EntryOnMove(INT x, INT y, const stFormMoveInfo *pInfo)
{
	OnMove(x, y, pInfo);

	if (bOwnCaret && !IsBaseForm())
		CaretMoveByOffset(pInfo->XOffset, pInfo->YOffset);
}

void uiFormBase::EntryOnPaint(uiDrawer* pDrawer, INT depth)
{
	OnFramePaint(pDrawer);

	UTX::CSimpleList *pSideDockedFormList = GetSideDockedFormList();
	if (pSideDockedFormList != nullptr)
	{
		const list_head &head = pSideDockedFormList->GetListHead();
		for (list_entry *pEntry = LIST_GET_HEAD(head); IS_VALID_ENTRY(pEntry, head); pEntry = pEntry->next)
		{
			uiFormBase *pForm = (uiFormBase*)pSideDockedFormList->GetAt(pEntry);

			if (pDrawer->PushDestRect(pForm->m_FrameRect))
			{
				ASSERT(pForm->m_FrameRect.IsValidRect());
				pForm->EntryOnPaint(pDrawer, depth + 1);
				pDrawer->PopDestRect();
			}
		}
	}

	if (pDrawer->PushDestRect(m_ClientRect))
	{
		OnPaint(pDrawer);

		for (list_entry *pEntry = LIST_GET_HEAD(m_ListChildren); IS_VALID_ENTRY(pEntry, m_ListChildren); pEntry = pEntry->next)
		{
			uiFormBase *pChild = CONTAINING_RECORD(pEntry, uiFormBase, m_ListChildrenEntry);

			if (!pChild->bShow || pChild->m_bSideDocked)
				continue;
			if (pChild->GetPlate() != this)
				continue;

			if (pDrawer->PushDestRect(pChild->m_FrameRect))
			{
				ASSERT(pChild->m_FrameRect.IsValidRect());
				pChild->EntryOnPaint(pDrawer, depth + 1);
				pDrawer->PopDestRect();
			}
		}

		pDrawer->PopDestRect();
	}
}

void uiFormBase::EntryOnSize(UINT nNewWidth, UINT nNewHeight)
{
	m_FrameRect.Resize(nNewWidth, nNewHeight);
	OnFrameSize(nNewWidth, nNewHeight);
}

void uiFormBase::EntryOnKBGetFocus()
{
	OnKBGetFocus();
}

void uiFormBase::EntryOnKBLoseFocus()
{
	OnKBLoseFocus();

	if (bOwnCaret)
		CaretShow(FALSE);
}

void uiFormBase::EntryOnCommand(UINT id)
{
	BOOL bDone = FALSE;
	std::vector<UINT> &IDList = GetIDList();
	for (size_t i = 0, total = IDList.size(); i < total; ++i)
	{
		if (IDList[i] == id)
		{
			bDone = TRUE;
			OnCommand(id, bDone);
			break;
		}
	}

	if (!bDone)
	{
		if (id < uiID_SYSTEM)
			uiFormBase::OnCommand(id, bDone);
		else
		{
			printx("No handler for command ID: %d\n", id);
		}
	}
}

void uiFormBase::OnCommand(INT id, BOOL &bDone)
{
	ASSERT(!bDone);
	bDone = TRUE;

	switch(id)
	{
	case uiID_CLOSE:
		Close();
		break;

	case uiID_MINIMIZE:
		Show(FSM_MINIMIZE);
		break;

	case uiID_MAXIMIZE:
		break;

	case uiID_RESTORE:
		break;
	}
}

BOOL uiFormBase::OnDeplate(INT iReason, uiFormBase *pDockingForm)
{
	return TRUE;
}

void uiFormBase::OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
//	printx("---> uiFormBase::OnMouseBtnClk Type:%d x:%d y:%d\n", KeyType, x, y);
}

void uiFormBase::OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
//	printx("---> uiFormBase::OnMouseBtnDbClk Type:%d x:%d y:%d\n", KeyType, x, y);
}

void uiFormBase::OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
//	printx("---> uiFormBase::OnMouseBtnDown Type:%d x:%d y:%d\n", KeyType, x, y);
}

void uiFormBase::OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
//	printx("---> uiFormBase::OnMouseBtnUp Type:%d x:%d y:%d\n", KeyType, x, y);
}

void uiFormBase::OnMouseEnter(INT x, INT y)
{
	//printx("---> uiFormBase::OnMouseEnter\n");
}

void uiFormBase::OnMouseLeave()
{
	//printx("---> uiFormBase::OnMouseLeave\n");
}

void uiFormBase::OnMouseFocusLost()
{
}

void uiFormBase::OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
{
}

void uiFormBase::OnMove(INT x, INT y, const stFormMoveInfo *pInfo)
{
//	printx("---> uiFormBase::OnMove x: %d, y: %d.\n", x, y);
}

INT uiFormBase::OnNCHitTest(INT x, INT y)
{
	return NCHT_CLIENT;
}

void uiFormBase::OnPaint(uiDrawer* pDrawer)
{
	// Draw edge.
	uiFormStyle *pStyle = m_pStyle;
	if (pStyle == nullptr)
	{
	}

	pDrawer->FillRect(GetClientRect(), RGB(60, 80, 60));
}

void uiFormBase::OnFramePaint(uiDrawer* pDrawer)
{
}

void uiFormBase::OnFrameSize(UINT nNewWidth, UINT nNewHeight)
{
	SetClientRect(uiRect(nNewWidth, nNewHeight));
}

void uiFormBase::OnSize(UINT nNewWidth, UINT nNewHeight)
{
//	printx("---> uiFormBase::OnSize\n");

//	printx("---> Frame x:%d y:%d x2:%d y2:%d (Width: %d Height: %d)\n",
//		m_FrameRect.Left, m_FrameRect.Top, m_FrameRect.Right, m_FrameRect.Bottom, m_FrameRect.Width(), m_FrameRect.Height());
}

void uiFormBase::OnTimer(stTimerInfo* ti)
{
//	printx("---> uiFormBase::OnTimer\n");
	printx("Warning: Unhandled timer found!\n");
	ti->nRunCount = 0;
}


BOOL uiMenuBar::Create(uiFormBase* parent, uiMenu *pMenu)
{
	ASSERT(pMenu != nullptr);

	BOOL bResult = uiFormBase::Create(parent, 0, 0, 100, DEFAULT_BAR_HEIGHT, FCF_NONE);

//	m_Count = pMenu->GetCount(FALSE);
	m_Count = 3;
	m_RectArray = new uiRect[3];

	INT height = GetClientRect().Bottom, width = 60, right = 0;

	for (INT i = 0; i < m_Count; ++i)
	{
		m_RectArray[i].Left = right;
		right += width;
		m_RectArray[i].Right = right;

		m_RectArray[i].Bottom = height;
	}

	return bResult;
}


void uiHeaderForm::EntryOnCommand(UINT id)
{
	ASSERT(GetParent() != nullptr);
	GetParent()->EntryOnCommand(id);
}

BOOL uiHeaderForm::ShowButton(BOOL bShowMin, BOOL bShowMax, BOOL bShowClose)
{
	ASSERT(IsCreated());
	return TRUE;
}

void uiHeaderForm::OnMouseEnter(INT x, INT y)
{
	printx("---> uiHeaderForm::OnMouseEnter\n");
//	ClientToWindow(x, y);
//	GetParent()->StartDragging(MKT_LEFT, x, y);
	if (GetKeyState(VK_F3) < 0)
	//	GetParent()->Close();
	//	GetParent()->MoveByOffset(0, -50);
		GetParent()->Size(100, 50);
}

void uiHeaderForm::OnMouseLeave()
{
	printx("---> uiHeaderForm::OnMouseLeave\n");

	if (GetKeyState(VK_F3) < 0)
		GetParent()->MoveByOffset(0, 5);
}

void uiHeaderForm::OnCreate()
{
	//	printx("---> uiHeaderForm::OnCreate\n");
	ASSERT(m_pCloseBtn == nullptr);

	uiRect rect = GetClientRect();
	if ((m_pCloseBtn = new uiButton) != nullptr)
	{
		m_pCloseBtn->SetID(uiID_CLOSE);
		m_pCloseBtn->Create(this, rect.Right - 30, 1, 28, 28);
	}

	if ((m_pMinBtn = new uiButton) != nullptr)
	{
		m_pMinBtn->SetID(uiID_MINIMIZE);
		m_pMinBtn->Create(this, rect.Right - 58, 1, 28, 28);
	}

	UpdateLayout(rect.Width(), rect.Height());
}

void uiHeaderForm::OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
{
//	if (GetKeyState(VK_F2) < 0)
//		Close();
}

void uiHeaderForm::OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	printx("---> uiHeaderForm::OnMouseBtnDown\n");

	ClientToWindow(x, y);

	switch (KeyType)
	{
	case MKT_LEFT:
		GetParent()->StartDragging(MKT_LEFT, x, y);
		break;
	case MKT_MIDDLE:
	//	GetParent()->StartDragging(MKT_MIDDLE, x, y);
		break;
	case MKT_RIGHT:
	//	GetParent()->StartDragging(MKT_RIGHT, x, y);
		break;
	}
}

void uiHeaderForm::OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	//printx("---> uiHeaderForm::OnMouseBtnUp\n");
}

void uiHeaderForm::OnPaint(uiDrawer* pDrawer)
{
//	printx("---> uiHeaderForm::OnPaint\n");

	uiRect rect = GetClientRect();
	UINT32 color = uiGetSysColor(SCN_CAPTAIN);
	pDrawer->FillRect(rect, color);
//	pDrawer->FillRect(rect, RGB(255, 255, 255));

	pDrawer->DrawText(GetParent()->GetTitle(), rect, DT_CENTER);
}

void uiHeaderForm::OnSize(UINT nNewWidth, UINT nNewHeight)
{
//	printx("---> uiHeaderForm::OnSize. Width: %d, Height: %d\n", nNewWidth, nNewHeight);
	UpdateLayout(nNewWidth, nNewHeight);
}

void uiHeaderForm::UpdateLayout(INT NewWidth, INT NewHeight)
{
	INT ButtonHeight = (GetClientRect().Height() - 1 * 2);

	if (m_pCloseBtn != nullptr)
	{
		m_pCloseBtn->Move(NewWidth - 30, 1);
		m_pCloseBtn->Size(-1, ButtonHeight);
	}
	if (m_pMinBtn != nullptr)
	{
		m_pMinBtn->Move(NewWidth - 58, 1);
		m_pMinBtn->Size(-1, ButtonHeight);
	}
}


BOOL uiForm::DockForm(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf)
{
//	ASSERT(pDockingForm->m_pDockingPlate == nullptr);
//	if (pDockingForm->m_pDockingPlate != nullptr)
//		return FALSE;

	uiRect CurRect = m_FrameRect;
	stFormSplitter NewSplitter;
	NewSplitter.RectFull = CurRect;

	if (m_pFormSplitter == nullptr)
	{
		UpdateDockRect(NewSplitter, fdf, 0, CurRect, pDockingForm, this);
		m_pFormSplitter = new stFormSplitter(NewSplitter);
	}

	//	if (pDockingForm)

	return TRUE;
}

BOOL uiForm::SideDock(uiFormBase* pDockingForm, FORM_DOCKING_FLAG fdf)
{
	ASSERT(!pDockingForm->m_bSideDocked);

	const uiRect& ClientRect = GetClientRectFS();
	uiRect rect = ClientRect;

	UINT DockingLocation = fdf & 0x0f;
	switch (DockingLocation)
	{
	case FDF_TOP:
		rect.Inflate(0, -pDockingForm->m_FrameRect.Height(), 0, 0);
		pDockingForm->Move(ClientRect.Left, ClientRect.Top);
		if (fdf & FDF_AUTO_SIZE)
			pDockingForm->Size(rect.Width(), -1);
		break;
	case FDF_BOTTOM:
		rect.Inflate(0, 0, 0, -pDockingForm->m_FrameRect.Height());
		if (fdf & FDF_AUTO_SIZE)
			pDockingForm->Size(rect.Width(), -1);
		break;
	case FDF_LEFT:
		rect.Inflate(-pDockingForm->m_FrameRect.Width(), 0, 0, 0);
		if (fdf & FDF_AUTO_SIZE)
			pDockingForm->Size(-1, rect.Height());
		break;
	case FDF_RIGHT:
		rect.Inflate(0, 0, -pDockingForm->m_FrameRect.Width(), 0);
		if (fdf & FDF_AUTO_SIZE)
			pDockingForm->Size(-1, rect.Height());
		break;
	}

	pDockingForm->m_bSideDocked = true;
	pDockingForm->m_DockFlag = fdf;
	ASSERT(pDockingForm->GetParent() == this);
	m_SideDockedFormList.push_back(pDockingForm);

	SetClientRect(rect);
	RedrawForm();

	return TRUE;
}

void uiForm::OnCreate()
{
}

BOOL uiForm::SetHeaderBar(const TCHAR* pStr, uiHeaderForm *pHeaderForm)
{
	INT HeaderHeight = 26;
	INT BorderWidth = 3;

	if (pHeaderForm == nullptr)
	{
		pHeaderForm = new uiHeaderForm;
		pHeaderForm->Create(this, 0, 0, 100, HeaderHeight, FCF_NONE);
	}
	m_minSize.iHeight = HeaderHeight + 2 * BorderWidth;
	m_minSize.iWidth = 150;

	SideDock(pHeaderForm, FDF_TOP | FDF_AUTO_SIZE);

	return FALSE;
}

BOOL uiForm::SetMenuBar(uiMenu* pMenu)
{
	uiMenuBar *pMenuBar = new uiMenuBar;
	pMenuBar->Create(this, pMenu);

//	m_minSize.iHeight = HeaderHeight + 2 * BorderWidth;
//	m_minSize.iWidth = 150;

	SideDock(pMenuBar, (FORM_DOCKING_FLAG)(FDF_TOP | FDF_AUTO_SIZE));

	return TRUE;
}

INT uiForm::OnNCHitTest(INT x, INT y)
{
//	printx("---> uiForm::OnNCHitTest width: %d, height: %d, x: %d, y: %d\n", m_FrameRect.Width(), m_FrameRect.Height(), x, y);

	INT iRet = NCHT_CLIENT, nBorderWidth = 3;

	if (!bNoBorder)
	{
		if (x < nBorderWidth)
			iRet |= NCHT_LEFT;
		if (m_FrameRect.Width() - x <= nBorderWidth)
			iRet |= NCHT_RIGHT;
		if (y < nBorderWidth)
			iRet |= NCHT_TOP;
		if (m_FrameRect.Height() - y <= nBorderWidth)
			iRet |= NCHT_BOTTOM;
	}

	return iRet;
}

void uiForm::OnPaint(uiDrawer* pDrawer)
{
	uiFormBase::OnPaint(pDrawer);
}

void uiForm::OnFrameSize(UINT nNewWidth, UINT nNewHeight)
{
	UpdataClientRect();
}

void uiForm::OnSize(UINT nNewWidth, UINT nNewHeight)
{

}

void uiForm::UpdataClientRect()
{
	uiRect rect = GetFrameRect();

	if (!bNoBorder)
	{
		INT BorderWidth = -3;
		rect.Inflate(BorderWidth, BorderWidth, BorderWidth, BorderWidth);
	}

	const list_head& ListHead = m_SideDockedFormList.GetListHead();
	for (list_entry *pEntry = LIST_GET_HEAD(ListHead); IS_VALID_ENTRY(pEntry, ListHead); pEntry = pEntry->next)
	{
		uiFormBase *pForm = (uiFormBase*)m_SideDockedFormList.GetAt(pEntry);
		UINT DockingLocation = pForm->m_DockFlag & 0x0f;
		switch (DockingLocation)
		{
		case FDF_TOP:
			if (pForm->m_DockFlag & FDF_AUTO_SIZE)
			{
				pForm->Move(rect.Left, rect.Top);
				pForm->Size(rect.Width(), -1);
			}
			rect.Inflate(0, -pForm->m_FrameRect.Height(), 0, 0);
			break;
		case FDF_BOTTOM:
			rect.Inflate(0, 0, 0, -pForm->m_FrameRect.Height());
			if (pForm->m_DockFlag & FDF_AUTO_SIZE)
				pForm->Size(rect.Width(), -1);
			break;
		case FDF_LEFT:
			rect.Inflate(-pForm->m_FrameRect.Width(), 0, 0, 0);
			break;
		case FDF_RIGHT:
			rect.Inflate(0, 0, -pForm->m_FrameRect.Width(), 0);
			break;
		}
	}

	if (rect != GetClientRectFS())
		SetClientRect(rect);
}


void uiFrame::OnCreate()
{
	//	m_ClientRect.Move(2, 2);
	//m_ClientRect.Inflate(-2, -2);

}

void uiFrame::OnFramePaint(uiDrawer* pDrawer)
{
//	uiRect rect = m_FrameRect;

//	DWORD color = GetSysColor(COLOR_ACTIVECAPTION);
	//pDrawer->FillRect(rect, color);
//	pDrawer->FillRect(rect, RGB(GetRValue(color), GetGValue(color), GetBValue(color)));
	//	pDrawer->FillRect(rect, RGB(255, 255, 255));
}

void uiFrame::OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
{
	printx("uiFrame::OnMouseMove x: %d, y: %d\n", x, y);


}

void uiFrame::OnPaint(uiDrawer* pDrawer)
{
	uiRect rect = GetClientRect();
	pDrawer->FillRect(rect, RGB(128, 128, 128));
}


uiDockableForm::uiDockableForm()
{
	INIT_LIST_HEAD(&m_ListDockingForm);
}

uiDockableForm::~uiDockableForm()
{
}

BOOL uiDockableForm::OnDeplate(INT iReason, uiFormBase *pDockingForm)
{
	return TRUE;
}


BOOL uiTabForm::SetProperty(TAB_FORM_FLAGS tff)
{
	m_Flag = tff;
	if (m_Flag & TFF_TAB_TOP || m_Flag & TFF_TAB_BOTTOM)
	{
		m_TabHeight = DEFAULT_TAB_HEIGHT;
	}
	else
	{
		m_TabWidth = DEFAULT_TAB_HEIGHT;
	}
	return TRUE;
}

BOOL uiTabForm::AddPane(uiFormBase *pForm, INT index, BOOL bActivate)
{
	stPaneInfo NewPaneInfo;

	if (TestFlag(TFF_TAB_TOP) || TestFlag(TFF_TAB_BOTTOM))
		NewPaneInfo.FullRect = uiSize(80, m_TabHeight); // test code.
	else
		NewPaneInfo.FullRect = uiSize(m_TabWidth, 80);

//	if (GetKeyState(VK_F2) < 0)
//		NewPaneInfo.FullRect = uiSize(200, m_TabHeight);

	if (pForm == nullptr)
	{
		pForm = new uiFormBase;
		pForm->Create(this, 0, 0, 1, 1, FCF_NONE);
		NewPaneInfo.bTabForm = true;
	}
	NewPaneInfo.pForm = pForm;

	index = AddPaneInfo(&NewPaneInfo, index);
	Layout();

	// m_PaneRegion is valid until Layout() is called.
	pForm->Move(m_PaneRegion.Left, m_PaneRegion.Top);
	pForm->Size(m_PaneRegion.Width(), m_PaneRegion.Height());
	pForm->Show(FSM_HIDE);

	if (bActivate || m_TotalPane == 1)
		ActivateTab(index);

	if (m_TotalPane == 2 && !TestFlag(TFF_FORCE_SHOW_TAB))
	{
		// Update size and position of the first pane.
		GetPaneInfo(0)->pForm->Move(m_PaneRegion.Left, m_PaneRegion.Top);
		GetPaneInfo(0)->pForm->Size(m_PaneRegion.Width(), m_PaneRegion.Height());
		RedrawForm();
	}
	else if (m_TotalPane != 1 || TestFlag(TFF_FORCE_SHOW_TAB))
	{
		RedrawTabs(-1); // It's safe to redraw all tabs here.
	}

	return TRUE;
}

BOOL uiTabForm::DeletePane(INT index)
{
	ASSERT(m_TotalPane > 0);

	if (m_TotalPane == 0 || index >= m_TotalPane)
		return FALSE;
	if (index < 0)
		index = m_TotalPane - 1;

	BOOL bUpdateTabsRect = (index != m_TotalPane - 1);
	INT NewActiveIndex = m_ActiveIndex;
	if (index == m_ActiveIndex)
	{
		NewActiveIndex -= (m_ActiveIndex == m_TotalPane - 1) ? 1 : -1;
		if (NewActiveIndex >= 0)
			ActivateTab(NewActiveIndex);
		else
			m_ActiveIndex = -1;
	}

	uiFormBase *pForm = GetPaneInfo(index)->pForm;
	pForm->Close();
	ReleasePaneInfo(index);

	if (m_ActiveIndex > index)
		--m_ActiveIndex;
	if (bUpdateTabsRect)
		UpdateTabsRect();

	if (m_TotalPane == 1 && !TestFlag(TFF_FORCE_SHOW_TAB))
	{
		Layout();
		GetPaneInfo(0)->pForm->Move(m_PaneRegion.Left, m_PaneRegion.Top);
		GetPaneInfo(0)->pForm->Size(m_PaneRegion.Width(), m_PaneRegion.Height());
		RedrawForm();
	}
	else
		RedrawTabs(-1);

	return TRUE;
}

BOOL uiTabForm::ActivateTab(INT index)
{
	if (m_ActiveIndex == index)
		return FALSE;

	if (m_TotalPane > 1)
		GetPaneInfo(m_ActiveIndex)->pForm->Show(FSM_HIDE);

	GetPaneInfo(index)->pForm->Show(FSM_SHOW);
	RedrawTabs(index, m_ActiveIndex);
	m_ActiveIndex = index;

	return TRUE;
}

void uiTabForm::Layout()
{
	if (m_TotalPane == 0)
	{
		m_TabsRegion.Reset();
		m_PaneRegion.Reset();
		return;
	}

	uiRect ClientRect = GetClientRect();
	ClientRect.Inflate(-m_LeftMargin, -m_TopMargin, -m_RightMargin, -m_BottomMargin);
	m_PaneRegion = m_TabsRegion = ClientRect;

	if (m_TotalPane == 1 && !TestFlag(TFF_FORCE_SHOW_TAB))
	{
		m_TabsRegion.Reset();
		return;
	}

	switch(m_Flag & (TFF_TAB_TOP | TFF_TAB_BOTTOM | TFF_TAB_LEFT | TFF_TAB_RIGHT))
	{
	case TFF_TAB_TOP:
		m_TabsRegion.Bottom = ClientRect.Top + m_TabHeight;
		m_PaneRegion.Inflate(0, -m_TabHeight, 0, 0);
		break;
	case TFF_TAB_BOTTOM:
		m_TabsRegion.Top = ClientRect.Height() - m_TabHeight;
		m_PaneRegion.Inflate(0, 0, 0, -m_TabHeight);
		break;
	case TFF_TAB_LEFT:
		m_TabsRegion.Right = ClientRect.Left + m_TabWidth;
		m_PaneRegion.Inflate(-m_TabWidth, 0, 0, 0);
		break;
	case TFF_TAB_RIGHT:
		m_TabsRegion.Left = ClientRect.Width() - m_TabWidth;
		m_PaneRegion.Inflate(0, 0, -m_TabWidth, 0);
		break;
	}

	UpdateTabsRect();
}

void uiTabForm::UpdateTabsRect()
{
	ASSERT(m_TotalPane >= 1);
//	ASSERT(m_pPaneInfoArray != nullptr);

	BOOL bUseFullSize = FALSE;
	uiSize FullTabsSize = GetFullTabsSize();
	INT top, bottom, left, right, width, height, mod, var = 0;
	uiRect rect = GetClientRect();
	rect.Inflate(-m_LeftMargin, -m_TopMargin, -m_RightMargin, -m_BottomMargin);

	if (m_Flag & TFF_TAB_TOP || m_Flag & TFF_TAB_BOTTOM)
	{
		top = (m_Flag & TFF_TAB_TOP) ? rect.Top : rect.Bottom - m_TabHeight;
		bottom = top + m_TabHeight;
		width = rect.Width() / m_TotalPane;
		mod = rect.Width() % m_TotalPane;
		if (rect.Width() > FullTabsSize.iWidth)
			bUseFullSize = TRUE;
		var = rect.Left;
	}
	else
	{
		left = (m_Flag & TFF_TAB_LEFT) ? rect.Left : rect.Right - m_TabWidth;
		right = left + m_TabWidth;
		height = rect.Height() / m_TotalPane;
		mod = rect.Height() % m_TotalPane;
		if (rect.Height() > FullTabsSize.iHeight)
			bUseFullSize = TRUE;
		var = rect.Top;
	}

	for (INT i = 0; i < m_TotalPane; ++i, mod--)
	{
		stPaneInfo *pInfo = GetPaneInfo(i);
		if (m_Flag & TFF_TAB_TOP || m_Flag & TFF_TAB_BOTTOM)
		{
			pInfo->Rect.Top = top;
			pInfo->Rect.Bottom = bottom;
			pInfo->Rect.Left = var;
			var += (bUseFullSize) ? pInfo->FullRect.iWidth : width;
			if (!bUseFullSize && mod > 0)
				++var;
			pInfo->Rect.Right = var;
		}
		else // if (m_Flag & TFF_TAB_LEFT || m_Flag & TFF_TAB_RIGHT)
		{
			pInfo->Rect.Left = left;
			pInfo->Rect.Right = right;
			pInfo->Rect.Top = var;
			var += (bUseFullSize) ? pInfo->FullRect.iHeight : height;
			if (!bUseFullSize && mod > 0)
				++var;
			pInfo->Rect.Bottom = var;
		}
	}
}

void uiTabForm::RedrawTabs(INT index1, INT index2)
{
	if (!IsVisible())
		return;

	if (index1 == -1)
	{
		RedrawForm(&m_TabsRegion);
		return;
	}

	uiRect rect = GetPaneInfo(index1)->Rect;
	if (index2 != -1)
		rect.UnionWith(GetPaneInfo(index2)->Rect);
	RedrawForm(&rect);
}

void uiTabForm::OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	switch (KeyType)
	{
	case MKT_LEFT:
		if (m_HighlightIndex != -1 && m_ActiveIndex != m_HighlightIndex)
			ActivateTab(m_HighlightIndex);
		if (m_HighlightIndex != -1 && TestFlag(TFF_DRAGGABLE_TAB))
		{
			SetCapture();
			m_bDraggingTab = true;
		}
		break;

	case MKT_RIGHT:
		if (m_HighlightIndex != -1 && m_ActiveIndex != m_HighlightIndex)
			ActivateTab(m_HighlightIndex);
		break;
	}
}

void uiTabForm::OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y)
{
	switch (KeyType)
	{
	case MKT_LEFT:
		if (m_HighlightIndex != -1 && m_ActiveIndex != m_HighlightIndex)
		{

		}
		if (m_bDraggingTab)
		{
			ReleaseCapture();
			m_bDraggingTab = false;
		}
		printx("---> MKT_LEFT up\n");
		break;

	case MKT_RIGHT:
		printx("---> MKT_RIGHT up\n");
		break;
	}
}

void uiTabForm::GetBufferRect(const uiRect &OldRect, const uiRect &NewRect, uiRect &out)
{
	if (TestFlag(TFF_TAB_TOP) || TestFlag(TFF_TAB_BOTTOM))
	{
		if (OldRect.Left < NewRect.Left)
			out.Init(OldRect.Right, OldRect.Top, NewRect.Left, OldRect.Bottom);
		else
			out.Init(NewRect.Right, OldRect.Top, OldRect.Left, OldRect.Bottom);
	}
	else
	{
		if (OldRect.Top < NewRect.Top)
			out.Init(OldRect.Left, OldRect.Bottom, OldRect.Right, NewRect.Top);
		else
			out.Init(OldRect.Left, NewRect.Bottom, OldRect.Right, OldRect.Top);
	}
}

void uiTabForm::OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
{
//	printx("---> uiTabForm::OnMouseMove client pos x:%d, y:%d.\n", x, y);
	if (!m_TotalPane)
		return;

	INT NewHighlightIndex = -1;
	for (INT i = 0; i < m_TotalPane; ++i)
	{
		stPaneInfo *pInfo = GetPaneInfo(i);
		if (!pInfo->Rect.IsPointIn(x, y))
			continue;
		NewHighlightIndex = i;
		break;
	}

	if (m_bDraggingTab)
	{
		while (NewHighlightIndex != -1 && NewHighlightIndex != m_ActiveIndex)
		{
			if (m_OldTabRect.IsValidRect())
			{
				uiRect BufRect;
				GetBufferRect(m_OldTabRect, m_PaneInfoArray[m_ActiveIndex].Rect, BufRect);
				if (BufRect.IsPointIn(x, y))
					break;
			}

			m_OldTabRect = m_PaneInfoArray[m_ActiveIndex].Rect;
			iter_swap(m_PaneInfoArray.begin() + NewHighlightIndex, m_PaneInfoArray.begin() + m_ActiveIndex);
			UpdateTabsRect();

			if (!m_PaneInfoArray[m_ActiveIndex].Rect.IsPointIn(x, y))
				m_OldTabRect.Reset();

			RedrawTabs(NewHighlightIndex, m_ActiveIndex);
			m_HighlightIndex = m_ActiveIndex = NewHighlightIndex;
			break;
		}
		return;
	}

	if (NewHighlightIndex != m_HighlightIndex)
	{
		if (NewHighlightIndex != -1)
			RedrawTabs(NewHighlightIndex, m_HighlightIndex);
		else if (m_HighlightIndex != -1)
			RedrawTabs(m_HighlightIndex, NewHighlightIndex);
		m_HighlightIndex = NewHighlightIndex;
	}
}

void uiTabForm::OnSize(UINT nNewWidth, UINT nNewHeight)
{
	Layout();
	BOOL bHide = !m_PaneRegion.IsValidRect();

	for (INT i = 0; i < m_TotalPane; ++i)
	{
		if (bHide)
		{
			GetPaneInfo(i)->pForm->Move(-5, -5); // Move to somewhere that system will clip it.
			GetPaneInfo(i)->pForm->Size(1, 1);
		}
		else
		{
			GetPaneInfo(i)->pForm->Move(m_PaneRegion.Left, m_PaneRegion.Top);
			GetPaneInfo(i)->pForm->Size(m_PaneRegion.Width(), m_PaneRegion.Height());
		}
	}
//	RedrawForm();
}

void uiTabForm::OnPaint(uiDrawer* pDrawer)
{
	uiRect rect = GetClientRect(), tRect;
	pDrawer->FillRect(rect, RGB(128, 128, 200));

	if (m_TabsRegion.IsEmpty())
		return;

	stPaneInfo *pPaneInfo;
	for (INT i = 0; i < m_TotalPane; ++i)
	{
		pPaneInfo = GetPaneInfo(i);

		tRect = GetPaneInfo(i)->Rect;
		tRect.Inflate(-1, -1, -1, -1);

	//	pDrawer->FillRect(tRect, (m_HighlightIndex == i) ? RGB(220, 220, 220) : RGB(200, 200, 200));

		if (m_ActiveIndex == i)
			pDrawer->FillRect(tRect, m_ColorActive);
		else
			pDrawer->FillRect(tRect, (m_HighlightIndex == i) ? m_ColorHover : m_ColorDefault);

		pDrawer->DrawText(pPaneInfo->pForm->GetTitle(), tRect, DT_CENTER);
	}
}


