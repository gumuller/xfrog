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

  Description:	Functions to read cfs files.

****************************************************************/



#ifndef _CFS_H
#define _CFS_H


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include "types.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool CFS_Init();
void CFS_Shutdown();


// CLASSES ////////////////////////////////////////////////////////////////////////////////////


class CFS_File
{
	public:

		CFS_File(char* filename);
		~CFS_File();

		// tries to open the file from the pack
		bool Open();

		// seeks the file
		void Seek(const int32 num_bytes, const int32 from);


		// reads the file to other placa
		bool Read(void* dest, const int32 size, const int32 count);

	private:

		char*				m_name;			// name of the file
		int32				m_size;			// file size

		byte*				m_data;			// file data
		byte*				m_ptr;			// pointer to data
};

#endif