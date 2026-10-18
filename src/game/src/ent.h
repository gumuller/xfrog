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

  Description:	The game entity definition.

****************************************************************/


#ifndef _ENT_H
#define _ENT_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "types.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


#define MAX_ENTITIES		64


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class GameEnt
{
	
	public:
		
		GameEnt() : m_width(0),
					m_height(0),
					m_x(0),
					m_y(0),
					m_speed_x(0),
					m_cwidth(0),
					m_cheight(0)
		{ 			
		};

		virtual ~GameEnt() {};


		// updates entity state
		virtual void UpdateState()			= 0;

		// draw the entity
		virtual void Draw()					= 0;

		// check if is on screen
		bool IsOnScreen() const
		{
			if( (m_x+(float)m_width  > 0 && m_x < 640.0f) &&  
				(m_y+(float)m_height > 0 && m_y < 480.0f) )
				return(true);
			else
				return(false);
		};
		
		int32 Width() const { return(m_width); };
		int32 Height() const { return(m_height); };

		int32 C_Width() const { return(m_cwidth); };
		int32 C_Height() const { return(m_cheight); };

		float X() const { return(m_x); };
		float Y() const { return(m_y); };

		float Speed() const { return(m_speed_x); };	

		// number of entities
		int32 NumberOfEntities() const { return(m_num_ent); };

		// init list
		static void InitEntList()
		{
			for(int32 i = 0; i < MAX_ENTITIES; i++)
			{
				GameEnt::m_ent_list[i] = NULL;
			}
		};

		friend void SortZ_Entities();
		friend int32 EntCompare(const void* elem1, const void* elem2);
		friend void DrawAllEnt();
		friend void ItemsUpdate();
		friend class Player;

	private:

		// the list of entities
		static GameEnt*			m_ent_list[MAX_ENTITIES];		

		// number of total entities
		static int32			m_num_ent;
		
	protected:

		
		// adds entity to the list
		void AddEntity(GameEnt* ent)
		{
			GameEnt::m_ent_list[GameEnt::m_num_ent] = ent;
			GameEnt::m_num_ent++;
		};

		// size
		int32					m_width;
		int32					m_height;

		// collision width and height
		int32					m_cwidth;
		int32					m_cheight;

		// world position
		float					m_x;
		float					m_y;

		// speed on the x axis
		float					m_speed_x;
};


#endif