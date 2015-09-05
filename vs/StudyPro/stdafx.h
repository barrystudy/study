// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif


#include <atlstr.h>     // Support for CString
#include <ATLComTime.h> // Support for COleDateTime

#include <tchar.h>
#include <strsafe.h>

typedef LPWSAOVERLAPPED_COMPLETION_ROUTINE LPWSACOMPLETIONROUTINE;

#include <assert.h>



#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(x) assert(x)

