/*****************************************************************************
 * file_globals.c
 *
 * This file contains a few globals defined for use with file routines...
 * and a number of more commonly used file functions (get env vars...)
 *
 * Contains:
 *==========
 * defintion of some globals for files...
 * file_get_GB_DATA_DIR()	-get the env var $GB_DATA_DIR
 * file_get_GROUNDBASE()	-get the env var $GROUNDBASE
 * file_get_GG_LOG_DIR()	-get the env var $GG_LOG_DIR
 * file_get_GB_ARCHIVE_DIR()	-get the env var $GB_ARCHIVE_DIR
 * file_create_mission_files	- will create all mission files
 * file_create_line_files	- will create all line files (for given line)
 * file_guess_record_location()	- will take guess at where record is in file
 * file_guess_record_location_ub()- will take guess at where record is in file
 *				  (for unbuffered IO file)
 * file_delete_record() 	-will delete the specified record number from
 *				 a file (works for any fixed record files)
 * 
 * 
 * 99/05/03 DR	-added new file extensions for ins32 and 64
 *
 * 99/12/09 DR	-brought up to date with Unix 2.3
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#   include <sys/types.h>
#   include <unistd.h>
#else
#   include <io.h>
#endif
#include "file_globals.h"

#include "flightline.h"
#include "hardware.h"

char mission_file_ext[NUM_MISSION_FILE_TYPES][7] =
{
	"fl",		/* Flight Line */
	"fm",		/* Flight Map */
	"haps",		/* HAPS */
	"hard",		/* hardware */
	"sl",		/* Survey Limits */
	"otfz",		/* OTF Zone */
	"tdtl",		/* tide detail */
	"thdr",		/* tide header */
	"rf",		/* results_flight */
	"df"		/* diagnostics_flight */
};

char	*flight_line_ext 	= mission_file_ext[0];
char	*flight_map_ext 	= mission_file_ext[1];
char	*haps_ext 		= mission_file_ext[2];
char	*hardware_ext 		= mission_file_ext[3];
char	*survey_limits_ext 	= mission_file_ext[4];
char	*otf_zone_ext 		= mission_file_ext[5];
char	*tide_detail_ext 	= mission_file_ext[6];
char	*tide_header_ext 	= mission_file_ext[7];
char	*results_flight_ext 	= mission_file_ext[8];
char	*diagnostics_flight_ext = mission_file_ext[9];

char	line_file_ext[NUM_LINE_FILE_TYPES][7] =
{
	"inp",		/* inputs */
	"wave",		/* waveforms */
	"abd",		/* airborne depth */
	"gps",		/* gps */
	"out",		/* outputs */
	"ogps",		/* otf_gps */
	"dp",		/* diagnostics pulse*/
	"rp",		/* results pulse*/
	"drlp",		/* diag_res_lplse  */
	"ins64",	/* ins64  */
	"ins32"		/* ins32  */
};

#define inputs_ext 		(line_file_ext[0])
#define waveform_ext 		(line_file_ext[1])
#define abdepth_ext 		(line_file_ext[2])
#define gps_ext 		(line_file_ext[3])
#define otf_gps_ext 		(line_file_ext[4])
#define outputs_ext 		(line_file_ext[5])
#define results_pulse_ext 	(line_file_ext[6])
#define diagnostics_pulse_ext 	(line_file_ext[7])
#define diag_res_lplse_ext 	(line_file_ext[8])
#define ins64_ext	 	(line_file_ext[9])
#define ins32_ext	 	(line_file_ext[10])


/*****************************************************************************
 * file_get_GB_DATA_DIR CSU
 *
 * Purpose:  	To get the env var $GB_DATA_DIR.  
 *
 *
 *		On failure, return a NULL
 *
 * Inputs:	1)   ptr to char string for returned path
 *
 * Outputs:     see above 
 * 
 * Modifications:
 * ==============
 * 98/05/13 DR	-wrote routine
 *****************************************************************************/

char *file_get_GB_DATA_DIR()
{
	static char 	path[100];

	if ( getenv( "GB_DATA_DIR" ) == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return NULL;
 	}
	strcpy(path, getenv( "GB_DATA_DIR" ));

	return((char *)path);
}

/*****************************************************************************
 * file_get_GROUNDBASE CSU
 *
 * Purpose:  	To get the env var $GROUNDBASE.  
 *
 *
 *		On failure, return a NULL
 *
 * Inputs:	1)   ptr to char string for returned path
 *
 * Outputs:     see above 
 * 
 * Modifications:
 * ==============
 * 98/05/13 DR	-wrote routine
 *****************************************************************************/

char *file_get_GROUNDBASE()
{
	static char 	path[100];

	if ( getenv( "GROUNDBASE" ) == NULL ) 
	{ 
		printf("${GROUNDBASE} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return NULL;
 	}
	strcpy(path, getenv( "GROUNDBASE" ));


	return((char *)path);
}

/*****************************************************************************
 * file_get_GG_DATA_DIR CSU
 *
 * Purpose:  	To get the env var $GG_DATA_DIR.  
 *
 *
 *		On failure, return a NULL
 *
 * Inputs:	1)   ptr to char string for returned path
 *
 * Outputs:     see above 
 * 
 * Modifications:
 * ==============
 * 98/05/13 DR	-wrote routine
 *****************************************************************************/

int file_get_GG_DATA_DIR(char *gg_path)
{
	char 	*path;

	path = getenv( "GG_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GG_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(FILE_ERROR);
 	}

	if(strcpy(gg_path, path) == NULL)
	{
		printf("STRCPY error copying $GG_DATA_DIR!\n");
		return(FILE_ERROR);

	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_get_GB_ARCHIVE_DIR CSU
 *
 * Purpose:  	To get the env var $GB_ARCHIVE_DIR.  
 *
 *
 *		On failure, return a Negative FILE_xxx code
 *
 * Inputs:	1)   ptr to char string for returned path
 *
 * Outputs:     see above 
 * 
 * Modifications:
 * ==============
 * 98/05/13 DR	-wrote routine
 *****************************************************************************/

int file_get_GB_ARCHIVE_DIR(char *gba_path)
{
	if ( getenv( "GB_ARCHIVE_DIR" ) == NULL ) 
	{ 
		printf("${GB_ARCHIVE_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(FILE_ERROR);
 	}
	strcpy( gba_path, getenv( "GB_ARCHIVE_DIR" ) );
	return(FILE_OK);
}

/*****************************************************************************
 * file_create_mission_files
 *
 * Purpose:  	Will create all files for mission (not line files).  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/22 DR	-wrote routine
 *****************************************************************************/

int	file_create_mission_files(char	*dbname)
{
	char filename[100]; 
	int	i;
	FILE	*file_ptr;
	char 	*path;

	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf( "${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return( FILE_ERROR );  
	}

#ifdef UNIX
	sprintf(filename, "%s/%s.fl", path, dbname);
	if((access(filename, 0) == 0))
#else
	sprintf(filename, "%s\\%s.fl", path, dbname);
	if((_access(filename, 0) == 0))
#endif
	{
		return( FILE_ALREADY_EXISTS );
	}


	for(i=0;i<NUM_MISSION_FILE_TYPES;i++)
	{
#ifdef UNIX
		sprintf(filename, "%s/%s.%s", path, dbname, 
			mission_file_ext[i]);
#else
		sprintf(filename, "%s\\%s.%s", path, dbname, 
			mission_file_ext[i]);
#endif
		if ((file_ptr = fopen(filename,"wb")) ==NULL) 
		{
			return(FILE_CREATE_ERR); 
		}
		fclose(file_ptr);
	}
	return(FILE_OK); 
}

int file_delete_mission_files( const char *db_name)
{
	int i;
	char buf[255];
	char *path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf( "${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return( FILE_ERROR );  
	}

	for(i=0;i<NUM_MISSION_FILE_TYPES;i++)
	{
		strchr(buf, '\0');
#ifdef UNIX
		sprintf(buf, "%s/%s.%s", path, db_name, 
			mission_file_ext[i]);
#else
		sprintf(buf, "%s\\%s.%s", path, db_name, 
			mission_file_ext[i]);
#endif


		if (unlink(buf) != 0) 
		{
			printf("FILE ERROR\n Deleting %s\n", buf);
			return (FILE_ERROR);
		}
	}
	return (FILE_OK);
}


/*****************************************************************************
 * file_create_line_files
 *
 * Purpose:  	Will create all files for a flightline.  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 * 		2)   flightline
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/22 DR	-wrote routine
 *****************************************************************************/

int	file_create_line_files( char *dbname, int flightline )
{
	char *path; 
	char filename[100]; 
	int	i;
	FILE	*file_ptr;

	path = getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf( "${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return( FILE_ERROR );  
	}


	for(i=0;i<NUM_LINE_FILE_TYPES;i++)
	{
#ifdef UNIX
		sprintf(filename, "%s/%s_%d.%s", path, dbname, flightline, 
			line_file_ext[i]);
#else
		sprintf(filename, "%s\\%s_%d.%s", path, dbname, flightline, 
			line_file_ext[i]);
#endif
		if ((file_ptr = fopen(filename,"wb")) ==NULL) 
		{
			return(FILE_CREATE_ERR); 
		}
		fclose(file_ptr);
	}
	return(FILE_OK);
}

int file_delete_line_files( const char *db_name, int flightline )
{
	char *path; 
	char filename[100]; 
	int	i;

	path = getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf( "${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return( FILE_ERROR );  
	}

	for(i=0;i<NUM_LINE_FILE_TYPES;i++)
	{
#ifdef UNIX
		sprintf(filename, "%s/%s_%d.%s", path, db_name, 
			flightline, line_file_ext[i]);
#else
		sprintf(filename, "%s\\%s_%d.%s", path, db_name, 
			flightline, line_file_ext[i]);
#endif
		
		if (unlink(filename) != 0) 
		{
			printf("FILE ERROR\n Deleting %s\n", filename);
			return ( FILE_ERROR );
		}
	}
	return( FILE_OK );
}


/*****************************************************************************
 * file_guess_record_location
 *
 * Purpose:  	Will attemp to find the approx location for a record in a   
 *		file (inputs, outputs, waveform).   This routine was written in
 *		a way that it will work for any open fixed length file.
 *
 * Description:	...
 *
 * Inputs:	1)   handle to an opened file
 * 		2)   database name
 * 		2)   flightline
 * 		2)   timestamp
 *
 * Outputs:     returns a FILE_OK on success or a FILE_ERROR on failure...
 * 
 * Modifications:
 * ==============
 * 98/07/10 DR	-wrote routine
 *****************************************************************************/

int	file_guess_record_location( FILE *file_handle,	/* Opened file */
								char	*db_name,
								short	flightline,
								int	timestamp,
								int	struct_size)	/* sizeof(xxxx_T) in calling routine */
{
	int 	start_pos, offset;
	FLIGHTLINE_GRD_T flightline_p;
	HARD_FILE_T	hard_p;

	file_get_flightline(db_name, flightline, &flightline_p);
	file_get_hardware(db_name, flightline, &hard_p);
	
	start_pos = ((int)(((float)(timestamp - flightline_p.start_time)/2.) /
		     (float)(1000./hard_p.au.rep_rate)) - 10)  * 
		    struct_size;

	if (start_pos > struct_size)
 	{
		offset = fseek(file_handle, start_pos, 0);
	}

	return(FILE_OK);
}

/*****************************************************************************
 * file_guess_record_location_ub
 *
 * Purpose:  	Will attemp to find the approx location for a record in a   
 *		file (inputs, outputs, waveform).   This routine was written in
 *		a way that it will work for any open fixed length file.   This 
 *		version of the routine is for unbuffered I/O
 *
 * Description:	...
 *
 * Inputs:	1)   handle to an opened file (type int not FILE*)
 * 		2)   database name
 * 		2)   flightline
 * 		2)   timestamp
 *
 * Outputs:     returns a FILE_OK on success or a FILE_ERROR on failure...
 * 
 * Modifications:
 * ==============
 * 98/08/25 DR	-wrote routine
 *****************************************************************************/

int	file_guess_record_location_ub(int	filehandle,	/* Opened file */
									char	*db_name,
									short	flightline,
									int	timestamp,
									int	struct_size)	/* sizeof(xxxx_T) in calling routine */
{
	int 	start_pos, offset;
	FLIGHTLINE_GRD_T flightline_p;
	HARD_FILE_T	hard_p;

	/* Need the start time */
	file_get_flightline(db_name, flightline, &flightline_p);
	
	/* Need the rep rate. */
	file_get_hardware(db_name, flightline, &hard_p);

	start_pos = ((int)(((float)(timestamp - flightline_p.start_time)/2.) /
		     (float)(1000./hard_p.au.rep_rate)) - 10)  * 
		    struct_size;

	if (start_pos > struct_size)
 	{
#ifdef UNIX
		offset = lseek(filehandle, start_pos, 0);
#else
		offset = _lseek(filehandle, start_pos, 0);
#endif
	}

	return(FILE_OK);
}

/*****************************************************************************
 * file_delete_record
 *
 * Purpose:  	will delete the specified record (by number) from a file. 
 *		Will work for any fixed length record file (was written for 
 *		otf_zone/tides)
 *
 * Description:	...
 *
 * Inputs:	1)   input filename (includes path)
 * 		2)   record_number
 * 		3)   record_size
 *
 * Outputs:     returns a FILE_OK on success or a FILE_ERROR on failure...
 * 
 * Modifications:
 * ==============
 * 98/09/15 DR	-wrote routine
 *
 * 98/10/12 DR	-delete the temp file after copy..
 *
 * 98/10/28 DR	-changed to 64k record size (hardware is > 16k)..
 *****************************************************************************/

int	file_delete_record( char	*in_filename,
						int		record_number,
						int	record_size)	/* sizeof(xxxx_T) in calling routine */
{
 
	FILE	*in_file, *out_file;
	char	temp_filename[100];
	char 	*path;
	char	*big_buff;	/* 64 k max size for record_size */
	int	count = 0;
	int	ret_code;

	if(record_size > 64*1024)
	{
		printf("Can't delete data for file %s.\n", in_filename);
		printf("Record size of %d is bigger than max allowed of %d\n",
			record_size , sizeof(big_buff));
		return(-100);
	}			

	/* Allocate the buffer space */
	big_buff = (char *)malloc(record_size + 1);

	if(big_buff == NULL)
	{
		printf("Can't delete data for file %s.\n", in_filename);
		printf("Failed allocating space...\n");
		return(-100);
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

	/***************************************/
	/****   OPEN THE (output) TEMP file ****/
	/***************************************/

#ifdef UNIX
	sprintf(temp_filename, "%s/temp_copy_file", path);
#else
	sprintf(temp_filename, "%s\\temp_copy_file", path);
#endif

	if( (out_file = fopen(temp_filename,"wb"))==NULL)
	{
		free(big_buff);
		return(FILE_OPEN_ERR);
 
	}

	/**********************************/
	/****   OPEN THE INPUT file ****/
	/**********************************/
	if( (in_file = fopen(in_filename,"rb"))==NULL)
	{
		free(big_buff);
		return(FILE_OPEN_ERR);
 
	}

	for(;;)
	{
		if(fread((void *)big_buff, sizeof(char),record_size, 
			in_file) == 0)
		{
			/* the read failed ... */
			
			if(feof(in_file))
			{
				ret_code = FILE_EOF;
			}
			else
			{
				ret_code = FILE_READ_ERR;
			}
			break;
		}
		count++;

		if(count != record_number)
		{
			if(fwrite((void *)big_buff, sizeof(char), record_size, 
			   out_file) ==0 )
			{
				ret_code = FILE_WRITE_ERR;
			}
		}
	}

	fclose(in_file);
	fclose(out_file);

	/* Now basically start again opening the temp file as the
	 * input file and open the passed file as the output file 
	 * (which will be deleted and re-writen)
	 */

	/***************************************/
	/****   OPEN THE (INPUT) TEMP file ****/
	/***************************************/

	if( (in_file = fopen(temp_filename,"rb"))==NULL)
	{
		free(big_buff);
		return(FILE_OPEN_ERR);
	}

	/**********************************/
	/****   OPEN THE output file ****/
	/**********************************/
	if( (out_file = fopen(in_filename,"wb"))==NULL)
	{
		free(big_buff);
		return(FILE_OPEN_ERR);
	}

	for(;;)
	{
		if(fread((void *)big_buff, sizeof(char),record_size, 
			in_file) == 0)
		{
			/* the read failed ... */
			
			if(feof(in_file))
			{
				ret_code = FILE_EOF;
			}
			else
			{
				ret_code = FILE_READ_ERR;
			}
			break;
		}

		if(fwrite((void *)big_buff, sizeof(char), record_size, 
			out_file) ==0 )
		{
			ret_code = FILE_WRITE_ERR;
		}
	}

	fclose(in_file);
	fclose(out_file);

	/* delete the temporary file. */
	unlink(temp_filename);
	free(big_buff);
	return(0);

}
