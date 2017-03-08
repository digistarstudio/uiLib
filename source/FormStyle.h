

#pragma once


#include "uiDrawer.h"
#include "uiCommon.h"


enum DEFAULT_SYSTEM_STYLE
{
	DSS_NULL,

	DSS_WIN_XP,
	DSS_WIN_7,
	DSS_WIN_8,
};


class uiCustomDrawImp
{
public:

	virtual void Draw(uiDrawer *pDrawer, uiFormBase *pFormBase) = 0;


};


struct uiFormStyle
{
	UINT8 FormEdgeWidth;
	UINT8 ClientEdgeType;
	UINT8 ClientEdgeWidth;
	UINT32 ClientEdgeColor;
	UINT32 ClientColor;

	uiCustomDrawImp *pImp = nullptr;
};


uiFormStyle* GetDefaultStyleObject(uiFormBase *pFormBase);
void uiSetDefaultStyle(DEFAULT_SYSTEM_STYLE DSS);


