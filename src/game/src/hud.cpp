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

  Date:			09/02/2000

  Description:	Xfrog HUD

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "font.h"
#include "video.h"
#include "mem_man.h"
#include "timer.h"
#include "player.h"
#include "car.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool HUDInit();
void HUDShutdown();
void HUDUpdate();
void TimeLeftInit();
bool TimeLeftOut();

static inline void DrawScore();
static void TimeLeftDraw();
static void TimeLeftUpdate();
static void DrawLives();



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// keep track of the timer activeness
bool m_timer_is_active = false;

// time left to deliver the pizza
static float		m_time_left = 20.0f;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



bool HUDInit()
{
	return(true);
}



void HUDShutdown()
{

}



void HUDUpdate()
{
	// draws the score
	DrawScore();

	// draw the number of lives
	DrawLives();

	// updates the time_left
	TimeLeftUpdate();
	TimeLeftDraw();
}


// init the time left counting
void TimeLeftInit()
{	
	m_time_left = 20.0f;
	m_timer_is_active = true;
}


// the time is over
bool TimeLeftOut()
{
	if(m_time_left == 0)
		return(true);
	else
		return(false);
}



// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


static inline void DrawScore()
{
	char score[64];
	Memset(score, '\0', sizeof(score));

	sprintf(score, "SCORE: %d", g_player->Score());

	Printf(632-(strlen(score)<<3), 8, score);;
}


// updates the timer
static void TimeLeftUpdate()
{	
	// time stopped
	if(g_cars[0]->GetAttribute() & CAR_ATTR_STOP)
		return;

	// update the time
	if(m_timer_is_active == true)
	{
		m_time_left -= g_timer->GetDelta();
		if(m_time_left <= 0)
		{
			m_time_left = 0;
			m_timer_is_active = false;
		}
	}
}


// updates the time left to deliver the pizza
static void TimeLeftDraw()
{
	char time[32];
	Memset(time, '\0', sizeof(time));

	// prints the time
	sprintf(time, "Time left: %d", ((int32) m_time_left));
	Printf(632-(strlen(time)<<3), 0, time);
}



// draw the number of lives on the screen
static void DrawLives()
{
	char lives[32];
	Memset(lives, '\0', sizeof(lives));

	// prints the time
	sprintf(lives, "Lives: %d", g_player->Lives());
	Printf(632-(strlen(lives)<<3), 16, lives);
}