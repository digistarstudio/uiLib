// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#pragma once


#include "targetver.h"

#define OEMRESOURCE
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <list>

using namespace std;

// TODO: reference additional headers your program requires here


//#include ".\CoreLib\source\CommonData.h"
#include ".\external\UTX.h"
#include ".\external\Template.h"

#ifdef _DEBUG
	#pragma comment(lib, "CorelibDU.lib")
#else
	#pragma comment(lib, "CorelibU.lib")
#endif


#ifndef CONTAINING_RECORD
	#define CONTAINING_RECORD(address, type, field) ((type*)((CHAR*)(address) - (DWORD_PTR)(&((type *)0)->field)))
#endif


