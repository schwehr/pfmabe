/*****************************************************************************
 * file_survey_limits.c
 *
 * This file contains all routines required for accessing the SURVEY_LIMITS
 * file.  It's access is very simple as only two real actions are ever
 * done on it.  Either the data is read in (there is only one record)
 * or it is written out (either as a new file or to do an update).
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS.
 *
 * Contains:
 *==========
 * file_get_survey_limits() 	-get data from the survey_limits file
 * file_put_survey_limits()	-(re)write data to the survey_limits file
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "file_globals.h"
#include "survey_limits.h"

/*****************************************************************************
 * file_get_survey_limits CSU
 *
 * Purpose:  	To get a record from survey_limits file.  
 *
 * Description:	Given the dbase (file) name and a pointer:
 *		-open file
 *		-read in data .
 *		-return the survey_limits data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   point to structure to hold hardware data..a.
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/02 DR	-wrote routine
 *
 *****************************************************************************/

int file_get_survey_limits(char *db_name, SURVEY_LIMITS_T *survey_limits_p)
{
	char	filename[100];
	int	ret_code = FILE_OK;
	FILE 	*limits_file;
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


	/**********************************/
	/****   OPEN THE Survey Limits file ****/
	/**********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.sl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.sl", path,  db_name);
#endif

	if( (limits_file = fopen(filename,"rb"))==NULL)
	{
		return(FILE_OPEN_ERR);
	}

	/* Get the record */
 	if(fread(survey_limits_p, sizeof(char),sizeof(SURVEY_LIMITS_T), 
		   limits_file) == 0)
	{
		ret_code = FILE_ERROR;

	}

	fclose(limits_file);

	return(ret_code);
}

/*****************************************************************************
 * file_put_survey_limits CSU
 *
 * Purpose:  	To put a record into the hardware file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file for append
 *		-put the new record into the file.
 *		-return to calling routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold hardware data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/24 DR	-wrote routine
 *****************************************************************************/

int file_put_survey_limits(char *db_name, SURVEY_LIMITS_T *survey_limits_p)
{
	char	filename[100];
	int	ret_code = FILE_OK;
	FILE 	*limits_file;
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


	/**********************************/
	/****   OPEN THE Survey Limits file ****/
	/**********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.sl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.sl", path,  db_name);
#endif

	if( (limits_file = fopen(filename,"wb"))==NULL)
	{
		return(FILE_OPEN_ERR);
	}


 	if(fwrite(survey_limits_p, sizeof(char),sizeof(SURVEY_LIMITS_T), 
		limits_file)==0)
	{
		ret_code = FILE_ERROR;
	}

	fclose(limits_file);

	return(ret_code);
}
