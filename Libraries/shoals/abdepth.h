#ifndef ABDEPTH_H
#define ABDEPTH_H
/*****************************************************************************
 *
 * File:	abdepth.h
 *
 * Purpose:	abdepth structure/file definition.  Also has prototypes
 *		for abdepth file routines.
 *
 *		 
 * Revision History:   
 * 98/04/16 DR	-wrote it
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct ABDEPTH_GRD
{	
	int	timestamp;
	short	flightline;
	float	confidence;
	float	latitude;
	float	longitude;
	float	abdepth;  
};
typedef struct ABDEPTH_GRD ABDEPTH_GRD_T;


FILE *file_open_abdepth( char *db_name, int flightline);
int file_read_abdepth( FILE *abdepth_file, ABDEPTH_GRD_T *abdepth_p);
int file_write_abdepth( FILE *abdepth_file, ABDEPTH_GRD_T *abdepth_p);

#ifdef  __cplusplus
}
#endif


#endif
