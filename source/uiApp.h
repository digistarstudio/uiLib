

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

void uiMonitorEvent();


#define RunBench(m1, m2, count) \
{ \
	CPerformanceCounter pc1, pc2; \
	pc1.Start(); \
	for (INT i = 0; i < count; ++i) \
	{ \
		m1; \
	} \
	pc1.End(); \
	pc2.Start(); \
	for (INT i = 0; i < count; ++i) \
	{ \
		m2; \
	} \
	pc2.End(); \
	FILE* pFile; \
	_tfopen_s(&pFile, _T("r:\\result.txt"), _T("a+t")); \
	fprintf(pFile, "Method 1: %f, Method 2: %f\n", pc1.Get(), pc2.Get()); \
	fclose(pFile); \
}


