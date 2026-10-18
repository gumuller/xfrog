/*

Xfrog - a frogger clone.
Copyright (C) 2000 Gustavo S. Muller

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/**************************************************************

  Author:		Gustavo Simi Muller -> muller@netpar.com.br

  Date:			24/01/2000

  Description:	This module takes care of the Windows stuff.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "mem_man.h"
#include "engine.h"
#include "video.h"
#include "input.h"
#include "timer.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


#define WINDOW_CLASS_NAME		"XFROG"
#define WINDOW_NAME				"XFrog"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
int WINAPI WinMain(HINSTANCE hinstance,HINSTANCE hprevinstance, LPSTR lpcmdline, int nshowcmd);


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////



HWND			g_window_hwnd;
HINSTANCE		g_hinstance;

bool			g_app_focus		= false;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// handles windows messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_CREATE:
			{
				return(0);
			} break;



		case WM_PAINT:
			{
				PAINTSTRUCT	paint_struct;
				BeginPaint(hwnd, &paint_struct);
				EndPaint(hwnd, &paint_struct);

				return(0);
			} break;

		// check if the game window is active
		case WM_ACTIVATEAPP:
			{
				static int32 started = false;

				int32 active = (int32) wparam;

				// the app is not active
				if(active == 0)
				{
					g_app_focus = false;

					if(started)
					{
						g_input->FocusManager();
					}
				}
				else
				{	
					g_app_focus = true;
					
					if(started)
					{
						g_video->GetObj()->RestoreAllSurfaces();
						g_input->FocusManager();
					}
				}

				started = true;
				return(0);
			} break;


		// closes the app
		case WM_CLOSE:
		case WM_DESTROY:
			{				
				PostQuitMessage(0);

				return(0);
			} break;



		default: break;

	} // end switch(msg)


	
	return(DefWindowProc(hwnd, msg, wparam, lparam));
}




int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int nshowcmd)
{
	const bool smoke_test = strcmp(lpcmdline, "--smoke-test") == 0;
	int smoke_frames = 0;
	FILE* smoke_log = smoke_test ? fopen("xfrog-smoke.log", "w") : NULL;

	// defines the window
	WNDCLASSEX wndclass;
	
	Memset(&wndclass, 0, sizeof(wndclass));

	wndclass.cbSize			= sizeof(wndclass);
	wndclass.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc	= WindowProc;
	wndclass.hInstance		= hinstance;
	wndclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName	= WINDOW_CLASS_NAME;
	wndclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);


	// try to register the window
	if(! RegisterClassEx(&wndclass))
		return(0);

	HWND hwnd = CreateWindowEx(WS_EX_TOPMOST,
							   WINDOW_CLASS_NAME,
							   WINDOW_NAME,
							   WS_POPUP | WS_VISIBLE,
							   0, 0,
							   GetSystemMetrics(SM_CXSCREEN),
							   GetSystemMetrics(SM_CYSCREEN),
							   NULL,
							   NULL,
							   hinstance,
							   NULL);

	if(! hwnd)
		return(0);	


	g_window_hwnd = hwnd;
	g_hinstance   = hinstance;

	SetFocus(g_window_hwnd);
    UpdateWindow(g_window_hwnd);


	// hide the mouse cursor
	ShowCursor(false);


	// INITS GAME /////////////////////////////////

	bool result = EngineInit();
	if(result == false)
	{
		exit(-1);
	}

	///////////////////////////////////////////////
	

	if(smoke_log)
	{
		fprintf(smoke_log, "Engine initialized\n");
		fflush(smoke_log);
	}

	// start the main loop
	MSG msg;
	while(true)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{			
			if(g_app_focus)	
			{
				// update the game
				EngineFrame();
				if(smoke_test && ++smoke_frames == 120)
				{
					if(smoke_log)
					{
						fprintf(smoke_log, "Rendered 120 menu frames\n");
						fflush(smoke_log);
					}
					PostQuitMessage(0);
				}
			}
			else 
			{
				// continue to update the timer
				g_timer->Time();

				// make CPU idle
				WaitMessage();
			}
		}
		
	}	


	const bool smoke_passed = smoke_frames == 120 && g_video->PresentedFrames() == 120;
	if(smoke_log)
		fprintf(smoke_log, "Successfully presented %u frames\n", g_video->PresentedFrames());

	// ENGINE SHUTDOWN //////////////////////////////

	EngineShutdown();

	/////////////////////////////////////////////////

	if(smoke_log)
	{
		fprintf(smoke_log, "Engine shut down\n");
		fclose(smoke_log);
	}
	if(smoke_test && !smoke_passed)
		return(1);

	// return to windows
	return(msg.wParam);
}