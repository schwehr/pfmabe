/*****************************************************************************
 * file_tide.c
 *
 * This file contains all routines required for accessing the TIDE files.
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS [auto or tide interface or tide program].
 *
 * Contains:
 *==========
 * file_open_tide_zone()	-open tide_zone file
 * file_read_tide_zone()	-read next tide_zone record
 * file_close_tide_zone()	-close tide_zone file
 *
 * file_open_tide_elev()	-open tide_elev file
 * file_read_tide_elev()	-read next tide_elev record
 * file_close_tide_elev()	-close tide_elev file
 *
 * Complete routines (do open, function and close)
 * -----------------------------------------------
 * file_get_tide_zone()		-get requested tide_zone record
 * file_put_tide_zone()		-put a new record in tide_zone file
 * file_update_tide_zone()	-update a new record in tide_zone file
 * file_delete_tide_zone()	-delete a tide_zone record 

 * file_get_tide_elev()		-get requested tide_elev record
 * file_put_tide_elev()		-put a new record in tide_elev file
 * file_count_tide_elev()	-counts the number of tide_elev records
 * file_update_tide_elev()	-update a new record in tide_elev file
 * file_delete_tide_elev()	-delete a tide_elev record 
 *int file_get_time_range_tide_elev() - get min and max timestamp for zone
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#ifndef UNIX
#include <io.h>
#endif

#include "tide.h"
#include "file_globals.h"
#include "cnsts.h"

static FILE	*tide_zone_file;
static FILE	*tide_elev_file;
/*****************************************************************************
 * file_open_tide_zone CSU
 *
 * Purpose:  	To open tide zone (thdr) file.  
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
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
int file_open_tide_zone(char* db_name)
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
	/****   OPEN THE tide zone  file     ****/
	/*****************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.thdr", path,  db_name);
#else
	sprintf(filename, "%s\\%s.thdr", path,  db_name);
#endif

	if( (tide_zone_file = fopen(filename,"rb"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	return(0);
}

/*****************************************************************************
 * file_close_tide_zone CSU
 *
 * Purpose:  	To close the tide zone file.  
 *
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
void file_close_tide_zone()
{
	fclose(tide_zone_file);
}

/*****************************************************************************
 * file_read_tide_zone CSU
 *
 * Purpose:  	To get the next record from tide zone file.  
 *
 * Description:	
 *		-read in the next record
 *		
 *		On failure, return a -1 for error or FILE_EOF
 *
 * Inputs:
 *		1)   point to structure to hold tide_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
int file_read_tide_zone(TIDE_ZONE_T* tide_zone_p)
{
	int	ret_code = 1;
	int	bytes;

	if((bytes = fread(tide_zone_p, sizeof(char), sizeof(TIDE_ZONE_T), 
		tide_zone_file))==0)
	{
		/* the read failed ... */
			
		if(feof(tide_zone_file))
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
 * file_open_tide_elev CSU
 *
 * Purpose:  	To open the tide elevation (tdtl) file.  
 *
 * Description:	Given the dbase (file) name:
 *		-open file
 *		
 *		On failure, return a (negative) FILE_XXX value.
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
int file_open_tide_elev(char* db_name)
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
	/****   OPEN THE tide elev  file     ****/
	/*****************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.tdtl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.tdtl", path,  db_name);
#endif

	if( (tide_elev_file = fopen(filename,"rb"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	return(0);
}

/*****************************************************************************
 * file_close_tide_elev CSU
 *
 * Purpose:  	To close the tide elevation  file.  
 *
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
void file_close_tide_elev()
{
	fclose(tide_elev_file);
}

/*****************************************************************************
 * file_read_tide_elev CSU
 *
 * Purpose:  	To get the next record from tide elevation file.  
 *
 * Description:	
 *		-read in the next record
 *		
 *		On failure, return FILE_XXX code (negative)
 *
 * Inputs:
 *		1)   point to structure to hold tide_elev data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
int file_read_tide_elev(TIDE_ELEV_T* tide_elev_p)
{
	int	ret_code = 1;
	int	bytes;

	if((bytes = fread(tide_elev_p, sizeof(char), sizeof(TIDE_ELEV_T), 
		tide_elev_file)) == 0)
	{
		/* the read failed ... */
			
		if(feof(tide_elev_file))
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
 * file_get_tide_zone CSU
 *
 * Purpose:  	To get specified record from tide_zone file.  
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
 *		3)   point to structure to hold tide_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/
int file_get_tide_zone(char* db_name, int zone_number, TIDE_ZONE_T*tide_zone_p)
{
	int	ret_code = 0;
	int	count = 0;


	if(file_open_tide_zone(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_tide_zone(tide_zone_p) < 0)
		{
			break;
		}
		/* Increment count */
		count++;

		/* FOUND IT!! */
		if(tide_zone_p->zone_number == zone_number)
		{
			ret_code = count;
			break;
		}
	}

	file_close_tide_zone();

	return(ret_code);
}

/*****************************************************************************
 * file_get_tide_elev CSU
 *
 * Purpose:  	To get specified record from tide_elev file.  
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
 *		3)   point to structure to hold tide_elev data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *
 * 99/01/13 DR	-changed so if the third arg (time_stamp) is -1, it
 *		find the first elevation entery for that zone.  Currently
 *		only used in the delete zone process...
 *
 *****************************************************************************/
int file_get_tide_elev(char* db_name, int zone_number, int time_stamp, TIDE_ELEV_T*tide_elev_p)
{
	int	count = 0;
	int	ret_code = 0;

	if(file_open_tide_elev(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_tide_elev(tide_elev_p) < 0)
		{
			break;
		}
		/* Increment count */
		count++;

		/* FOUND IT!! */
		if((tide_elev_p->zone_number == zone_number) &&
		   (tide_elev_p->time_stamp == time_stamp))
		{
			ret_code = count;
			break;
		}
		else if((tide_elev_p->zone_number == zone_number) &&
		   (time_stamp == -1))
		{
			ret_code = count;
			break;
		}
	}

	file_close_tide_elev();

	return(ret_code);
}

/*****************************************************************************
 * file_count_tide_elev
 *
 * Purpose:  	To count the number of elevation records in the file for 
 *		a given survey date.  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   survey date
 *
 * Returns:	Negative FILE_XXX code for file error
 *		0 for no matches
 *		positive count for # of matches 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/

int file_count_tide_elev(char* db_name, int survey_date)
{
	TIDE_ELEV_T	tide_elev_p;
	int	count = 0;

	if(file_open_tide_elev(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_tide_elev(&tide_elev_p) < 0)
		{
			break;
		}
		if(tide_elev_p.date == survey_date)
			count++;

	}

	file_close_tide_elev();

	return(count);
}

/*****************************************************************************
 * file_get_time_range_tide_elev
 *
 * Purpose:  	To get the min and max timestamp for a given survey date
 *		for elevation records.  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   survey date
 *
 * Outputs:	1)   Minimum Timestamp
 * 		2)   Maximum Timestamp
 *
 * Returns:	Negative FILE_XXX code for file error
 * 
 * Modifications:
 * ==============
 * 98/09/21 DR	-wrote routine
 *****************************************************************************/

int file_get_time_range_tide_elev(char* db_name, int survey_date, int *min_time, int *max_time)
{
	TIDE_ELEV_T	tide_elev_p;

	(*min_time) = (int)(0xffffffff/2);
	(*max_time) = 0;

	if(file_open_tide_elev(db_name) < 0) 
		return(FILE_OPEN_ERR);

	for(;;)
	{
		/* Get a record */
	 	if(file_read_tide_elev(&tide_elev_p) < 0)
		{
			break;
		}
		if(tide_elev_p.date = survey_date)
		{
			if(tide_elev_p.time_stamp < (*min_time))
				(*min_time) = tide_elev_p.time_stamp;
			if(tide_elev_p.time_stamp > (*max_time))
				(*max_time) = tide_elev_p.time_stamp;
		}
	}
	file_close_tide_elev();

	return(FILE_OK);
}

/*****************************************************************************
 * file_put_tide_zone CSU
 *
 * Purpose:  	To put a record into tide zone file.  
 *
 * Description:	Given the dbase (file) name and a pointer to the zone:
 *		-open file
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   pointer to structure holding zone data
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/

int file_put_tide_zone(char* db_name, TIDE_ZONE_T*tide_zone_p)
{
	char	filename[100];
	int	ret_code = 1;
	FILE 	*tide_zone_file;
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
	/****   OPEN THE TIDE ZONE file     ****/
	/**************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.thdr", path,  db_name);
#else
	sprintf(filename, "%s\\%s.thdr", path,  db_name);
#endif

	if( (tide_zone_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	if(fwrite(tide_zone_p, sizeof(char), sizeof(TIDE_ZONE_T), 
		tide_zone_file) == 0)
	{
		ret_code = FILE_WRITE_ERR;
	}

	fclose(tide_zone_file);

	return(ret_code);
}


/*****************************************************************************
 * file_put_tide_elev CSU
 *
 * Purpose:  	To put a record into tide elev file.  
 *
 * Description:	Given the dbase (file) name and a pointer to the elevation rec:
 *		-open file
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   pointer to structure holding elev data
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/16 DR	-wrote routine
 *****************************************************************************/

int file_put_tide_elev(char* db_name, TIDE_ELEV_T*tide_elev_p)
{
	char	filename[100];
	int	ret_code = 1;
	FILE 	*tide_elev_file;
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
	/****   OPEN THE TIDE ZONE file     ****/
	/**************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.tdtl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.tdtl", path,  db_name);
#endif

	if( (tide_elev_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	if(fwrite(tide_elev_p, sizeof(char), sizeof(TIDE_ELEV_T), 
		tide_elev_file) == 0)
	{
		ret_code = FILE_WRITE_ERR;
	}

	fclose(tide_elev_file);

	return(ret_code);
}

/*****************************************************************************
 * file_update_tide_zone CSU
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
 *		3)   point to structure holding tide_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 * 
 * 98/09/18 DR	-added args of cur_zone and cur_timestamp to allow
 *		 the ability to update the zone number and/or tne timestamp
 *****************************************************************************/
int file_update_tide_zone(char* db_name, int cur_zone, TIDE_ZONE_T*tide_zone_p)
{
	char	filename[100];
	int	ret_code = 1;
	int 	tide_zone_file;
	char 	*path;
	TIDE_ZONE_T	temp_tide_zone_p;
	int	bytes;


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
	sprintf(filename, "%s/%s.thdr", path,  db_name);
	if( (tide_zone_file = open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s.thdr", path,  db_name);
	if( (tide_zone_file = open(filename,O_RDWR | O_BINARY))==-1)
#endif
	{
		return(FILE_OPEN_ERR);
 
	}

	for(;;)
	{
		/* Get a record */
	 	if((bytes = read(tide_zone_file, &temp_tide_zone_p, 
		   sizeof(TIDE_ZONE_T))) <= 0)
		{
			/* Check if the read failed (0=EOF) */
			if(bytes < 0)
				ret_code = FILE_READ_ERR;
			break;
		}
		if(temp_tide_zone_p.zone_number == cur_zone)
		{

			lseek(tide_zone_file, (-1) * (int)sizeof(TIDE_ZONE_T), 
				SEEK_CUR);

			/* Found the record we want so back up the 
			 * file pointer to so we can overwrite it with
			 * the new data...
			 */
		 	if(write(tide_zone_file, tide_zone_p,  
			   sizeof(TIDE_ZONE_T))==-1)
			{
				ret_code = FILE_WRITE_ERR;
			}
			break;
		}
	}

	close(tide_zone_file);

	return(ret_code);
}
/*****************************************************************************
 * file_update_tide_elev CSU
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
 *		3)   point to structure holding tide_elev data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 * 
 * 98/09/18 DR	-added args of cur_zone and cur_timestamp to allow
 *		 the ability to update the zone number and/or tne timestamp
 * 
 * 99/02/08 DR	-moved check on time from begining of loop to after read.
 *
 *****************************************************************************/
int file_update_tide_elev(char* db_name, int cur_zone, int cur_timestamp, TIDE_ELEV_T*tide_elev_p)
{
	char	filename[100];
	int	ret_code = 1;
	int 	tide_elev_file;
	char 	*path;
	TIDE_ELEV_T	temp_tide_elev_p;
	int	update_zone_only = FALSE;
	int	time_criteria, bytes;


	/* Check for special case where couple of items are set to -1
	 * to flag that only the zone is to be updated (is also only
	 * item that was passed so update is done differently below).
	 */
	if((tide_elev_p->time_stamp == -1) && (tide_elev_p->date == -1))
	{
		update_zone_only = TRUE;
	}
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
	/****   OPEN THE OTF_DETAIL file **/
	/**********************************/

#ifdef UNIX
	sprintf(filename, "%s/%s.tdtl", path,  db_name);
	if( (tide_elev_file = open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s.tdtl", path,  db_name);
	if( (tide_elev_file = open(filename,O_RDWR | O_BINARY))==-1)
#endif
	{
		return(FILE_READ_ERR);
 
	}

	for(;;)
	{
		
		/* Get a record */
	 	if((bytes = read(tide_elev_file, &temp_tide_elev_p, 
		   sizeof(TIDE_ELEV_T))) <= 0)
		{
			/* Check if the read failed (0=EOF) */
			if(bytes < 0)
				ret_code = FILE_READ_ERR;
			break;
		}

		/* The timestamp is an optional criteria.  If a 0 is
		 * passed, it is ignored, but if a time is passed,
		 * the passed time must match the read in time...
		 */
		if(cur_timestamp == 0) 
			time_criteria = TRUE;
		else if (temp_tide_elev_p.time_stamp == cur_timestamp)
			time_criteria = TRUE;
		else
			time_criteria = FALSE;
			
		if((temp_tide_elev_p.zone_number == cur_zone) && 
		   (time_criteria == TRUE))
		{	
			/* If the zone is the only item being updated, use
			 * info from "temp" structure for rest of values.
			 */
			if(update_zone_only == TRUE)
			{
				tide_elev_p->time_stamp =
					temp_tide_elev_p.time_stamp;
				tide_elev_p->date = temp_tide_elev_p.date;
				tide_elev_p->time = temp_tide_elev_p.time;
				tide_elev_p->elevation = 
					temp_tide_elev_p.elevation;
				tide_elev_p->fill = temp_tide_elev_p.fill;
			}		

			lseek(tide_elev_file, (-1) * (int)sizeof(TIDE_ELEV_T), 
				SEEK_CUR);

			/* Found the record we want so back up the 
			 * file pointer to so we can overwrite it with
			 * the new data...
			 */
		 	if(write(tide_elev_file, tide_elev_p,  
			   sizeof(TIDE_ELEV_T))==-1)
			{
				ret_code = FILE_WRITE_ERR;
			}
		}
	}

	close(tide_elev_file);

	return(ret_code);
}

/*****************************************************************************
 * file_delete_tide_zone CSU
 *
 * Purpose:  	To delete the specified record from tide_zone file.  
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
 *		3)   point to structure to hold tide_zone data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *****************************************************************************/
int file_delete_tide_zone(char* db_name, int zone_number)
{
	char	filename[100];
	int	ret_code = 1;
	char 	*path;
	TIDE_ZONE_T	tide_zone_p;
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
	sprintf(filename, "%s/%s.thdr", path,  db_name);
#else
	sprintf(filename, "%s\\%s.thdr", path,  db_name);
#endif

	/* Find out the record number of the zone we want to delete */
	record_number = file_get_tide_zone(db_name, zone_number, &tide_zone_p);

	if(( FILECODE = file_delete_record(filename, record_number, 
		sizeof(TIDE_ZONE_T))) < 0)
	{
		return(FILE_ERROR);
	}
	return(1);
}

/*****************************************************************************
 * file_delete_tide_elev CSU
 *
 * Purpose:  	To delete the specified record from tide_elev file.  
 *
 * Description:	Given the dbase (file) name and a zone number and time_stamp:
 *		-generate input file name
		-figure out record number
 *		-call file_delete_record() routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   zone_number
 *		3)   time_stamp
 *		4)   point to structure to hold tide_elev data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *
 * 99/01/13 DR	-changed so if the third arg (time_stamp) is -1, it
 *		will delete all the elevation enteries for that zone.
 *****************************************************************************/
int file_delete_tide_elev(char* db_name, int zone_number, int time_stamp)
{
	char	filename[100];
	int	ret_code = 1;
	char 	*path;
	TIDE_ELEV_T	tide_elev_p;
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
	sprintf(filename, "%s/%s.tdtl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.tdtl", path,  db_name);
#endif

	if(time_stamp != -1) /* Delete specified record */
	{
		/* Find out the record number of the elevation 
		 * we want to delete 
		 */
		record_number = file_get_tide_elev(db_name, zone_number, 
			time_stamp, &tide_elev_p);
		if(record_number <= 0)
		{
			return(FILE_NOT_FOUND);
		}

		if(( FILECODE = file_delete_record(filename, record_number, 
			sizeof(TIDE_ELEV_T))) < 0)
		{
			return(FILE_ERROR);
		}
	}
	else	/* Delete all records for the whole zone */
	{
		for(;;)
		{
			/* Find out the record number of the elevation 
			 * we want to delete 
			 */
			record_number = file_get_tide_elev(db_name, zone_number,
				-1, &tide_elev_p);

			/* This should mean that we got them all already */
			if(record_number <= 0)
			{
				return(1);
			}


			if(( FILECODE = file_delete_record(filename, 
				record_number, sizeof(TIDE_ELEV_T))) < 0)
			{
				return(FILE_ERROR);
			}
			printf("Zone %d, rec_number %d deleted\n",
				zone_number, record_number);

		}
	}
	return(1);
}
 
