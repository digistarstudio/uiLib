

#pragma once


//void* 

//#define DEC_INT 
//#define DECLARE_METHOD virtual


class uiAppBase
{
	virtual INT Run() abstract;

};


template<typename T>
class uiApp : public T
{
public:

	uiApp() {}
	~uiApp() {}

	BOOL Init()
	{
		UTXLibraryInit();
		return TRUE;
	}
	void Close()
	{
	}


protected:


};


#ifdef NO_VTABLE_LOOKUP
	#define APP_BASE_TYPE uiApp
#else
	#define APP_BASE_TYPE uiApp<uiAppBase>
#endif


class CMyTestApp : public APP_BASE_TYPE
{
	INT Run();
};


INLINE INT CMyTestApp::Run()
{
	return 0;
}

//CMyTestApp GMTA;


