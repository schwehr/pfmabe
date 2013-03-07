/*****************************************************************************
 * file_gps.c
 *
 * This file contains all routines required for accessing the GPS files
 * (both differential and kinematic [OTF]).  As both file types are
 * the same, aside from the open routine, the same access routines
 * are used for both (except OPEN).
 *
 * Contains:
 *==========
 * file_open_new_otf_gps()	-Open a new OTF gps file (overwrite)
 * file_open_new_gps()		-Open a new gps file (overwrite)
 * file_open_otf_gps()	-Open the OTF gps file
 * file_open_gps()	-Open the gps file
 * file_read_gps()	-get a record from the gps file (OTF or trad)
 * file_write_gps()	-counts the number of tide_dtl records
 *
 *****************************************************************************/
#include <stdio.h>
#include "file_globals.h"
#include "gps.h"


/*****************************************************************************
 * file_open_otf_gps
 *
 * Purpose:  	To open the OTF gps file...[will be opened in "w+" mode
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
 * 98/04/01 DR	-wrote routine
 *
 * 98/04/01 DR	-set file pointer to BOF (by rewinding file).
 *****************************************************************************/

FILE *file_open_otf_gps( char *db_name, int	flightline )
{
	FILE	*otf_gps_file;
	char	filename[100];

	otf_gps_file = NULL;

	
	/*************************************/
	/****   OPEN THE OTF GPS file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.ogps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.ogps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	if( (otf_gps_file = fopen(filename,"rb+"))==NULL)
	{
		 return(otf_gps_file); 
	}

	/* Set pointer to BOF */
	rewind(otf_gps_file);

	return(otf_gps_file);
}
/*****************************************************************************
 * file_open_new_gps
 *
 * Purpose:  	To open a new gps file.  If the file exists, it will
 *		be overwritten.  
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
 * 98/12/10 DR	-wrote routine
 *****************************************************************************/

FILE *file_open_new_gps( char *db_name, int	flightline)
{
	FILE	*gps_file;
	char	filename[100];

	gps_file = NULL;

	
	/*********************************/
	/****   OPEN THE GPS file     ****/
	/*********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.gps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.gps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	if( (gps_file = fopen(filename,"wb"))==NULL)
	{
		 return(gps_file); 
	}

	return(gps_file);
}
/*****************************************************************************
 * file_open_new_otf_gps
 *
 * Purpose:  	To open the OTF gps file.  If the file exists, it will
 *		be overwritten.  
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
 * 98/05/14 DR	-wrote routine
 *****************************************************************************/

FILE *file_open_new_otf_gps( char *db_name, int flightline )
{
	FILE	*otf_gps_file;
	char	filename[100];

	otf_gps_file = NULL;

	
	/*************************************/
	/****   OPEN THE OTF GPS file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.ogps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.ogps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	if( (otf_gps_file = fopen(filename,"wb"))==NULL)
	{
		 return(otf_gps_file); 
	}


	return(otf_gps_file);
}

/*****************************************************************************
 * file_open_gps
 *
 * Purpose:  	To open the gps file...[will be opened in "w+" mode
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
 * 98/04/01 DR	-wrote routine
 *
 * 98/04/01 DR	-set file pointer to BOF (by rewinding file).
 *****************************************************************************/

FILE *file_open_gps( char *db_name, int	flightline )
{
	FILE	*gps_file;
	char	filename[100];
	
	gps_file = NULL;

	/*********************************/
	/****   OPEN THE GPS file     ****/
	/*********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.gps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.gps", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	if( (gps_file = fopen(filename,"rb+"))==NULL)
	{
		return(gps_file); 
	}

	/* Set pointer to BOF */
	rewind(gps_file);

	return(gps_file);
}


/*****************************************************************************
 * file_read_gps CSU
 *
 * Purpose:  	To get a record from the open GPS file file.  (OTF or TRAD)
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold gps data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/
int file_read_gps( FILE	*gps_file, GPS_T *gps_p )
{

	/* Get a record */
 	if(fread(gps_p, sizeof(char),sizeof(GPS_T), 
		   gps_file) == 0)
	{		
		if(feof(gps_file))
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
 * file_write_gps CSU
 *
 * Purpose:  	To put a record into gps file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
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

int file_write_gps( FILE *gps_file, GPS_T *gps_p )
{
	if(fwrite(gps_p, sizeof(char),sizeof(GPS_T), gps_file) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}
