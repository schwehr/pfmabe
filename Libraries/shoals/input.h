#ifndef INPUT_H
#define INPUT_H

/*****************************************************************************
 * input.h 	Header
 *
 * Purpose:  	This header file hold the definition of the inputs 
 *		structure for use with the the inputs file.
 *              
 *
 * Revision History:
 * 98/06/11 DR	-created it from a couple of other older structures. 
 *
 * 98/09/25 DR	-moved flightlineto be near top.... 
 * 
 * 98/09/28 DR	-renamed t_interval_meter_X to timX (X=1 or 2).
 *		-shortened digitizer_start_time to dig_start_t
 *		-shortened hdwr_sfc_status to hdwr_sfc_stat
 *
 * 98/09/29 DR	-removed polarization_land_flag and land_water_flag;
 *		-renamed change_land_status to user_land_flag
 *
 * 98/10/08 DR	-raman_amp_switch added (can change on shot by shot 
 *		 basis...)
 *
 * 98/11/11 DR	-tim1_flag added (for bad TIM1 shots... )
 * 
 * 98/12/09 DR	-added gr_speed
 * 
 * 99/02/17 DR	GG CC #9
 *		-removed from inputs:
 * 			short	raman_amp_switch;
 * 		-Also backed out of other ram2 code as dual gain amplifier
 *		has been physically removed from system.
 *
 * 99/12/03 DR	-brought up to date with 2.30 unix version.
 ****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>

struct INPUT_GRD
{
	double	micro_sec; 
	int	timestamp;
	int	shot_id;
	short	flightline;
	short	fill;

	float	avg_slant_range;
	float	dig_start_t[4]; 
	float	ext_sr_time;
	float	gr_speed;
	float	heading;
	float	laser_energy;
	float	pitch;
	float	roll;
	float	scanner_x;
	float	scanner_y;
	float	tim1;
	float	tim2;
	float	vert_accel;

	short	data_offset_t[4];
	short	hdwr_sfc_stat[4]; 
	short	pmt_bkgnd_offset; 
	short	pod_temperature; 
	short	rcvr_rnge_gate_dis;
	short	rcvr_status_flags;
	short	scanner_fix_attrib;
	short	sfc_channel;
	short	tim1_flag;
	short	user_land_flag;	/* 0, no change, 1 made land, 2 made water */
	short	extra_short;
};
typedef struct INPUT_GRD INPUT_GRD_T;

FILE *file_open_input( char *db_name, int flightline);
int file_delete_input( char *db_name, int flightline);
int file_read_input( FILE *input_file, INPUT_GRD_T *input_p);
int file_write_input( FILE *input_file, INPUT_GRD_T *input_p);
int file_get_input( char *db_name, short flightline, int timestamp, INPUT_GRD_T *input_p);
int file_update_rec_input( char *db_name, INPUT_GRD_T *input_p);


#ifdef  __cplusplus
}
#endif

#endif
