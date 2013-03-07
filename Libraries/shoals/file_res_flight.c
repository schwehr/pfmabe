/*****************************************************************************
 * file_res_flight.c
 *
 * This file contains all routines required for accessing the
 * file for results flight
 *
 * Contains:
 *==========
 * file_open_resf()	-Open the results flight file
 * file_read_resf()	-get [next] a record from the results flight file
 * file_write_resf()	-put a record to the results flight file
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag_res.h"
#include "file_globals.h"

/*****************************************************************************
 * file_open_resf
 *
 * Purpose:  	To open the results flight file...[will be opened in "r+"
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
FILE	*file_open_resf(char* db_name)
{
	FILE	*resf_file;
	char	filename[100];
	
	resf_file = NULL;
	
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s.rf", file_get_GB_DATA_DIR(),  db_name);
#else
	sprintf(filename, "%s\\%s.rf", file_get_GB_DATA_DIR(),  db_name);
#endif

	
	if( (resf_file = fopen(filename,"rb+"))==NULL)
	{
		 return(resf_file); 
	}

	return(resf_file);
}


/*****************************************************************************
 * file_read_resf CSU
 *
 * Purpose:  	To get a record from the open results flight file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold results flight data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
int file_read_resf(FILE *resf_file, RES_FLIGHT_T *resf_p)
{
	/* Get a record */
 	if(fread(resf_p, sizeof(char),sizeof(RES_FLIGHT_T), 
		   resf_file) == 0)
	{		
		if(feof(resf_file))
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
 * file_write_resf CSU
 *
 * Purpose:  	To put a record into results flight file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold results flight data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/

int file_write_resf(FILE *resf_file, RES_FLIGHT_T *resf_p)
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(resf_p, sizeof(char), sizeof(RES_FLIGHT_T), 
		resf_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

