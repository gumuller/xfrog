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

  Date:			02/02/2000

  Description:	XFrog menu functions

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "menu.h"
#include "video.h"
#include "sound.h"
#include "mem_man.h"
#include "error.h"
#include "tga.h"
#include "engine.h"
#include "input.h"
#include "font.h"
#include "version.h"
#include "timer.h"
#include "game.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool MenuInit();
void MenuShutdown();
void MenuUpdate();

static void MenuItemNewGame();
static void MenuItemReturnGame();
static void MenuItemExit();
static void CheckInput();
static void PrintXfrogVersion();


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// the menu state
enum menu_state_t 
{ 
	MENU_NEW_GAME = 0, 
	MENU_RETURN_GAME,
	MENU_EXIT 
};


// TYPEDEFS ///////////////////////////////////////////////////////////////////////////////////


// functions to menu execute on a menu item call
typedef void (*menu_func_t)();


// STRUCTS ////////////////////////////////////////////////////////////////////////////////////


// a menu item
struct menu_item_t
{
	int32			x;						// screen position
	int32			y;						// ''
	Surface*		surface;				// the grafic of the menu
	menu_func_t		func;					// function to call
};


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// the menus itens
static menu_item_t	m_new_game;
static menu_item_t  m_return_game;
static menu_item_t	m_exit_item;



// the xfrog title
static Surface*		m_title			= NULL;


// the menu cursor
static Surface*		m_cursor		= NULL;


// check for enter pressing
static bool			m_selected		= false;

// current menu selection
static int32		m_curr_selection = MENU_NEW_GAME;


// cursor change sound
static SoundBuffer*	m_frog_sound = NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



// inits the menu
bool MenuInit()
{
	// inits the menu itens
	m_new_game.surface		= LoadTGA("gfx/ui/newgame.tga", MEM_SYSTEM, NULL);
	if(m_new_game.surface == NULL)
		return(false);
	
	m_new_game.func			= MenuItemNewGame;
	m_new_game.x			= 320-m_new_game.surface->GetWidth()/2;
	m_new_game.y			= 250;

	
	m_return_game.surface	= LoadTGA("gfx/ui/return.tga", MEM_SYSTEM, NULL);
	if(m_return_game.surface == NULL)
		return(false);

	m_return_game.func		= MenuItemReturnGame;
	m_return_game.x			= 320-m_return_game.surface->GetWidth()/2;
	m_return_game.y			= 300;



	m_exit_item.surface		= LoadTGA("gfx/ui/exit.tga", MEM_SYSTEM, NULL);
	if(m_exit_item.surface == NULL)
		return(false);	

	m_exit_item.func		= MenuItemExit;
	m_exit_item.x			= 320-m_exit_item.surface->GetWidth()/2;
	m_exit_item.y			= 350;



	// loads the title
	m_title = LoadTGA("gfx/ui/xfroglogo.tga", MEM_SYSTEM, NULL);
	if(m_title == NULL)
		return(false);

	// loads the menu cursor
	m_cursor = LoadTGA("gfx/ui/cursor.tga", MEM_SYSTEM, NULL);
	if(m_cursor == NULL)
		return(false);

	// loads the frog sound
	m_frog_sound = new SoundBuffer;
	m_frog_sound->LoadWAV("sounds/frog/frog.wav");

	return(true);
}



// shutsdown the menu
void MenuShutdown()
{
	// clears memory
	delete m_new_game.surface;
	delete m_return_game.surface;
	delete m_exit_item.surface;
	delete m_title;
	delete m_cursor;
	delete m_frog_sound;
}



// updates the menu
void MenuUpdate()
{	
	// cache the title pos
	static int32 title_xpos = 320-(m_title->GetWidth()/2);
	
	PrintXfrogVersion();

	// blit the title
	m_title->Blit(title_xpos, 90, false);
	
	// blit the menu itens
	m_new_game.surface->Blit(m_new_game.x, m_new_game.y, false);
	m_return_game.surface->Blit(m_return_game.x, m_return_game.y, false);
	m_exit_item.surface->Blit(m_exit_item.x, m_exit_item.y, false);

	// check for user input
	CheckInput();

	// update the menu with the right state
	switch(m_curr_selection)
	{

		case MENU_NEW_GAME:
			{
				// draws the cursor
				m_cursor->Blit(m_new_game.x - 42, m_new_game.y, false);
				
				// the user has selected this item
				if(m_selected)
				{
					m_new_game.func();
					m_selected = false;
				}

			} break;

		case MENU_RETURN_GAME:
			{
				// draws the cursor
				m_cursor->Blit(m_return_game.x - 42, m_return_game.y, false);

				// the user has selected this item
				if(m_selected)
				{
					m_return_game.func();
					m_selected = false;
				}

			} break;

		case MENU_EXIT:
			{
				// draws the cursor
				m_cursor->Blit(m_exit_item.x - 42, m_exit_item.y, false);
				
				// the user has selected this item
				if(m_selected)
				{
					// call the exit function
					m_exit_item.func();
					m_selected = false;
				}

			} break;

		default: break;
	}	
}


// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


// play the game
static void MenuItemNewGame()
{
	g_engine_state = GAME;
	g_game_state   = STARTING;
}



// return to game
static void MenuItemReturnGame()
{
	// there is no game to return
	if(g_is_playing == false)
	{
		g_engine_state = MENU;
		return;
	}
	else
	{
		// return to game
		g_engine_state = GAME;
		g_game_state   = PLAYING;
	}
}


// exits the game
static void MenuItemExit()
{	
	g_engine_state = EXIT;
}


// checks for user input
static void CheckInput()
{
	if(InputCheckKey(KEY_UP, TICK))
	{
		m_curr_selection--;
		m_frog_sound->Play(false, LOW);
	}

	if(InputCheckKey(KEY_DOWN, TICK))
	{
		m_curr_selection++;
		m_frog_sound->Play(false, LOW);
	}

	if(m_curr_selection < MENU_NEW_GAME)
		m_curr_selection = MENU_NEW_GAME;
	else if(m_curr_selection > MENU_EXIT)
		m_curr_selection = MENU_EXIT;

	// the user has selected the option
	if(InputCheckKey(KEY_RETURN, DOWN))
		m_selected = true;
}


// print the xfrog version on the bottom right of the screen
static void PrintXfrogVersion()
{
	static int32 x = 640-((strlen(XFROG_VERSION) + 2) << 3);
	Printf(x, 472, "v%s", XFROG_VERSION);
}


