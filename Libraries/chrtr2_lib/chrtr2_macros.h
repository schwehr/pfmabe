
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



#ifndef __CHRTR2_MACROS_H__
#define __CHRTR2_MACROS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define CHRTR2_MAX_FILES               128  /*!<  Maximum number of open CHRTR2 files.  */


#define CHRTR2_METERS                  0    /*!<  Meters  */
#define CHRTR2_FEET                    1    /*!<  Feet  */
#define CHRTR2_FATHOMS                 2    /*!<  Fathoms  */


#define CHRTR2_MISP                    0    /*!<  Minimum Curvature Spline Interpolated grid.  */
#define CHRTR2_GMT                     1    /*!<  Generic Mapping Tools surface grid.  */


#define CHRTR2_UPDATE                  0    /*!<  Open file for update.  */
#define CHRTR2_READONLY                1    /*!<  Open file read only.  */


  /*  Status flags.  */

#define       CHRTR2_NULL                 0       /*!< 0000 0000 0000  =  Cell has not been populated.  */
#define       CHRTR2_REAL                 1       /*!< 0000 0000 0001  =  Cell has been populated with at least one surveyed data point.  */
#define       CHRTR2_INTERPOLATED         2       /*!< 0000 0000 0010  =  Cell has been populated with interpolated data.  */
#define       CHRTR2_DIGITIZED_CONTOUR    4       /*!< 0000 0000 0100  =  Cell has been populated with hand-drawn or digitized contour data.  */
#define       CHRTR2_CHECKED              8       /*!< 0000 0000 1000  =  Cell has been marked as CHECKED by user.  */
#define       CHRTR2_USER_01              16      /*!< 0000 0001 0000  =  User flag 01.  */
#define       CHRTR2_USER_02              32      /*!< 0000 0010 0000  =  User flag 02.  */
#define       CHRTR2_USER_03              64      /*!< 0000 0100 0000  =  User flag 03.  */
#define       CHRTR2_USER_04              128     /*!< 0000 1000 0000  =  User flag 04.  */
#define       CHRTR2_LAND_MASK            256     /*!< 0001 0000 0000  =  Land mask flag (may be a topo value or a constant mask value).  */
#define       CHRTR2_SPARE1               512     /*!< 0010 0000 0000  =  Spare  */
#define       CHRTR2_SPARE2               1024    /*!< 0100 0000 0000  =  Spare  */
#define       CHRTR2_SPARE3               2048    /*!< 1000 0000 0000  =  Spare  */

#define       CHRTR2_USER                 496     /*!< 0001 1111 0000  =  User flag mask  */


  /*!  Since REAL, INTERPOLATED, and DIGITIZED are mutually exclusive we use a mask when we set these.  
       Note that when you set the record to NULL you need to zero out the entire status value since
       NULL data can't have any state (not even CHRTR2_CHECKED).  */

#define       CHRTR2_REAL_MASK           4089     /*!< 1111 1111 1001 */
#define       CHRTR2_INTERPOLATED_MASK   4090     /*!< 1111 1111 1010 */
#define       CHRTR2_DIGITIZED_MASK      4092     /*!< 1111 1111 1100 */


  /*!  Error conditions.  */

#define       CHRTR2_SUCCESS                      0
#define       CHRTR2_WRITE_FSEEK_ERROR           -1
#define       CHRTR2_WRITE_ERROR                 -2
#define       CHRTR2_HEADER_WRITE_FSEEK_ERROR    -3
#define       CHRTR2_HEADER_WRITE_ERROR          -4
#define       CHRTR2_TOO_MANY_OPEN_FILES         -5
#define       CHRTR2_CREATE_ERROR                -6
#define       CHRTR2_OPEN_UPDATE_ERROR           -7
#define       CHRTR2_OPEN_READONLY_ERROR         -8
#define       CHRTR2_NOT_CHRTR2_FILE_ERROR       -9
#define       CHRTR2_NEWER_FILE_VERSION_WARNING  -10
#define       CHRTR2_CLOSE_ERROR                 -11
#define       CHRTR2_INVALID_COORD               -12
#define       CHRTR2_INVALID_ROW_COLUMN          -13
#define       CHRTR2_READ_FSEEK_ERROR            -14
#define       CHRTR2_READ_ERROR                  -15
#define       CHRTR2_VALUE_OUT_OF_RANGE_ERROR    -16
#define       CHRTR2_INVALID_LAT_LON             -17
#define       CHRTR2_NO_COV_MAP                  -18



#ifdef  __cplusplus
}
#endif


#endif
