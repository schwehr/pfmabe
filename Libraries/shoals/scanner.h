#ifndef SCANNER_H
#define SCANNER_H
/*****************************************************************************
 *
 * File:	scanner.h
 *
 *
 * Purpose:	Scanner stuff used by SHOALS 400 PFPS for the scanner
 *		adjustment code (in DS)
 *
 * Revision History:   
 * 98/04/21 DR	-wrote it
 *
 * 98/11/05 DR	-boosted values of :
 *			NUMBER_INPUTS_BUFFERED 		 20  40
 * 			MAX_SHOTS_REQUIRED_FOR_CURVE_FIT 17  27
 *			NUM_SHOTS_IN_SCAN_CHECK_RA	  9  9
 * 		-added MAX_SHOTS_REQUIRED_FOR_CURVE_FIT
 * 		-added SCANNER_COR_TIME_BEFORE_TEOS
 * 		-added ATTRIB_ARRAY_SIZE
 * 		-added ATTRIB_ARRAY_ZERO_LOCATION_OFFSET
 *  
 * 98/11/26 DR	-added ATTRIB_UNRECOGNIZED_SCANNER_POINT.  This will
 * 		be assigned to all shots in DataStrip that do not have
 *		another attribute (usually only bad data or partial scanns).
 *		Shots with this will not be fixed...
 * 
 * 98/12/03 DR	-new def of SCANNER_END_BAD_OFFSET (3000 series)
 *		-upped NUMBER_INPUTS_BUFFERED from 40 to 61
 * 		-removed SHOTS_TO_FIX_BEFORE_TURN
 * 		-upped MAX_SHOTS_REQUIRED_FOR_CURVE_FIT from 27 to 60
 *		-changed (previously unused) SCANNER_COR_TIME_BEFORE_TEOS
 * 		 from 25 to 200.
 * 		-added SCANNER_END_BAD
 * 
 * 98/12/08 DR	-added new type SCANNER_T for use in AUTO/POS algs.
 * 
 * 98/12/12 DR	-now support left/right sides....
 * 
 * 99/01/11 DR	-max scanner pattern files added.... (20)
 * 
 * 99/04/15 DR	-reduced "SCANNER_COR_TIME_BEFORE_TEOS" from 200 to 180
 *		[GC said max currently used was about 140.  Having too big 
 *		 could cause problems (as seen in 200 Hz) where more than 
 *		 one complete scan was in buffer]
 * 		-added new SCANNER_COR_TIME_BEFORE_TEOS_200 for 200 Hz
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 ****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif 

#define ATTRIB_ARRAY_SIZE			401 
#define ATTRIB_ARRAY_ZERO_LOCATION_OFFSET	200
#define	SCANNER_END_UNACCEPTABLE	-2
#define	SCANNER_NOT_END 		-1
#define	SCANNER_END_OK 			0
#define	SCANNER_END_PINNED 		1	
#define	SCANNER_END_MULTI_INTERP 	2
#define	SCANNER_END_BAD		 	3
#define	SCANNER_END_NOT_FOUND	 	4	/* from UNRECOGNIZED (9999) */

#define	SCANNER_END_PINNED_OFFSET		1000
#define	SCANNER_END_MULTI_INTERP_OFFSET		2000
#define	SCANNER_END_BAD_OFFSET			3000
#define	ATTRIB_UNRECOGNIZED_SCANNER_POINT	9999

#define	NUMBER_INPUTS_BUFFERED 			61


/* 
 * Number of buffered shots required for finding EOS..
 */
#define	NUM_SHOTS_IN_SCAN_CHECK_RA		9

/*
 * BUffered shots that are passed into "find_teos".. It should be 
 * (NUM_SHOTS_IN_SCAN_CHECK_RA * 2) - 1 + NUM_SHOTS_BEFORE_CURVE_BUFFER
 * The first NUM_SHOTS_BEFORE_CURVE_BUFFER will be ignored in "find_teos"
 * but are buffered so we can correct back behind the end of the scan...
 */
#define	MAX_SHOTS_REQUIRED_FOR_CURVE_FIT	60

/* Amount of time we will back up before teos to apply scanner attribute */
#define SCANNER_COR_TIME_BEFORE_TEOS		180	/* In 1/2 Milliseconds*/
#define SCANNER_COR_TIME_BEFORE_TEOS_200	200	/* 200 Hz verison.. */

struct SCANNER
{
	short	fix_att_range_start;
	short	fix_att_range_stop;
	double	fix_angle_left[ATTRIB_ARRAY_SIZE];
	double	fix_angle_right[ATTRIB_ARRAY_SIZE];
};
typedef struct SCANNER SCANNER_T;


#define MAX_PATTERN_FILES	20


#ifdef  __cplusplus
}
#endif

#endif /*scanner_h*/
