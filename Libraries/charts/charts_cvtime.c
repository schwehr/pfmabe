#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "nvtypes.h"

void charts_cvtime (NV_INT64 micro_sec, NV_INT32 *year, NV_INT32 *jday, 
                    NV_INT32 *hour, NV_INT32 *minute, NV_FLOAT32 *second)
{
    static NV_INT32      tz_set = 0;
    struct tm            time_struct, *time_ptr = &time_struct;
    time_t               tv_sec;
    NV_INT32             msec;


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
    

    tv_sec = micro_sec / 1000000;
    time_ptr = localtime (&tv_sec);

    msec = micro_sec % 1000000;

    *year = (short) time_ptr->tm_year;
    *jday = (short) time_ptr->tm_yday + 1;
    *hour = (short) time_ptr->tm_hour;
    *minute = (short) time_ptr->tm_min;
    *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) msec / 1000000.);
}



void charts_jday2mday (int year, int jday, int *mon, int *mday)
{
  static int              months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int                     l_year;

  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }


    *mday = jday;
    for (*mon = 0 ; *mon < 12 ; (*mon)++)
    {
        if (*mday - months[*mon] <= 0) break;
        *mday -= months[*mon];
    }
}
