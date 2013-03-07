#ifndef HAPS_H
#define HAPS_H
/*****************************************************************************
 *
 * File:	haps.h
 *
 *
 * Purpose:	structure used for holding the HAPS values (and define
 *		the haps file structure.
 *
 *		Also defined here is a CONTROL_T structure which 
 *		is an internal AUTO structure for telling software 
 *		about the line it is processing... [likely will move]
 *
 *		 
 * Revision History:   
 * 98/03/16 DR	-400 Hz 
 *		-added flightline to CONTROL_T structure
 *		-added haps_version to CONTROL_T structure
 *		-moved otf_switch to CONTROL_T
 *		-added db_name to CONTROL_T
 *
 * 98/08/17 DR	-changed scanad_flag to scanad_sw (PEL/GG)
 *		-changed tilt_bias... to pmt_bias... (PEL/GG)
 *
 * 98/10/07 DR	-added apri_topo_bias
 * 
 * 98/11/11 DR	-added bad_tim1_kill_sw
 * 
 * 98/11/25 DR	-added interleave_sw
 * 
 * 99/02/11 DR	-added gain_ramp_fom_thresh (was using sfc_fom_th[0])
 * 
 * 99/02/11 DR	GG CC#3
 *		-removed 	float	green_green_depth_thresh;
 *		-removed 	float	noise_normalizer;
 *		-removed 	short	max_predict_time;
 *		-removed	short	num_stripes;	
 * 
 * 99/02/15 DR	GG CC #6
 * 		-added 		short	pmt_100_gate_shift;
 * 
 * 99/02/15 DR	GG CC #4
 * 		-moved from HAPS to hardware:
 *			float	apri_dep_bias_pmt;
 *			float	apri_dep_bias_apd;
 *			float	apri_topo_bias;
 *			float	nadir_angle_err_thresh;
 *			float	pmt_bias_left;
 *			float	pmt_bias_right;
 *			float	surfing_conf_level;
 *			short	asymmetry_bins[2];
 *			short	bottom_horizontal_bias;	
 *
 * 99/02/15 DR	-changed pmt_100_gate_shift to a float...
 ****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct CONTROL {
	int	proc_start_time;
	int	proc_stop_time;
	int	output_start_time;
	int	output_stop_time;
	int	survey_date;
	short	process_type;  
	short	diagnostics;  
	short	results;  
	short	auto_output_type;
	short	flightline;
	short	haps_version;
	short	otf_switch;
	char	db_name[16];
};

typedef struct CONTROL CONTROL_T;

struct HAPS_GRD
{
	short	flightline;
	short	version;

	float	apriori_k;
	float	bot_fom_thresh[2];
	float	early_sfc_tracker_thresh;
	float	gain_ramp_fom_thresh;
	float	height_std_dev_thresh;
	float	index_of_refraction;
	float	late_sfc_tracker_thresh;
	float	max_rms_wave_ht;
	float	min_water_fraction;
	float	pmt_100_gate_shift;
	float	sfc_fom_thresh[4];	/*  [0] not used */
	float	extra_floats[4];

	short	apriori_depth_limit;
	short	bad_data_pulse_limit;
	short	bot_min_run_offset[2];
	short	bot_risetime_thresh[2];
	short	bottom_logic_sw;
	short	interleave_sw;
	short	ir_land_thresh;
	short	land_flag_sw;	
	short	max_bot_risetime_sw;
	short	otf_alt_conf_thresh;
	short	safety_margin[2];
	short	scanad_sw;
	short	sfc_dif_thresh;	
	short	sfc_min_run_offset[4];	/*  [0] not used */
	short	sfc_tracker_boxcar_size;
	short	sfc_risetime_thresh[4];	/*  [0] not used */
	short	soft_sfc_chan_code;
	short	bad_tim1_kill_sw;
	short	use_hrd_sfc_switch;
	short	wave_cor_sw;	
	short	window_size;
	short	extra_short[4];

};
typedef struct HAPS_GRD HAPS_GRD_T;


int file_get_haps( char *db_name, short flightline, HAPS_GRD_T *haps_p, short version );
int file_put_haps( char *db_name, HAPS_GRD_T *haps_p);
int file_get_haps_version( char *db_name, short flightline);
FILE *file_open_haps( char *db_name );
int	file_read_haps( FILE *haps_file, HAPS_GRD_T *haps_p );


#ifdef  __cplusplus
}
#endif

#endif /*HAPS_H*/
