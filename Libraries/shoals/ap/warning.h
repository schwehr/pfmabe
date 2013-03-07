/*****************************************************************************
 * warnings.h 	Header
 *
 * Purpose:  	This header file hold the structure for the warnings
 * 		used throughout Automated Processing
 * 
 * 		NOTE:	nadir_angle_flag is not included here as it is 
 *		 	considered an input to auto so it is put into 
 *			the inputs structure when it is set.
 *
 * Revision History:
 * =================
 * ...
 *              
 * 98/09/29 DR	-renamed land_water_flag to land_flag...
 *              
 * 98/11/03 DR	-added interleave_flag and  land_flag 
 *              
 * 98/11/11 DR	-added data_offset_zero_flag
 *              
 * 98/11/25 DR	-added interp_edge_flag, pinned_edge_flag
 *		and scanner_y_pinned_flag
 * 
 * 98/12/10 DR	-add scanner_edge_bad_flag  and scanner_edge_not_found_flag
 *
 * 99/01/21 DR	-changed no_ir_sfc_flag to no_hard_or_soft_sfc_flag
 *		-added 2 new flags stretched_ir_flag and  
 *		 stretched_raman_flag  
 *
 * 99/06/11 DR	-added gps_flag and ins_flag (killers)
 * 
 * 99/11/16 DR	-added waveheight_flag 
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 *****************************************************************************/
#include "warns.h"	/* Was ap/au_flags.h, now warns.h */

typedef struct {
 	int 	timestamp;
	short	land_flag;
	short	land_and_raman_flag;
	short	no_land_no_raman_flag;
	short	early_hdwr_sfc_flag;
	short	late_hdwr_sfc_flag;
	short	noisy_height_flag;
	short	sfc_tracker_buffer_flag;
	short	no_sfc_data_flag;
	short	glint_error_flag;
	short	stretched_sfc_flag;
	short	soft_sfc_dif_flag_ra[4];	/* [0] not used */
	short	hard_soft_sfc_dif_flag;
	short	lesser_pulse_selected_flag;
	short	shoal_peak_flag;
	short	peak_fifty_error_flag;
	short	stretched_bot_flag;
	short	green_green_depth_dif_flag;
	short	accel_bias_change_flag;
	short	st_surfing_flag;
	short	st_jerk_flag;
	short	interpolated_flag;
	short	saturated_sfc_flag;
	short	no_hard_or_soft_sfc_flag;
	short	no_tide_zone_flag;
	short	saturated_apd_bot_flag;
	short	otf_alt_flag;
	short	no_otf_datum_zone_flag;
	short	raman_bottom_flag;

	short	sec_raman_bottom_flag;
	short	dlessone;
	short	d2lessone;
	short	sec_peak_fifty_error_flag;
	short	sec_sat_apd_bot_flag;
	short	sec_stretched_bot_flag;
	short	sec_shoal_peak_flag;
	short	interleave_flag;
 	short	tim1_flag;
 	short	data_offset_zero_flag;
 	short	interp_edge_flag;
 	short	pinned_edge_flag;
 	short	scanner_y_pinned_flag;
 	short	scanner_edge_bad_flag;
 	short	scanner_edge_not_found_flag;
	short	stretched_ir_flag;
	short	stretched_raman_flag;
	short	ins_flag;
	short	gps_flag;
	short	waveheight_flag;
} WARNING_GRD_T;

