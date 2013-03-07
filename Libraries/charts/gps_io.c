#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#include "FileGPSOutput.h"

static NV_BOOL swap = NVFalse;
static NV_INT64 start_timestamp, end_timestamp, start_week;
static NV_INT32 year, month, day, start_record, end_record;


static void swap_gps (GPS_OUTPUT_T *gps)
{
  NV_INT32 i;


  swap_NV_FLOAT64 (&gps->gps_time);
  swap_NV_FLOAT64 (&gps->Time1);
  swap_NV_FLOAT64 (&gps->Time2);
  swap_NV_FLOAT32 (&gps->HDOP);
  swap_NV_FLOAT32 (&gps->VDOP);
  for (i = 0 ; i < 10 ; i++) swap_NV_FLOAT32 (&gps->fill_float[i]);
  swap_NV_INT16 (&gps->TimeType);
  swap_NV_INT16 (&gps->Mode);
  swap_NV_INT16 (&gps->num_sats);
  swap_NV_INT16 (&gps->week);
  for (i = 0 ; i < 8 ; i++) swap_NV_INT16 (&gps->fill_short[i]);
}


FILE *open_gps_file (NV_CHAR *path)
{
  FILE                   *fp;
  NV_INT32               i;
  GPS_OUTPUT_T           gps;
  time_t                 tv_sec;
  NV_INT32               tv_nsec;
  struct tm              tm;
  static NV_INT32        tz_set = 0;


  NV_INT32 big_endian ();


  sscanf (&path[strlen (path) - 15], "%02d%02d%02d", &year, &month, &day);


  /*  tm struct wants years since 1900!!!  */

  tm.tm_year = year + 100;
  tm.tm_mon = month - 1;
  tm.tm_mday = day;
  tm.tm_hour = 0.0;
  tm.tm_min = 0.0;
  tm.tm_sec = 0.0;
  tm.tm_isdst = -1;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      _putenv("TZ=GMT");
      _tzset();
  #else
      _putenv("TZ=GMT");
      _tzset();
  #endif
#else
      putenv("TZ=GMT");
      tzset();
#endif
      tz_set = 1;
    }


  /*  Get seconds from the epoch (01-01-1970) for the date in the filename. 
      This will also give us the day of the week for the GPS seconds of
      week calculation.  */

  tv_sec = mktime (&tm);
  tv_nsec = 0.0;


  /*  Subtract the number of days since Saturday midnight (Sunday morning) in seconds.  */

  tv_sec = tv_sec - (tm.tm_wday * 86400);
  start_week = tv_sec;


  /*  We have to assume that the file is little endian since there is no
      header and no field that we can use to deduce what it is.  */

  swap = (NV_BOOL) big_endian ();


  if ((fp = fopen (path, "rb")) == NULL)
    {
      return ((FILE *) NULL);
    }
  else
    {
      fread (&gps, sizeof (GPS_OUTPUT_T), 1, fp);
      if (swap) swap_gps (&gps);
      start_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + gps.gps_time) * 1000000.0);
      start_record = 0;


      i = fseeko64 (fp, -sizeof (GPS_OUTPUT_T), SEEK_END);

      fread (&gps, sizeof (GPS_OUTPUT_T), 1, fp);
      if (swap) swap_gps (&gps);
      end_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + gps.gps_time) * 1000000.0);

      end_record = ftell (fp) / sizeof (GPS_OUTPUT_T);

      fseek (fp, 0, SEEK_SET);
    }

  return (fp);
}


NV_INT32 gps_read_record (FILE *fp, GPS_OUTPUT_T *gps)
{
  if (!fread (gps, sizeof (GPS_OUTPUT_T), 1, fp)) return (-1);
  if (swap) swap_gps (gps);

  return (0);
}


void gps_dump_record (GPS_OUTPUT_T gps)
{
  fprintf (stderr, "GPS seconds of week : %f\n", gps.gps_time);
  fprintf (stderr, "POS Time1           : %f\n", gps.Time1);
  fprintf (stderr, "POS Time2           : %f\n", gps.Time2);
  fprintf (stderr, "HDOP                : %f\n", gps.HDOP);
  fprintf (stderr, "VDOP                : %f\n", gps.VDOP);
  fprintf (stderr, "Time type           : %hd\n", gps.TimeType);
  fprintf (stderr, "Mode                : %hd\n", gps.Mode);
  fprintf (stderr, "Num sats            : %hd\n", gps.num_sats);
  fprintf (stderr, "Week                : %hd\n", gps.week);
  fflush (stderr);
}
