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

  Date:			04/02/2000

  Description:	Controls the game logic.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <search.h>

#include "types.h"
#include "error.h"
#include "video.h"
#include "font.h"
#include "sound.h"
#include "game.h"
#include "engine.h"
#include "input.h"
#include "tga.h"
#include "car.h"
#include "thief.h"
#include "player.h"
#include "hud.h"
#include "mem_man.h"
#include "item.h"
#include "timer.h"
#include "highscore.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool GameInit();
void GameShutdown();
void GameUpdate();


static void PrintStartingInfo();
static void StartingUpdate();
static void PlayingUpdate();
static void SortZ_Entities();
static int32 EntCompare(const void* elem1, const void* elem2);
static inline void DrawAllEnt();
static void DrawRoads();
static void DrawSidewalkAndBuildings();
static inline void DrawPizzaria();
static void DrawParking();
static void PizzaSpotsInit();
static bool BallonInit();
static void BallonShutdown();
static void BallonUpdate();
static void GameOverUpdate();
static void DeliverSpotUpdate();
static void PickUpDeliverSpot();



// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// check if the player is playing
bool				g_is_playing	= false;

// the game state
game_state_t		g_game_state	= STARTING;


// check to see if there is any active deliver spot
bool				g_has_spot		= false;


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////



// the buildings
static Surface*			m_buildings		= NULL;

// the sidewalk
static Surface*			m_sidewalk		= NULL;


// the road surface
static Surface*			m_road			= NULL;


// the pizza parking floor
static Surface*			m_parking		= NULL;

// the parking trees
static Surface*			m_parking_tree  = NULL;

// the pizzaria
static Surface*			m_pizzaria		= NULL;


// the places to deliver the pizza
static pizza_spot_t		m_pizza_spots[PIZZA_SPOTS];

// the current spot to deliver
static pizza_spot_t*	m_curr_spot  = NULL;

// ballon surface
static Surface*			m_here_ballon	= NULL;


// the here sound
static SoundBuffer*		m_here_sound	= NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inits the game data
bool GameInit()
{
	bool result;

	// init the HUD
	result = HUDInit();
	if(result == false)
	{   
		Error error("Error trying to init the HUD\n");
		error.Report();

		return(false);
	}

	// loads the builings
	m_buildings = LoadTGA("gfx/env/buildings.tga", MEM_SYSTEM, NULL);
	if(m_buildings == NULL)
	{
		Error error("Error trying to load the buildings\n");
		error.Report();

		return(false);
	}

	// loads the sidewalk
	m_sidewalk = LoadTGA("gfx/env/sidewalk.tga", MEM_SYSTEM, NULL);
	if(m_sidewalk == NULL)
	{
		Error error("Error trying to load the sidewalk\n");
		error.Report();

		return(false);
	}

	// loads the road
	m_road = LoadTGA("gfx/env/road.tga", MEM_SYSTEM, NULL);
	if(m_road == NULL)
	{
		Error error("Error trying to load the road\n");
		error.Report();

		return(false);
	}

	// loads the pizzaria
	m_pizzaria = LoadTGA("gfx/env/pizzamario.tga", MEM_SYSTEM, NULL);
	if(m_pizzaria == NULL)
	{
		Error error("Error trying to load the pizzaria\n");
		error.Report();

		return(false);
	}

	// loads the parking
	m_parking = LoadTGA("gfx/env/parkingfloor.tga", MEM_SYSTEM, NULL);
	if(m_parking == NULL)
	{
		Error error("Error trying to load the parking texture\n");
		error.Report();

		return(false);
	}

	// loads the parking tree
	m_parking_tree = LoadTGA("gfx/env/tree.tga", MEM_SYSTEM, NULL);
	if(m_parking_tree == NULL)
	{
		Error error("Error trying to load the parking slot\n");
		error.Report();

		return(false);

	}

	// loads the here sound
	m_here_sound = new SoundBuffer;
	result = m_here_sound->LoadWAV("sounds/env/here.wav");
	if(result == false)
	{
		Error error("Error trying to load the \"here\" sound\n");
		error.Report();

		return(false);
	}

	// init entity list
	GameEnt::InitEntList();

	// init cars
	result = CarsInit();
	if(result == false)
	{
		return(false);
	}

	// init items
	result = ItemsInit();
	if(result == false)
	{
		return(false);
	}

	// init thief
	result = ThiefInit();
	if(result == false)
	{
		return(false);
	}

	// init the player
	result = PlayerInit();
	if(result == false)
	{
		return(false);
	}

	// init the timer to deliver the pizza
	TimeLeftInit();

	// init the pizza spots
	PizzaSpotsInit();

	// init the here ballon
	result = BallonInit();
	if(result == false)
		return(false);
	

	// init high score system 
	result = HighScoreInit();
	if(result == false)
		return(false);

	return(true);
}


void GameShutdown()
{
	HighScoreShutdown();
	BallonShutdown();
	PlayerShutdown();
	ThiefShutdown();
	ItemsShutdown();
	CarsShutdown();
	
	if(m_buildings != NULL)
		delete m_buildings;

	if(m_sidewalk != NULL)
		delete m_sidewalk;

	if(m_road != NULL)
		delete m_road;

	if(m_pizzaria != NULL)
		delete m_pizzaria;

	if(m_parking != NULL)
		delete m_parking;

	if(m_parking_tree != NULL)
		delete m_parking_tree;

	if(m_here_sound != NULL)
		delete m_here_sound;

	HUDShutdown();
}

// updates the game
void GameUpdate()
{
	switch(g_game_state)
	{		
		case STARTING:
			{

				StartingUpdate();

			} break;

		case PLAYING:
			{

				PlayingUpdate();

			} break;

		
		case GAME_OVER:
			{

				GameOverUpdate();

			} break;

		default: break;
	}	
}



// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


// prints intro info
static void PrintStartingInfo()
{
	PrintCenter(10, "BE WELCOME TO FROG CITY!");
	PrintCenter(26, "You are a pizza deliverer and your work is to deliver as many pizzas as you can.");
	PrintCenter(34, "But it isn't that simple. You'll have hard obstacles to pass.");
	PrintCenter(42, "On Frog City drivers are crazy frogs that run over on all things that cross");
	PrintCenter(50, "their path.");
	PrintCenter(58, "Be careful with the thieves too. Don't cross their way while they are passing.");
	PrintCenter(66, "Otherwise they will steal your pizza and you'll lose money.");
	PrintCenter(74, "People on Frog City love pizzas. So you have little time to deliver the pizzas.");
	PrintCenter(82, "Twenty seconds is your time. So get this work quick !!!");
	PrintCenter(90, "Pay attention to items that may appear on the street. They'll be useful to you.");
	PrintCenter(106, "GOOD LUCK !!!");

	PrintCenter(472, "(Press SPACE to continue ...)");
}



// updates start state
static void StartingUpdate()
{
	PrintStartingInfo();

	if(InputCheckKey(DIK_SPACE, TICK))
	{
		g_game_state = PLAYING;

		// inits the states of the entities
		g_player->ReInit();
		CarsReInit();
		ItemsReInit();
		g_thief->ReInit();		

		// the game is running
		g_is_playing = true;
		
		// no spot
		g_has_spot  = false;
	}
	else if(InputCheckKey(DIK_ESCAPE, TICK))
	{
		g_engine_state = MENU;
	}
}




// updates the gameplay
static void PlayingUpdate()
{
	if(InputCheckKey(DIK_ESCAPE, DOWN))
	{
		g_engine_state = MENU;	
	}
	
	// draw the roads
	DrawRoads();
	
	// draw the sidewalk and the buildings
	DrawSidewalkAndBuildings();

	// draw the parking
	DrawParking();

	// draw the pizzaria
	DrawPizzaria();

	// updates and draw the items state
	ItemsUpdate();

	// draw the here ballon
	BallonUpdate();

	// draw the player hud
	HUDUpdate();

	// draws the high score
	HighScoreUpdate();

	// pick up a deliver spot
	DeliverSpotUpdate();

	// updates player
	PlayerUpdate();

	// draw the thief
	ThiefUpdate();

	// draw and update cars
	CarsUpdate();

	// sort the order of drawing
	SortZ_Entities();

	// draw all entities
	DrawAllEnt();	
}



// sort the list of entities to draw
static void SortZ_Entities()
{
	// sort the game entities
	qsort((void*) &GameEnt::m_ent_list[0],
		  GameEnt::m_num_ent,
		  sizeof(GameEnt*),
		  EntCompare);
}



// compare two game entites for the sorting
static int32 EntCompare(const void* elem1, const void* elem2)
{	
	
	// gets the entities
	GameEnt** ent1 = (GameEnt**) elem1;
	GameEnt** ent2 = (GameEnt**) elem2;
	
	if((*ent1)->m_y < (*ent2)->m_y)
	{
		return(-1);
	}
	else if((*ent1)->m_y > (*ent2)->m_y)
	{
		return(1);
	}
	else if((*ent1)->m_y == (*ent2)->m_y)
	{
		return(0);	
	}
	else
		return(0);	
}


// draw all entities
static inline void DrawAllEnt()
{
	// draw all the game entities
	GameEnt** curr_ptr = &GameEnt::m_ent_list[0];
	for(;*curr_ptr != NULL; curr_ptr++)
	{
		(*curr_ptr)->Draw();		
		
	}
}


static void DrawRoads()
{
	// draw the roads
	for(int32 i = 0; i < 10; i++)
	{
		m_road->Blit((i<<6), ROAD_1, false);
		m_road->Blit((i<<6), ROAD_2, false);
		m_road->Blit((i<<6), ROAD_3, false);
		m_road->Blit((i<<6), ROAD_4, false);	
	}	
}


static void DrawSidewalkAndBuildings()
{
	// draw the buildings
	m_buildings->Blit(0, 0, false);

	// draw the sidewalk
	for(int32 i = 0; i < 20; i++)
		m_sidewalk->Blit((i<<5), m_buildings->GetHeight(), false);
}



// draw the pizzaria
static inline void DrawPizzaria()
{
	m_pizzaria->Blit(PIZZA_X, PIZZA_Y, true);
}


// draw the pizza parking
static void DrawParking()
{
	// draw
	for(int32 y = 366; y < 480; y += 32)
	{
		for(int32 x = 0; x < 640; x += 32)
		{
			m_parking->Blit(x, y, false);
		}
	}

	// draw the trees
	for(int32 y = 448; y < 480; y += 16)
	{		
		for(int32 x = 0; x < 640; x += 16)
		{
			m_parking_tree->Blit(x, y, true);
		}
	}
}


// init the pizza spots
static void PizzaSpotsInit()
{
	Memset(m_pizza_spots, 0, sizeof(m_pizza_spots));

	// now init the values
	m_pizza_spots[0].x			= 28;
	m_pizza_spots[0].y			= 145;
	m_pizza_spots[0].width		= 50;
	m_pizza_spots[0].height		= 35;

	m_pizza_spots[1].x			= 175;
	m_pizza_spots[1].y			= 145;
	m_pizza_spots[1].width		= 45;
	m_pizza_spots[1].height		= 35;

	m_pizza_spots[2].x			= 314;
	m_pizza_spots[2].y			= 145;
	m_pizza_spots[2].width		= 45;
	m_pizza_spots[2].height		= 35;

	m_pizza_spots[3].x			= 429;
	m_pizza_spots[3].y			= 145;
	m_pizza_spots[3].width		= 38;
	m_pizza_spots[3].height		= 35;

	m_pizza_spots[4].x			= 552;
	m_pizza_spots[4].y			= 145;
	m_pizza_spots[4].width		= 42;
	m_pizza_spots[4].height		= 35;
}




// init the here ballon
static bool BallonInit()
{	
	m_here_ballon = LoadTGA("gfx/env/here.tga", MEM_SYSTEM, NULL);
	if(m_here_ballon == NULL)
	{
		Error error("Error trying to load the ballon\n");
		error.Report();
		
		return(false);
	}	

	return(true);
}


// shutsdown the here ballon
static void BallonShutdown()
{
	if(m_here_ballon)
	{		
		delete m_here_ballon;
	}
}


// updates the ballon
static void BallonUpdate()
{	
	// show the ballon while the player has the pizza
	if(g_has_spot == true)
	{
		m_here_ballon->Blit(m_curr_spot->x-17, m_curr_spot->y-17, true);
	}
}



// game over
static void GameOverUpdate()
{
	// the player is not playing any more
	g_is_playing = false;

	// print game over info
	PrintCenter(20, "GAME OVER");
	PrintCenter(36, "You are FIRED !. You've failed in delivering the pizzas !");
	PrintCenter(44, "You've scored: %d", g_player->Score());
	PrintCenter(52, "Try again !");
	PrintCenter(472,"Press ESC to return to the menu");

	// check for user input
	if(InputCheckKey(DIK_ESCAPE, DOWN))
	{
		g_engine_state = MENU;
		g_game_state   = STARTING;
	}
}



// updates the delivering place logic
static void DeliverSpotUpdate()
{
	// if it has no pizza spot, pick up one
	if(g_has_spot == false)
	{
		PickUpDeliverSpot();
		g_has_spot = true;
	}		
}



// sets a deliver spot to the player
static void PickUpDeliverSpot()
{ 
	int32 pizza_spot = rand()%PIZZA_SPOTS;

	// let the player deliver it
	g_player->DeliverPizzaTo(&m_pizza_spots[pizza_spot]);

	m_curr_spot = &m_pizza_spots[pizza_spot];

	// plays the sound
	m_here_sound->Play(false, NORMAL);	
}
