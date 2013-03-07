/*****************************************************************************
 *
 * File:	au_hardware.h
 *
 *
 * Purpose:	-structure used by auto for hardware arrays.
 *		-includes "hardware.h" as it has the general structures
 *		 used for hardware including the file definition.  
 *		 
 * Revision History:   
 * =================
 * 98/02/09 DR	-400 Hz
 *		-new file... ....
 *****************************************************************************/

/**
 ** Use in AU ONLY... It will be used to store arrays 
 ** it needs from structure (HARD_OTHER_T) in hardware.h
 **/

#ifndef AU_HARDWARE_H
#define AU_HARDWARE_H

struct HARDWARE_RA_T
{
	float	pmt_total_gain_ra[GAIN_RA_NUM+2];
     	float	apd_total_gain_ra[GAIN_RA_NUM+2];
     	float	ir_total_gain_ra[GAIN_RA_NUM+2];
	float	log_amp_delay_table[4][GAIN_RA_NUM+2];
};

#endif
