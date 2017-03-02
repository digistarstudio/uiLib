// uiLib.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "uiLib.h"

#include "uiForm.h"
#include "uiMsWin.h"


#define MAX_LOADSTRING 100


// Global Variables:
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name


class CTabFormContext1 : public uiFormBase
{
public:

	void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(100, 200, 0));
		pDrawer->DrawText(_T("Context 1"), rect, DT_CENTER);
	}
	void OnMouseEnter()
	{
		printx("---> CTabFormContext1::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> CTabFormContext1::OnMouseLeave\n");
	}
};

class CTabFormContext2 : public uiFormBase
{
public:

	void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(10, 125, 175));
		pDrawer->DrawText(_T("Context 2"), rect, DT_CENTER);
	}
	void OnMouseEnter()
	{
		printx("---> CTabFormContext2::OnMouseEnter\n");
	}
	void OnMouseLeave()
	{
		printx("---> CTabFormContext2::OnMouseLeave\n");
	}

};



class CFormEx : public uiForm
{
public:

	CFormEx() = default;
	~CFormEx() = default;

	void OnCreate()
	{
		uiForm::OnCreate();
		printx("---> CMyForm::OnCreate\n");

		m_pSubForm = new uiForm;
		m_pSubForm->Create(this, 20, 20, 100, 60, FCF_VISIBLE);
		m_pSubForm->SetHeaderBar(_T("test child form"));
	}

	virtual void OnPaint(uiDrawer* pDrawer)
	{
		uiRect rect = GetClientRect();
		pDrawer->FillRect(rect, RGB(100, 125, 125));
	}


protected:

	uiButton *m_pButton;
	uiForm *m_pSubForm;


};

class CFormEx2 : public uiForm
{
public:

	CFormEx2() = default;
	~CFormEx2() = default;

	void OnCreate()
	{
		uiForm::OnCreate();
		printx("---> CFormEx2::OnCreate\n");

		m_pTabForm = new uiTabForm;
	//	m_pTabForm->Create(this, uiTabForm::TFF_TAB_TOP);
		m_pTabForm->Create(this, uiTabForm::TFF_TAB_BOTTOM);
	//	m_pTabForm->Create(this, uiTabForm::TFF_TAB_LEFT);
	//	m_pTabForm->Create(this, uiTabForm::TFF_TAB_RIGHT);
	//	m_pSubForm->SetHeaderBar(_T("test child form"));

	//	uiFormBase *pBase = nullptr;
	//	m_pTabForm->AddPane(pBase, -1);

		m_pButton = new uiButton;
		m_pButton->Create(this, 60, 400, 50, 50, FCF_VISIBLE);
		Bind(m_pButton->GetID());
	}

	virtual void OnCommand(INT id, BOOL &bDone)
	{
		printx("---> CFormEx2::OnCommand ID: %d\n", id);
		static INT CalledCount = 0;

		if (id == m_pButton->GetID())
		{
			uiFormBase *pBaseForm = nullptr;
			if (CalledCount % 2 == 0)
			{
				pBaseForm = new CTabFormContext1;
			}
			else
			{
				pBaseForm = new CTabFormContext2;
			}

			pBaseForm->Create(m_pTabForm, 0, 0, 0, 0, FCF_VISIBLE);
			m_pTabForm->AddPane(pBaseForm, -1, FALSE);

			++CalledCount;
			bDone = TRUE;
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

	uiButton *m_pButton;
	uiTabForm *m_pTabForm;


};


#define BASE_FRAME uiForm


class CMyForm : public BASE_FRAME
{
public:

	CMyForm() = default;
	~CMyForm() = default;

	void OnCreate()
	{
		BASE_FRAME::OnCreate();

		printx("---> CMyForm::OnCreate\n");

		m_pButton = new uiButton;
		m_pButton->Create(this, 60, 60, 50, 50, FCF_VISIBLE);

		m_pButton2 = new uiButton2;
		m_pButton2->Create(this, 60, 120, 50, 50, FCF_VISIBLE);

		m_pSubForm = new CFormEx;
		m_pSubForm->Create(this, 150, 150, 100, 150, FCF_VISIBLE);
		m_pSubForm->SetHeaderBar(_T("test child form"));

		Bind(m_pButton->GetID());
		Bind(m_pButton2->GetID());
		Bind(uiID_CLOSE);

		VERIFY(m_menu.CreatePopupMenu());
		VERIFY(m_menu.InsertItem(_T("Test item 1"), 2, 1));
		m_menu.ChangeStyle();
	}

	virtual void OnMouseBtnClk(MOUSE_KEY_TYPE KeyType, INT x, INT y)
	{
		switch (KeyType)
		{
		case MKT_LEFT:
			printx("---> OnMouseBtnClk\n");
	//		Size(50, 50);
			break;

		case MKT_RIGHT:
			printx("---> MKT_RIGHT!\n");
			ClientToScreen(x, y);
			uiWindow* pWnd = GetBaseWnd();
			VERIFY(m_menu.Popup((HWND)pWnd->GetHandle(), x, y));
			break;
		}
	}

	virtual void OnCommand(INT id, BOOL &bDone)
	{
		printx("---> CMyForm::OnCommand ID: %d\n", id);

		if (id == m_pButton->GetID())
		{
			uiForm *pToolForm = new uiForm;
			pToolForm->Create(this, 100, 100, 200, 200, FCF_TOOL | FCF_VISIBLE);
			pToolForm->SetHeaderBar(_T("test"));
			bDone = TRUE;
		}
		else if (id == m_pButton2->GetID())
		{
			printx("Button 2 was clicked!\n");

			uiForm *pForm = new CFormEx2;
			pForm->Create(this, 100, 100, 200, 280, FCF_TOOL | FCF_VISIBLE);
			pForm->SetHeaderBar(_T("test"));
			bDone = TRUE;
		}

		if (id == uiID_CLOSE)
		{
			printx("OnClose() hooked!\n");
			bDone = FALSE;
		}
	}


protected:

	uiButton *m_pButton, *m_pButton2;
	uiForm *m_pSubForm;

	uiWinMenu m_menu;


};


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

	uiString a, b;
	a = _T("Unicode string\n");

	printx("sizeof CSimpleList: %d Bytes\n", sizeof UTX::CSimpleList);
	printx("sizeof uiFormBase: %d Bytes\n", sizeof uiFormBase);
	printx("sizeof uiForm: %d Bytes\n", sizeof uiForm);

	BASE_FRAME *pForm = new CMyForm;
	pForm->Create(nullptr, 150, 150, 600, 400, FORM_CREATION_FLAG::FCF_CENTER);

	pForm->SetHeaderBar(_T("test"));
	pForm->SetMenuBar(nullptr);
	pForm->Show(FSM_SHOW);

//	pForm->Move(1000, 600);

	uiMonitorEvent();
/*
	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
*/

	UTXLibraryEnd();

	return 0;
//	return (int) msg.wParam;
}


