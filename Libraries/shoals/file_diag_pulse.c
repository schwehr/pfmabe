/*****************************************************************************
 * file_diag_pulse.c
 *
 * This file contains all routines required for accessing the
 * file for diagnostics pulse 
 *
 * Contains:
 *==========
 * file_delete_diagp()	-delete the diagnostics pulse file
 * file_open_diagp()	-Open the diagnostics pulse file
 * file_read_diagp()	-get [next] a record from the diagnostics pulse file
 * file_write_diagp()	-put a record to the diagnostics pulse file
 * file_get_diagp() -get a record to the diagnostics pulse file
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag_res.h"
#include "file_globals.h"


/*****************************************************************************
 * file_open_diagp
 *
 * Purpose:  	To open the  diagnostics pulse file...[will be opened in "r+" 
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
FILE	*file_open_diagp( char *db_name, int flightline)
{
	FILE	*diagp_file;
	char	filename[100];

	diagp_file = NULL;

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.dp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.dp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	
	if( (diagp_file = fopen(filename,"rb+"))==NULL)
	{
		 return(diagp_file); 
	}

	return(diagp_file);
}
/*****************************************************************************
 * file_delete_diagp
 *
 * Purpose:  	To delete the diagnostics pulse file for the given flightline
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

int	file_delete_diagp(char *db_name, int flightline)
{
	char	filename[100];
	FILE	*diagp_file;

	
	/* delete the file and then recreate it (empty) */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.dp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.dp", file_get_GB_DATA_DIR(),  
		db_name, flightline);
#endif

	unlink(filename);
	if( (diagp_file = fopen(filename,"wb+"))==NULL)
	{
		return(FILE_CREATE_ERR); 
	}
	fclose(diagp_file);

	return(FILE_OK);
}


/*****************************************************************************
 * file_read_diagp CSU
 *
 * Purpose:  	To get a record from the open diagnostics pulse file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   pointer to structure to hold diagnostics pulse data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/10/12 DR	-wrote routine
 *****************************************************************************/
int file_read_diagp( FILE *diagp_file, DIAG_PULSE_T	*diagp_p )
{

	/* Get a record */
 	if(fread(diagp_p, sizeof(char),sizeof(DIAG_PULSE_T), 
		   diagp_file) == 0)
	{		
		if(feof(diagp_file))
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
 * file_write_diagp CSU
 *
 * Purpose:  	To put a record into diagnostics pulse file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   point to structure to hold diagnostics pulse data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_diagp( FILE *diagp_file, DIAG_PULSE_T *diagp_p )
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(diagp_p, sizeof(char), sizeof(DIAG_PULSE_T), 
		diagp_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_get_diagp CSU
 *
 * Purpose:  	To get the requested record from diagp file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the diagp data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold diagp data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/11/09 DR	-wrote routine
 *
 * 98/11/11 DR	-removed file_get_record_location() call as diagnostics will
 *		almost be run for small ranges only...
 *
 *****************************************************************************/

int file_get_diagp(char *db_name, int flightline, int timestamp, DIAG_PULSE_T *diagp_p)
{
	int	ret_val = FILE_OK;
	int	FILECODE;
	FILE 	*diagp_file;

	if((diagp_file = file_open_diagp(db_name, flightline))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}

	for(;;)
	{
		/* Get a record */
	 	FILECODE = file_read_diagp(diagp_file, diagp_p);
		if(FILECODE < 0)
		{
			ret_val = FILECODE;
			if(FILECODE != FILE_EOF)
			{
				printf("/tRead error\n");
			}
			break;
		}

		/* CHECK IF IT IS THE ONE */
		if(diagp_p->timestamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(diagp_file);

	return(ret_val);
}


