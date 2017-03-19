

#pragma once


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

		rect.Inflate(-2, -2, -2, -2);
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
		uiPoint pt(x, y);
		ClientToScreen(pt);
		printx("OnMouseMove client pos x:%d, y:%d. Screen pos x:%d, y:%d\n", x, y, pt.x, pt.y);
		POINT p;
		if (::GetCursorPos(&p))
			printx("Real screen pos - x:%d, y:%d\n", p.x, p.y);
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



};



enum STATIC_CONTROL_PROPERTY
{
	SCP_NONE = 0,

	SCP_SHOW_FRAME = 0x01,
};

class uiStatic : public uiControl
{
public:

	void SetProperty()
	{
	}

	void OnPaint(uiDrawer* pDrawer)
	{
		//	printx("---> uiButton2::OnPaint\n");
		uiRect rect = GetClientRect();
		const INT size = 3;


		rect.Inflate(-1, -1, -1, -1);
		//	rect.Inflate(10, 10);
	//	pDrawer->DrawText(GetName(), rect, );
	}





};


