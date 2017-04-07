

#pragma once


class uiControl;


//#define NAME

class UI_INTERFACE IControlHooker
{
public:

	enum HOOK_METHOD_FLAGS
	{
		HMF_ONCREATE       = 0x01,
		HMF_ONPAINT        = 0x01 << 1,
		HMF_ONCONTROLPAINT = 0x01 << 2,

		HMF_NONE = 0,
		HMF_ALL  = 0xFFFFFFFF,
	};

	virtual BOOL OnCreate(uiControl* pCtrl) { DEBUG_CHECK(ShowWarnString()); return TRUE; }
	virtual void OnPaint(uiControl* pCtrl, uiDrawer* pDrawer) { DEBUG_CHECK(ShowWarnString()); } // draw control frame
	virtual void OnControlPaint(uiControl* pCtrl, uiDrawer* pDrawer, const uiRect& rect) { DEBUG_CHECK(ShowWarnString()); }

	virtual BOOL IsMethodHooked(HOOK_METHOD_FLAGS hmf) = 0;


#ifdef _DEBUG
//	virtual CHAR* GetTypeName() const = 0;
	void Msg(const CHAR* TypeName, const CHAR* IName)
	{
		printx("%s::%s wasn't overrided!\n", TypeName, IName);
	}
	void ShowWarnString()
	{

	}
#endif


};

IMPLEMENT_ENUM_FLAG(IControlHooker::HOOK_METHOD_FLAGS);


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


	BOOL OnCreate() override final
	{
		if (m_ID == uiID_INVALID)
			m_ID = uiGetID();

		IControlHooker *pICH = GetIControlHooker();
		if (pICH != nullptr && pICH->IsMethodHooked(IControlHooker::HMF_ONCREATE))
			return pICH->OnCreate(this);

		return TRUE;
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

	FORM_CLASS GetClass() const override { return FC_CONTROL; }

	void OnPaint(uiDrawer* pDrawer) override //final // draw frame if need.
	{
		IControlHooker *pICH = GetIControlHooker();
		if (pICH != nullptr && pICH->IsMethodHooked(IControlHooker::HMF_ONPAINT))
			pICH->OnPaint(this, pDrawer);

		if (pICH != nullptr && pICH->IsMethodHooked(IControlHooker::HMF_ONCONTROLPAINT))
			pICH->OnPaint(this, pDrawer);
		else
			OnControlPaint(pDrawer, GetFrameRect());
	} 

	virtual void OnControlPaint(uiDrawer* pDrawer, const uiRect& client) {}
	virtual void SizeToContent() {}

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


	DECLARE_INTERFACE(IControlHooker);


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

	void OnControlPaint(uiDrawer* pDrawer, const uiRect& rect)
	{
		//	printx("---> uiButton::OnPaint\n");

		if (IsMouseHovering())
			pDrawer->FillRect(rect, RGB(230, 200, 230));
		else
			pDrawer->FillRect(rect, RGB(155, 200, 155));

		pDrawer->FillRect(rect.InflateRV(-2, -2, -2, -2), RGB(30, 50, 30));
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

	void OnControlPaint(uiDrawer* pDrawer, const uiRect& rect)
	{
		//	printx("---> uiButton2::OnPaint\n");
		const INT size = 3;

		if (IsMouseHovering())
			pDrawer->RoundRect(rect, RGB(230, 200, 230), size, size);
		else
			pDrawer->RoundRect(rect, RGB(155, 200, 155), size, size);

		pDrawer->FillRect(rect.InflateRV(-1, -1, -1, -1), RGB(30, 50, 30));
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

	void OnControlPaint(uiDrawer* pDrawer, const uiRect& rect) override
//	void OnPaint(uiDrawer* pDrawer)
	{
		//	printx("---> uiButton2::OnPaint\n");
		const INT size = 3;


	//	rect.Inflate(-1, -1, -1, -1);
		//	rect.Inflate(10, 10);
	//	pDrawer->DrawText(GetName(), rect, );
	}


	void OnPaint(uiDrawer* pDrawer) override
	{
	//	pDrawer->Text();

	}


protected:

	uiString m_Text;


};


class IAniImage : public IControlHooker
{
public:

	IAniImage()
	:m_pCtrl(nullptr)
	{
	}

	INLINE BOOL SetImg(const uiImage& img, BOOL bSizeCtrl)
	{
		if (!(m_Img = img).IsValid())
			return FALSE;

		if (m_pCtrl != nullptr && m_pCtrl->IsCreated())
		{
		//	uiSize size = m_Img.GetSize();
		//	if (bSizeCtrl)
		//		m_pCtrl->Size(size.iWidth, size.iHeight);
			m_pCtrl->RedrawForm();
		}

		return TRUE;
	}

	void OnControlPaint(uiControl* pCtrl, uiDrawer* pDrawer, const uiRect& rect)
	{
		stDrawImageParam param(0, 0, 0, 0);
		pDrawer->DrawImage(m_Img, param);
	}

	BOOL IsMethodHooked(HOOK_METHOD_FLAGS hmf) override
	{
		if (hmf & HMF_ONCREATE)
			return TRUE;
		if (hmf & HMF_ONCONTROLPAINT)
			return TRUE;
		return FALSE;
	}


protected:

	uiControl* m_pCtrl;
	uiImage m_Img;


};


class uiImageViewer : public uiControl, public IAniImage
{
	IMPLEMENT_INTERFACE_COI(IControlHooker)
};


