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

  Description:	Input module.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <SDL3/SDL.h>

#include "../types.h"
#include "../input.h"
#include "../error.h"
#include "../mem_man.h"
#include "../xfrog_main.h"
#include "../timer.h"


// PROTOTYPES //////////////////////////////////////////////////////////////////////////////////


bool InputInit();
void InputShutdown();
void InputUpdate();
bool InputCheckKey(const int32 key_code, const key_state_t state);


static void CheckForKeyTicks();
static inline void KeyEvent(const int32 key_code, const key_state_t state);


// GLOBALS /////////////////////////////////////////////////////////////////////////////////////


// the input object
Input*					g_input = NULL;


// PRIVATE DATA ////////////////////////////////////////////////////////////////////////////////


// number of changed keys between flushes



// all the keys
static input_key_t			m_keys[NUM_OF_KEYS];


// FUNCTIONS ///////////////////////////////////////////////////////////////////////////////////


// inits the input module
bool InputInit()
{
	// create the input
	g_input = new Input();


	// init keys
	for(int32 i = 0; i < NUM_OF_KEYS; i++)
	{
		m_keys[i].code	= i;
		m_keys[i].state	= UP;
		m_keys[i].time  = 0;
		m_keys[i].tick  = false;
	}
	
	return(true);
}


void InputShutdown()
{
	delete g_input;
	g_input = nullptr;
}



void InputUpdate()
{
	// updates input
	g_input->FlushBuffer();
	g_input->CheckKeyEvents();

	// updates key ticks
	CheckForKeyTicks();
}



// check for the key state
bool InputCheckKey(const int32 key_code, const key_state_t state)
{
	// Reject invalid key codes before indexing the keyboard state.
	if(key_code < 0 || key_code >= NUM_OF_KEYS) return false;

	// check for a key tick
	if(state == TICK)
	{
		// the tick occurred
		if(m_keys[key_code].tick)
		{
			// clears the tick
			m_keys[key_code].tick = false;
			return(true);
		}
		else
		{
			return(false);
		}
	}

	// check for other state
	else if(m_keys[key_code].state == state)
		return(true);
	else
		return(false);
}



// PRIVATE FUNCTIONS ///////////////////////////////////////////////////////////////////////////


// check is time to tick
static void CheckForKeyTicks()
{	
	for(int32 i = 0; i < NUM_OF_KEYS; i++)
	{
		if(m_keys[i].state == DOWN)
		{
			m_keys[i].time += (KEY_RATE * g_timer->GetDelta());

			// the tick has occurred
			if(m_keys[i].time >= 1.0f)
			{
				m_keys[i].tick = true;

				// reset time
				m_keys[i].time = 0;
			}			
		}
	}
}


// a key event
static inline void KeyEvent(const int32 key_code, const key_state_t state)
{
	m_keys[key_code].state = state;
	if(state == DOWN)
	{
		m_keys[key_code].time  = 0;
		m_keys[key_code].tick  = true;
	}
	else
	{
		m_keys[key_code].time  = 0;
		m_keys[key_code].tick  = false;
	}
}



void Input::FocusManager() {
    for(int i=0;i<NUM_OF_KEYS;++i) { m_keys[i].state=UP; m_keys[i].tick=false; m_keys[i].time=0; }
}
void Input::HandleEvent(const SDL_Event& event) {
    if ((event.type==SDL_EVENT_KEY_DOWN || event.type==SDL_EVENT_KEY_UP) && !event.key.repeat) {
        int code=event.key.scancode;
        if (code>=0 && code<NUM_OF_KEYS) KeyEvent(code,event.key.down ? DOWN : UP);
    }
}
