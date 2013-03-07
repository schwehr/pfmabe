#ifndef FLIGHTLINE_H
#define FLIGHTLINE_H

/*****************************************************************************
 *
 * File:	flightline.h
 *
 *
 * Purpose:	Flightline structure used by SHOALS 400 PFPS.
 *
 * Revision History:   
 * 98/03/24 DR	-wrote it
 * 00/01/05 DR	-added file_get_min_flightline()
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct FLIGHTLINE_GRD
{
	int	survey_date;
	int	start_time;
	int	end_time;
	short	flightline;
	short	date_code;
	short	gps_time_offset;
	char	oi_line[6];
};
typedef struct FLIGHTLINE_GRD FLIGHTLINE_GRD_T;

int file_get_flightline( char *db_name, short flightline, FLIGHTLINE_GRD_T *flightline_p);
int file_put_flightline( char *db_name, FLIGHTLINE_GRD_T *flight_line_p);

int file_delete_flightline( char *db_name, short flightline );
FILE *file_open_flightline( char *db_name );
int file_read_flightline( FILE *flightline_file, FLIGHTLINE_GRD_T *flightline_p);

int file_get_max_flightline( char *db_name );
int file_get_min_flightline(char *db_name);
int file_access_flightline( char *db_name);

int file_get_fl_by_date( char *db_name, int survey_date,FLIGHTLINE_GRD_T *flightline_p);

#ifdef  __cplusplus
}
#endif


#endif /*FLIGHTLINE_H*/
