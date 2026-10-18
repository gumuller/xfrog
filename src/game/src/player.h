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


#ifndef _PLAYER_H
#define _PLAYER_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "ent.h"
#include "video.h"
#include "game.h"
#include "hud.h"
#include "sound.h"

// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// number of player frames
#define NUM_PLAYER_FRAMES		22

// number of death animation
#define NUM_PLAYER_DEAD_FRAMES	11


// frames per second on the animation
#define PLAYER_FPS				16.5f

// player attributes
#define PLAYER_ATTR_NONE		0
#define PLAYER_ATTR_PIZZA		1
#define PLAYER_ATTR_SPEED		2


// player start position
#define PLAYER_START_X			304.0f
#define PLAYER_START_Y			370.0f


// the player speed
#define PLAYER_SPEED			110.0f


// time of duration of the speed item
#define PLAYER_SPEED_TIME		15.0f


// one life for each 500 points
#define PLAYER_LIVES_SCORE		500


// number of lives
#define PLAYER_LIVES			4


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// player states
enum player_state_t 
{ 
	DEAD = 0,
	BLOCKED,
	STAND_RIGHT,
	STAND_LEFT,
	STAND_DOWN,
	STAND_UP,
	WALKING_RIGHT,
	WALKING_LEFT,
	WALKING_DOWN,
	WALKING_UP 
};


// the delivering state
enum deliver_state_t
{
	NOPIZZA,
	RUNNED_OVER,
	TIMEOUT,
	STOLEN,
	STOLED,
	DELIVERING,
	DELIVERED
};


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Player : public GameEnt
{
	public:

		Player();
		~Player();


		// update the player state
		void UpdateState();

		// draw the player
		void Draw();

		// check if the player has the pizza
		bool HasPizza() const { return((m_attr & PLAYER_ATTR_PIZZA)); }; 

		// deliver the pizza to specified spot
		void DeliverPizzaTo(pizza_spot_t* spot) { m_deliver_spot = spot; };

		// set the delivering state to the player
		void SetDeliveringState(deliver_state_t state) { m_deliver_state = state; };

		// get the current delivery state
		deliver_state_t GetDeliveringState() const { return(m_deliver_state);};


		// set a attribute to the player
		void SetAttribute(const int32 attr) { m_attr |= attr; };

		// reset a player attribute
		void ResetAttribute(const int32 attr) { m_attr &= ~attr; };

		// give points to the player
		void GivePoints(const int32 points)	{ m_score += points; };

		// plays the death sound
		void PlayDeathSound() { m_death->Play(false, HIGH); };

		// re-init the player state
		void ReInit();

		// player is blocked
		void Blocked()
		{
			Stop();
			Collide();		
		};		

		int32 Lives() const { return(m_lives); };
		int32 Score() const { return(m_score); };		


		friend bool  PlayerInit();
		friend class Car;		

	private:
		

		// collision with all cars check
		void CheckCollisionWithCars();

		// single collision check
		bool CheckCollision(const GameEnt* ent);

		// check if the player has delivered the pizza
		void DeliveringUpdateState();

		// check if the player is on the area to get the pizza
		void IsGettingPizza();

		// load player sounds
		bool LoadSounds();

		// check for screen bounds
		void ScreenBounds();

		// check the player bounds
		void CheckBounds()
		{
			if(m_is_blocked == false)
			{
				// saves unblocked coordinates
				m_old_x = m_x;
				m_old_y = m_y;	
			}
		}

		// check for bonus lives
		void CheckBonusLives();

		// the player has collided
		void Collide()
		{
			if(m_is_blocked == true)
			{
				// restore unblocked coord.
				m_x = m_old_x;
				m_y = m_old_y;
				Walk();
			}
		}


		// player is free
		void Walk() { m_is_blocked = false; };

		// stops the player
		void Stop() { m_is_blocked = true; };		

		void GotPizza()        
		{ 
			// pizza attribute set
			m_attr |= PLAYER_ATTR_PIZZA; 
			
			// the player is delivering the pizza
			m_deliver_state = DELIVERING;
		};

		// drops the pizza
		void DropPizza()	
		{ 
			// reset the pizza attribute
			m_attr &= ~PLAYER_ATTR_PIZZA; 
			
			// no pizza to deliver
			m_deliver_state = NOPIZZA;

			// init the time again
			TimeLeftInit();
		};


		// check for player input
		void CheckPlayerActions();

		// loads all player frames from the templates
		bool LoadFrames();

		// old free collision coordinates
		float					m_old_x;
		float					m_old_y;

		// the player state
		player_state_t			m_state;

		// the speed magnitude
		float					m_speed;


		// the fast attribute
		float					m_fast;

		// the y speed axis
		float					m_speed_y;

		// player attributes
		int32					m_attr;

		// frame to draw
		float					m_curr_frame;

		// counts how much left seconds to the next frame
		float					m_counter;


		// the player is blocked
		bool					m_is_blocked;

		// spot to deliver the pizza
		pizza_spot_t*			m_deliver_spot;

		// state of the delivering
		deliver_state_t			m_deliver_state;

		// the player score
		int32					m_score;

		// number of lives
		int32					m_lives;

		// player frames without pizza
		Surface*				m_surface[NUM_PLAYER_FRAMES];

		// player frames with pizza
		Surface*				m_surface_pizza[NUM_PLAYER_FRAMES];

		// holds the dead frames
		Surface*				m_surface_dead[NUM_PLAYER_DEAD_FRAMES];

		// points to the frame to draw
		Surface*				m_draw;

		// the death sound
		SoundBuffer*			m_death;

		// cash sound
		SoundBuffer*			m_cash;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool PlayerInit();
void PlayerShutdown();
void PlayerUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the player
extern Player*			g_player;


#endif
