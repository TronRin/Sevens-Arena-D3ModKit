/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "tools/edit_gui_common.h"
#include "../../sys/win32/rc/resource.h"
#include "LightApp.h"
#include "lightdlg.h"
#include <process.h>
#include <ddeml.h>

BEGIN_MESSAGE_MAP(CLightApp, CWinApp)
END_MESSAGE_MAP()

/*
================
CLightApp::CLightApp
================
*/
CLightApp::CLightApp()
{
}

CLightApp theApp;

/*
================
CLightApp::InitInstance
================
*/
BOOL CLightApp::InitInstance() {
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof( InitCtrls );

	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx( &InitCtrls );

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if ( !AfxOleInit() ) {
		return FALSE;
	}

	AfxEnableControlContainer();

	AfxInitRichEdit2();

	// Change the registry key under which our settings are stored.
	//SetRegistryKey( EDITOR_REGISTRY_KEY );

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// create main MDI Frame window
	qglEnableClientState( GL_VERTEX_ARRAY );

	m_pMainWnd = g_LightDialog;

	// The main window has been initialized, so show and update it.
	g_LightDialog->ShowWindow( m_nCmdShow );
	g_LightDialog->UpdateWindow();

	return TRUE;
}

/*
================
CLightApp::ExitInstance
================
*/
int CLightApp::ExitInstance() {
	common->Shutdown();
	g_LightDialog = NULL;
	int ret = CWinApp::ExitInstance();
	ExitProcess(0);
	return ret;
}

/*
================
CLightApp::OnIdle
================
*/
BOOL CLightApp::OnIdle( LONG lCount ) {
	return FALSE;
}

/*
================
CLightApp::Run
================
*/
int CLightApp::Run( void ) {
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;

#if _MSC_VER >= 1300
	MSG *msg = AfxGetCurrentMessage();			// TODO Robert fix me!!
#else
	MSG *msg = &m_msgCur;
#endif

	// phase1: check to see if we can do idle work
	while ( bIdle &&	!::PeekMessage( msg, NULL, NULL, NULL, PM_NOREMOVE ) ) {
		// call OnIdle while in bIdle state
		if ( !OnIdle( lIdleCount++ ) ) {
			bIdle = FALSE; // assume "no idle" state
		}
	}

	// phase2: pump messages while available
	do {
		// pump message, but quit on WM_QUIT
		if ( !PumpMessage() ) {
			return ExitInstance();
		}

		// reset "no idle" state after pumping "normal" message
		if ( IsIdleMessage( msg ) ) {
			bIdle = TRUE;
			lIdleCount = 0;
		}

	} while ( ::PeekMessage( msg, NULL, NULL, NULL, PM_NOREMOVE ) );

	return 0;
}