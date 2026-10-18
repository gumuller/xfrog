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



#ifndef _INPUT_H
#define _INPUT_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <dinput.h>

#include "types.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// size of the read buffer
#define BUFFER_SIZE 16


// number of keys
#define NUM_OF_KEYS 256


// rate of the keys
const float KEY_RATE = 0.333f;


// ENUMS //////////////////////////////////////////////////////////////////////////////////////


// state of a key
enum key_state_t { NONE = 0, UP, DOWN, TICK };


// STRUCTS ////////////////////////////////////////////////////////////////////////////////////


// one key
struct key_t
{
	int32			code;				// code of the key
	key_state_t		state;				// state of the key

	float			time;				// time the key was pressed
	bool			tick;				// the tick has occurred
};


// TYPEDEFS ///////////////////////////////////////////////////////////////////////////////////


typedef LPDIRECTINPUT8A			DI;
typedef LPDIRECTINPUTDEVICE8A	DI_DEVICE;


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class Input
{
	public:
		
		Input();

		~Input();


		// reads the changed keys into the buffer
		void FlushBuffer();

		// handles changes on focus
		void FocusManager();

		// check for key events that occurred
		void CheckKeyEvents();

		DI_DEVICE Keyboard() const { return(m_keyboard); };

	private:

		// dinput object
		DI						m_di;

		// the keyboard
		DI_DEVICE				m_keyboard;

		// keyboard buffer
		DIDEVICEOBJECTDATA		m_keydata[BUFFER_SIZE];
};



// PROTOTYPES //////////////////////////////////////////////////////////////////////////////////


bool InputInit();
void InputShutdown();
void InputUpdate();
bool InputCheckKey(const int32 key_code, const key_state_t state);


// GLOBALS /////////////////////////////////////////////////////////////////////////////////////


extern Input*				g_input;

#endif