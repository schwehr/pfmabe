#include "nvtypes.h"

/*****************************************************************************

  FilePOSOutput.h

  The POS file:

  This file is a subset of the DGPS/POS data as collected in the air.  This 
  file is created in a mission/POS directory during the downloading of a 
  mission (where  mission  is the mission name). The extension of the file is
  .pos.  The file format is binary and the data will be 200 Hz. The file does 
  NOT have a header.


  The POS SBET file:

  This file is NOT created during the download process. It will be created 
  after processing of GPS and POS data by the user in POSPROC. The filename 
  will be in the form  sbet_xxx.out  where the xxx part of the name is 
  controlled by the user processing the data. The file location is also user 
  controlled. This file is identical in structure to the POS Airborne file 
  (above), except that IMU and POS data have been post processed.

*****************************************************************************/


#ifndef POSHead
#define POSHead


#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"

#ifndef M_PI
#define M_PI         3.14159265358979323846    /* pi */
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG   180.0 / M_PI
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD   M_PI / 180.0
#endif


typedef struct
{
  NV_FLOAT64     gps_time;         /* seconds in GPS week                  */
  NV_FLOAT64     latitude;         /* radians                              */
  NV_FLOAT64     longitude;        /* radians                              */
  NV_FLOAT64     altitude;         /* meters from ellipsoid                */
  NV_FLOAT64     x_velocity;       /* m/sec                                */
  NV_FLOAT64     y_velocity;       /* m/sec                                */
  NV_FLOAT64     z_velocity;       /* m/sec                                */
  NV_FLOAT64     roll;             /* radians                              */
  NV_FLOAT64     pitch;            /* radians                              */
  NV_FLOAT64     platform_heading; /* radians  ** see below                */
  NV_FLOAT64     wander_angle;     /* radians  ** see below                */
  NV_FLOAT64     x_body_accel;     /* m/sec                                */
  NV_FLOAT64     y_body_accel;     /* m/sec                                */
  NV_FLOAT64     z_body_accel;     /* m/sec                                */
  NV_FLOAT64     x_body_ang_rate;  /* m/sec                                */
  NV_FLOAT64     y_body_ang_rate;  /* m/sec                                */
  NV_FLOAT64     z_body_ang_rate;  /* m/sec                                */
} POS_OUTPUT_T;

  /*

  ** To get a "true heading" the following equation should be used:
  true_heading = platform_heading - wander_angle

  */


  NV_BOOL get_pos_file (NV_CHAR *hof_tof_file, NV_CHAR *pos_file);
  FILE *open_pos_file (NV_CHAR *path);
  NV_INT64 pos_find_record (FILE *fp, POS_OUTPUT_T *pos, NV_INT64 timestamp);
  NV_INT64 pos_get_start_timestamp ();
  NV_INT64 pos_get_end_timestamp ();
  NV_INT64 pos_get_timestamp (POS_OUTPUT_T pos);
  NV_INT32 pos_read_record (FILE *fp, POS_OUTPUT_T *pos);
  NV_INT32 pos_read_record_num (FILE *fp, POS_OUTPUT_T *pos, NV_INT32 recnum);
  void pos_dump_record (POS_OUTPUT_T pos);


#ifdef  __cplusplus
}
#endif

#endif
