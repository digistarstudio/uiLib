

#include "stdafx.h"
#include "uiForm.h"
#include "uiDrawer.h"
#include "UnitTest.h"


class uiCursorDrawForm : public uiForm
{
public:

	uiCursorDrawForm() = default;
	~uiCursorDrawForm() = default;

	BOOL OnCreate() override
	{
		SetHeaderBar(_T("Draw cursor test"));
		return TRUE;
	}


protected:


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



void LogicalTest()
{
	uiImageTest();


}



void UnitTestMain()
{
	LogicalTest();
	GUITest();
}


