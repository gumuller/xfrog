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

  Date:			14/02/2000

  Description:	Xfrog items.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "item.h"
#include "tga.h"
#include "error.h"
#include "mem_man.h"
#include "timer.h"
#include "car.h"
#include "sound.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool ItemsInit();
void ItemsShutdown();
void ItemsUpdate();
void ItemsReInit();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// points to the current item on the world
Item*				g_curr_item = NULL;

Item*				g_items[NUM_OF_ITEMS];


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// time accumulator
static float			m_time_accum		= 0;


// the item pick sound
static SoundBuffer*		m_beep				= NULL;		


// a item probability table
static int32	m_item_probability[] = { 0, 0, 0, 1, 1, 2, 2, 4, 4, 3 };


// size of the probability table
static int32 	 m_item_prob_size=(sizeof(m_item_probability)/sizeof(m_item_probability[0]));


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////

// init the game items
bool ItemsInit()
{
	// loads the item sound
	m_beep = new SoundBuffer;
	bool result = m_beep->LoadWAV("sounds/item/item.wav");
	if(result == false)
		return(false);



	// load the items
	g_items[0] = new Food(FLOUR_AWARD);
	g_items[0]->m_surface = LoadTGA("gfx/item/flour.tga", MEM_SYSTEM, NULL);
	if(g_items[0] == NULL)
		return(false);
	g_items[0]->m_width   = g_items[0]->m_surface->GetWidth();
	g_items[0]->m_height  = g_items[0]->m_surface->GetHeight();
	g_items[0]->m_cwidth  = (int32)(g_items[0]->m_width  * 0.80f);
	g_items[0]->m_cheight = (int32)(g_items[0]->m_height * 0.80f);



	g_items[1] = new Food(TOMATO_AWARD);
	g_items[1]->m_surface = LoadTGA("gfx/item/tomatoes.tga", MEM_SYSTEM, NULL);
	if(g_items[1] == NULL)
		return(false);
	g_items[1]->m_width   = g_items[1]->m_surface->GetWidth();
	g_items[1]->m_height  = g_items[1]->m_surface->GetHeight();
	g_items[1]->m_cwidth  = (int32)(g_items[1]->m_width  * 0.80f);
	g_items[1]->m_cheight = (int32)(g_items[1]->m_height * 0.80f);




	g_items[2] = new Food(CHEESE_AWARD);
	g_items[2]->m_surface = LoadTGA("gfx/item/cheese.tga", MEM_SYSTEM, NULL);
	if(g_items[2] == NULL)
		return(false);
	g_items[2]->m_width   = g_items[2]->m_surface->GetWidth();
	g_items[2]->m_height  = g_items[2]->m_surface->GetHeight();
	g_items[2]->m_cwidth  = (int32)(g_items[2]->m_width  * 0.80f);
	g_items[2]->m_cheight = (int32)(g_items[2]->m_height * 0.80f);


	
	g_items[3] = new Stop;
	g_items[3]->m_surface = LoadTGA("gfx/item/stop.tga", MEM_SYSTEM, NULL);
	if(g_items[3] == NULL)
		return(false);	
	g_items[3]->m_width   = g_items[3]->m_surface->GetWidth();
	g_items[3]->m_height  = g_items[3]->m_surface->GetHeight();
	g_items[3]->m_cwidth  = (int32)(g_items[3]->m_width  * 0.80f);
	g_items[3]->m_cheight = (int32)(g_items[3]->m_height * 0.80f);



	g_items[4] = new Speed;
	g_items[4]->m_surface = LoadTGA("gfx/item/speed.tga", MEM_SYSTEM, NULL);
	if(g_items[4] == NULL)
		return(false);	
	g_items[4]->m_width   = g_items[4]->m_surface->GetWidth();
	g_items[4]->m_height  = g_items[4]->m_surface->GetHeight();
	g_items[4]->m_cwidth  = (int32)(g_items[4]->m_width  * 0.80f);
	g_items[4]->m_cheight = (int32)(g_items[4]->m_height * 0.80f);



	return(true);
}



// shutdown the items
void ItemsShutdown()
{
	for(int32 i = 0; i < NUM_OF_ITEMS; i++)
	{
		if(g_items[i])
			delete g_items[i];
	}

	// delete sounds
	if(m_beep)
		delete m_beep;
}


// update the items state
void ItemsUpdate()
{
	// add time
	m_time_accum += g_timer->GetDelta();

	// time has reached the limit
	if(m_time_accum >= TIME_TO_SPAWN_ITEM)
	{
		m_time_accum = 0;
		
		// no item on map
		if(g_curr_item == NULL)
		{
			int32 item_num = m_item_probability[(rand()%m_item_prob_size)];
			g_curr_item = g_items[item_num];

			// set item position
			g_items[item_num]->m_x=(float)(ITEM_SPAWN_AREA_X+(rand()%ITEM_SPAWN_AREA_WIDTH));
			g_items[item_num]->m_y=(float)(ITEM_SPAWN_AREA_Y+(rand()%ITEM_SPAWN_AREA_HEIGHT));
		}
		// there is a item on the map
		else
		{
			// reset the picked state
			g_curr_item->m_picked_up = false;

			// clear the item and wait for next item spawn
			g_curr_item = NULL;
		}
		
	}

	// try to upddate the current item if there is a item on the world
	if(g_curr_item != NULL)
	{
		// if the item was not picked up, update and draw it
		if(g_curr_item->PickedUp() == false)
		{
			g_curr_item->UpdateState();
			g_curr_item->Draw();
		}		
	}
}


// reinit items
void ItemsReInit()
{
	// reset the itens
	for(int32 i = 0; i < NUM_OF_ITEMS; i++)
	{
		g_items[i]->m_picked_up = false;		
	}

	// reset external data
	g_curr_item				= NULL;
	m_time_accum			= 0;
}



// ITEM CLASS IMP /////////////////////////////////////////////////////////////////////////////


Item::Item()
     : m_surface(NULL),
	   m_picked_up(false)
{
}



Item::~Item()
{
	if(m_surface)
		delete m_surface;
}



void Item::Draw()
{
	if(m_surface == NULL)
		return;

	if(IsOnScreen())
		m_surface->Blit((int32) m_x, (int32) m_y, true);
}


// FOOD CLASS IMP ////////////////////////////////////////////////////////////////////////////



Food::Food(int32 award)
	 : m_award(award)
{	
}



Food::~Food()
{

}



void Food::UpdateState()
{
	// if collided with the player, give him points
	if(CheckCollisionWithPlayer())
	{
		// plays sound
		m_beep->Play(false, NORMAL);

		// give points to player
		g_player->GivePoints(m_award);

		// the item was picked up
		m_picked_up = true;	
	}	
}



// STOP CLASS IMP /////////////////////////////////////////////////////////////////////////////



Stop::Stop()
{
}



Stop::~Stop()
{
}



void Stop::UpdateState()
{
	// if collided with the player, stop the cars
	if(CheckCollisionWithPlayer())
	{
		// plays sound
		m_beep->Play(false, NORMAL);

		// set the stop attribute to the cars
		CarsSetAttribute(CAR_ATTR_STOP);

		// the item was picked up
		m_picked_up = true;
	}
}



// SPEED CLASS IMP ////////////////////////////////////////////////////////////////////////////



void Speed::UpdateState()
{
	// if the player collided with the item, make him fast !
	if(CheckCollisionWithPlayer())
	{
		// plays sound
		m_beep->Play(false, NORMAL);

		// set player speed attribute
		g_player->SetAttribute(PLAYER_ATTR_SPEED);

		// the item was picked up
		m_picked_up = true;
	}
}