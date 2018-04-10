///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENTRY_H
#define _ENTRY_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../@Libraries/Debug/Debug.H"
#include "../../@Libraries/CMemPool/CMemPool.H"
#include "../../@Libraries/CCmdLine/CCmdLine.H"
#include "../../@Libraries/CService/CService.H"
#include "../../@Libraries/NSWFL/NSWFL.H"

#include "../Resources/Resource.H"

#include <Wininet.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern HINSTANCE ghAppInstance;

extern char gsAppCopyRight[64];
extern char gsFileVersion[32];
extern char gsTitleCaption[64];
extern char gsUpdateInfoURL[1024];
extern char gsCurrentVersion[64];
extern char gsServiceName[1024];
extern HINTERNET ghNet;

extern CService gService; //The global service.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
