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



// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "types.h"
#include "error.h"
#include "video.h"
#include "tga.h"
#include "mem_man.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool FontEngineInit();
void FontEngineShutdown();
void PrintChar(const int32 ch, const int32 x, const int32 y);
void Printf(int32 x, int32 y,  char* str, ...);
void PrintCenter(int32 y,  char* str, ...);


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// hold the font data
static Surface* m_font = NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inits the font engine
bool FontEngineInit()
{
	// load the font data
	m_font = LoadTGA("gfx/ui/gfont.tga", MEM_SYSTEM, NULL);
	if(m_font == NULL)
	{
		Error error("Error trying to load the game font\n");
		error.Report();

		return(false);
	}

	return(true);
}


// shutsdown the font engine
void FontEngineShutdown()
{
	if(m_font != NULL)
	{
		delete m_font;
	}
}



// print a char on the screen
void PrintChar(const int32 ch, const int32 x, const int32 y)
{
	int32 char_code = ch;
	char_code -= 32;

	// get the row and the column of the char
	int32 row_num  = char_code >> 4;
	int32 col_num  = char_code & 15;

	// get the position on the image
	int32 c_x = col_num << 3;
	int32 c_y = row_num << 3;

	// get the font from the surface
	byte* font_buffer	= m_font->Lock();
	byte* font_ptr		= &font_buffer[c_x + c_y*m_font->GetPitch()];

	// get the address on the back surface
	byte* back_buffer	= g_video->LockBackSurface();
	byte* back_ptr		= &back_buffer[x + y*g_video->BackBufferPitch()];

	// draw the caracter on the screen
	for(int32 y_i = 0; y_i < 8; y_i++)
	{
		for(int32 x_i = 0; x_i < 8; x_i++)
		{
			if(*font_ptr != 0)
				*back_ptr = *font_ptr;

			font_ptr++;
			back_ptr++;
		}

		font_ptr += (m_font->GetPitch() - 8);
		back_ptr += (g_video->BackBufferPitch() - 8);
	}

	m_font->Unlock();
	g_video->UnlockBackSurface();
}



// prints text on the screen
void Printf(int32 x, int32 y,  char* str, ...)
{
	char* text = new char[strlen(str) + 64];
	Memset(text, '\0', strlen(str) + 64);
	
	// prepares the list of arguments
	va_list  arglist;
	va_start(arglist, str);
	vsprintf(text, str, arglist);
	va_end(arglist);

	// gets the text length
	int32 text_size = strlen(text);
	char* text_ptr	= text;

	// saves the start x
	int32 old_x = x;

	// prints the text
	for(int32 i = 0; i < text_size; i++)
	{
		if((*text_ptr) != '\n' || (*text_ptr) != '\0')
			PrintChar(*text_ptr, x, y);

		if(*text_ptr == '\n')
		{
			y += 8;
			x = old_x;
			text_ptr++;
			continue;
		}

		x += 8;
		text_ptr++;
	}

	delete [] text;
}



// prints text on the center of the screen
void PrintCenter(int32 y,  char* str, ...)
{
	char* text = new char[strlen(str) + 64];
	Memset(text, '\0', strlen(str) + 64);
	
	// prepares the list of arguments
	va_list  arglist;
	va_start(arglist, str);
	vsprintf(text, str, arglist);
	va_end(arglist);

	int32 text_size = strlen(text);

	int32 center_x = 320 - (text_size << 2);
	Printf(center_x, y, text);

	delete [] text;
}