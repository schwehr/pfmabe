/********************************************************************
 *
 * Module Name : SNPUTIL
 *
 * Author/Date : J. S. Byrne / 21 Jul 1994
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who	when	  what
 * ---	----	  ----
 * jsb	7-21-94   Added MakePath and DetermineAddressing functions
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

#ifndef __SNPUTIL__
#define __SNPUTIL__

#include <stdio.h>
#include "saic.h"
#include "parse.h"
#include "snptime.h"
#include "npstring.h"

char * OPTLK GetEnvVal(char *key, FILE *fd);

/*
 * TranslatePosition(TRANSLATEPOSITION *dp) -
 *	is used to interpolate or extrapolate a new position at a given time,
 *	given also the position at surrounding times.
 *     (note - extrapolation usage should be limited to a short delta
 *	       in time. The maximum extrapolate tolerlance should be less
 *	       than 2 seconds to preserve accuracy.)
 * Inputs :
 *  date_time_posn  *dtp_lo	:Pointer to lower bound date_time_posn structure
 *  date_time_posn  *dtp_hi	:Pointer to upper bound date_time_posn structure
 *  date_time_posn  *dtp_target :Pointer to target date_time_posn structure
 *  double  max_extrapolate	:The max extrapolate tolerlence in seconds
 * Returns :
 *	0  = Position successful interpolated
 *	1  = Position successful extrapolated
 *	-1 = Indicates error.
 */

int OPTLK TranslatePosition( date_time_posn *dtp_lo,
		       date_time_posn *dtp_hi,
		       date_time_posn *dtp_target,
		       double max_extrapolate );

int OPTLK MakePath (char *path);
/*
 * Description : This function creates subdirectories given a fully
 *     qualified path.	The path may be up to MAKE_PATH_MAX_SUBDIRECTORIES
 *     deep, and the subdirectory names may be up to
 *     MAKE_PATH_MAX_DIR_NAME_LENGTH long.
 *
 * Inputs :
 *    char *path = a pointer to a FULLY qualified file specification
 *
 * Returns : This function returns zero if successful, or non-zero on
 *    failure.
 *
 * Error Conditions :
 *    MAKE_PATH_MAX_LENGTH_EXCEEDED
 *    MAKE_PATH_TOO_MANY_SUBDIRS
 *    MAKE_PATH_MEMORY_ALLOCATION_FAILED
 *    MAKE_PATH_SUBDIR_CREATION_FAILED
 *
 */
/* Put a ceiling on the number of subdirectories which may be created */
#define MAKE_PATH_MAX_SUBDIRECTORIES	16

/* Put a ceiling on the length of the subdirectory names */
#define MAKE_PATH_MAX_DIR_NAME_LENGTH   64	

/* Define return error codes for MakePath */
#define MAKE_PATH_TOO_MANY_SUBDIRS	     -1
#define MAKE_PATH_MEMORY_ALLOCATION_FAILED   -2
#define MAKE_PATH_SUBDIR_CREATION_FAILED     -3
#define MAKE_PATH_MAX_LENGTH_EXCEEDED	     -4


int DetermineAddressing(void);
/*
 * Description : This function determines the integer addressing scheme
 *  being used by the host machine.
 *
 * Inputs : none
 *
 * Returns :
 *    This function returns HOST_BIG_ENDIAN if the host machine uses
 *	  big endian addressing
 *    This function returns HOST_LITTLE_ENDIAN if the host machine uses
 *	  little endian addressing
 *    This function returns HOST_UNKOWN_ADDRESSING if the addressing
 *	  integer scheme cannot be determined.
 *
 * Error Conditions : none
 */
#define HOST_BIG_ENDIAN 	 1
#define HOST_LITTLE_ENDIAN	 2
#define HOST_UNKOWN_ADDRESSING	 3

#endif
