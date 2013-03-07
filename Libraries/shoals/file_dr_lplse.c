/*****************************************************************************
 * file_dr_lplse.c
 *
 * This file contains all routines required for accessing the the
 * file for late diagnostics and results (dr_lplse) 
 * files. 
 *
 * Contains:
 *==========
 * file_delete_drlp()	-delete the drlp file
 * file_open_drlp()	-Open the drlp file
 * file_read_drlp()	-get [next] a record from the drlp file
 * file_write_drlp()	-put a record to the drlp file
 * file_get_drlp()	-get requested timestamp's drlp record
 *
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "diag_res.h"
#include "file_globals.h"


/*****************************************************************************
 * file_open_drlp
 *
 * Purpose:  	To open the  drlp file...[will be opened in "r+" mode
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
FILE	*file_open_drlp( char *db_name, int flightline )
{
	FILE	*drlp_file;
	char	filename[100];
	char 	*path;




	drlp_file = NULL;
	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(drlp_file); 
	}
	
	/*************************************/
	/****   OPEN THE OUTPUT file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.drlp", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.drlp", path,  db_name, flightline);
#endif

	
	if( (drlp_file = fopen(filename,"rb+"))==NULL)
	{
		 return(drlp_file); 
	}

	return(drlp_file);
}
/*****************************************************************************
 * file_delete_drlp
 *
 * Purpose:  	To open the  drlp file...[will be opened in "r+" mode
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

int	file_delete_drlp(char *db_name, int flightline)
{
	char	filename[100];
	FILE	*drlp_file;


	/* delete the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.drlp", file_get_GB_DATA_DIR(),  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.drlp", file_get_GB_DATA_DIR(),  db_name, flightline);
#endif

	unlink(filename);
	if( (drlp_file = fopen(filename,"wb+"))==NULL)
	{
		return(FILE_CREATE_ERR); 
	}
	fclose(drlp_file);

	return(FILE_OK);
}


/*****************************************************************************
 * file_read_drlp CSU
 *
 * Purpose:  	To get a record from the open drlp file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold drlp data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/
int file_read_drlp(FILE *drlp_file, DIAG_RES_LPULSE_T *drlp_p )
{

	/* Get a record */
 	if(fread(drlp_p, sizeof(char),sizeof(DIAG_RES_LPULSE_T), 
		   drlp_file) == 0)
	{		
		if(feof(drlp_file))
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
 * file_write_drlp CSU
 *
 * Purpose:  	To put a record into drlp file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold drlp data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_drlp(FILE *drlp_file, DIAG_RES_LPULSE_T *drlp_p )
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(drlp_p, sizeof(char), sizeof(DIAG_RES_LPULSE_T), 
		drlp_file)) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

