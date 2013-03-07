
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



#ifndef __LLZ_H__
#define __LLZ_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include <time.h>
#include "nvutility.h"


  /*! \mainpage LLZ Data Format

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

       The LLZ data type is a simple, binary, [time]/lat/lon/depth format.  It includes a 16384 byte ASCII
       header.  The data can be little or big endian.  All Z values are stored as depths not elevations.
       That is, depths are positive and elevations are negative (this is the Naval Oceanographic Office 
       not the Naval Topographic Office).  The ASCII header includes the following keys:

       <pre>
       [VERSION] =
       [ENDIAN] =
       [TIME FLAG] = 
       [DEPTH UNITS] = 
       [CLASSIFICATION] =
       [DISTRIBUTION] =
       [DECLASSIFICATION] =
       [CLASSIFICATION JUSTIFICATION] =
       [DOWNGRADE] =
       [SOURCE] =
       [COMMENTS] =
       [CREATION_DATE] =
       [LAST MODIFIED DATE] =
       [NUMBER OF RECORDS] =
       [END OF HEADER]
       </pre>

       The [ENDIAN] = field will always be either BIG or LITTLE depending on the byte ordering of the
       binary data in the file.  The time portion of the llz is optional.  If the [TIME FLAG] field is
       equal to 0 or there is no [TIME FLAG] field in the header then time will not be included with
       each lat/lon/depth record.  Following the 16384 byte ASCII header will be [NUMBER OF RECORDS]
       records.  The latitude, longitude, and depth in each record are stored as scaled, signed 32 bit
       integers.  The status is stored as an unsigned, 16 bit integer.  If time is included with each
       record it will be stored as two 32 bit fields that will be POSIX time fields.  That is, the first
       32 bit field will contain integer seconds from January 1, 1970 and the second will contain 
       nanoseconds of the second.  If time is not included in each record these fields in the LLZ_REC 
       structure will be set to 0.  If [DEPTH UNITS] is not set it defaults to METERS.  The other options
       for depth units are FEET, FATHOMS, CUBITS (roman), and WILLETTS.  These are set in the LLZ_HEADER
       record as 0, 1, 2, 3, and 4 respectively.  Order and scaling of the records is as follows:

       <pre>
       (NV_INT32) seconds from January 1, 1970 - optional
       (NV_INT32) nanoseconds - optional
       NINT (latitude * 10000000.0L)
       NINT (longitude * 10000000.0L)
       NINT (depth * 10000.0L)
       (NV_INT16) status
       </pre>

       The status bits are explained in llz.h.

*/


#define MAX_LLZ_FILES 32
#define LLZ_HEADER_SIZE 16384


#define LLZ_METERS 0
#define LLZ_FEET 1
#define LLZ_FATHOMS 2
#define LLZ_CUBITS 3
#define LLZ_WILLETTS 4


#define LLZ_NEXT_RECORD -1


typedef struct
{
  NV_CHAR              version[50];
  NV_BOOL              time_flag;
  NV_U_BYTE            depth_units;            /*!<  0 - meters, 1 - feet, 2 - fathoms, 3 - cubits, 4 - willetts */
  NV_CHAR              classification[50];
  NV_CHAR              distribution[1000];
  NV_CHAR              declassification[200];
  NV_CHAR              class_just[200];
  NV_CHAR              downgrade[100];
  NV_CHAR              source[100];
  NV_CHAR              comments[500];
  NV_CHAR              creation_date[30];
  NV_CHAR              modified_date[30];
  NV_INT32             number_of_records;
} LLZ_HEADER;

typedef struct
{
  time_t               tv_sec;
  long                 tv_nsec;
  NV_F64_POS           xy;
  NV_FLOAT32           depth;
  NV_U_INT32           status;
} LLZ_REC;


  NV_INT32 create_llz (const NV_CHAR *path, LLZ_HEADER llz_header);
  NV_INT32 open_llz (const NV_CHAR *path, LLZ_HEADER *llz_header);
  void close_llz (NV_INT32 hnd);
  NV_BOOL read_llz (NV_INT32 hnd, NV_INT32 recnum, LLZ_REC *data);
  NV_BOOL append_llz (NV_INT32 hnd, LLZ_REC data);
  NV_BOOL update_llz (NV_INT32 hnd, NV_INT32 recnum, LLZ_REC data);
  NV_INT32 ftell_llz (NV_INT32 hnd);


#define             LLZ_MANUALLY_INVAL      1       /*!<  Point has been manually marked as invalid : 0000 0000 0000 0001 */
#define             LLZ_FILTER_INVAL        2       /*!<  Point has been automatically marked as invalid : 0000 0000 0000 0010 */
#define             LLZ_INVAL               3       /*!<  Mask to check for either type of invalidity : 0000 0000 0000 0011 */


#ifdef  __cplusplus
}
#endif

#endif
