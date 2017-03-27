//---------------------------------------------------------------------------------------------------------------------
//
// File name   : UTX.cpp
// Author      : Created by Pointer.
// State       : 
// Update date : 06/01/16
// Description : 
//
//---------------------------------------------------------------------------------------------------------------------


#include "StdAfx.h"
#include ".\external\UTX.h"
#include <conio.h>


static INT nGUtxInitialized = 0;
static CCriticalSectionUTX GPrintxCs;

DOUBLE CPerformanceCounter::factor = 0.0;
LARGE_INTEGER CPerformanceCounter::freq;

TObjectPool<stSimpleListNode> stSimpleListNode::m_ObjectPool;


BOOL UTXLibraryInit()
{
	if (InterlockedIncrementT(&nGUtxInitialized) > 1)
		return nGUtxInitialized;

	LARGE_INTEGER li;
	::QueryPerformanceFrequency(&li);
	CPerformanceCounter::factor = 1000.0 / li.QuadPart;
	CPerformanceCounter::freq = li;

	VERIFY(stSimpleListNode::m_ObjectPool.CreatePool(200, 5000));

	return nGUtxInitialized;
}

void UTXLibraryEnd()
{
	ASSERT(nGUtxInitialized > 0);
	if (InterlockedDecrementT(&nGUtxInitialized) != 0)
		return;

	stSimpleListNode::m_ObjectPool.ReleasePool();
}

int printx(const char *format, ...)
{
	//#ifndef _DEBUG
	//	#pragma message(FILE_LOC "Release build printx disabled.")
	//	return 0;
	//#else
	CHAR buffer[1024];
	INT nCharacters;
	va_list vlist;

	va_start(vlist, format);
	nCharacters = vsprintf_s(buffer, _countof(buffer), format, vlist);
	va_end(vlist);

	ASSERT(nCharacters <= sizeof(buffer));

	GPrintxCs.EnterCriticalSection();
	_cprintf(buffer);
	GPrintxCs.LeaveCriticalSection();

	return nCharacters;
	//#endif
}

int printx(const wchar_t *format, ...)
{
	//#ifndef _DEBUG
	//	#pragma message(FILE_LOC "Release build printx disabled.")
	//	return 0;
	//#else
	WCHAR buffer[1024];
	INT nCharacters;
	va_list vlist;

	va_start(vlist, format);
	nCharacters = vswprintf(buffer, _countof(buffer) - 1, format, vlist);
	va_end(vlist);

	ASSERT(nCharacters <= sizeof(buffer));

	GPrintxCs.EnterCriticalSection();
	_cwprintf(buffer);
	GPrintxCs.LeaveCriticalSection();

	return nCharacters;
	//#endif
}

void UTXStringLowercase(TCHAR *in)
{
	std::size_t length = _tcslen(in), i;
	for(i = 0; i < length; i++)
		if('A' <= in[i] && in[i] <= 'Z')
			in[i] -= 'A' - 'a';
}

void UTXStringUppercase(TCHAR *in)
{
	std::size_t length = _tcslen(in), i;
	for(i = 0; i < length; i++)
		if('a' <= in[i] && in[i] <= 'z')
			in[i] += 'A' - 'a';
}

BOOL UTXIsStringLowercase(TCHAR const *in)
{
	std::size_t length = _tcslen(in), i;
	for(i = 0; i < length; i++)
		if('A' <= in[i] && in[i] <= 'Z')
			return FALSE;
	return TRUE;
}

BOOL UTXIsStringUppercase(TCHAR const *in)
{
	std::size_t length = _tcslen(in), i;
	for(i = 0; i < length; i++)
		if('a' <= in[i] && in[i] <= 'z')
			return FALSE;
	return TRUE;
}


