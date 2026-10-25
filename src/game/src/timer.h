#ifdef XFROG_SDL
#include "sdl/timer.h"
#else
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

  Description:	XFrog timer.

****************************************************************/


#ifndef _TIMER_H
#define _TIMER_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool TimerInit();
void TimerShutdown();


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Timer
{
	public:
		
		// init the timer
		Timer();

		~Timer() {};

		// get the current tick
		float GetTime();
	    
		
		// starts counting
		// should be called on the top of the engine loop
		void Time()	
		{
			m_start_time	= m_end_time;
			m_end_time		= GetTime();
			m_delta			= m_end_time - m_start_time;
		};


		// get the frame delta
		float GetDelta() const { return(m_delta); };


		// get the fps of the game
		float GetFPS() const { return(1.0f/m_delta); };
		
	private:

		static __int64 m_start_clock;	// the first tick
		static double  m_inv_rate;		// inverted frequency		

		float  m_start_time;
		float  m_end_time;
		float  m_delta;
};


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the game timer
extern Timer*			g_timer;


#endif

#endif
