/*****************************************************************************
 * gconst.h 
 *
 * Purpose:	Constants used thru the system 
 *		region window.
 *
 * Modifications: 
 * 96/08/30 DR	-added SOFT_LAND and HARD_LAND.   
 * 96/10/16 DR	-removed SOFT_LAND and HARD_LAND (now all done using
 *		 abdc (otfabdc) values. 
 *
 * 97/05/26 DR	-moved TRUE and FALSE defs to shoals_defs.h.  This was
 *		 done so that a number of generic definitions with NO
 *		 X window ties could be put in one place (triggered
 *		 by HAWKEYE release that needed defs of SHOALS
 *		 and SHOALS_HAWKEYE (previously in shoals/const.h)... 
 *
 * 97/10/21 DR	-added SHOALS and SHOALS_HAWKEYE
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#include "shoals_defs.h"

#ifndef _GCONSTS
#define _GCONSTS

#define SURV_WIDTH	400
#define SURV_HEIGHT	240

#define NONE -998
#define LAND 0
 
#define XYZ     1
#define RESULTS 2
#define DIAGNOSTICS 4

extern int CURRENT_FRAME_KEY,
	   LIMITS_FRAME_KEY;

#endif

#ifndef SHOALS
#define  SHOALS 		1
#define  SHOALS_HAWKEYE 	2
#endif

#ifdef  __cplusplus
}
#endif
