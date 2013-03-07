/*****************************************************************************
 * file_input.c
 *
 * This file contains all rinpines required for accessing the input 
 * files. 
 *
 * Contains:
 *==========
 * file_open_input()	-Open the input file
 * file_read_input()	-get [next] a record from the input file
 * file_write_input()	-put a record to the input file
 * file_delete_input()	-delete the input file 
 * file_get_input()	-get requested timestamp's input record
 *
 * file_update_rec_input() -will update the file for the passed
 *			 record/timestamp (no open/close required)
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef UNIX
#include <io.h>
#endif

#include "file_globals.h"
#include "input.h"

/*****************************************************************************
 * file_open_input
 *
 * Purpose:  	To open the  input file...[will be opened in "r+" mode
 *		which will allow both reading and writing]  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   flightline
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote rinpine
 *****************************************************************************/
FILE	*file_open_input( char *db_name, int flightline )
{
	FILE	*input_file;
	char	filename[100];
	char 	*path;

	input_file = NULL;
	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(input_file); 
	}
	
	/*************************************/
	/****   OPEN THE INPUT file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.inp", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.inp", path,  db_name, flightline);
#endif

	if( (input_file = fopen(filename,"rb+"))==NULL)
	{
		return(input_file); 
	}

	return(input_file);
}
/*****************************************************************************
 * file_delete_input
 *
 * Purpose:  	To delete the  input file for the given flightline.  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   flightline
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote rinpine
 *****************************************************************************/

int	file_delete_input( char	*db_name, int flightline )
{
	char	filename[100];
	char 	*path;
	FILE	*input_file;

	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(FILE_ERROR);
	}
	
	/*************************************/
	/****   OPEN THE INPUT file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.inp", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.inp", path,  db_name, flightline);
#endif

	unlink(filename);
	if( (input_file = fopen(filename,"wb+"))==NULL)
	{
		return(FILE_OPEN_ERR); 
	}
	fclose(input_file);

	return(FILE_OK);
}


/*****************************************************************************
 * file_read_input CSU
 *
 * Purpose:  	To get a record from the open input file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold input data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote rinpine
 *****************************************************************************/
int file_read_input( FILE *input_file, INPUT_GRD_T *input_p)
{

	/* Get a record */
 	if(fread(input_p, sizeof(char),sizeof(INPUT_GRD_T), 
		   input_file) == 0)
	{		
		if(feof(input_file))
		{
			return(FILE_EOF);
		}
		else
		{
			return(FILE_READ_ERR);
		}
	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_write_input CSU
 *
 * Purpose:  	To put a record into input file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling rinpine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold input data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote rinpine
 *****************************************************************************/

int file_write_input( FILE *input_file, INPUT_GRD_T *input_p )
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(input_p, sizeof(char), sizeof(INPUT_GRD_T), 
		input_file)) 
	    == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_get_input CSU
 *
 * Purpose:  	To get the requested record from input file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the input data to calling rinpine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold input data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote rinpine
 *
 *****************************************************************************/

int file_get_input( char *db_name, short flightline, int timestamp, INPUT_GRD_T *input_p )
{
	char	filename[100];
	int	ret_val = FILE_OK;
	FILE 	*input_file;
	char 	*path;

	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
 	}


	/*
	*******************************
	*   OPEN THE INPUT file 
	*******************************
	*/

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.inp", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.inp", path,  db_name, flightline);
#endif

	if( (input_file = fopen(filename,"rb"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	/* Go to the approximate location of the record in the file */
	file_guess_record_location(input_file, db_name, flightline,
		timestamp, sizeof(INPUT_GRD_T));


	for(;;)
	{
		/* Get a record */
	 	if(fread(input_p, sizeof(char),sizeof(INPUT_GRD_T), 
		   input_file) == 0)
		{
			/* the read failed ... */
			if(feof(input_file))
			{
				ret_val = FILE_EOF;
			}
			else
			{
				ret_val = FILE_READ_ERR;
			}
			break;
		}

		/* CHECK IF IT IS THE ONE */
		if(input_p->timestamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(input_file);

	return(ret_val);
}


/*****************************************************************************
 * file_update_rec_input CSU
 *
 * Purpose:  	-will update the file for the passed record/timestamp  
 *		(if using in MP..., you will have to first load the
 *		record, change the required items and then rewrite
 *		it.. (flightline and timestamp info is in the record...)
 *
 * Description:	Given the dbase (file), flightline, timestamp and data:
 *		-open file
 *		-find record.
 *		-rewrite record
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   point to structure to hold input data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/08/24 DR	-wrote rinpine
 *
 *****************************************************************************/
int file_update_rec_input( char	*db_name, INPUT_GRD_T *input_p )
{
	char	filename[100];
	int	ret_val = FILE_OK;
	int 	filehandle;
	char 	*path;

	/**********************************/
	/* THIS CODE COULD BE MOVED TO A NEW 
	 * ROUTINE file_open_rewrite_inpput
 	 * if if required for many cases... */
	/**********************************/
	
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

	/**********************************/
	/****   OPEN THE OUTPUT file ****/
	/**********************************/
	/* open the file */


#ifdef UNIX
	sprintf(filename, "%s/%s_%d.inp", path,  db_name, input_p->flightline);
	if( (filehandle = _open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s_%d.inp", path,  db_name, input_p->flightline);
	if( (filehandle = _open(filename,O_RDWR | _O_BINARY))==-1)
#endif
	{
		return(FILE_OPEN_ERR);
 
	}
	

	/**********************************/
	/* THIS CODE COULD BE MOVED TO A NEW 
	 * ROUTINE file_setup_rewrite_input
 	 * if if required for many cases... */
	/**********************************/

	file_guess_record_location_ub(filehandle, db_name, input_p->flightline, 
		input_p->timestamp, sizeof(INPUT_GRD_T));

	for(;;)
	{
		INPUT_GRD_T	read_in;
		/* Get a record */
	 	if(_read(filehandle, &read_in, sizeof(INPUT_GRD_T))==-1)
		{
			/* the read failed ... */
			ret_val = FILE_READ_ERR;
			break;
		}
		
		if(input_p->timestamp == read_in.timestamp)
		{
			/* Found the record we want so back up the 
			 * file pointer...
			 */
			_lseek(filehandle, -(long)sizeof(INPUT_GRD_T), 
				SEEK_CUR);

			break;
		}
	}

	if(_write(filehandle, input_p, sizeof(INPUT_GRD_T))==-1)
	{
		ret_val = FILE_WRITE_ERR;
	}

	_close(filehandle);

	return(ret_val);
}
