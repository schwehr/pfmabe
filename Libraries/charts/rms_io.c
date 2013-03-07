#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>

#include "FileRMSOutput.h"

static NV_BOOL swap = NVFalse;
static NV_BOOL midnight = 0.0;
static NV_FLOAT64 start_gps_time = 0.0;
static NV_INT64 start_timestamp, end_timestamp, start_week;
static NV_INT32 year, month, day, start_record, end_record;


#ifdef NVWIN3X
    static NV_CHAR separator = '\\';
#else
    static NV_CHAR separator = '/';
#endif


#define WEEK_OFFSET  7.0L * 86400.0L * 1000000.0L






/***************************************************************************\
*                                                                           *
*   Module Name:        ngets                                               *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       May 1999                                            *
*                                                                           *
*   Purpose:            This is an implementation of fgets that strips the  *
*                       carriage return off of the end of the string if     *
*                       present.                                            *
*                                                                           *
*   Arguments:          See fgets                                           *
*                                                                           *
*   Return Value:       See fgets                                           *
*                                                                           *
\***************************************************************************/

static NV_CHAR *ngets (NV_CHAR *s, NV_INT32 size, FILE *stream)
{
    if (fgets (s, size, stream) == NULL) return (NULL);

    while( strlen(s)>0 && (s[strlen(s)-1] == '\n' || s[strlen(s)-1] == '\r') )
	s[strlen(s)-1] = '\0';

    if (s[strlen (s) - 1] == '\n') s[strlen (s) - 1] = 0;


    return (s);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        gen_basename                                        *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 2004                                       *
*                                                                           *
*   Purpose:            Generic replacement for POSIX basename.  One        *
*                       advantage to this routine over the POSIX one is     *
*                       that it doesn't destroy the input path.  This       *
*                       works on Windoze even when using MSYS (both types   *
*                       of specifiers).                                     *
*                                                                           *
*   Arguments:          path        -   path to parse                       *
*                                                                           *
*   Return Value:       NV_CHAR *   -   basename of the file, for example   *
*                                                                           *
*                       path           dirname        basename              *
*                       "/usr/lib"     "/usr"         "lib"                 *
*                       "/usr/"        "/"            "usr"                 *
*                       "usr"          "."            "usr"                 *
*                       "/"            "/"            "/"                   *
*                       "."            "."            "."                   *
*                       ".."           "."            ".."                  *
*                                                                           *
*   Calling Routines:   Utility routine                                     *
*                                                                           * 
\***************************************************************************/
                                                                            
static NV_CHAR *gen_basename (NV_CHAR *path)
{
  static NV_CHAR  basename[512];
  NV_INT32        i, j, start = 0, len;


  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;

  start = 0;
  for (i = len - 1 ; i >= 0 ; i--)
    {
      if (path[i] == '/' || path[i] == '\\')
        {
          start = i + 1;
          break;
        }
    }

  if (!start) return (path);

  for (j = start ; j < len ; j++) basename[j - start] = path[j];
  basename[len - start] = 0;

  return (basename);
}


/***************************************************************************\
*                                                                           *
*   Module Name:        gen_dirname                                         *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 2004                                       *
*                                                                           *
*   Purpose:            Generic replacement for POSIX dirname.  One         *
*                       advantage to this routine over the POSIX one is     *
*                       that it doesn't destroy the input path.  This       *
*                       works on Windoze even when using MSYS (both types   *
*                       of specifiers).                                     *
*                                                                           *
*   Arguments:          path        -   path to parse                       *
*                                                                           *
*   Return Value:       NV_CHAR *   -   basename of the file, for example   *
*                                                                           *
*                       path           dirname        basename              *
*                       "/usr/lib"     "/usr"         "lib"                 *
*                       "/usr/"        "/"            "usr"                 *
*                       "usr"          "."            "usr"                 *
*                       "/"            "/"            "/"                   *
*                       "."            "."            "."                   *
*                       ".."           "."            ".."                  *
*                                                                           *
*   Calling Routines:   Utility routine                                     *
*                                                                           * 
\***************************************************************************/
                                                                            
static NV_CHAR *gen_dirname (NV_CHAR *path)
{
  static NV_CHAR  dirname[512];
  NV_INT32        i, j, end = 0, len;


  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;
  if (!len) return (path);

  end = 0;
  for (i = len - 1 ; i >= 0 ; i--)
    {
      if (path[i] == '/' || path[i] == '\\')
        {
          end = i;
          break;
        }
    }

  if (!end) return (".");

  for (j = 0 ; j < end ; j++) dirname[j] = path[j];
  dirname[end] = 0;

  return (dirname);
}



/*  
    Given the HOF, TOF, PGPS, or IMG file name, this function returns the current RMS file name.  These files
    contain the RMS error information for the HOF/TOF/PGPS/IMG file.  The SBET file name is stored in the .kin
    file which should have the same name as the RMS file (but with a .kin extension).  We will change the file
    name found in the .kin file to be smrmsg_XXXX.out from SBET_XXXX.out.  If none of the files can be found
    the function will return NVFalse.
*/

NV_BOOL get_rms_file (NV_CHAR *htpi_file, NV_CHAR *rms_file)
{
  static NV_CHAR        prev_htpi[512], prev_rms[512];
  static NV_BOOL        prev_ret = NVFalse;
  NV_CHAR               kin_file[512], rms_dir[512], temp[512], string[512], rms_basename[512];
  FILE                  *fp;
  NV_INT32              i;


  /*  If we haven't changed HOF/TOF files we don't need to check again.  */

  if (!strcmp (htpi_file, prev_htpi))
    {
      if (prev_ret) strcpy (rms_file, prev_rms);
      return (prev_ret);
    }


  prev_ret = NVFalse;


  /*
      The SBET file name will be stored in the .kin file by GCS.  We will read that file first (if it exists)
      to get the correct filename.  It may still not match because Windoze doesn't care about upper/lower
      case but we can deal with that.  If there is no .kin file we're out of here.
  */


  strcpy (temp, htpi_file);
  strcpy (rms_basename, gen_basename (temp));


  /*  If the input filename is a .pgps file we can just use the basename of the file.  If it's a .hof, .tof, or.img
      we have to strip off some stuff and change DS to MD.  */

  if (!strstr (htpi_file, ".pgps"))
    {
      rms_basename[strlen (rms_basename) - 12] = 0;
      rms_basename[2] = 'M';
      rms_basename[3] = 'D';
    }


  /*  Strip off the HOF/TOF/PGPS extension.  */

  for (i = strlen (rms_basename) ; i > 0 ; i--)
    {
      if (rms_basename[i] == '.')
        {
          rms_basename[i] = 0;
          break;
        }
    }


  strcpy (rms_dir, gen_dirname (gen_dirname (temp)));


  /*  If you happen to be in the directory where the HOF/TOF/PGPS/IMG file is located you can't move up 
      to gen_dirname (gen_dirname (temp)).  That is, you can't go up a level since it's not in the
      htpi name.  In this case we set rms_dir to ..  */

  if (!strcmp (gen_dirname (temp), ".")) strcpy (rms_dir, "..");
  strcpy (temp, rms_dir);
  sprintf (kin_file, "%s%1cpos%1c%s.kin", rms_dir, separator, separator, rms_basename);

  if ((fp = fopen (kin_file, "r")) == NULL) return (NVFalse);


  ngets (string, sizeof (string), fp);
  fclose (fp);


  /*  Replace SBET with smrmsg.  */

  if (strstr (string, "SBET"))
    {
      strcpy (temp, "SMRMSG");
      strcat (temp, strstr (string, "SBET") + 4);
    }
  else
    {
      strcpy (temp, "smrmsg");
      strcat (temp, strstr (string, "sbet") + 4);
    }
  strcpy (string, temp);


  /*  The file name in the .kin file may be correct - will miracles never cease?  */

  sprintf (rms_file, "%s%1cpos%1c%s", rms_dir, separator, separator, string);
  if ((fp = fopen (rms_file, "rb")) != NULL)
    {
      fclose (fp);
      prev_ret = NVTrue;
      strcpy (prev_htpi, htpi_file);
      strcpy (prev_rms, rms_file);
      return (prev_ret);
    }


  /*  The file name in the .kin file may be upper or lower case.  First try lower case.  */

  for (i = 0 ; i < strlen (string) ; i++) string[i] = tolower (string[i]);


  sprintf (rms_file, "%s%1cpos%1c%s", rms_dir, separator, separator, string);
  if ((fp = fopen (rms_file, "rb")) != NULL)
    {
      fclose (fp);
      prev_ret = NVTrue;
      strcpy (prev_htpi, htpi_file);
      strcpy (prev_rms, rms_file);
      return (prev_ret);
    }


  /*  Next try upper case.  */

  for (i = 0 ; i < strlen (string) ; i++) string[i] = toupper (string[i]);

  sprintf (rms_file, "%s%1cpos%1c%s", rms_dir, separator, separator, string);
  if ((fp = fopen (rms_file, "rb")) != NULL)
    {
      fclose (fp);
      prev_ret = NVTrue;
      strcpy (prev_htpi, htpi_file);
      strcpy (prev_rms, rms_file);
      return (prev_ret);
    }

  return (prev_ret);
}



static void swap_rms (RMS_OUTPUT_T *rms)
{
  swap_NV_FLOAT64 (&rms->gps_time);
  swap_NV_FLOAT64 (&rms->north_pos_rms);
  swap_NV_FLOAT64 (&rms->south_pos_rms);
  swap_NV_FLOAT64 (&rms->down_pos_rms);
  swap_NV_FLOAT64 (&rms->north_vel_rms);
  swap_NV_FLOAT64 (&rms->south_vel_rms);
  swap_NV_FLOAT64 (&rms->down_vel_rms);
  swap_NV_FLOAT64 (&rms->roll_rms);
  swap_NV_FLOAT64 (&rms->pitch_rms);
  swap_NV_FLOAT64 (&rms->heading_rms);
}


FILE *open_rms_file (NV_CHAR *path)
{
  FILE                   *fp;
  NV_INT32               i;
  RMS_OUTPUT_T           rms;
  time_t                 tv_sec;
  NV_INT32               tv_nsec;
  struct tm              tm;
  static NV_INT32        tz_set = 0;


  NV_INT32 big_endian ();


  /*  Check the file name for following the naming convention as best we can.  */

  if (path[strlen (path) - 16] != '_' || path[strlen (path) - 9] != '_' || path[strlen (path) - 4] != '.')
    {
      fprintf (stderr, "\n\n\nFilename %s does not conform to standard\n", path);
      fprintf (stderr, "Please correct to match _YYMMDD_NNNN.out standard.\n\n\n");
      fflush (stderr);
      return (NULL);
    }


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
      putenv("TZ=GMT");
      tzset();
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
      fread (&rms, sizeof (RMS_OUTPUT_T), 1, fp);
      if (swap) swap_rms (&rms);
      start_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + rms.gps_time) * 1000000.0);
      start_record = 0;
      start_gps_time = rms.gps_time;


      i = fseek (fp, -(sizeof (RMS_OUTPUT_T)), SEEK_END);

      fread (&rms, sizeof (RMS_OUTPUT_T), 1, fp);
      if (swap) swap_rms (&rms);
      end_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + rms.gps_time) * 1000000.0);


      /*  Check for crossing midnight at end of GPS week (stupid f***ing Applanix bozos).  */

      if (end_timestamp < start_timestamp)
        {
          midnight = NVTrue;
          end_timestamp += WEEK_OFFSET;
        }


      end_record = ftell (fp) / sizeof (RMS_OUTPUT_T);

      fseek (fp, 0, SEEK_SET);
    }

  return (fp);
}


static NV_FLOAT64 interp (NV_FLOAT64 t0, NV_FLOAT64 t1, NV_FLOAT64 t2, NV_FLOAT64 y0, NV_FLOAT64 y2)
{
  return (y0 + (y2 - y0) * ((t1 - t0) / (t2 - t0)));
}


static RMS_OUTPUT_T rms_interp (RMS_OUTPUT_T rms, RMS_OUTPUT_T prev_rms, NV_FLOAT64 t1)
{
  RMS_OUTPUT_T new_rms;


  new_rms.gps_time = t1;

  new_rms.north_pos_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.north_pos_rms, prev_rms.north_pos_rms);
  new_rms.south_pos_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.south_pos_rms, prev_rms.south_pos_rms);
  new_rms.down_pos_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.down_pos_rms, prev_rms.down_pos_rms);
  new_rms.north_vel_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.north_vel_rms, prev_rms.north_vel_rms);
  new_rms.south_vel_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.south_vel_rms, prev_rms.south_vel_rms);
  new_rms.down_vel_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.down_vel_rms, prev_rms.down_vel_rms);
  new_rms.roll_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.roll_rms, prev_rms.roll_rms);
  new_rms.pitch_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.pitch_rms, prev_rms.pitch_rms);
  new_rms.heading_rms = interp (rms.gps_time, t1, prev_rms.gps_time, rms.heading_rms, prev_rms.heading_rms);

  return (new_rms);
}



NV_INT64 rms_find_record (FILE *fp, RMS_OUTPUT_T *rms, NV_INT64 timestamp)
{
  NV_INT64          x[3], time_found;
  NV_INT32          y[3], j;
  RMS_OUTPUT_T      prev_rms, new_rms;
  NV_FLOAT64        t1;


  if (timestamp < start_timestamp || timestamp > end_timestamp) return (0);


  t1 = (NV_FLOAT64) timestamp / 1000000.0 - start_week;


  /*  Load the x and y values into the local arrays.  */
    
  y[0] = start_record;
  y[2] = end_record;
  x[0] = start_timestamp;
  x[1] = timestamp;
  x[2] = end_timestamp;


  /*  Give it three shots at finding the time.    */
    
  for (j = 0; j < 3; j++)
    {
      y[1] = y[0] + (NV_INT32) ((NV_FLOAT64) (y[2] - y[0]) * ((NV_FLOAT64) (x[1] - x[0]) / (NV_FLOAT64) (x[2] - x[0])));


      /*  Get the time of the interpolated record.   */

      fseek (fp, (start_record + y[1] * sizeof (RMS_OUTPUT_T)), SEEK_SET);
      fread (rms, sizeof (RMS_OUTPUT_T), 1, fp);
      if (swap) swap_rms (rms);


      /*  Dealing with end of week midnight *&^@$^#%*!  */

      if (midnight && rms->gps_time < start_gps_time) rms->gps_time += WEEK_OFFSET;


      time_found = ((NV_FLOAT64) start_week + rms->gps_time) * 1000000.0;


      /*  If time found is less than the time searched for... */
        
      if (time_found < x[1])
        {
          x[0] = time_found;
          y[0] = y[1];
        }


      /*  If time found is greater than or equal to the time searched for... */

      else
        {
          x[2] = time_found;
          y[2] = y[1];
        }
    }


  prev_rms = *rms;


  /*  If time found is less than the time searched for, walk forward. */
    
  if (time_found <= x[1])
    {
      while (1)
        {
          y[1]++;

          fseek (fp, (start_record + y[1] * sizeof (RMS_OUTPUT_T)), SEEK_SET);
          fread (rms, sizeof (RMS_OUTPUT_T), 1, fp);
          if (swap) swap_rms (rms);


          /*  Dealing with end of week midnight *&^@$^#%*!  */

          if (midnight && rms->gps_time < start_gps_time) rms->gps_time += WEEK_OFFSET;


          time_found = ((NV_FLOAT64) start_week + rms->gps_time) * 1000000.0;


          if (time_found >= x[1]) 
            {
              /*
                rms_dump_record (prev_rms);
                rms_dump_record (*rms);
              */

              if (rms->gps_time - prev_rms.gps_time < 1000000.0)
                {
                  new_rms = rms_interp (prev_rms, *rms, t1);
                  *rms = new_rms;

                  /*
                    rms_dump_record (*rms);
                  */

                  return (timestamp);
                }
              else
                {
                  return (time_found);
                }
            }
        }
    }


  /*  If time found is greater than the time searched for, walk backward. */

  else
    {
      while (1)
        {
          y[1]--;

          fseek (fp, (start_record + y[1] * sizeof (RMS_OUTPUT_T)), SEEK_SET);
          fread (rms, sizeof (RMS_OUTPUT_T), 1, fp);
          if (swap) swap_rms (rms);


          /*  Dealing with end of week midnight *&^@$^#%*!  */

          if (midnight && rms->gps_time < start_gps_time) rms->gps_time += WEEK_OFFSET;


          time_found = ((NV_FLOAT64) start_week + rms->gps_time) * 1000000.0;


          if (time_found <= x[1])
            {
              /*
                rms_dump_record (*rms);
                rms_dump_record (prev_rms);
              */

              if (rms->gps_time - prev_rms.gps_time < 1000000.0)
                {
                  new_rms = rms_interp (*rms, prev_rms, t1);
                  *rms = new_rms;

                  /*
                    rms_dump_record (*rms);
                  */

                  return (timestamp);
                }
              else
                {
                  return (time_found);
                }
            }
        }
    }


  /*  If you get to here you haven't found a match.  */

  return (0);
}


NV_INT64 rms_get_start_timestamp ()
{
  return (start_timestamp);
}


NV_INT64 rms_get_end_timestamp ()
{
  return (end_timestamp);
}


NV_INT32 rms_read_record (FILE *fp, RMS_OUTPUT_T *rms)
{
  if (!fread (rms, sizeof (RMS_OUTPUT_T), 1, fp)) return (-1);
  if (swap) swap_rms (rms);


  /*  Dealing with end of week midnight *&^@$^#%*!  */

  if (midnight && rms->gps_time < start_gps_time) rms->gps_time += WEEK_OFFSET;


  return (0);
}


NV_INT32 rms_read_record_num (FILE *fp, RMS_OUTPUT_T *rms, NV_INT32 recnum)
{
  if (fseek (fp, recnum * sizeof (RMS_OUTPUT_T), SEEK_SET)) return (-1);

  if (!fread (rms, sizeof (RMS_OUTPUT_T), 1, fp)) return (-1);
  if (swap) swap_rms (rms);


  /*  Dealing with end of week midnight *&^@$^#%*!  */

  if (midnight && rms->gps_time < start_gps_time) rms->gps_time += WEEK_OFFSET;


  return (0);
}


void rms_dump_record (RMS_OUTPUT_T rms)
{
  fprintf (stderr, "GPS seconds of week      : %f\n", rms.gps_time);
  fprintf (stderr, "North position RMS error : %f\n", rms.north_pos_rms);
  fprintf (stderr, "South position RMS error : %f\n", rms.north_pos_rms);
  fprintf (stderr, "Down position RMS error  : %f\n", rms.north_pos_rms);
  fprintf (stderr, "North velocity RMS error : %f\n", rms.north_vel_rms);
  fprintf (stderr, "South velocity RMS error : %f\n", rms.north_vel_rms);
  fprintf (stderr, "Down velocity RMS error  : %f\n", rms.north_vel_rms);
  fprintf (stderr, "Roll RMS error           : %f\n", rms.roll_rms * ARC_TO_DEG);
  fprintf (stderr, "Pitch RMS error          : %f\n", rms.pitch_rms * ARC_TO_DEG);
  fprintf (stderr, "Heading RMS error        : %f\n", rms.heading_rms * ARC_TO_DEG);
  fflush (stderr);
}
