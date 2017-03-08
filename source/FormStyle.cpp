

#include "stdafx.h"
#include "uiForm.h"
#include "FormStyle.h"


static DEFAULT_SYSTEM_STYLE GDSS = DSS_NULL;


uiFormStyle* GetDefaultStyleObject(uiFormBase *pFormBase)
{
	FORM_CLASS fs = pFormBase->GetClass();

	switch (fs)
	{
	case FC_BASE:
		break;

	case FC_TOOLBAR:
		break;

	case FC_FORM:
		break;

	case FC_HEADER_BAR:
		break;

	case FC_CONTROL:
		break;

	case FC_BUTTON:
		break;

	case FC_STATIC:
		break;
	}

	return nullptr;
}


void uiSetDefaultStyle(DEFAULT_SYSTEM_STYLE DSS)
{
	ASSERT(GDSS == DSS_NULL); // It's better you call this once before doing anything.




}



