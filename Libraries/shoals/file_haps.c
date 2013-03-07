/*****************************************************************************
 * file_haps.c
 *
 * This file contains all routines required for accessing the HAPS file.
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS.
 *
 * Contains:
 *==========
 * file_get_haps()		-get requested haps record
 * file_put_haps()		-put a new record in haps file
 * file_get_haps_version()	-returns the highest haps version for flightline
 * file_open_haps()		-open for read
 * file_read_haps()		-read in (next) haps record
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "file_globals.h"
#include "haps.h"

 /*****************************************************************************
 * file_get_haps CSU
 *
 * Purpose:  	To get a record from haps file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found (match
 *		 flightline and get maximum haps_version).
 *		-return the haps data to calling routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold haps data..
 *		4)	 Version (set to -1 to get max version)
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *
 * 98/10/29 DR	-Ret code will be the record number for found records...
 *****************************************************************************/
int file_get_haps( char *db_name, short flightline, HAPS_GRD_T *haps_p, short version)
{
	int	ret_code = 0;
	FILE 	*haps_file;
	int	haps_version;

	/* Will get max version if no version is passed */
	if(version < 0)
	{
		/*
		 * Get the maximum haps version for this line... -1 is 
		 * returned if new line
		 */
		haps_version = file_get_haps_version(db_name, flightline);
	}	
	else
		haps_version = version;

	haps_file = file_open_haps(db_name);

	if(haps_file == NULL)
	{
		ret_code = FILE_OPEN_ERR;
		return(ret_code); 
	}

	for(;;)
	{
		/* Get a record */
		if(file_read_haps(haps_file, haps_p) < 0)
		{
			/* the read failed ... */
			
			if(feof(haps_file))
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
		if((haps_p->flightline == flightline) && 
		   (haps_p->version == haps_version))
		{
			break;
		}
	}

	fclose(haps_file);

	return(ret_code);
}

/*****************************************************************************
 * file_put_haps CSU
 *
 * Purpose:  	To put a record into haps file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-find out if a haps record for this line exists, and if so, 
 *		 make version one higher that last...
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold haps data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_put_haps( char *db_name, HAPS_GRD_T *haps_p )
{
	char	filename[100];
	int	ret_code = 1;
	FILE 	*haps_file;
	int	max_haps_version;
	char 	*path;


	/*
	 * Get the maximum haps version for this line... -1 is 
	 * returned if new line
	 */
	max_haps_version = file_get_haps_version(db_name, haps_p->flightline);
	
	/* Increament version for this new record */
	haps_p->version = (short)max_haps_version + 1;
	
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
	/****   OPEN THE HAPS file     ****/
	/**********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.haps", path,  db_name);
#else
	sprintf(filename, "%s\\%s.haps", path,  db_name);
#endif

	if( (haps_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


  	if(fwrite(haps_p, sizeof(char),sizeof(HAPS_GRD_T), haps_file) == 0)
	{
		ret_code = FILE_WRITE_ERR;
	}

	fclose(haps_file);

	return(ret_code);
}

/*****************************************************************************
 * file_get_haps_version
 *
 * Purpose:  	To get the "max" haps version for a given flightline.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-find out if a haps record for this line exists, and if so, 
 *		 find the maximum version...
 *		-return max version (a -1 is returned for non-existing line)
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/24 DR	-wrote routine
 *****************************************************************************/

int file_get_haps_version( char *db_name, short	flightline )
{
	int	ret_code = 1;
	int	max_version;
	FILE	*haps_file;
	HAPS_GRD_T	haps_p;

	/* Init max_version to -1 (value for not found or new line)*/
	max_version = -1;

	haps_file = file_open_haps(db_name);

	if(haps_file == NULL)
	{
		ret_code = FILE_OPEN_ERR;
		return(max_version); 
	}

	for(;;)
	{
		/* Get a record */
	 	if(fread(&haps_p, sizeof(char),sizeof(HAPS_GRD_T), 
		   haps_file) == 0)
		{
			/* No message printed...  Will always have a read 
 			 * error because of EOF.  Return a -1 if no
 			 * records exist for line or the max version if 
 			 * one or more is found.
			 */
			break;
		}
		/* FOUND ONE!! */
		if(haps_p.flightline == flightline)
		{
			max_version = haps_p.version;
		}
	}

	fclose(haps_file);

	return(max_version);
}


/*****************************************************************************
 * file_open_haps CSU
 *
 * Purpose:  	To  open the haps file for READING ONLY
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
FILE *file_open_haps( char *db_name )
{
	char	filename[100];
	FILE 	*haps_file;
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
	/****   OPEN THE HAPS file ****/
	/*************************************/

#ifdef UNIX
	sprintf(filename, "%s/%s.haps", path,  db_name);
#else
	sprintf(filename, "%s\\%s.haps", path,  db_name);
#endif

	if( (haps_file = fopen(filename,"rb"))==NULL)
	{
		printf("FILE ERROR\n\topening '%s'\n", filename);
		return(haps_file); 
	}


	return(haps_file);
}

/*****************************************************************************
 * file_read_haps CSU
 *
 * Purpose:  	To  get the next record from the haps file
 *
 * Description:	Given FILE pointer a pointer to a HAPS_GRD_T 
 *		structure, this routine will get the next record from the
 *		haps file.
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
int	file_read_haps( FILE *haps_file, HAPS_GRD_T *haps_p)
{
	int	ret_code = 0;

	/* Get a record */
	 if(fread(haps_p, sizeof(char),sizeof(HAPS_GRD_T), 
	   haps_file) == 0)
	{
		/* the read failed ... */
		if(feof(haps_file))
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
