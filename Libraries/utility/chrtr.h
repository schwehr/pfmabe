
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



#ifndef _CHRTR_H_
#define _CHRTR_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nvtypes.h"
#include "nvdef.h"
#include "swap_bytes.h"


#define CHRTRNULL               10000000000000000.0  /*!<  CHRTR null value  */


#define MAX_CHRTR_FILES         32                   /*!<  MAximum number of CHRTR files that may be opened at once  */


  typedef struct 
  {
    NV_FLOAT32          wlon;                        /*!<  Western longitude (west negative)  */
    NV_FLOAT32          elon;                        /*!<  Eastern longitude (west negative)  */
    NV_FLOAT32          slat;                        /*!<  Southern latitude (south negative)  */
    NV_FLOAT32          nlat;                        /*!<  Northern latitude (south negative)  */
    NV_FLOAT32          grid_minutes;                /*!<  Grid size in minutes  */
    NV_INT32            width;                       /*!<  Width in grid cells  */
    NV_INT32            height;                      /*!<  Height in grid cells  */
    NV_INT32            endian;                      /*!<  Endian indicator (0x00010203)  */
    NV_FLOAT32          min_z;                       /*!<  Minimum observed Z value in file  */
    NV_FLOAT32          max_z;                       /*!<  Maximum observed Z value in file  */
  } CHRTR_HEADER;


  void bit_set (NV_FLOAT32 *value, NV_INT32 bit, NV_INT32 binary);
  NV_INT32 bit_test (NV_FLOAT32 value, NV_INT32 bit);
  NV_INT32 open_chrtr (const NV_CHAR *path, CHRTR_HEADER *header);
  NV_INT32 create_chrtr (const NV_CHAR *path, CHRTR_HEADER *header);
  void close_chrtr (NV_INT32 hnd);
  NV_BOOL read_chrtr (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 num_cols, NV_FLOAT32 *data);
  NV_BOOL write_chrtr (NV_INT32 hnd, NV_INT32 row, NV_INT32 start_col, NV_INT32 num_cols, NV_FLOAT32 *data);
  NV_BOOL get_chrtr_value (NV_INT32 hnd, NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT32 *value);
  void dump_chrtr_header (NV_INT32 hnd);



#ifdef  __cplusplus
}
#endif

#endif
