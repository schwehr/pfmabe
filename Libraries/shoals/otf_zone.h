#ifndef OTF_ZONE_H
#define OTF_ZONE_H
/*****************************************************************************
 *
 * File:	otf_zone.h
 *
 *
 * Purpose:	otf zone datum structure used by SHOALS 400 PFPS.
 *
 *		 
 * Revision History:   
 * 98/03/25 DR	-wrote it
 *
 * 98/09/15 DR	-added couple new ones...
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct OTF_ZONE
{
	int	zone_number;
	float	otf_datum;
	double	min_lat;
	double	min_long;
	double	max_lat;
	double	max_long;
	char	zone_description[24];
	char	comments[40];
};
typedef struct OTF_ZONE OTF_ZONE_T;

int file_open_otf_zone(char *db_name);
void file_close_otf_zone();
int file_read_otf_zone(OTF_ZONE_T *otf_zone_p);
int file_put_otf_zone(char *db_name, OTF_ZONE_T *otf_zone_p);
int file_count_otf_zone(char *db_name);
int file_get_otf_zones(char *db_name, int zone_number, OTF_ZONE_T *otf_zone_p);
int file_update_otf_zones(char *db_name, OTF_ZONE_T *otf_zone_p);
int file_delete_otf_zones(char *db_name, int zone_number);


#ifdef  __cplusplus
}
#endif

#endif /*OTF_ZONE_H*/






