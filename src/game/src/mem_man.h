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

  Description:	This module takes care of the memory copies,
				fills.

****************************************************************/



#ifndef _MEM_MAN_H
#define _MEM_MAN_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


void Memcpy(void* dest, void* src, int32 count);
void Memset(void* dest, int32 num, int32 size);



#endif