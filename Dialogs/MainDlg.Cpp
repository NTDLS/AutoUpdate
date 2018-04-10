///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright © NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MAINDLG_CPP
#define _MAINDLG_CPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.H>
#include <Stdio.H>
#include <Stdlib.H>

#include "../Source/Entry.H"
#include "../Source/ClientUpdate.H"

#include "../Dialogs/MainDlg.H"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace NSWFL::Windows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------(Variable Declarations)
HWND hMainDialog = NULL;
HWND hStatusText  = NULL;
HWND hProgressBar = NULL;
HWND hAppTitle    = NULL;

HANDLE hUpdateThread = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //--------------------------------------------------------------------------
    if(uMsg == WM_INITDIALOG) // Received A Initilize Dialog Message
    {
        hMainDialog = hWnd;

        SendMessage(hWnd, (UINT)WM_SETTEXT, (WPARAM)0, (LPARAM)gsTitleCaption);
        SendMessage(hWnd, WM_SETICON, TRUE, (LPARAM) LoadIcon(ghAppInstance, MAKEINTRESOURCE(IDI_MAIN)) );

        hStatusText  = GetDlgItem(hWnd, IDC_STATUSTEXT);
        hProgressBar = GetDlgItem(hWnd, IDC_PROGRESSBAR);
        hAppTitle    = GetDlgItem(hWnd, IDC_APPTITLE);

        Set_Text(hStatusText, "Please wait...");

        CenterWindow(hWnd);

        return TRUE; // Return TRUE to set the keyboard focus, Otherwise return FALSE.
    }
	//--------------------------------------------------------------------------
	else if(uMsg == WM_SHOWWINDOW)
	{
		static bool bPreReleaseDlgShown = false;
		if(!bPreReleaseDlgShown)
		{
	    	hUpdateThread = CreateThread(NULL, 0, Update_Thread, (LPVOID)0, 0, NULL);
		}
	}
	//--------------------------------------------------------------------------
    else if(uMsg == WM_COMMAND)
    {
        if(wParam == IDC_CANCEL) //- Cancel Button.
        {
			SuspendThread(hUpdateThread);
			if(MessageBox(hMainDialog, "Are you sure you want to cancel?",
				gsTitleCaption, MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2) == IDYES)
			{
				TerminateThread(hUpdateThread, 0);
				EndDialog(hWnd, 0);
				DestroyWindow(hWnd);
				hMainDialog = NULL;
			}
			ResumeThread(hUpdateThread);

            return TRUE;
        }

        return FALSE;
    }
    //--------------------------------------------------------------------------
	else if(uMsg == WM_PAINT)
    {
        HDC ThisHDC;
        PAINTSTRUCT ThisPS;

        ThisHDC = BeginPaint(hWnd, &ThisPS);

        //- Any painting should be done here.

        EndPaint(hWnd, &ThisPS);
        return TRUE;
    }
    //--------------------------------------------------------------------------
    else if(uMsg == WM_CLOSE) //- Received close message.
    {
        EndDialog(hWnd,0);
        DestroyWindow(hWnd);
		hMainDialog = NULL;
        return TRUE;
    }

    //--------------------------------------------------------------------------

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD WINAPI Update_Thread(LPVOID lpVoid)
{
    Sleep(1000);
	ClientUpdate();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
