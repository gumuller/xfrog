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


#include <dinput.h>

#include "types.h"
#include "input.h"
#include "error.h"
#include "mem_man.h"
#include "xfrog_main.h"
#include "timer.h"


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
static int32			m_ckeys = 0;


// all the keys
static key_t			m_keys[NUM_OF_KEYS];


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


// INPUT CLASS IMP /////////////////////////////////////////////////////////////////////////////



Input::Input()
	  : m_di(NULL),
	    m_keyboard(NULL)
{
	// DI /////////////////////////////////////////////////////////////

	// creates the dinput object
	HRESULT result = DirectInput8Create(g_hinstance,
										 DIRECTINPUT_VERSION,
										 IID_IDirectInput8A,
										 (void**) &m_di,							
										 NULL);
	if(result != DI_OK)
	{
		Error error("Error trying to create di object\n");

		error.Report();	

		PostQuitMessage(0);
	}


	// KEYBOARD /////////////////////////////////////////////////////////

	// creates the keyboard
	result = m_di->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if(result != DI_OK)
	{
		Error error("Error trying to create keyboard\n");

		error.Report();		

		PostQuitMessage(0);
	}

	// sets the cooperative mode
	result = m_keyboard->SetCooperativeLevel(g_window_hwnd, 
											 (DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));
	if(result != DI_OK)
	{
		Error error("Error trying to set the cooperative level\nwith windows \
					for the keyboard\n");

		error.Report();

		PostQuitMessage(0);
	}

	// set the data format
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(result != DI_OK)
	{
		Error error("Error trying to set the data format for the keyboard\n");

		error.Report();

		PostQuitMessage(0);
	}

	// buffer properties
	DIPROPDWORD		dipdw;
	dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj		= 0;
    dipdw.diph.dwHow		= DIPH_DEVICE;
    dipdw.dwData			= BUFFER_SIZE;
	
	m_keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

	// gets the keyboard
	result = m_keyboard->Acquire();
	if(result != DI_OK)
	{
		Error error("Error trying to acquire keyboard\n");

		error.Report();

		PostQuitMessage(0);
	}

}



Input::~Input()
{
	if(m_keyboard != NULL)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
	}

	if(m_di != NULL)
		m_di->Release();
}



void Input::FlushBuffer()
{
	m_ckeys = BUFFER_SIZE;

	// read the buffer
	m_keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
				 		      m_keydata, 
							  (unsigned long*)&m_ckeys,
							  NULL);
}



void Input::FocusManager()
{
	if(m_keyboard == NULL)
		return;

	static bool old_focus = g_app_focus;

	// the focus has changed
	if(old_focus != g_app_focus)
	{
		if(g_app_focus == false)
		{
			m_keyboard->Unacquire();
		}
		else
		{
			m_keyboard->Acquire();
		}
	}

	// saves the old state
	old_focus = g_app_focus;
}




void Input::CheckKeyEvents()
{
	// no keys where pressed
	if(m_ckeys == 0)
		return;

	// go over changed keys
	for(int32 i = 0; i < m_ckeys; i++)
	{
		// the key was pressed
		if(m_keydata[i].dwData & 0x80)
		{
			KeyEvent(m_keydata[i].dwOfs, DOWN);
		}
		// the key was released
		else 
		{
			KeyEvent(m_keydata[i].dwOfs, UP);
		}
	}

}