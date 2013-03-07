/*****************************************************************************
 * file_diag_flight.c
 *
 * This file contains all routines required for accessing the
 * file for diagnostics flight
 *
 * Contains:
 *==========
 * file_open_diagf()	-Open the diagnostics flight file
 * file_read_diagf()	-get [next] a record from the diagnostics flight file
 * file_write_diagf()	-put a record to the diagnostics flight file
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag_res.h"
#include "file_globals.h"

/*****************************************************************************
 * file_open_diagf
 *
 * Purpose:  	To open the diagnostics flight file...[will be opened in "r+"
 *		mode which will allow both reading and writing]  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
FILE	*file_open_diagf(char *db_name)
{
	FILE	*diagf_file;
	char	filename[100];
	
	diagf_file = NULL;
	
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.df", file_get_GB_DATA_DIR(),  db_name);
#else
	sprintf(filename, "%s\\%s.df", file_get_GB_DATA_DIR(),  db_name);
#endif
	
	if( (diagf_file = fopen(filename,"rb+"))==NULL)
	{
		 return(diagf_file); 
	}

	return(diagf_file);
}


/*****************************************************************************
 * file_read_diagf CSU
 *
 * Purpose:  	To get a record from the open diagnostics flight file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold diag flight data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
int file_read_diagf(FILE *diagf_file, DIAG_FLIGHT_T *diagf_p )
{
	/* Get a record */
 	if(fread(diagf_p, sizeof(char),sizeof(DIAG_FLIGHT_T), 
		   diagf_file) == 0)
	{		
		if(feof(diagf_file))
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
 * file_write_diagf CSU
 *
 * Purpose:  	To write a record into diagnostics flight file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold diagnostics flight data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/

int file_write_diagf( FILE *diagf_file, DIAG_FLIGHT_T *diagf_p )
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(diagf_p, sizeof(char), sizeof(DIAG_FLIGHT_T), 
		diagf_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

