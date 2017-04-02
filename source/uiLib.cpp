// uiLib.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "uiLib.h"

#include "uiForm.h"
#include "uiMsWin.h"
#include "UnitTest.h"


#define MAX_LOADSTRING 100


// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


class uiDraggableButton : public uiButton
{
public:

	void OnMouseBtnDown(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiDraggableButton::OnMouseBtnDown: %d %d %d\n", KeyType, x, y);

		uiPoint pt(x, y);
		ClientToWindow(pt);
		printx("ClientToWindow %d: %d\n", pt.x, pt.y);
		switch(KeyType)
		{
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
		printx("---> uiDraggableButton::OnMouseBtnUp: %d %d %d\n", KeyType, x, y);
		switch (KeyType)
		{
		case MKT_MIDDLE:
			break;
		case MKT_RIGHT:
	//		ReleaseCapture();
			break;
		}
	}

	void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> uiDraggableButton::OnMouseBtnDbClk: %d %d %d\n", KeyType, x, y);
	//	SetCapture();
	}
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
	{
		printx("---> uiDraggableButton::OnMouseMove x: %d, y: %d\n", x, y);

		uiPoint pt(x, y);
		ClientToScreen(pt);
		printx("OnMouseMove client pos x:%d, y:%d. Screen pos x:%d, y:%d\n", x, y, pt.x, pt.y);
		POINT p;
		if (::GetCursorPos(&p))
			printx("Real screen pos - x:%d, y:%d\n", p.x, p.y);

	}


};

class IDerivedAC : public IAreaCursor
{
public:

	uiImage GetCursorImage(uiFormBase *pForm, INT csX, INT csY, uiFormBase::CLIENT_AREA_TYPE cat) override
	{
		uiRect crect = pForm->GetClientRect();

		if (csX > crect.Width() / 2)
			return m_cur1;
		else
			return m_cur2;
	}

	IDerivedAC()
	{
	//	m_cur1.LoadCursor(IDR_ANI_CURSOR, _T("ANI_CURSOR"));
	//	m_cur2.LoadCursor(IDR_ANI_CURSOR2, _T("ANI_CURSOR"));

		m_cur1.LoadFromFile(_T("R:\\uiLib\\busy.ani"));
		m_cur2.LoadFromFile(_T("R:\\uiLib\\Working In Background.ani"));

	}

	uiImage m_cur1, m_cur2;
};

class CTabFormContext1 : public uiFormBase, public IDerivedAC
{
public:

	BOOL OnCreate()
	{
		SetName(_T("Tab Pane No. 1"));
		return TRUE;
	}
	void OnPaint(uiDrawer* pDrawer)
	{
		printx("---> CTabFormContext1::OnPaint\n");
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(100, 200, 0));
		pDrawer->DrawText(_T("Context 1"), rect, DT_CENTER);
	}
	void OnMouseEnter(INT x, INT y)
	{
		printx("---> CTabFormContext1::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> CTabFormContext1::OnMouseLeave\n");
	}
	void OnSize(UINT nw, UINT nh) override
	{
		printx("---> CTabFormContext1::OnSize nw: %d, nh: %d\n", nw, nh);
	}

	IMPLEMENT_INTERFACE_COI(IAreaCursor)

};

class CTabFormContext2 : public uiFormBase
{
public:

	BOOL OnCreate()
	{
		SetName(_T("Tab Pane No. 2"));
		return TRUE;
	}
	void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(10, 125, 175));
		pDrawer->DrawText(_T("Context 2"), rect, DT_CENTER);
	}
	void OnMouseEnter(INT x, INT y)
	{
		printx("---> CTabFormContext2::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> CTabFormContext2::OnMouseLeave\n");
	}

};

class CTabFormContext3 : public uiFormBase
{
public:

	BOOL OnCreate()
	{
		SetName(_T("Tab Pane No. 3"));

		uiButton *pButton = new uiDraggableButton;
		pButton->Create(this, 10, 10, 80, 80);

		uiButton *pButton2 = new uiDraggableButton;
		pButton2->Create(pButton, 10, 10, 30, 30);

		return TRUE;
	}
	void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(199, 225, 175));
		pDrawer->DrawText(_T("Context 3"), rect, DT_CENTER);
	}
	void OnMouseEnter(INT x, INT y)
	{
		printx("---> CTabFormContext3::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> CTabFormContext3::OnMouseLeave\n");
	}

};

class CFormEx : public uiForm
{
public:

	CFormEx() = default;
	~CFormEx() = default;

	BOOL OnCreate()
	{
		uiForm::OnCreate();
		printx("---> CFormEx::OnCreate\n");

		SetHeaderBar(_T("test child form"));

		m_pButton = new uiButton2;
		m_pButton->Create(this, 10, 10, 80, 80, FCF_CENTER);
		Bind(m_pButton->GetID());

		m_pSubForm = new uiForm;
		m_pSubForm->Create(this, 20, 20, 110, 35, FCF_CENTER);
	//	m_pSubForm->Create(this, 20, 20, 110, 60);
		m_pSubForm->SetHeaderBar(_T("test child form"));

		uiButton *pBtn = new uiDraggableButton;
		pBtn->Create(m_pSubForm, 10, 10, 80, 80, FCF_CENTER);

		return TRUE;
	}
	void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(100, 125, 125));
	}

	void OnCommand(INT id, BOOL &bDone)
	{
		ASSERT(id == m_pButton->GetID());
		CaretShow(TRUE, 5, 5, 5, 15);
	}
	
	void OnMouseMove(INT x, INT y, MOVE_DIRECTION mmd)
	{
	}
	void OnMouseBtnDbClk(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> CFormEx::OnMouseBtnDbClk: %d %d %d\n", KeyType, x, y);
		SetCapture();
	}
	void OnMouseBtnUp(MOUSE_KEY_TYPE KeyType, INT x, INT y) override
	{
		printx("---> CFormEx::OnMouseBtnUp: %d %d %d\n", KeyType, x, y);
	//	ReleaseCapture();
	}
	void OnMouseEnter(INT x, INT y)
	{
		printx("--->CFormEx::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("--->CFormEx::OnMouseLeave\n");
	}


protected:

	uiButton *m_pButton;
	uiForm *m_pSubForm;


};

class CTimerTestForm : public uiForm
{
public:

	CTimerTestForm() = default;
	~CTimerTestForm() = default;

	BOOL OnCreate()
	{
		m_pButton = new uiButton;
		m_pButton->Create(this, 30, 100, 120, 30);

		Bind(m_pButton->GetID());

		TimerStart(555, 1000, -1, nullptr);

		return TRUE;
	}

	void OnCommand(INT id, BOOL &bDone)
	{
		TimerStop(555);
	}
	void OnTimer(stTimerInfo* ti)
	{
		printx("---> CTimerTestForm::OnTimer ID: %d\n", ti->id);
		if (ti->id == 0)
			MoveByOffset(0, -20);
		else if (ti->id == 1)
			MoveByOffset(20, 0);
	}

	uiButton *m_pButton, *m_pButton2;

};

class CFormEx2 : public uiForm
{
public:

	CFormEx2() = default;
	~CFormEx2() = default;

	BOOL OnCreate()
	{
		uiForm::OnCreate();
		printx("---> CFormEx2::OnCreate\n");

		m_pTabForm = new uiTabForm;
	
		m_pTabForm->Create(this, 5, 5, 100, 150, FCF_CENTER);
		m_pTabForm->SetMargin(3, 3, 3, 3);
		m_pTabForm->SetProperty(uiTabForm::TFF_TAB_TOP | uiTabForm::TFF_FORCE_SHOW_TAB | uiTabForm::TFF_DRAGGABLE_TAB);
	//	m_pTabForm->SetProperty(uiTabForm::TFF_TAB_BOTTOM | uiTabForm::TFF_FORCE_SHOW_TAB | uiTabForm::TFF_DRAGGABLE_TAB);
	//	m_pTabForm->SetProperty(uiTabForm::TFF_TAB_LEFT | uiTabForm::TFF_FORCE_SHOW_TAB | uiTabForm::TFF_DRAGGABLE_TAB);
	//	m_pTabForm->SetProperty(uiTabForm::TFF_TAB_RIGHT | uiTabForm::TFF_FORCE_SHOW_TAB | uiTabForm::TFF_DRAGGABLE_TAB);

		m_pTabForm->SetHeaderBar(_T("test child form"));

	//	uiFormBase *pBase = nullptr;
	//	m_pTabForm->AddPane(pBase, -1);

		m_pButton = new uiButton;
		m_pButton->Create(this, 60, 400, 50, 50);
		Bind(m_pButton->GetID());

		m_pButton2 = new uiButton2;
		m_pButton2->Create(this, 125, 400, 50, 50);
		Bind(m_pButton2->GetID());

		VERIFY(SetTimer(0, 1000, -1, nullptr));
	//	VERIFY(SetTimer(1, 2000, 3, nullptr));

		return TRUE;
	}

	void OnTimer(stTimerInfo* ti)
	{
		printx("---> CFormEx2::OnTimer ID: %d\n", ti->id);
		if (ti->id == 0)
			MoveByOffset(0, -20);
		else if (ti->id == 1)
			MoveByOffset(20, 0);
	}

	virtual void OnCommand(INT id, BOOL &bDone)
	{
		printx("---> CFormEx2::OnCommand ID: %d\n", id);
		static INT CalledCount = 0;

		if (id == m_pButton->GetID())
		{
			uiFormBase *pBaseForm = nullptr;
			if (CalledCount % 3 == 0)
				pBaseForm = new CTabFormContext1;
			else if (CalledCount % 3 == 1)
				pBaseForm = new CTabFormContext2;
			else
				pBaseForm = new CTabFormContext3;

			pBaseForm->Create(m_pTabForm, 0, 0, 0, 0);
			m_pTabForm->AddPane(pBaseForm, -1, TRUE);

			++CalledCount;
			bDone = TRUE;
		}
		else if (id == m_pButton2->GetID())
		{
	//		m_pTabForm->RedrawForm();
			m_pTabForm->DeletePane(1);
		}

		if (id == uiID_CLOSE)
		{
			printx("OnClose() hooked!\n");
			bDone = FALSE;
		}
	}

	virtual void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(200, 200, 200));
	}


protected:

	uiButton *m_pButton, *m_pButton2;
	uiTabForm *m_pTabForm;


};


class CMyForm : public uiForm
{
public:

	CMyForm() = default;
	~CMyForm() = default;

	BOOL OnCreate()
	{
		uiForm::OnCreate();

		printx("---> CMyForm::OnCreate\n");

		SetHeaderBar(_T("test"));
	//	SetMenuBar(nullptr);

//*
		m_pButton = new uiButton;
		m_pButton->Create(this, 150, 60, 50, 50);

		m_pButton2 = new uiButton2;
		m_pButton2->Create(this, 150, 120, 50, 50);

		m_pSubForm = new CFormEx;
		m_pSubForm->Create(this, 150, 150, 170, 200);
		//m_pSubForm->MoveToCenter();

		m_pButton3 = new uiButton;
	//	m_pButton3->Create(this, 250, 120, 50, 50);
		m_pButton3->Create(this, 150, 120, 50, 50);

		m_pButton4 = new uiButton;
		m_pButton4->Create(this, 150, 200, 50, 50);

		m_pButton5 = new uiDraggableButton;
		m_pButton5->Create(this, 250, 200, 50, 50);

		Bind(m_pButton->GetID());
		Bind(m_pButton2->GetID());
		Bind(m_pButton3->GetID());
		Bind(m_pButton4->GetID());
		Bind(uiID_CLOSE); //*/


		SetBorder(5, uiForm::FBF_ALL); // FBF_LEFT FBF_RIGHT FBF_TOP FBF_BOTTOM
		SetDraggableThickness(50, 10, 10, 10);

		uiTabForm* m_pTabForm = new uiTabForm;
		m_pTabForm->Create(m_pSubForm, 5, 5, 100, 150, FCF_CENTER);
		m_pTabForm->SetMargin(3, 3, 3, 3);
		m_pTabForm->SetProperty(uiTabForm::TFF_TAB_RIGHT | uiTabForm::TFF_FORCE_SHOW_TAB | uiTabForm::TFF_DRAGGABLE_TAB);
		m_pTabForm->SetHeaderBar(_T("test child form"));

	//	TimerStart(0, 1000, -1, nullptr);


		VERIFY(m_menu.CreatePopupMenu());
		VERIFY(m_menu.InsertItem(_T("Test item 1"), 2, 1));
		m_menu.ChangeStyle();

		return TRUE;
	}

	virtual void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		switch (KeyType)
		{
		case MKT_LEFT:
			printx("---> OnMouseBtnClk\n");
	//		Size(50, 50);
			{
				POINT pt;
				::GetCursorPos(&pt);

				uiPoint p(x, y);
				ClientToScreen(p);

				printx("real x: %d, y: %d - x: %d, y: %d\n", pt.x, pt.y, p.x, p.y);
			}
			break;

		case MKT_RIGHT:
			printx("---> MKT_RIGHT!\n");
			{
				POINT pt;
				::GetCursorPos(&pt);

				uiPoint p(x, y);
				ClientToScreen(p);

				printx("real x: %d, y: %d - x: %d, y: %d\n", pt.x, pt.y, p.x, p.y);
				uiWindow* pWnd = GetBaseWnd();
				VERIFY(m_menu.Popup((HWND)pWnd->GetHandle(), p.x, p.y + 10));
			}
			break;
		}
	}

	void OnTimer(stTimerInfo* ti)
	{
	//	printx("---> CMyForm::OnTimer ID: %d\n", ti->id);
	//	RedrawForm();
	//	MoveByOffset(0, 20);

	//*/
		POINT wpt;
		::GetCursorPos(&wpt);
		
		HWND hWnd = ::WindowFromPoint(wpt);
		printx("---> CMyForm::OnTimer HWND: %p\n", hWnd);
	//*/
	}

	virtual void OnCommand(INT id, BOOL &bDone)
	{
		printx("---> CMyForm::OnCommand ID: %d\n", id);

		if (id == m_pButton->GetID())
		{
			m_pSubForm->SetBorder(10, uiForm::FBF_RIGHT); // FBF_LEFT FBF_RIGHT FBF_TOP FBF_BOTTOM
			m_pSubForm->SetDraggableThickness(10, 10, 10, 10);

			SetBorder(-1, 0, -1, -1, uiForm::FBF_ALL); // FBF_LEFT FBF_RIGHT FBF_TOP FBF_BOTTOM

		//	uiForm *pToolForm = new uiForm;
		//	pToolForm->Create(this, 100, 100, 200, 200, FCF_TOOL);
		//	pToolForm->SetHeaderBar(_T("test"));
		}
		else if (id == m_pButton2->GetID())
		{
			printx("Button 2 was clicked!\n");

			uiForm *pForm = new CFormEx2;
			pForm->Create(this, 100, 100, 200, 280, FCF_TOOL | FCF_CENTER);
			pForm->SetHeaderBar(_T("test"));
		}
		else if (id == m_pButton3->GetID())
		{
			printx("Button 3 was clicked!\n");

			m_pButton3->MoveByOffset(60, 60);
		//	CaretShow(TRUE, 0, 0, 5, 15);
		}
		else if (id == m_pButton4->GetID())
		{
			printx("Button 4 was clicked!\n");

			uiForm *pForm = new CTimerTestForm;
			pForm->Create(this, 100, 100, 200, 280, FCF_TOOL | FCF_CENTER);
			pForm->SetHeaderBar(_T("test"));
		}

		if (id == uiID_CLOSE)
		{
			printx("OnClose() hooked!\n");
			bDone = FALSE;
		}
	}

	void OnSize(UINT nw, UINT nh) override
	{
		printx("---> CMyForm::OnSize nw: %d, nh: %d\n", nw, nh);

		uiRect rect1 = GetFrameRect();
		uiRect rect2 = GetClientRect();
		printx("---> CMyForm::OnSize nw: %d, nh: %d\n", nw, nh);
	}


protected:

	uiButton *m_pButton, *m_pButton2, *m_pButton3, *m_pButton4, *m_pButton5;
	uiForm *m_pSubForm;

	uiWinMenu m_menu;


};

class uiInterfaceTestForm1 : public uiFormBase // Pure compostion.
{
public:

	uiInterfaceTestForm1()
	{
		printx("---> uiInterfaceTestForm1::uiInterfaceTestForm1\n");
	}

	IMPLEMENT_INTERFACE(IAreaCursor, IDerivedAC)

};

class uiInterfaceTestForm2 : public uiFormBase, public IDerivedAC // Composition over inheritance.
{
public:

	uiInterfaceTestForm2()
	{
		printx("---> uiInterfaceTestForm2::uiInterfaceTestForm2\n");
	}

	IAreaCursor* GetIAreaCursor() override { return dynamic_cast<IAreaCursor*>(this); }

};

void InterfaceTest()
{
	IDerivedAC *pI = new IDerivedAC;
	uiInterfaceTestForm1 *pTF1 = new uiInterfaceTestForm1;
	uiInterfaceTestForm2 *pTF2 = new uiInterfaceTestForm2;

	printx("sizeof uiFormBase: %d Bytes\n", sizeof uiFormBase);
	printx("sizeof IDerivedAC: %d Bytes\n", sizeof IDerivedAC);
	printx("sizeof uiInterfaceTestForm1: %d Bytes\n", sizeof uiInterfaceTestForm1);
	printx("sizeof uiInterfaceTestForm2: %d Bytes\n", sizeof uiInterfaceTestForm2);

//	pTF1->GetIAreaCursor()->GetCursorImage(nullptr, 0, 0, uiFormBase::CAT_CLIENT);
//	pTF2->GetIAreaCursor()->GetCursorImage(nullptr, 0, 0, uiFormBase::CAT_CLIENT);

	delete pI;
	delete pTF1;
	delete pTF2;
}

void FontTest()
{
	uiFont font, font2, font3;
	font.Create(_T("Arial"), 20, 10);

//	uiFont *ptr = new uiFont;
//	new (&font) uiFont();

	const int i = sizeof(uiFont);

	font2 = font;

	font.~uiFont();
//	font2.~uiFont();

	font2 = font3;

	font3.~uiFont();

	for (UINT i = 0; i < 5; ++i)
	{
		uiImage img, img2, img3, img4, img5, img6, img7, img8, a, b, c, d, e;

		img.LoadFromFile(_T("R:\\test.cur"));

		img2 = (uiImage&&)img;
		img3 = img2;

		img.LoadFromFile(_T("R:\\test2.cur"));
		img2 = (uiImage&&)img;

		img4.LoadFromFile(_T("R:\\test4.cur"));
		img2.LoadFromFile(_T("R:\\test2.cur"));
		img6.LoadFromFile(_T("R:\\a1.cur"));
		img7.LoadFromFile(_T("R:\\z.cur"));
		img5.LoadFromFile(_T("R:\\test5.cur"));
		img3.LoadFromFile(_T("R:\\test3.cur"));

		d.LoadCursorRes(IDC_CURSOR1);
		d.LoadCursorRes(IDC_CURSOR1);
		d.LoadCursorRes(IDR_ANI_CURSOR, _T("ANI_CURSOR"));
		b.LoadCursorRes(IDR_ANI_CURSOR, _T("ANI_CURSOR"));
		c.LoadCursorRes(IDR_ANI_CURSOR2, _T("ANI_CURSOR"));
	}
//	c.LoadCursor(_T("R:\\wait.ani"));

//	a = img3;

//	b.LoadCursor(IDR_ANI_CURSOR, _T("ANI_CURSOR"));
//	c.LoadCursor(IDR_ANI_CURSOR2, _T("ANI_CURSOR"));
	uiImage img;
//	img.LoadCursor(IDR_ANI_CURSOR2, _T("ANI_CURSOR"));
	img.LoadFromFile(_T("R:\\test.Cur"));
//	::SetCursor((HCURSOR)img.GetHandle());

//	d.LoadCursor(IDC_CURSOR1);

//	img.Initial(uiImage::IT_BMP);
	font3.~uiFont();

	uiString s1(_T("test"));
	uiString s2(s1);

	s1 = _T("");
	INT len = s1.Length();

}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UTXLibraryInit();

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	void *pAddr = new int[30];

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_UILIB, szWindowClass, MAX_LOADSTRING);

	BOOL m_bHasConsole = FALSE;
#ifdef _DEBUG
	m_bHasConsole = AllocConsole();
	printx("Console initialized!\n");
#endif

	FontTest();
	InterfaceTest();


	uiString a, b;
	a = _T("Unicode string\n");
	
	printx("sizeof std::shared_ptr<>: %d Bytes\n", sizeof std::shared_ptr<uiString>);
	printx("sizeof uiSideDockableFrame: %d Bytes\n", sizeof uiSideDockableFrame);
	printx("sizeof CMyForm: %d Bytes\n", sizeof CMyForm);
	printx("sizeof CSimpleList: %d Bytes\n", sizeof UTX::CSimpleList);
	printx("sizeof std::vector<UINT>: %d Bytes\n", sizeof std::vector<UINT>);
	printx("sizeof uiWindow: %d Bytes\n", sizeof uiWindow);
	printx("sizeof uiFormBase: %d Bytes\n", sizeof uiFormBase);
	printx("sizeof uiForm: %d Bytes\n", sizeof uiForm);


	CMyForm* pForm = new CMyForm;
	pForm->Create(nullptr, 150, 150, 600, 400, FCF_CENTER);

//	pForm->Create(nullptr, 150, 150, 600, 400, FCF_INVISIBLE);

//	pForm = new CMyForm;
//	pForm->Create(nullptr, 150, 150, 600, 400);

//	pForm->Show(FSM_SHOW);
//	pForm->Move(1000, 680);

	UnitTestMain();

	uiMonitorEvent();

/*
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) // Main message loop
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	} //*/

	UTXLibraryEnd();

	return 0;
//	return (int) msg.wParam;
}


