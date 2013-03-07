
/*********************************************************************************************

    This library is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/

#include <errno.h>
#include <signal.h>
#include <math.h>

#include "hawkeye.h"
#include "hawkeye_version.h"


#define         DPRINT          fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);
#define         WEEK_OFFSET     7.0L * 86400.0L


#undef HAWKEYE_DEBUG


/*  This is the structure we use to keep track of important formatting data for an open HAWKEYE file.  */

typedef struct
{
  FILE          *fp;                        /*  HAWKEYE file pointer.  */
  FILE          *sfm_fp;                    /*  SFM index file pointer.  */
  FILE          *dat_fp;                    /*  DAT waveform file pointer.  */
  NV_BOOL       swap;
  NV_U_BYTE     *point_record_buffer;
  NV_CHAR       path[1024];                 /*  File name.  */
  NV_CHAR       sfm_path[1024];             /*  SFM file name.  */
  NV_CHAR       dat_path[1024];             /*  DAT file name (almost, the file number comes from the sfm record).  */
  void          *rec_ptr[100];              /*  Record pointers within the HAWKEYE_RECORD structure (right now there are only 80 possible and
                                                I just didn't feel like realloc'ing this thing as I read the header).  */
  NV_INT32      previous_dat_file_index;

  NV_FLOAT64    start_gps_time;
  NV_FLOAT64    start_week;


  /*  HAWKEYE headers.  */

  HAWKEYE_META_HEADER                       MetaHeader;
  HAWKEYE_CONTENTS_HEADER                   ContentsHeader;
} INTERNAL_HAWKEYE_STRUCT;


/*  This is where we'll store the formatting data (including headers) for all open HAWKEYE files.  */

static INTERNAL_HAWKEYE_STRUCT hawkeye[HAWKEYE_MAX_FILES];


/*  This record is used to unpack data from the character strings that are stored in the hawkeye file.  */

static HAWKEYE_RECORD internal_hawkeye_record;


/*  Startup flag used by hawkeye_open_file to initialize the internal struct array and set the SIGINT handler.  */

static NV_BOOL first = NVTrue;


/*  Used for packing/unpacking data to/from packed input/output buffers.  */

static NV_U_BYTE *ptr;

static union
{
  NV_U_BYTE    buff[8];
  NV_U_BYTE    u;
  NV_CHAR      c;
  NV_BOOL      b;
  NV_INT16     s;
  NV_U_INT16   us;
  NV_INT32     i;
  NV_U_INT32   ui;
  NV_FLOAT32   f;
  NV_FLOAT64   d;
} data;


/*  HAWKEYE error handling variables.  */

typedef struct 
{
  NV_INT32      system;            /*  Last system error condition encountered.  */
  NV_INT32      hawkeye;           /*  Last HAWKEYE error condition encountered.  */
  NV_CHAR       file[512];         /*  Name of file being accessed when last error encountered (if applicable).  */
  NV_INT32      recnum;            /*  Record number being accessed when last error encountered (if applicable).  */
  NV_CHAR       info[128];
} HAWKEYE_ERROR_STRUCT;


static HAWKEYE_ERROR_STRUCT hawkeye_error;

static NV_BOOL tz_set = NVFalse;


/*********************************************************************************************

    UTILITY FUNCTIONS

*********************************************************************************************/



/*  Months start at zero, days at 1 (go figure).  */

static NV_INT32              months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


/*********************************************************************************************

    Function        hawkeye_jday2mday - Convert from day of year to month and day.

    Synopsis        hawkeye_jday2mday (year, jday, &mon, &mday);

                    NV_INT32 year           4 digit year or offset from 1900
                    NV_INT32 jday           day of year
                    NV_INT32 mon            month
                    NV_INT32 mday           day of month

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The returned month value will start at 0 for January.

*********************************************************************************************/
 
static void hawkeye_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday)
{
  NV_INT32 l_year;

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



/*********************************************************************************************

    Function        hawkeye_cvtime - Convert from POSIX time to year, day of year, hour, minute,
                    second.

    Synopsis        hawkeye_cvtime (tv_sec, tv_nsec, &year, &jday, &hour, &minute, &second);

                    time_t tv_sec           POSIX seconds from epoch (Jan. 1, 1970)
                    long tv_nsec            POSIX nanoseconds of second
                    NV_INT32 year           4 digit year - 1900
                    NV_INT32 jday           day of year
                    NV_INT32 hour           hour of day
                    NV_INT32 minute         minute of hour
                    NV_FLOAT32 second       seconds of minute

    Returns         N/A

    Author          Jan C. Depner

    Caveats         The localtime function will return the year as a 2 digit year (offset
                    from 1900).

*********************************************************************************************/
 
static void hawkeye_cvtime (time_t tv_sec, long tv_nsec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour, NV_INT32 *minute, NV_FLOAT32 *second)
{
  struct tm            time_struct, *time_ptr = &time_struct;

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
      tz_set = NVTrue;
    }

  time_ptr = localtime (&tv_sec);

  *year = (NV_INT16) time_ptr->tm_year;
  *jday = (NV_INT16) time_ptr->tm_yday + 1;
  *hour = (NV_INT16) time_ptr->tm_hour;
  *minute = (NV_INT16) time_ptr->tm_min;
  *second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) tv_nsec / 1000000000.);
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
                                                                            
static NV_CHAR *hawkeye_gen_dirname (const NV_CHAR *path)
{
  static NV_CHAR  dirname[512];
  NV_INT32        i, j, end = 0, len;


  strcpy (dirname, path);

  len = strlen (path);

  if (path[len - 1] == '/' || path[len - 1] == '\\') len--;
  if (!len) return (dirname);

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



/***************************************************************************\
*                                                                           *
*   Module Name:        gen_strrstr                                         *
*                                                                           *
*   Programmer(s):      Unknown                                             *
*                                                                           *
*   Date Written:       Unknown                                             *
*                                                                           *
*   Purpose:            One of the many implementations of strrstr (which   *
*                       doesn't exist in C) to be found on the internet.    *
*                       There are more efficient ways to do this but I      *
*                       always appreciate simplicity.                       *
*                                                                           *
*   Arguments:          str         -   path to search                      *
*                       strSearch   -   string to be searched for           *
*                                                                           *
*   Return Value:       NV_CHAR *   -   pointer to search string in str     *
*                                                                           *
\***************************************************************************/
                                                                            
static NV_CHAR *hawkeye_gen_strrstr (const NV_CHAR *str, const NV_CHAR *strSearch)
{
  NV_CHAR *ptr, *last = NULL;

  ptr = (NV_CHAR *) str;

  while ((ptr = strstr (ptr, strSearch))) last = ptr++;

  return last;
}



/***************************************************************************\
*                                                                           *
*   Module Name:        big_endian                                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Purpose:            This function checks to see if the system is        *
*                       big-endian or little-endian.                        *
*                                                                           *
*   Arguments:          NONE                                                *
*                                                                           *
*   Returns:            3 if big-endian, 0 if little-endian                 *
*                                                                           *
\***************************************************************************/

static NV_INT32 hawkeye_big_endian ()
{
    union
    {
        NV_INT32        word;
        NV_U_BYTE       byte[4];
    } a;

    a.word = 0x00010203;
    return ((NV_INT32) a.byte[3]);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_INT32                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a four byte int.       *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_INT32     *
*                                                                           *
\***************************************************************************/

static void hawkeye_swap_NV_INT32 (NV_INT32 *word)
{
    NV_U_INT32    temp[4];

    temp[0] = *word & 0x000000ff;

    temp[1] = (*word & 0x0000ff00) >> 8;

    temp[2] = (*word & 0x00ff0000) >> 16;

    temp[3] = (*word & 0xff000000) >> 24;

    *word = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_FLOAT32                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 1992                                           *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a four byte float.     *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_FLOAT32   *
*                                                                           *
\***************************************************************************/

static void hawkeye_swap_NV_FLOAT32 (NV_FLOAT32 *word)
{
    NV_U_INT32    temp[4];

    union
    {
        NV_U_INT32    iword;
        NV_FLOAT32    fword;
    } eq;

    eq.fword = *word;

    temp[0] = eq.iword & 0x000000ff;

    temp[1] = (eq.iword & 0x0000ff00) >> 8;

    temp[2] = (eq.iword & 0x00ff0000) >> 16;

    temp[3] = (eq.iword & 0xff000000) >> 24;

    eq.iword = (temp[0] << 24) + (temp[1] << 16) + (temp[2] << 8) + temp[3];

    *word = eq.fword;

    return;
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_FLOAT64                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       January 2000                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in an eight byte double.  *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_FLOAT64   *
*                                                                           *
\***************************************************************************/

static void hawkeye_swap_NV_FLOAT64 (NV_FLOAT64 *word)
{
    NV_INT32    i;
    NV_CHAR     temp;
    union
    {
        NV_CHAR     bytes[8];
        NV_FLOAT64  d;
    }eq;
    
    memcpy (&eq.bytes, word, 8);

    for (i = 0 ; i < 4 ; i++)
    {
        temp = eq.bytes[i];
        eq.bytes[i] = eq.bytes[7 - i];
        eq.bytes[7 - i] = temp;
    }

    *word = eq.d;
}



/***************************************************************************\
*                                                                           *
*   Module Name:        swap_NV_INT16                                       *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 1996                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function swaps bytes in a two byte int.        *
*                                                                           *
*   Arguments:          word                -   pointer to the NV_INT16     *
*                                                                           *
\***************************************************************************/

static void hawkeye_swap_NV_INT16 (NV_INT16 *word)
{
    NV_INT16   temp;

    swab (word, &temp, 2);

    *word = temp;

    return;
}



/*********************************************************************************************

    INTERNAL FUNCTIONS

*********************************************************************************************/


/*********************************************************************************************

  Function:    hawkeye_parse_tags

  Purpose:     Checks the field tag for it's contents and points the record pointer for
               that data type to the correct part of the internal HAWKEYE_RECORD structure.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The HAWKEYE file handle
               i              -    Record pointer index
               field_tag      -    The field tag

  Returns:     N/A

*********************************************************************************************/

static void hawkeye_parse_tags (NV_INT32 hnd, NV_INT32 i, NV_CHAR *field_tag)
{
  if (strstr (field_tag, "Timestamp"))
    {
      hawkeye[hnd].ContentsHeader.available.Timestamp = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Timestamp;
    }
  else if (strstr (field_tag, "Surface Latitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Latitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Latitude;
    }
  else if (strstr (field_tag, "Surface Longitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Longitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Longitude;
    }
  else if (strstr (field_tag, "Surface Northing StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Northing_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Northing_StdDev;
    }
  else if (strstr (field_tag, "Surface Northing"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Northing = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Northing;
    }
  else if (strstr (field_tag, "Surface Easting StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Easting_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Easting_StdDev;
    }
  else if (strstr (field_tag, "Surface Easting"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Easting = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Easting;
    }
  else if (strstr (field_tag, "Surface Altitude, interpol."))
    {
      hawkeye[hnd].ContentsHeader.available.Interpolated_Surface_Altitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Interpolated_Surface_Altitude;
    }
  else if (strstr (field_tag, "Surface Altitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Altitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Altitude;
    }
  else if (strstr (field_tag, "Surface Altitude StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_Altitude_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_Altitude_StdDev;
    }
  else if (strstr (field_tag, "Point Latitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Latitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Latitude;
    }
  else if (strstr (field_tag, "Point Longitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Longitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Longitude;
    }
  else if (strstr (field_tag, "Point Northing StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Northing_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Northing_StdDev;
    }
  else if (strstr (field_tag, "Point Northing"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Northing = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Northing;
    }
  else if (strstr (field_tag, "Point Easting StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Easting_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Easting_StdDev;
    }
  else if (strstr (field_tag, "Point Easting"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Easting = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Easting;
    }
  else if (strstr (field_tag, "Point Altitude StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Altitude_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Altitude_StdDev;
    }
  else if (strstr (field_tag, "Point Altitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Point_Altitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Point_Altitude;
    }
  else if (strstr (field_tag, "Azimuth"))
    {
      hawkeye[hnd].ContentsHeader.available.Azimuth = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Azimuth;
    }
  else if (strstr (field_tag, "Air Nadir Angle"))
    {
      hawkeye[hnd].ContentsHeader.available.Air_Nadir_Angle = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Air_Nadir_Angle;
    }
  else if (strstr (field_tag, "Water Nadir Angle"))
    {
      hawkeye[hnd].ContentsHeader.available.Water_Nadir_Angle = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Water_Nadir_Angle;
    }
  else if (strstr (field_tag, "Slant Range Comp. Model"))
    {
      hawkeye[hnd].ContentsHeader.available.Slant_Range_Comp_Model = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Slant_Range_Comp_Model;
    }
  else if (strstr (field_tag, "Slant range"))
    {
      hawkeye[hnd].ContentsHeader.available.Slant_range = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Slant_range;
    }
  else if (strstr (field_tag, "Wave Height"))
    {
      hawkeye[hnd].ContentsHeader.available.Wave_Height = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Wave_Height;
    }
  else if (strstr (field_tag, "Water Quality Correction"))
    {
      hawkeye[hnd].ContentsHeader.available.Water_Quality_Correction = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Water_Quality_Correction;
    }
  else if (strstr (field_tag, "Tidal Correction"))
    {
      hawkeye[hnd].ContentsHeader.available.Tidal_Correction = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Tidal_Correction;
    }
  else if (strstr (field_tag, "Depth Amplitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Depth_Amplitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Depth_Amplitude;
    }
  else if (strstr (field_tag, "Depth Class"))
    {
      hawkeye[hnd].ContentsHeader.available.Depth_Class = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Depth_Class;
    }
  else if (strstr (field_tag, "Depth Confidence"))
    {
      hawkeye[hnd].ContentsHeader.available.Depth_Confidence = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Depth_Confidence;
    }
  else if (strstr (field_tag, "Depth"))
    {
      hawkeye[hnd].ContentsHeader.available.Depth = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Depth;
    }
  else if (strstr (field_tag, "Pixel Index"))
    {
      hawkeye[hnd].ContentsHeader.available.Pixel_Index = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Pixel_Index;
    }
  else if (strstr (field_tag, "Scanner Angle X"))
    {
      hawkeye[hnd].ContentsHeader.available.Scanner_Angle_X = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scanner_Angle_X;
    }
  else if (strstr (field_tag, "Scanner Angle Y"))
    {
      hawkeye[hnd].ContentsHeader.available.Scanner_Angle_Y = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scanner_Angle_Y;
    }
  else if (strstr (field_tag, "Aircraft Latitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Latitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Latitude;
    }
  else if (strstr (field_tag, "Aircraft Longitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Longitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Longitude;
    }
  else if (strstr (field_tag, "Aircraft Northing StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Northing_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Northing_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Northing"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Northing = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Northing;
    }
  else if (strstr (field_tag, "Aircraft Easting StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Easting_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Easting_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Easting"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Easting = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Easting;
    }
  else if (strstr (field_tag, "Aircraft Altitude StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Altitude_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Altitude_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Altitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Altitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Altitude;
    }
  else if (strstr (field_tag, "Aircraft Roll StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Roll_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Roll_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Roll"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Roll = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Roll;
    }
  else if (strstr (field_tag, "Aircraft Pitch StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Pitch_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Pitch_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Pitch"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Pitch = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Pitch;
    }
  else if (strstr (field_tag, "Aircraft Heading StdDev"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Heading_StdDev = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Heading_StdDev;
    }
  else if (strstr (field_tag, "Aircraft Heading"))
    {
      hawkeye[hnd].ContentsHeader.available.Aircraft_Heading = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Aircraft_Heading;
    }
  else if (strstr (field_tag, "Extracted Waveform Attributes"))
    {
      hawkeye[hnd].ContentsHeader.available.Extracted_Waveform_Attributes = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Extracted_Waveform_Attributes;
    }
  else if (strstr (field_tag, "Receiver Data Used"))
    {
      hawkeye[hnd].ContentsHeader.available.Receiver_Data_Used = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Receiver_Data_Used;
    }
  else if (strstr (field_tag, "Manual Output Screening Flags"))
    {
      hawkeye[hnd].ContentsHeader.available.Manual_Output_Screening_Flags = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Manual_Output_Screening_Flags;
    }
  else if (strstr (field_tag, "Waveform Peak: Amplitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Amplitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Amplitude;
    }
  else if (strstr (field_tag, "Waveform Peak: Classification"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Classification = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Classification;
    }
  else if (strstr (field_tag, "Waveform Peak: Contrast"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Contrast = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Contrast;
    }
  else if (strstr (field_tag, "Waveform Peak: Debug Flags"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Debug_Flags = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Debug_Flags;
    }
  else if (strstr (field_tag, "Waveform Peak: Attributes"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Attributes = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Attributes;
    }
  else if (strstr (field_tag, "Waveform Peak: Jitter"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Jitter = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Jitter;
    }
  else if (strstr (field_tag, "Waveform Peak: Position"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Position = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Position;
    }
  else if (strstr (field_tag, "Waveform Peak: Pulsewidth"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Pulsewidth = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Pulsewidth;
    }
  else if (strstr (field_tag, "Waveform Peak: SNR"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_SNR = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_SNR;
    }
  else if (strstr (field_tag, "Scan Direction Flag"))
    {
      hawkeye[hnd].ContentsHeader.available.Scan_Direction_Flag = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scan_Direction_Flag;
    }
  else if (strstr (field_tag, "Edge of Flightline"))
    {
      hawkeye[hnd].ContentsHeader.available.Edge_of_Flightline = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Edge_of_Flightline;
    }
  else if (strstr (field_tag, "Scan Angle Rank"))
    {
      hawkeye[hnd].ContentsHeader.available.Scan_Angle_Rank = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scan_Angle_Rank;
    }
  else if (strstr (field_tag, "SelectBottom Case"))
    {
      hawkeye[hnd].ContentsHeader.available.SelectBottomCase = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.SelectBottomCase;
    }
  else if (strstr (field_tag, "Elevation"))
    {
      hawkeye[hnd].ContentsHeader.available.Elevation = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Elevation;
    }
  else if (strstr (field_tag, "Return Number"))
    {
      hawkeye[hnd].ContentsHeader.available.Return_Number = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Return_Number;
    }
  else if (strstr (field_tag, "Number of Returns"))
    {
      hawkeye[hnd].ContentsHeader.available.Number_of_Returns = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Number_of_Returns;
    }
  else if (strstr (field_tag, "Surf Alt interp, tide corrected"))
    {
      hawkeye[hnd].ContentsHeader.available.Surface_altitude_interpolated_tide_corrected = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Surface_altitude_interpolated_tide_corrected;
    }
  else if (strstr (field_tag, "Infrared polarisation ratio"))
    {
      hawkeye[hnd].ContentsHeader.available.IR_polarisation_ratio = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.IR_polarisation_ratio;
    }
  else if (strstr (field_tag, "Spatial Quality"))
    {
      hawkeye[hnd].ContentsHeader.available.Spatial_Quality = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Spatial_Quality;
    }
  else if (strstr (field_tag, "Infrared amplitude"))
    {
      hawkeye[hnd].ContentsHeader.available.IR_AMPLITUDE = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.IR_AMPLITUDE;
    }
  else if (strstr (field_tag, "Scan Position"))
    {
      hawkeye[hnd].ContentsHeader.available.Scan_Position = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scan_Position;
    }
  else if (strstr (field_tag, "K-Lidar"))
    {
      hawkeye[hnd].ContentsHeader.available.K_Lidar = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.K_Lidar;
    }
  else if (strstr (field_tag, "k-lidar average"))
    {
      hawkeye[hnd].ContentsHeader.available.K_Lidar_Average = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.K_Lidar_Average;
    }
  else if (strstr (field_tag, "k-lidar deep momentary"))
    {
      hawkeye[hnd].ContentsHeader.available.K_Lidar_Deep_Momentary = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.K_Lidar_Deep_Momentary;
    }
  else if (strstr (field_tag, "k-lidar shallow momentary"))
    {
      hawkeye[hnd].ContentsHeader.available.K_Lidar_Shallow_Momentary = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.K_Lidar_Shallow_Momentary;
    }
  else if (strstr (field_tag, "Relative reflectivity"))
    {
      hawkeye[hnd].ContentsHeader.available.Relative_reflectivity = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Relative_reflectivity;
    }
  else if (strstr (field_tag, "Waveform Peak: Gain"))
    {
      hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Gain = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Waveform_Peak_Gain;
    }
  else if (strstr (field_tag, "Adjusted Amplitude"))
    {
      hawkeye[hnd].ContentsHeader.available.Adjusted_Amplitude = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Adjusted_Amplitude;
    }
  else if (strstr (field_tag, "Travel time water"))
    {
      hawkeye[hnd].ContentsHeader.available.Water_Travel_Time = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Water_Travel_Time;
    }
  else if (strstr (field_tag, "IOPs: Attenuation c"))
    {
      hawkeye[hnd].ContentsHeader.available.Attenuation_c = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Attenuation_c;
    }
  else if (strstr (field_tag, "IOPs: Absorption a"))
    {
      hawkeye[hnd].ContentsHeader.available.Absorption_a = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Absorption_a;
    }
  else if (strstr (field_tag, "IOPs: Scattering s"))
    {
      hawkeye[hnd].ContentsHeader.available.Scattering_s = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Scattering_s;
    }
  else if (strstr (field_tag, "IOPs: Backscattering Bb"))
    {
      hawkeye[hnd].ContentsHeader.available.Backscattering_Bb = NVTrue;
      hawkeye[hnd].rec_ptr[i] = (void *) &internal_hawkeye_record.Backscattering_Bb;
    }
  else
    {
      fprintf (stderr, "Unknown Hawkeye field tag - %s\n\n", field_tag);
      exit (-1);
    }
}



/*********************************************************************************************

  Function:    clean_exit

  Purpose:     Exit from the application after first cleaning up memory.
               This will only be called in the case of an abnormal exit (like a failed malloc).

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   ret            -    Value to be passed to exit ();  If set to -999 we were
                                   called from the SIGINT signal handler so we must return
                                   to allow it to SIGINT itself.

  Returns:     N/A

*********************************************************************************************/

static void clean_exit (NV_INT32 ret)
{
  NV_INT32 i, j;

  for (i = 0 ; i < HAWKEYE_MAX_FILES ; i++)
    {
      if (hawkeye[i].fp != NULL)
        {
          if (hawkeye[i].ContentsHeader.PointDataFieldDescr)
            {
              for (j = 0 ; j < hawkeye[i].ContentsHeader.NbrOfPointDataFields ; j++) free (hawkeye[i].ContentsHeader.PointDataFieldDescr[j]);
              free (hawkeye[i].ContentsHeader.PointDataFieldDescr);
              hawkeye[i].ContentsHeader.PointDataFieldDescr = NULL;
            }

          if (hawkeye[i].ContentsHeader.PointDataFieldTags)
            {
              for (j = 0 ; j < hawkeye[i].ContentsHeader.NbrOfPointDataFields ; j++) free (hawkeye[i].ContentsHeader.PointDataFieldTags[j]);
              free (hawkeye[i].ContentsHeader.PointDataFieldTags);
              hawkeye[i].ContentsHeader.PointDataFieldTags = NULL;
            }

          if (hawkeye[i].ContentsHeader.PointDataMax) free (hawkeye[i].ContentsHeader.PointDataMax);
          if (hawkeye[i].ContentsHeader.PointDataMin) free (hawkeye[i].ContentsHeader.PointDataMin);
          if (hawkeye[i].ContentsHeader.PointDataFieldTypes) free (hawkeye[i].ContentsHeader.PointDataFieldTypes);
          if (hawkeye[i].point_record_buffer) free (hawkeye[i].point_record_buffer);

          fclose (hawkeye[i].fp);
        }
    }


  /*  Return to the SIGINT handler.  */

  if (ret == -999 && getpid () > 1) return;


  exit (ret);
}



/*********************************************************************************************

  Function:    sigint_handler

  Purpose:     Simple little SIGINT handler.  Allows us to clean up the files if someone does a CTRL-C.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   sig            -    The signal

  Returns:     N/A

  Caveats:     The way to do this was borrowed from "Proper handling of SIGINT/SIGQUIT",
               http://www.cons.org/cracauer/sigint.html

*********************************************************************************************/

static void sigint_handler (int sig)
{
  clean_exit (-999);

  signal (SIGINT, SIG_DFL);

#ifdef NVWIN3X
  raise (sig);
#else
  kill (getpid (), SIGINT);
#endif
}



/*********************************************************************************************

  Function:    pack_point_record

  Purpose:     Pack the byte packed HAWKEYE record into the byte buffer (I don't like using
               pragmas).

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The HAWKEYE file handle
               buffer         -    Unsigned character buffer (data will be packed into this buffer)
               rec            -    The HAWKEYE point record that holds the data

  Returns:     N/A

*********************************************************************************************/

static void pack_point_record (NV_INT32 hnd, NV_U_BYTE *buffer, HAWKEYE_RECORD rec)
{
  NV_INT32 i;


  /*  Populate the internal hawkeye record with data from the record passed in.  The reason we do this is that the rec_ptr pointers in 
      the internal hawkeye[hnd] structure point to the internal hawkeye record.  */

  memcpy (&internal_hawkeye_record, &rec, sizeof (HAWKEYE_RECORD));


  ptr = buffer;


  for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++)
    {
      switch (hawkeye[hnd].ContentsHeader.PointDataFieldTypes[i])
        {
        case HawkeyeCSSOutCHAR:
          data.c = *((NV_CHAR *) hawkeye[hnd].rec_ptr[i]);
          memcpy (ptr, data.buff, 1);
          ptr += 1;
          break;

        case HawkeyeCSSOutBOOL:
          data.b = *((NV_BOOL *) hawkeye[hnd].rec_ptr[i]);
          memcpy (ptr, data.buff, 1);
          ptr += 1;
          break;

        case HawkeyeCSSOutFLOAT32:
          data.f = *((NV_FLOAT32 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT32 (&data.f);
          memcpy (ptr, data.buff, 4);
          ptr += 4;
          break;

        case HawkeyeCSSOutFLOAT64:
          data.d = *((NV_FLOAT64 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
          memcpy (ptr, data.buff, 8);
          ptr += 8;
          break;

        case HawkeyeCSSOutINT16:
          data.s = *((NV_INT16 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 (&data.s);
          memcpy (ptr, data.buff, 2);
          ptr += 2;
          break;

        case HawkeyeCSSOutINT32:
          data.i = *((NV_INT32 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 (&data.i);
          memcpy (ptr, data.buff, 4);
          ptr += 4;
          break;

        case HawkeyeCSSOutUINT16:
          data.us = *((NV_U_INT16 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 ((NV_INT16 *) &data.us);
          memcpy (ptr, data.buff, 2);
          ptr += 2;
          break;

        case HawkeyeCSSOutUINT32:
          data.ui = *((NV_U_INT32 *) hawkeye[hnd].rec_ptr[i]);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
          memcpy (ptr, data.buff, 4);
          ptr += 4;
          break;

        default:
          break;
        }
    }
}



/*********************************************************************************************

  Function:    unpack_point_record

  Purpose:     Unpack the byte packed HAWKEYE record from the byte buffer.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The HAWKEYE file handle
               buffer         -    Unsigned character buffer
               rec            -    The HAWKEYE point record to receive the data

  Returns:     N/A

*********************************************************************************************/

static void unpack_point_record (NV_INT32 hnd, NV_U_BYTE *buffer, HAWKEYE_RECORD *rec)
{
  NV_INT32 i;


  /*  Zero out the internal record (that hawkeye[hnd].rec_ptr points to parts of).  */

  memset (&internal_hawkeye_record, 0, sizeof (HAWKEYE_RECORD));


  /*  Populate the parts of the record for which we have data.  */

  ptr = buffer;

  for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++)
    {
      switch (hawkeye[hnd].ContentsHeader.PointDataFieldTypes[i])
        {
        case HawkeyeCSSOutCHAR:
          memcpy (data.buff, ptr, 1);
          *((NV_CHAR *) hawkeye[hnd].rec_ptr[i]) = data.c;
          ptr += 1;
          break;

        case HawkeyeCSSOutBOOL:
          memcpy (data.buff, ptr, 1);
          *((NV_BOOL *) hawkeye[hnd].rec_ptr[i]) = data.b;
          ptr += 1;
          break;

        case HawkeyeCSSOutFLOAT32:
          memcpy (data.buff, ptr, 4);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT32 (&data.f);
          *((NV_FLOAT32 *) hawkeye[hnd].rec_ptr[i]) = data.f;
          ptr += 4;
          break;

        case HawkeyeCSSOutFLOAT64:
          memcpy (data.buff, ptr, 8);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
          *((NV_FLOAT64 *) hawkeye[hnd].rec_ptr[i]) = data.d;
          ptr += 8;
          break;

        case HawkeyeCSSOutINT16:
          memcpy (data.buff, ptr, 2);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 (&data.s);
          *((NV_INT16 *) hawkeye[hnd].rec_ptr[i]) = data.s;
          ptr += 2;
          break;

        case HawkeyeCSSOutINT32:
          memcpy (data.buff, ptr, 4);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 (&data.i);
          *((NV_INT32 *) hawkeye[hnd].rec_ptr[i]) = data.i;
          ptr += 4;
          break;

        case HawkeyeCSSOutUINT16:
          memcpy (data.buff, ptr, 2);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 ((NV_INT16 *) &data.us);
          *((NV_U_INT16 *) hawkeye[hnd].rec_ptr[i]) = data.us;
          ptr += 2;
          break;

        case HawkeyeCSSOutUINT32:
          memcpy (data.buff, ptr, 4);
          if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
          *((NV_U_INT32 *) hawkeye[hnd].rec_ptr[i]) = data.ui;
          ptr += 4;
          break;

        default:
          break;
        }
    }

  memcpy (rec, &internal_hawkeye_record, sizeof (HAWKEYE_RECORD));
}



/*********************************************************************************************

  Function:    unpack_sfm_record

  Purpose:     Unpack the byte packed HAWKEYE SFM record structure.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/15/11

  Arguments:   hnd            -    The HAWKEYE file handle
               buffer         -    Unsigned character buffer with packed data
               rec            -    The SFM record structure to receive the data

  Returns:     N/A

*********************************************************************************************/

static void unpack_sfm_record (NV_INT32 hnd, NV_U_BYTE *buffer, HAWKEYE_SFM_RECORD *rec)
{
  ptr = buffer;

  memcpy (data.buff, ptr, 8);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
  rec->Timestamp = data.d;
  ptr += 8;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 (&data.i);
  rec->FilePosIntermediate = data.i;
  ptr += 4;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 (&data.i);
  rec->FilePosOriginal = data.i;
  ptr += 4;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 (&data.i);
  rec->FileIndex = data.i;
  ptr += 4;
}



/*********************************************************************************************

  Function:    unpack_common_header

  Purpose:     Unpack the byte packed HAWKEYE common header structure of the shot data hydro
               record.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/15/11

  Arguments:   hnd            -    The HAWKEYE file handle
               buffer         -    Unsigned character buffer with packed data
               rec            -    The common header record structure to receive the data

  Returns:     N/A

*********************************************************************************************/

static void unpack_common_header (NV_INT32 hnd, NV_U_BYTE *buffer, HAWKEYE_COMMON_HEADER *rec)
{
  ptr = buffer;

  memcpy (data.buff, ptr, 2);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 ((NV_INT16 *) &data.us);
  rec->msgId = data.s;
  ptr += 2;

  memcpy (data.buff, ptr, 2);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT16 ((NV_INT16 *) &data.us);
  rec->msgSize = data.s;
  ptr += 2;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
  rec->senderSeqMsgId = data.ui;
  ptr += 4;
}



/*********************************************************************************************

  Function:    unpack_shot_data_hydro

  Purpose:     Unpack the byte packed HAWKEYE shot data hydro structure of the shot data hydro
               record (minus the waveforms).

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/15/11

  Arguments:   hnd            -    The HAWKEYE file handle
               buffer         -    Unsigned character buffer with packed data
               rec            -    The shot data hydro record structure to receive the data

  Returns:     N/A

*********************************************************************************************/

static void unpack_shot_data_hydro (NV_INT32 hnd, NV_U_BYTE *buffer, HAWKEYE_SHOT_DATA_HYDRO *rec)
{
  NV_INT32 i;


  ptr = buffer;

  memcpy (data.buff, ptr, 8);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
  rec->pcTimestamp = data.d;
  ptr += 8;

  memcpy (data.buff, ptr, 8);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
  rec->Timestamp = data.d;
  ptr += 8;

  memcpy (data.buff, ptr, 8);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT64 (&data.d);
  rec->scannerTimestamp = data.d;
  ptr += 8;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
  rec->seqShotID = data.ui;
  ptr += 4;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT32 (&data.f);
  rec->scannerXaxisAngle = data.f;
  ptr += 4;

  memcpy (data.buff, ptr, 4);
  if (hawkeye[hnd].swap) hawkeye_swap_NV_FLOAT32 (&data.f);
  rec->scannerYaxisAngle = data.f;
  ptr += 4;

  for (i = 0 ; i < hydroChannelID_nrOfChannels ; i++)
    {
      memcpy (&rec->channelInfo[i].dataValid, ptr, 1);
      ptr += 1;

      memcpy (&rec->channelInfo[i].waveformFormat, ptr, 1);
      ptr += 1;

      memcpy (&rec->channelInfo[i].nbrBytesInOneSample, ptr, 1);
      ptr += 1;

      memcpy (&rec->channelInfo[i].padding, ptr, 1);
      ptr += 1;

      memcpy (data.buff, ptr, 4);
      if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
      rec->channelInfo[i].pulseLength = data.ui;
      ptr += 4;

      memcpy (data.buff, ptr, 4);
      if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
      rec->channelInfo[i].reflexLength = data.ui;
      ptr += 4;

      memcpy (data.buff, ptr, 4);
      if (hawkeye[hnd].swap) hawkeye_swap_NV_INT32 ((NV_INT32 *) &data.ui);
      rec->channelInfo[i].intermissionLength = data.ui;
      ptr += 4;
    }
}



/*********************************************************************************************

  Function:    hawkeye_read_header

  Purpose:     Read the Hawkeye meta and contents headers.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The HAWKEYE file handle

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_META_HEADER_READ_FSEEK_ERROR
                                   HAWKEYE_CONTENTS_HEADER_READ_FSEEK_ERROR
                                   HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR

*********************************************************************************************/

static NV_INT32 hawkeye_read_header (NV_INT32 hnd)
{
  NV_CHAR dummy[2], field_tag[HAWKEYE_CSSOUT_DATAFIELD_TAG_LENGTH], descr[HAWKEYE_CSSOUT_DATAFIELD_DESCR_LENGTH];
  NV_INT32 i;


  if (fseeko64 (hawkeye[hnd].fp, 0LL, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_META_HEADER_READ_FSEEK_ERROR);
    }

  fread (hawkeye[hnd].MetaHeader.FileSignature, 6, 1, hawkeye[hnd].fp);


  /*  Check the file signature to see if this is a HAWKEYE file.  */

  if (strncmp (hawkeye[hnd].MetaHeader.FileSignature, "CSSOUT", 6))
    {
      hawkeye_error.system = 0;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_NOT_HAWKEYE_FILE_ERROR);
    }


  /*  The META header.  */

  fread (dummy, 2, 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].MetaHeader.VersionMajor, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].MetaHeader.VersionMinor, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].MetaHeader.MetaHeaderSize, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].MetaHeader.ContentsHeaderSize, sizeof (NV_INT32), 1, hawkeye[hnd].fp);

  if (hawkeye[hnd].swap)
    {
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].MetaHeader.VersionMajor);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].MetaHeader.VersionMinor);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].MetaHeader.MetaHeaderSize);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].MetaHeader.ContentsHeaderSize);
    }

  if (fseeko64 (hawkeye[hnd].fp, hawkeye[hnd].MetaHeader.MetaHeaderSize, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_READ_FSEEK_ERROR);
    }


  /*  The Contents header.  */

  fread (hawkeye[hnd].ContentsHeader.GeneratingSoftware, 32, 1, hawkeye[hnd].fp);
  fread (hawkeye[hnd].ContentsHeader.SystemIdentifier, 32, 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.NbrOfPointDataFields, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.NbrOfPointRecords, sizeof (NV_U_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.OffsetToPointData, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.PointRecordSize, sizeof (NV_INT32), 1, hawkeye[hnd].fp);

  if (hawkeye[hnd].swap)
    {
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].ContentsHeader.NbrOfPointDataFields);
      hawkeye_swap_NV_INT32 ((NV_INT32 *) &hawkeye[hnd].ContentsHeader.NbrOfPointRecords);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].ContentsHeader.OffsetToPointData);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].ContentsHeader.PointRecordSize);
    }


  hawkeye[hnd].ContentsHeader.PointDataFieldDescr = (NV_CHAR **) calloc (hawkeye[hnd].ContentsHeader.NbrOfPointDataFields, sizeof (NV_CHAR *));

  if (hawkeye[hnd].ContentsHeader.PointDataFieldDescr == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
    }


  hawkeye[hnd].ContentsHeader.PointDataFieldTags = (NV_CHAR **) calloc (hawkeye[hnd].ContentsHeader.NbrOfPointDataFields, sizeof (NV_CHAR *));

  if (hawkeye[hnd].ContentsHeader.PointDataFieldTags == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
    }

  hawkeye[hnd].ContentsHeader.PointDataMax = (NV_U_BYTE *) calloc (hawkeye[hnd].ContentsHeader.PointRecordSize, 1);
  if (hawkeye[hnd].ContentsHeader.PointDataMax == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
    }

  hawkeye[hnd].ContentsHeader.PointDataMin = (NV_U_BYTE *) calloc (hawkeye[hnd].ContentsHeader.PointRecordSize, 1);
  if (hawkeye[hnd].ContentsHeader.PointDataMin == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
    }

  hawkeye[hnd].ContentsHeader.PointDataFieldTypes = (E_HawkeyeCSSOutputDataFieldTypes *) calloc (hawkeye[hnd].ContentsHeader.NbrOfPointDataFields,
                                                                                                 sizeof (E_HawkeyeCSSOutputDataFieldTypes));
  if (hawkeye[hnd].ContentsHeader.PointDataFieldTypes == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
    }


  /*  Field descriptors, field types, and field tags.  The data can be stored in any order.  This is where we use hawkeye_parse_tags
      to point the parts of the record structure to the correct place.  */

  for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++)
    {
      fread (descr, HAWKEYE_CSSOUT_DATAFIELD_DESCR_LENGTH, 1, hawkeye[hnd].fp);
      
      hawkeye[hnd].ContentsHeader.PointDataFieldDescr[i] = (NV_CHAR *) malloc (strlen (descr) + 1);

      if (hawkeye[hnd].ContentsHeader.PointDataFieldDescr[i] == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].path);
          return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
        }

      strcpy (hawkeye[hnd].ContentsHeader.PointDataFieldDescr[i], descr);
    }


  for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++)
    {
      fread (field_tag, 1, HAWKEYE_CSSOUT_DATAFIELD_TAG_LENGTH, hawkeye[hnd].fp);

      hawkeye[hnd].ContentsHeader.PointDataFieldTags[i] = (NV_CHAR *) malloc (strlen (field_tag) + 1);

      if (hawkeye[hnd].ContentsHeader.PointDataFieldTags[i] == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].path);
          return (hawkeye_error.hawkeye = HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR);
        }

      strcpy (hawkeye[hnd].ContentsHeader.PointDataFieldTags[i], field_tag);
    }

  fread (hawkeye[hnd].ContentsHeader.PointDataMax, hawkeye[hnd].ContentsHeader.PointRecordSize, 1, hawkeye[hnd].fp);
  fread (hawkeye[hnd].ContentsHeader.PointDataMin, hawkeye[hnd].ContentsHeader.PointRecordSize, 1, hawkeye[hnd].fp);
  fread (hawkeye[hnd].ContentsHeader.PointDataFieldTypes, hawkeye[hnd].ContentsHeader.NbrOfPointDataFields, sizeof (E_HawkeyeCSSOutputDataFieldTypes),
         hawkeye[hnd].fp);


  for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++) hawkeye_parse_tags (hnd, i, hawkeye[hnd].ContentsHeader.PointDataFieldTags[i]);


  /*  Now that we know what field is where we can unpack the min and max values per field from the Contents header's
      min and max point fields.  */

  unpack_point_record (hnd, hawkeye[hnd].ContentsHeader.PointDataMax, &hawkeye[hnd].ContentsHeader.MaxValues);
  unpack_point_record (hnd, hawkeye[hnd].ContentsHeader.PointDataMin, &hawkeye[hnd].ContentsHeader.MinValues);
  

  fread (hawkeye[hnd].ContentsHeader.SoftwareVersion, 16, 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.SurveyYear, sizeof (NV_U_INT16), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.SurveyDay, sizeof (NV_U_INT16), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.ProcessingYear, sizeof (NV_U_INT16), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.ProcessingDay, sizeof (NV_U_INT16), 1, hawkeye[hnd].fp);
  fread (hawkeye[hnd].ContentsHeader.UTMDatum, 16, 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.UTMZone, sizeof (NV_U_INT16), 1, hawkeye[hnd].fp);
  fread (&hawkeye[hnd].ContentsHeader.UTMSouth, sizeof (NV_INT32), 1, hawkeye[hnd].fp);
  fread (hawkeye[hnd].ContentsHeader.UTMUnits, 16, 1, hawkeye[hnd].fp);

  if (hawkeye[hnd].swap)
    {
      hawkeye_swap_NV_INT16 ((NV_INT16 *) &hawkeye[hnd].ContentsHeader.SurveyYear);
      hawkeye_swap_NV_INT16 ((NV_INT16 *) &hawkeye[hnd].ContentsHeader.SurveyDay);
      hawkeye_swap_NV_INT16 ((NV_INT16 *) &hawkeye[hnd].ContentsHeader.ProcessingYear);
      hawkeye_swap_NV_INT16 ((NV_INT16 *) &hawkeye[hnd].ContentsHeader.ProcessingDay);
      hawkeye_swap_NV_INT16 ((NV_INT16 *) &hawkeye[hnd].ContentsHeader.UTMZone);
      hawkeye_swap_NV_INT32 (&hawkeye[hnd].ContentsHeader.UTMSouth);
    }


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

    API FUNCTIONS (public)

*********************************************************************************************/


/*********************************************************************************************

  Function:    hawkeye_open_file

  Purpose:     Open a HAWKEYE file.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   path           -    The HAWKEYE file path
               MetaHeader     -    Pointer to HAWKEYE_META_HEADER
               ContentsHeader -    Pointer to HAWKEYE_CONTENTS_HEADER
               mode           -    HAWKEYE_UPDATE or HAWKEYE_READ_ONLY

  Returns:     NV_INT32       -    The file handle (0 or positive) or
                                   HAWKEYE_TOO_MANY_OPEN_FILES
                                   HAWKEYE_NOT_HAWKEYE_FILE_ERROR
                                   HAWKEYE_OPEN_UPDATE_ERROR
                                   HAWKEYE_OPEN_READONLY_ERROR
                                   HAWKEYE_POINT_RECORD_BUFFER_CALLOC_ERROR
                                   HAWKEYE_META_HEADER_WRONG_VERSION_ERROR

  Caveats:     Since the MetaHeader and the ContentsHeader are allocated by this function
               they are freed when the file is closed.  Don't expect them to be there
               after you close a file.

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_open_file (const NV_CHAR *path, HAWKEYE_META_HEADER **MetaHeader, HAWKEYE_CONTENTS_HEADER **ContentsHeader, NV_INT32 mode)
{
  NV_INT32               i, hnd, month, day;
  NV_CHAR                fl_dir[512], *fl_ptr, dat_dir[512];
  time_t                 tv_sec;
  NV_INT32               tv_nsec;
  struct tm              tm;


  /*  The first time through we want to initialize (zero) the hawkeye handle array.  */

  if (first)
    {
      for (i = 0 ; i < HAWKEYE_MAX_FILES ; i++) 
        {
          memset (&hawkeye[i], 0, sizeof (INTERNAL_HAWKEYE_STRUCT));
          hawkeye[i].fp = NULL;
          hawkeye[i].sfm_fp = NULL;
          hawkeye[i].dat_fp = NULL;
          hawkeye[i].previous_dat_file_index = -1;
        }


      /*  Set up the SIGINT handler.  */

      signal (SIGINT, sigint_handler);


      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = HAWKEYE_MAX_FILES;
  for (i = 0 ; i < HAWKEYE_MAX_FILES ; i++)
    {
      if (hawkeye[i].fp == NULL)
        {
          hnd = i;
          hawkeye[i].sfm_fp = NULL;
          hawkeye[i].dat_fp = NULL;
          hawkeye[i].previous_dat_file_index = -1;
          break;
        }
    }


  if (hnd == HAWKEYE_MAX_FILES) return (hawkeye_error.hawkeye = HAWKEYE_TOO_MANY_OPEN_FILES);


  /*  Determine if this is hydro or topo.  */

  if (strstr (path, "_HD.bin"))
    {
      hawkeye[hnd].ContentsHeader.data_type = HAWKEYE_HYDRO_BIN_DATA;
    }
  else if (strstr (path, "_TD.bin"))
    {
      hawkeye[hnd].ContentsHeader.data_type = HAWKEYE_TOPO_BIN_DATA;
    }
  else
    {
      hawkeye_error.system = 0;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_NOT_HAWKEYE_FILE_ERROR);
    }


  /*  Check for a big endian system.  */

  hawkeye[hnd].swap = (NV_BOOL) hawkeye_big_endian ();


  /*  Internal structs are zeroed above and on close of file so we don't have to do it here.  */


  /*  Build the sfm file name and the (almost) dat file name from the bin file name.  We won't open these unless
      the waveforms are requested.  This is a massive PITA.  We have to parse the sfm, and dat file names from
      the bin filename.  The layout of the files is such that the bin and sfm files are in a directory under
      the "Output" directory while the dat file is in a "Flightline" directory at the same level as the "Output"
      directory.  Here are three example files:

      \20100410_133204_Brittany\Output\2010-04-13 13.41.51\20100410_133204_Brittany_FL000_3218176_Flightline 0_HD.bin
      \20100410_133204_Brittany\Output\2010-04-13 13.41.51\FL000_3218176_Flightline 0_HD.sfm
      \20100410_133204_Brittany\FL000_3218176_Flightline 0\HD_FL000_3218176_Flightline 0_000.dat

      Note the spaces (DOH!).  Note that the flightline number can be one, two, or three digits (DOH!!!).  Why don't
      people think of these things when they define a file heirarchy?  Now I have to parse the "Flightline" directory
      name out of the bin file name.  I'll do this by searching backward for "_Flightline " in the bin file name 
      and then moving back 13 characters to find the "FL".  Note that we build the entire dat file name but hardwire
      the index number to 000 so that all we have to do when we want to open it is to replace the number with the index
      value from the SFM record.  */

  strcpy (fl_dir, hawkeye_gen_dirname (path));

  fl_ptr = (NV_CHAR *) (hawkeye_gen_strrstr (path, "_Flightline ") - 13);
  sprintf (hawkeye[hnd].sfm_path, "%s%1c%s", fl_dir, SEPARATOR, fl_ptr);
  strcpy (&hawkeye[hnd].sfm_path[strlen (hawkeye[hnd].sfm_path) - 4], ".sfm");

  sprintf (dat_dir, "%s%1c..%1c..%1c%s", fl_dir, SEPARATOR, SEPARATOR, SEPARATOR, fl_ptr);
  dat_dir[strlen (dat_dir) - 7] = 0;

  if (hawkeye[hnd].ContentsHeader.data_type == HAWKEYE_HYDRO_BIN_DATA)
    {
      sprintf (hawkeye[hnd].dat_path, "%s%1cHD_%s", dat_dir, SEPARATOR, fl_ptr);
    }
  else
    {
      sprintf (hawkeye[hnd].dat_path, "%s%1cTD_%s", dat_dir, SEPARATOR, fl_ptr);
    }
  strcpy (&hawkeye[hnd].dat_path[strlen (hawkeye[hnd].dat_path) - 6], "000.dat");


  /*  Open the file and read the header.  */

  switch (mode)
    {
    case HAWKEYE_UPDATE:
      if ((hawkeye[hnd].fp = fopen64 (path, "rb+")) == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].path);
          return (hawkeye_error.hawkeye = HAWKEYE_OPEN_UPDATE_ERROR);
        }
      break;

    case HAWKEYE_READONLY:
      if ((hawkeye[hnd].fp = fopen64 (path, "rb")) == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].path);
          return (hawkeye_error.hawkeye = HAWKEYE_OPEN_READONLY_ERROR);
        }
      break;
    }


  if (hawkeye_read_header (hnd)) return (hawkeye_error.hawkeye);


  *MetaHeader = &hawkeye[hnd].MetaHeader;
  *ContentsHeader = &hawkeye[hnd].ContentsHeader;


  /*  Save the file name for error messages.  */

  strcpy (hawkeye[hnd].path, path);


  hawkeye[hnd].point_record_buffer = (NV_U_BYTE *) calloc (hawkeye[hnd].ContentsHeader.PointRecordSize, 1);
  if (hawkeye[hnd].point_record_buffer == NULL)
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_POINT_RECORD_BUFFER_CALLOC_ERROR);
    }


  /*  Check the version.  */

  if (hawkeye[hnd].MetaHeader.VersionMajor != 1 || hawkeye[hnd].MetaHeader.VersionMinor != 3)
    {
      hawkeye_error.system = 0;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_META_HEADER_WRONG_VERSION_ERROR);
    }


  /*  Compute the start_week seconds from 01/01/1970.  tm struct wants years since 1900!!!  */

  hawkeye_jday2mday (hawkeye[hnd].ContentsHeader.SurveyYear, hawkeye[hnd].ContentsHeader.SurveyDay, &month, &day);

  tm.tm_year = hawkeye[hnd].ContentsHeader.SurveyYear - 1900;
  tm.tm_mon = month;
  tm.tm_mday = day;
  tm.tm_hour = 0.0;
  tm.tm_min = 0.0;
  tm.tm_sec = 0.0;
  tm.tm_isdst = -1;

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
      putenv("TZ=GMT");
      tzset();
#endif
      tz_set = NVTrue;
    }


  /*  Get seconds from the epoch (01-01-1970) for the date in the header.  This will also give us the day of the week for the GPS seconds of
      week calculation.  */

  tv_sec = mktime (&tm);
  tv_nsec = 0.0;


  /*  Subtract the number of days since Saturday midnight (Sunday morning) in seconds.  */

  tv_sec = tv_sec - (tm.tm_wday * 86400);
  hawkeye[hnd].start_week = tv_sec;


  hawkeye_read_record (hnd, 0, &internal_hawkeye_record);
  hawkeye[hnd].start_gps_time = internal_hawkeye_record.Timestamp;


  hawkeye_error.system = 0;
  return (hnd);
}


/*********************************************************************************************

  Function:    hawkeye_close_file

  Purpose:     Close a HAWKEYE file, SFM file, and DAT file (if open) and free memory where
               needed.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The file handle

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_CLOSE_ERROR

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_close_file (NV_INT32 hnd)
{
  NV_INT32 i;


  /*  Just in case someone tries to close a file more than once... */

  if (hawkeye[hnd].fp == NULL) return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);


  /*  Close the bin file.  */

  if (fclose (hawkeye[hnd].fp))
    {
      hawkeye_error.system = errno;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_CLOSE_ERROR);
    }
  hawkeye[hnd].fp = NULL;


  /*  Close the sfm file (if needed).  */

  if (hawkeye[hnd].sfm_fp != NULL)
    {
      if (fclose (hawkeye[hnd].sfm_fp))
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
          return (hawkeye_error.hawkeye = HAWKEYE_CLOSE_ERROR);
        }

      hawkeye[hnd].sfm_fp = NULL;
    }


  /*  Close the dat file (if needed).  */

  if (hawkeye[hnd].dat_fp != NULL)
    {
      if (fclose (hawkeye[hnd].dat_fp))
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
          return (hawkeye_error.hawkeye = HAWKEYE_CLOSE_ERROR);
        }

      hawkeye[hnd].dat_fp = NULL;
      hawkeye[hnd].previous_dat_file_index = -1;
    }


  /*  Free allocated memory.  */

  if (hawkeye[hnd].ContentsHeader.PointDataFieldDescr)
    {
      for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++) free (hawkeye[hnd].ContentsHeader.PointDataFieldDescr[i]);
      free (hawkeye[hnd].ContentsHeader.PointDataFieldDescr);
      hawkeye[hnd].ContentsHeader.PointDataFieldDescr = NULL;
    }

  if (hawkeye[hnd].ContentsHeader.PointDataFieldTags)
    {
      for (i = 0 ; i < hawkeye[hnd].ContentsHeader.NbrOfPointDataFields ; i++) free (hawkeye[hnd].ContentsHeader.PointDataFieldTags[i]);
      free (hawkeye[hnd].ContentsHeader.PointDataFieldTags);
      hawkeye[hnd].ContentsHeader.PointDataFieldTags = NULL;
    }

  if (hawkeye[hnd].ContentsHeader.PointDataMax) free (hawkeye[hnd].ContentsHeader.PointDataMax);
  if (hawkeye[hnd].ContentsHeader.PointDataMin) free (hawkeye[hnd].ContentsHeader.PointDataMin);
  if (hawkeye[hnd].ContentsHeader.PointDataFieldTypes) free (hawkeye[hnd].ContentsHeader.PointDataFieldTypes);
  if (hawkeye[hnd].point_record_buffer) free (hawkeye[hnd].point_record_buffer);


  /*  Clear the internal structure.  */

  memset (&hawkeye[hnd], 0, sizeof (INTERNAL_HAWKEYE_STRUCT));
  hawkeye[hnd].fp = NULL;


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

  Function:    hawkeye_read_record

  Purpose:     Retrieve a HAWKEYE point record.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The file handle
               recnum         -    The record number of the HAWKEYE record to be retrieved
               hawkeye_record -    The returned HAWKEYE record

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_INVALID_RECORD_NUMBER_ERROR
                                   HAWKEYE_READ_FSEEK_ERROR
                                   HAWKEYE_READ_ERROR

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_read_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_RECORD *hawkeye_record)
{
  NV_INT64 pos;


  /*  Check for record out of bounds.  */

  if (recnum >= hawkeye[hnd].ContentsHeader.NbrOfPointRecords || recnum < 0)
    {
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_INVALID_RECORD_NUMBER_ERROR);
    }


  /*  Compute the position of the record within the file.  */

  pos = (NV_INT64) recnum * (NV_INT64) hawkeye[hnd].ContentsHeader.PointRecordSize + (NV_INT64) hawkeye[hnd].ContentsHeader.OffsetToPointData;


  if (fseeko64 (hawkeye[hnd].fp, pos, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_READ_FSEEK_ERROR);
    }

  if (!fread (hawkeye[hnd].point_record_buffer, hawkeye[hnd].ContentsHeader.PointRecordSize, 1, hawkeye[hnd].fp))
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_READ_ERROR);
    }


  /*  Unpack the record.  */

  unpack_point_record (hnd, hawkeye[hnd].point_record_buffer, hawkeye_record);


  /*  Compute the fully qualified time using the starting gps time and the gps time of this record.  */

  if (hawkeye_record->Timestamp < hawkeye[hnd].start_gps_time)
    {
      hawkeye_record->tv_sec = (time_t) (hawkeye[hnd].start_week + WEEK_OFFSET + hawkeye_record->Timestamp);
      hawkeye_record->tv_nsec = (long) (fmod (hawkeye[hnd].start_week + WEEK_OFFSET + hawkeye_record->Timestamp, 1.0) * 1000000000.0);
    }
  else
    {
      hawkeye_record->tv_sec = (time_t) (hawkeye[hnd].start_week + hawkeye_record->Timestamp);
      hawkeye_record->tv_nsec = (long) (fmod (hawkeye[hnd].start_week + hawkeye_record->Timestamp, 1.0) * 1000000000.0);
    }


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

  Function:    hawkeye_update_record

  Purpose:     Update a Hawkeye point record.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The file handle
               recnum         -    The record number
               hawkeye_record -    The HAWKEYE_RECORD structure to save.

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_INVALID_RECORD_NUMBER_ERROR
                                   HAWKEYE_UPDATE_RECORD_FSEEK_ERROR
                                   HAWKEYE_UPDATE_RECORD_WRITE_ERROR

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_update_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_RECORD hawkeye_record)
{
  NV_INT64 pos;


  /*  Check for record out of bounds.  */

  if (recnum >= hawkeye[hnd].ContentsHeader.NbrOfPointRecords || recnum < 0)
    {
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_INVALID_RECORD_NUMBER_ERROR);
    }


  /*  Pack the record.  */

  pack_point_record (hnd, hawkeye[hnd].point_record_buffer, hawkeye_record);


  pos = (NV_INT64) recnum * (NV_INT64) hawkeye[hnd].ContentsHeader.PointRecordSize + (NV_INT64) hawkeye[hnd].ContentsHeader.OffsetToPointData;

  if (fseeko64 (hawkeye[hnd].fp, pos, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_UPDATE_RECORD_FSEEK_ERROR);
    }


  if (!fwrite (hawkeye[hnd].point_record_buffer, hawkeye[hnd].ContentsHeader.PointRecordSize, 1, hawkeye[hnd].fp))
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].path);
      return (hawkeye_error.hawkeye = HAWKEYE_UPDATE_RECORD_WRITE_ERROR);
    }


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

  Function:    hawkeye_read_sfm_record

  Purpose:     Retrieve a HAWKEYE sfm waveform index record.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/15/11

  Arguments:   hnd            -    The file handle
               recnum         -    The record number of the HAWKEYE record to be retrieved
               sfm_record     -    The returned sfm index record

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_INVALID_RECORD_NUMBER_ERROR
                                   HAWKEYE_SFM_FILE_OPEN_ERROR
                                   HAWKEYE_SFM_READ_FSEEK_ERROR
                                   HAWKEYE_SFM_READ_ERROR

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_read_sfm_record (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_SFM_RECORD *hawkeye_sfm_record)
{
  static NV_INT32 prev_pos = -1;
  static HAWKEYE_SFM_RECORD last_sfm_record;
  NV_INT32 pos, save_error;
  NV_U_BYTE sfm_record_buffer[HAWKEYE_SFM_RECORD_SIZE];


  /*  Check for record out of bounds.  */

  if (recnum >= hawkeye[hnd].ContentsHeader.NbrOfPointRecords || recnum < 0)
    {
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
      return (hawkeye_error.hawkeye = HAWKEYE_INVALID_RECORD_NUMBER_ERROR);
    }


  /*  Check to see if the SFM file has been opened.  */

  if (hawkeye[hnd].sfm_fp == NULL)
    {
      if ((hawkeye[hnd].sfm_fp = fopen (hawkeye[hnd].sfm_path, "rb")) == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
          return (hawkeye_error.hawkeye = HAWKEYE_SFM_FILE_OPEN_ERROR);
        }

      prev_pos = -1;
    }


  /*  Divide the record number by 4 since there are four returns/pixels per shot.  Size of SFM header is always 20 bytes.  */

  pos = (recnum / 4) * HAWKEYE_SFM_RECORD_SIZE + 20;


  /*  Don't reread the sfm record if we already read it (four bin records to each sfm/dat record).  */

  if (pos == prev_pos)
    {
      memcpy (hawkeye_sfm_record, &last_sfm_record, sizeof (HAWKEYE_SFM_RECORD));
      hawkeye_error.system = 0;
      return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
    }


  /*  Save the previous record position.  */

  prev_pos = pos;

  if (fseek (hawkeye[hnd].sfm_fp, pos, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
      return (hawkeye_error.hawkeye = HAWKEYE_SFM_READ_FSEEK_ERROR);
    }

  if (!fread (sfm_record_buffer, HAWKEYE_SFM_RECORD_SIZE, 1, hawkeye[hnd].sfm_fp))
    {
      save_error = errno;

      if (feof (hawkeye[hnd].sfm_fp))
        {
          hawkeye_error.system = save_error;
          hawkeye_error.recnum = recnum;
          strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
          return (hawkeye_error.hawkeye = HAWKEYE_SFM_EOF_ERROR);
        }
      else
        {
          hawkeye_error.system = save_error;
          hawkeye_error.recnum = recnum;
          strcpy (hawkeye_error.file, hawkeye[hnd].sfm_path);
          return (hawkeye_error.hawkeye = HAWKEYE_SFM_READ_ERROR);
        }
    }


  /*  Unpack the SFM record.  */

  unpack_sfm_record (hnd, sfm_record_buffer, hawkeye_sfm_record);


  /*  Save the sfm record so we don't have to read and unpack it again if we ask for the same one.  */

  memcpy (&last_sfm_record, hawkeye_sfm_record, sizeof (HAWKEYE_SFM_RECORD));


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

  Function:    hawkeye_read_shot_data_hydro

  Purpose:     Retrieve a HAWKEYE shot data waveform record.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        02/15/11

  Arguments:   hnd            -    The file handle
               recnum         -    The record number of the HAWKEYE record to be retrieved
               dat_record     -    The returned dat index record

  Returns:     NV_INT32       -    HAWKEYE_SUCCESS
                                   HAWKEYE_INVALID_RECORD_NUMBER_ERROR
                                   HAWKEYE_DAT_FILE_OPEN_ERROR
                                   HAWKEYE_DAT_READ_FSEEK_ERROR
                                   HAWKEYE_DAT_READ_ERROR

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_read_shot_data_hydro (NV_INT32 hnd, NV_INT32 recnum, HAWKEYE_SHOT_DATA_HYDRO *hawkeye_shot_data_hydro)
{
  static NV_INT32 prev_pos = -1;
  static HAWKEYE_SHOT_DATA_HYDRO last_shot_data_hydro;
  NV_INT32 pos, ret, i, j, wave_size;
  HAWKEYE_SFM_RECORD sfm_record;
  HAWKEYE_COMMON_HEADER common_header;
  NV_U_BYTE waveforms[HAWKEYE_MAX_TOTAL_WAVEFORMS_BYTES];
  NV_U_BYTE common_header_buffer[HAWKEYE_COMMON_HEADER_SIZE];
  NV_U_BYTE shot_data_hydro_buffer[HAWKEYE_SHOT_DATA_HYDRO_SIZE];


  /*  Get the SFM record so we can find the waveform file and records.  */

  if ((ret = hawkeye_read_sfm_record (hnd, recnum, &sfm_record))) return (ret);


  /*  Check to see if we're switching dat file numbers.  If so, we need to close the last file (if needed) and open the
      new file.  */

  if (sfm_record.FileIndex != hawkeye[hnd].previous_dat_file_index)
    {
      if (hawkeye[hnd].dat_fp != NULL) fclose (hawkeye[hnd].dat_fp);

      sprintf (&hawkeye[hnd].dat_path[strlen (hawkeye[hnd].dat_path) - 7], "%03d.dat", sfm_record.FileIndex);

      if ((hawkeye[hnd].dat_fp = fopen (hawkeye[hnd].dat_path, "rb")) == NULL)
        {
          hawkeye_error.system = errno;
          strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
          return (hawkeye_error.hawkeye = HAWKEYE_DAT_FILE_OPEN_ERROR);
        }

      prev_pos = -1;
    }


  /*  Save the file index so we can tell if we need to close the last file and open a new one.  */

  hawkeye[hnd].previous_dat_file_index = sfm_record.FileIndex;


  /*  Move to the position of the waveform records in the dat file.  */

  pos = sfm_record.FilePosOriginal;


  /*  Don't reread the dat record if we already read it (four bin records to each sfm/dat record).  */

  if (pos == prev_pos)
    {
      memcpy (hawkeye_shot_data_hydro, &last_shot_data_hydro, sizeof (HAWKEYE_SHOT_DATA_HYDRO));
      hawkeye_error.system = 0;
      return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
    }

  /*  Save the previous record position.  */

  prev_pos = pos;


  if (fseek (hawkeye[hnd].dat_fp, pos, SEEK_SET) < 0)
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
      return (hawkeye_error.hawkeye = HAWKEYE_DAT_READ_FSEEK_ERROR);
    }


  /*  Read the common header.  */

  if (!fread (common_header_buffer, HAWKEYE_COMMON_HEADER_SIZE, 1, hawkeye[hnd].dat_fp))
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
      return (hawkeye_error.hawkeye = HAWKEYE_DAT_READ_HEADER_ERROR);
    }


  /*  Unpack the common header.  */

  unpack_common_header (hnd, common_header_buffer, &common_header);


  /*  Check the msgId.  */

  if (common_header.msgId != msgId_shotDataHydro && common_header.msgId != msgId_shotDataTopo)
    {
      hawkeye_error.system = 0;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
      return (hawkeye_error.hawkeye = HAWKEYE_NOT_SHOT_DATA_ERROR);
    }


  /*  Read the first part of the shot hydro data (minus the waveforms).  */

  if (!fread (shot_data_hydro_buffer, HAWKEYE_SHOT_DATA_HYDRO_SIZE, 1, hawkeye[hnd].dat_fp))
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
      return (hawkeye_error.hawkeye = HAWKEYE_DAT_READ_SHOT_ERROR);
    }


  /*  Unpack the shot data hydro record.  */

  unpack_shot_data_hydro (hnd, shot_data_hydro_buffer, hawkeye_shot_data_hydro);


  /*  Figure out how many bytes we need to read.  */

  wave_size = common_header.msgSize - (HAWKEYE_SHOT_DATA_HYDRO_SIZE + HAWKEYE_COMMON_HEADER_SIZE);


  /*  Read the waveforms as one big block of unsigned bytes.  */

  if (!fread (waveforms, wave_size, 1, hawkeye[hnd].dat_fp))
    {
      hawkeye_error.system = errno;
      hawkeye_error.recnum = recnum;
      strcpy (hawkeye_error.file, hawkeye[hnd].dat_path);
      return (hawkeye_error.hawkeye = HAWKEYE_DAT_READ_WAVE_ERROR);
    }


  /*  Now, break out the waveforms.  */

  j = 0;
  for (i = 0 ; i < hydroChannelID_nrOfChannels ; i++)
    {
      wave_size = hawkeye_shot_data_hydro->channelInfo[i].pulseLength;

      memcpy (hawkeye_shot_data_hydro->wave_pulse[i], &waveforms[j], wave_size);

      j += wave_size;

      wave_size = hawkeye_shot_data_hydro->channelInfo[i].reflexLength;

      memcpy (hawkeye_shot_data_hydro->wave_reflex[i], &waveforms[j], wave_size);

      j += wave_size;
    }


  /*  Save the shot hydro record so we don't have to read and unpack it again if we ask for the same one.  */

  memcpy (&last_shot_data_hydro, hawkeye_shot_data_hydro, sizeof (HAWKEYE_SHOT_DATA_HYDRO));


  hawkeye_error.system = 0;
  return (hawkeye_error.hawkeye = HAWKEYE_SUCCESS);
}



/*********************************************************************************************

  Function:    hawkeye_get_errno

  Purpose:     Returns the latest HAWKEYE error condition code

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   N/A

  Returns:     NV_INT32       -    error condition code

  Caveats:     The only thing this is good for at present is to determine if, when you opened
               the file, the library version was older than the file.  That is, if
               HAWKEYE_NEWER_FILE_VERSION_WARNING has been set when you called hawkeye_open_file.
               Otherwise, you can just use hawkeye_perror or hawkeye_strerror to get the last
               error information.

*********************************************************************************************/

HAWKEYE_DLL NV_INT32 hawkeye_get_errno ()
{
  return (hawkeye_error.hawkeye);
}



/*********************************************************************************************

  Function:    hawkeye_strerror

  Purpose:     Returns the error string related to the latest error.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   N/A

  Returns:     NV_CHAR       -    Error message

*********************************************************************************************/

HAWKEYE_DLL NV_CHAR *hawkeye_strerror ()
{
  static NV_CHAR message[1024];
  static NV_CHAR errno_message[512];

  if (hawkeye_error.system)
    {
      strcpy (errno_message, strerror (hawkeye_error.system));
    }
  else
    {
      strcpy (errno_message, _("Undefined system error"));
    }

  switch (hawkeye_error.hawkeye)
    {
    case HAWKEYE_SUCCESS:
      sprintf (message, _("\nSUCCESS!\n"));
      break;

    case HAWKEYE_META_HEADER_READ_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_META_HEADER_READ_FSEEK_ERROR\nFile : %s\nError during fseek prior to reading meta header :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_CONTENTS_HEADER_READ_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_CONTENTS_HEADER_READ_FSEEK_ERROR\nFile : %s\nError during fseek prior to reading contents header :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_CONTENTS_HEADER_CALLOC_ERROR:
      sprintf (message, _("\nHAWKEYE_CONTENTS_HEADER_CALLOC_ERROR\nFile : %s\nError allocating header memory :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_TOO_MANY_OPEN_FILES:
      sprintf (message, _("\nHAWKEYE_TOO_MANY_OPEN_FILES\nToo many HAWKEYE files are already open.\n"));
      break;

    case HAWKEYE_OPEN_UPDATE_ERROR:
      sprintf (message, _("\nHAWKEYE_OPEN_UPDATE_ERROR\nFile : %s\nError opening HAWKEYE file for update :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_OPEN_READONLY_ERROR:
      sprintf (message, _("\nHAWKEYE_OPEN_READONLY_ERROR\nFile : %s\nError opening HAWKEYE file read-only :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_POINT_RECORD_BUFFER_CALLOC_ERROR:
      sprintf (message, _("\nHAWKEYE_POINT_RECORD_BUFFER_CALLOC_ERROR\nFile : %s\nError allocating point record buffer header memory :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_META_HEADER_WRONG_VERSION_ERROR:
      sprintf (message, _("\nHAWKEYE_META_HEADER_WRONG_VERSION_ERROR\nFile : %s\nThe file version is incorrect.\n"),
               hawkeye_error.file);
      break;

    case HAWKEYE_CLOSE_ERROR:
      sprintf (message, _("\nHAWKEYE_CLOSE_ERROR\nFile : %s\nError closing HAWKEYE file :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_INVALID_RECORD_NUMBER_ERROR:
      sprintf (message, _("\nHAWKEYE_INVALID_RECORD_NUMBER_ERROR\nFile : %s\nRecord : %d\nInvalid record number.\n"),
               hawkeye_error.file, hawkeye_error.recnum);
      break;

    case HAWKEYE_READ_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_READ_FSEEK_ERROR\nFile : %s\nRecord : %d\nError during fseek prior reading a record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_READ_ERROR:
      sprintf (message, _("\nHAWKEYE_READ_ERROR\nFile : %s\nRecord : %d\nError reading record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_UPDATE_RECORD_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_UPDATE_RECORD_FSEEK_ERROR\nFile : %s\nError during fseek prior to updating record :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_UPDATE_RECORD_WRITE_ERROR:
      sprintf (message, _("\nHAWKEYE_UPDATE_RECORD_WRITE_ERROR\nFile : %s\nRecord : %d\nError updating record value :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_NOT_HAWKEYE_FILE_ERROR:
      sprintf (message, _("\nHAWKEYE_NOT_HAWKEYE_FILE_ERROR\nFile : %s\nThe file signature string is corrupt or indicates that this is not a HAWKEYE file.\n"),
               hawkeye_error.file);
      break;

    case HAWKEYE_SFM_FILE_OPEN_ERROR:
      sprintf (message, _("\nHAWKEYE_SFM_FILE_OPEN_ERROR\nFile : %s\nError opening HAWKEYE SFM file for read :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_SFM_READ_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_SFM_READ_FSEEK_ERROR\nFile : %s\nRecord : %d\nError during SFM fseek prior reading a record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_SFM_READ_ERROR:
      sprintf (message, _("\nHAWKEYE_SFM_READ_ERROR\nFile : %s\nRecord : %d\nError reading SFM record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_SFM_EOF_ERROR:
      sprintf (message, _("\nHAWKEYE_SFM_EOF_ERROR\nFile : %s\nRecord : %d\nEnd of file reached reading SFM record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_DAT_FILE_OPEN_ERROR:
      sprintf (message, _("\nHAWKEYE_DAT_FILE_OPEN_ERROR\nFile : %s\nError opening HAWKEYE DAT file for read :\n%s\n"),
               hawkeye_error.file, errno_message);
      break;

    case HAWKEYE_DAT_READ_FSEEK_ERROR:
      sprintf (message, _("\nHAWKEYE_DAT_READ_FSEEK_ERROR\nFile : %s\nRecord : %d\nError during DAT fseek prior reading a record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_DAT_READ_HEADER_ERROR:
      sprintf (message, _("\nHAWKEYE_DAT_READ_HEADER_ERROR\nFile : %s\nRecord : %d\nError reading DAT header record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_NOT_SHOT_DATA_ERROR:
      sprintf (message, _("\nHAWKEYE_NOT_SHOT_DATA_ERROR\nFile : %s\nRecord : %d\nNot a shot data record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_DAT_READ_SHOT_ERROR:
      sprintf (message, _("\nHAWKEYE_DAT_READ_SHOT_ERROR\nFile : %s\nRecord : %d\nError reading DAT shot record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;

    case HAWKEYE_DAT_READ_WAVE_ERROR:
      sprintf (message, _("\nHAWKEYE_DAT_READ_WAVE_ERROR\nFile : %s\nRecord : %d\nError reading DAT waveform record :\n%s\n"),
               hawkeye_error.file, hawkeye_error.recnum, errno_message);
      break;
    }

  return (message);
}



/*********************************************************************************************

  Function:    hawkeye_perror

  Purpose:     Prints (to stderr) the latest error messages.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   N/A

  Returns:     N/A

*********************************************************************************************/

HAWKEYE_DLL void hawkeye_perror ()
{
  fprintf (stderr, hawkeye_strerror ());
  fflush (stderr);
}



/*********************************************************************************************

  Function:    hawkeye_get_version

  Purpose:     Returns the HAWKEYE library version string

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   N/A

  Returns:     NV_CHAR       -    version string

*********************************************************************************************/

HAWKEYE_DLL NV_CHAR *hawkeye_get_version ()
{
  return (HAWKEYE_VERSION);
}



/*********************************************************************************************

  Function:    hawkeye_dump_record

  Purpose:     Print the HAWKEYE record to stdout.

  Author:      Jan C. Depner (jan.depner@navy.mil, eviltwin69@cableone.net)

  Date:        06/22/10

  Arguments:   hnd            -    The file handle
               hawkeye_record     -    The HAWKEYE record

  Returns:     N/A

*********************************************************************************************/

HAWKEYE_DLL void hawkeye_dump_record (NV_INT32 hnd, HAWKEYE_RECORD hawkeye_record)
{
  NV_INT32        year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  if (hawkeye[hnd].ContentsHeader.available.Timestamp)
    {
      hawkeye_cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);
      hawkeye_jday2mday (year, day, &month, &mday);
      month++;

      printf (N_("******************************************************************\n"));
      printf (_("Date/Time : %d-%02d-%02d (%d) %02d:%02d:%05.9f\n"), year + 1900, month, mday, day, hour, minute, second);
      printf (_("Timestamp %f\n"), hawkeye_record.Timestamp);
    }

  if (hawkeye[hnd].ContentsHeader.available.Surface_Latitude) printf (_("Surface_Latitude %.9f\n"), hawkeye_record.Surface_Latitude);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Longitude) printf (_("Surface_Longitude %.9f\n"), hawkeye_record.Surface_Longitude);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Northing) printf (_("Surface_Northing %f\n"), hawkeye_record.Surface_Northing);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Easting) printf (_("Surface_Easting %f\n"), hawkeye_record.Surface_Easting);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Altitude) printf (_("Surface_Altitude %f\n"), hawkeye_record.Surface_Altitude);
  if (hawkeye[hnd].ContentsHeader.available.Interpolated_Surface_Altitude) printf (_("Interpolated_Surface_Altitude %f\n"),
                                                                                   hawkeye_record.Interpolated_Surface_Altitude);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Northing_StdDev) printf (_("Surface_Northing_StdDev %f\n"), hawkeye_record.Surface_Northing_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Easting_StdDev) printf (_("Surface_Easting_StdDev %f\n"), hawkeye_record.Surface_Easting_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Surface_Altitude_StdDev) printf (_("Surface_Altitude_StdDev %f\n"), hawkeye_record.Surface_Altitude_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Point_Latitude) printf (_("Point_Latitude %.9f\n"), hawkeye_record.Point_Latitude);
  if (hawkeye[hnd].ContentsHeader.available.Point_Longitude) printf (_("Point_Longitude %.9f\n"), hawkeye_record.Point_Longitude);
  if (hawkeye[hnd].ContentsHeader.available.Point_Northing) printf (_("Point_Northing %f\n"), hawkeye_record.Point_Northing);
  if (hawkeye[hnd].ContentsHeader.available.Point_Easting) printf (_("Point_Easting %f\n"), hawkeye_record.Point_Easting);
  if (hawkeye[hnd].ContentsHeader.available.Point_Altitude) printf (_("Point_Altitude %f\n"), hawkeye_record.Point_Altitude);
  if (hawkeye[hnd].ContentsHeader.available.Point_Northing_StdDev) printf (_("Point_Northing_StdDev %f\n"), hawkeye_record.Point_Northing_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Point_Easting_StdDev) printf (_("Point_Easting_StdDev %f\n"), hawkeye_record.Point_Easting_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Point_Altitude_StdDev) printf (_("Point_Altitude_StdDev %f\n"), hawkeye_record.Point_Altitude_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Azimuth) printf (_("Azimuth %f\n"), hawkeye_record.Azimuth);
  if (hawkeye[hnd].ContentsHeader.available.Air_Nadir_Angle) printf (_("Air_Nadir_Angle %f\n"), hawkeye_record.Air_Nadir_Angle);
  if (hawkeye[hnd].ContentsHeader.available.Water_Nadir_Angle) printf (_("Water_Nadir_Angle %f\n"), hawkeye_record.Water_Nadir_Angle);
  if (hawkeye[hnd].ContentsHeader.available.Slant_range) printf (_("Slant_range %f\n"), hawkeye_record.Slant_range);
  if (hawkeye[hnd].ContentsHeader.available.Slant_Range_Comp_Model) printf (_("Slant_Range_Comp_Model %d\n"), hawkeye_record.Slant_Range_Comp_Model);
  if (hawkeye[hnd].ContentsHeader.available.Wave_Height) printf (_("Wave_Height %f\n"), hawkeye_record.Wave_Height);
  if (hawkeye[hnd].ContentsHeader.available.Water_Quality_Correction) printf (_("Water_Quality_Correction %f\n"), hawkeye_record.Water_Quality_Correction);
  if (hawkeye[hnd].ContentsHeader.available.Tidal_Correction) printf (_("Tidal_Correction %f\n"), hawkeye_record.Tidal_Correction);
  if (hawkeye[hnd].ContentsHeader.available.Depth) printf (_("Depth %f\n"), hawkeye_record.Depth);
  if (hawkeye[hnd].ContentsHeader.available.Depth_Amplitude) printf (_("Depth_Amplitude %f\n"), hawkeye_record.Depth_Amplitude);
  if (hawkeye[hnd].ContentsHeader.available.Depth_Class) printf (_("Depth_Class %d\n"), hawkeye_record.Depth_Class);
  if (hawkeye[hnd].ContentsHeader.available.Depth_Confidence) printf (_("Depth_Confidence %f\n"), hawkeye_record.Depth_Confidence);
  if (hawkeye[hnd].ContentsHeader.available.Pixel_Index) printf (_("Pixel_Index %d\n"), hawkeye_record.Pixel_Index);
  if (hawkeye[hnd].ContentsHeader.available.Scanner_Angle_X) printf (_("Scanner_Angle_X %f\n"), hawkeye_record.Scanner_Angle_X);
  if (hawkeye[hnd].ContentsHeader.available.Scanner_Angle_Y) printf (_("Scanner_Angle_Y %f\n"), hawkeye_record.Scanner_Angle_Y);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Latitude) printf (_("Aircraft_Latitude %.9f\n"), hawkeye_record.Aircraft_Latitude);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Longitude) printf (_("Aircraft_Longitude %.9f\n"), hawkeye_record.Aircraft_Longitude);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Northing) printf (_("Aircraft_Northing %f\n"), hawkeye_record.Aircraft_Northing);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Easting) printf (_("Aircraft_Easting %f\n"), hawkeye_record.Aircraft_Easting);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Altitude) printf (_("Aircraft_Altitude %f\n"), hawkeye_record.Aircraft_Altitude);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Roll) printf (_("Aircraft_Roll %f\n"), hawkeye_record.Aircraft_Roll);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Pitch) printf (_("Aircraft_Pitch %f\n"), hawkeye_record.Aircraft_Pitch);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Heading) printf (_("Aircraft_Heading %f\n"), hawkeye_record.Aircraft_Heading);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Northing_StdDev) printf (_("Aircraft_Northing_StdDev %f\n"), hawkeye_record.Aircraft_Northing_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Easting_StdDev) printf (_("Aircraft_Easting_StdDev %f\n"), hawkeye_record.Aircraft_Easting_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Altitude_StdDev) printf (_("Aircraft_Altitude_StdDev %f\n"), hawkeye_record.Aircraft_Altitude_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Roll_StdDev) printf (_("Aircraft_Roll_StdDev %f\n"), hawkeye_record.Aircraft_Roll_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Pitch_StdDev) printf (_("Aircraft_Pitch_StdDev %f\n"), hawkeye_record.Aircraft_Pitch_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Aircraft_Heading_StdDev) printf (_("Aircraft_Heading_StdDev %f\n"), hawkeye_record.Aircraft_Heading_StdDev);
  if (hawkeye[hnd].ContentsHeader.available.Extracted_Waveform_Attributes) printf (_("Extracted_Waveform_Attributes %d\n"),
                                                                                   hawkeye_record.Extracted_Waveform_Attributes);
  if (hawkeye[hnd].ContentsHeader.available.Receiver_Data_Used) printf (_("Receiver_Data_Used %d\n"), hawkeye_record.Receiver_Data_Used);
  if (hawkeye[hnd].ContentsHeader.available.Manual_Output_Screening_Flags) printf (_("Manual_Output_Screening_Flags %x\n"),
                                                                                   hawkeye_record.Manual_Output_Screening_Flags);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Amplitude) printf (_("Waveform_Peak_Amplitude %f\n"), hawkeye_record.Waveform_Peak_Amplitude);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Classification) printf (_("Waveform_Peak_Classification %d\n"),
                                                                                  hawkeye_record.Waveform_Peak_Classification);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Contrast) printf (_("Waveform_Peak_Contrast %f\n"), hawkeye_record.Waveform_Peak_Contrast);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Debug_Flags) printf (_("Waveform_Peak_Debug_Flags %x\n"),
                                                                               hawkeye_record.Waveform_Peak_Debug_Flags);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Attributes) printf (_("Waveform_Peak_Attributes %d\n"),
                                                                              hawkeye_record.Waveform_Peak_Attributes);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Jitter) printf (_("Waveform_Peak_Jitter %f\n"), hawkeye_record.Waveform_Peak_Jitter);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Position) printf (_("Waveform_Peak_Position %f\n"), hawkeye_record.Waveform_Peak_Position);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Pulsewidth) printf (_("Waveform_Peak_Pulsewidth %f\n"), hawkeye_record.Waveform_Peak_Pulsewidth);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_SNR) printf (_("Waveform_Peak_SNR %f\n"), hawkeye_record.Waveform_Peak_SNR);
  if (hawkeye[hnd].ContentsHeader.available.Scan_Direction_Flag) printf (_("Scan_Direction_Flag %d\n"), hawkeye_record.Scan_Direction_Flag);
  if (hawkeye[hnd].ContentsHeader.available.Edge_of_Flightline) printf (_("Edge_of_Flightline %d\n"), hawkeye_record.Edge_of_Flightline);
  if (hawkeye[hnd].ContentsHeader.available.Scan_Angle_Rank) printf (_("Scan_Angle_Rank %d\n"), hawkeye_record.Scan_Angle_Rank);
  if (hawkeye[hnd].ContentsHeader.available.SelectBottomCase) printf (_("SelectBottomCase %d\n"), hawkeye_record.SelectBottomCase);
  if (hawkeye[hnd].ContentsHeader.available.Elevation) printf (_("Elevation %f\n"), hawkeye_record.Elevation);
  if (hawkeye[hnd].ContentsHeader.available.Return_Number) printf (_("Return_Number %d\n"), hawkeye_record.Return_Number);
  if (hawkeye[hnd].ContentsHeader.available.Number_of_Returns) printf (_("Number_of_Returns %d\n"), hawkeye_record.Number_of_Returns);
  if (hawkeye[hnd].ContentsHeader.available.Surface_altitude_interpolated_tide_corrected) printf (_("Surface_altitude_interpolated_tide_corrected %f\n"),
                                                                                                  hawkeye_record.Surface_altitude_interpolated_tide_corrected);
  if (hawkeye[hnd].ContentsHeader.available.IR_polarisation_ratio) printf (_("IR_polarisation_ratio %f\n"), hawkeye_record.IR_polarisation_ratio);
  if (hawkeye[hnd].ContentsHeader.available.Spatial_Quality) printf (_("Spatial_Quality %f\n"), hawkeye_record.Spatial_Quality);
  if (hawkeye[hnd].ContentsHeader.available.IR_AMPLITUDE) printf (_("IR_AMPLITUDE %f\n"), hawkeye_record.IR_AMPLITUDE);
  if (hawkeye[hnd].ContentsHeader.available.Scan_Position) printf (_("Scan_Position %d\n"), hawkeye_record.Scan_Position);
  if (hawkeye[hnd].ContentsHeader.available.K_Lidar) printf (_("K_Lidar %f\n"), hawkeye_record.K_Lidar);
  if (hawkeye[hnd].ContentsHeader.available.K_Lidar_Average) printf (_("K_Lidar_Average %f\n"), hawkeye_record.K_Lidar_Average);
  if (hawkeye[hnd].ContentsHeader.available.K_Lidar_Deep_Momentary) printf (_("K_Lidar_Deep_Momentary %f\n"), hawkeye_record.K_Lidar_Deep_Momentary);
  if (hawkeye[hnd].ContentsHeader.available.K_Lidar_Shallow_Momentary) printf (_("K_Lidar_Shallow_Momentary %f\n"), hawkeye_record.K_Lidar_Shallow_Momentary);
  if (hawkeye[hnd].ContentsHeader.available.Relative_reflectivity) printf (_("Relative_reflectivity %f\n"), hawkeye_record.Relative_reflectivity);
  if (hawkeye[hnd].ContentsHeader.available.Waveform_Peak_Gain) printf (_("Waveform_Peak_Gain %f\n"), hawkeye_record.Waveform_Peak_Gain);
  if (hawkeye[hnd].ContentsHeader.available.Adjusted_Amplitude) printf (_("Adjusted_Amplitude %f\n"), hawkeye_record.Adjusted_Amplitude);
  if (hawkeye[hnd].ContentsHeader.available.Water_Travel_Time) printf (_("Water_Travel_Time %f\n"), hawkeye_record.Water_Travel_Time);
  if (hawkeye[hnd].ContentsHeader.available.Attenuation_c) printf (_("Attenuation_c %f\n"), hawkeye_record.Attenuation_c);
  if (hawkeye[hnd].ContentsHeader.available.Absorption_a) printf (_("Absorption_a %f\n"), hawkeye_record.Absorption_a);
  if (hawkeye[hnd].ContentsHeader.available.Scattering_s) printf (_("Scattering_s %f\n"), hawkeye_record.Scattering_s);
  if (hawkeye[hnd].ContentsHeader.available.Backscattering_Bb) printf (_("Backscattering_Bb %f\n"), hawkeye_record.Backscattering_Bb);

  fflush (stdout);
}
