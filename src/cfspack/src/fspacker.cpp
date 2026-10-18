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

  Description:	Main

****************************************************************/


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <iostream.h>
#include <stl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <ctype.h>
#include <unistd.h>

#include "types.h"
#include "cfs.h"



// DEFINES /////////////////////////////////////////////////////////////////////////////////////


// packer version
#define CFS_PACKER_VERSION			"1.1"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////



static void PrintInitInfo();
static void CheckParams();
static void TreeWalk();
static int FilePacker(const char *file_path, const struct ffblk *ff);
static char* CorrectFileName(char filename[]);
static bool IsMySelf(char* filename);


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////


// cmd args
int32				g_argc;
char**				g_argv;



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


static char*		m_pack_filename = NULL;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
	g_argc = argc;
	g_argv = argv;

	PrintInitInfo();
	CheckParams();

	// creates a pack
	bool result = CreatePack(m_pack_filename);
	if(result == false)
		exit(1);
	
	TreeWalk();

	ClosePack();

	return(0);
}



// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////


// imprime informacao de entrada
static void PrintInitInfo()
{
	printf("\nCompressed File Sytem Packer v%s   %s %s\n", CFS_PACKER_VERSION,
														   __DATE__,
														   __TIME__);
	printf("Author: Gustavo Muller\n\n");
}



// checa por parametros de entrada
static void CheckParams()
{
	// numero de parametros insuficiente
	if(g_argc < 2)
	{
		printf("Error: You must specify the pack filename.\n");
		printf("Example: cfspack pack.cfs\n");

		exit(1);
	}
	else if(g_argc > 2)
	{
		printf("Error: Parameters overflow.\n");

		exit(1);
	}
	else if(g_argc == 2)
	{
		m_pack_filename = g_argv[1];

	}
}




// anda pela arvore de diretorios e vai mostrando os arquivos
static void TreeWalk()
{
	fflush(stdout);
	__file_tree_walk("./", FilePacker);
}



// pega os nomes que a funcao TreeWalk() fornece e os escreve para o
// pack
static int FilePacker(const char *file_path, const struct ffblk *ff)
{
	char		filename[1024];
	memset(filename, '\0', sizeof(filename));

	strcpy(filename, file_path);
	char* name_ptr = CorrectFileName(filename);

	// se nao for o arquivo proprio programa ou 
	// o pack compacte
	if(IsMySelf(name_ptr) == false)
	{
		// se nao for diretorio packeie
		if(!(ff->ff_attrib & FA_DIREC)) 
		{
			printf("compressing: %s ...", name_ptr);

			bool result = WriteFileToPack(name_ptr);
			if(result == true)
			{
				printf(" ok !\n");
			}
			else
			{
				printf(" failed !\n");
			}

		}
	}

	return(0);
}



// troca as barras \ por barras /
// tira o ./ do nome do arquivo
// deixa ele de tamanho minusculo
static char* CorrectFileName(char filename[])
{
	char* str_ptr	= &filename[0];
	char* last_ptr	= NULL;
	
	// corrige a string
	while(*str_ptr != NULL)
	{
		// troca as barras
		if(*str_ptr == '\\')
		{
			*str_ptr = '/';
		}	

		str_ptr++;
	}

	str_ptr = &filename[0];

	// tira o ./ do nome
	while(*str_ptr != NULL)
	{
		if((*str_ptr == '.') && (*(str_ptr + 1) == '/'))
		{			
			return(str_ptr += 2);
		}
		str_ptr++;
	}

	return(str_ptr);
}



// checa se o arquivo a ser compactado e' o proprio compactador
static bool IsMySelf(char* filename)
{
	if(strcmp("cfspack.exe", filename) == 0)
		return(true);
	else if(strcmp(m_pack_filename, filename) == 0)
		return(true);
	else
		return(false);
}