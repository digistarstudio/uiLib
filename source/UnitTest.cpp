

#include "stdafx.h"
#include "uiForm.h"
#include "uiDrawer.h"
#include "UnitTest.h"
#include "uiControl.h"
#include "MsWinHelper.h"
#include "uiMsWin.h"


class uiCursorDrawForm : public uiForm
{
public:

	uiCursorDrawForm() = default;
	~uiCursorDrawForm() = default;


	void OnMouseEnter(INT x, INT y)
	{
		printx("---> uiCursorDrawForm::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> uiCursorDrawForm::OnMouseLeave\n");
	}

	BOOL OnCreate() override
	{
		SetHeaderBar(_T("Draw cursor test"));
		m_AniCursor.LoadFromFile(_T("r:\\img\\wait.ani"));


		pBtn = new uiButton();
		pBtn->Create(this, 20, 20, 80, 15);

		m_pIV = new uiImageViewer;
		m_pIV->Create(this, 0, 0, 28, 28);

		m_CurFrame = 0;

		uiImage::stImageInfo ii;
		m_AniCursor.GetInfo(ii);

		TimerStart(1, 1000, -1, nullptr);

	//	TimerStart(1, 1000 / m_ii.DispRate, -1, nullptr);
		return TRUE;
	}

	void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
	//	uiFormBase* pOld = pBtn->SetCapture();

		switch (KeyType)
		{
		case MKT_LEFT:
			pBtn->SetCapture();
			break;
		case MKT_RIGHT:
			break;
		}
	//	if ()
	}
	void OnTimer(stTimerInfo* ti) override
	{
	//	if (++m_CurFrame == m_ii.TotalFrame)
	//		m_CurFrame = 0;
	//	RedrawForm(&uiRect(m_ii.Width, m_ii.Height));
	/*	if (GetKeyState(VK_F2) < 0)
		{
			pBtn->Close();
			return;
		} //*/
		if (GetKeyState(VK_F3) < 0)
		{
			uiFormBase* pOldForm = SetActive();
			if (pOldForm != nullptr)
			{
				printx(_T("Original active form: %s\n"), pOldForm->GetName());
			}
			else
			{
				printx(_T("Nothing happened!\n"));
			}
		}

		if (GetKeyState(VK_F4) < 0)
			pBtn->SetCapture();
		if (GetKeyState(VK_F5) < 0)
			pBtn->ReleaseCapture();

		if (GetKeyState(VK_F7) < 0)
			pBtn->MoveByOffset(5, 0);
		if (GetKeyState(VK_F8) < 0)
			pBtn->MoveByOffset(-5, 0);
	}

	//void OnPaint(uiDrawer* pDrawer)
	//{
	//	pDrawer->SetAniFrameIndex(m_CurFrame);
	//	pDrawer->DrawImage(m_AniCursor, 0, 0, 0, 0, SRCCOPY);
	//}


protected:

	UINT    m_CurFrame;
	uiImage m_AniCursor;

	uiImageViewer* m_pIV;
	uiButton* pBtn;

};

class uiButtonWnd : public uiButton
{
public:

	BOOL OnCreate() override
	{
		uiButton::OnCreate();
		Bind(GetID());
		return TRUE;
	}
	void OnCommand(INT_PTR id, BOOL &bDone) override
	{
		Close();
	}
	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiButtonWnd::OnMouseBtnDown: %d %d %d\n", KeyType, x, y);

		uiPoint pt(x, y);
		ClientToWindow(pt);
		printx("ClientToWindow %d: %d\n", pt.x, pt.y);
		switch (KeyType)
		{
		case MKT_LEFT:
			Close(); // StartDragging(MKT_LEFT, pt.x, pt.y);
			break;
		case MKT_MIDDLE:
			StartDragging(MKT_MIDDLE, pt.x, pt.y);
			break;
		case MKT_RIGHT:
			StartDragging(MKT_RIGHT, pt.x, pt.y);
			break;
		}
	}
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiButtonWnd::OnMouseBtnUp: %d %d %d\n", KeyType, x, y);
		switch (KeyType)
		{
		case MKT_MIDDLE:
			break;
		case MKT_RIGHT:
			//		ReleaseCapture();
			break;
		}
	}


};


class uiDropdownFormBase : public uiForm
{
public:


	FORM_CLASS GetClass() const override final
	{
		return FC_DROPDOWN_FORM;
	}

	MOUSE_ACTIVATE_RESULT OnMouseActivate() override // MAR_ACTIVATE, MAR_ACTIVATE_EAT, MAR_NO_ACTIVATE, MAR_NO_ACTIVATE_EAT
	{
		return MAR_NO_ACTIVATE;
	}
	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiDropdownForm::OnMouseBtnDown: %d %d %d\n", KeyType, x, y);

		switch (KeyType)
		{
		case MKT_LEFT:
			break;
		case MKT_MIDDLE:
			Close();
			break;
		case MKT_RIGHT:
			break;
		}
	}


};


class uiComboBox : public uiFormBase
{
public:

	enum
	{
		BTN_WIDTH = 18,
		FRAME_THICKNESS = 2,
	};


	uiComboBox()
	:m_pDDF(nullptr)
	{
	}

	void OnActivate(BOOL bActive) override
	{
		printx("---> uiComboBox::OnActivate Active: %d\n", bActive);
		uiFormBase::OnActivate(bActive);

		if (!bActive)
		{
			if (m_pDDF != nullptr)
			{
				m_pDDF->Close();
				m_pDDF = nullptr;
			}
		}
	}

	void OnKBFocus(BOOL bGet, uiFormBase* pForm) override
	{
		printx("---> uiComboBox::OnKBGetFocus. bGet: %d\n", bGet);

		if (bGet)
		{

		}
		else
		{
			if (GetKeyState(VK_F2) < 0)
			_CrtDbgBreak();
		}
	}

	void OnMouseEnter(INT x, INT y) override
	{
		RedrawForm();
	}
	void OnMouseLeave() override
	{
		RedrawForm();
	}

	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		uiPoint pt(x, y);
		ClientToWindow(pt);

		switch (KeyType)
		{
		case MKT_LEFT:
		//	new uiDropdownFormBase();

			break;

		case MKT_MIDDLE:
			StartDragging(MKT_MIDDLE, pt.x, pt.y);
			break;

		case MKT_RIGHT:
			StartDragging(MKT_RIGHT, pt.x, pt.y);
			break;
		}
	}
	void OnPaint(uiDrawer* pDrawer) override
	{
		uiRect rect = GetClientRect(), rect2 = rect.InflateRV(-FRAME_THICKNESS, -FRAME_THICKNESS, -FRAME_THICKNESS, -FRAME_THICKNESS);
		UINT FrameColor = IsMouseHovering() ? RGB(200, 200, 255) : RGB(200, 200, 200);
		
		pDrawer->FillRect(rect, FrameColor);
		pDrawer->FillRect(rect2, RGB(255, 255, 255));

		const uiFont& font = uiGetSysFont(SFT_SM_CAPTION);


		pDrawer->Text(uiString(_T("Test string")), rect, font);


		pDrawer->FillRect(BtnRect, FrameColor);
		pDrawer->Text(uiString(_T("¡¿")), BtnRect, font);
	}
	void UpdateLayout(const uiRect& rect)
	{
		BtnRect = rect.InflateRV(-FRAME_THICKNESS, -FRAME_THICKNESS, -FRAME_THICKNESS, -FRAME_THICKNESS);
		BtnRect.Left = BtnRect.Right - BTN_WIDTH;
	}
	void OnSize(UINT nNewWidth, UINT nNewHeight) override
	{
		uiRect rect(nNewWidth, nNewHeight);
		UpdateLayout(rect);
	}


protected:

	uiRect BtnRect;
	uiDropdownFormBase *m_pDDF;


};


class uiTestForm : public uiForm
{
public:

	BOOL OnCreate() override
	{
		SetHeaderBar(_T("Combobox Test"));
		uiFormBase* pCombo = new uiComboBox;
		pCombo->Create(this, 0, 0, 100, 23, FCF_CENTER);
		return TRUE;
	}


};


void GUITest()
{
	uiFormBase* pForm = new uiCursorDrawForm;
	pForm->Create(nullptr, 0, 0, 200, 100, FCF_CENTER);

	uiFormBase* pButton = new uiTestForm;
	pButton->Create(nullptr, 0, 0, 200, 100, FCF_CENTER);

}

void uiImageTest()
{
	BOOL bRet;
	uiImage img1, img2, FailedImg;
	ASSERT(img1 == img2); // default behavior of empty shared_ptr

	img1.LoadFromFile(_T("R:\\img\\tt.bmp"));

	uiImage img3(std::move(img1));

	bRet = FailedImg.LoadFromFile(_T("NoFile.bmp"));
	ASSERT(!bRet);
}

void uiStringTest()
{
	uiString us1(_T("Abc")), us2;
	ASSERT(us1.Length() == 3);
	us1 = nullptr;
	ASSERT(us1.Length() == 0);

	const TCHAR* pStr1 = _T("Us1"), *pStr2 = _T("Us2"), *pStr3 = _T("XYZ");
	us1 = pStr1;
	us2 = pStr2;
	us1 = std::move(us2);
	ASSERT(us2.Length() == 0);
	ASSERT(us2 == _T(""));
	ASSERT(us1.CmpRight(3, pStr2));

	us1 = pStr3;
	us2.MakeLower();

	TCHAR buf[6];
	CHAR  ABuf[10];
	INT iLen;

	ASSERT(uiString::GetBufferLength(_T("%d"), 123) == 4);
	ASSERT(uiString::GetBufferLength("%d", 123) == 4);

	iLen = uiString::Format(buf, _countof(buf), _T("%d"), 45678);
	iLen = uiString::Format(ABuf, _countof(ABuf), "%d", 45678);
}

void LogicalTest()
{
	uiImageTest();
	uiStringTest();
}

void UnitTestMain()
{
	LogicalTest();
	GUITest();
}


