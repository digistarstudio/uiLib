

#include "stdafx.h"
#include "uiApp.h"
#include "uiDrawer.h"


uiApp* uiApp::pAppIns = nullptr;
HINSTANCE uiApp::AppIns = NULL;


void DbgShowIns(HINSTANCE hIns) { printx("App instance: %p\n", hIns); }

BOOL uiApp::Init(HINSTANCE hIns)
{
	ASSERT(AppIns == NULL);
	AppIns = hIns;
	UTXLibraryInit();
	InitSystemFont();

	DEBUG_CHECK(DbgShowIns(hIns));

	return TRUE;
}

void uiApp::Close()
{
	uiGDIObjCacher::Release(); // TODO

	ReleaseSystemFont();
	UTXLibraryEnd();
}


