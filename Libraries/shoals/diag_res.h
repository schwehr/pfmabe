#ifndef DIAG_RES_H
#define DIAG_RES_H
/*****************************************************************************
 * diag_res.h 	Header
 *
 * Purpose:  	This header file hold the definition of all the diagnostics
 *	 	and results used in the Automated Processing algorithm.
 *		As some variables are in a per pulse and some per 
 *		fightline basis (and some are diagnostics and some are 
 *		results), so it is split up into 5 different structures:
 *			DIAG_RES_LPULSE_T	-late diags and results (tstar)
 *			RES_PULSE_T
 *			DIAG_PULSE_T
 *			RES_FLIGHT_T
 *			DIAG_FLIGHT_T
 *
 * Revision History:
 * =================
 * 98/10/05 DR	-wrote header...
 * 
 * 99/02/17 DR	GG CC #9
 * 		-removed:
 * 			float	noise_constant_ram2;
 *			float	ram2_pe_per_count;
 * 		-The dual gain amplifier has been physically removed
 *		 from the system.
 *
 * 99/03/03 DR	GG CC #12
 * 		-removed ir_height from RES_PULSE_T
 *
 * 99/03/10 DR	GG CC #5
 * 		-changed land_prob to land_percent
 *		-changed hdwr_sfc_percent[] to hd_sfc_pc_used[]
 *		-changed hdwr_sfc_nongreen_prob to hd_sfc_prb_nongrn
 *		-changed hdwr_sfc_overall_prob to hd_sfc_prb_overall
 * 
 * 99/03/10 DR	GG CC #8
 *		-added to results_flight:
 * 			float	sft_sfc_pc_usd_apd;
 *			float	sft_sfc_pc_usd_ir;
 *			float	sft_sfc_pc_usd_ram;
 * 
 * 99/05/20 DR	-added roll/pitch/heading/vert_accel (all followed with 
 *		 _interp) to the res_pulse file for the 400 Hz interpolated
 *		 (or 200) INS data.  The data is interpolated by auto
 *		 from the raw 32/64 Hz data.
 * 
 * 99/06/06 DR	-changed the "fill" (was int) value in the res and diag flight
 *		 structures to "gb_version" (now float) which will hold the 
 *		 version that the line was processed with. 
 * 
 * 99/06/09 DR	-moved acc_bias_dif_thres from results flight to diagnostics
 *		flight (gg)
 *
 * 99/08/05 DR	GG CC #8b
 * 		-changed names in GG CC #8 to array (0 not used)
 *			float	sft_sfc_pc_usd[4]

 * 99/08/27 DR	GG Email 990827
 * 		-added short late_shift to the diagnostics_pulse table.
 *		(also added a extra short "fill2").
 *
 * 99/11/23 DR	-brought up to date with 2.30 unix version.
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#include "cnsts.h"

struct DIAG_RES_LPULSE {
	int	timestamp;
	int	fill;
	float	accel_bias;			/* res */
	float	accel_bias_dif;			/* res */
	float	double_integral;		/* diag */
	float	otf_elip_elev;			/* res */
	float	otf_sec_elip_elev;		/* res */
	float	otf_cor_altitude;		/* res */
	float	prop_depth_bias_1;		/* res */
	float	prop_depth_bias_2;		/* res */
	float	tilt_bias;
	float	ym;

 	double	win_wave_ht_m;
	double	win_wave_ht_sd;

	union {
     		double 	ra[NUM_STRIPES+1];
     		char 	ch[(NUM_STRIPES+1)*sizeof(double)];
     	} win_wave_ht_ms; 
	union {
     		double 	ra[NUM_STRIPES+1];
     		char 	ch[(NUM_STRIPES+1)*sizeof(double)];
     	} win_wave_ht_sds; 

};
typedef struct DIAG_RES_LPULSE DIAG_RES_LPULSE_T;

struct RES_PULSE {
	int	timestamp;
	int	fill;

	float	bot_peak_sig[2];		/*diag bottom_signal[] */
	float	cor_thresh_time;		/*cor_threshold_time*/
	float	sec_cor_thrsh_time;		/*second_cor_threshold_time */
	float	cor1_slnt_rnge_t;		/*diag cor1_slant_range_time */
	float	cor2_slnt_rnge_t;		/*diag cor2_slant_range_time */
	float	est_bot_det_prob;		/*est_bot_detection_prob*/
	float	sec_est_bt_det_prb;		/*sec_est_bot_det_prob*/
	float	hd_sft_sfc_t_dif;		/*hard_soft_sfc_time_dif*/
	float	gr_gr_depth_dif;		/*green_green_depth_dif */
	float	height;				
	float	apd_height;			
	float	sfc_peak_sig[4];		/*[0] not used */
	float	sfc_trac_height_m;		/*d sfc_tracker_height_mean */
	float	sfc_trac_height_sd;		/*d sfc_tracker_height_std_dev*/
	float	sft_sfc_t_dif[4];		/*soft_sfc_time_dif_ra[4]*/
	float	thresh_dif;			/* threshold_dif */
	float	sec_thresh_dif;			/*second_threshold_dif */
	float	total_apd_time;
	float	total_ir_time;
	float	total_raman_time;
	float	total_bot_time;			/* d total_bottom_time */
	float	roll_interp;			/* Interpolated to 400/200 Hz */
	float	pitch_interp;			/* Interpolated to 400/200 Hz */
	float	heading_interp;			/* Interpolated to 400/200 Hz */
	float	vert_accel_interp;		/* Interpolated to 400/200 Hz */

	short	bottom_chan;
	short	sec_bottom_chan;
	short	bot_risetime_bins[2];	
	short	num_sat_bins_apd;		/* num_sat_adp_bins */
	short	num_sat_bins_ir;		/* num_sat_ir_bins */
	short	sfc_channel_used;
};
typedef struct RES_PULSE RES_PULSE_T;


struct DIAG_PULSE {
	int	timestamp;
	int	fill;

	float	ambient_pe[4];
	float	bot_conf;			
	float	sec_bot_conf;			
	float	bot_fom[2];
	float	sec_bot_fom[2];			/* second_bot_fom[2]; */
	float	cor_soft_sfc_time[4];		
	float	est_bot_signal_basis[2];
	float	sec_time_dif;			/* second_time_dif */
	float	sfc_fom[4];				

	short	ambient_bkgnd;
	short	bot_start_bin_cor;
	short	bot_bin[2];
	short	sec_bot_bin[2];			/* second_bot_bin */
	short	bot_conf_flag;
	short	sec_bot_conf_flag;		/* second_bot_conf_flag */
	short	bot_fom_flag[2];
	short	sec_bot_fom_flag[2];		/* second_bot_fom_flag */
	short	bot_peak_bin[2];
	short	sec_bot_peak_bin[2];
	short	sfc_bin[4];
	short	sfc_fom_flag[4];	
	short	sfc_peak_bin[4];	
	short	sfc_risetime_bins[4];	
	short	sec_bot_risetime_bins[2];	
	short	str_bot_flag[2];
	short	sec_str_bot_flag[2];
	short	late_shift;
	short	fill2;


	union {
     		float 	ra[PMT_LEN+1];
     		char 	ch[(PMT_LEN+1)*sizeof(float)];
     	} pmt_waveform; 
	union {
     		float 	ra[APD_LEN+1];
     		char 	ch[(APD_LEN+1)*sizeof(float)];
	} apd_waveform; 
	union {
     		float 	ra[IR_LEN+1];
     		char 	ch[(IR_LEN+1)*sizeof(float)];
	} ir_waveform; 
	union {
     		float 	ra[RAMAN_LEN+1];
     		char 	ch[(RAMAN_LEN+1)*sizeof(float)];
	} raman_waveform; 

	union {
     		int 	ra[PMT_LEN+1];
     		char 	ch[(PMT_LEN+1)*sizeof(int)];
	} pmt_run; 
	union {
     		int 	ra[APD_LEN+1];
     		char 	ch[(APD_LEN+1)*sizeof(int)];
	} apd_sfc_run; 
	union {
     		int 	ra[APD_LEN+1];
     		char 	ch[(APD_LEN+1)*sizeof(int)];
	} apd_bot_run; 
	union {
     		int 	ra[IR_LEN+1];
     		char 	ch[(IR_LEN+1)*sizeof(int)];
	} ir_run; 
	union {
     		int 	ra[RAMAN_LEN+1];
     		char 	ch[(RAMAN_LEN+1)*sizeof(int)];
	} raman_run; 
};
typedef struct DIAG_PULSE DIAG_PULSE_T;


struct RES_FLIGHT {
	short	flightline;
	short	version;
	int	start_time;
	int	stop_time;
	float	gb_version;


	float	hd_sft_sfc_t_dif_m;		/*hard_soft_sfc_time_dif_mean*/
	float	hd_sft_sfc_t_dif_s;	     /*hard_soft_sfc_time_dif_std_dev*/
	float	hd_sfc_prb_nongrn;
	float	hd_sfc_prb_overall;
	float	hd_sfc_pc_used[4];		
	float	hdwr_sfc_prob[4];			
	float	land_percent;
	float	raman_fom_mean;
	float	raman_fom_std_dev;
	float	sft_sfc_nongreen_prob;		/*soft_sfc_nongreen_prob*/
	float	sft_sfc_overall_prob;		/*sft_sfc_overall_prob*/
	float	sft_sfc_pc_usd[4];		/* 0 not used */
	float	sft_sfc_prob[4];			
	float	sft_sfc_t_df_m[4];	/*soft_sfc_time_dif_mean_ra[4]*/
	float	sft_sfc_t_df_sd[4];	/*soft_sfc_time_dif_std_dev_ra[4]*/
	float	sfc_trac_perc_rejc;	/*sfc_tracker_percent_rejects*/
	
	int	glint_count;
	int	numss[4];		/* numss[0] not used */
};
typedef struct RES_FLIGHT RES_FLIGHT_T;



struct DIAG_FLIGHT {
	short	flightline;
	short	version;
	int	start_time;
	int	stop_time;
	float	gb_version;

	float	acc_bias_dif_thres;		/* accel_bias_dif_thresh*/
	float	est_laser_pulse_width;
	float	excess_noise_factor[4];
	float	noise_constant[4];
	float	noise_reduction_factor[4];
	float	raman_pe_per_count;
	float	surface_bias_time[4];	

	short	bottom_run_required[2];
	short	bottom_start_bin[2];
	short	bottom_stop_bin[2];
	short	sfc_run_required[4];	
	short	sfc_stop_bin[4];	

	union {
     		float 	ra[GAIN_RA_NUM+1];
     		char 	ch[(GAIN_RA_NUM+1)*sizeof(float)];
     	} pmt_pe_per_count; 
	union {
     		float 	ra[GAIN_RA_NUM+1];
     		char 	ch[(GAIN_RA_NUM+1)*sizeof(float)];
     	} apd_pe_per_count; 
	union {
     		float 	ra[GAIN_RA_NUM+1];
     		char 	ch[(GAIN_RA_NUM+1)*sizeof(float)];
     	} ir_pe_per_count;
};
typedef struct DIAG_FLIGHT DIAG_FLIGHT_T;

/****************************************/
/* From file_diag_flight.c			*/
/****************************************/
int file_write_resf(FILE *resf_file, RES_FLIGHT_T *resf_p);
int file_read_resf(FILE *resf_file, RES_FLIGHT_T *resf_p);
FILE	*file_open_diagf(char *db_name);
int file_read_diagf(FILE *diagf_file, DIAG_FLIGHT_T *diagf_p );
int file_write_diagf( FILE *diagf_file, DIAG_FLIGHT_T *diagf_p );

/****************************************/
/* From file_dr_lplse.c			*/
/****************************************/
FILE	*file_open_drlp(char *db_name, int flightline);
int	file_delete_drlp(char *db_name, int flightline);
int	file_read_drlp(FILE *drlp_file, DIAG_RES_LPULSE_T *drlp_p);
int	file_write_drlp(FILE *drlp_file, DIAG_RES_LPULSE_T *drlp_p);

/****************************************/
/* From file_diag_pulse.c			*/
/****************************************/
FILE	*file_open_diagp(char *db_name, int flightline);
int	file_delete_diagp(char *db_name, int flightline);
int	file_read_diagp(FILE *diagp_file, DIAG_PULSE_T *diagp_p);
int	file_write_diagp(FILE *diagp_file, DIAG_PULSE_T *diagp_p);
int	file_get_diagp(char *diagp_file, int fl, int timestamp, DIAG_PULSE_T *diagp_p);

/****************************************/
/* From file_res_pulse.c			*/
/****************************************/
FILE	*file_open_resp(char *db_name, int flightline);
int	file_delete_resp(char *db_name, int flightline);
int	file_read_resp(FILE *resp_file, RES_PULSE_T *resp_p);
int	file_write_resp(FILE *resp_file, RES_PULSE_T *resp_p);

/****************************************/
/* From file_res_flight.c		*/
/****************************************/
FILE	*file_open_resf(char *db_name);
int	file_read_resf(FILE *resf_file, RES_FLIGHT_T *resf_p);
int	file_write_resf(FILE *resf_file, RES_FLIGHT_T *resf_p);

/*******************************
* Item in old RESULT_GRD_T
* that are not in above... 
********************************
*	float	signal_false_alarm_prob;
*
********************************/

/*******************************
* Item in old DIAGNOSTICS_GRD_T
* that are not in above... 
********************************
*	float	threshold[4];				
*
*********************************/

#ifdef  __cplusplus
}
#endif


#endif /*DIAG_RES_H*/
