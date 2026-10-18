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

  Date:			25/01/2000

  Description:	TGA functions

****************************************************************/


#ifndef _TGA_H
#define _TGA_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <ddraw.h>

#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


Surface* LoadTGA(char* filename, const surface_mem_t mem_type, PALETTEENTRY* palette);


#endif
