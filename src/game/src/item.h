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

  Date:			14/02/2000

  Description:	Xfrog items.

****************************************************************/


#ifndef _ITEM_H
#define _ITEM_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"
#include "video.h"
#include "sound.h"
#include "ent.h"
#include "player.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// number of items
#define NUM_OF_ITEMS		5

// points to the player
#define FLOUR_AWARD			10
#define TOMATO_AWARD		20
#define CHEESE_AWARD		30

// in seconds
#define TIME_TO_SPAWN_ITEM	15.0f

// item spawn area
#define ITEM_SPAWN_AREA_X		0
#define ITEM_SPAWN_AREA_Y		182
#define ITEM_SPAWN_AREA_WIDTH	600
#define ITEM_SPAWN_AREA_HEIGHT	150


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


// a item class
class Item : public GameEnt
{
	public:

		Item();
		virtual ~Item();

		// draws the item
		void Draw();
		
		bool PickedUp() const { return(m_picked_up); };

		friend bool ItemsInit();
		friend void ItemsUpdate();
		friend void ItemsReInit();

	protected:

		// check collision with player
		bool CheckCollisionWithPlayer()
		{
			// get the center coordinates from the car
			int32 width1  = (m_cwidth   >> 1);
			int32 height1 = (m_cheight  >> 1);

			// get the center coordinates from the player
			int32 width2  = (g_player->C_Width()   >> 1);
			int32 height2 = (g_player->C_Height()  >> 1);

			// translate it to world coordinates			
			int32 c_wx1 = (width1  + (int32) m_x);
			int32 c_wy1 = (height1 + (int32) m_y);

			// translate it to world coordinates
			int32 c_wx2 = (width2  + (int32) g_player->X());
			int32 c_wy2 = (height2 + (int32) g_player->Y());

			// find the distance between the centers
			int32 delta_x = abs(c_wx2 - c_wx1);
			int32 delta_y = abs(c_wy2 - c_wy1);

			// check for collision
			if(delta_x < (width1+width2) && delta_y < (height1+height2))
				return(true);
			else 
				return(false);
		}

		// the surface of the item
		Surface*			m_surface;

		// was the item picked up by the player
		bool				m_picked_up;
};


// the food item class
class Food : public Item
{
	public:

		Food(int32 award);
		~Food();

		void UpdateState();

	private:

		// award to the player
		int32			m_award;

		
};


// the stop item
class Stop : public Item
{
	public:

		Stop();
		~Stop();

		void UpdateState();	
};



// the speed item
class Speed : public Item
{
	public:

		Speed() {};
		~Speed() {};

		void UpdateState();
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool ItemsInit();
void ItemsShutdown();
void ItemsUpdate();
void ItemsReInit();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////

// points to the current item on the world
extern Item*				g_curr_item;

extern Item*				g_items[NUM_OF_ITEMS];



#endif
