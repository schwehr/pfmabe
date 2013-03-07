/*****************************************************************************
 * file_flightline.c
 *
 * This file contains all routines required for accessing the FLIGHT_LINE file.
 * These routines were written in such a way that it can be called from  
 * anywhere in the PFPS.
 *
 * Contains:
 *==========
 * 
 * file_open_flightline()	-open flightline file
 * file_close_flightline()	-close flightline file
 * file_access_flightline()	-see if file exists 
 * file_read_flightline()	-get next record in file 
 * 
 * Complete functions (open, do something and close file).
 * ===================------------------------------------
 * file_get_flightline()	-get requested flight_line record
 * file_put_flightline()	-put a new record in flight_line file
 * file_delete_flightline()	-deletes a flightline from the flight_line
 *				 file
 * file_get_fl_by_date()	-get a rec from fl file that matches the
 *				 supplied date.
 * file_get_max_flightline()	-get the maximum flightline number
 * file_get_min_flightline()	-get the minimum flightline number
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#ifdef UNIX
#   include <sys/types.h>
#   include <unistd.h>
#else
#   include <io.h>
#endif

#include "flightline.h"
#include "file_globals.h"

/*****************************************************************************
 * file_get_flightline CSU
 *
 * Purpose:  	To get a record from flight_line file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the flight_line data to calling routine
 *
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold flight_line data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *
 * 98/10/29 DR	-Ret code will be the record number for found records...
 *****************************************************************************/
int file_get_flightline( char				*db_name,
						short				flightline,
						FLIGHTLINE_GRD_T	*flightline_p)
{
	int	ret_code = 0;
	FILE 	*flightline_file;

	flightline_file = file_open_flightline(db_name);

	if(flightline_file == NULL)
	{
		ret_code = FILE_OPEN_ERR;
		return(ret_code); 
	}

	for(;;)
	{
		/* Get a record */
		if(file_read_flightline(flightline_file, flightline_p) < 0)
		{
			/* the read failed ... */
			
			if(feof(flightline_file))
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
		if(flightline_p->flightline == flightline) 
		{
			break;
		}
	}

	fclose(flightline_file);

	return(ret_code);
}
/*****************************************************************************
 * file_get_fl_by_date CSU
 *
 * Purpose:  	To get a record from flight_line file that matches the
 *		supplied survey date...  
 *
 * Description:	Given the dbase (file) name and a survey date:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the flight_line data to calling routine
 *
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   Database name
 *		2)   survey date
 *		3)   point to structure to hold flight_line data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/01/06 DR	-wrote routine
 *
 *****************************************************************************/
int file_get_fl_by_date(char 		*db_name,
						int		survey_date,
						FLIGHTLINE_GRD_T	*flightline_p )
{
	int	ret_code = 0;
	FILE 	*flightline_file;

	flightline_file = file_open_flightline(db_name);

	if(flightline_file == NULL)
	{
		ret_code = FILE_OPEN_ERR;
		return(ret_code); 
	}

	for(;;)
	{
		/* Get a record */
		if(file_read_flightline(flightline_file, flightline_p) < 0)
		{
			/* the read failed ... */
			
			if(feof(flightline_file))
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
		if(flightline_p->survey_date == survey_date) 
		{
			break;
		}
	}

	fclose(flightline_file);

	return(ret_code);
}


/*****************************************************************************
 * file_put_flightline CSU
 *
 * Purpose:  	To put a record into flight_line file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-find out if a flight_line record for this line exists, and  
 *		 if so, make version one higher that last...
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold flight_line data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_put_flightline(char		 		*db_name,
						FLIGHTLINE_GRD_T	*flight_line_p)
{
	char	filename[100];
	int	ret_code = 0;
	FILE 	*flightline_file;
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
	
	/****************************************/
	/****   OPEN THE FLIGHTLINE file     ****/
	/****************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.fl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.fl", path,  db_name);
#endif

	if( (flightline_file = fopen(filename,"ab+"))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


  	if(fwrite(flight_line_p, sizeof(char),sizeof(FLIGHTLINE_GRD_T), 
		flightline_file) == 0)
	{
		printf("ERROR writing FLIGHTLINE file.\n");
		ret_code = FILE_WRITE_ERR;
	}

	fclose(flightline_file);

	return(ret_code);
}

/*****************************************************************************
 * file_delete_flightline
 *
 * Purpose:  	To delete a record from the flight_line file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-find requested flightline in file and delete it...
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

int file_delete_flightline(char 	*db_name,
							short	flightline )
{
	/* TBD */
	return 0;
}

/*****************************************************************************
 * file_open_flightline CSU
 *
 * Purpose:  	To  open the flight_line file for READING ONLY
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
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/
FILE *file_open_flightline( char *db_name)
{
	char	filename[100];
	FILE 	*flightline_file;
	char 	*path;

	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n"
				"Set it before starting up \"shoals\"\n");
 	}

	/*************************************/
	/****   OPEN THE FLIGHT_LINE file ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.fl", path,  db_name);
#else
	sprintf(filename, "%s\\%s.fl", path,  db_name);
#endif

	if( (flightline_file = fopen(filename,"rb"))==NULL)
	{
		printf("FILE ERROR\n\topening '%s'\n", filename);
		return (flightline_file);
	}


	return(flightline_file);
}

/*****************************************************************************
 * file_read_flightline CSU
 *
 * Purpose:  	To  get the next record from the flightline file
 *
 * Description:	Given FILE pointer a pointer to a FLIGHTLINE_GRD_T 
 *		structure, this routine will get the next record from the
 *		flightline file.
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
int	file_read_flightline(FILE 			*flightline_file,
						FLIGHTLINE_GRD_T	*flightline_p)
{
	int	ret_code = 0;

	/* Get a record */
	 if(fread(flightline_p, sizeof(char),sizeof(FLIGHTLINE_GRD_T), 
	   flightline_file) == 0)
	{
		/* the read failed ... */
		if(feof(flightline_file))
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
 * file_get_max_flightline
 *
 * Purpose:  	To get the "max" flightline number for the database.  
 *
 * Description:	Given the dbase (file) name :
 *		-open file
 *		-find the maximum flightline number.
 *		-return max number...
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/30 DR	-wrote routine
 *****************************************************************************/

int file_get_max_flightline( char *db_name)
{
	int	ret_code;
	FILE 	*flightline_file;
	FLIGHTLINE_GRD_T	flightline_p;
	int	max_flightline = 0;

	flightline_file = file_open_flightline(db_name);

	if(flightline_file == NULL)
	{
		return(max_flightline); 
	}

	for(;;)
	{
		ret_code = file_read_flightline(flightline_file, 
			&flightline_p);

		if(ret_code < 0)
			break;
			
		/* FOUND IT!! */
		if(flightline_p.flightline > max_flightline) 
		{
			max_flightline = flightline_p.flightline;
		}
	}

	fclose(flightline_file);

	return(max_flightline);
}

/*****************************************************************************
 * file_get_min_flightline
 *
 * Purpose:  	To get the "min" flightline number for the database.  
 *
 * Description:	Given the dbase (file) name :
 *		-open file
 *		-find the minimum flightline number.
 *		-return min number...
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/09/01 DR	-wrote routine
 *****************************************************************************/
 
int file_get_min_flightline(char *db_name)
{
	int	ret_code;
	FILE 	*flightline_file;
	FLIGHTLINE_GRD_T	flightline_p;
	int	min_flightline = 0;

	flightline_file = file_open_flightline(db_name);

	if(flightline_file == NULL)
	{
		return(min_flightline); 
	}

	ret_code = file_read_flightline(flightline_file, 
		&flightline_p);

	if(ret_code < 0)
 		return(1);
			
	/* FOUND IT!! */
	min_flightline = flightline_p.flightline;
	
	fclose(flightline_file);

	return(min_flightline);
}

/*****************************************************************************
 * file_access_flightline CSU
 *
 * Purpose:  	To see if the "flightline" file exists (dbase exists)
 *
 * Description:	Given the dbase (file) name:
 *		-access file
 *		
 *		On failure, return a negative number (FILE_XXXX_ERROR)
 *		else return a 0.
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/
int	file_access_flightline(char *db_name)
{
	char	filename[100];
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
	/****   OPEN THE FLIGHT_LINE file ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.fl", path,  db_name);

	/* Check if file exists (and for read permission) */

	if(access(filename,4) != 0) return -1; 
#else
	sprintf(filename, "%s\\%s.fl", path,  db_name);

	/* Check if file exists (and for read permission) */

	if(_access(filename,4) != 0) return -1; 
#endif

	return 0;
}
