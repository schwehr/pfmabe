
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



#include <stdio.h>
#include <time.h>
#include "nvtypes.h"


/***************************************************************************/
/*!

  - Module Name:        print_time

  - Programmer:         Dave Johanson

  - Date Written:       January 1996

  - Purpose:            Prints out year, day, hour, minute, and second
                        given the ISS60 POSIX format time values.
                        user.

  - Inputs:
                        - tv_sec       =   seconds from the Epoch Jan. 1 1970
                        - tv_nsec      =   nanoseconds

  - Outputs:
                        - void

****************************************************************************/

void print_time (time_t tv_sec, long tv_nsec)
{
   struct tm            time_struct, *time_ptr = &time_struct;
   NV_INT16             year, jday, hour, minute;
   NV_FLOAT32           second;

   time_ptr = gmtime(&tv_sec);

   year = (NV_INT16) time_ptr->tm_year;
   jday = (NV_INT16) time_ptr->tm_yday + 1;
   hour = (NV_INT16) time_ptr->tm_hour;
   minute = (NV_INT16) time_ptr->tm_min;
   second = (NV_FLOAT32) time_ptr->tm_sec + (NV_FLOAT32) ((NV_FLOAT64) tv_nsec / 1000000000.);

   printf ("%4d/%03d %02d:%02d:%04.1f", year + 1900, jday, hour, minute, second);
}
