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

  Date:			15/02/2000

  Description:	High Score handling.

****************************************************************/



// INCLUDES ///////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "font.h"
#include "video.h"
#include "player.h"
#include "error.h"
#include "mem_man.h"


// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////


bool HighScoreInit();
void HighScoreShutdown();
void HighScoreUpdate();


// GLOBALS ////////////////////////////////////////////////////////////////////////////////////



// the game high score
static int32			g_high_score	= 0;


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////


// the file pointer
static FILE*		m_fp	= NULL;


// saves old high score
static int32		m_old_high_score = 0;


// FUNCTIONS //////////////////////////////////////////////////////////////////////////////////



// init high score
bool HighScoreInit()
{	
	FILE* fp = fopen("hs.dat", "rb");
	// the high score file don't exist
	if(fp == NULL)
	{
		Error error("The high score file doens't exist\n");
		error.Report();

		return(false);
	}

	fseek(fp, 0, SEEK_END);
	int32 filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(filesize > 4)
	{
		Error error("The high score file is not valid\n");
		error.Report();

		fclose(fp);

		return(false);
	}

	// read the highest score
	fread(&g_high_score, sizeof(int32), 1, fp);

	// set the pointer to the begin of the file
	fseek(fp, 0, SEEK_SET);

	// decrypt
	g_high_score ^= 2125;

	// saves the old high score 
	m_old_high_score = g_high_score;

	// pass the pointer to the module pointer
	m_fp = fp;	

	return(true);
}


// shutdown high score
void HighScoreShutdown()
{	
	if(m_fp == NULL)
		return;

	// the new high score is bigger ! so we need to write it.
	if(g_high_score > m_old_high_score)
	{
		fclose(m_fp);
		FILE* fp = fopen("hs.dat", "wb");
		if(fp == NULL)
			return;		

		// encrypt
		g_high_score ^= 2125;

		fseek(fp, 0, SEEK_SET);
		fwrite(&g_high_score, sizeof(int32), 1, fp);

		fclose(fp);
	}
	// simply close the file
	else
		fclose(m_fp);
}



// update high score state
void HighScoreUpdate()
{
	// the player has reached the high score, so save it
	if(g_player->Score() > g_high_score)
	{
		g_high_score = g_player->Score();
	}

	char high_score[64];
	Memset(high_score, '\0', sizeof(high_score));
	sprintf(high_score, "HIGH SCORE: %d", g_high_score);
	int32 x = 320 - ((strlen(high_score) << 3)/2);
	
	// print the high score
	Printf(x, 0, high_score);
}