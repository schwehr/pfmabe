#include "nvtypes.h"

/*****************************************************************************

  FileGPS.h

  The GPS file:

  This file is a subset of the GPS/POS data as collected in the air.  This 
  file is created in a mission/POS directory during the downloading of a 
  mission (where  mission  is the mission name). The extension of the file is
  .gps.  The file format is binary. The file does NOT have a header.

*****************************************************************************/


#ifndef __GPSHead__
#define __GPSHead__


#ifdef  __cplusplus
extern "C" {
#endif


#include "charts.h"


typedef struct
{
  NV_FLOAT64 gps_time;            /* GPS time                                  */
  NV_FLOAT64 Time1;               /* POS Time1 // S.B. UTC                     */
  NV_FLOAT64 Time2;               /* POS Time2 // S.B. sec since turnon        */
  NV_FLOAT32 HDOP;
  NV_FLOAT32 VDOP;
  NV_FLOAT32 fill_float[10];

  NV_INT16 TimeType;              /* SB. Bit 2 (UTC) and Bit5 (POS time) only  */
  NV_INT16 Mode;                  /* 0 = No data from receiver                 */
                                  /* 1 = 2d C/A                                */
                                  /* 2 = 3d C/A                                */
                                  /* 3 = 2d DGPS                               */
                                  /* 4 = 3d DGPS                               */
                                  /* 5 = float RTK                             */
                                  /* 6 = Integer wide lane RTK                 */
                                  /* 7 = Integer narrow lane RTK               */

  NV_INT16 num_sats;              /* Number of satellites tracked...           */
  NV_INT16 week;
  NV_INT16 fill_short[8];
} GPS_OUTPUT_T;


  FILE *open_gps_file (NV_CHAR *path);
  NV_INT32 gps_read_record (FILE *fp, GPS_OUTPUT_T *gps);
  void gps_dump_record (GPS_OUTPUT_T gps);


#ifdef  __cplusplus
}
#endif

#endif
