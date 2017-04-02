

#include "stdafx.h"
#include "uiForm.h"
#include "uiDrawer.h"
#include "UnitTest.h"
#include "uiControl.h"


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

	//	TimerStart(1, 1000 / m_ii.DispRate, -1, nullptr);
		return TRUE;
	}

	//void OnTimer(stTimerInfo* ti) override
	//{
	//	if (++m_CurFrame == m_ii.TotalFrame)
	//		m_CurFrame = 0;
	//	RedrawForm(&uiRect(m_ii.Width, m_ii.Height));
	//}

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


void GUITest()
{
	uiFormBase *pForm = new uiCursorDrawForm;

	pForm->Create(nullptr, 0, 0, 200, 100, FCF_CENTER);

}

void uiImageTest()
{
	BOOL bRet;
	uiImage img1, img2, FailedImg;
	ASSERT(img1 == img2); // default behavior of empty shared_ptr

//	img1.LoadFromFile(_T("R:\\"));

	bRet = FailedImg.LoadFromFile(_T("NoFile.bmp"));
	ASSERT(!bRet);
}

void uiStringTest()
{
	uiString us1(_T("Abc")), us2;
	ASSERT(us1.Length() == 3);
	us1 = nullptr;
	ASSERT(us1.Length() == 0);

	const TCHAR* pStr1 = _T("Us1"), * pStr2 = _T("Us2");
	us1 = pStr1;
	us2 = pStr2;
	us1 = std::move(us2);
	ASSERT(us2.Length() == 0);
	ASSERT(us2 == _T(""));
	ASSERT(us1.CmpRight(3, pStr2));
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


