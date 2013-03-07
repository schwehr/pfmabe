/*****************************************************************************
 * cnsts.h 	Header
 *
 * Purpose:  	This header file hold some constants and macros for 
 *		Automated Processing.
 *              
 *
 * Revision History:
 * 93		DR	Created
 *
 * 95/02/09 DR	-added constants HAWKEYE (163) and SHOALS(123) for use
 *		 in calculation of hardware parameter "system".  These 
 *		 parameters are also used in DS (def'ed in ds/ds_def.h).
 *
 * 96/12/09 DR	-added MIN and MAX macros to header
 *
 * 98/02/04 DR	-added TRUE and FALSE
 *
 * 98/07/06 DR	-added FILE_FIRST_RECORD
 *
 * 98/07/06 DR	-added D_TO_R, R_TO_D and PI (were defed in several 
 *		 different position and auto routines..)
 * 
 * 99/02/15 DR	-moved FILE_FIRST_RECORD to file.h
 * 
 * 99/12/03 DR	-Brought up to date with 2.30
 *
 ***************************************************************************/

#ifndef HAWKEYE
#define HAWKEYE	163
#endif

#ifndef SHOALS
#define SHOALS	123
#endif

#ifndef FOA
#define FOA	321
#endif

#define AU_PROCESS_LINES 1
#define AU_PROCESS_TIMES 2
#define AU_MAX_LINES 	10
#define AU_DBA_OK 	0
#define AU_DBA_NOT_OK 	1

#define	PMT_LEN		330
#define	APD_LEN		148
#define	IR_LEN		41
#define	RAMAN_LEN	41

#define	MAX_BINS	330
#define	GAIN_RA_NUM	1023

#define	PMT		0
#define	APD		1
#define	IR		2
#define	RAMAN		3

#define	NUM_STRIPES		10 
#define	MAX_BUFFER_SIZE		12000 
#define	MAX_SFC_BOX_SIZE 	12000  

#ifndef FALSE
#define	FALSE		0
#endif

#ifndef TRUE
#define	TRUE		1
#endif

#define	NPOINT		2048

#ifndef ABS
#define	ABS(x)		(((x) < 0)?-(x): (x))
#endif

#ifndef MAX
#define MAX(x,y) 	( ((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) 	( ((x) < (y)) ? (x) : (y))
#endif

#ifndef PI
#define PI 		3.141592654
#endif


#define D_TO_R(x)	((x)/180.*(PI)) /* Translate DEGREES to RADIANS */

#define R_TO_D(x) 	((x)*180./(PI))	/* Translate RADIANS to DEGREES */

#define	ONE_SECOND 2000

