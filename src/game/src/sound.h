#ifdef XFROG_SDL
#include "sdl/sound.h"
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

  Date:			31/01/2000

  Description:	XFrog sound module.

****************************************************************/


#ifndef _SOUND_H
#define _SOUND_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "types.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


#define MAX_CHANNELS			16


// TYPEDEFS ///////////////////////////////////////////////////////////////////////////////////


typedef LPDIRECTSOUND				DS;
typedef LPDIRECTSOUNDBUFFER			DS_BUFFER;
typedef DSBUFFERDESC				DS_BUFFERDESC;


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// sound priorities
enum sound_priority_t { LOW = 0, NORMAL, HIGH };


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


// the sound class
class Sound
{
	public:

		Sound();

		~Sound();

		// creates the primary buffer with 11 khz 8 bit mono
		bool CreatePrimaryBuffer();


		DS GetDS() const { return(m_ds); };

	private:

		// dsound object
		DS				m_ds;

		// primary buffer
		DS_BUFFER		m_primary;
};



// a sound buffer
class SoundBuffer
{
	public:

		SoundBuffer() : m_buffer(NULL), m_priority(NORMAL) {};

		~SoundBuffer() 
		{
			if(m_buffer != NULL)
			{
				m_buffer->Release();
				m_buffer = NULL;
			}
		};

		// creates the buffer
		bool LoadWAV(char filename[]);

		// plays the sound if it's possible
		bool Play(const bool loop, const sound_priority_t priority);

		// stop playing
		void Stop();

		// checks if the sound is playing
		bool IsPlaying();

		// returns the buffer priority
		sound_priority_t Priority() const { return(m_priority); };

	private:

		// plays the current buffer
		void PlayBuffer(const bool loop);

		// the sound data
		DS_BUFFER			m_buffer;

		// the buffer priority
		sound_priority_t	m_priority;	
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool SoundInit();
void SoundShutdown();
void SoundUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the sound object
extern Sound*					g_sound;



#endif

#endif
