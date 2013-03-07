#ifndef SURVEY_LIMITS_H
#define SURVEY_LIMITS_H
/*****************************************************************************
 *
 * File:	survey_limits.h
 *
 *
 * Purpose:	Definition of the SURVEY_LIMITS_T structure for 
 *		file definition and internal use....
 *
 *		 
 * Revision History:   
 * 98/04/02 DR	-wrote it
 *****************************************************************************/

#ifdef  __cplusplus
extern "C" {
#endif


struct SURVEY_LIMITS
{
	float	map_lllat;
	float	map_lllon;
	float	map_urlat;
	float	map_urlon;
	float	cal_lllat;
	float	cal_lllon;
	float	cal_urlat;
	float	cal_urlon;
	float	total_shots;
};
typedef struct SURVEY_LIMITS SURVEY_LIMITS_T;

int file_get_survey_limits(char *db_name, SURVEY_LIMITS_T *survey_limits_p);
int file_put_survey_limits(char *db_name, SURVEY_LIMITS_T *survey_limits_p);


#ifdef  __cplusplus
}
#endif

#endif /*survey_limits_h*/
