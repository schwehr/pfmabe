/*****************************************************************************
 * file_ins.c
 *
 * This file contains all routines required for accessing the ins
 * files. 
 *
 * Contains:
 *==========
 * file_open_ins64()	-Open the ins 64 file
 * file_open_ins32()	-Open the ins 32 file
 * file_read_ins32()	-read a record from the ins32form file
 * file_write_ins32()	-write a record to the ins32form file
 * file_get_ins32()	-get specified ins32 from file
 * file_get_ins64()	-get specified ins64 from file
 *****************************************************************************/
#include <stdio.h>
#include "file_globals.h"
#include "ins.h"

/*****************************************************************************
 * file_open_ins64
 *
 * Purpose:  	To open the ins64 file...[will be opened in "r+" mode
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
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/

FILE *file_open_ins64(char *db_name, int flightline)
{
	FILE	*ins64_file;
	char	filename[100];

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.ins64", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#else
	sprintf(filename, "%s\\%s_%d.ins64", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#endif

	if( (ins64_file = fopen(filename,"rb+"))==NULL)
	{
		 return(ins64_file); 
	}
	/* Set pointer to BOF */
	rewind(ins64_file);

	return(ins64_file);
}

/*****************************************************************************
 * file_open_ins32
 *
 * Purpose:  	To open the ins32 file...[will be opened in "r+" mode
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
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/

FILE *file_open_ins32(char *db_name, int flightline)
{
	FILE	*ins32_file;
	char	filename[100];	

	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.ins32", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#else
	sprintf(filename, "%s\\%s_%d.ins32", file_get_GB_DATA_DIR(), db_name,
		 flightline);
#endif

	if( (ins32_file = fopen(filename,"rb+"))==NULL)
	{
		 return(ins32_file); 
	}
	/* Set pointer to BOF */
	rewind(ins32_file);

	return(ins32_file);
}


/*****************************************************************************
 * file_read_ins64 CSU
 *
 * Purpose:  	To read a record from the open ins64 file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold ins64 data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/
int file_read_ins64(FILE *ins64_file, INS_64_T *ins64_p)
{

	/* Get a record */
 	if(fread(ins64_p, sizeof(char),sizeof(INS_64_T), 
		   ins64_file) == 0)
	{		
		if(feof(ins64_file))
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
 * file_read_ins32 CSU
 *
 * Purpose:  	To read a record from the open ins32 file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold ins32 data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/
int file_read_ins32(FILE *ins32_file, INS_32_T *ins32_p)
{
	/* Get a record */
 	if(fread(ins32_p, sizeof(char),sizeof(INS_32_T), 
		   ins32_file) == 0)
	{		
		if(feof(ins32_file))
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
 * file_write_ins64 CSU
 *
 * Purpose:  	To write a record into ins64 file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold ins data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/

int file_write_ins64(FILE *ins64_file, INS_64_T *ins64_p)
{
	if(fwrite(ins64_p, sizeof(char), sizeof(INS_64_T), ins64_file) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}
/*****************************************************************************
 * file_write_ins32 CSU
 *
 * Purpose:  	To write a record into ins32 file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold ins data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *****************************************************************************/

int file_write_ins32(FILE *ins32_file, INS_32_T *ins32_p)
{
	if(fwrite(ins32_p, sizeof(char), sizeof(INS_32_T), ins32_file) == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}
/*****************************************************************************
 * file_get_ins64 CSU
 *
 * Purpose:  	To get the requested record from ins64 file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the ins64 data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold ins64 data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *
 *****************************************************************************/

int file_get_ins64(char *db_name, short flightline, int timestamp, INS_64_T *ins64_p)
{
	int	ret_val = FILE_OK;
	int	FILECODE;
	FILE 	*ins64_file;

	/*
	*******************************
	*   OPEN THE INS 64 file 
	*******************************
	*/

	if(( ins64_file = file_open_ins64(db_name, flightline))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


	/* Go to the approximate location of the record in the file */
	file_guess_record_location(ins64_file, db_name, flightline,
		timestamp, sizeof(INS_64_T));

	for(;;)
	{
		/* Get a record */
	 	FILECODE = file_read_ins64(ins64_file, ins64_p);
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
		if(ins64_p->timestamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(ins64_file);

	return(ret_val);
}


/*****************************************************************************
 * file_get_ins32 CSU
 *
 * Purpose:  	To get the requested record from ins32 file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the ins32 data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold ins32 data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 99/05/03 DR	-wrote routine
 *
 * 99/06/11 DR	-in initial version, data was being put into "INS_64_T"
 *		structure (which is bigger luckily).  This was fixed
 *		
 *****************************************************************************/

int file_get_ins32(char *db_name, short flightline, int timestamp, INS_32_T *ins32_p)
{
	int	ret_val = FILE_OK;
	int	FILECODE;
	FILE 	*ins32_file;

	/*
	*******************************
	*   OPEN THE INS 32 file 
	*******************************
	*/

	if(( ins32_file = file_open_ins32(db_name, flightline))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


	/* Go to the approximate location of the record in the file */
	file_guess_record_location(ins32_file, db_name, flightline,
		timestamp, sizeof(INS_32_T));

	for(;;)
	{
		/* Get a record */
	 	FILECODE = file_read_ins32(ins32_file, ins32_p);
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
		if(ins32_p->timestamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(ins32_file);

	return(ret_val);
}
