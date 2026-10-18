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

  Description:	Module creates the pack and put files into it.

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include "types.h"
#include "cfs.h"


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


bool CreatePack(char* filename);
void ClosePack();
bool WriteFileToPack(char* filename);


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// module file pointer
static FILE*		m_fp = NULL;


// number of files in the pack
static int32		m_num_files = 0;


// current offset on the pack
static int32		m_curr_offset = sizeof(cfs_header_t);


// uncompressed size of the pack
static int32		m_pack_usize  = 0;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


bool CreatePack(char* filename)
{
	FILE* fp = NULL;

	fp = fopen(filename, "wb");
	if(fp == NULL)
	{
		printf("Error trying to create pack\n");

		return(false);
	}
	
	// adjust the offset
	fseek(fp, sizeof(cfs_header_t), SEEK_SET);
	m_fp = fp;

	return(true);
}



// closes the pack
void ClosePack()
{
	if(m_fp == NULL)
	{
		printf("\nError trying to close pack\n");
		return;
	}

	fseek(m_fp, 0, SEEK_END);
	int32 packsize = ftell(m_fp);
	fseek(m_fp, 0, SEEK_SET);

	cfs_header_t header;
	memset(&header, 0, sizeof(header));

	strcpy(header.id, CFS_HEADER);
	header.num_files = m_num_files;
	header.pack_size = packsize;

	fwrite(&header, sizeof(header), 1, m_fp);

	fclose(m_fp);

	// print pack info
	printf("\n\nPack written.\n");
	printf("Number of files:        %d\n", m_num_files);
	printf("Uncompressed pack size: %d\n", m_pack_usize);
	printf("Compressed pack size:   %d\n", packsize);

	// reset info
	m_curr_offset	= 0;
	m_num_files		= sizeof(cfs_header_t);
	m_pack_usize    = 0;
}



// writes a file to pack
bool WriteFileToPack(char* filename)
{
	if(m_fp == NULL)
	{
		printf("Pack not opened yet\n");

		return(false);
	}

	FILE* fp = NULL;

	fp = fopen(filename, "rb");
	if(fp == NULL)
	{
		printf("error trying to open %s ", filename);

		return(false);
	}

	fseek(fp, 0, SEEK_END);
	int32 filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// allocates memory for read data
	byte* file_data = new byte[filesize];
	memset(file_data, 0, filesize);

	// read the file
	fread(file_data, filesize, 1, fp);

	// close the file
	fclose(fp);

	// creates a cfs_file structure
	cfs_file_t	cfs_file;
	memset(&cfs_file, 0, sizeof(cfs_file));


	// alloc space to store compressed data
	int32 comp_size = (int32) (filesize + (filesize*0.2f) + 12);
	byte* comp_data = new byte[comp_size];
	memset(comp_data, 0, comp_size);	

	// compress data
	int32 result = compress2((Bytef*)comp_data, 
							 (uLongf*)&comp_size, 
							 (Bytef*)file_data, 
							 (uLongf)filesize,
							 Z_BEST_COMPRESSION);
	if(result != Z_OK)
	{
		printf("error trying to compress %s\n", filename);

		// delete uncompressed data
		delete [] file_data;

		// detete compressed data
		delete [] comp_data;

		return(false);
	}

	// delete uncompressed data
	delete [] file_data;

	// prepares the file structure
	strcpy(cfs_file.name, strlwr(filename));
	cfs_file.offset = m_curr_offset;
	cfs_file.usize  = filesize;
	cfs_file.csize	= comp_size;
	cfs_file.next	= NULL;
	
	// writes the file struct to the pack
	fwrite(&cfs_file, sizeof(cfs_file), 1, m_fp);

	// writes the compressed data to the pack
	fwrite(comp_data, comp_size, 1, m_fp);

	// advance the offset
	m_curr_offset += (comp_size + sizeof(cfs_file));

	// one more file
	m_num_files++;	

	// add to pack size
	m_pack_usize += (filesize + sizeof(cfs_file));

	delete [] comp_data;

	// print file info
	printf(" %d -> %d", cfs_file.usize, cfs_file.csize);

	return(true);
}