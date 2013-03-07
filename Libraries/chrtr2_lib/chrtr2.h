
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



#ifndef __CHRTR2_H__
#define __CHRTR2_H__


#ifdef  __cplusplus
extern "C" {
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS


  /*  Preparing for language translation using GNU gettext at some point in the future.  */

#define _(String) (String)
#define N_(String) String

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

  /*
#include <libintl.h>
#define _(String) gettext (String)
#define gettext_noop(String) String
#define N_(String) gettext_noop (String)
  */



  /*  Windoze insanity.  Note that there may be nothing defined for any of these DLL values.  When building statically on Windows I
      strip out the __declspec(dllexport) and __declspec(dllimport) definitions before I copy the .h file to the central include
      location.  That's so I don't have to modify a bunch of application Makefiles to check for static build adn set the _STATIC
      option.  Of course, on Linux/UNIX we don't have to do anything to these ;-)  */

#ifdef CHRTR2_DLL_EXPORT
#  define CHRTR2_DLL __declspec(dllexport)
#else
#  ifdef NVWIN3X
#    ifdef CHRTR2_STATIC
#      define CHRTR2_DLL
#    else
#      define CHRTR2_DLL __declspec(dllimport)
#    endif
#  else
#    define CHRTR2_DLL
#  endif
#endif

#endif /*  DOXYGEN_SHOULD_SKIP_THIS  */


#include "chrtr2_nvtypes.h"
#include "chrtr2_macros.h"


  /*! \mainpage CHRTR2 Data Format

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section intro Introduction

       Back in 1994 I realized that we needed a simple replacement for the old CHRTR grid format.
       The major shortcomings of CHRTR were that it didn't support unequal lat and lon grid sizes
       and it couldn't handle anything smaller than about 2 meter grids (due to 32 bit floating
       point precision).  Instead of just sitting down and writing the damn thing I talked to a
       bunch of other people about it.  At which point I realized the truth of the saying "the
       only thing with 20 legs, 20 arms, and 10 heads that doesn't have a brain is a committee".
       The finalized version of the new grid format was released some time around 2006.  It sits
       on top of HDF5 which is a gigantic container "format".  In order to use it you have to map
       in a huge library and use a bunch of arcane function calls.  In it's defense, it can hold
       anything and everything.  That's right, it's the world's largest crescent wrench.  I hate
       crescent wrenches...

       The CHRTR2 format was designed in keeping with the KISS principle (Keep It Simple Stupid).
       It is used to store and retrieve a simple depth value latitude and longitude grid with
       status and some optional fields (e.g.uncertainty).  It has an ASCII header that you can
       read with your own beady little eyes by simply "more"ing or "less"ing or, God forbid,
       "type /page"ing the file.  It is arranged south to north, west to east.  That is, the
       first row is the southernmost row and the first point in that row is the westernmost
       point.  The data is stored as bit packed, scaled integers so the file has no "endian-ness".
       As opposed to the "designed by committee" format described above, CHRTR2 took about two
       days to write.  I should have done this in 1994!


       <br><br>\section sec1 CHRTR2 API I/O function definitions

       The CHRTR2 API is very simple and consists of only about 25 functions.  The public functions and data structures
       documentation can be accessed from chrtr2.h in the Doxygen generated HTML documentation.  If you need to do
       any Inter-Process Communication between programs that use the CHRTR2 format you also need to look at the structure
       and constants defined in chrtr2_shared.h.



       <br><br>\section sec2 Example code for CHRTR2 files


       To make this as easy as possible to understand I am going to give a couple of pseudo-code 
       examples of how you would create, read, and write a CHRTR2 file.  Understanding how the data
       is actually stored in the CHRTR2 file isn't really all that important.  It's sort of like the
       Wizard of Oz - pay no attention to the man behind the curtain.  Without further ado...


       Creating a CHRTR2 file is the most complicated thing you can do with the CHRTR2 library.  The
       reason being that you must populate a CHRTR2 header that contains all of the information
       needed to define how the data will be stored internally.


       First you must include the chrtr2.h file:


       <pre>
       include "chrtr2.h"
       </pre>


       Then you have to define the header and record structures and an integer handle:


       <pre>
       CHRTR2_HEADER      chrtr2_header, egm_header;
       CHRTR2_RECORD      chrtr2_record;
       NV_INT32           chrtr2_handle;
       </pre>


       After that you populate the header with the info you need:


       <pre>
       memset (&chrtr2_header, 0, sizeof (CHRTR2_HEADER));

       strcpy (chrtr2_header.creation_software, "Program name and version");
       chrtr2_header.z_units = CHRTR2_METERS;
       chrtr2_header.grid_type = CHRTR2_MISP;
       chrtr2_header.mbr.wlon = Westernmost longitude;
       chrtr2_header.mbr.slat = Southernmost latitude;
       chrtr2_header.width = Number of grid columns;
       chrtr2_header.height = Number of grid rows;
       chrtr2_header.lat_grid_size_degrees = latitude grid size in degrees;
       chrtr2_header.lon_grid_size_degrees = longitude grid size in degrees;
       chrtr2_header.min_z = Minimum possible Z value;
       chrtr2_header.max_z = Maximum possible Z value;
       chrtr2_header.z_scale = Scale for Z values (gives resolution, for example, 100.0 would be centimeters);
       chrtr2_header.min_horizontal_uncertainty = Minimum possible horizontal uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.max_horizontal_uncertainty = Maximum possible horizontal uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.horizontal_uncertainty_scale = Scale for horizontal uncertainty values (if set to 0.0 no horizontal uncertainty is stored);
       chrtr2_header.min_vertical_uncertainty = Minimum possible vertical uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.max_vertical_uncertainty = Maximum possible vertical uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.vertical_uncertainty_scale = Scale for vertical uncertainty values (if set to 0.0 no vertical uncertainty is stored);
       chrtr2_header.min_uncertainty = Minimum possible total uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.max_uncertainty = Maximum possible total uncertainty (not needed if scale is set to 0.0);
       chrtr2_header.uncertainty_scale = Scale for total uncertainty values (if set to 0.0 no total uncertainty is stored);
       strcpy (chrtr2_header.z0_name, "Some local datum name like MLLW");
       chrtr2_header.min_z0 = Minimum possible datum/MSL separation value (not needed if scale is set to 0.0);
       chrtr2_header.max_z0 = Maximum possible datum/MSL separation value (not needed if scale is set to 0.0);
       chrtr2_header.z0_scale = Scale for datum/MSL separation values (if set to 0.0 no datum/MSL separation is stored);
       chrtr2_header.min_z1 = Minimum possible MSL/ellipsoid separation value (not needed if scale is set to 0.0);
       chrtr2_header.max_z1 = Maximum possible MSL/ellipsoid separation value (not needed if scale is set to 0.0);
       chrtr2_header.z1_scale = Scale for MSL/ellipsoid separation values (if set to 0.0 no MSL/ellipsoid separation is stored);
       chrtr2_header.max_number_of_points = Maximum number of contributing points for the bin (if set to 0, number of points is not stored);
       strcpy (chrtr2_header.uncertainty_name, "Standard Deviation");
       </pre>


       Try to create and open the chrtr2 file:


       <pre>
       chrtr2_handle = chrtr2_create_file ("File name", &chrtr2_header);
       if (chrtr2_handle < 0)
         {
           chrtr2_perror ();
           exit (-1);
         }  
       </pre>


       Then all you have to do is populate CHRTR2 records (probably in a loop of some sort) and write them to the file:


       <pre>
       memset (&chrtr2_record, 0, sizeof (CHRTR2_RECORD));

       chrtr2_record.z = Some Z value;
       chrtr2_record.horizontal_uncertainty = Some horizontal uncertainty value;
       chrtr2_record.vertical_uncertainty = Some vertical uncertainty value;
       chrtr2_record.uncertainty = Some total uncertainty value (e.g. STD);
       chrtr2_record.z0 = Some datum/MSL separation value;
       chrtr2_record.z1 = Some MSL/ellipsoid separation value;
       chrtr2_record.number_of_points = Some number of points;
       chrtr2_record.status = CHRTR2_REAL or CHRTR2_INTERPOLATED;


       if (chrtr2_write_record_row_col (chrtr2_handle, Row number, Column number, chrtr2_record))
         {
           chrtr2_perror ();
           exit (-1);
         }
       </pre>


       Finally, you have to close the file:


       <pre>
       chrtr2_close_file (chrtr2_handle);
       </pre>



       To read from a CHRTR2 file you just have to open the file:


       <pre>
       chrtr2_handle = chrtr2_open_file ("File name", &chrtr2_header, CHRTR2_READONLY);

       if (chrtr2_handle < 0)
         {
           chrtr2_perror ();
           exit (-1);
         }
       </pre>


       Then either read each record separately or loop through and read them all:


       <pre>
       for (i = 0 ; i < chrtr2_header.height ; i++)
         {
           for (j = 0 ; j < chrtr2_header.width ; j++)
             {
               if (chrtr2_read_record_row_col (chrtr2_handle, i, j, &chrtr2_record))
                 {
                   chrtr2_perror ();
                   exit (-1);
                 }


               /#  Do something...  #/
             }
         }
       </pre>


       Then close the file when you're done:


       <pre>
       chrtr2_close_file (chrtr2_handle);
       </pre>



       In addition to reading/writing single points from a CHRTR2 file you can read/write full or partial rows
       using a couple of other functions:


       <pre>
       chrtr2_read_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 length, CHRTR2_RECORD *chrtr2_record);
       chrtr2_write_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 length, CHRTR2_RECORD *chrtr2_record);
       </pre>

  */



  /*!

      - CHRTR2 header structure.  The use keys are defined as follows:

          - (0)  = Populated by the API.
          - (1)  = Optional.
          - (2)  = Required on creation of file.  Not modifiable after creation.
          - (3)  = Required on creation of file unless scale value is set to 0.0.  Not modifiable after creation.
          - (4)  = Scale values are essentially the required resolution.  Normally they would be something like 1000.0
                   but they could be any positive value, like 3.14159.  This value, in conjuction with the min and 
                   max values is used to determine storage requirements (in bits).
          - (5)  = Modifiable after creation of the file.  Most descriptive strings in the header are modifiable.
          - (6)  = Set to 0 to exclude data from the file.
          - (7)  = The wlon and slat values in the MBR are required to be set.  The elon and nlat values will be
                   computed using the width, height, lon_grid_size_degrees, and lat_grid_size_degrees.  If you set the
                   max_x and max_y they will still be recomputed so the the size is an EXACT (or as close as you can get
                   in binary representation) multiple of the span times the grid size.
  */

  typedef struct
  {
    NV_CHAR         version[128];                 /*!<  Library version information  (0)  */
    time_t          creation_tv_sec;              /*!<  File creation POSIX seconds from 01-01-1970  (0)  */
    NV_CHAR         creation_software[128];       /*!<  File creation software  (1)  */
    time_t          modification_tv_sec;          /*!<  File modification POSIX seconds from 01-01-1970  (0)  */
    NV_CHAR         modification_software[128];   /*!<  File modification software  (1)(5)  */
    NV_CHAR         security_classification[32];  /*!<  Security classification  (1)(5)  */
    NV_CHAR         distribution[1024];           /*!<  Security distribution statement  (1)(5)  */
    NV_CHAR         declassification[256];        /*!<  Security declassification statement  (1)(5)  */
    NV_CHAR         class_just[256];              /*!<  Security classification justification  (1)(5)  */
    NV_CHAR         downgrade[128];               /*!<  Security downgrade statement  (1)(5)  */
    NV_CHAR         user_flag_name[5][64];        /*!<  CHRTR2 user/application defined status flag names  (1)(5)  */
    NV_CHAR         uncertainty_name[64];         /*!<  Name describing value stored for total uncertainty  (1)(5)  */
    NV_CHAR         z0_name[64];                  /*!<  Datum name used for datum/MSL separation values (e.g. MLLW)  (1)(5)  */
    NV_CHAR         comments[4096];               /*!<  Comments  (1)(5)  */
    NV_INT16        grid_type;                    /*!<  Type of gridding application used  (1)  */
    NV_FLOAT32      min_observed_z;               /*!<  Minimum observed Z value for file (1)  */
    NV_FLOAT32      max_observed_z;               /*!<  Maximum observed Z value for file (1)  */


    /*  The following fields will be required to create a file and will not be modifiable after creation of the file.
        If you do not want to include a data type (e.g. horizontal uncertainty) set the ???_scale value to 0.0.
        See footnotes for more info.*/

    NV_U_BYTE       z_units;                      /*!<  Z units, either CHRTR2_METERS, CHRTR2_FEET, or CHRTR2_FATHOMS  (2)  */
    NV_F64_MBR      mbr;                          /*!<  Minimum bounding rectangle (7)  */
    NV_INT32        width;                        /*!<  Width of the data in grid cells (2)  */
    NV_INT32        height;                       /*!<  Height of the data in grid cells (2)  */
    NV_FLOAT64      lon_grid_size_degrees;        /*!<  Longitude grid size in degrees (2)  */
    NV_FLOAT64      lat_grid_size_degrees;        /*!<  Latitude grid size in degrees (2)  */
    NV_FLOAT32      min_z;                        /*!<  Minimum possible Z value for file (2)  */
    NV_FLOAT32      max_z;                        /*!<  Maximum possible Z value for file (2)  */
    NV_FLOAT32      z_scale;                      /*!<  Scale to multiply Z values by (2)(4)  */
    NV_FLOAT32      min_horizontal_uncertainty;   /*!<  Minimum possible horizontal uncertainty value for file (3)  */
    NV_FLOAT32      max_horizontal_uncertainty;   /*!<  Maximum possible horizontal uncertainty value for file (3)  */
    NV_FLOAT32      horizontal_uncertainty_scale; /*!<  Scale to multiply horizontal uncertainty values by (2)(6)  */
    NV_FLOAT32      min_vertical_uncertainty;     /*!<  Minimum possible vertical uncertainty value for file (3)  */
    NV_FLOAT32      max_vertical_uncertainty;     /*!<  Maximum possible vertical uncertainty value for file (3)  */
    NV_FLOAT32      vertical_uncertainty_scale;   /*!<  Scale to multiply vertical uncertainty values by (2)(6)  */
    NV_FLOAT32      min_uncertainty;              /*!<  Minimum possible total uncertainty value for file (3)  */
    NV_FLOAT32      max_uncertainty;              /*!<  Maximum possible total uncertainty value for file (3)  */
    NV_FLOAT32      uncertainty_scale;            /*!<  Scale to multiply total uncertainty values by (2)(6)  */
    NV_FLOAT32      min_z0;                       /*!<  Minimum possible datum/MSL separation value for file (3)  */
    NV_FLOAT32      max_z0;                       /*!<  Maximum possible datum/MSL separation value for file (3)  */
    NV_FLOAT32      z0_scale;                     /*!<  Scale to multiply datum/MSL separation values by (2)(6)  */
    NV_FLOAT32      min_z1;                       /*!<  Minimum possible MSL/ellipsoid separation value for file (3)  */
    NV_FLOAT32      max_z1;                       /*!<  Maximum possible MSL/ellipsoid separation value for file (3)  */
    NV_FLOAT32      z1_scale;                     /*!<  Scale to multiply MSL/ellipsoid separation values by (2)(6)  */
    NV_INT32        max_number_of_points;         /*!<  Maximum number of points per bin  (2)(6)  */
  } CHRTR2_HEADER;


  /*
      - CHRTR2 point record structure.  The use keys are defined as follows:

          - (0)  = Required.
          - (1)  = Optional.
          - (2)  = Modifiable after creation of the file.  These fields are open to end user interpretation so we
                   need to be able to modify them after file creation.
  */

  typedef struct
  {
    NV_FLOAT32       z;                      /*!<  Z value (positive down - i.e. depth) at center of grid (0)  */
    NV_FLOAT32       horizontal_uncertainty; /*!<  In [Z UNITS]  (1)  */
    NV_FLOAT32       vertical_uncertainty;   /*!<  In [Z UNITS]  (1)  */
    NV_U_INT16       status;                 /*!<  CHRTR2_STATUS_BITS bits of status information (0)(2)  */
    NV_FLOAT32       uncertainty;            /*!<  In [Z UNITS]  (1)  */
    NV_FLOAT32       z0;                     /*!<  datum/MSL separation in [Z UNITS]  (1)  */
    NV_FLOAT32       z1;                     /*!<  MSL/ellipsoid separation in [Z UNITS]  (1)  */
    NV_INT32         number_of_points;       /*!<  Number of points  (1)  */
  } CHRTR2_RECORD;



  /*!  Public API I/O function declarations.  */

  CHRTR2_DLL NV_INT32 chrtr2_create_file (const NV_CHAR *path, CHRTR2_HEADER *chrtr2_header);
  CHRTR2_DLL NV_INT32 chrtr2_open_file (const NV_CHAR *path, CHRTR2_HEADER *chrtr2_header, NV_INT32 mode);
  CHRTR2_DLL NV_INT32 chrtr2_close_file (NV_INT32 hnd);
  CHRTR2_DLL NV_INT32 chrtr2_read_record (NV_INT32 hnd, NV_I32_COORD2 coord, CHRTR2_RECORD *chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_read_record_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, CHRTR2_RECORD *chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_read_record_lat_lon (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, CHRTR2_RECORD *chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_read_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 length, CHRTR2_RECORD *chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_write_record (NV_INT32 hnd, NV_I32_COORD2 coord, CHRTR2_RECORD chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_write_record_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, CHRTR2_RECORD chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_write_record_lat_lon (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, CHRTR2_RECORD chrtr2_record);
  CHRTR2_DLL NV_INT32 chrtr2_write_row (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 length, CHRTR2_RECORD *chrtr2_record);
  CHRTR2_DLL void chrtr2_update_header (NV_INT32 hnd, CHRTR2_HEADER chrtr2_header);
  CHRTR2_DLL NV_INT32 chrtr2_open_cov_map (NV_INT32 hnd);
  CHRTR2_DLL NV_INT32 chrtr2_read_cov_map (NV_INT32 hnd, NV_I32_COORD2 coord, NV_U_INT16 *status);
  CHRTR2_DLL NV_INT32 chrtr2_read_cov_map_row_col (NV_INT32 hnd, NV_INT32 row, NV_INT32 col, NV_U_INT16 *status);
  CHRTR2_DLL NV_INT32 chrtr2_close_cov_map (NV_INT32 hnd);
  CHRTR2_DLL NV_INT32 chrtr2_get_coord (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_I32_COORD2 *coord);
  CHRTR2_DLL NV_INT32 chrtr2_get_lat_lon (NV_INT32 hnd, NV_FLOAT64 *lat, NV_FLOAT64 *lon, NV_I32_COORD2 coord);
  CHRTR2_DLL NV_INT32 chrtr2_get_errno ();
  CHRTR2_DLL NV_CHAR *chrtr2_strerror ();
  CHRTR2_DLL void chrtr2_perror ();
  CHRTR2_DLL NV_CHAR *chrtr2_get_version ();
  CHRTR2_DLL void chrtr2_cvtime (time_t tv_sec, long tv_nsec, NV_INT32 *year, NV_INT32 *jday, NV_INT32 *hour,
                                 NV_INT32 *minute, NV_FLOAT32 *second);
  CHRTR2_DLL void chrtr2_jday2mday (NV_INT32 year, NV_INT32 jday, NV_INT32 *mon, NV_INT32 *mday);
  CHRTR2_DLL void chrtr2_inv_cvtime (NV_INT32 year, NV_INT32 jday, NV_INT32 hour, NV_INT32 min, NV_FLOAT32 sec,
                                     time_t *tv_sec, long *tv_nsec);
  CHRTR2_DLL void chrtr2_mday2jday (NV_INT32 year, NV_INT32 mon, NV_INT32 mday, NV_INT32 *jday);



#ifdef  __cplusplus
}
#endif

#endif
