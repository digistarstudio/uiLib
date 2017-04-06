

#pragma once


class uiApp
{
public:

	uiApp() { ASSERT(pAppIns == nullptr); pAppIns = this; }
	virtual ~uiApp() { ASSERT(pAppIns == this); pAppIns = nullptr; }


	BOOL Init(HINSTANCE hIns);
	void Close();

	virtual INT  InitApp() { return TRUE; }
	virtual INT  ExitApp() { return 0; }
	virtual void Run() = 0;

	static uiApp* GetSingleton() { return pAppIns; }


protected:

	friend HINSTANCE uiGetAppIns();

	static uiApp* pAppIns;
	static HINSTANCE AppIns;


};


INLINE HINSTANCE uiGetAppIns() { return uiApp::AppIns; }


