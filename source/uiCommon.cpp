

#include "stdafx.h"
#include "uiCommon.h"


std::list<UINT> GIDList;


UINT uiGetID(INT count)
{
	static UINT index = uiID_SYSTEM + 1;

	UINT out;
	if (count == 1 && GIDList.size() != 0)
	{
		out = GIDList.front();
		GIDList.pop_front();
		return out;
	}
	out = index;
	index += count;
	return out;
}

void uiReleaseID(UINT id)
{
#ifdef _DEBUG
	for (std::list<UINT>::iterator it = GIDList.begin(); it != GIDList.end(); ++it)
	{
		ASSERT(id != *it);
	}
#endif
	GIDList.push_front(id);
}


uiMenu::uiMenu()
{
	m_iCount = 0;
//	m_pItemArray = nullptr;
}

uiMenu::~uiMenu()
{
//	SAFE_DELETE_ARRAY(m_pItemArray);
}

BOOL uiMenu::AddItem(INT index, TCHAR* pStr, UINT type)
{

	return TRUE;
}

BOOL uiMenu::AppendItem(TCHAR* pStr, UINT type)
{

	return TRUE;
}

void DeleteItem(INT count, uiMenuItem* pItemArray)
{
	for (INT i = 0; i < count; ++i)
	{

		if (pItemArray[i].pNext != nullptr)
		{

		}
	}
}


void uiMenu::Release()
{

//	if (m_pItemArray != nullptr)

}

INT uiMenu::GetCount(BOOL bCountSeparator)
{
	if (bCountSeparator)
		return m_ItemArray.size();

	INT count = 0;
	for (INT i = 0, Total = m_ItemArray.size(); i < Total; ++i)
	{
	}

	return count;
}


