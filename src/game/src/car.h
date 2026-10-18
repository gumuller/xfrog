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


#ifndef _CAR_H
#define _CAR_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "ent.h"
#include "video.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// max of cars
#define MAX_CARS		16

// 50 pixels per second
#define CAR_SPEED		50


// the car attributes
#define CAR_ATTR_NONE	0
#define CAR_ATTR_STOP   1

// time effect of the stop item
#define CAR_STOP_TIME   20.0f


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// car states
//enum car_state_t { DEAD = 0, RUNNING };


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Car : public GameEnt
{
	public:

		Car();
		~Car();
		
		// draw the car
		void Draw()
		{
			if(m_surface == NULL)
				return;

			if(IsOnScreen())
				m_surface->Blit((int32) m_x, (int32) m_y, true);
		};

		// updates the car state
		void UpdateState();	
		
		// reinit the cars state
		void Reinit();

		int32 GetAttribute() const { return(m_attr); };

		friend bool CarsInit();
		friend void CarsSetAttribute(const int32 attr);
		friend void CarsResetAttribute(const int32 attr);

	private:
		
		// load the surface
		bool Load(char* filename);

		// check for collision with the player
		bool CheckCollisionWithPlayer();

		// the car attributes
		int32				m_attr;

		// speed factor
		float				m_speed_factor;

		// holds the surface of the car
		Surface*			m_surface;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool CarsInit();
void CarsShutdown();
void CarsUpdate();
void CarsSetAttribute(const int32 attr);
void CarsResetAttribute(const int32 attr);
void CarsReInit();

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the game cars
extern Car*			g_cars[MAX_CARS];


// the speed rate of the cars when the game becomes more difficult
extern float			g_fast_factor;


#endif