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

  Description:	Module of video routines

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "video.h"
#include "error.h"
#include "mem_man.h"
#include "xfrog_main.h"
#include "xfrog_defs.h"
#include "tga.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////



bool VideoInit();
void VideoShutdown();
void VideoUpdate();




// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the video object
Video*					g_video = NULL;


// paleta atual do jogo
PALETTEENTRY			g_actual_palette[256];


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// the game palette
static Surface*			m_surf_palette = NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inicia o video
bool VideoInit()
{
	g_video = new Video();

	// sets the video mode
	bool result = g_video->SetMode(640, 480);
	if(result == false)
	{
		// exits
		PostQuitMessage(0);
	}

	// create flip system
	result = g_video->CreateFlipSystem();
	if(result == false)
	{
		// exits
		PostQuitMessage(0);
	}
	

	// clear palette struct
	Memset(g_actual_palette, 0, sizeof(g_actual_palette));

	// loads the game palette
	m_surf_palette = LoadTGA("gfx/palette.tga", MEM_SYSTEM, g_actual_palette);
	delete m_surf_palette;
	m_surf_palette = NULL;
	g_actual_palette[0].peRed		= 0;
	g_actual_palette[0].peGreen		= 0;
	g_actual_palette[0].peBlue		= 0;
	g_actual_palette[0].peFlags		= PC_NOCOLLAPSE;
	g_actual_palette[255].peRed		= 255;
	g_actual_palette[255].peGreen	= 255;
	g_actual_palette[255].peBlue	= 255;
	g_actual_palette[255].peFlags	= PC_NOCOLLAPSE;

	// set the palette
	result = g_video->SetPalette();
	if(result == false)
	{
		PostQuitMessage(0);
	}

	return(true);
}



// shutsdown the video system
void VideoShutdown()
{
	delete g_video;
}



// updates video display
void VideoUpdate()
{	
	g_video->Flip();
	g_video->WaitVsync();
}


// VIDEO CLASS IMP ////////////////////////////////////////////////////////////////////////////


Video::Video()
	  : m_dd(NULL),
	    m_primary(NULL),
		m_secondary(NULL),
		m_palette(NULL),
		m_is_locked(false),
		m_back_buffer(NULL),
		m_clipper(NULL),
		m_presented_frames(0)
{
	// creates ddraw object
	HRESULT result = DirectDrawCreateEx(NULL, (VOID**)&m_dd, IID_IDirectDraw7, NULL);
	if(result != DD_OK)
	{
		Error error("Error trying to create ddraw object\n");

		error.Report();

		return;
	}

	// sets cooperative mode with windows
	result = m_dd->SetCooperativeLevel(g_window_hwnd, DDSCL_FULLSCREEN  | 
													  DDSCL_ALLOWREBOOT | 
													  DDSCL_EXCLUSIVE);
	if(result != DD_OK)
	{
		Error error("Error trying to set ddraw coop with windows\n");

		error.Report();

		return;
	}
}




Video::~Video()
{

	if(m_clipper != NULL)
		m_clipper->Release();

	if(m_palette != NULL)
		m_palette->Release();

	if(m_secondary != NULL)
		m_secondary->Release();

	if(m_primary != NULL)
		m_primary->Release();

	if(m_dd != NULL)
		m_dd->Release();
}




bool Video::SetMode(const int32 width, const int32 height)
{
	HRESULT result = m_dd->SetDisplayMode(width, height, 8, 0, 0);
	if(result != DD_OK)
	{
		Error error("Error trying to set video resolution\n");
		error.Report();

		return(false);
	}

	return(true);	

}



bool Video::CreateFlipSystem()
{
	DD_SURFACE_DESC		surface_desc;

	CLEAR_STRUCT(surface_desc);

	// double buffer description
	surface_desc.dwFlags			= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	surface_desc.ddsCaps.dwCaps		= DDSCAPS_PRIMARYSURFACE |
									  DDSCAPS_FLIP           |
									  DDSCAPS_COMPLEX;
	surface_desc.dwBackBufferCount	= 1;


	// creates the primary surface
	HRESULT result = m_dd->CreateSurface(&surface_desc, &m_primary, NULL);
	if(result != DD_OK)
	{
		Error error("Error trying to create primary surface\n");
		error.Report();

		return(false);
	}

	surface_desc.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

	// attach the back surface
	m_primary->GetAttachedSurface(&surface_desc.ddsCaps, &m_secondary);

	// create the clipper
	result = m_dd->CreateClipper(0, &m_clipper, NULL);
	if(result != DD_OK)
	{
		Error error("Error trying to create the clipper\n");

		error.Report();

		return(false);
	}

	// set the window handle
	m_clipper->SetHWnd(0, g_window_hwnd);

	// set the clipper for the secondary surface
	m_secondary->SetClipper(m_clipper);
	
	// clears the surfaces
	DD_BLTFX bltfx;
	CLEAR_STRUCT(bltfx);
	bltfx.dwFillColor = 0;
	m_primary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	m_secondary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
	

	return(true);
}




void Video::Flip()
{
	// updates the screen
	// DDFLIP_WAIT already waits for the flip. A lost surface or focus change
	// must return to the message loop instead of spinning forever.
	if(m_primary->Flip(NULL, DDFLIP_WAIT) == DD_OK)
		m_presented_frames++;
}




bool Video::SetPalette()
{
	if(m_palette != NULL)
	{
		// releases the palette first
		m_palette->Release();
		m_palette = NULL;
	}
	
	HRESULT result = m_dd->CreatePalette(DDPCAPS_8BIT		|  
										 DDPCAPS_ALLOW256	|
										 DDPCAPS_INITIALIZE,
										 g_actual_palette,
										 &m_palette,
										 NULL);
	if(result != DD_OK)
	{
		Error error("Error trying to create palette\n");
		error.Report();

		return(false);
	}

	// sets the palette for the primary surface
	m_primary->SetPalette(m_palette);

	return(true);
}




void Video::ClearBackSurface(const int32 color)
{
	int32 fill_color = color & 255;

	DD_BLTFX bltfx;
	CLEAR_STRUCT(bltfx);
	bltfx.dwFillColor = fill_color;
	m_secondary->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
}



byte* Video::LockBackSurface()
{
	if(m_is_locked == true)
		return(m_back_buffer);

	DD_SURFACE_DESC surface_desc;
	CLEAR_STRUCT(surface_desc);

	// gains access
	m_secondary->Lock(NULL, &surface_desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	m_is_locked = true;
	
	m_pitch			= surface_desc.lPitch;
	m_back_buffer	= (byte*)surface_desc.lpSurface;	
	
	return(m_back_buffer);
}



// SURFACE CLASS IMP //////////////////////////////////////////////////////////////////////////



Surface::Surface()
		: m_surface(NULL),
		  m_width(0),
		  m_height(0),
		  m_sbuffer(NULL),
		  m_pitch(0),
		  m_is_locked(false)
{
}



Surface::~Surface()
{
	if(m_surface != NULL)
	{	
		m_surface->Release();
		m_surface = NULL;
	}
}





bool Surface::Create(const int32 width, 
					 const int32 height, 
					 const surface_mem_t mem_type)
{
	if(m_surface != NULL)
		return(true);

	if(g_video == NULL)
		return(false);

	DD_SURFACE_DESC surface_desc;
	CLEAR_STRUCT(surface_desc);

	// set surface properties
	surface_desc.dwFlags	= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	surface_desc.dwWidth	= width;
	surface_desc.dwHeight	= height;

	// check for memory type
	if(mem_type == MEM_SYSTEM)
	{
		surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	}
	else if(mem_type == MEM_VIDEO)
	{
		surface_desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY;
	}

	// creates the ddraw surface
	HRESULT result = g_video->GetObj()->CreateSurface(&surface_desc, &m_surface, NULL);
	if(result != DD_OK)
	{
		Error error("Error trying to create a surface\n");
		error.Report();

		return(false);
	}

	// sets the object values
	m_width  = width;
	m_height = height;

	// sets the color key to the first index
	DD_COLORKEY	color_key;
	color_key.dwColorSpaceLowValue	 = 0;
	color_key.dwColorSpaceHighValue  = 0;

	m_surface->SetColorKey(DDCKEY_SRCBLT, &color_key);

	return(true);
}




void Surface::Clear(const int32 color)
{
	if(m_is_locked == true)
		return;

	int32 fill_color = color & 255;

	DD_BLTFX bltfx;
	CLEAR_STRUCT(bltfx);
	bltfx.dwFillColor = fill_color;
	m_surface->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
}



byte* Surface::Lock()
{
	if(m_is_locked == true)
		return(m_sbuffer);

	DD_SURFACE_DESC surface_desc;
	CLEAR_STRUCT(surface_desc);
	
	// gains access
	m_surface->Lock(NULL, &surface_desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	m_is_locked = true;
	
	m_pitch			= surface_desc.lPitch;
	m_sbuffer		= (byte*)surface_desc.lpSurface;
	
	return(m_sbuffer);
}



void Surface::Blit(const int32 x, const int32 y, bool is_trans)
{
	RECT src_rect	= { 0, 0, m_width, m_height };
	RECT dest_rect	= { x, y, (x + m_width), (y + m_height) };

	if(is_trans == true)
	{
		g_video->m_secondary->Blt(&dest_rect, 
								  m_surface, 
								  &src_rect, 
								  (DDBLT_WAIT | DDBLT_KEYSRC), 
								  NULL);
	}
	else
		g_video->m_secondary->Blt(&dest_rect, m_surface, &src_rect, DDBLT_WAIT, NULL);
}



void Surface::Mirror()
{
	if(m_surface == NULL)
		return;

	// the surface buffer
	byte* s_buffer	= Lock();
	byte* s_ptr		= s_buffer;

	// temp buffer to do the mirror
	byte* t_buffer	= new byte[m_width*m_height];
	Memset(t_buffer, 0, m_width*m_height);
	byte* t_ptr		= t_buffer;

	// do the mirror
	for(int32 y = 0; y < m_height; y++)
	{
		for(int32 x = (m_width-1); x >= 0; x--)
		{
			*t_ptr = s_buffer[x + y*m_pitch];

			t_ptr++;
		}		
	}
	

	t_ptr		= t_buffer;

	// copies the image back
	for(int32 y = 0; y < m_height; y++)
	{
		Memcpy(s_ptr, t_ptr, m_width);

		t_ptr += m_width;
		s_ptr += m_pitch;
	}

	Unlock();

	delete [] t_buffer;
}