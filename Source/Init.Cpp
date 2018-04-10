///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _INIT_CPP
#define _INIT_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <WindowsX.H>
#include <ShellAPI.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <CommCtrl.H>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"

#include "../../@Libraries/CXML/CXML.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::File;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InitializeApp(void)
{
    LoadString(ghAppInstance, IDS_APPCOPYRIGHT, gsAppCopyRight, sizeof(gsAppCopyRight));
    LoadString(ghAppInstance, IDS_FILEVERSION, gsFileVersion, sizeof(gsFileVersion));
    LoadString(ghAppInstance, IDS_TITLECAPTION, gsTitleCaption, sizeof(gsTitleCaption));

	char sExeName[MAX_PATH];
	char sXMLFile[MAX_PATH];
	if(!GetModuleFileName(NULL, sExeName, sizeof(sExeName)))
	{
		MessageBox(NULL, "Failed to retrieve the current module name.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}
	if(!GetFilePath(sExeName, sXMLFile, sizeof(sXMLFile)))
	{
		MessageBox(NULL, "Failed to retrieve the current module path.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	strcat_s(sXMLFile, sizeof(sXMLFile), "AutoUpdate.xml");

	CXMLReader XML;
	if(!XML.FromFile(sXMLFile))
	{
		char sText[1024];
		sprintf_s(sText, "Configuration file not found: \"%s\"", sXMLFile);
		MessageBox(NULL, sText, gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	char sUpdateInfoURL[sizeof(gsUpdateInfoURL)];
	int iAppID = 0;
	bool bXMLResult = true;
	char sOverrideArchitecture[64];

	bXMLResult = bXMLResult && XML.ToString("URL", sUpdateInfoURL, sizeof(sUpdateInfoURL));
	bXMLResult = bXMLResult && XML.ToString("AppVersion", gsCurrentVersion, sizeof(gsCurrentVersion));
	bXMLResult = bXMLResult && XML.ToInteger("AppID", &iAppID);
	if(XML.ToString("Architecture", sOverrideArchitecture, sizeof(sOverrideArchitecture)) <= 0)
	{
		memset(sOverrideArchitecture, 0, sizeof(sOverrideArchitecture));
	}

	memset(gsServiceName, 0, sizeof(gsServiceName));
	XML.ToString("ServiceName", gsServiceName, sizeof(gsServiceName)); //Not required

	if(!bXMLResult)
	{
		MessageBox(NULL, 
			"One or more required configuration file XML attributes could not be found",
			gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	sprintf_s(gsUpdateInfoURL, sizeof(gsUpdateInfoURL), "%s?ID=%d", sUpdateInfoURL, iAppID);

	
	if(strlen(sOverrideArchitecture) > 0)
	{
		strcat_s(gsUpdateInfoURL, sizeof(gsUpdateInfoURL), "&Arch=");
		strcat_s(gsUpdateInfoURL, sizeof(gsUpdateInfoURL), sOverrideArchitecture);
	}
	else
	{
#ifdef _X64_
		strcat_s(gsUpdateInfoURL, sizeof(gsUpdateInfoURL), "&Arch=x64");
#else
		strcat_s(gsUpdateInfoURL, sizeof(gsUpdateInfoURL), "&Arch=x32");
#endif
	}

	if(strlen(gsServiceName) > 0)
	{
		gService.Initialize(gsServiceName, NULL, NULL);
	}

	INITCOMMONCONTROLSEX CommCtl;
    CommCtl.dwSize = sizeof(CommCtl);
    CommCtl.dwICC = ICC_PROGRESS_CLASS;

	if(!InitCommonControlsEx(&CommCtl))
	{
		MessageBox(NULL, "Failed to initialize common controls.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	char sAgent[1024];
	sprintf_s(sAgent, sizeof(sAgent), "%s - %s", gsTitleCaption, gsFileVersion);
	ghNet = InternetOpen(sAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if(!ghNet)
	{
        MessageBox(NULL, "Failed to open internet connection.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UninitializeApp(void)
{
	InternetCloseHandle(ghNet);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif

