/****************************************************************** 
 * ring.h	-This header file holds all of the structures that exist  
 * 		 in the ring buffer and the gps buffer. 
 *
 * Modifications:	
 * 94/12/14 dr	-added scanner_azimuth and warnings to ring buffer (outputs). 
 *
 * 95/01/13 DR	-added est_bot_det_prob and sfc_channel_used to outputs
 *		 part of ring buffer 
 *
 * 95/02/02 DR	-added bot_risetime_bins[0] & [1] to outputs part of 
 * 		 ring buffer 
 *
 * 95/02/17 DR	-changed "avg_slant_range" to "cor1_slnt_rnge_t" in the 
 *		 POSITION_INPUTS_T part of the ring buffer. GG9503-2
 *
 * 95/02/17 DR	-added "avg_slant_range" back to the POSITION_INPUTS_T part 
 *		 of the ring buffer. Fix to GG9503-2/1
 *		-removed "sfc_channel" from the pos_in part of the ring buffer
 *		 as it is not required by the position algorithm.
 *
 * 96/02/25 DR	-made changes for FIRST/LAST pulse logic changes:
 *		-REMOVED bot_risetime_bins from ring buffer [now a early
 *		result.
 *		-REMOVED bottom_bin[], second_bot_bin[2], bottom_fom[] and
 *		second_bot_fom[] from ring buffer
 *		-ADDED bottom_bin_used[], bottom_fom_used[], sec_bot_bin_used,
 *		sec_bot_fom_used[2] to ring buffer.
 *		GG9607-98
 *  
 * 96/05-07 DR	OTF Changes
 *		-added added altitude to gps ring buffer (only used by otf)
 *		-change latitude and longitude from float to double
 * 
 * 97/10/23 DR	GG9710-2c
 *		-removed from OUTPUTS_T:
 *			float	bottom_fom_used[2];
 *			float	sec_bot_fom_used[2];	
 *		-replaced with:
 *			bottom_bin_first
 *			bottom_bin_second
 *			bot_conf
 *			second_bot_conf
 * 
 * 
 * 97/10/27 DR	-added warnings2 to outputs section.
 *		-removed "est_bot_det_prob" from ring
 * 
 * 98/03/24 DR	-added "receive_time" to GPS_RING_T
 * 
 * 98/07/07 DR	-removed GPS stuff (use gps.h)
 *		-removed one layer from ring (still more could be done)
 *		 to make names shorter and whole thing a bit less
 *		 complicated (ugly).
 *
 * 98/09/29 DR	-removed polarization_land_flag (now use land_flag warning)
 *
 * 98/12/12 DR	-added scanner_fix_attrib to POSN_EARLY_OUTPUTS_T.  It
 *		 will contain the value that is set in POS angle...
 *****************************************************************/

/****************************************************************** 
 *		R I N G 	B U F F E R 	  
 *****************************************************************/

/************************************ 
 * Inputs to the position algorithm   
 * that are not in the HAPs and the   
 * Hardware table
 ************************************/
#ifndef RING_H
#define RING_H

typedef struct
{
	double	roll;			/* from ins 	*/
	double	pitch;			/* from ins	*/
	double	heading;		/* from ins	*/
	float	scanner_x_angle;	/* from inputs table */
	float	scanner_y_angle;	/* from inputs table */
	float	cor1_slnt_rnge_t;	/* from alg (early result) */
	float	avg_slant_range;	/* from inputs */
} POSITION_INPUTS_T;

/************************************ 
 * Outputs from the early position    
 * call that are needed later (or in  
 * the outputs			      
 ************************************/
typedef struct
{
	double	nadir_angle;		/* from position (output)  (w.r) */
	double	tanqx;			/* from position (internal) */
	double	tanqy;			/* from position (internal) */
	double	tanq;			/* from position (internal) */
	float	scanner_azimuth;	/* outputs ..*/
	short	scanner_fix_attrib;	/* internal only.. Calced from input.*/
	short	fill;			/* outputs ..*/
} POSN_EARLY_OUTPUTS_T;

/************************************ 
 * Outputs from the early part of the 
 * depth algorithm.		      
 ************************************/
typedef struct
{
	float	surface_fom[4];		/* outputs (use 1-3) (w.o)*/
	float	tide;			/* outputs NOT USED YET....*/
	float	bot_conf;
	float	second_bot_conf;
	int	warnings;		/* warnings....*/
	int	warnings2;		/* more warnings....*/
	short	bot_channel;		/* outputs (w.o)*/
	short	surface_bin[4];		/* outputs (use 1-3) (w.o)*/
	short	second_bot_chan;	/* outputs (w.o)*/
	short	sfc_channel_used;	/* PIII */
	short	bottom_bin_first;
	short	bottom_bin_second;
	short	bottom_bin_used[2];	/* NEW - first/last pulse logic */
	short	sec_bot_bin_used[2];	/* NEW - first/last pulse logic */
} OUTPUTS_T;

/********************************************************
 ** Define a stucture that will hold all the ring data
 *******************************************************/ 
typedef struct
{
	double			pulse_time;
	int			timestamp;
	POSITION_INPUTS_T	pos_in;
	POSN_EARLY_OUTPUTS_T	pos_eout;
	OUTPUTS_T		outputs;
}  RING_BUF_T;

/********************************* 
 * This is the data that will be   
 * written to the database from    
 * the ring buffer 		   
 **********************************/
typedef struct
{
	float			nadir_angle;
	float			scanner_azimuth;
	OUTPUTS_T		outputs;
} WRITE_RING_BUF_T;

/****************************************************************** 
 *		P O S I T I O N  	B U F F E R 		
 ******************************************************************/ 

#endif

