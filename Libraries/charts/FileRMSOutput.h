#include "nvtypes.h"

/*****************************************************************************

  FileRMSOutput.h

  The RMS file:

  This file is NOT created during the download process. It will be created 
  after processing of GPS and POS data by the user in POSPROC. The filename 
  will be in the form  smrmsg_xxx.out  where the xxx part of the name is 
  controlled by the user processing the data. The file location is also user 
  controlled.

*****************************************************************************/


#ifndef RMSHead
#define RMSHead


#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


#define ARC_TO_DEG   0.016
#define DEG_TO_ARC   62.5


typedef struct
{
  NV_FLOAT64     gps_time;         /* seconds in GPS week                  */
  NV_FLOAT64     north_pos_rms;    /* meters                               */
  NV_FLOAT64     south_pos_rms;    /* meters                               */
  NV_FLOAT64     down_pos_rms;     /* meters                               */
  NV_FLOAT64     north_vel_rms;    /* m/sec                                */
  NV_FLOAT64     south_vel_rms;    /* m/sec                                */
  NV_FLOAT64     down_vel_rms;     /* m/sec                                */
  NV_FLOAT64     roll_rms;         /* arc minutes                          */
  NV_FLOAT64     pitch_rms;        /* arc minutes                          */
  NV_FLOAT64     heading_rms;      /* arc minutes                          */
} RMS_OUTPUT_T;


  NV_BOOL get_rms_file (NV_CHAR *hof_tof_file, NV_CHAR *rms_file);
  FILE *open_rms_file (NV_CHAR *path);
  NV_INT64 rms_find_record (FILE *fp, RMS_OUTPUT_T *rms, NV_INT64 timestamp);
  NV_INT64 rms_get_start_timestamp ();
  NV_INT64 rms_get_end_timestamp ();
  NV_INT32 rms_read_record (FILE *fp, RMS_OUTPUT_T *rms);
  NV_INT32 rms_read_record_num (FILE *fp, RMS_OUTPUT_T *rms, NV_INT32 recnum);
  void rms_dump_record (RMS_OUTPUT_T rms);


#ifdef  __cplusplus
}
#endif

#endif
