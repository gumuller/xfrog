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

  Description:	Handles errors.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef XFROG_SDL
#include <stdexcept>
#endif

#include "error.h"
#include "xfrog_main.h"
#include "types.h"
#include "mem_man.h"
#include "video.h"


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



Error::Error(char msg[], ...)
{
	// keeps space for extra parameters
	m_msg = new char[2048];
	Memset(m_msg, '\0', 2048);

	// prepares the list of arguments
	va_list  arglist;
	va_start(arglist, msg);
	vsnprintf(m_msg, 2048, msg, arglist);
	va_end(arglist);
}



Error::~Error()
{
	delete [] m_msg;
}



void Error::Report()
{
	#ifdef XFROG_SDL
	throw std::runtime_error(m_msg);
#else
	// flip the GDI to top
	g_video->GetObj()->FlipToGDISurface();

	
	ShowCursor(true);	
	MessageBox(g_window_hwnd,
			   m_msg, 
			   "XFrog Error:",
			   MB_OK		 |
			   MB_ICONERROR);
	
	ShowCursor(false);
#endif
	
}