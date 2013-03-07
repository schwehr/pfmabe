/*  This file is no longer used - saved for info purposes only.  We now use liblas.  */

#ifndef __FILE_LAS_H__
#define __FILE_LAS_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "nvtypes.h"
#include "charts.h"


#define  MAX_LAS_FILES        200
#define  LAS_NEXT_RECORD      (-1)

#define  GCS_WGS_84           4326
#define  GCS_NAD83            4269



/*  These describe LASF 1.0 format not 1.1  */

typedef struct 
{
  NV_CHAR                     file_signature[5];
  NV_U_INT32                  reserved;
  NV_U_INT32                  GUID_data_1;
  NV_U_INT16                  GUID_data_2;
  NV_U_INT16                  GUID_data_3;
  NV_CHAR                     GUID_data_4[9];
  NV_U_BYTE                   version_major;
  NV_U_BYTE                   version_minor;
  NV_CHAR                     system_id[33];
  NV_CHAR                     generating_software[33];
  NV_U_INT16                  flight_date_julian;
  NV_U_INT16                  year;
  NV_U_INT16                  header_size;
  NV_U_INT32                  offset_to_point_data;
  NV_U_INT32                  number_of_var_length_records;
  NV_U_BYTE                   point_data_format_id;
  NV_U_INT16                  point_data_record_length;
  NV_U_INT32                  number_of_point_records;
  NV_U_INT32                  number_of_points_by_return[5];
  NV_FLOAT64                  x_scale_factor;
  NV_FLOAT64                  y_scale_factor;
  NV_FLOAT64                  z_scale_factor;
  NV_FLOAT64                  x_offset;
  NV_FLOAT64                  y_offset;
  NV_FLOAT64                  z_offset;
  NV_FLOAT64                  max_x;
  NV_FLOAT64                  min_x;
  NV_FLOAT64                  max_y;
  NV_FLOAT64                  min_y;
  NV_FLOAT64                  max_z;
  NV_FLOAT64                  min_z;
} LAS_HEADER_T;


typedef struct 
{
  NV_U_INT16                  record_signature;
  NV_CHAR                     user_id[17];
  NV_U_INT16                  record_id;
  NV_U_INT16                  record_length_after_header;
  NV_CHAR                     description[33];
  NV_U_INT16                  key_directory_version;
  NV_U_INT16                  key_revision;
  NV_U_INT16                  minor_revision;
  NV_U_INT16                  number_of_keys;
  NV_U_INT16                  key_id_0;
  NV_U_INT16                  TIFF_tag_location_0;
  NV_U_INT16                  count_0;
  NV_U_INT16                  value_offset_0;
  NV_U_INT16                  key_id_1;
  NV_U_INT16                  TIFF_tag_location_1;
  NV_U_INT16                  count_1;
  NV_U_INT16                  value_offset_1;
  NV_U_INT16                  key_id_2;
  NV_U_INT16                  TIFF_tag_location_2;
  NV_U_INT16                  count_2;
  NV_U_INT16                  value_offset_2;
} LAS_GEOKEYDIRECTORYTAG_RECORD_T;


typedef struct
{
  NV_INT32                    x;          /* scaled int (see x scale factor in header) */
  NV_INT32                    y;          /* scaled int (see y scale factor in header) */
  NV_INT32                    z;          /* scaled int (see z scale factor in header) */
  NV_U_INT16                  intensity;
  NV_U_BYTE                   flags;      /* return # (3 bits - 0, 1, 2), # of returns (3 bits - 3, 4, 5), scan */
                                          /* direction flag (1 bit, bit 6), edge of flightline (1 bit, bit 7) */
  NV_U_BYTE                   classification;
  NV_BYTE                     scan_angle_rank;
  NV_U_BYTE                   file_marker;
  NV_U_INT16                  user_bit_field;
} LAS_POINT_DATA_FORMAT_0_T;


typedef struct
{
  NV_INT32                    x;          /* scaled int (see x scale factor in header) */
  NV_INT32                    y;          /* scaled int (see y scale factor in header) */
  NV_INT32                    z;          /* scaled int (see z scale factor in header) */
  NV_U_INT16                  intensity;
  NV_U_BYTE                   flags;      /* return # (3 bits - 0, 1, 2), # of returns (3 bits - 3, 4, 5), scan */
                                          /* direction flag (1 bit, bit 6), edge of flightline (1 bit, bit 7) */
  NV_U_BYTE                   classification;
  NV_BYTE                     scan_angle_rank;
  NV_U_BYTE                   file_marker;
  NV_U_INT16                  user_bit_field;
  NV_FLOAT64                  GPS_time;
} LAS_POINT_DATA_FORMAT_1_T;


NV_INT32 open_las_file (NV_CHAR *path, NV_CHAR *mode);
void close_las_file (NV_INT32 hnd);
NV_INT32 las_write_header (NV_INT32 hnd, LAS_HEADER_T *head, NV_INT16 horizontal_datum);
NV_INT32 las_read_header_var (NV_INT32 hnd, LAS_HEADER_T *head, LAS_GEOKEYDIRECTORYTAG_RECORD_T *var_record);
NV_INT32 las_read_header (NV_INT32 hnd, LAS_HEADER_T *head);
NV_INT32 las_read_record_0 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_0_T *record);
NV_INT32 las_read_record_1 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_1_T *record);
void las_write_record_0 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_0_T record);
void las_write_record_1 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_1_T record);
void las_dump_record_0 (NV_INT32 hnd, LAS_POINT_DATA_FORMAT_0_T record);
void las_dump_record_1 (NV_INT32 hnd, LAS_POINT_DATA_FORMAT_1_T record);


#ifdef  __cplusplus
}
#endif

#endif
