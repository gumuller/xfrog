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



// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


void Memcpy(void* dest, void* src, int32 count);
void Memset(void* dest, int32 num, int32 size);


extern "C" void dword_memset(void* dest, int32 num, int32 size);
extern "C" void dword_memcpy(void* dest, void* src, int32 count);


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// fills memory as fast as possible
void Memset(void* dest, int32 num, int32 size)
{
	// check for dword bytes align
	if( ( (((long)dest) | size) & 3) == 0)		
	{
		// size in dwords
		size >>= 2;

		// fills a 32 bit number
		num = ((num) | (num << 8) | (num << 16) | (num << 24));

		dword_memset(dest, num, size);

		return;
	}
	// fill using bytes
	else
	{
		memset(dest, num, size);
	}

	return;
}



// copies memory as fast as possible
void Memcpy(void* dest, void* src, int32 count)
{
	// check for DWORD align
	if( (( ((long)dest) | ((long)src) | count) & 3) == 0)		
	{
		count >>= 2;

		dword_memcpy(dest, src, count);

		return;
	}
	else
	{
		memcpy(dest, src, count);

		return;
	}
}