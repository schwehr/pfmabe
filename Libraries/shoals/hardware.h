#ifndef HARDWARE_H
#define HARDWARE_H
/*****************************************************************************
 *
 * File:	hardware.h
 *
 *
 * Purpose:	Hardware structures used by SHOALS 400 PFPS.
 *
 *		 
 * Revision History:   
 * 
 * 98/01/13 DR	-added operating_altitude, swathwidth, forwrd_spot_spcing and
 *		 laterl_spot_spcing for use with scanner correction
 *		 code in au_pos_angles.c
 *
 * 98/02/05 DR	-400 Hz Cleanup
 *		-removed old (97 and earlier) comments (to OLD_COMMENTS file)
 *
 * 98/02/09 DR	-400 Hz
 *		-moved all arrays (ones that were blobs) to HARDWARE_RA
 *		 structure instead of the HARDWARE structure.  Now will be
 *		 read/written in three chunks (also has temp HARDWARE2)...
 *		-removed the "GRD" from the struture names...
 *
 * 98/02/11 DR	-more reord.  Now 3 main structures and a separate one for
 *		 file definition.  This is to make it so all programs that 
 *		 use the hardware file  can use the same structures...
 *
 * 98/02/17 DR	-moved pmt_high_voltage to main struct
 *		-renamed couple structures to map to what I had in AUTO
 *		 (lot of changes in AUTO if I did not)
 *
 * 98/02/18 DR	-more of the same... Now file made up of a cobination of the
 *		 two separate structures defined below.  The main one
 *		 is made up of all the items used by AUTO except the larger
 *		 arrays.  The second part is "other" stuff, including the
 *		 arrays used by auto.  They are put into a separate structure
 *		 by auto.  [the reason for the split is mainly that the
 *		 debugger will not display all the items at once].
 *
 * 98/10/08 DR	-added apd_nd_filter_od to the OTHER part of hardware
 *		 (new value used in calc of bkgnd_ratio values).
 *
 * 98/10/14 DR	-above_surface_time ==>	above_sfc_time
 *		-above_sfc_bins_pmt/apd/ir/ram	==> above_sfc_bins[]
 *
 *
 * 98/10/27 DR	-made amp_noise[] an array of 5 now (instead of 4).  The 5th
 *		 element is for the RAMAN High Gain value (4th is low gain).
 *
 * 98/10/30 DR	-changed t_intvl_mtr1_const to tim1_const (it was ugly)
 *		-added code to get the 3 new Raman2 variables:
 *			ram2_gain
 *			ram2_t_shift
 *			ram2_amp_noise
 *
 * 98/12/08 DR	-scanner_pattern moved from "OTHER" to "GRD" structure.
 *
 * 99/02/15 DR	GG CC #4
 * 		-moved from HAPS to hardware:
 *			float	apri_dep_bias; (was _pmt and _apd)
 *			float	apri_topo_bias;
 *			float	nadir_angle_err_thresh
 *			float	pmt_bias_left;
 *			float	pmt_bias_right;
 *			float	surfing_conf_level;
 *			short	asymmetry_bins[2];
 *			short	bottom_horizontal_bias;
 *
 * 99/02/17 DR	GG CC #9
 *		-removed :
 * 			float	ram2_amp_noise;
 *			float	ram2_gain;
 * 			float	ram2_t_shift;
 * 		-Also backed out of other ram2 code as dual gain amplifier
 *		has been physically removed from system.

 * 99/02/17 DR	GG CC #9b
 *		-removed :
 * 			short	l_w_discrim_thres;
 *
 * 99/02/17 DR/GC
 * 		-added:
 *			float	sc_frm_off_ang_r;
 * 			float	sc_frm_off_ang_p;
 *			float	sc_frm_off_ang_h
 *		-also added a number of "extra_xxxx" variables for
 *		 future use...
 *
 * 99/06/05 DR	-added "rcvr_off_ang_y" and "rcvr_off_ang_z" to the
 *		float section (used up 2 extra_float vars which reduced
 * 		that array from 4 to 2 spares).  These new vars are
 *		for GC's calibration algorithm...
 *
 * 99/06/30 DR	GG Email 99/06/29 
 *		-added "roll_slope_400" and "pitch_slope_400" to the
 *		float section (used up last 2 extra_float vars).  These
 *		new vars are found on FP tape and are used in 
 *		position algorithm.  
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 ****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#include "cnsts.h"

struct HARDWARE_GRD
{
	short	flightline;
	short	system;

	float	gb_version;

	double	extra_double[2];	

	float	above_sfc_time[4];
	float	amp_noise[5];		/* 0, 4 not used....*/
	float	antenna_offset_x;
	float	antenna_offset_y;
	float	antenna_offset_z;
	float	apri_depth_bias;
	float	apri_topo_bias;
	float	bin_width;
	float	bkgnd_ratio[4];		/* [0] not used */
	float	cage_angle;
	float	delta_theta_x;
	float	delta_theta_y;
	float	delta_t_soft[4];	
	float	delta_t_hard[4];
	float	detector_noise;
	float	digitizer_voltage_range;
	float	eff_pulse_risetime[4];
	float	forwrd_spot_spcing;
	float	laterl_spot_spcing;
	float	nadir_angle_err_thresh;
	float	nom_nadir_angle;
	float	pmt_bias_left;
	float	pmt_bias_right;
	float	preamp_gain[4];
 	float	raman_total_gain;
	float 	sc_frm_off_ang_r; 
	float	sc_frm_off_ang_p; 
	float	sc_frm_off_ang_h; 
	float	surfing_conf_level;
	float	rcvr_off_ang_y;
	float	rcvr_off_ang_z;
	float	roll_slope_400;
	float	pitch_slope_400;
	/*float	extra_float[10];	ADD IN IF MORE ARE EVER NEEDED */


	short	ac_zero_offset[4];
	short	asymmetry_bins[2];
	short	bandwidth[4];
	short	bottom_horizontal_bias;
	short	data_length[4];
	short	detector_gain[4];	/* PMT is set in AU from 6 */
	short	max_angle;
	short	operating_altitude;
	short 	pmt_high_voltage;
	short	pmt_turnon_delay;
	short	rep_rate;
	short	return_priority;
	short	scanner_pattern;
	short	swathwidth;
	float	extra_short[4];	
};
typedef struct HARDWARE_GRD HARDWARE_GRD_T;



struct HARD_OTHER
{
	float	pmt_total_gain_ras[6][GAIN_RA_NUM+2];
   	float	apd_total_gain_ra[GAIN_RA_NUM+2];
   	float	ir_total_gain_ra[GAIN_RA_NUM+2];
	float	log_amp_delay_table[4][GAIN_RA_NUM+2];
	float	amp_noise_pmt[6];
	float	delta_t_pmt_hv_ra[6];
	short	receiver_turnon_time_ra[6][64];

	float apd_nd_filter_od;
	float ir_nd_filter_od;
	float beam_diverg;
	float bkgnd_ratio_apd_c;
	float bkgnd_ratio_ir_c;
	float bkgnd_ratio_ram_c;
	float digitizer_offset;
	float digitizr_rnge_gate;
	float field_of_view;
	float frequency;
	float min_mrad_setting;
	float max_mrad_setting;
	float pmt_gate_depth;
	float rcvr_target_t;
	float scanner_mode;  
	float scan_limit_posi_x;
	float scan_limit_nega_x;
	float scan_limit_posi_y;
	float scan_limit_nega_y;
	float sfc_bias_apd;
	float sfc_bias_ir;
	float sfc_bias_ram;
	float swath_alti_ratio;
	float tim1_const;

	int   lamp_life;

	short above_sfc_bins[4];
	short alti_rnge_divg_prd;
	short centre_position_x ;
	short centre_position_y;
	short config_data;
	short detector_gain_pmt[6];
	short digitizer_delay_t;
	short digitizer_const_t;
	short diagnostic_flag;
	short flight_speed;
	short gyrostbiliz_on_off;
	short green_discrim_th;
	short inhibit_status;
	short ir_discrim_thres;
	short min_encod_setting; 
	short max_encod_setting;
	short mrad_spat_block_0;
	short mrad_spat_block_3;
	short mrad_spat_block_6;
	short mrad_spat_block_10;
	short mrad_spat_block_15;
	short optical_atten;
	short opticl_attn_of_1;
	short opticl_attn_of_4;
	short opticl_attn_of_16;
	short opticl_attn_of_256;
	short pmt_gain_values[6];
	short pmt_gate_on_off;
	short pmt_mode_on_off;
	short pmt_serial_number;
	short pod_door_status;
	short range_gate_offset;
	short ram_discrim_thres;
	short sensor_subsystem;
	short size_x;
	short size_y;
	short slnt_rnge_avg_valu;
	short spatial_block;
	short temp_limit_upper;
	short temp_limit_lower;
	short t0_source;
	short trigger_rate;
	short trigger_delay;
	short trigger_source;
};
typedef struct HARD_OTHER HARD_OTHER_T;

/**************
 * This is the structure used to define a record in the hardware file. 
 **************/
struct HARD_FILE
{
	HARDWARE_GRD_T	au;
	HARD_OTHER_T	other;
};
typedef struct HARD_FILE HARD_FILE_T;

int file_get_hardware(char *db_name, short flightline, HARD_FILE_T *hard_file_p);
int file_put_hardware(char *db_name, short flightline, HARD_FILE_T *hard_file_p);
int file_update_hardware(char *db_name, short flightline, HARD_FILE_T *hard_file_p);
FILE *file_open_hardware(char *db_name);
int	file_read_hardware(FILE *hard_file ,HARD_FILE_T *hard_file_p);


#ifdef  __cplusplus
}
#endif


#endif /*hardware_h*/
