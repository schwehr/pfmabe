/*****************************************************************************
 * file_res_pulse.c
 *
 * This file contains all routines required for accessing the
 * file for results pulse 
 *
 * Contains:
 *==========
 * file_delete_resp()	-delete the results pulse file
 * file_open_resp()	-Open the results pulse file
 * file_read_resp()	-get [next] a record from the results pulse file
 * file_write_resp()	-put a record to the results pulse file
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag_res.h"
#include "file_globals.h"

/*****************************************************************************
 * file_open_resp
 *
 * Purpose:  	To open the  results pulse file...[will be opened in "r+" 
 *		mode which will allow both reading and writing]  
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
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
FILE *file_open_resp(char *db_name, int flightline)
{
	FILE	*resp_file;
	char	filename[100];

	resp_file = NULL;

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.rp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.rp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	
	if( (resp_file = fopen(filename,"rb+"))==NULL)
	{
		 return(resp_file); 
	}

	return(resp_file);
}
/*****************************************************************************
 * file_delete_resp
 *
 * Purpose:  	To delete the results pulse file for the given flightline
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

int	file_delete_resp(char *db_name, int flightline)
{
	char	filename[100];
	FILE	*resp_file;

	
	/* delete the file and then recreate it (empty) */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.rp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.rp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	unlink(filename);
	if( (resp_file = fopen(filename,"wb+"))==NULL)
	{
		return(FILE_CREATE_ERR); 
	}
	fclose(resp_file);

	return(FILE_OK);
}


/*****************************************************************************
 * file_read_resp CSU
 *
 * Purpose:  	To get a record from the open results pulse file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   pointer to structure to hold results pulse data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
int file_read_resp(FILE *resp_file, RES_PULSE_T *resp_p )
{

	/* Get a record */
 	if(fread(resp_p, sizeof(char),sizeof(RES_PULSE_T), 
		   resp_file) == 0)
	{		
		if(feof(resp_file))
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
 * file_write_resp CSU
 *
 * Purpose:  	To put a record into results pulse file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   point to structure to hold results pulse data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_resp(FILE *resp_file, RES_PULSE_T *resp_p )
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(resp_p, sizeof(char), sizeof(RES_PULSE_T), 
		resp_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

