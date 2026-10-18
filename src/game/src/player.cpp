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

  Date:			08/02/2000

  Description:	This module contains functions that deal with
				all player actions.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "video.h"
#include "player.h"
#include "error.h"
#include "mem_man.h"
#include "input.h"
#include "sound.h"
#include "timer.h"
#include "tga.h"
#include "game.h"
#include "hud.h"
#include "car.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool PlayerInit();
void PlayerShutdown();
void PlayerUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the player
Player*			g_player = NULL;


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// the animation sequencies
static int32 m_walking_right[] =  { 0,  1,  2,  3,  4,  5,  6,  7 };
static int32 m_walking_left[]  =  { 8,  9,  10, 11, 12, 13, 14, 15 };
static int32 m_walking_down[]  =  { 16, 17, 18 };
static int32 m_walking_up[]	   =  { 19, 20, 21 };


// handles the bonus lives counting
static int32 m_old_score   = 0;
static int32 m_score_accum = 0;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inits the player
bool PlayerInit()
{
	bool result;

	g_player = new Player;

	// loads player frames
	result = g_player->LoadFrames();
	if(result == false)
	{
		Error error("Error trying to load player frames\n");
		error.Report();

		return(false);
	}

	// loads the player sounds
	result = g_player->LoadSounds();
	if(result == false)
		return(false);

	return(true);
}


// shutsdown the player
void PlayerShutdown()
{
	delete g_player;
}


// updates the palyer
void PlayerUpdate()
{
	g_player->UpdateState();
}


// PLAYER CLASS IMP ///////////////////////////////////////////////////////////////////////////


Player::Player()
	   : m_state(STAND_RIGHT),
	     m_speed(PLAYER_SPEED),
		 m_fast(1.0f),
	     m_speed_y(0),
		 m_draw(NULL),
		 m_curr_frame(0),
		 m_counter(0),
		 m_attr(PLAYER_ATTR_NONE),
		 m_is_blocked(false),
		 m_deliver_spot(NULL),
		 m_deliver_state(NOPIZZA),
		 m_score(0),
		 m_lives(PLAYER_LIVES),
		 m_death(NULL),
		 m_cash(NULL)
{	
	
	// init surfaces
	for(int32 i = 0; i < NUM_PLAYER_FRAMES; i++)
	{
		m_surface[i]		= NULL;
		m_surface_pizza[i]	= NULL;
	}
	for(int32 i = 0; i < NUM_PLAYER_DEAD_FRAMES; i++)
	{
		m_surface_dead[i] = NULL;
	}

	// set the sprite size
	m_width  = 32;
	m_height = 32;

	m_cwidth  = (int32) (m_width  * 0.80f);
	m_cheight = (int32) (m_height * 0.80f);

	// set the start position
	m_x = PLAYER_START_X;
	m_y = PLAYER_START_Y;

	m_old_x = m_x;
	m_old_y = m_y;

	// adds entity to the list
	AddEntity(this);
}



Player::~Player()
{	
	// delete surfaces
	for(int32 i = 0; i < NUM_PLAYER_FRAMES; i++)
	{
		if(m_surface[i] != NULL)
			delete m_surface[i];

		if(m_surface_pizza[i] != NULL)
			delete m_surface_pizza[i];
	}
	for(int32 i = 0; i < NUM_PLAYER_DEAD_FRAMES; i++)
	{
		if(m_surface_dead[i] != NULL)
			delete m_surface_dead[i];
	}

	// delete the sounds
	if(m_death)
		delete m_death;

	if(m_cash)
		delete m_cash;

}


void Player::UpdateState()
{
	// get the time to render a frame
	float delta = g_timer->GetDelta();

	static float timer = 0;
	// if the speed attribute is set, speed up the player
	if(m_attr & PLAYER_ATTR_SPEED)
	{
		timer += delta;
		m_fast = 3.0f;

		// time limit reached
		if(timer >= PLAYER_SPEED_TIME)
		{
			timer = 0;

			// normal speed
			m_fast = 1.0f;

			ResetAttribute(PLAYER_ATTR_SPEED);
		}
	}	

	// get the user input
	CheckPlayerActions();

	// updates the frames according to the state
	switch(m_state)
	{		
		case WALKING_RIGHT:
			{				
				// updates the counter
				m_counter += (delta * PLAYER_FPS * m_fast);

				// check if it's time to the next frame
				if(m_counter >= 1.0f)
				{
					m_curr_frame += m_counter;
					if(m_curr_frame >= 8.0f)
						m_curr_frame = 0;

					m_counter = 0;
				}

				// get the frame to draw
				int32 frame_to_draw = m_walking_right[(int32) m_curr_frame];
				m_draw = HasPizza() ? m_surface_pizza[frame_to_draw]:m_surface[frame_to_draw];

				// update speed
				m_speed_x = m_speed;
				m_speed_y = 0;

			} break;

		case WALKING_LEFT:
			{
				// updates the counter
				m_counter += (delta * PLAYER_FPS * m_fast);

				// check if it's time to the next frame
				if(m_counter >= 1.0f)
				{
					m_curr_frame += m_counter;
					if(m_curr_frame >= 8.0f)
						m_curr_frame = 0;

					m_counter = 0;
				}

				// get the frame to draw
				int32 frame_to_draw = m_walking_left[(int32) m_curr_frame];
				m_draw = HasPizza() ? m_surface_pizza[frame_to_draw]:m_surface[frame_to_draw];

				// update speed
				m_speed_x = -m_speed;
				m_speed_y = 0;

			} break;

		case WALKING_DOWN:
			{
				// updates the counter
				m_counter += (delta * PLAYER_FPS * m_fast);

				// check if it's time to the next frame
				if(m_counter >= 1.0f)
				{
					m_curr_frame += m_counter;
					if(m_curr_frame >= 3.0f)
						m_curr_frame = 0;
					
					m_counter = 0;
				}

				// get the frame to draw
				int32 frame_to_draw = m_walking_down[(int32) m_curr_frame];
				m_draw = HasPizza() ? m_surface_pizza[frame_to_draw]:m_surface[frame_to_draw];

				// update speed
				m_speed_x = 0;
				m_speed_y = m_speed;

			} break;

		case WALKING_UP:
			{
				// updates the counter
				m_counter += (delta * PLAYER_FPS * m_fast);

				// check if it's time to the next frame
				if(m_counter >= 1.0f)
				{
					m_curr_frame += m_counter;
					if(m_curr_frame >= 3.0f)
						m_curr_frame = 0;
					
					m_counter = 0;
				}

				// get the frame to draw
				int32 frame_to_draw = m_walking_up[(int32) m_curr_frame];
				m_draw = HasPizza() ? m_surface_pizza[frame_to_draw]:m_surface[frame_to_draw];

				// update speed
				m_speed_x = 0;
				m_speed_y = -m_speed;

			} break;


		case STAND_RIGHT:
			{
				// get the frame to draw
				m_draw = HasPizza() ? m_surface_pizza[0]:m_surface[0];

				// update speed
				m_speed_x = 0;
				m_speed_y = 0;

			} break;

		case STAND_LEFT:
			{
				// get the frame to draw
				m_draw = HasPizza() ? m_surface_pizza[8]:m_surface[8];

				// update speed
				m_speed_x = 0;
				m_speed_y = 0;

			} break;

		case STAND_DOWN:
			{
				// get the frame to draw
				m_draw = HasPizza() ? m_surface_pizza[16]:m_surface[16];

				// update speed
				m_speed_x = 0;
				m_speed_y = 0;

			} break;

		case STAND_UP:
			{
				// get the frame to draw
				m_draw = HasPizza() ? m_surface_pizza[19]:m_surface[19];

				// update speed
				m_speed_x = 0;
				m_speed_y = 0;

			} break;

		// the player has died
		case DEAD:
			{
				// update frame counter
				m_counter += (delta * 7.0f);

				// time to update
				if(m_counter >= 1.0f)
				{
					// advance frame
					m_curr_frame += m_counter;

					// end of animation
					if(m_curr_frame >= NUM_PLAYER_DEAD_FRAMES)
					{
						// reset frame to start
						m_curr_frame = 0;

						// decrement player life
						m_lives--;

						// player lives again
						m_state = STAND_DOWN;
					
						// set the start position
						m_x = PLAYER_START_X;
						m_y = PLAYER_START_Y;
				
						// set the free collision coord.
						m_old_x = m_x;
						m_old_y = m_y;
						

						// reset item timer
						timer = 0;
						// normal speed
						m_fast = 1.0f;
						ResetAttribute(PLAYER_ATTR_SPEED);

					}

					m_counter	 = 0;
				}
				
				// get the frame to draw
				m_draw = m_surface_dead[(int32) m_curr_frame];

				m_speed_x = 0;
				m_speed_y = 0;

			} break;

		
		default: break;
	}

	
	// updates the position on the screen
	m_x += (m_speed_x * delta * m_fast);
	m_y += (m_speed_y * delta * m_fast);

	// check if the player got the pizza only if he has no pizza
	IsGettingPizza();

	// check the delivering state
	DeliveringUpdateState();
	
	// time stopped, check block collisions
	if(g_cars[0]->GetAttribute() & CAR_ATTR_STOP)
		CheckCollisionWithCars();

	// check for screen bounds
	ScreenBounds();

	// check the player bounds with objects
	CheckBounds();

	// check for bonus lives that player may gain
	CheckBonusLives();

	// the game is over
	if(m_lives <= 0)
		g_game_state = GAME_OVER;
}


void Player::Draw()
{
	if(m_draw == NULL)
		return;

	if(IsOnScreen())
		m_draw->Blit((int32) m_x, (int32) m_y, true);
}



void Player::ReInit()
{
	m_state			= STAND_RIGHT;
	m_speed			= PLAYER_SPEED;
	m_fast          = 1.0f;
	m_speed_x		= 0;
	m_speed_y		= 0;
	m_curr_frame	= 0;
	m_counter		= 0;
	m_attr			= PLAYER_ATTR_NONE;
	m_deliver_spot	= NULL;
	m_deliver_state = NOPIZZA;
	m_score			= 0;
	m_lives			= PLAYER_LIVES;
	m_x				= PLAYER_START_X;
	m_y				= PLAYER_START_Y;
	m_old_x			= m_x;
	m_old_y			= m_y;
	m_is_blocked    = false;
	m_old_score		= 0;
	m_score_accum	= 0;


	TimeLeftInit();
}


// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////



bool Player::LoadFrames()
{
	// loads the image template
	Surface* template_surf = LoadTGA("gfx/player/player.tga", MEM_SYSTEM, NULL);
	if(template_surf == NULL)
	{
		return(false);
	}

	// gets the surface address
	byte* template_buffer = template_surf->Lock();
	
	// current position on the template
	int32 curr_template_x	= 0;
	int32 curr_template_y	= 0;

	// extract the frames from the template
	for(int32 frm_num = 0; frm_num < NUM_PLAYER_FRAMES; frm_num++)
	{
		m_surface[frm_num] = new Surface;
		m_surface[frm_num]->Create(32, 32, MEM_SYSTEM);

		// get the frame address
		byte* frame_buffer = m_surface[frm_num]->Lock();
		byte* frame_ptr    = frame_buffer;

		// get the frame position on the template
		byte* template_ptr = &template_buffer[curr_template_x + (curr_template_y*template_surf->GetPitch())];

		// copies the frame
		for(int32 y = 0; y < 32; y++)
		{
			Memcpy(frame_ptr, template_ptr, 32);

			template_ptr += template_surf->GetPitch();
			frame_ptr    += m_surface[frm_num]->GetPitch();
		}

		// unlocks the frame surface
		m_surface[frm_num]->Unlock();
		

		// end of a template line, go to next line
		if(((frm_num+1) & 7) == 0 && frm_num != 0)
		{
			curr_template_x = 0;
			curr_template_y += 32;
		}
		else
		{
			// advances to next frame on template
			curr_template_x += 32;
		}
	}

	template_surf->Unlock();
	delete template_surf;

	// loads the image template
	template_surf = LoadTGA("gfx/player/playerp.tga", MEM_SYSTEM, NULL);
	if(template_surf == NULL)
	{
		return(false);
	}

	// gets the surface address
	template_buffer = template_surf->Lock();

	curr_template_x	= 0;
	curr_template_y	= 0;

	// extract the frames from the template, now with pizza
	for(int32 frm_num = 0; frm_num < NUM_PLAYER_FRAMES; frm_num++)
	{
		m_surface_pizza[frm_num] = new Surface;
		m_surface_pizza[frm_num]->Create(32, 32, MEM_SYSTEM);

		// get the frame address
		byte* frame_buffer = m_surface_pizza[frm_num]->Lock();
		byte* frame_ptr    = frame_buffer;

		// get the frame position on the template
		byte* template_ptr = &template_buffer[curr_template_x + (curr_template_y*template_surf->GetPitch())];

		// copies the frame
		for(int32 y = 0; y < 32; y++)
		{
			Memcpy(frame_ptr, template_ptr, 32);

			template_ptr += template_surf->GetPitch();
			frame_ptr    += m_surface_pizza[frm_num]->GetPitch();
		}

		// unlocks the frame surface
		m_surface_pizza[frm_num]->Unlock();
		

		// end of a template line, go to next line
		if(((frm_num+1) & 7) == 0 && frm_num != 0)
		{
			curr_template_x = 0;
			curr_template_y += 32;
		}
		else
		{
			// advances to next frame on template
			curr_template_x += 32;
		}
	}

	template_surf->Unlock();
	delete template_surf;


	// now load the dying animation

	// loads the image template
	template_surf = LoadTGA("gfx/player/player_dying.tga", MEM_SYSTEM, NULL);
	if(template_surf == NULL)
	{
		return(false);
	}

	// gets the surface address
	template_buffer = template_surf->Lock();

	curr_template_x	= 0;	

	// extract the frames from the template, now with pizza
	for(int32 frm_num = 0; frm_num < NUM_PLAYER_DEAD_FRAMES; frm_num++)
	{
		m_surface_dead[frm_num] = new Surface;
		m_surface_dead[frm_num]->Create(32, 32, MEM_SYSTEM);

		// get the frame address
		byte* frame_buffer = m_surface_dead[frm_num]->Lock();
		byte* frame_ptr    = frame_buffer;

		// get the frame position on the template
		byte* template_ptr = &template_buffer[curr_template_x];

		// copies the frame
		for(int32 y = 0; y < 32; y++)
		{
			Memcpy(frame_ptr, template_ptr, 32);

			template_ptr += template_surf->GetPitch();
			frame_ptr    += m_surface_dead[frm_num]->GetPitch();
		}

		// unlocks the frame surface
		m_surface_dead[frm_num]->Unlock();		
		
		// advances to next frame on template
		curr_template_x += 32;		
	}

	template_surf->Unlock();
	delete template_surf;
	
	return(true);
}



void Player::CheckPlayerActions()
{
	if(m_state == DEAD)
		return;

	static player_state_t old_state;


	// saves the old state
	old_state = m_state;
	
	// check if the player want to walk
	if(InputCheckKey(DIK_RIGHT, DOWN))
	{
		m_state = WALKING_RIGHT;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
		}

	
		return;
	}
	else if(InputCheckKey(DIK_LEFT, DOWN))
	{
		m_state = WALKING_LEFT;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
		}
	
		return;
	}

	else if(InputCheckKey(DIK_DOWN, DOWN))
	{
		m_state = WALKING_DOWN;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
		}
		
		return;
	}
	else if(InputCheckKey(DIK_UP, DOWN))
	{
		m_state = WALKING_UP;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
		}


		return;
	}
	
	// the player is not walking keep him stopped with the right frame
	else if(old_state == WALKING_RIGHT)
	{
		m_state = STAND_RIGHT;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
					
		}

		return;
	}
	else if(old_state == WALKING_LEFT)
	{
		m_state = STAND_LEFT;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;
		
		}

		return;
	}
	else if(old_state == WALKING_DOWN)
	{
		m_state = STAND_DOWN;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;

		}

		return;
	}
	else if(old_state == WALKING_UP)
	{
		m_state = STAND_UP;
		if(m_state != old_state)
		{
			m_counter	 = 0;
			m_curr_frame = 0;

		}

		return;
	}
}



// check if the player is on the area to get the pizza
void Player::IsGettingPizza()
{	
	// get the center coordinates from the car
	int32 width1  = (m_width   >> 1);
	int32 height1 = (m_height  >> 1);

	// translate it to world coordinates
	int32 c_wx1 = (width1  + ((int32) m_x));
	int32 c_wy1 = (height1 + ((int32) m_y));

	// cache the center coordinates from the player
	static int32 width2  = (PIZZA_WIDTH  >> 1);
	static int32 height2 = (PIZZA_HEIGHT >> 1);

	// translate it to world coordinates, and cache it
	static int32 c_wx2 = (width2  + (int32) PIZZA_X);
	static int32 c_wy2 = (height2 + (int32) PIZZA_Y);

	// find the distance between the centers
	int32 delta_x = abs(c_wx2 - c_wx1);
	int32 delta_y = abs(c_wy2 - c_wy1);

	// check for collision with pizzaria
	if(delta_x < (width1+width2) && delta_y < (height1+height2))
	{
		if(HasPizza() == false)
		{	
			// the player has got the pizza
			g_player->GotPizza();
		}

		// the player is blocked
		Blocked();
	}	
}



void Player::DeliveringUpdateState()
{
	// check if he has the pizza
	if(m_deliver_state != NOPIZZA && m_deliver_state != STOLED)
	{
		// should never happen
		if(m_deliver_spot == NULL)
			return;

		// get the player coordinates
		int32 player_x = (int32) g_player->X();
		int32 player_y = (int32) g_player->Y();

		// check if the player has delivered the pizza
		if((player_x+m_cwidth>=m_deliver_spot->x && player_x<=m_deliver_spot->x+m_deliver_spot->width)&&
		   (player_y+m_cheight>=m_deliver_spot->y && player_y<=m_deliver_spot->y+m_deliver_spot->height))
		{
			if(InputCheckKey(DIK_SPACE, DOWN))
			{
				m_deliver_state = DELIVERED;				
			}
		}

	}
	
	// the time has ended
	if(TimeLeftOut())
	{
		m_deliver_state = TIMEOUT;
	}
	

	// now take the actions
	switch(m_deliver_state)
	{		
		// the pizza was delivered
		case DELIVERED:
			{
				// drop the pizza
				DropPizza();

				// update score
				m_score += 15;

				// make the cars faster
				g_fast_factor += 0.02f;

				// plays the cash sound
				m_cash->Play(false, HIGH);

				// clear spot state, to pick up a new deliver spot
				g_has_spot = false;

			} break;
		
		// time to deliver ended
		case TIMEOUT:
			{
				DropPizza();
				m_score -= 30;

				// clear spot state, to pick up a new deliver spot
				g_has_spot = false;
				
			} break;

		// do not reset the time here
		// this state is set by the thief object
		case STOLEN:
			{
				// reset the pizza attribute
				m_attr &= ~PLAYER_ATTR_PIZZA; 
			
				// pizza was stolen
				m_deliver_state = STOLED;

				m_score -= 15;

			} break;

		// the player lost the pizza
		// this state is set by the car objects
		case RUNNED_OVER:
			{				
				// update score
				DropPizza();

				// clear spot state, to pick up a new deliver spot
				g_has_spot = false;				
				
			} break;
	}
}



void Player::ScreenBounds()
{
	// check for the screen boundaries
	if(m_x <= 0)
		m_x = 0;
	else if(m_x > 608.0f)
		m_x = 607.0f;
		
	if(m_y < 156.0f)
		m_y = 155.0f;
	else if(m_y > 416.0f)
		m_y = 415.0f;
}



bool Player::LoadSounds()
{
	bool result;

	// loads the death sound
	m_death = new SoundBuffer;
	result = m_death->LoadWAV("sounds/player/death.wav");
	if(result == false)
	{
		Error error("Error trying to load the death sound\n");
		error.Report();

		return(false);
	}

	// loads the cash sound
	m_cash = new SoundBuffer;
	result = m_cash->LoadWAV("sounds/player/cash.wav");
	if(result == false)
	{
		Error error("Error trying to load the cash sound\n");
		error.Report();

		return(false);
	}

	return(true);
}



void Player::CheckCollisionWithCars()
{
	// check for collision with all cars
	for(int32 i = 0; i < MAX_CARS; i++)
	{
		// check for collision
		bool collision = CheckCollision(g_cars[i]);
		
		// a collision has occurred, put player on a unblocked
		// position
		if(collision)
			Blocked();		
	}
}



bool Player::CheckCollision(const GameEnt* ent)
{
	// get the center coordinates from the car
	int32 width1  = (m_cwidth   >> 1);
	int32 height1 = (m_cheight  >> 1);

	// get the center coordinates from the player
	int32 width2  = (ent->m_cwidth   >> 1);
	int32 height2 = (ent->m_cheight  >> 1);
	
	// translate it to world coordinates
	int32 c_wx1 = (width1  + (int32) m_x);
	int32 c_wy1 = (height1 + (int32) m_y);
	
	// translate it to world coordinates, plus 23 
	// to correct perspective collision
	int32 c_wx2 = (width2  + (int32) ent->m_x);
	int32 c_wy2 = (height2 + ((int32) ent->m_y + 23));

	// find the distance between the centers
	int32 delta_x = abs(c_wx2 - c_wx1);
	int32 delta_y = abs(c_wy2 - c_wy1);

	// check for collision
	if(delta_x < (width1+width2) && delta_y < (height1+height2))
		return(true);
	else 
		return(false);
}



void Player::CheckBonusLives()
{	
	// the score has changed, check for life bonus
	if(m_old_score != m_score)
	{
		// get the earned points
		m_score_accum += (m_score - m_old_score);
		
		// time to give some free life
		if(m_score_accum >= PLAYER_LIVES_SCORE)
		{
			m_lives += (m_score_accum/PLAYER_LIVES_SCORE);
			m_score_accum = 0;
		}
		
		// saves the last score
		m_old_score = m_score;
	}	
}