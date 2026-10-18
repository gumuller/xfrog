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


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "sound.h"
#include "error.h"
#include "xfrog_main.h"
#include "xfrog_defs.h"
#include "cfs.h"
#include "font.h"


// STRUCTS ////////////////////////////////////////////////////////////////////////////////////


#pragma pack(1)

// wave file header
struct wave_header_t
{
	char			riff_id[4];
	uint32			size;
	char			wave_id[4];
	char			fmt_id[4];
	uint32			format_length;			// must be 16
	uint16			format_tag;				// must be PCM
	uint16			num_channels;
	uint32			sample_rate;
	uint32			avg_bytes_per_sec;
	uint16			block_align;
	uint16			bits_per_sample;
	char			data_id[4];
	uint32			data_size;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool SoundInit();
void SoundShutdown();
void SoundUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// the sound object
Sound*					g_sound = NULL;


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////



// the sound channels
static SoundBuffer*		m_sound_channels[MAX_CHANNELS];


// number of used channels
static int32			m_used_channels = 0;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


// inits the sound module
bool SoundInit()
{
	bool result;

	g_sound = new Sound();	
	result = g_sound->CreatePrimaryBuffer();
	if(result != true)
	{
		PostQuitMessage(0);

		return(false);
	}

	// init the sound channels
	for(int32 i = 0; i < MAX_CHANNELS; i++)
	{
		m_sound_channels[i] = NULL;
	}

	return(true);
}


// shutsdown the sound module
void SoundShutdown()
{
	delete g_sound;
}




// updates sounds structs
void SoundUpdate()
{	
	int32 used_channels = 0;

	// updates used channels
	for(int32 i = 0; i < MAX_CHANNELS; i++)
	{
		// check to see if the channel is on use
		if(m_sound_channels[i] != NULL)
		{
			// if there is a sound playing, increment the number
			// of channels used
			if(m_sound_channels[i]->IsPlaying())
			{
				used_channels++;
			}
			// if there is no sound playing the channel can be freed
			else
			{
				m_sound_channels[i] = NULL;
			}
		}
	}

	// update the number of channels currently been used
	m_used_channels = used_channels;

	//Printf(0, 0, "Used channels: %d\n", m_used_channels);
}


// SOUND CLASS IMP ////////////////////////////////////////////////////////////////////////////



Sound::Sound()
	  : m_ds(NULL),
	    m_primary(NULL)
{
	HRESULT result;

	// creates DS object
	result = DirectSoundCreate(NULL, &m_ds, NULL);
	if(result != DS_OK)
	{
		Error error("Error trying to create dsound object\n");

		error.Report();

		PostQuitMessage(0);

		return;
	}

	// set the cooperative level
	result = m_ds->SetCooperativeLevel(g_window_hwnd, DSSCL_PRIORITY);
	if(result != DS_OK)
	{
		Error error("Error trying to set dsound coop. level\n");

		error.Report();

		PostQuitMessage(0);

		return;
	}
}



Sound::~Sound()
{
	if(m_primary != NULL)
	{
		m_primary->Release();
		m_primary = NULL;
	}

	if(m_ds != NULL)
	{
		m_ds->Release();
		m_ds = NULL;
	}
}




bool Sound::CreatePrimaryBuffer()
{	
	HRESULT result;

	DS_BUFFERDESC buffer_desc;
	CLEAR_STRUCT(buffer_desc);

	// primary buffer flag
	buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	// creates the primary buffer
	result = m_ds->CreateSoundBuffer(&buffer_desc,
									 &m_primary,
									 NULL);
	if(result != DS_OK)
	{
		Error error("Error trying to create primary buffer\n");

		error.Report();

		return(false);
	}

	// set the buffer format
	WAVEFORMATEX wfx;
	Memset(&wfx, 0, sizeof(wfx));

	wfx.wFormatTag			= WAVE_FORMAT_PCM;
	wfx.nChannels			= 1;
	wfx.nSamplesPerSec		= 11025;
	wfx.wBitsPerSample		= 8;
	wfx.nBlockAlign			= (wfx.wBitsPerSample/8) * wfx.nChannels;
	wfx.nAvgBytesPerSec		= wfx.nSamplesPerSec * wfx.nBlockAlign;
	
	result = m_primary->SetFormat(&wfx);
	if(result != DS_OK)
	{
		Error error("Error trying to set the buffer format for dsound\n");
		error.Report();

		return(false);
	}

	return(true);
}



// SOUND BUFFER CLASS IMP /////////////////////////////////////////////////////////////////////


bool SoundBuffer::LoadWAV(char filename[])
{
	if(m_buffer != NULL)
		return(false);

	bool result;

	CFS_File	file(filename);
	
	result = file.Open();
	if(result == false)
	{
		Error error("Error trying to open %s\n", filename);

		error.Report();

		return(false);
	}

	// a wave header
	wave_header_t	header;
	Memset(&header, 0, sizeof(wave_header_t));

	// reads the header
	file.Read(&header, sizeof(wave_header_t), 1);

	// check for the riff string
	if(strncmp(header.riff_id, "RIFF", strlen("RIFF")) != 0)
	{
		Error error("Error, %s is a invalid wave, RIFF string not found\n", filename);
		error.Report();

		return(false);
	}

	// check for the wave string
	if(strncmp(header.wave_id, "WAVE", strlen("WAVE")) != 0)
	{
		Error error("Error, %s is a invalid wave, WAVE string not found\n", filename);
		error.Report();

		return(false);
	}

	// check for the right format chunk length
	if(header.format_length != 16)
	{
		Error error("XFrog doesn't support the format chunk of %s\n", filename);
		error.Report();

		return(false);
	}

	// check for the format tag
	if(header.format_tag != 1)
	{
		Error error("XFrog only supports PCM Wave files\n");
		error.Report();

		return(false);
	}

	
	// prepare the wave ds struct
	WAVEFORMATEX wfx;
	Memset(&wfx, 0, sizeof(wfx));

	wfx.wFormatTag		= header.format_tag;
	wfx.nChannels		= header.num_channels;
	wfx.nSamplesPerSec	= header.sample_rate;
	wfx.wBitsPerSample	= header.bits_per_sample;
	wfx.nBlockAlign		= (wfx.wBitsPerSample/8) * wfx.nChannels;
	wfx.nAvgBytesPerSec	= wfx.nSamplesPerSec * wfx.nBlockAlign;

	
	DS_BUFFERDESC buffer_desc;
	CLEAR_STRUCT(buffer_desc);

	// set the buffer properties
	buffer_desc.dwFlags			= DSBCAPS_STATIC		| 
								  DSBCAPS_LOCSOFTWARE	|
								  DSBCAPS_CTRLPAN       |
								  DSBCAPS_CTRLVOLUME    |
								  DSBCAPS_CTRLFREQUENCY;
	buffer_desc.dwBufferBytes	= header.data_size;
	buffer_desc.lpwfxFormat		= &wfx;

	// now creates the buffer
	HRESULT ds_result = g_sound->GetDS()->CreateSoundBuffer(&buffer_desc,
				 									        &m_buffer,
														    NULL);
	if(ds_result != DS_OK)
	{
		Error error("Error trying to create sound buffer\n");

		error.Report();

		return(false);
	}

	
	// creates temp memory to hold the sound data
	byte* sound_data = new byte[header.data_size];
	Memset(sound_data, 0, header.data_size);
	file.Seek(sizeof(wave_header_t), SEEK_SET);
	file.Read(sound_data, header.data_size, 1);

	// now reads the sound data to the buffer
	DWORD			audio_length_1 = 0,
					audio_length_2 = 0;

	byte			*audio_ptr_1 = NULL,
					*audio_ptr_2 = NULL;

	
	// gets the dsound buffer address
	ds_result = m_buffer->Lock(0, 
							   header.data_size,
							   (void**) &audio_ptr_1,
							   &audio_length_1,
							   (void**) &audio_ptr_2,
							   &audio_length_2,
							   DSBLOCK_FROMWRITECURSOR);
	if(ds_result != DS_OK)
	{
		Error error("Error trying to lock %s sound buffer\n", filename);
		error.Report();

		delete [] sound_data;

		return(false);
	}

	// copies the data to the circular buffer
	Memcpy(audio_ptr_1, sound_data, audio_length_1);
	Memcpy(audio_ptr_2, (sound_data + audio_length_1), audio_length_2);
	

	// returns the access to dsound
	ds_result = m_buffer->Unlock(audio_ptr_1,
								 audio_length_1,
								 audio_ptr_2,
								 audio_length_2);
	if(ds_result != DS_OK)
	{
		Error error("Error trying to unlock %s sound buffer\n", filename);
		error.Report();

		delete [] sound_data;

		return(false);
	}

	delete [] sound_data;


	return(true);
}




bool SoundBuffer::Play(const bool loop, const sound_priority_t priority)
{
	if(m_buffer == NULL)
		return(false);	

	m_priority = priority;

	// find if the sound is already playing
	for(int32 i = 0; i < MAX_CHANNELS; i++)
	{
		if(m_sound_channels[i] == this)
			return(false);
	}

	// no channels free, try to eliminate a lower priority sound
	if(m_used_channels == (MAX_CHANNELS-1))
	{
		// try to find a less priority channel to free
		for(int32 i = 0; i < MAX_CHANNELS; i++)
		{
			// found a victim
			if(m_sound_channels[i]->Priority() < m_priority)
			{
				// stop the sound
				m_sound_channels[i]->Stop();

				m_sound_channels[i] = this;

				// now play the sound
				m_sound_channels[i]->PlayBuffer(loop);

				return(true);
			}
		}

		// we could not play the sound
		return(false);
	}
	// there are free channels to play with
	else
	{
		// find the free channel
		for(int32 i = 0; i < MAX_CHANNELS; i++)
		{
			// found a free channel
			if(m_sound_channels[i] == NULL)
			{
				// play the sound
				m_sound_channels[i] = this;
				m_sound_channels[i]->PlayBuffer(loop);

				// one more used channel
				m_used_channels++;

				// ok
				return(true);
			}
		}

		// no free channels(should not happen)
		return(false);
	}

	// this should not happen
	return(false);
}



void SoundBuffer::Stop()
{
	if(m_buffer == NULL)
		return;

	m_buffer->Stop();
	m_buffer->SetCurrentPosition(0);
}



bool SoundBuffer::IsPlaying()
{
	if(m_buffer == NULL)
		return(false);

	DWORD status;

	m_buffer->GetStatus(&status);
	if(status == DSBSTATUS_PLAYING || status == DSBSTATUS_LOOPING)
		return(true);
	else
		return(false);
}



void SoundBuffer::PlayBuffer(const bool loop)
{
	if(m_buffer == NULL)
		return;

	if(loop == false)
	{
		m_buffer->Play(0, 0, NULL);
	}
	else
	{
		m_buffer->Play(0, 0, DSBPLAY_LOOPING);
	}
}