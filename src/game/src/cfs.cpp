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


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include "types.h"

#include "mem_man.h"
#include "error.h"
#include "cfs.h"


// DEFINES ////////////////////////////////////////////////////////////////////////////////////


// the header of the pack
#define CFS_ID			"CFS\0"


// name of the pack
#define PACK_NAME		"xfrog.cfs"


// STRUCTS ////////////////////////////////////////////////////////////////////////////////////


#pragma pack(1)

// the cfs header
struct cfs_header_t
{
	char		id[4];				// id of the file
	int32		num_files;			// number of files in
	int32		pack_size;			// size of the pack
};


#pragma pack(1)

// one file of the pack
struct cfs_file_t
{
	char		name[64];			// name of the file
	int32		offset;				// offset on the pack
	int32		csize;				// compressed size
	int32		usize;				// uncompressed size

	cfs_file_t* next;
};


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool CFS_Init();
void CFS_Shutdown();

static bool AddFileToList(cfs_file_t* file);


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// pack file pointer
static FILE*				m_fp			= NULL;


// the list of the files read
static cfs_file_t*			m_file_list		= NULL;



// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



// inits the Compressed File System
bool CFS_Init()
{
	if(m_fp != NULL)
		return(true);

	m_fp = fopen(PACK_NAME, "rb");
	if(m_fp == NULL)
	{
		Error error("Error trying to open pack %s\n", PACK_NAME);
		error.Report();	

		return(false);
	}	

	
	// read the header
	cfs_header_t header;
	Memset(&header, 0, sizeof(header));
	fread(&header, sizeof(header), 1, m_fp);

	// check if is a CFS pack
	if(strcmp(header.id, CFS_ID) != 0)
	{
		Error error("Error the file %s is not a CFS pack\n", PACK_NAME);
		error.Report();

		fclose(m_fp);

		return(false);
	}
	
	// reads the number of files in the pack
	int32 num_files = header.num_files;

	// adjusts the pack for the file list reading
	int32 curr_offset = sizeof(header);
	
	// reads the file list
	for(int32 i = 0; i < num_files; i++)
	{
		// creates a new cfs file
		cfs_file_t* file = new cfs_file_t;
		Memset(file, 0, sizeof(cfs_file_t));

		// go to the right offset
		fseek(m_fp, curr_offset, SEEK_SET);

		// reads the file struct
		fread(file, sizeof(cfs_file_t), 1, m_fp);

		AddFileToList(file);

		// adjust the offset
		curr_offset += file->csize + sizeof(cfs_file_t);
	}
    

	// ok
	return(true);
}




// shutdown the CFS
void CFS_Shutdown()
{
	if(m_fp == NULL)
		return;

	// lets clear all the list
	cfs_file_t**	curr_ptr = &m_file_list;
	while(*curr_ptr != NULL)
	{
		cfs_file_t* temp_ptr = m_file_list->next;
		delete m_file_list;
		m_file_list = temp_ptr;
	}

	fclose(m_fp);
	m_fp = NULL;
}



// CFS_File CLASS IMP /////////////////////////////////////////////////////////////////////////



CFS_File::CFS_File(char* filename)
		 : m_name(NULL),
		   m_size(0),
		   m_data(NULL),
		   m_ptr(NULL)
{
	m_name = new char[strlen(filename)+1];
	Memset(m_name, '\0', strlen(filename)+1);

	strcpy(m_name, filename);	
}




CFS_File::~CFS_File()
{
	if(m_data != NULL)
	{
		delete [] m_data;
	}

	if(m_name != NULL)
	{
		delete [] m_name;
	}
}



bool CFS_File::Open()
{
	if(m_name == NULL)
		return(false);

	// runs along the list to find the file
	for(cfs_file_t* curr_ptr = m_file_list;
	    curr_ptr != NULL;
		curr_ptr = curr_ptr->next)
		{
			// found it !
			if(strcmp(curr_ptr->name, m_name) == 0)
			{
				// allocate temp space for compressed data
				byte* temp_data = new byte[curr_ptr->csize];
				Memset(temp_data, 0, curr_ptr->csize);

				// allocate space for file data
				m_data = new byte[curr_ptr->usize];
				Memset(m_data, 0, curr_ptr->usize);
				int32 usize = curr_ptr->usize;
				

				// reads data out
				fseek(m_fp, (curr_ptr->offset + sizeof(cfs_file_t)), SEEK_SET);
				fread(temp_data, curr_ptr->csize, 1, m_fp);


				// uncompress data
				int32 result = uncompress((Bytef*)  m_data,
					                      (uLongf*) &usize,
										  (Bytef*)  temp_data,
										  (uLongf)	curr_ptr->csize);
				
				if(result != Z_OK)
				{
					Error error("Error trying to uncompress %s\n", m_name);
					error.Report();

					delete [] temp_data;

					return(false);
				}
				

				delete [] temp_data;

				// point to the data
				m_ptr  = m_data;
				m_size = curr_ptr->usize;

				// get out of the loop
				return(true);
			}
		}	

	Error error("File %s doesn't exists on pack\n", m_name);
	error.Report();

	return(false);
}



void CFS_File::Seek(const int32 num_bytes, const int32 from)
{
	if(m_data == NULL)
		return;

	switch(from)
	{
		// from the start of the file
		case SEEK_SET:
			{
				m_ptr = m_data;
				m_ptr += num_bytes;

				return;
			} break;

		// from current position
		case SEEK_CUR:
			{
				m_ptr += num_bytes;

				return;
			} break;

			
		// from the end
		case SEEK_END:
			{
				m_ptr = &m_data[m_size-1];
				m_ptr += num_bytes;

				return;
			} break;

		default: break;
	}
}



bool CFS_File::Read(void* dest, const int32 size, const int32 count)
{
	if(m_data == NULL)
		return(false);

	byte* dest_ptr = (byte*) dest;

	// reads the file
	for(int32 i = 0; i < count; i++)
	{
		Memcpy(dest_ptr, m_ptr, size);

		m_ptr    += size;
		dest_ptr += size;
	}

	return(true);
}



// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////



// add a file to the top of the list
static bool AddFileToList(cfs_file_t* file)
{
	if(file == NULL)
		return(false);

	file->next	= m_file_list;
	m_file_list = file;

	return(true);
}