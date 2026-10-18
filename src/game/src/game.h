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

  Date:			04/02/2000

  Description:	Controls the game logic.

****************************************************************/



#ifndef _GAME_H
#define _GAME_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// position of the roads
#define ROAD_1		182
#define ROAD_2		222
#define ROAD_3		262
#define ROAD_4		302

// number of roads
#define NUM_OF_ROADS 4

// the pizza position
#define PIZZA_X			259
#define PIZZA_Y			403
#define PIZZA_WIDTH		123
#define PIZZA_HEIGHT	76


// the number of places to deliver the pizza
#define PIZZA_SPOTS		5


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// the states of the "in-game"
enum game_state_t { STARTING, PLAYING, GAME_OVER };


// STRUCTS ////////////////////////////////////////////////////////////////////////////////////

// the place where the player has to deliver the pizza
struct pizza_spot_t
{
	// position on scrren
	int32 x;
	int32 y;

	// dimensions
	int32 width;
	int32 height;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool GameInit();
void GameShutdown();
void GameUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// check if the player is playing
extern bool					g_is_playing;

// the game state
extern game_state_t			g_game_state;

// check to see if there is any active deliver spot
extern bool					g_has_spot;





#endif
