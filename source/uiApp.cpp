

#include "stdafx.h"
#include "uiApp.h"
#include "uiDrawer.h"
#include "uiMsWin.h"


uiApp* uiApp::pAppIns = nullptr;
HINSTANCE uiApp::AppIns = NULL;


BOOL uiApp::Init(HINSTANCE hIns)
{
	ASSERT(AppIns == NULL);
	AppIns = hIns;
	UTXLibraryInit();
	InitSystemFont();
	InitWindowSystem();

	return TRUE;
}

void uiApp::Close()
{
	uiGDIObjCacher::Release(); // TODO

	CloseWindowSystem();
	ReleaseSystemFont();
	UTXLibraryEnd();
}


void uiMonitorEvent()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0U, 0U))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


