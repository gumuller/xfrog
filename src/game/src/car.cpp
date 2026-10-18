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

  Date:			05/02/2000

  Description:	Module that handle the cars.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "car.h"
#include "tga.h"
#include "mem_man.h"
#include "timer.h"
#include "game.h"
#include "player.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool CarsInit();
void CarsShutdown();
void CarsUpdate();
void CarsSetAttribute(const int32 attr);
void CarsResetAttribute(const int32 attr);
void CarsReInit();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the game cars
Car*			g_cars[MAX_CARS];


// the speed rate of the cars when the game becomes more difficult
float			g_fast_factor = 1.0f;


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// the car names
static char*	m_car_names[]	= { "car00.tga",
									"car01.tga",
								    "car02.tga",
									"car03.tga",
								    "bus00.tga",
									"truck00.tga",
									"minitruck00.tga" };

// number of car types
static int32	m_num_car_types = (sizeof(m_car_names)/sizeof(m_car_names[0]));


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// init the game vehicles
bool CarsInit()
{
	// init the cars
	for(int32 i = 0; i < MAX_CARS; i++)
	{
		// pick a car type
		int32 car_type = (rand()%m_num_car_types);

		// setup the file name
		static char name[64];
		Memset(name, '\0', sizeof(name));
		strcpy(name, "gfx/cars/");
		strcat(name, m_car_names[car_type]);

		g_cars[i] = new Car;
		g_cars[i]->Load(name);
	}

	int32 cars_per_road = (int32) (MAX_CARS/NUM_OF_ROADS);
	int32 cars_inited   = 0;
	int32 car_x;

	// put cars on the roads
	for(int32 road_num = 0; road_num < NUM_OF_ROADS; road_num++)
	{	
		// the car start offset
		car_x = -64;

		// init the car positon according to the road
		for(int32 i = cars_inited; i < cars_per_road*(road_num+1); i++)
		{
			if(road_num == 0)
			{
				g_cars[i]->m_y				= (float) ROAD_1;
				g_cars[i]->m_speed_factor	= -1.8f;
				g_cars[i]->m_surface->Mirror();
			}
			else if(road_num == 1)
			{
				g_cars[i]->m_y				= (float) ROAD_2;
				g_cars[i]->m_speed_factor	= 1.6f;
			}
			else if(road_num == 2)
			{
				g_cars[i]->m_y				= (float) ROAD_3;
				g_cars[i]->m_speed_factor	= -1.4f;
				g_cars[i]->m_surface->Mirror();
			}
			else if(road_num == 3)
			{
				g_cars[i]->m_y				= (float) ROAD_4;
				g_cars[i]->m_speed_factor	= 1.2f;
			}

			// assign the car position
			g_cars[i]->m_x     = (float) car_x;

			// advance the car position
			car_x += 164;
		}

		// advance to the other cars
		cars_inited += cars_per_road;
	}

	return(true);
}


void CarsShutdown()
{
	// shutsdown the cars
	for(int32 i = 0; i < MAX_CARS; i++)
	{
		delete g_cars[i];		
	}
}


// updates all cars of the game
void CarsUpdate()
{
	static float time_accum = 0;

	// if the cars has the stop attribute, start counting until the effect 
	// has gone
	if(g_cars[0]->GetAttribute() & CAR_ATTR_STOP)
	{
		time_accum += g_timer->GetDelta();

		if(time_accum >= CAR_STOP_TIME)
		{
			time_accum = 0;
			CarsResetAttribute(CAR_ATTR_STOP);
		}
	}

	// updates the car state
	for(int32 i = 0; i < MAX_CARS; i++)
	{		
		g_cars[i]->UpdateState();	
	}	
}


// set attributes for all cars
void CarsSetAttribute(const int32 attr)
{
	for(int32 i = 0; i < MAX_CARS; i++)
	{
		g_cars[i]->m_attr |= attr;
	}
}


// reset attributes of all cars
void CarsResetAttribute(const int32 attr)
{
	for(int32 i = 0; i < MAX_CARS; i++)
	{
		g_cars[i]->m_attr &= ~attr;
	}
}



void CarsReInit()
{
	g_fast_factor = 1.0f;
	CarsResetAttribute(CAR_ATTR_STOP);	
}


// CAR CLASS IMP //////////////////////////////////////////////////////////////////////////////


Car::Car()
    : m_speed_factor(1.0f),
	  m_surface(NULL),
	  m_attr(CAR_ATTR_NONE)
{
	 // adds entity to the list
	AddEntity(this);
}


Car::~Car()
{
	if(m_surface != NULL)
	{
		delete m_surface;
	}
}



void Car::UpdateState()
{	
	// only move the cars if the cars don't have the stop attribute
	if(!(m_attr & CAR_ATTR_STOP))
	{
		// updates car speed
		m_speed_x = CAR_SPEED * m_speed_factor * g_timer->GetDelta() * g_fast_factor;
		m_x       += m_speed_x;

		// end of the screen
		if(m_x+m_width < 0)
		{
			m_x = 639;
		}
		else if(m_x > 640)
		{
			m_x = (float) (-m_width);
		}
	}	

	// if the player is not dead, check for collision
	if(g_player->m_state != DEAD)
	{
		// check for collision a death collision
		bool collision = CheckCollisionWithPlayer();
		if(collision == true)
		{
			// only kill the player if the car is moving
			if(!(m_attr & CAR_ATTR_STOP))
			{
				// dead player
				g_player->m_state = DEAD;
	
				if(g_player->HasPizza())
				{
					// delivering unsuccessfull
					g_player->m_deliver_state = RUNNED_OVER;
				}
				
				// plays the death sound
				g_player->PlayDeathSound();
			}
			
		}
	}
}



// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////



bool Car::CheckCollisionWithPlayer()
{
	// get the center coordinates from the car
	int32 width1  = (m_cwidth   >> 1);
	int32 height1 = (m_cheight  >> 1);

	// get the center coordinates from the player
	int32 width2  = (g_player->m_cwidth   >> 1);
	int32 height2 = (g_player->m_cheight  >> 1);

	// translate it to world coordinates, plus 25 
	// to correct perspective collision
	int32 c_wx1 = (width1  + (int32) m_x);
	int32 c_wy1 = (height1 + ((int32) m_y) + 23);

	// translate it to world coordinates
	int32 c_wx2 = (width2  + (int32) g_player->m_x);
	int32 c_wy2 = (height2 + (int32) g_player->m_y);

	// find the distance between the centers
	int32 delta_x = abs(c_wx2 - c_wx1);
	int32 delta_y = abs(c_wy2 - c_wy1);

	// check for collision
	if(delta_x < (width1+width2) && delta_y < (height1+height2))
		return(true);
	else 
		return(false);
}


bool Car::Load(char* filename)
{
	m_surface = LoadTGA(filename, MEM_SYSTEM, NULL);
	if(m_surface == NULL)
	{
		return(false);
	}

	m_width  = m_surface->GetWidth();
	m_height = m_surface->GetHeight();

	// collision dimensions
	m_cwidth  = (int32) (m_width *0.90f);
	m_cheight = (int32) (m_height*0.01f);

	return(true);
}