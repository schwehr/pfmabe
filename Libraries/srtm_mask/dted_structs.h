/*****************************************************************************\

    This module is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/

#ifndef __DTED_STRUCTS_H__
#define __DTED_STRUCTS_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "nvtypes.h"
#include "nvdef.h"


typedef struct
{
  NV_CHAR            sentinel[4];
  NV_FLOAT64         ll_lon;
  NV_FLOAT64         ll_lat;
  NV_FLOAT64         lon_int;
  NV_FLOAT64         lat_int;
  NV_CHAR            vertical_accuracy[5];
  NV_CHAR            security_code[2];
  NV_CHAR            unique_reference[13];
  NV_U_INT16         num_lon_lines;
  NV_U_INT16         num_lat_points;
  NV_CHAR            multiple_accuracy;
} UHL;


typedef struct
{
  NV_CHAR            sentinel[3];
  NV_CHAR            security_code;
  NV_CHAR            security_control[2];
  NV_CHAR            security_handling[27];
  NV_CHAR            reserved1[26];
  NV_CHAR            NIMA_designator[5];
  NV_CHAR            unique_reference[15];
  NV_CHAR            reserved2[8];
  NV_CHAR            edition_number[2];
  NV_CHAR            match_merge_version;
  NV_CHAR            maintenance_date[4];
  NV_CHAR            match_merge_date[4];
  NV_CHAR            maintenance_description[4];
  NV_CHAR            producer_code[8];
  NV_CHAR            reserved3[16];
  NV_CHAR            product_specification[9];
  NV_CHAR            product_spec_numbers[2];
  NV_CHAR            product_spec_date[4];
  NV_CHAR            vertical_datum[3];
  NV_CHAR            horizontal_datum[5];
  NV_CHAR            collection_system[10];
  NV_CHAR            latitude_origin[9];
  NV_CHAR            longitude_origin[10];
  NV_CHAR            latitude_sw[7];
  NV_CHAR            longitude_sw[8];
  NV_CHAR            latitude_nw[7];
  NV_CHAR            longitude_nw[8];
  NV_CHAR            latitude_ne[7];
  NV_CHAR            longitude_ne[8];
  NV_CHAR            latitude_se[7];
  NV_CHAR            longitude_se[8];
  NV_CHAR            orientation[9];
  NV_CHAR            lat_int[4];
  NV_CHAR            lon_int[4];
  NV_CHAR            num_lat_lines[4];
  NV_CHAR            num_lon_lines[4];
  NV_CHAR            partial_cell_indicator[2];
  NV_CHAR            reserved_NIMA[101];
  NV_CHAR            reserved_producer[100];
  NV_CHAR            reserved_free_text[156];
} DSI;


typedef struct
{
  NV_CHAR            sentinel[3];
  NV_CHAR            absolute_horizontal_accuracy[4];
  NV_CHAR            absolute_vertical_accuracy[4];
  NV_CHAR            relative_horizontal_accuracy[4];
  NV_CHAR            relative_vertical_accuracy[4];
  NV_CHAR            not_really_pad[2681];
} ACC;


typedef struct
{
  NV_U_BYTE          sentinel;
  NV_INT32           block_count;
  NV_INT16           longitude_count;
  NV_INT16           latitude_count;
  NV_INT16           elev[3601];
  NV_U_INT32         checksum;
} DTED_DATA;



#ifdef  __cplusplus
}
#endif

#endif
