///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CLIENT_UPDATE_CPP
#define _CLIENT_UPDATE_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>
#include <WinLDap.H>
#include <ShlWapi.H>
#include <CommCtrl.H>
#include <UrlMon.H>
#include <Wininet.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Entry.H"
#include "Init.H"
#include "ClientUpdate.H"

#include "../Dialogs/MainDlg.H"
#include "../../@Libraries/CXML/CXML.H"
#include "../../@Libraries/CStringBuilder/CStringBuilder.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;
using namespace NSWFL::File;
using namespace NSWFL::String;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	-1 : sOne is greater that sTwo.
	0  : The versions are equal.
	+1 : sTwo is greater that sOne.
*/
int CompareVersions(const char *sOne, const char *sTwo)
{
	int iOneSz = (int)strlen(sOne);
	int iTwoSz = (int)strlen(sTwo);
	int iRPos = 0;

	char sVerOne[4][64];
	memset(sVerOne, 0, sizeof(sVerOne));

	char sVerTwo[4][64];
	memset(sVerTwo, 0, sizeof(sVerTwo));

	if(GetNextToken(sOne, iOneSz, sVerOne[0], sizeof(sVerOne[0]), &iRPos, ".", 1) > 0)
	{
		iRPos++;
		if(GetNextToken(sOne, iOneSz, sVerOne[1], sizeof(sVerOne[1]), &iRPos, ".", 1) > 0)
		{
			iRPos++;
			if(GetNextToken(sOne, iOneSz, sVerOne[2], sizeof(sVerOne[2]), &iRPos, ".", 1) > 0)
			{
				iRPos++;
				GetNextToken(sOne, iOneSz, sVerOne[3], sizeof(sVerOne[3]), &iRPos, ".", 1);
			}
		}
	}

	//-----------

	iRPos = 0;
	if(GetNextToken(sTwo, iTwoSz, sVerTwo[0], sizeof(sVerTwo[0]), &iRPos, ".", 1) > 0)
	{
		iRPos++;
		if(GetNextToken(sTwo, iTwoSz, sVerTwo[1], sizeof(sVerTwo[1]), &iRPos, ".", 1) > 0)
		{
			iRPos++;
			if(GetNextToken(sTwo, iTwoSz, sVerTwo[2], sizeof(sVerTwo[2]), &iRPos, ".", 1) > 0)
			{
				iRPos++;
				GetNextToken(sTwo, iTwoSz, sVerTwo[3], sizeof(sVerTwo[3]), &iRPos, ".", 1);
			}
		}
	}

	int iOne = 0;
	int iTwo = 0;

	for(int iSeg = 0; iSeg < 4; iSeg++)
	{
		iOne = atol(sVerOne[iSeg]);
		iTwo = atol(sVerTwo[iSeg]);

		if(iOne > iTwo)
		{
			return -1;
		}
		else if(iTwo > iOne)
		{
			return 1;
		}
	}


	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ExecInternal(const char *sFile, const char *sParams)
{
	char sPath[MAX_PATH];
	char sCmd[MAX_PATH];
	GetFilePath(sFile, sPath, sizeof(sPath));

	STARTUPINFO SI;
	memset(&SI, 0, sizeof(SI));
	SI.cb = sizeof(SI);

	PROCESS_INFORMATION PI;
	memset(&PI, 0, sizeof(PI));

	if(sParams)
	{
		sprintf_s(sCmd, "%s %s", sFile, sParams);
	}
	else{
		strcpy_s(sCmd, sFile);
	}

	if(CreateProcess(NULL, sCmd, NULL, NULL, FALSE, 0, NULL, sPath, &SI, &PI) <= 0)
	{
		return false;
	}

	DWORD dwExitCode = 0;
	do{
		GetExitCodeProcess(PI.hProcess, &dwExitCode);
		Sleep(100);
	}while(dwExitCode == STILL_ACTIVE);

    return dwExitCode == 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GetFileSize(const char *sURL)
{
	char sBuffer[1024];
	DWORD dwBufferSz = sizeof(sBuffer);
	DWORD dwIndex = 0;
	int iFileSize = -1;

	HINTERNET hINetFile = InternetOpenUrl(ghNet, sURL, NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0); 
	if(!hINetFile)
	{
        MessageBox(hMainDialog, "Failed to open internet URL.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	dwBufferSz = sizeof(sBuffer);
	dwIndex = 0;
	if(HttpQueryInfo(hINetFile, HTTP_QUERY_STATUS_CODE, sBuffer, &dwBufferSz, &dwIndex))
	{
		if(atol(sBuffer) != 200)
		{
			CStringBuilder errorText;
			errorText.AppendF("The update service is currently unavailable: Error: %s.\nPlease try again later!", sBuffer);
	        MessageBox(hMainDialog, errorText.Buffer, gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);

			InternetCloseHandle(hINetFile);
			return -1;
		}
	}

	dwBufferSz = sizeof(sBuffer);
	dwIndex = 0;
	if(HttpQueryInfo(hINetFile, HTTP_QUERY_CONTENT_LENGTH , sBuffer, &dwBufferSz, &dwIndex))
	{
		iFileSize = atol(sBuffer);
	}

	InternetCloseHandle(hINetFile);

	return iFileSize;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool DownloadFile(const char *sURL, const char *sFile, int iTotalSize)
{
	bool bResult = false;
	const int BUFFER_SIZE = 8 * 1024;
	unsigned long iSize = 0;
	char sBuffer[BUFFER_SIZE+1];
	DWORD dwBufferSz = 0;
	DWORD dwIndex = 0;
	char sTotalSize[255];
	char sCurrentSize[255];
 	FILE *hFile = NULL;
	float fProgress = 0;
	float fProgressMax = (float) iTotalSize;
	FileSizeFriendly(iTotalSize, 2, sTotalSize, sizeof(sTotalSize));

	HINTERNET hINetFile = InternetOpenUrl(ghNet, sURL, NULL, 0,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE, 0); 
	if(!hINetFile)
	{
        MessageBox(hMainDialog, "Failed to open internet URL.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
		return false;
	}

	dwBufferSz = sizeof(sBuffer);
	dwIndex = 0;
	if(HttpQueryInfo(hINetFile, HTTP_QUERY_STATUS_CODE, sBuffer, &dwBufferSz, &dwIndex))
	{
		if(atol(sBuffer) != 200)
		{
			CStringBuilder errorText;
			errorText.AppendF("The update service is currently unavailable: Error: %s.\nPlease try again later!", sBuffer);
	        MessageBox(hMainDialog, errorText.Buffer, gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);

			InternetCloseHandle(hINetFile);
			return false;
		}
	}

	dwBufferSz = sizeof(sBuffer);
	dwIndex = 0;
	if(HttpQueryInfo(hINetFile, HTTP_QUERY_CONTENT_LENGTH , sBuffer, &dwBufferSz, &dwIndex))
	{
		iTotalSize = atol(sBuffer);
	}

	if(fopen_s(&hFile, sFile, "wb") != 0)
	{
        MessageBox(hMainDialog, "Failed to open temp file.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);

		InternetCloseHandle(hINetFile);
		return false;
	}

	while((bResult = InternetReadFile(hINetFile, sBuffer, BUFFER_SIZE, &iSize) > 0))
	{
		if(iSize <= 0)
		{
			bResult = true;
			break;
		}
		fwrite(sBuffer, sizeof(char), iSize, hFile);
		fProgress += iSize;

		if(iTotalSize > 0)
		{
			if(fProgress > fProgressMax)
			{
				fProgress = fProgressMax;
			}

            float fPercentage  = (float) ((fProgress / fProgressMax) * 100);

            sprintf_s(sBuffer, sizeof(sBuffer),
				"Downloaded %s of %s (%.1f%%)",
				FileSizeFriendly((__int64)fProgress, 2, sCurrentSize, sizeof(sCurrentSize)),
				sTotalSize, fPercentage);
			Set_Text(hStatusText, sBuffer);
            SendMessage(hProgressBar, PBM_SETPOS, (ULONG)fPercentage, 0);
		}
		else{
            sprintf_s(sBuffer, sizeof(sBuffer),
				"Downloaded %s", FileSizeFriendly((__int64)fProgress, 2, sCurrentSize, sizeof(sCurrentSize)));
			Set_Text(hStatusText, sBuffer);
		}
		Sleep(10);
	}

	if(!bResult)
	{
        MessageBox(hMainDialog, "Failed to download file.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
	}

	fclose(hFile);
	InternetCloseHandle(hINetFile);

	return bResult;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UpdateInternal(const char *sXMLFile, const char *sSetupFile, const char *sTempFolder)
{
    char sText[1024];
 
	Set_Text(hStatusText, "Checking for new version.");
    if(DownloadFile(gsUpdateInfoURL, sXMLFile, 0))
    {
		Set_Text(hStatusText, "Update information obtained.");

		CXMLReader XML;
		if(!XML.FromFile(sXMLFile))
		{
			MessageBox(hMainDialog, "Failed to open XML file.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
			return false;
		}

		char sName[1024];
		char sVersion[1024];
		char sLastUpdate[1024];
		char sFileURL[1024];
		char sFileSize[128];
		int iFileSize = 0;

		/*
		<CurrentVersion>
			<ID></ID>
			<Name></Name>
			<Version></Version>
			<OS></OS>
			<ShortDescription></ShortDescription>
			<Description></Description>
			<FileName></FileName>
			<FileSize></FileSize>
			<LastUpdate></LastUpdate>
		</CurrentVersion>
		*/

		if(XML.ToString("Name", sName, sizeof(sName)) <= 0)
		{
			MessageBox(hMainDialog,
				"An invalid update meta-file was received.\n"
				"Please try again later!", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
			return false;
		}
		XML.ToString("Version", sVersion, sizeof(sVersion));
		XML.ToString("LastUpdate", sLastUpdate, sizeof(sLastUpdate));
		XML.ToString("FileName", sFileURL, sizeof(sFileURL));

		if((iFileSize = GetFileSize(sFileURL)) < 0)
		{
			XML.ToInteger("FileSize", &iFileSize);
		}

		FileSizeFriendly(iFileSize, 2, sFileSize, sizeof(sFileSize));

		sprintf_s(sText, sizeof(sText), "%s %s", sName, sVersion);
		Set_Text(hAppTitle, sText);

        Sleep(2500);

		int iVerResult = CompareVersions(sVersion, gsCurrentVersion);

		DWORD dwFlags = MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2;

		if(iVerResult == 0)
		{
			sprintf_s(sText, sizeof(sText),
				"%s is up-to-date.\nVersion %s, released on %s.",
				sName, sVersion, sLastUpdate);
			dwFlags = MB_ICONINFORMATION|MB_OK;
		}
		else if(iVerResult < 0)
		{
			sprintf_s(sText, sizeof(sText),
				"A new version of %s is avaliable.\n"
				"Version %s was released on %s.\r\n\r\n"
				"Would you like to download it now (%s)?",
				sName, sVersion, sLastUpdate, sFileSize);

			dwFlags = MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON1;
		}
		else if(iVerResult > 0)
		{
			sprintf_s(sText, sizeof(sText),
				"It seems as though you have a newer version of %s than is currenly available."
				" Version %s was released on %s but you are running version %s.\r\n\r\n"
				"Would you like to download the older version now (%s)?",
				sName, sVersion, sLastUpdate, gsCurrentVersion, sFileSize);

			dwFlags = MB_ICONINFORMATION|MB_YESNO|MB_DEFBUTTON2;
		}

		if(MessageBox(hMainDialog, sText, gsTitleCaption, dwFlags) != IDYES)
		{
			return true;
		}

		Set_Text(hStatusText, "Downloading latest version.");
	    if(DownloadFile(sFileURL, sSetupFile, iFileSize))
	    {
			if(strlen(gsServiceName) > 0)
			{
				Set_Text(hStatusText, "Stopping Service.");
				gService.Stop();
				Sleep(1000);
			}

			bool bResult = false;

			Set_Text(hStatusText, "Installing update.");
			Sleep(1000);
			if(ExecInternal(sSetupFile, "/sp- /silent"))
			{	
				bResult = true;
			}

			if(strlen(gsServiceName) > 0)
			{
				Set_Text(hStatusText, "Starting Service.");
				gService.Start();
				Sleep(1000);
			}

			if(bResult)
			{
				MessageBox(hMainDialog, "The update was installed successfully.", gsTitleCaption, MB_ICONINFORMATION);
			}

			return bResult;
		}
    }

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ClientUpdate(void)
{
	EnableWindow(GetDlgItem(hMainDialog, IDC_CANCEL), TRUE);

    char sXMLFile[MAX_PATH];
    char sSetupFile[MAX_PATH];
    char sTempFolder[MAX_PATH];

	if(Get_TempDirectory(sTempFolder, sizeof(sTempFolder)))
	{
		GetTempFileName(sTempFolder, "XML", 0, sXMLFile);
		DeleteFile(sXMLFile);
		strcat_s(sXMLFile, sizeof(sXMLFile), ".xml");

		GetTempFileName(sTempFolder, "EXE", 0, sSetupFile);
		DeleteFile(sSetupFile);
		strcat_s(sSetupFile, sizeof(sSetupFile), ".exe");
	}
	else{
        MessageBox(hMainDialog, "Failed to retrieve temp directory.", gsTitleCaption, MB_ICONSTOP|MB_TASKMODAL);
	    EndDialog(hMainDialog, 0);
		return 0;
	}

	bool bResult = UpdateInternal(sXMLFile, sSetupFile, sTempFolder);

    DeleteFile(sXMLFile);
    DeleteFile(sSetupFile);

	EndDialog(hMainDialog, 0);

    return bResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
