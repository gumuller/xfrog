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


#ifndef _THIEF_H
#define _THIEF_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "ent.h"
#include "video.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// number of animation frames
#define NUM_THIEF_FRAMES	8

// speed of the thief
#define THIEF_SPEED			100

// frames per second on the animation
#define THIEF_FPS			16.0f

// minimal time to the thief appear in seconds
#define TIME_TO_APPEAR		9


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Thief : public GameEnt 
{
	public:

		Thief();
		~Thief();

		void UpdateState();

		void Draw();

		// reinit the thief internal state
		void ReInit();

		float Count() const { return(m_counter); };

		friend bool ThiefInit();

	private:

		// check if the thief has stolen the pizza !
		bool CheckCollisionWithPlayer();

		// loads the animation frames
		bool LoadFrames();

		// actual frame
		float					m_curr_frame;

		// generic counter
		float					m_counter;

		// counts the time to appear
		float					m_appear_counter;

		// the time needed to appear
		float					m_total_time_to_appear;

		// out of the scene
		bool					m_out_the_scene;

		// the thief has the pizza
		bool					m_has_pizza;

		// points to the frame to draw
		Surface*				m_draw;

		// hold the thief without pizza
		Surface*				m_surface[NUM_THIEF_FRAMES];

		// hold the thief with pizza
		Surface*				m_surface_pizza[NUM_THIEF_FRAMES];
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool ThiefInit();
void ThiefShutdown();
void ThiefUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////



// the thief
extern Thief*			 g_thief;


#endif
