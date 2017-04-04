

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


class CTBase
{
public:

	CTBase() :i(0), j(0), k(0) {}
	~CTBase() {}

	INT i, j, k;

};

template<typename T>
class CTypeT : public T
{
public:

	void DoSomething() { Print(); }

};


class CVInterface : public CTBase
{
public:

	virtual void Print() = 0;

};

//#define NO_VTABLE_LOOKUP

#ifdef NO_VTABLE_LOOKUP
	#define _BASE_TYPE CTypeT<CTBase>
//	#define DRAWER_BASE_TYPE uiDrawerBase
#else
	#define _BASE_TYPE CTypeT<CVInterface>
	typedef uiDrawerT<uiDrawerVInterface> uiDrawer;
	#define OVERRIDE override
#endif

class MyType : public _BASE_TYPE
{
public:
	void Print() { printx("---> MyType::Print\n"); }
};
class MyType2 : public _BASE_TYPE
{
public:
	void Print() { printx("---> MyType2::Print\n"); }
};


MyType mt;
MyType2 mt2;
void SetInterface(_BASE_TYPE*& pI)
{
#ifndef NO_VTABLE_LOOKUP
	static INT i = 0;
	if (i % 2 == 0)
		pI = &mt;
	else
		pI = &mt2;
	++i;
#endif
}

void GUITest()
{
	uiFormBase *pForm = new uiCursorDrawForm;
	pForm->Create(nullptr, 0, 0, 200, 100, FCF_CENTER);

	printx("Sizeof MyType: %d\n", sizeof(MyType));

#ifndef NO_VTABLE_LOOKUP
	_BASE_TYPE* pI;
	for (INT i = 0; i < 5; ++i)
	{
		SetInterface(pI);
		pI->DoSomething();
	}
#else
	mt.Print();
#endif

	printx("\n\n\n");
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


