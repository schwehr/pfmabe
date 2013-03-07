/*************************************************************************
 * ...
 *
 * Changes:
 * ---------
 * 99/08/24 DR	-This is used in the late part of the algorithm.  If all data
 *		in range requested from reprocess is done (written to files), 
 *		it will be set to 1 which will in turn signal the early part 
 *		of auto to set "last_pulse" so the program will terminate
 *		properly (last_pulse stats calculated). 
 *
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
***************************************************************************/

#ifndef PULSE_H
#define PULSE_H

typedef struct {
	unsigned int	first_pulse;
	unsigned int	last_pulse;
	unsigned int	done_range;	
} PULSE_GRD_T;

#endif
