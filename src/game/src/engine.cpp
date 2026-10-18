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

  Description:	XFrog engine.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "error.h"
#include "video.h"
#include "engine.h"
#include "input.h"
#include "timer.h"
#include "tga.h"
#include "cfs.h"
#include "sound.h"
#include "menu.h"
#include "font.h"
#include "game.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool EngineInit();
void EngineShutdown();
void EngineFrame();


static void EngineExit();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


engine_state_t	g_engine_state = INTRO;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inits the engine, and returns the result
bool EngineInit()
{
	bool result;

	// INITS CFS //////////////////////////////////////

	result = CFS_Init();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	// INITS VIDEO ////////////////////////////////////
	
	result = VideoInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////


	// INITS INPUT ////////////////////////////////////

	result = InputInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	// INITS TIMER ////////////////////////////////////

	result = TimerInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	
	// INITS SOUND ////////////////////////////////////

	result = SoundInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	// INITS MENU /////////////////////////////////////

	result = MenuInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	// INITS THE FONT ENGINE //////////////////////////

	result = FontEngineInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////
		
	// INITS GAME /////////////////////////////////////

	result = GameInit();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	///////////////////////////////////////////////////

	// set the game state
	g_engine_state = MENU;

	return(true);
}



// shutdown the engine and free resources
void EngineShutdown()
{

	// GAME SHUTDOWN ///////////////////////////////////

	GameShutdown();

	////////////////////////////////////////////////////

	// FONT ENGINE SHUTDOWN ////////////////////////////

	FontEngineShutdown();

	////////////////////////////////////////////////////

	// MENU SHUTDOWN ///////////////////////////////////

	MenuShutdown();

	////////////////////////////////////////////////////

	// SOUND SHUTDOWN //////////////////////////////////

	SoundShutdown();

	////////////////////////////////////////////////////

	// TIMER SHUTDOWN //////////////////////////////////

	TimerShutdown();

	////////////////////////////////////////////////////

	// INPUT SHUTDOWN //////////////////////////////////

	InputShutdown();

	////////////////////////////////////////////////////

	// VIDEO SHUTDOWN //////////////////////////////////

	VideoShutdown();

	////////////////////////////////////////////////////

	// CFS SHUTDOWN ////////////////////////////////////

	CFS_Shutdown();

	////////////////////////////////////////////////////

}



// renders the next frame, no matters the game state
void EngineFrame()
{	
	g_timer->Time();
	g_video->ClearBackSurface(0);
	InputUpdate();
	SoundUpdate();

	// choose the module update according to the engine state
	switch(g_engine_state)
	{
		case GAME:
			{

				GameUpdate();

			} break;

		case MENU:
			{

				MenuUpdate();

			} break;

		
		case EXIT:
			{				
				EngineExit();		

			} break;
	}
	
	// updates video
	VideoUpdate();
}




// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


static void EngineExit()
{
	g_video->ClearBackSurface(0);

	PrintCenter(0, "Xfrog Staff");
	PrintCenter(24,"Programming:");
	PrintCenter(32,"Gustavo Muller");
	PrintCenter(48,"Design:");
	PrintCenter(56,"Gustavo Muller");
	PrintCenter(72,"Additional Design");
	PrintCenter(80,"Guilherme Muller");
	PrintCenter(96,"Sounds:");
	PrintCenter(104,"Various sources");
	PrintCenter(120,"Art:");
	PrintCenter(128,"Guilherme Muller");
	
	PrintCenter(236,"Do you want to exit ? (Y/N)");

	if(InputCheckKey(DIK_Y, DOWN))
		PostQuitMessage(0);
	else if(InputCheckKey(DIK_N, DOWN))
		g_engine_state = MENU;

}