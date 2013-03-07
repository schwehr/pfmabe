/*****************************************************************************
 * file_flightmap.c
 *
 * This file contains all routines required for accessing the FLIGHTMAP file.
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS.
 *
 * Contains:
 *==========
 * file_open_flightmap()	-open flightmap file
 * file_read_flightmap()	-get a flightmap record from file
 * file_write_flightmap()	-put a new record in flightmap file
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "file_globals.h"
#include "flightmap.h"

/*****************************************************************************
 * file_read_flightmap CSU
 *
 * Purpose:  	To  get the next record from the flightmap file
 *
 * Description:	Given FILE pointer a pointer to a FLIGHTMAP_GRD_T 
 *		structure, this routine will get the next record from the
 *		flightmap file.
 *		
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   file pointer
 *		2)   pointer to a flightmap structure
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/15 DR	-wrote routine
 *****************************************************************************/
int	file_read_flightmap( FILE *flightmap_file, FLIGHTMAP_GRD_T *flightmap_p )
{
	int	ret_code = FILE_OK;

	/* Get a record */
	 if(fread(flightmap_p, sizeof(char),sizeof(FLIGHTMAP_GRD_T), 
	   flightmap_file) == 0)
	{
		/* the read failed ... */
		if(feof(flightmap_file))
		{
			ret_code = FILE_EOF;
		}
		else
		{
			ret_code = FILE_READ_ERR;
		}
	}
	return(ret_code);
}

/*****************************************************************************
 * file_write_flightmap CSU
 *
 * Purpose:  	To put a record into flightmap file.  
 *
 * Description:	Given FILE pointer a pointer to a FLIGHTMAP_GRD_T 
 *		structure, this routine will save new record to the
 *		flightmap file.
 *
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   file pointer
 *		2)   pointer to a flightmap structure
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/15 DR	-wrote routine
 *****************************************************************************/

int file_write_flightmap( FILE *flightmap_file, FLIGHTMAP_GRD_T *flightmap_p )
{
	int	ret_code = FILE_OK;
	

  	if(fwrite(flightmap_p, sizeof(char),sizeof(FLIGHTMAP_GRD_T), 
		flightmap_file) == 0)
	{
		ret_code = FILE_WRITE_ERR;
	}

	return(ret_code);
}

/*****************************************************************************
 * file_open_flightmap CSU
 *
 * Purpose:  	To  open the flightmap file
 *
 * Description:	Given the dbase (file) name:
 *		-open file
 *		
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/15 DR	-wrote routine
 *****************************************************************************/
FILE *file_open_flightmap(char *db_name)
{
	char	filename[100];
	FILE 	*flightmap_file;
	char 	*path;

	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
 	}

	/*************************************/
	/****   OPEN THE FLIGHTMAP file ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.fm", path,  db_name);
#else
	sprintf(filename, "%s\\%s.fm", path,  db_name);
#endif

	if( (flightmap_file = fopen(filename,"rb+"))==NULL)
	{
		return(flightmap_file); 
	}

	return(flightmap_file);
}
 
