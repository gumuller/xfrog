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


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ddraw.h>


#include "types.h"
#include "video.h"
#include "error.h"
#include "mem_man.h"
#include "cfs.h"



// DEFINES ////////////////////////////////////////////////////////////////////////////////////

// types of supported TGA orientation
#define BOTTOM_LEFT		0
#define TOP_LEFT		2



// STRUCTS ////////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)

// tga header
struct tga_header_t
{
	byte				id_length;
	byte				color_map_type;
	byte				image_type;
	uint16				color_map_index;
	uint16				color_map_length;
	byte				color_map_size;
	uint16				x_origin;
	uint16				y_origin;
	byte				low_width;
	byte				high_width;
	byte				low_height;
	byte				high_height;
	byte				bpp;
	byte				image_descriptor;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


Surface* LoadTGA(char* filename, const surface_mem_t mem_type, PALETTEENTRY* palette);


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



// loads a tga file and return it on a surface
Surface* LoadTGA(char* filename, const surface_mem_t mem_type, PALETTEENTRY* palette)
{
	static char name[64];
	Memset(name, '\0', sizeof(name));

	strcpy(name, filename);

	CFS_File file(name);

	bool result = file.Open();
	if(result == false)
	{
		Error error("Error trying to open TGA file %s\n", filename);
		error.Report();

		return(NULL);
	}

	// a tga header
	tga_header_t	header;
	Memset(&header, 0, sizeof(tga_header_t));
	
	// read the header
	file.Read(&header, sizeof(tga_header_t), 1);

	// check for the right type
	if(header.image_type != 1)
	{
		Error error("XFrog only supports color mapped uncompressed TGAs\n");
		error.Report();

		return(NULL);
	}

	// check for the right bpp
	if(header.bpp != 8)
	{
		Error error("XFrog only supports 8 bit TGAs\nthe image %s is %d\n", filename, header.bpp);
		error.Report();	

		return(NULL);
	}

	// get image width
	int32 width = header.high_width;
	width		<<= 8;
	width		+= header.low_width;

	// get image height
	int32 height = header.high_height;
	height		<<= 8;
	height		+= header.low_height;

	// get the bpp
	int32 bpp = header.bpp;


	int32 palette_bpe	 = header.color_map_size;		// palette bits per entry
	int32 palette_length = header.color_map_length;    	// gets the palette length
	if(palette_length > 256)
	{
		Error error("Palette overflow loading %s\n", filename);
		error.Report();

		return(NULL);
	}

	// palette in bytes
	int32 palette_size = ((palette_length*palette_bpe)/8);

	// check if we need the palette
	if(palette != NULL)
	{	
		// allocate space for read palette		
		byte* temp_palette = new byte[palette_size];
		Memset(temp_palette, 0, palette_size);

		// go for the palette
		file.Seek((header.id_length+18), SEEK_SET);

		// read the palette
		file.Read(temp_palette, palette_size, 1);

		// convert to ddraw format
		for(int32 i = 0; i < palette_length; i++)
		{
			int32 offset = i*3;//((palette_bpe)/8);
		
			palette->peBlue		= temp_palette[offset+0];
			palette->peGreen	= temp_palette[offset+1];
			palette->peRed		= temp_palette[offset+2];
			palette->peFlags	= PC_NOCOLLAPSE;

			palette++;
		}

		delete [] temp_palette;
	}

	// check for image description
	int32 orientation = ((header.image_descriptor & 48) >> 4);
	if(orientation != TOP_LEFT && orientation != BOTTOM_LEFT)
	{
		Error error("Wrong type of image orientation %s\n", filename);
		error.Report();

		return(NULL);
	}

	// set the correct position for read pixels
	file.Seek((header.id_length + 18 + palette_size), SEEK_SET);

	// temp space for the image data
	byte* temp_buffer = new byte[width*height];
	Memset(temp_buffer, 0, width*height);

	// load with the correct orientation
	if(orientation == TOP_LEFT)
	{
		for(int32 i = 0; i < width*height; i++)
		{
			file.Read(&temp_buffer[i], 1, 1);
		}		
	}
	else if(orientation == BOTTOM_LEFT)
	{
		for(int32 i = ((width*height)-1); i >= 0; i--)
		{
			file.Read(&temp_buffer[i], 1, 1);
		}
	}

	// alloc surface
	Surface* surface = new Surface;
	surface->Create(width, height, mem_type);
	surface->Clear(0);

	byte* surface_buffer = surface->Lock();
	int32 surface_pitch  = surface->GetPitch();

	byte* temp_ptr = temp_buffer;

	// read data
	for(int32 i = 0; i < height; i++)
	{
		Memcpy(surface_buffer, temp_ptr, width);

		surface_buffer	+= surface_pitch;
		temp_ptr		+= width;
	}
	surface->Unlock();

	delete [] temp_buffer;

	// return the loaded surface
	return(surface);
}