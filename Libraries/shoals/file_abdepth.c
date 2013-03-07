/*****************************************************************************
 * file_abdepth.c
 *
 * This file contains all routines required for accessing the abdepth 
 * files. 
 *
 * Contains:
 *==========
 * file_open_abdepth()	-Open the  abdepth file
 * file_read_abdepth()	-read a record from the abdepth file
 * file_write_abdepth()	-write a record to the abdepth file
 *
 *****************************************************************************/
#include <stdio.h>
#include "file_globals.h"
#include "abdepth.h"

/*****************************************************************************
 * file_open_abdepth
 *
 * Purpose:  	To open the  abdepth file...[will be opened in "w+" mode
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
 *****************************************************************************/

FILE *file_open_abdepth( char *db_name, int flightline )
{
	FILE	*abdepth_file;
	char	filename[100];

	abdepth_file = NULL;
	

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.abd", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.abd", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	if( (abdepth_file = fopen(filename,"rb+"))==NULL)
	{
		 return(abdepth_file); 
	}

	return(abdepth_file);
}


/*****************************************************************************
 * file_read_abdepth CSU
 *
 * Purpose:  	To read a record from the open abdepth file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold abdepth data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/
int file_read_abdepth( FILE *abdepth_file, ABDEPTH_GRD_T *abdepth_p )
{

	/* Get a record */
 	if(fread(abdepth_p, sizeof(char),sizeof(ABDEPTH_GRD_T), 
		   abdepth_file) == 0)
	{		
		if(feof(abdepth_file))
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
 * file_write_abdepth CSU
 *
 * Purpose:  	To write a record into abdepth file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold abdepth data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_abdepth( FILE *abdepth_file, ABDEPTH_GRD_T *abdepth_p )
{
	int	ret_val;
	if((ret_val = fwrite(abdepth_p, sizeof(char), sizeof(ABDEPTH_GRD_T), 
	    abdepth_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}
