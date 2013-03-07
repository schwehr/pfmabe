
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



#ifndef __CHRTR2_INTERNALS_H__
#define __CHRTR2_INTERNALS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#ifndef NINT
  #define NINT(a)                      ((a)<0.0 ? (NV_INT32) ((a) - 0.5) : (NV_INT32) ((a) + 0.5))
#endif

#ifndef NINT64
  #define NINT64(a)                    ((a)<0.0 ? (NV_INT64) ((a) - 0.5) : (NV_INT64) ((a) + 0.5))
#endif

#ifndef MAX
  #define MAX(x,y)                     (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
  #define MIN(x,y)                     (((x) < (y)) ? (x) : (y))
#endif

#ifndef DPRINT
  #define DPRINT                       fprintf (stderr, "%s %d\n", __FILE__, __LINE__);fflush (stderr);
#endif

#ifndef LOG2
#define LOG2                           0.30102999566398L
#endif


#define                                COV_MAP_BITS                   4       /*!<  Bits used in the coverage map.  */
#define                                COV_MAP_MASK                   0x0f    /*!<  Coverage map mask (4 bits).  */
#define                                CHRTR2_HEADER_SIZE             65536   /*!<  Default CHRTR2 header size.  */
#define                                CHRTR2_STATUS_BITS             12      /*!<  Number of status bits.  */


#undef CHRTR2_DEBUG


/*  This is the structure we use to keep track of important formatting data for an open CHRTR2 file.  */

  typedef struct
  {
    FILE          *fp;                        /*!<  CHRTR2 file pointer.  */
    NV_CHAR       path[1024];                 /*!<  File name.  */
    NV_BOOL       modified;                   /*!<  Set if the file has been modified.  */
    NV_BOOL       created;                    /*!<  Set if we created the file.  */
    NV_INT16      major_version;              /*!<  Major version number for backward compatibility.  */
    NV_U_INT32    header_size;                /*!<  Header size in bytes.  */
    NV_U_INT16    record_size;                /*!<  Record size in bytes.  */


    /*!  Number of bits needed for each field of the record.  */

    NV_U_BYTE     z_bits;
    NV_U_BYTE     status_bits;
    NV_U_BYTE     h_uncert_bits;
    NV_U_BYTE     v_uncert_bits;
    NV_U_BYTE     uncert_bits;
    NV_U_BYTE     z0_bits;
    NV_U_BYTE     z1_bits;
    NV_U_BYTE     num_bits;


    /*!  Max and min values for the hardwired fields (only populated when creating file).  */

    NV_INT32      max_status;


    /*!  Bit positions within the record of each field.  */

    NV_U_INT16    z_pos;
    NV_U_INT16    status_pos;
    NV_U_INT16    h_uncert_pos;
    NV_U_INT16    v_uncert_pos;
    NV_U_INT16    uncert_pos;
    NV_U_INT16    z0_pos;
    NV_U_INT16    z1_pos;
    NV_U_INT16    num_pos;


    /*!  CHRTR2 header.  */

    CHRTR2_HEADER    header;


    /*!  Optional coverage map.  Created with chrtr2_create_cov_map.  */

    NV_U_BYTE        *cov_map;
  } INTERNAL_CHRTR2_STRUCT;


  /*  CHRTR2 error handling variables.  */

  typedef struct 
  {
    NV_INT32      system;            /*!<  Last system error condition encountered.  */
    NV_INT32      chrtr2;            /*!<  Last CHRTR2 error condition encountered.  */
    NV_CHAR       file[512];         /*!<  Name of file being accessed when last error encountered (if applicable).  */
    NV_I32_COORD2 coord;             /*!<  Coordinates (grid cell) being accessed when last error encountered (if applicable).  */
    NV_FLOAT64    lat;               /*!<  Latitude being  accessed when last error encountered (if applicable).  */
    NV_FLOAT64    lon;               /*!<  Longitude being  accessed when last error encountered (if applicable).  */
    NV_INT32      length;            /*!<  Length of row being read when last error encountered (if applicable).  */
    NV_CHAR       info[128];
  } CHRTR2_ERROR_STRUCT;


#ifdef  __cplusplus
}
#endif


#endif
