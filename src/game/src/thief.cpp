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

  Date:			07/02/2000

  Description:	Thief of Xfrog

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <time.h>

#include "types.h"
#include "ent.h"
#include "thief.h"
#include "tga.h"
#include "mem_man.h"
#include "error.h"
#include "timer.h"
#include "player.h"
#include "car.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool ThiefInit();
void ThiefShutdown();
void ThiefUpdate();



// GLOBALS ////////////////////////////////////////////////////////////////////////////////////



// the thief
Thief*			 g_thief = NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// init the thief
bool ThiefInit()
{
	bool result;

	g_thief = new Thief;

	// load the frames
	result = g_thief->LoadFrames();
	if(result == NULL)
	{
		Error error("Error trying to load thief frames\n");
		error.Report();

		return(false);
	}


	return(true);
}


// shutsdown the thief
void ThiefShutdown()
{
	if(g_thief)
		delete g_thief;
}


// updates the thief
void ThiefUpdate()
{
	g_thief->UpdateState();
	
}


// THIEF CLASS IMP ////////////////////////////////////////////////////////////////////////////



Thief::Thief()
	  : m_counter(0),
		m_draw(NULL),
		m_curr_frame(0),
		m_appear_counter(0),
		m_out_the_scene(false),
		m_total_time_to_appear(TIME_TO_APPEAR),
		m_has_pizza(false)
{
	for(int32 i = 0; i < NUM_THIEF_FRAMES; i++)
	{
		m_surface[i]	   = NULL;	
		m_surface_pizza[i] = NULL;
	}	

	m_speed_x = THIEF_SPEED;
	
	m_x = -32.0f;
	m_y = 167.0f;

	// adds entity to the list
	AddEntity(this);
}



Thief::~Thief()
{
	for(int32 i = 0; i < NUM_THIEF_FRAMES; i++)
	{
		if(m_surface[i])
		{
			delete m_surface[i];
			m_surface[i] = NULL;
		}

		if(m_surface_pizza[i])
		{
			delete m_surface_pizza[i];
			m_surface_pizza[i] = NULL;
		}
	}
}



void Thief::UpdateState()
{
	// time stopped, return
	if(g_cars[0]->GetAttribute() & CAR_ATTR_STOP)
		return;

	float delta =  g_timer->GetDelta();	

	// check if is time to appear and if it is in the scene
	m_appear_counter += delta;
	if(m_appear_counter < m_total_time_to_appear && m_out_the_scene == true)
		return;	
	
	// updates counter to frame animation
	m_counter	+= (delta * THIEF_FPS);
	
	// time to advance the frame
	if(m_counter >= 1.0f)
	{
		m_curr_frame += (m_counter);

		if(m_curr_frame >= NUM_THIEF_FRAMES-1)
			m_curr_frame = 0;

		m_counter = 0;
	}

	// pick the right frame
	m_draw = m_has_pizza ? m_surface_pizza[(int32)m_curr_frame]:m_surface[(int32)m_curr_frame];
	
	// updates the thief position
	m_x += (m_speed_x*delta);

	// he is out of the scene, reset his state
	if(m_x >= 640)
	{
		m_x = -32;

		// reset the thief
		m_out_the_scene  = true;
		m_appear_counter = 0;
		// time to wait for the next appearance
		m_total_time_to_appear = (float) (TIME_TO_APPEAR + rand()%8);

		// he has eaten the pizza
		m_has_pizza = false;
	}

	// check if the player has the pizza
	if(g_player->HasPizza())
	{
		// only check for collison if the thief has no pizza
		if(m_has_pizza == false)
		{
			// check for the collision
			bool collision = CheckCollisionWithPlayer();
			if(collision == true)
			{
				// stolen pizza
				g_player->SetDeliveringState(STOLEN);
		
				// now the thief has the pizza
				m_has_pizza = true;
			}
		}
	}

//	Draw();
}



void Thief::Draw()
{
	if(m_draw == NULL)
		return;
	

	if(IsOnScreen())
		m_draw->Blit((int32) m_x, (int32) m_y, true);
}



void Thief::ReInit()
{
	m_counter				= 0;
	m_curr_frame			= 0;
	m_appear_counter		= 0;
	m_out_the_scene			= false;
	m_total_time_to_appear	= TIME_TO_APPEAR;
	m_has_pizza				= false;

	m_speed_x = THIEF_SPEED;
	
	m_x = -32.0f;
	m_y = 167.0f;

}

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


// load all the thief animation frames
bool Thief::LoadFrames()
{
	// loads the image template
	Surface* template_surf = LoadTGA("gfx/thief/thief.tga", MEM_SYSTEM, NULL);
	if(template_surf == NULL)
	{
		return(false);
	}

	// points to data
	byte* template_buffer = template_surf->Lock();	

	// width and height of a single frame
	int32 width  = (template_surf->GetWidth()/NUM_THIEF_FRAMES);
	int32 height = 32;

	int32 curr_template_x = 0;

	// now extract frames from the template
	for(int32 i = 0; i < NUM_THIEF_FRAMES; i++)
	{		

		// creates the surface to hold the frame
		m_surface[i] = new Surface;		
		if(m_surface[i]->Create(width, height, MEM_SYSTEM) == NULL)
			continue;			

		// lock
		byte* frame_buffer  = m_surface[i]->Lock();
		byte* frame_ptr     = frame_buffer;
		
		// gets the right image start point
		byte* template_ptr	  = &template_buffer[curr_template_x];

		// copies the image
		for(int32 y = 0; y < height; y++)
		{
			// copies one line of the frame
			Memcpy(frame_ptr, template_ptr, width);

			frame_ptr		+= m_surface[i]->GetPitch();
			template_ptr	+= template_surf->GetPitch();
		}

		m_surface[i]->Unlock();

		// advances to the next frame on the template
		curr_template_x += width;
	}

	template_surf->Unlock();
	delete template_surf;

	// loads the image template
	template_surf = LoadTGA("gfx/thief/thiefp.tga", MEM_SYSTEM, NULL);
	if(template_surf == NULL)
	{
		return(false);
	}

	// points to data
	template_buffer = template_surf->Lock();

	curr_template_x = 0;
	// now extract frames from the template, with pizza
	for(int32 i = 0; i < NUM_THIEF_FRAMES; i++)
	{		

		// creates the surface to hold the frame
		m_surface_pizza[i] = new Surface;		
		if(m_surface_pizza[i]->Create(width, height, MEM_SYSTEM) == NULL)
			continue;			

		// lock
		byte* frame_buffer  = m_surface_pizza[i]->Lock();
		byte* frame_ptr     = frame_buffer;
		
		// gets the right image start point
		byte* template_ptr	  = &template_buffer[curr_template_x];

		// copies the image
		for(int32 y = 0; y < height; y++)
		{
			// copies one line of the frame
			Memcpy(frame_ptr, template_ptr, width);

			frame_ptr		+= m_surface_pizza[i]->GetPitch();
			template_ptr	+= template_surf->GetPitch();
		}

		m_surface_pizza[i]->Unlock();

		// advances to the next frame on the template
		curr_template_x += width;
	}

	template_surf->Unlock();
	delete template_surf;

	m_width  = width;
	m_height = height;

	// ok !
	return(true);
}




bool Thief::CheckCollisionWithPlayer()
{	
	// test for collison plus 10, for sidewalk coverage
	if((m_x >= g_player->X() && m_x <= g_player->X()+g_player->Width()) &&
	   (m_y+10 >= g_player->Y() && m_y <= g_player->Y()+g_player->Height()+10))
		return(true);
	else
		return(false);
}