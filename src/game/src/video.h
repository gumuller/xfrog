#ifdef XFROG_SDL
#include "sdl/video.h"
#else
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


#ifndef _VIDEO_H
#define _VIDEO_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <ddraw.h>
typedef PALETTEENTRY PaletteEntry;

#include "types.h"


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// type of surface memory
enum surface_mem_t { MEM_VIDEO = 0, MEM_SYSTEM };


// TYPEDEFS ///////////////////////////////////////////////////////////////////////////////////


typedef LPDIRECTDRAW7			DD;
typedef LPDIRECTDRAWSURFACE7	DD_SURFACE;
typedef DDSURFACEDESC2			DD_SURFACE_DESC;
typedef DDSCAPS2				DD_SURFACE_CAPS;
typedef LPDIRECTDRAWPALETTE		DD_PALETTE;
typedef DDCOLORKEY				DD_COLORKEY;
typedef DDBLTFX					DD_BLTFX;
typedef LPDIRECTDRAWCLIPPER		DD_CLIPPER;


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Video
{
	public: 

		// inits ddraw
		Video();

		// shutsdown video
		~Video();


		// sets the video mode
		bool SetMode(const int32 width, const int32 height);


		// create the flip system
		bool CreateFlipSystem();


		// FIXME: put inline
		// flip the surfaces
		void Flip();
		uint32 PresentedFrames() const { return(m_presented_frames); };


		// set the palette
		bool SetPalette();


		// clears back surface
		void ClearBackSurface(const int32 color);


		// puts a pixel on back surface
		void PutPixel(const int32 x, const int32 y, const int32 color)
		{
			if(m_is_locked == false)
				return;

			m_back_buffer[x + y*m_pitch] = color;
		}
	

		// locks the back surface
		byte* LockBackSurface();


		// unlocks the back surface
		void UnlockBackSurface()
		{
			if(m_is_locked == false)
				return;

			m_secondary->Unlock(NULL);
			m_is_locked = false;
		}


		// wait Vsync
		void WaitVsync()
		{
			m_dd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
		};


		byte* BackBuffer() const	  { return(m_back_buffer); };
		int32 BackBufferPitch() const { return(m_pitch); };

		DD GetObj() const { return(m_dd); };		
		

		friend class Surface;
		

	private:

		// ddraw object
		DD					m_dd;

		// surface system
		DD_SURFACE			m_primary;
		DD_SURFACE			m_secondary;

		// actual palette
		DD_PALETTE			m_palette;


		// the clipper
		DD_CLIPPER			m_clipper;


		// checks to see if the secondary suface is locked
		bool				m_is_locked;


		// back buffer
		byte*				m_back_buffer;


		// saves the pitch of back buffer
		int32				m_pitch;
		uint32 m_presented_frames;
};



// XFrog surface
class Surface
{
	public:

		Surface();

		~Surface();

		// creates a surface
		bool Create(const int32 width, const int32 height, const surface_mem_t mem_type);

		// clears the surface to the color
		void Clear(const int32 color);

		// gains access to the surface
		byte* Lock();

		void Unlock()
		{
			if(m_is_locked == false)
				return;

			m_surface->Unlock(NULL);
			
			m_sbuffer	= NULL;
			m_pitch		= 0;
			m_is_locked = false;
		};


		// blit on the screen
		void Blit(const int32 x, const int32 y, bool is_trans);

		// makes a mirror of the surface
		void Mirror();
		

		byte* GetBuffer() const		{ return(m_sbuffer); };
		int32 GetPitch()  const		{ return(m_pitch); };
		int32 GetWidth()  const		{ return(m_width); };
		int32 GetHeight() const		{ return(m_height); };
		

	private:

		// the surface
		DD_SURFACE			m_surface;

		// width and height
		int32				m_width;
		int32				m_height;

		// surface buffer
		byte*				m_sbuffer;

		// the surface pitch
		int32				m_pitch;

		// keep tracks of lock state
		bool				m_is_locked;

};



// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////



bool VideoInit();
void VideoShutdown();
void VideoUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the video object
extern Video*					g_video;

// paleta atual do jogo
extern PaletteEntry				g_actual_palette[256];



#endif

#endif
