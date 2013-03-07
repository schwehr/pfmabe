
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "normtime.h"


/********************************************************************************************/
/*!

 - Function:    unnormtime

 - Purpose:     Convert from seconds from 01-01-1970 to year, day of year, hour, minute, second.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - time           =    Seconds from epoch (Jan. 1, 1970)
                - year           =    2 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute

 - Returns:
                - void

 - Caveats:     The localtime function will return the year as a 2 digit year (offset from 1900).

*********************************************************************************************/
 
void unnormtime (NV_FLOAT64 time, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour, NV_INT32 *minute, NV_FLOAT32 *second)
{
  static NV_INT32      tz_set = 0;
  struct tm            time_struct, *time_ptr = &time_struct;
  time_t               tv_sec;
  NV_FLOAT64           psec;


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
    

  tv_sec = (NV_INT64) time;
  time_ptr = localtime (&tv_sec);
  psec = fmod (time, 1.0);

  *year = (short) time_ptr->tm_year;
  *jday = (short) time_ptr->tm_yday + 1;
  *hour = (short) time_ptr->tm_hour;
  *minute = (short) time_ptr->tm_min;
  *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) (psec);
}



/********************************************************************************************/
/*!

 - Function:    normtime

 - Purpose:     Convert from year, day of year, hour, minute, second to seconds from
                01-01-1970.

 - Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

 - Date:        07/20/11

 - Arguments:
                - year           =    2 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute

 - Returns:
                - void

 - Caveats:     The mktime function takes a 2 digit year (offset from 1900).

*********************************************************************************************/
 
NV_FLOAT64 normtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec)
{
  struct tm                    tm;
  static NV_INT32              tz_set = 0;
  time_t                       tv_sec;
  long                         tv_nsec;

  void jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday);


  tm.tm_year = year;

  jday2mday (year, jday, &tm.tm_mon, &tm.tm_mday);

  tm.tm_hour = hour;
  tm.tm_min = min;
  tm.tm_sec = (NV_INT32) sec;
  tm.tm_isdst = 0;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv ("TZ=GMT");
      tzset ();
#endif
      tz_set = 1;
    }

  tv_sec = mktime (&tm);
  tv_nsec = (long)(fmod ((double) sec, 1.0) * 1.0e9);

  return ((NV_FLOAT64) tv_sec + (NV_FLOAT64) tv_nsec / 1000000000.0L);
}
