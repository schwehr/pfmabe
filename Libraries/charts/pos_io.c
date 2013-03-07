#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>

#include "FilePOSOutput.h"

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


#define WEEK_OFFSET  7.0L * 86400.0L






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
    Given the HOF, TOF, PGPS, or IMG file name, this function returns the current SBET file name or, failing that,
    the .pos file name.  These files contain the precise navigation information for the HOF/TOF/PGPS/IMG file.
    We can use the information in these files to position data which has a different timestamp than the
    HOF/TOF/PGPS/IMG data such as the downlooking images.  The SBET file contains post-processed navigation that is
    better than the .pos data.  The SBET file name is stored in the .kin file which should have the same
    name as the .pos file (but with a .kin extension).  If none of the files can be found the function will
    return NVFalse.
*/

NV_BOOL get_pos_file (NV_CHAR *htpi_file, NV_CHAR *pos_file)
{
  static NV_CHAR        prev_htpi[512], prev_pos[512];
  static NV_BOOL        prev_ret = NVFalse;
  NV_CHAR               kin_file[512], pos_dir[512], temp[512], string[512], pos_basename[512];
  FILE                  *fp;
  NV_INT32              i;


  /*  If we haven't changed HOF/TOF files we don't need to check again.  */

  if (!strcmp (htpi_file, prev_htpi))
    {
      if (prev_ret) strcpy (pos_file, prev_pos);
      return (prev_ret);
    }


  prev_ret = NVFalse;


  /*
      The SBET file name will be stored in the .kin file by GCS.  We will read that file first (if it exists)
      to get the correct filename.  It may still not match because Windoze doesn't care about upper/lower
      case but we can deal with that.  If the .kin file isn't there or we can't find the SBET file we'll
      revert to the .pos file.
  */


  strcpy (temp, htpi_file);
  strcpy (pos_basename, gen_basename (temp));


  /*  If the input filename is a .pgps file we can just use the basename of the file.  If it's a .hof, .tof, or.img
      we have to strip off some stuff and change DS to MD.  */

  if (!strstr (htpi_file, ".pgps"))
    {
      pos_basename[strlen (pos_basename) - 12] = 0;
      pos_basename[2] = 'M';
      pos_basename[3] = 'D';
    }


  /*  Strip off the HOF/TOF/PGPS extension.  */

  for (i = strlen (pos_basename) ; i > 0 ; i--)
    {
      if (pos_basename[i] == '.')
        {
          pos_basename[i] = 0;
          break;
        }
    }


  strcpy (pos_dir, gen_dirname (gen_dirname (temp)));


  /*  If you happen to be in the directory where the HOF/TOF/PGPS/IMG file is located you can't move up 
      to gen_dirname (gen_dirname (temp)).  That is, you can't go up a level since it's not in the
      htpi name.  In this case we set pos_dir to ..  */

  if (!strcmp (gen_dirname (temp), ".")) strcpy (pos_dir, "..");
  strcpy (temp, pos_dir);
  sprintf (kin_file, "%s%1cpos%1c%s.kin", pos_dir, separator, separator, pos_basename);

  if ((fp = fopen (kin_file, "r")) != NULL)
    {
      ngets (string, sizeof (string), fp);
      fclose (fp);


      /*  The file name in the .kin file may be correct - will miracles never cease?  */

      sprintf (pos_file, "%s%1cpos%1c%s", pos_dir, separator, separator, string);
      if ((fp = fopen (pos_file, "rb")) != NULL)
        {
          fclose (fp);
          prev_ret = NVTrue;
          strcpy (prev_htpi, htpi_file);
          strcpy (prev_pos, pos_file);
          return (prev_ret);
        }


      /*  The file name in the .kin file may be upper or lower case.  First try lower case.  */

      for (i = 0 ; i < strlen (string) ; i++) string[i] = tolower (string[i]);


      sprintf (pos_file, "%s%1cpos%1c%s", pos_dir, separator, separator, string);
      if ((fp = fopen (pos_file, "rb")) != NULL)
        {
          fclose (fp);
          prev_ret = NVTrue;
          strcpy (prev_htpi, htpi_file);
          strcpy (prev_pos, pos_file);
          return (prev_ret);
        }


      /*  Next try upper case.  */

      for (i = 0 ; i < strlen (string) ; i++) string[i] = toupper (string[i]);

      sprintf (pos_file, "%s%1cpos%1c%s", pos_dir, separator, separator, string);
      if ((fp = fopen (pos_file, "rb")) != NULL)
        {
          fclose (fp);
          prev_ret = NVTrue;
          strcpy (prev_htpi, htpi_file);
          strcpy (prev_pos, pos_file);
          return (prev_ret);
        }
    }


  /*  If we got here then there was either no .kin file or we couldn't find/open the file listed in the
      .kin file.  Now we fall back to the .pos file.  */

  strcpy (temp, pos_dir);
  sprintf (pos_file, "%s%1cpos%1c%s.pos", pos_dir, separator, separator, pos_basename);

  if ((fp = fopen (pos_file, "rb")) != NULL)
    {
      fclose (fp);
      prev_ret = NVTrue;
      strcpy (prev_htpi, htpi_file);
      strcpy (prev_pos, pos_file);
      return (prev_ret);
    }

  return (prev_ret);
}



static void swap_pos (POS_OUTPUT_T *pos)
{
  swap_NV_FLOAT64 (&pos->gps_time);
  swap_NV_FLOAT64 (&pos->latitude);
  swap_NV_FLOAT64 (&pos->longitude);
  swap_NV_FLOAT64 (&pos->altitude);
  swap_NV_FLOAT64 (&pos->x_velocity);
  swap_NV_FLOAT64 (&pos->y_velocity);
  swap_NV_FLOAT64 (&pos->roll);
  swap_NV_FLOAT64 (&pos->pitch);
  swap_NV_FLOAT64 (&pos->platform_heading);
  swap_NV_FLOAT64 (&pos->wander_angle);
  swap_NV_FLOAT64 (&pos->x_body_accel);
  swap_NV_FLOAT64 (&pos->y_body_accel);
  swap_NV_FLOAT64 (&pos->z_body_accel);
  swap_NV_FLOAT64 (&pos->x_body_ang_rate);
  swap_NV_FLOAT64 (&pos->y_body_ang_rate);
  swap_NV_FLOAT64 (&pos->z_body_ang_rate);
}


FILE *open_pos_file (NV_CHAR *path)
{
  FILE                   *fp;
  NV_INT32               i;
  POS_OUTPUT_T           pos;
  time_t                 tv_sec;
  NV_INT32               tv_nsec;
  struct tm              tm;
  static NV_INT32        tz_set = 0;


  NV_INT32 big_endian ();


  /*  Check the file name for following the naming convention as best we can.  */

  if (path[strlen (path) - 16] != '_' || path[strlen (path) - 9] != '_' || path[strlen (path) - 4] != '.')
    {
      fprintf (stderr, "\n\n\nFilename %s does not conform to standard\n", path);
      fprintf (stderr, "Please correct to match _YYMMDD_NNNN.pos or _YYMMDD_NNNN.out standard.\n\n\n");
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
      fread (&pos, sizeof (POS_OUTPUT_T), 1, fp);
      if (swap) swap_pos (&pos);
      start_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + pos.gps_time) * 1000000.0);
      start_record = 0;
      start_gps_time = pos.gps_time;


      i = fseek (fp, -sizeof (POS_OUTPUT_T), SEEK_END);

      fread (&pos, sizeof (POS_OUTPUT_T), 1, fp);
      if (swap) swap_pos (&pos);
      end_timestamp = (NV_INT64) (((NV_FLOAT64) start_week + pos.gps_time) * 1000000.0);


      /*  Check for crossing midnight at end of GPS week (stupid f***ing Applanix bozos).  */

      if (end_timestamp < start_timestamp)
        {
          midnight = NVTrue;
          end_timestamp += ((NV_INT64) WEEK_OFFSET * 1000000);
        }


      end_record = ftell (fp) / sizeof (POS_OUTPUT_T);

      fseek (fp, 0, SEEK_SET);
    }

  return (fp);
}


static NV_FLOAT64 interp (NV_FLOAT64 t0, NV_FLOAT64 t1, NV_FLOAT64 t2, NV_FLOAT64 y0, NV_FLOAT64 y2)
{
  return (y0 + (y2 - y0) * ((t1 - t0) / (t2 - t0)));
}


static POS_OUTPUT_T pos_interp (POS_OUTPUT_T pos, POS_OUTPUT_T prev_pos, NV_FLOAT64 t1)
{
  POS_OUTPUT_T new_pos;


  new_pos.gps_time = t1;

  new_pos.latitude = interp (pos.gps_time, t1, prev_pos.gps_time, pos.latitude, prev_pos.latitude);
  new_pos.longitude = interp (pos.gps_time, t1, prev_pos.gps_time, pos.longitude, prev_pos.longitude);
  new_pos.altitude = interp (pos.gps_time, t1, prev_pos.gps_time, pos.altitude, prev_pos.altitude);
  new_pos.x_velocity = interp (pos.gps_time, t1, prev_pos.gps_time, pos.x_velocity, prev_pos.x_velocity);
  new_pos.y_velocity = interp (pos.gps_time, t1, prev_pos.gps_time, pos.y_velocity, prev_pos.y_velocity);
  new_pos.z_velocity = interp (pos.gps_time, t1, prev_pos.gps_time, pos.z_velocity, prev_pos.z_velocity);
  new_pos.roll = interp (pos.gps_time, t1, prev_pos.gps_time, pos.roll, prev_pos.roll);
  new_pos.pitch = interp (pos.gps_time, t1, prev_pos.gps_time, pos.pitch, prev_pos.pitch);
  new_pos.platform_heading = interp (pos.gps_time, t1, prev_pos.gps_time, pos.platform_heading, prev_pos.platform_heading);
  new_pos.wander_angle = interp (pos.gps_time, t1, prev_pos.gps_time, pos.wander_angle, prev_pos.wander_angle);
  new_pos.x_body_accel = interp (pos.gps_time, t1, prev_pos.gps_time, pos.x_body_accel, prev_pos.x_body_accel);
  new_pos.y_body_accel = interp (pos.gps_time, t1, prev_pos.gps_time, pos.y_body_accel, prev_pos.y_body_accel);
  new_pos.z_body_accel = interp (pos.gps_time, t1, prev_pos.gps_time, pos.z_body_accel, prev_pos.z_body_accel);
  new_pos.x_body_ang_rate = interp (pos.gps_time, t1, prev_pos.gps_time, pos.x_body_ang_rate, prev_pos.x_body_ang_rate);
  new_pos.y_body_ang_rate = interp (pos.gps_time, t1, prev_pos.gps_time, pos.y_body_ang_rate, prev_pos.y_body_ang_rate);
  new_pos.z_body_ang_rate = interp (pos.gps_time, t1, prev_pos.gps_time, pos.z_body_ang_rate, prev_pos.z_body_ang_rate);

  return (new_pos);
}



NV_INT64 pos_find_record (FILE *fp, POS_OUTPUT_T *pos, NV_INT64 timestamp)
{
  NV_INT64          x[3], time_found;
  NV_INT32          y[3], j;
  POS_OUTPUT_T      prev_pos, new_pos;
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

      fseek (fp, (start_record + y[1] * sizeof (POS_OUTPUT_T)), SEEK_SET);
      fread (pos, sizeof (POS_OUTPUT_T), 1, fp);
      if (swap) swap_pos (pos);


      /*  Dealing with end of week midnight *&^@$^#%*!  */

      if (midnight && pos->gps_time < start_gps_time) pos->gps_time += WEEK_OFFSET;


      time_found = ((NV_FLOAT64) start_week + pos->gps_time) * 1000000.0;


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


  prev_pos = *pos;


  /*  If time found is less than the time searched for, walk forward. */
    
  if (time_found <= x[1])
    {
      while (1)
        {
          y[1]++;

          fseek (fp, (start_record + y[1] * sizeof (POS_OUTPUT_T)), SEEK_SET);
          fread (pos, sizeof (POS_OUTPUT_T), 1, fp);
          if (swap) swap_pos (pos);


          /*  Dealing with end of week midnight *&^@$^#%*!  */

          if (midnight && pos->gps_time < start_gps_time) pos->gps_time += WEEK_OFFSET;


          time_found = ((NV_FLOAT64) start_week + pos->gps_time) * 1000000.0;


          if (time_found >= x[1]) 
            {
              /*
                pos_dump_record (prev_pos);
                pos_dump_record (*pos);
              */

              if (pos->gps_time - prev_pos.gps_time < 1000000.0)
                {
                  new_pos = pos_interp (prev_pos, *pos, t1);
                  *pos = new_pos;

                  /*
                    pos_dump_record (*pos);
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

          fseek (fp, (start_record + y[1] * sizeof (POS_OUTPUT_T)), SEEK_SET);
          fread (pos, sizeof (POS_OUTPUT_T), 1, fp);
          if (swap) swap_pos (pos);


          /*  Dealing with end of week midnight *&^@$^#%*!  */

          if (midnight && pos->gps_time < start_gps_time) pos->gps_time += WEEK_OFFSET;


          time_found = ((NV_FLOAT64) start_week + pos->gps_time) * 1000000.0;


          if (time_found <= x[1])
            {
              /*
                pos_dump_record (*pos);
                pos_dump_record (prev_pos);
              */

              if (pos->gps_time - prev_pos.gps_time < 1000000.0)
                {
                  new_pos = pos_interp (*pos, prev_pos, t1);
                  *pos = new_pos;

                  /*
                    pos_dump_record (*pos);
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


NV_INT64 pos_get_start_timestamp ()
{
  return (start_timestamp);
}


NV_INT64 pos_get_end_timestamp ()
{
  return (end_timestamp);
}


NV_INT64 pos_get_timestamp (POS_OUTPUT_T pos)
{
  NV_INT64 time_found;

  if (midnight && pos.gps_time < start_gps_time) pos.gps_time += WEEK_OFFSET;

  time_found = ((NV_FLOAT64) start_week + pos.gps_time) * 1000000.0;

  return (time_found);
}


NV_INT32 pos_read_record (FILE *fp, POS_OUTPUT_T *pos)
{
  if (!fread (pos, sizeof (POS_OUTPUT_T), 1, fp)) return (-1);
  if (swap) swap_pos (pos);


  /*  Dealing with end of week midnight *&^@$^#%*!  */

  if (midnight && pos->gps_time < start_gps_time) pos->gps_time += WEEK_OFFSET;


  return (0);
}


NV_INT32 pos_read_record_num (FILE *fp, POS_OUTPUT_T *pos, NV_INT32 recnum)
{
  if (fseek (fp, recnum * sizeof (POS_OUTPUT_T), SEEK_SET)) return (-1);

  if (!fread (pos, sizeof (POS_OUTPUT_T), 1, fp)) return (-1);
  if (swap) swap_pos (pos);


  /*  Dealing with end of week midnight *&^@$^#%*!  */

  if (midnight && pos->gps_time < start_gps_time) pos->gps_time += WEEK_OFFSET;


  return (0);
}


void pos_dump_record (POS_OUTPUT_T pos)
{
  fprintf (stderr, "GPS seconds of week : %f\n", pos.gps_time);
  fprintf (stderr, "latitude            : %f\n", pos.latitude * RAD_TO_DEG);
  fprintf (stderr, "longitude           : %f\n", pos.longitude * RAD_TO_DEG);
  fprintf (stderr, "altitude            : %f\n", pos.altitude);
  fprintf (stderr, "x velocity          : %f\n", pos.x_velocity);
  fprintf (stderr, "y velocity          : %f\n", pos.y_velocity);
  fprintf (stderr, "z velocity          : %f\n", pos.z_velocity);
  fprintf (stderr, "roll                : %f\n", pos.roll * RAD_TO_DEG);
  fprintf (stderr, "pitch               : %f\n", pos.pitch * RAD_TO_DEG);
  fprintf (stderr, "platform heading    : %f\n", pos.platform_heading * RAD_TO_DEG);
  fprintf (stderr, "wander angle        : %f\n", pos.wander_angle * RAD_TO_DEG);
  fprintf (stderr, "x body acceleration : %f\n", pos.x_body_accel);
  fprintf (stderr, "y body acceleration : %f\n", pos.y_body_accel);
  fprintf (stderr, "z body acceleration : %f\n", pos.z_body_accel);
  fprintf (stderr, "x body angular rate : %f\n", pos.x_body_ang_rate);
  fprintf (stderr, "y body angular rate : %f\n", pos.y_body_ang_rate);
  fprintf (stderr, "z body angular rate : %f\n\n", pos.z_body_ang_rate);
  fflush (stderr);
}
