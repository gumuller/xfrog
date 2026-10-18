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

  Date:			03/02/2000

  Description:	This module contains functions of the font 
				engine.

****************************************************************/


#ifndef _FONT_H
#define _FONT_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool FontEngineInit();
void FontEngineShutdown();
void PrintChar(const int32 ch, const int32 x, const int32 y);
void Printf(int32 x, int32 y,  char* str, ...);
void PrintCenter(int32 y,  char* str, ...);


#endif
