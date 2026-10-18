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


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "types.h"
#include "timer.h"
#include "error.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool TimerInit();
void TimerShutdown();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the game timer
Timer*			g_timer = NULL;


// STATICS ////////////////////////////////////////////////////////////////////////////////////


__int64 Timer::m_start_clock	= 0;		// the first tick
double  Timer::m_inv_rate	    = 0;		// inverted frequency


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



bool TimerInit()
{
	g_timer = new Timer();

	return(true);
}



void TimerShutdown()
{
	delete g_timer;
}


// TIMER CLASS IMP ////////////////////////////////////////////////////////////////////////////



Timer::Timer()
	  : m_start_time(0),
	    m_end_time(0)
{
	// get the timer frequency
	__int64 rate;
	if(QueryPerformanceFrequency( (LARGE_INTEGER*) &rate) == 0)
	{
		Error error("Error on trying to get timer freq.\n");
		error.Report();

		PostQuitMessage(0);
	}

	if(! rate)
	{
		Error error("Error on trying to get timer freq.\n");
		error.Report();

		PostQuitMessage(0);
	}

	// calculate the rate reciprocal
	m_inv_rate = (double) (1.0f/(double)rate);

	// get the actual ticks
	if(QueryPerformanceCounter( (LARGE_INTEGER*) &m_start_clock) == 0)
	{
		Error error("Error on trying to get timer tick count\n");
		
		error.Report();

		PostQuitMessage(0);
	}

	// inits random seed
	srand(static_cast<unsigned int>(time(0)));
}



float Timer::GetTime()
{
	__int64 end_clock;
	
	QueryPerformanceCounter((LARGE_INTEGER*) &end_clock);

	float final_tick = (float) (( ((__int64)end_clock) - m_start_clock)*m_inv_rate);
	
	return(final_tick);
}