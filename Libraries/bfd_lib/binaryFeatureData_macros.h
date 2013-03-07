
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



#ifndef __BFD_MACROS_H__
#define __BFD_MACROS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define BFDATA_MAX_FILES               32        /*!<  Maximum number of open BFD files  */
#define BFDATA_HEADER_SIZE             65536     /*!<  Header size  */
#define BFDATA_POLY_VERSION_SIZE       512       /*!<  Polygon file header size  */
#define BFDATA_NEXT_RECORD             -1        /*!<  Next record flag  */
#define BFDATA_POLY_ARRAY_SIZE         10000     /*!<  Serious overkill ;-)  */


  /*  File open modes.  */

#define BFDATA_UPDATE                  0         /*!<  Open file for update  */
#define BFDATA_READONLY                1         /*!<  Open file read only  */


  /*  Error conditions.  */

#define       BFDATA_SUCCESS                      0
#define       BFDATA_POLYGON_TOO_LARGE_ERROR      -1
#define       BFDATA_INVALID_RECORD_NUMBER        -2
#define       BFDATA_POLY_WRITE_FSEEK_ERROR       -3
#define       BFDATA_POLY_WRITE_ERROR             -4
#define       BFDATA_RECORD_WRITE_FSEEK_ERROR     -5
#define       BFDATA_RECORD_WRITE_ERROR           -6
#define       BFDATA_HEADER_WRITE_FSEEK_ERROR     -7
#define       BFDATA_HEADER_WRITE_ERROR           -8
#define       BFDATA_TOO_MANY_OPEN_FILES          -9
#define       BFDATA_CREATE_ERROR                 -10
#define       BFDATA_CREATE_POLY_ERROR            -11
#define       BFDATA_OPEN_UPDATE_ERROR            -12
#define       BFDATA_OPEN_POLY_UPDATE_ERROR       -13
#define       BFDATA_OPEN_READONLY_ERROR          -14
#define       BFDATA_OPEN_POLY_READONLY_ERROR     -15
#define       BFDATA_NOT_BFD_FILE_ERROR           -16
#define       BFDATA_NEWER_FILE_VERSION_ERROR     -17
#define       BFDATA_CLOSE_ERROR                  -18
#define       BFDATA_CLOSE_POLY_ERROR             -19
#define       BFDATA_RECORD_READ_FSEEK_ERROR      -20
#define       BFDATA_RECORD_READ_ERROR            -21
#define       BFDATA_NO_POLYGON_AVAILABLE         -22
#define       BFDATA_POLY_READ_FSEEK_ERROR        -23
#define       BFDATA_POLY_READ_ERROR              -24
#define       BFDATA_END_OF_FILE                  -25
#define       BFDATA_IMAGE_WRITE_FSEEK_ERROR      -26
#define       BFDATA_IMAGE_WRITE_ERROR            -27
#define       BFDATA_NO_IMAGE_AVAILABLE           -28
#define       BFDATA_IMAGE_READ_FSEEK_ERROR       -29
#define       BFDATA_IMAGE_READ_ERROR             -30
#define       BFDATA_IMAGE_FILE_OPEN_ERROR        -31
#define       BFDATA_IMAGE_FILE_READ_ERROR        -32



#ifdef  __cplusplus
}
#endif


#endif

