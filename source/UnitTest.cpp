

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

	BOOL OnCreate() override
	{
		SetHeaderBar(_T("Draw cursor test"));
		m_AniCursor.LoadFromFile(_T("r:\\img\\wait.ani"));

		m_pIV = new uiImageViewer;
		m_pIV->Create(this, 0, 0, 28, 28);

		m_CurFrame = 0;

		uiImage::stImageInfo ii;
		m_AniCursor.GetInfo(ii);

		TimerStart(1, 1000, -1, nullptr);

	//	TimerStart(1, 1000 / m_ii.DispRate, -1, nullptr);
		return TRUE;
	}

	void OnTimer(stTimerInfo* ti) override
	{
	//	if (++m_CurFrame == m_ii.TotalFrame)
	//		m_CurFrame = 0;
	//	RedrawForm(&uiRect(m_ii.Width, m_ii.Height));

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


void GUITest()
{
	uiFormBase* pForm = new uiCursorDrawForm;
	pForm->Create(nullptr, 0, 0, 200, 100, FCF_CENTER);

	uiFormBase* pButton = new uiButtonWnd;
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


