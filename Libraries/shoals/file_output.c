/*****************************************************************************
 * file_output.c
 *
 * This file contains all routines required for accessing the output 
 * files. 
 *
 * Contains:
 *==========
 * file_delete_output()	-delete the output file
 * file_open_output()	-Open the output file
 * file_read_output()	-get [next] a record from the output file
 * file_write_output()	-put a record to the output file
 * file_get_output()	-get requested timestamp's output record
 *
 * Lower level (unbuffered) routines so updating can occur [random access]:
 * file_open_rewrite_output()	-open file for update (for reprocessing)
 * file_ll_read_output()	-get [next] a record from the output file 
 * file_setup_rewrite_output()	-will set the file position for updating
 *				 to the correct record.
 * file_rewrite_output()	-update output record (for reprocessing)
 *
 * file_update_rec_output() 	-will update the file for the passed
 *				 record/timestamp (no open/close required)
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef UNIX
#include <io.h>
#endif

#include "file_globals.h"
#include "output.h"

/*****************************************************************************
 * file_open_output
 *
 * Purpose:  	To open the  output file...[will be opened in "r+" mode
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
FILE	*file_open_output(char *db_name, int flightline)
{
	FILE	*output_file;
	char	filename[100];
	char 	*path;

	output_file = NULL;
	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(output_file); 
	}
	
	/*************************************/
	/****   OPEN THE OUTPUT file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.out", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.out", path,  db_name, flightline);
#endif

	if( (output_file = fopen(filename,"rb+"))==NULL)
	{
		 return(output_file); 
	}

	return(output_file);
}
/*****************************************************************************
 * file_open_output
 *
 * Purpose:  	To open the  output file...[will be opened in "r+" mode
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

int	file_delete_output(char *db_name, int flightline)
{
	char	filename[100];
	char 	*path;
	FILE	*output_file;

	/**********************************/
	/****   Find Data  Directories ****/
	/**********************************/
	/* General GB FILES Data dir */
	path = (char *)getenv( "GB_DATA_DIR" );
	if ( path == NULL ) 
	{ 
		printf("${GB_DATA_DIR} undefined !\n\
			Set it before starting up \"shoals\"\n");
		return(FILE_ERROR);
	}
	
	/*************************************/
	/****   OPEN THE OUTPUT file     ****/
	/*************************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.out", path,  db_name, flightline);
#else
	sprintf(filename, "%s\\%s_%d.out", path,  db_name, flightline);
#endif

	unlink(filename);
	if( (output_file = fopen(filename,"wb+"))==NULL)
	{
		return(FILE_CREATE_ERR); 
	}
	fclose(output_file);

	return(FILE_OK);
}


/*****************************************************************************
 * file_read_output CSU
 *
 * Purpose:  	To get a record from the open output file 
 *
 * Description:	
 *		-read in next record
 *		-return a 0 for got one or a -1 for failure
 *
 * Inputs:	1)   file pointer 
 *		2)   point to structure to hold output data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/04/01 DR	-wrote routine
 *****************************************************************************/
int file_read_output(FILE *output_file, OUTPUT_FILE_T *output_p)
{

  /* Get a record */

  if (fread (&output_p->au.timestamp, sizeof (int), 1, output_file) == 0)
    {		
      if (feof (output_file))
	{
	  return (FILE_EOF);
	}
      else
	{
	  return (FILE_READ_ERR);
	}
    }


  /*  This is the only way to get it to work on 64 bit systems.  */

  fread (&output_p->au.flightline, sizeof (short), 1, output_file);
  fread (&output_p->au.haps_version, sizeof (short), 1, output_file);
  fread (&output_p->au.pulse_time, sizeof (double), 1, output_file);
  fread (&output_p->au.latitude, sizeof (double), 1, output_file);
  fread (&output_p->au.longitude, sizeof (double), 1, output_file);
  fread (&output_p->au.sec_latitude, sizeof (double), 1, output_file);
  fread (&output_p->au.sec_longitude, sizeof (double), 1, output_file);
  fread (&output_p->au.tide_cor_depth, sizeof (float), 1, output_file);
  fread (&output_p->au.reported_depth, sizeof (float), 1, output_file);
  fread (&output_p->au.altitude, sizeof (float), 1, output_file);
  fread (&output_p->au.elevation, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_datum, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_elevation, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_res_elev, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_sec_elev, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_topo, sizeof (float), 1, output_file);
  fread (&output_p->au.otf_water_level, sizeof (float), 1, output_file);
  fread (&output_p->au.result_depth, sizeof (float), 1, output_file);
  fread (&output_p->au.sec_depth, sizeof (float), 1, output_file);
  fread (&output_p->au.topo, sizeof (float), 1, output_file);
  fread (&output_p->au.wave_height, sizeof (float), 1, output_file);
  fread (&output_p->au.depth_conf, sizeof (int), 1, output_file);
  fread (&output_p->au.sec_depth_conf, sizeof (int), 1, output_file);
  fread (&output_p->au.ab_dep_conf, sizeof (short), 1, output_file);
  fread (&output_p->au.sec_ab_dep_conf, sizeof (short), 1, output_file);
  fread (&output_p->au.otf_abd_conf, sizeof (short), 1, output_file);
  fread (&output_p->au.otf_sec_abd_conf, sizeof (short), 1, output_file);
  fread (&output_p->au.position_conf, sizeof (short), 1, output_file);
  fread (&output_p->au.status, sizeof (short), 1, output_file);

  fread (&output_p->early.bot_conf, sizeof (float), 1, output_file);
  fread (&output_p->early.sec_bot_conf, sizeof (float), 1, output_file);
  fread (&output_p->early.nadir_angle, sizeof (float), 1, output_file);
  fread (&output_p->early.scanner_azimuth, sizeof (float), 1, output_file);
  fread (&output_p->early.sfc_fom_apd, sizeof (float), 1, output_file);
  fread (&output_p->early.sfc_fom_ir, sizeof (float), 1, output_file);
  fread (&output_p->early.sfc_fom_ram, sizeof (float), 1, output_file);
  fread (&output_p->early.warnings, sizeof (int), 1, output_file);
  fread (&output_p->early.warnings2, sizeof (int), 1, output_file);
  fread (&output_p->early.bot_bin_first, sizeof (short), 1, output_file);
  fread (&output_p->early.bot_bin_second, sizeof (short), 1, output_file);
  fread (&output_p->early.bot_bin_used_pmt, sizeof (short), 1, output_file);
  fread (&output_p->early.sec_bot_bin_used_pmt, sizeof (short), 1, output_file);
  fread (&output_p->early.bot_bin_used_apd, sizeof (short), 1, output_file);
  fread (&output_p->early.sec_bot_bin_used_apd, sizeof (short), 1, output_file);
  fread (&output_p->early.bot_channel, sizeof (short), 1, output_file);
  fread (&output_p->early.sec_bot_chan, sizeof (short), 1, output_file);
  fread (&output_p->early.sfc_bin_apd, sizeof (short), 1, output_file);
  fread (&output_p->early.sfc_bin_ir, sizeof (short), 1, output_file);
  fread (&output_p->early.sfc_bin_ram, sizeof (short), 1, output_file);
  fread (&output_p->early.sfc_channel_used, sizeof (short), 1, output_file);


  /*  This barfs on 64 bit systems.

 	if(fread(output_p, sizeof(char),sizeof(OUTPUT_FILE_T), 
		   output_file) == 0)
	{		
		if(feof(output_file))
		{
			return(FILE_EOF);
		}
		else
		{
			return(FILE_READ_ERR);
		}
	}
  */
	return(FILE_OK);
}

/*****************************************************************************
 * file_write_output CSU
 *
 * Purpose:  	To put a record into output file.  
 *
 * Description:	Given the file pointer and the data pointer
 *		-save new record
 *		-return to calling routine
 *
 *		On failure, return a FILE_WRITE_ERR
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold output data..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/03/18 DR	-wrote routine
 *****************************************************************************/

int file_write_output(FILE *output_file, OUTPUT_FILE_T *output_p)
{
	int	ret_val = FILE_OK;

	if((ret_val = fwrite(output_p, sizeof(char), sizeof(OUTPUT_FILE_T), 
		output_file)) 
	    == 0)
	{
		return(FILE_WRITE_ERR);
	}
	return(FILE_OK);
}

/*****************************************************************************
 * file_open_rewrite_output
 *
 * Purpose:  	To open the output file for updating (low level open)  
 *
 * Description:	...
 *
 * Inputs:	1)   Database name
 *		2)   flightline
 *
 * Outputs:     None 
 * 
 * returns: 	filehandle (or error code)
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *****************************************************************************/
int file_open_rewrite_output(char *db_name, short flightline)
{
	char	filename[100];
	int	ret_val = FILE_OK;
	int 	output_filehandle;
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

	/**********************************/
	/****   OPEN THE OUTPUT file ****/
	/**********************************/
	/* open the file */

#ifdef UNIX
	sprintf(filename, "%s/%s_%d.out", path,  db_name, flightline);
	if( (output_filehandle = open(filename,O_RDWR))==-1)
#else
	sprintf(filename, "%s\\%s_%d.out", path,  db_name, flightline);
	if( (output_filehandle = open(filename,O_RDWR | O_BINARY))==-1)
#endif
	{
		return(FILE_OPEN_ERR);
 
	}
	
	return(output_filehandle);
}


/*****************************************************************************
 * file_setup_rewrite_output CSU
 *
 * Purpose:  	To find the correct position in the output file for 
 *		updating (reprocessing)
 *
 * Description:	Given the file handle, flightline number and timestamp:
 *		-find the record in the file to be updated
 *		-back up to begining of record 
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   db_name
 *		2)   filehandle
 *		2)   flightline number
 *		3)   timestamp..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *
 * 98/07/07 DR	-If an non-timestamp (ie. users guess was off by one)
 *		 processing in this routine would never end.  Added code
 *		 to make sure we don't pass it and to check for EOF.
 *****************************************************************************/
int file_setup_rewrite_output( char *db_name, int filehandle, int flightline, int timestamp)
{
	int	ret_val = FILE_OK;
	OUTPUT_FILE_T	output_p;
	int	bytes_read;

	file_guess_record_location_ub(filehandle, db_name, flightline, 
		timestamp, sizeof(OUTPUT_FILE_T));

	for(;;)
	{
		/* Get a record */
	 	if((bytes_read = read(filehandle, &output_p,
			 sizeof(OUTPUT_FILE_T)))==-1)
		{
			/* the read failed ... */
			ret_val = FILE_READ_ERR;
			break;
		}

		if(bytes_read == 0)
		{
			ret_val = FILE_EOF;
			break;
		}
		
		if(output_p.au.timestamp >= timestamp)
		{
			/* Found the record we want so back up the 
			 * file pointer...
			 */
			lseek(filehandle, (-1) * (int)sizeof(OUTPUT_FILE_T), 
				SEEK_CUR);

			break;
		}
	}

	return(ret_val);
}
/*****************************************************************************
 * file_ll_read_output CSU
 *
 * Purpose:  	Read in nex record from output file.  Lower level (unbuffered)
 *		read is used, so file must be opened using 
 *		file_open_rewrite_output.
 *
 * Description:	Given the file handle and pointer to output structre:
 *		-read in the next record in the file 
 *
 *		On failure, return a -2
 *
 * Inputs:	1)   filehandle
 *		2)   output structure pointer..
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *****************************************************************************/
int file_ll_read_output( int filehandle,  OUTPUT_FILE_T *output_p)
{
	int	ret_val = FILE_OK;
	int	bytes_read;

	/* Get a record */
 	if((bytes_read = read(filehandle, output_p, sizeof(OUTPUT_FILE_T))) 
		<= 0)
	{
		if(bytes_read == 0)
			ret_val = FILE_EOF;
		else
			ret_val = FILE_READ_ERR;
	}

	return(ret_val);
}

/*****************************************************************************
 * file_rewrite_output CSU
 *
 * Purpose:  	To update a record into the output file.
 *
 * Description:	Given the filehandle and the output data:
 *		-update the current record with new data.
 *		-return to calling routine
 *		
 *		On failure, return a -1
 *
 * Inputs:	1)   filehandle
 *		2)   point to structure to hold output data..
 *		3)   backup_flag.  If set, will back file up 1 record
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *
 * 98/08/20 DR	-added file backup capability.   If 3rd arg is true,
 *		routine will back up in the file 1 record so the last
 *		record read can be updated.
 *****************************************************************************/
int file_rewrite_output(int filehandle, OUTPUT_FILE_T *output_p, int backup_flag)
{
	int	ret_val = FILE_OK;
	int	bytes;

	/* If a user wants to rewrite the LAST record, read, back up fileptr */
	if(backup_flag == 1)
	{
		lseek(filehandle, (-1) * (int)sizeof(OUTPUT_FILE_T), SEEK_CUR);
	}


	if((bytes = write(filehandle, output_p, sizeof(OUTPUT_FILE_T)))==-1)
	{
		ret_val = FILE_WRITE_ERR;
	}
	return(ret_val);
}

/*****************************************************************************
 * file_get_output CSU
 *
 * Purpose:  	To get the requested record from output file.  
 *
 * Description:	Given the dbase (file) name and a flightline number:
 *		-open file
 *		-read in records until desired record is found.
 *		-return the output data to calling routine
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   flightline number
 *		3)   point to structure to hold output data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *
 * 98/10/28 DR	-changed routine to use other file_xxx_output routines in 
 *		it (it was directly opening the file and reading from
 *		the file using "c" routines... (simplified it this way)
 *
 *****************************************************************************/

int file_get_output(char *db_name, short flightline, int timestamp, OUTPUT_FILE_T *output_p)
{
	int	ret_val = FILE_OK;
	int	FILECODE;
	FILE 	*output_file;

	if((output_file = file_open_output(db_name, flightline))==NULL)
	{
		return(FILE_OPEN_ERR);
 
	}


	/* Go to the approximate location of the record in the file */
	file_guess_record_location(output_file, db_name, flightline,
		timestamp, sizeof(OUTPUT_FILE_T));

	for(;;)
	{
		/* Get a record */
	 	FILECODE = file_read_output(output_file, output_p);
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
		if(output_p->au.timestamp  == timestamp) 
		{
			/* FOUND IT!! */
			ret_val = 1;
			break;
		}
	}

	fclose(output_file);

	return(ret_val);
}


/*****************************************************************************
 * file_update_rec_output CSU
 *
 * Purpose:  	-will update the file for the passed record/timestamp  
 *		(if using in MP..., you will have to first load the
 *		record, change the required items and then rewrite
 *		it.. (flightline and timestamp info is in the record...)
 *
 * Description:	Given the dbase (file), flightline, timestamp and data:
 *		-open file
 *		-find record.
 *		-rewrite record
 *
 *		On failure, return a -1
 *
 * Inputs:	1)   Database name
 *		2)   point to structure to hold output data...
 *
 * Outputs:     None 
 * 
 * Modifications:
 * ==============
 * 98/07/07 DR	-wrote routine
 *
 *****************************************************************************/

int file_update_rec_output(char *db_name, OUTPUT_FILE_T *output_p)
{
	int	filehandle;
	int	ret_val = FILE_OK;

	filehandle = file_open_rewrite_output(db_name, output_p->au.flightline);
	file_setup_rewrite_output(db_name, filehandle, output_p->au.flightline, 
		output_p->au.timestamp);
	file_rewrite_output(filehandle, output_p, (int)0);
	close(filehandle);

	return(ret_val);
}

void sun_shift_early (OUTPUT_EARLY_T *early)
{
    union
    {
        OUTPUT_EARLY_T     early;
        SUN_OUTPUT_EARLY_T sun_early;
    } local;


    local.early = *early;

    *early = local.sun_early.early;
}


void dump_output (OUTPUT_FILE_T out)
{
    fprintf (stderr, "*****************************\n");
    fprintf (stderr, "timestamp - %d\n", out.au.timestamp);
    fprintf (stderr, "flightline - %d\n", out.au.flightline);
    fprintf (stderr, "haps_version - %d\n", out.au.haps_version);
    fprintf (stderr, "pulse_time - %f\n", out.au.pulse_time);
    fprintf (stderr, "latitude - %f\n", out.au.latitude);
    fprintf (stderr, "longitude - %f\n", out.au.longitude);
    fprintf (stderr, "sec_latitude - %f\n", out.au.sec_latitude);
    fprintf (stderr, "sec_longitude - %f\n", out.au.sec_longitude);
    fprintf (stderr, "tide_cor_depth - %f\n", out.au.tide_cor_depth);
    fprintf (stderr, "reported_depth - %f\n", out.au.reported_depth);
    fprintf (stderr, "altitude - %f\n", out.au.altitude);
    fprintf (stderr, "elevation - %f\n", out.au.elevation);
    fprintf (stderr, "otf_datu - %f\n", out.au.otf_datum);
    fprintf (stderr, "otf_elevation - %f\n", out.au.otf_elevation);
    fprintf (stderr, "otf_res_elev - %f\n", out.au.otf_res_elev);
    fprintf (stderr, "otf_sec_elev - %f\n", out.au.otf_sec_elev);
    fprintf (stderr, "otf_topo - %f\n", out.au.otf_topo);
    fprintf (stderr, "otf_water_level - %f\n", out.au.otf_water_level);
    fprintf (stderr, "result_depth - %f\n", out.au.result_depth);
    fprintf (stderr, "sec_depth - %f\n", out.au.sec_depth);
    fprintf (stderr, "topo - %f\n", out.au.topo);
    fprintf (stderr, "wave_height - %f\n", out.au.wave_height);
    fprintf (stderr, "depth_conf - %d\n", out.au.depth_conf);
    fprintf (stderr, "sec_depth_conf - %d\n", out.au.sec_depth_conf);
    fprintf (stderr, "ab_dep_conf - %d\n", out.au.ab_dep_conf);
    fprintf (stderr, "sec_ab_dep_conf - %d\n", out.au.sec_ab_dep_conf);
    fprintf (stderr, "otf_abd_conf - %d\n", out.au.otf_abd_conf);
    fprintf (stderr, "otf_sec_abd_conf - %d\n", out.au.otf_sec_abd_conf);
    fprintf (stderr, "position_conf - %d\n", out.au.position_conf);
    fprintf (stderr, "status - %d\n\n", out.au.status);

    fprintf (stderr, "bot_conf - %f\n", out.early.bot_conf);
    fprintf (stderr, "sec_bot_conf - %f\n", out.early.sec_bot_conf);
    fprintf (stderr, "nadir_angle - %f\n", out.early.nadir_angle);
    fprintf (stderr, "scanner_azimuth - %f\n", out.early.scanner_azimuth);
    fprintf (stderr, "sfc_fom_apd - %f\n", out.early.sfc_fom_apd);
    fprintf (stderr, "sfc_fom_ir - %f\n", out.early.sfc_fom_ir);
    fprintf (stderr, "sfc_fom_ram - %f\n", out.early.sfc_fom_ram);
    fprintf (stderr, "warnings - %d\n", out.early.warnings);
    fprintf (stderr, "warnings2 - %d\n", out.early.warnings2);
    fprintf (stderr, "bot_bin_first - %d\n", out.early.bot_bin_first);
    fprintf (stderr, "bot_bin_second - %d\n", out.early.bot_bin_second);
    fprintf (stderr, "bot_bin_used_pmt - %d\n", out.early.bot_bin_used_pmt);
    fprintf (stderr, "sec_bot_bin_used_pmt - %d\n", out.early.sec_bot_bin_used_pmt);
    fprintf (stderr, "bot_bin_used_apd - %d\n", out.early.bot_bin_used_apd);
    fprintf (stderr, "sec_bot_bin_used_apd - %d\n", out.early.sec_bot_bin_used_apd);
    fprintf (stderr, "bot_channel - %d\n", out.early.bot_channel);
    fprintf (stderr, "sec_bot_chan - %d\n", out.early.sec_bot_chan);
    fprintf (stderr, "sfc_bin_apd - %d\n", out.early.sfc_bin_apd);
    fprintf (stderr, "sfc_bin_ir - %d\n", out.early.sfc_bin_ir);
    fprintf (stderr, "sfc_bin_ram - %d\n", out.early.sfc_bin_ram);
    fprintf (stderr, "sfc_channel_used - %d\n", out.early.sfc_channel_used);
}
