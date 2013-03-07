/*****************************************************************************
 * shoals_defs.h 
 *
 * Purpose:	DEFINES used by a number of different routines.  This 
 *		file contains no global variables or X window items.. Just
 *		DEFINES so it can be used with the main MP exec or 
 *		other execs (such as UNLOAD...).
 *
 * Modifications: 
 * 97/05/26 DR	-created file (took many of the values from other .h files). 
 *	
 * NOTE:
 * In the future, more items should be moved to here to avoid having to 
 * duplicate defs:
 * 		ds/ds_defs (?)	SHOALS and FLASH
 *		ap/cnsts	SHOALS and HAWKEYE, Waveform lengths,
 *				Channel labels (PMT, APD...), MIN, MAX,
 *				TRUE, FALSE, ABS
		vq/const	Waveform lengths
 * 97/05/26 DR	-created file (took many of the values from other .h files). 
 *
 * 99/09/02 DR	-added "FOA" def and changed shoals to 123 (was 1) and 
 *		SHOALS_HAWKEYE to 163 (was 2)
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 *		**(NOTE: Unix version is in $SRC)
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef _SHOALS
#define _SHOALS

#define  TRUE 1
#define  FALSE 0

#endif

#ifndef SHOALS

#define  SHOALS 		123
#define  SHOALS_HAWKEYE 	163
#define  FOA		 	321
#endif


#ifdef  __cplusplus
}
#endif
