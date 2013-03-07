#ifndef GPS_H
#define GPS_H
/*****************************************************************************
 *
 * File:	gps.h
 *
 * Purpose:	gps file definition.  
 *
 *		 
 * Revision History:   
 * 98/04/06 DR	-wrote it
 * 
 * 98/09/29 DR	-removed constants (to ap/cnsts.h) and prototypes (file.h)
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif



struct GPS
{	
	double	receive_time;
 	double	latitude;
	double	longitude;
 	int	timestamp;
	float	altitude;
	short	hdop;
	short	fill;
};
typedef struct GPS GPS_T;

FILE *file_open_new_otf_gps( char *db_name, int flightline);
FILE *file_open_new_gps( char *db_name, int flightline);
FILE *file_open_otf_gps( char *db_name, int flightline);
FILE *file_open_gps( char *db_name, int flightline);
int file_read_gps( FILE *gps_file, GPS_T *gps_p);
int file_write_gps(FILE *gps_file, GPS_T *gps_p);


#ifdef  __cplusplus
}
#endif

#endif /*GPS_H*/
