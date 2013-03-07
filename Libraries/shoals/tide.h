#ifndef TIDE_H
#define TIDE_H
/*****************************************************************************
 *
 * File:	tide.h
 *
 *
 * Purpose:	tide structure used by SHOALS 400 PFPS.
 *
 *		 
 * Revision History:   
 * 98/03/25 DR	-wrote it
 *
 * 98/09/29 DR	-changed TIDE_HDR_T to TIDE_ZONE_T
 * 		-changed TIDE_DTL_T to TIDE_ELEV_T
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct TIDE_ZONE
{
	int	zone_number;
	int	fill;
	double	min_lat;
	double	min_long;
	double	max_lat;
	double	max_long;
	char	zone_description[24];
	char	comments[40];
};
typedef struct TIDE_ZONE TIDE_ZONE_T;

struct TIDE_ELEV
{
	int	time_stamp;
	int	zone_number;
	int	date;
	int	time;
	float	elevation;
	float	fill;
};
typedef struct TIDE_ELEV TIDE_ELEV_T;

/* PROTOTYPES  */
int file_open_tide_zone(char *db_name);
void file_close_tide_zone();
int file_read_tide_zone(TIDE_ZONE_T *tide_zone_p);
int file_open_tide_elev(char *db_name);
void file_close_tide_elev();
int file_read_tide_elev(TIDE_ELEV_T *tide_elev_p);
int file_get_tide_zone(char *db_name, int zone_number, TIDE_ZONE_T *tide_zone_p);
int file_get_tide_elev(char *db_name, int zone_number, int timestamp, TIDE_ELEV_T *tide_elev_p);
int file_count_tide_elev(char *db_name, int survey_date);
int file_put_tide_zone(char *db_name, TIDE_ZONE_T *tide_zone_p);
int file_put_tide_elev(char *db_name, TIDE_ELEV_T *tide_elev_p);
int file_update_tide_zone(char *db_name, int cur_zone, TIDE_ZONE_T *tide_zone_p);
int file_update_tide_elev(char *db_name, int cur_zone, int timestamp, TIDE_ELEV_T *tide_elev_p);
int file_delete_tide_zone(char *db_name, int zone_number);
int file_delete_tide_elev(char *db_name, int zone_number, int timestamp);
int file_get_time_range_tide_elev(char *db_name,int survey_date, int *min_time, int *max_time);


#ifdef  __cplusplus
}
#endif

#endif /*tide_h*/
