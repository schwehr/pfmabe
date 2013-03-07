#ifndef INS_H
#define INS_H
/*****************************************************************************
 * ins.h 	Header
 *
 * Purpose:  	This header file hold the definition of the ins 
 *		structures for use with the the ins file.
 *              
 *
 * Revision History:
 * 99/05/03 DR	-created it 
 *
 * 99/12/10 DR	-added prototypes and updated for NT
 *
 * 99/12/14 DR	-added prototypes from src/ins.c
 ****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif



typedef struct 
{
	int	timestamp;
	float	heading;
} INS_32_T;

typedef struct 
{
	int	timestamp;
	float	pitch;
	float	roll;
	float	vert_accel;
} INS_64_T;

typedef struct 
{
	int	timestamp;
	float	gr_speed;
} INS_16_T;


/* From file_ins.c */
FILE	*file_open_ins64(char *db_name, int flightline);
FILE	*file_open_ins32(char *db_name, int flightline);
int	file_read_ins64(FILE *ins64_file, INS_64_T *ins64_p);
int	file_read_ins32(FILE *wave_file, INS_32_T *ins32_p);
int	file_write_ins64(FILE *wave_file, INS_64_T *ins64_p);
int	file_write_ins32(FILE *wave_file, INS_32_T *ins32_p);
int 	file_get_ins64(char *db_name, short flightline,
		int timestamp, INS_64_T	*ins64_p);
int 	file_get_ins32(char *db_name, short flightline,
		int timestamp, INS_32_T	*ins32_p);

/* From src/ins.c */
void	init_ins_interp();
int	get_ins_interp(int	timestamp, float *interp_roll, float *interp_pitch,
				   float *interp_heading, float *interp_vert_accel);
void  get_ins64_record(INS_64_T **ins64_in_1, INS_64_T **ins64_in_2, 
					   int time_stamp, int *ins_flag);
void	get_ins32_record(INS_32_T **ins32_in_1, INS_32_T **ins32_in_2, int time_stamp, int *ins_flag);
void	interp_ins(int laser_time_stamp, int ins_time_1, float ins_item_1,
				int ins_time_2, float ins_item_2, float *output_ins_item);


#ifdef  __cplusplus
}
#endif

#endif
