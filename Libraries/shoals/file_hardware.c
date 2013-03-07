/*****************************************************************************
 * file_hardware.c
 *
 * This file contains all routines required for accessing the HARDWARE file.
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS.
 *
 * Contains:
 *==========
 * file_get_hardware()		-get requested hardware record
 * file_put_hardware()		-put a new record in hardware file
 * file_update_hardware()	-update a record in hardware file 
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef UNIX
#   include <sys/types.h>
#   include <unistd.h>
#else
#   include <io.h>
#endif

#include "file_globals.h"
#include "hardware.h"

/*****************************************************************************
 * file_open_hardware CSU
 *
 * Purpose:  	To  open the hardware file for READING ONLY
 *
 * Description:	Given the dbase (file) name and a flightline number:
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
 * 98/10/07	DR	-wrote routine
 *****************************************************************************/
FILE *file_open_hardware( char *db_name )
{
	char	filename[100];
	FILE 	*hardware_file;
	char 	*path;


	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
 	}

	/*************************************/
	/****   OPEN THE HARDWARE file ****/
	/*************************************/

#ifdef UNIX
	sprintf(filename, "%s/%s.hard", path,  db_name);
#else
	sprintf(filename, "%s\\%s.hard", path,  db_name);
#endif

	if( (hardware_file = fopen(filename,"rb"))==NULL)
	{
		printf("FILE ERROR\n\topening '%s'\n", filename);
		return(hardware_file); 
	}


	return(hardware_file);
}

/*****************************************************************************
 * file_read_hardware CSU
 *
 * Purpose:  	To  get the next record from the hardware file
 *
 * Description:	Given FILE pointer a pointer to a HARD_FILE_T 
 *		structure, this routine will get the next record from the
 *		hardware file.
 *		
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/
int	file_read_hardware(FILE *hardware_file, HARD_FILE_T *hardware_p )
{
	int	ret_code = 0;

	/* Get a record */
	 if(fread(hardware_p, sizeof(char),sizeof(HARD_FILE_T), 
	   hardware_file) == 0)
	{
		/* the read failed ... */
		if(feof(hardware_file))
		{
			ret_code = FILE_EOF;
		}
		else
		{
			printf("\tRead error\n");
			ret_code = FILE_READ_ERR;
		}
	}
	return(ret_code);
}

/*****************************************************************************
 * file_get_hardware CSU
 *
 * Purpose:  	To get a record from hardware file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the hardware data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold hardware data..a.
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *
 * 98/07/06 DR	-added flightline flag so you can just request the first
 *		 record in the file.   By passing the flightline number
 *		 FILE_FIRST_RECORD (ap/cnsts.h), it will get the first
 *		 record...
 *
 * 98/10/29 DR	-Ret code will be the record number for found records...
 *****************************************************************************/

int file_get_hardware(char *db_name, short flightline, HARD_FILE_T *hard_file_p )
{
	int	ret_code = 0;
	FILE 	*hard_file;

	hard_file = file_open_hardware(db_name);

	if(hard_file == NULL)
	{
		ret_code = FILE_OPEN_ERR;
		return(ret_code); 
	}

	for(;;)
	{
		/* Get a record */
		if(file_read_hardware(hard_file, hard_file_p) < 0)
		{
			/* the read failed ... */
			
			if(feof(hard_file))
			{
				ret_code = FILE_EOF;
			}
			else
			{
				ret_code = FILE_READ_ERR;
			}
			break;
		}

		/* Ret code will be the record number for found records...*/
		ret_code++;

		/* Check to see if we found it... */
		if((hard_file_p->au.flightline == flightline) || 
		   (flightline == FILE_FIRST_RECORD))
		{
			/* FOUND IT!! */
			break;
		}
	}

	fclose(hard_file);

	return(ret_code);
}

/*****************************************************************************
 * file_put_hardware CSU
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

int file_put_hardware(char *db_name, short flightline, HARD_FILE_T *hard_file_p )
{
	char	filename[100];
	int	ret_code = 1;
	FILE 	*hard_file;
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
	/****   OPEN THE HARDWARE file ****/
	/**********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.hard", path,  db_name);
#else
	sprintf(filename, "%s\\%s.hard", path,  db_name);
#endif

	if( (hard_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


 	if(fwrite(hard_file_p, sizeof(char),sizeof(HARD_FILE_T), hard_file)==0)
	{
		ret_code = FILE_WRITE_ERR;
	}

	fclose(hard_file);

	return(ret_code);
}

/*****************************************************************************
 * file_update_hardware CSU
 *
 * Purpose:  	To update a record into the hardware file.
 *
 * Description:	Given the dbase (file) name and a flightline number and data:
 *		-open file for update
 *		-find the record in the file to be updated
 *		-back up to begining of record 
 *		-update record with new data.
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
 *
 * 99/05/26 DR	-added "close" for hardware file at end of routine...
 ****************************************************************************/
int file_update_hardware( char *db_name, short flightline, HARD_FILE_T *hard_file_p )
{
	char	filename[100];
	int	ret_code = 1;
	int 	hard_filehandle;
	char 	*path;
	HARD_FILE_T	temp_hard_p;


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
	/****   OPEN THE HARDWARE file ****/
	/**********************************/
	/* open the file */



#ifdef UNIX
	sprintf(filename, "%s/%s.hard", path,  db_name);
	if( (hard_filehandle = open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s.hard", path,  db_name);
	if( (hard_filehandle = _open(filename,O_RDWR | _O_BINARY))==-1)
#endif
	{
		return(FILE_OPEN_ERR); 
	}

	for(;;)
	{
		/* Get a record */
#ifdef UNIX
	 	if(read(hard_filehandle, &temp_hard_p,sizeof(HARD_FILE_T))==-1)
#else
	 	if(_read(hard_filehandle, &temp_hard_p,sizeof(HARD_FILE_T))==-1)
#endif
		{
			/* the read failed ... */
			
			ret_code = FILE_EOF;
			break;
		}
		if(temp_hard_p.au.flightline == flightline)
		{

#ifdef UNIX
			lseek(hard_filehandle, -(long)sizeof(HARD_FILE_T), 
				SEEK_CUR);
#else
			_lseek(hard_filehandle, -(long)sizeof(HARD_FILE_T), 
				SEEK_CUR);
#endif

			/* Found the record we want so back up the 
			 * file pointer to so we can overwrite it with
			 * the new data...
			 */
		 	if(write(hard_filehandle, hard_file_p,  
			   sizeof(HARD_FILE_T))==-1)
			{
				ret_code = FILE_READ_ERR;
			}
			break;
		}
	}
	close(hard_filehandle);

	return(ret_code);
}
