/*****************************************************************************
 * file_otf_zone.c
 *
 * This file contains all routines required for accessing the OTF Datum/Zone
 * file.  These routines were written in such a way that it can be called   
 * from anywhere in the PFPS [auto or tide interface or tide program].
 *
 * Contains:
 *==========
 * file_open_otf_zone()		-open otf_zone file
 * file_close_otf_zone()	-close otf_zone file
 * file_read_otf_zone()		-read next otf_zone record
 * file_put_otf_zone()		-put a new record in otf_zone file
 * file_get_otf_zone()		-get specified record from otf_zone file
 *
 * file_count_otf_zones()	-counts the number of otf_zone records
 * file_update_otf_zone()	-to upate a record in the otf_zone file
 * file_delete_otf_zone()	-delete a otf_zone record 
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifndef UNIX
#include <io.h>
#endif

#include "otf_zone.h"
#include "file_globals.h"

static	FILE	*otf_zone_file;
/*****************************************************************************
 * file_open_otf_zone CSU
 *
 * Purpose:  	To get a record from otf zone file.  
 *
 * Description:	Given the dbase (file) name:
 *		-open file
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_open_otf_zone(char* db_name)
{
	char	filename[100];
	int	ret_code = 1;
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

	/*****************************************/
	/****   OPEN THE OTF zone detail file     ****/
	/*****************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.otfz", path,  db_name);
#else
	sprintf(filename, "%s\\%s.otfz", path,  db_name);
#endif

	if( (otf_zone_file = fopen(filename,"rb"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	return(0);
}

/*****************************************************************************
 * file_close_otf_zone CSU
 *
 * Purpose:  	To close the otf zone file.  
 *
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
void file_close_otf_zone()
{
	fclose(otf_zone_file);
}

/*****************************************************************************
 * file_read_otf_zone CSU
 *
 * Purpose:  	To get the next record from otf zone file.  
 *
 * Description:	
 *		-read in the next record
 *		
 *		On failure, return a -1 for error or FILE_EOF
 *
 * Inputs:
 *		1)   point to structure to hold otf_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_read_otf_zone(OTF_ZONE_T* otf_zone_p)
{
	int	ret_code = 1;

	if(fread(otf_zone_p, sizeof(char),sizeof(OTF_ZONE_T), otf_zone_file)==0)
	{
		/* the read failed ... */
			
		if(feof(otf_zone_file))
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
 * file_put_otf_zone CSU
 *
 * Purpose:  	To put a record into otf zone file.  
 *
 * Description:	Given the dbase (file) name and a pointer to the zone:
 *		-open file
 *		-find out if a otf_zone record for this line exists
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
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/

int file_put_otf_zone(char* db_name, OTF_ZONE_T* otf_zone_p)
{
	char	filename[100];
	int	ret_code = 1;
	FILE 	*lotf_zone_file;
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
	
	/**************************************/
	/****   OPEN THE OTF ZONE file     ****/
	/**************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.otfz", path,  db_name);
#else
	sprintf(filename, "%s\\%s.otfz", path,  db_name);
#endif

	if( (lotf_zone_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	if(fwrite(otf_zone_p, sizeof(char), sizeof(OTF_ZONE_T), lotf_zone_file)
	    ==0 )
	{
		ret_code = FILE_WRITE_ERR;
	}

	fclose(lotf_zone_file);

	return(ret_code);
}
/*****************************************************************************
 * file_get_otf_zone CSU
 *
 * Purpose:  	To get specified record from otf_zone file.  
 *
 * Description:	Given the dbase (file) name and a zone number:
 *		-open file
 *		-read in records until desired record is found (match
 *		 zone_number).
 *		-return the zone data to calling routine
 *		
 *		On failure, return a file error (negative number)
		or the record number of the requested zone (positive number)
 *
 * Inputs:	1)   Database name
 *		2)   zone_number
 *		3)   point to structure to hold otf_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_get_otf_zone(char* db_name, int zone_number, OTF_ZONE_T* otf_zone_p)
{
	int	ret_code = 0;


	if(file_open_otf_zone(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_otf_zone(otf_zone_p) < 0)
		{
			break;
		}
		/* Increment count */
		ret_code++;

		/* FOUND IT!! */
		if(otf_zone_p->zone_number == zone_number)
		{
			break;
		}
	}

	file_close_otf_zone();

	return(ret_code);
}


/*****************************************************************************
 * file_count_otf_zone
 *
 * Purpose:  	To count the number of detail records in the file for 
 *		a given survey date.  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   survey date
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/

int file_count_otf_zone(char *db_name)
{
	OTF_ZONE_T	otf_zone_p;
	int	count = 0;

	if(file_open_otf_zone(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_otf_zone(&otf_zone_p) < 0)
		{
			break;
		}
		count++;

	}

	file_close_otf_zone();

	return(count);
}

/*****************************************************************************
 * file_update_otf_zone CSU
 *
 * Purpose:  	To update a record into the hardware file.
 *
 * Description:	Given the dbase (file) name and a zone_number and data:
 *		-open file for update
 *		-find the record in the file to be updated
 *		-back up to begining of record 
 *		-update record with new data.
 *		-return to calling routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		3)   point to structure holding otf_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_update_otf_zone(char* db_name, OTF_ZONE_T*otf_zone_p)
{
	char	filename[100];
	int	ret_code = 1;
	int 	lotf_zone_file;
	char 	*path;
	OTF_ZONE_T	temp_otf_zone_p;


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
	/****   OPEN THE OTF_ZONE file ****/
	/**********************************/

#ifdef UNIX
	sprintf(filename, "%s/%s.otfz", path,  db_name);
	if( (lotf_zone_file = open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s.otfz", path,  db_name);
	if( (lotf_zone_file = open(filename,O_RDWR | O_BINARY))==-1)
#endif
	{
		return(FILE_READ_ERR);
 
	}

	for(;;)
	{
		/* Get a record */
	 	if(read(lotf_zone_file, &temp_otf_zone_p, 
		   sizeof(OTF_ZONE_T))==-1)
		{
			/* the read failed ... */
			ret_code = FILE_READ_ERR;
			break;
		}
		if(temp_otf_zone_p.zone_number == otf_zone_p->zone_number)
		{

			lseek(lotf_zone_file, (-1) * (int)sizeof(OTF_ZONE_T), 
				SEEK_CUR);

			/* Found the record we want so back up the 
			 * file pointer to so we can overwrite it with
			 * the new data...
			 */
		 	if(write(lotf_zone_file, otf_zone_p,  
			   sizeof(OTF_ZONE_T))==-1)
			{
				ret_code = FILE_WRITE_ERR;
			}
			break;
		}
	}

	close(lotf_zone_file);

	return(ret_code);
}
/*****************************************************************************
 * file_delete_otf_zone CSU
 *
 * Purpose:  	To delete the specified record from otf_zone file.  
 *
 * Description:	Given the dbase (file) name and a zone number:
 *		-generate input file name
		-figure out record number
 *		-call file_delete_record() routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   zone_number
 *		3)   point to structure to hold otf_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_delete_otf_zone(char* db_name, int zone_number)
{
	char	filename[100];
	int	ret_code = 1;
	char 	*path;
	OTF_ZONE_T	otf_zone_p;
	int	record_number;
	int	FILECODE;

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

#ifdef UNIX
	sprintf(filename, "%s/%s.otfz", path,  db_name);
#else
	sprintf(filename, "%s\\%s.otfz", path,  db_name);
#endif

	/* Find out the record number of the zone we want to delete */
	record_number = file_get_otf_zone(db_name, zone_number, &otf_zone_p);

	if(( FILECODE = file_delete_record(filename, record_number, 
		sizeof(OTF_ZONE_T))) < 0)
	{
		return(FILE_ERROR);
	}
	return(1);
}
 
