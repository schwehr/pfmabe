#ifndef FLIGHT_MAP_H
#define FLIGHT_MAP_H
/*****************************************************************************
 *
 * File:	flight_map.h
 *
 *
 * Purpose:	flight_map structure used by SHOALS 400 PFPS.
 *		In dbase version, map_pen was either an "U" or
 *		"D" (up/down).  Now, it is a short int with
 *		 a 0 or 1 (up/down).
 *		 
 * Revision History:   
 * 98/04/15 DR	-wrote it
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct FLIGHTMAP_GRD
{
	float	map_x;
	float	map_y;
	short	map_pen;
	short	fill;
};
typedef struct FLIGHTMAP_GRD FLIGHTMAP_GRD_T;

FILE *file_open_flightmap( char *db_name);
int	file_read_flightmap( FILE *flightmap_file, FLIGHTMAP_GRD_T *flightmap_p);
int file_write_flightmap( FILE *flightmap_file, FLIGHTMAP_GRD_T *flightmap_p);

#ifdef  __cplusplus
}
#endif


#endif
