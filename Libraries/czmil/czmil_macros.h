
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



#ifndef __CZMIL_MACROS_H__
#define __CZMIL_MACROS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define       CZMIL_MAX_FILES                    128         /*!<  Maximum number of open CZMIL files.  */
#define       CZMIL_MAX_PACKETS                  14          /*!<  Maximum number of 64 sample packets in a single waveform
                                                                   (never decrease this number).  */
#define       CZMIL_APPEND_CXY_RECORD            -1          /*!<  Record number used for appending to CXY file instead of
                                                                   updating a specific record.  */


  /*  Channel indexes.  */

#define       CZMIL_DEEP_CHANNEL                 0
#define       CZMIL_SHALLOW_CHANNEL_1            1
#define       CZMIL_SHALLOW_CHANNEL_2            2
#define       CZMIL_SHALLOW_CHANNEL_3            3
#define       CZMIL_SHALLOW_CHANNEL_4            4
#define       CZMIL_SHALLOW_CHANNEL_5            5
#define       CZMIL_SHALLOW_CHANNEL_6            6
#define       CZMIL_SHALLOW_CHANNEL_7            7
#define       CZMIL_IR_CHANNEL                   8


  /*  File open modes.  */

#define       CZMIL_UPDATE                       0          /*!<  Open file for update.  */
#define       CZMIL_READONLY                     1          /*!<  Open file for read only.  */


  /*  Status bit definitions.  */

#define       CZMIL_MANUALLY_INVAL               1           /*!<  0000 0001 = 0x01  */
#define       CZMIL_FILTER_INVAL                 2           /*!<  0000 0010 = 0x02  */
#define       CZMIL_SUSPECT                      4           /*!<  0000 0100 = 0x04  */
#define       CZMIL_REFERENCE                    8           /*!<  0000 1000 = 0x08  */

#define       CZMIL_INVAL                        3           /*!<  0000 0011 = 0x03  */
#define       CZMIL_STAT_MASK                    15          /*!<  0000 1111 = 0x0f  */


  /*  Error conditions.  */

#define       CZMIL_SUCCESS                      0
#define       CZMIL_CWF_WRITE_FSEEK_ERROR        -1
#define       CZMIL_CWF_WRITE_ERROR              -2
#define       CZMIL_CXY_WRITE_FSEEK_ERROR        -3
#define       CZMIL_CXY_WRITE_ERROR              -4
#define       CZMIL_CDX_WRITE_FSEEK_ERROR        -5
#define       CZMIL_CDX_WRITE_ERROR              -6
#define       CZMIL_CXY_HEADER_WRITE_FSEEK_ERROR -7
#define       CZMIL_CXY_HEADER_WRITE_ERROR       -8
#define       CZMIL_CWF_HEADER_WRITE_FSEEK_ERROR -9
#define       CZMIL_CWF_HEADER_WRITE_ERROR       -10
#define       CZMIL_CDX_HEADER_WRITE_FSEEK_ERROR -11
#define       CZMIL_CDX_HEADER_WRITE_ERROR       -12
#define       CZMIL_CXY_HEADER_READ_FSEEK_ERROR  -13
#define       CZMIL_CWF_HEADER_READ_FSEEK_ERROR  -14
#define       CZMIL_CDX_HEADER_READ_FSEEK_ERROR  -15
#define       CZMIL_TOO_MANY_OPEN_FILES          -16
#define       CZMIL_CREATE_CXY_ERROR             -17
#define       CZMIL_CREATE_CWF_ERROR             -18
#define       CZMIL_CREATE_CDX_ERROR             -19
#define       CZMIL_OPEN_CXY_UPDATE_ERROR        -20
#define       CZMIL_OPEN_CXY_READONLY_ERROR      -21
#define       CZMIL_OPEN_CWF_UPDATE_ERROR        -22
#define       CZMIL_OPEN_CWF_READONLY_ERROR      -23
#define       CZMIL_OPEN_CDX_UPDATE_ERROR        -24
#define       CZMIL_OPEN_CDX_READONLY_ERROR      -25
#define       CZMIL_NOT_CZMIL_FILE_ERROR         -26
#define       CZMIL_NEWER_FILE_VERSION_WARNING   -27
#define       CZMIL_CLOSE_ERROR                  -28
#define       CZMIL_INVALID_RECORD_NUMBER        -29
#define       CZMIL_CXY_READ_FSEEK_ERROR         -30
#define       CZMIL_CXY_READ_ERROR               -31
#define       CZMIL_CWF_READ_FSEEK_ERROR         -32
#define       CZMIL_CWF_READ_ERROR               -33
#define       CZMIL_CDX_READ_FSEEK_ERROR         -34
#define       CZMIL_CDX_READ_ERROR               -35
#define       CZMIL_CXY_APPEND_ERROR             -36
#define       CZMIL_CXY_VALUE_OUT_OF_RANGE_ERROR -37
#define       CZMIL_CWF_VALUE_OUT_OF_RANGE_ERROR -38
#define       CZMIL_REGEN_CDX_ERROR              -39
#define       CZMIL_CWF_APPEND_ERROR             -40
#define       CZMIL_CDX_APPEND_ERROR             -41
#define       CZMIL_CXY_CDX_BUFFER_SIZE_ERROR    -42
#define       CZMIL_CWF_CDX_BUFFER_SIZE_ERROR    -43
#define       CZMIL_CXY_LON_OUT_OF_RANGE_ERROR   -44


  /*  Supported local vertical datums.  These match the vertical datum values used in Generic Sensor Format (GSF).  */

#define       CZMIL_V_DATUM_UNDEFINED            0           /*!<  No vertical datum shift has been applied  */
#define       CZMIL_V_DATUM_UNKNOWN              1           /*!<  Unknown vertical datum  */
#define       CZMIL_V_DATUM_MLLW                 2           /*!<  Mean lower low water  */
#define       CZMIL_V_DATUM_MLW                  3           /*!<  Mean low water  */
#define       CZMIL_V_DATUM_ALAT                 4           /*!<  Aprox Lowest Astronomical Tide  */
#define       CZMIL_V_DATUM_ESLW                 5           /*!<  Equatorial Springs Low Water  */
#define       CZMIL_V_DATUM_ISLW                 6           /*!<  Indian Springs Low Water  */
#define       CZMIL_V_DATUM_LAT                  7           /*!<  Lowest Astronomical Tide  */
#define       CZMIL_V_DATUM_LLW                  8           /*!<  Lowest Low Water  */
#define       CZMIL_V_DATUM_LNLW                 9           /*!<  Lowest Normal Low Water  */
#define       CZMIL_V_DATUM_LWD                  10          /*!<  Low Water Datum  */
#define       CZMIL_V_DATUM_MLHW                 11          /*!<  Mean Lower High Water  */
#define       CZMIL_V_DATUM_MLLWS                12          /*!<  Mean Lower Low Water Springs  */
#define       CZMIL_V_DATUM_MLWN                 13          /*!<  Mean Low Water Neap  */
#define       CZMIL_V_DATUM_MSL                  14          /*!<  Mean Sea Level  */


#ifdef  __cplusplus
}
#endif


#endif
