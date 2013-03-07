#ifndef OUTPUT_H
#define OUTPUT_H
/*****************************************************************************
 * output.h 	Header
 *
 * Purpose:  	This header file hold the definition of the output 
 *		structure for use with the Automated Processing  algorithm.
 *              
 * Revision History:
 * ...
 *
 * 98/03/24 DR	-updated for 400 hz.  Added some prototyes. 
 *
 * 98/07/07 DR	-More prototypes 
 *		-new structure OUTPUT_FILE_T added which contains the
 *		 auto structure "OUTPUT_GRD_T" plus all other elements
 *		 that go into the output file (ring elements).
 *
 * 98/07/07 DR	-added prototype for file_ll_read_output() 
 *
 * 98/09/23 DR	-moved all of the "ring" items in the OUTPUT_FILE_T 
 *			definition into a new structure called OUTPUT_RING_T
 *			which is a part of OUTPUT_FILE_T...
 * 98/09/28 DR	-removed "st_" stuff from vars...
 *		-changed pos_latitude/longitude to longitude, latitude
 *		-changed all "second_" to "sec_"
 *		-changed all "bottom_" to "bot_"
 *		-changed all "surface_" to "sfc_"
 *		-changed all "sec_lat/log" to "sec_latitude/longitude"
 *		-reordered some items
 *****************************************************************************/

/* OUTPUTS */

#ifdef  __cplusplus
extern "C" {
#endif


struct OUTPUT_GRD {
	int		timestamp;
	short		flightline;
	short		haps_version;
	double		pulse_time;
	double		latitude;
	double		longitude;
	double		sec_latitude;
	double		sec_longitude;
	float		tide_cor_depth;
	float		reported_depth;
	float		altitude;
	float		elevation;
	float		otf_datum;
	float		otf_elevation;
	float		otf_res_elev;
	float		otf_sec_elev;
	float		otf_topo;
	float		otf_water_level;
	float		result_depth;
	float		sec_depth;
	float		topo;
	float		wave_height;
	int		depth_conf;
	int		sec_depth_conf;
	short		ab_dep_conf;
	short		sec_ab_dep_conf;
	short		otf_abd_conf;
	short		otf_sec_abd_conf;
 	short		position_conf;
	short		status;
};
typedef struct OUTPUT_GRD OUTPUT_GRD_T;


struct OUTPUT_EARLY {
	float	bot_conf;
	float	sec_bot_conf;	 
	float	nadir_angle;		
	float	scanner_azimuth;
	float	sfc_fom_apd;	 
	float	sfc_fom_ir;		 
	float	sfc_fom_ram;	 
	int	warnings;		
	int	warnings2;
	short	bot_bin_first;	 
	short	bot_bin_second;	 
	short	bot_bin_used_pmt;	 
	short	sec_bot_bin_used_pmt;	 
	short	bot_bin_used_apd;	 
 	short	sec_bot_bin_used_apd;
	short	bot_channel;		 
	short	sec_bot_chan;	 
	short	sfc_bin_apd;	 	
	short	sfc_bin_ir;		 	
	short	sfc_bin_ram;	 	
	short	sfc_channel_used;
};
typedef struct OUTPUT_EARLY OUTPUT_EARLY_T;

struct SUN_OUTPUT_EARLY {
	float	        unknown1;
        OUTPUT_EARLY_T  early;
	int	        unknown2;		
};
typedef struct SUN_OUTPUT_EARLY SUN_OUTPUT_EARLY_T;

struct OUTPUT_FILE {
	OUTPUT_GRD_T	au;	/* All items from "t-star" in auto (4.x) */
	OUTPUT_EARLY_T	early;	/* All earlier items which go into ring buff */
};
typedef struct OUTPUT_FILE OUTPUT_FILE_T;		/* Whole output file defintion... */
	
FILE *file_open_output(char *db_name, int flightline);
int file_read_output(FILE *output_file, OUTPUT_FILE_T *output_p);
int file_write_output(FILE *output_file, OUTPUT_FILE_T *output_p);
int file_delete_output(char *db_name, int flightline);
int file_get_output(char *db_name, short flightline, int timestamp, OUTPUT_FILE_T *output_p);
int file_open_rewrite_output(char *db_name, short flightline);
int file_ll_read_output(int filehandle, OUTPUT_FILE_T *output_p);
int file_rewrite_output(int filehandle, OUTPUT_FILE_T *output_p, int backup_flag);
int file_setup_rewirte_output(char *db_name, int filehandle, short flightline, int timestamp);
int file_update_rec_output(char *db_name, OUTPUT_FILE_T *output_p);
void sun_shift_early (OUTPUT_EARLY_T *early);
void dump_output (OUTPUT_FILE_T out);


#ifdef  __cplusplus
}
#endif


#endif /*output_h*/
