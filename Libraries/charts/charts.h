#ifndef __CHARTS_H__
#define __CHARTS_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>


#ifndef MAX
#define MAX(x,y) 	( ((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y) 	( ((x) < (y)) ? (x) : (y))
#endif


#undef CHARTS_DEBUG


  void charts_cvtime (NV_INT64 micro_sec, NV_INT32 *year, NV_INT32 *jday, 
		      NV_INT32 *hour, NV_INT32 *minute, NV_FLOAT32 *second);
  void charts_jday2mday (int year, int jday, int *mon, int *mday);
  void swap_NV_INT32 (NV_INT32 *word);
  void swap_NV_FLOAT32 (NV_FLOAT32 *word);
  void swap_NV_FLOAT64 (NV_FLOAT64 *word);
  void swap_NV_INT64 (NV_INT64 *word);
  void swap_NV_INT16 (NV_INT16 *word);
  void lidar_get_string (NV_CHAR *in, NV_CHAR *out);


#ifdef  __cplusplus
}
#endif


#endif
