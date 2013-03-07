/*  This file is no longer used - saved for info purposes only.  We now use liblas.  */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "FileLAS.h"


static NV_BOOL first = NVTrue, swap[MAX_LAS_FILES];
static LAS_HEADER_T hd[MAX_LAS_FILES];
static FILE *las_fp[MAX_LAS_FILES];
static NV_INT32 las_hnd[MAX_LAS_FILES];


static void swap_las_header (LAS_HEADER_T *head)
{
  NV_INT16 i;

  swap_NV_INT32 (&head->reserved);
  swap_NV_INT32 (&head->GUID_data_1);
  swap_NV_INT16 (&head->GUID_data_2);
  swap_NV_INT16 (&head->GUID_data_3);
  swap_NV_INT16 (&head->flight_date_julian);
  swap_NV_INT16 (&head->year);
  swap_NV_INT16 (&head->header_size);
  swap_NV_INT32 (&head->offset_to_point_data);
  swap_NV_INT32 (&head->number_of_var_length_records);
  swap_NV_INT16 (&head->point_data_record_length);
  swap_NV_INT32 (&head->number_of_point_records);

  for (i = 0 ; i < 5 ; i++) swap_NV_INT32 (&head->number_of_points_by_return[i]);

  swap_NV_FLOAT64 (&head->x_scale_factor);
  swap_NV_FLOAT64 (&head->y_scale_factor);
  swap_NV_FLOAT64 (&head->z_scale_factor);
  swap_NV_FLOAT64 (&head->x_offset);
  swap_NV_FLOAT64 (&head->y_offset);
  swap_NV_FLOAT64 (&head->z_offset);
  swap_NV_FLOAT64 (&head->max_x);
  swap_NV_FLOAT64 (&head->min_x);
  swap_NV_FLOAT64 (&head->max_y);
  swap_NV_FLOAT64 (&head->min_y);
  swap_NV_FLOAT64 (&head->max_z);
  swap_NV_FLOAT64 (&head->min_z);
}


static void swap_las_record_0 (LAS_POINT_DATA_FORMAT_0_T *record)
{
  swap_NV_INT32 (&record->x);
  swap_NV_INT32 (&record->y);
  swap_NV_INT32 (&record->z);
  swap_NV_INT16 (&record->intensity);
  swap_NV_INT16 (&record->user_bit_field);
}


static void swap_las_record_1 (LAS_POINT_DATA_FORMAT_1_T *record)
{
  swap_NV_INT32 (&record->x);
  swap_NV_INT32 (&record->y);
  swap_NV_INT32 (&record->z);
  swap_NV_INT16 (&record->intensity);
  swap_NV_INT16 (&record->user_bit_field);
  swap_NV_FLOAT64 (&record->GPS_time);
}


NV_INT32 open_las_file (NV_CHAR *path, NV_CHAR *mode)
{
  NV_INT32 i, hnd;

  NV_INT32 big_endian ();


  if (first)
    {
      for (i = 0 ; i < MAX_LAS_FILES ; i++) las_hnd[i] = -1;

      first = NVFalse;
    }

  hnd = -1;

  for (i = 0 ; i < MAX_LAS_FILES ; i++)
    {
      if (las_hnd[i] == -1)
        {
          las_hnd[i] = i;
          hnd = i;
          break;
        }
    }


  if ((las_fp[hnd] = fopen64 (path, mode)) == NULL) 
    {
      las_hnd[hnd] = -1;
      perror (path);
    }


  swap[hnd] = NVFalse;
  if (big_endian ()) swap[hnd] = NVTrue;


  if (!strchr (mode, 'w') && las_hnd[hnd] != -1) las_read_header (hnd, &hd[hnd]);


  return (hnd);
}


void close_las_file (NV_INT32 hnd)
{
  fclose (las_fp[hnd]);
  las_hnd[hnd] = -1;
}


NV_INT32 las_write_header (NV_INT32 hnd, LAS_HEADER_T *head, NV_INT16 horizontal_datum)
{
  NV_INT16   i;
  NV_U_INT16 start;
  LAS_GEOKEYDIRECTORYTAG_RECORD_T   var_record;


  fseeko64 (las_fp[hnd], 0LL, SEEK_SET);


  if (swap[hnd]) swap_las_header (head);

  fwrite ("LASF", 4, 1, las_fp[hnd]);


  head->reserved = 0;

  fwrite (&head->reserved, 4, 1, las_fp[hnd]);


  fwrite (&head->GUID_data_1, 4, 1, las_fp[hnd]);
  fwrite (&head->GUID_data_2, 2, 1, las_fp[hnd]);
  fwrite (&head->GUID_data_3, 2, 1, las_fp[hnd]);
  fwrite (head->GUID_data_4, 8, 1, las_fp[hnd]);


  head->version_major = 1;

  fwrite (&head->version_major, 1, 1, las_fp[hnd]);


  head->version_minor = 0;

  fwrite (&head->version_minor, 1, 1, las_fp[hnd]);


  fwrite (head->system_id, 32, 1, las_fp[hnd]);
  fwrite (head->generating_software, 32, 1, las_fp[hnd]);
  fwrite (&head->flight_date_julian, 2, 1, las_fp[hnd]);
  fwrite (&head->year, 2, 1, las_fp[hnd]);


  head->header_size = 227;

  fwrite (&head->header_size, 2, 1, las_fp[hnd]);


  head->offset_to_point_data = 227 + 78 + 2;

  fwrite (&head->offset_to_point_data, 4, 1, las_fp[hnd]);


  head->number_of_var_length_records = 1;

  fwrite (&head->number_of_var_length_records, 4, 1, las_fp[hnd]);


  head->point_data_format_id = 1;

  fwrite (&head->point_data_format_id, 1, 1, las_fp[hnd]);


  head->point_data_record_length = 28;

  fwrite (&head->point_data_record_length, 2, 1, las_fp[hnd]);


  fwrite (&head->number_of_point_records, 4, 1, las_fp[hnd]);

  for (i = 0 ; i < 5 ; i++) fwrite (&head->number_of_points_by_return[i], 4, 1, las_fp[hnd]);

  fwrite (&head->x_scale_factor, 8, 1, las_fp[hnd]);
  fwrite (&head->y_scale_factor, 8, 1, las_fp[hnd]);
  fwrite (&head->z_scale_factor, 8, 1, las_fp[hnd]);
  fwrite (&head->x_offset, 8, 1, las_fp[hnd]);
  fwrite (&head->y_offset, 8, 1, las_fp[hnd]);
  fwrite (&head->z_offset, 8, 1, las_fp[hnd]);
  fwrite (&head->max_x, 8, 1, las_fp[hnd]);
  fwrite (&head->min_x, 8, 1, las_fp[hnd]);
  fwrite (&head->max_y, 8, 1, las_fp[hnd]);
  fwrite (&head->min_y, 8, 1, las_fp[hnd]);
  fwrite (&head->max_z, 8, 1, las_fp[hnd]);
  fwrite (&head->min_z, 8, 1, las_fp[hnd]);


  hd[hnd] = *head;


  /*  Hard coded since I'm only gonna write this one variable length record.  */

  memset (&var_record, 0, sizeof (LAS_GEOKEYDIRECTORYTAG_RECORD_T));

  var_record.record_signature = 0xaabb;
  if (swap[hnd]) swap_NV_INT16 ((NV_INT16 *) &var_record.record_signature);
  strcpy (var_record.user_id, "LASF_Projection");
  var_record.record_id = 34735;
  if (swap[hnd]) swap_NV_INT16 (&var_record.record_id);


  /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).  */

  var_record.record_length_after_header = 24/*32*/;
  if (swap[hnd]) swap_NV_INT16 (&var_record.record_length_after_header);
  strcpy (var_record.description, "GeoKeyDirTagDesc");
  var_record.key_directory_version = 1;
  if (swap[hnd]) swap_NV_INT16 (&var_record.key_directory_version);
  var_record.key_revision = 1;
  if (swap[hnd]) swap_NV_INT16 (&var_record.key_revision);
  var_record.minor_revision = 0;
  var_record.number_of_keys = 3;
  if (swap[hnd]) swap_NV_INT16 (&var_record.number_of_keys);


  /*  You can look these up in the GeoTIFF spec (appendices) basically they mean...  */


  /*  Key 1  */

  /*  GTModelTypeGeoKey (1024)  */

  var_record.key_id_0 = 1024;
  if (swap[hnd]) swap_NV_INT16 (&var_record.key_id_0);


  /*  Empty  (0, and 1)  */

  var_record.TIFF_tag_location_0 = 0;
  var_record.count_0 = 1;
  if (swap[hnd]) swap_NV_INT16 (&var_record.count_0);


  /*  ModelTypeGeographic  (2)   Geographic latitude-longitude System  */

  var_record.value_offset_0 = 2;
  if (swap[hnd]) swap_NV_INT16 (&var_record.value_offset_0);


  /*  Key 2  */

  /*  GeographicTypeGeoKey (2048)  */

  var_record.key_id_1 = 2048;
  if (swap[hnd]) swap_NV_INT16 (&var_record.key_id_1);


  /*  Empty (0, and 1)  */

  var_record.TIFF_tag_location_1 = 0;
  var_record.count_1 = 1;
  if (swap[hnd]) swap_NV_INT16 (&var_record.count_1);


  /*  GCS_WGS_84 (4326)  GCS_NAD83 (4269)  */

  var_record.value_offset_1 = horizontal_datum;
  if (swap[hnd]) swap_NV_INT16 (&var_record.value_offset_1);


  /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).  */
  /*  Key 3  */

  /*  ProjLinearUnitsGeoKey (3076)  */

  var_record.key_id_2 = 3076;
  if (swap[hnd]) swap_NV_INT16 (&var_record.key_id_2);


  /*  Empty (0, and 1)  */

  var_record.TIFF_tag_location_2 = 0;
  var_record.count_2 = 1;
  if (swap[hnd]) swap_NV_INT16 (&var_record.count_2);


  /*  Linear_Meter (9001)  */

  var_record.value_offset_2 = 9001;
  if (swap[hnd]) swap_NV_INT16 (&var_record.value_offset_2);


  /*  Write the var length record data.  */

  fwrite (&var_record.record_signature, 2, 1, las_fp[hnd]);
  fwrite (var_record.user_id, 16, 1, las_fp[hnd]);
  fwrite (&var_record.record_id, 2, 1, las_fp[hnd]);
  fwrite (&var_record.record_length_after_header, 2, 1, las_fp[hnd]);
  fwrite (var_record.description, 32, 1, las_fp[hnd]);
  fwrite (&var_record.key_directory_version, 2, 1, las_fp[hnd]);
  fwrite (&var_record.key_revision, 2, 1, las_fp[hnd]);
  fwrite (&var_record.minor_revision, 2, 1, las_fp[hnd]);
  fwrite (&var_record.number_of_keys, 2, 1, las_fp[hnd]);

  fwrite (&var_record.key_id_0, 2, 1, las_fp[hnd]);
  fwrite (&var_record.TIFF_tag_location_0, 2, 1, las_fp[hnd]);
  fwrite (&var_record.count_0, 2, 1, las_fp[hnd]);
  fwrite (&var_record.value_offset_0, 2, 1, las_fp[hnd]);

  fwrite (&var_record.key_id_1, 2, 1, las_fp[hnd]);
  fwrite (&var_record.TIFF_tag_location_1, 2, 1, las_fp[hnd]);
  fwrite (&var_record.count_1, 2, 1, las_fp[hnd]);
  fwrite (&var_record.value_offset_1, 2, 1, las_fp[hnd]);


  /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).  */
  /*
  fwrite (&var_record.key_id_2, 2, 1, las_fp[hnd]);
  fwrite (&var_record.TIFF_tag_location_2, 2, 1, las_fp[hnd]);
  fwrite (&var_record.count_2, 2, 1, las_fp[hnd]);
  fwrite (&var_record.value_offset_2, 2, 1, las_fp[hnd]);
  */


  start = 0xccdd;
  if (swap[hnd]) swap_NV_INT16 (&start);
  fwrite (&start, 2, 1, las_fp[hnd]);

  return (0);
}


NV_INT32 las_read_header_var (NV_INT32 hnd, LAS_HEADER_T *head, LAS_GEOKEYDIRECTORYTAG_RECORD_T *var_record)
{
  NV_INT16   i;


  fseeko64 (las_fp[hnd], 0LL, SEEK_SET);


  fread (head->file_signature, 4, 1, las_fp[hnd]);
  head->file_signature[4] = 0;

  fread (&head->reserved, 4, 1, las_fp[hnd]);
  fread (&head->GUID_data_1, 4, 1, las_fp[hnd]);
  fread (&head->GUID_data_2, 2, 1, las_fp[hnd]);
  fread (&head->GUID_data_3, 2, 1, las_fp[hnd]);

  fread (head->GUID_data_4, 8, 1, las_fp[hnd]);
  head->GUID_data_4[8] = 0;

  fread (&head->version_major, 1, 1, las_fp[hnd]);
  fread (&head->version_minor, 1, 1, las_fp[hnd]);

  fread (head->system_id, 32, 1, las_fp[hnd]);
  head->system_id[32] = 0;

  fread (head->generating_software, 32, 1, las_fp[hnd]);
  head->generating_software[32] = 0;

  fread (&head->flight_date_julian, 2, 1, las_fp[hnd]);
  fread (&head->year, 2, 1, las_fp[hnd]);
  fread (&head->header_size, 2, 1, las_fp[hnd]);

  fread (&head->offset_to_point_data, 4, 1, las_fp[hnd]);

  fread (&head->number_of_var_length_records, 4, 1, las_fp[hnd]);
  fread (&head->point_data_format_id, 1, 1, las_fp[hnd]);

  fread (&head->point_data_record_length, 2, 1, las_fp[hnd]);

  fread (&head->number_of_point_records, 4, 1, las_fp[hnd]);

  for (i = 0 ; i < 5 ; i++) fread (&head->number_of_points_by_return[i], 4, 1, las_fp[hnd]);

  fread (&head->x_scale_factor, 8, 1, las_fp[hnd]);
  fread (&head->y_scale_factor, 8, 1, las_fp[hnd]);
  fread (&head->z_scale_factor, 8, 1, las_fp[hnd]);
  fread (&head->x_offset, 8, 1, las_fp[hnd]);
  fread (&head->y_offset, 8, 1, las_fp[hnd]);
  fread (&head->z_offset, 8, 1, las_fp[hnd]);
  fread (&head->max_x, 8, 1, las_fp[hnd]);
  fread (&head->min_x, 8, 1, las_fp[hnd]);
  fread (&head->max_y, 8, 1, las_fp[hnd]);
  fread (&head->min_y, 8, 1, las_fp[hnd]);
  fread (&head->max_z, 8, 1, las_fp[hnd]);
  fread (&head->min_z, 8, 1, las_fp[hnd]);


  if (swap[hnd]) swap_las_header (head);


  /*  Read the var length record data.  */

  fread (&var_record->record_signature, 2, 1, las_fp[hnd]);
  fread (var_record->user_id, 16, 1, las_fp[hnd]);
  fread (&var_record->record_id, 2, 1, las_fp[hnd]);
  fread (&var_record->record_length_after_header, 2, 1, las_fp[hnd]);
  fread (var_record->description, 32, 1, las_fp[hnd]);
  fread (&var_record->key_directory_version, 2, 1, las_fp[hnd]);
  fread (&var_record->key_revision, 2, 1, las_fp[hnd]);
  fread (&var_record->minor_revision, 2, 1, las_fp[hnd]);
  fread (&var_record->number_of_keys, 2, 1, las_fp[hnd]);

  fread (&var_record->key_id_0, 2, 1, las_fp[hnd]);
  fread (&var_record->TIFF_tag_location_0, 2, 1, las_fp[hnd]);
  fread (&var_record->count_0, 2, 1, las_fp[hnd]);
  fread (&var_record->value_offset_0, 2, 1, las_fp[hnd]);

  fread (&var_record->key_id_1, 2, 1, las_fp[hnd]);
  fread (&var_record->TIFF_tag_location_1, 2, 1, las_fp[hnd]);
  fread (&var_record->count_1, 2, 1, las_fp[hnd]);
  fread (&var_record->value_offset_1, 2, 1, las_fp[hnd]);


  /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).  */
  /*
  fread (&var_record->key_id_2, 2, 1, las_fp[hnd]);
  fread (&var_record->TIFF_tag_location_2, 2, 1, las_fp[hnd]);
  fread (&var_record->count_2, 2, 1, las_fp[hnd]);
  fread (&var_record->value_offset_2, 2, 1, las_fp[hnd]);
  */


  if (swap[hnd])
    {
      swap_NV_INT16 (&var_record->record_signature);
      swap_NV_INT16 (&var_record->record_id);
      swap_NV_INT16 (&var_record->record_length_after_header);
      swap_NV_INT16 (&var_record->key_directory_version);
      swap_NV_INT16 (&var_record->key_revision);
      swap_NV_INT16 (&var_record->number_of_keys);
      swap_NV_INT16 (&var_record->key_id_0);
      swap_NV_INT16 (&var_record->count_0);
      swap_NV_INT16 (&var_record->value_offset_0);
      swap_NV_INT16 (&var_record->key_id_1);
      swap_NV_INT16 (&var_record->count_1);
      swap_NV_INT16 (&var_record->value_offset_1);
      swap_NV_INT16 (&var_record->key_id_2);
      swap_NV_INT16 (&var_record->count_2);
      swap_NV_INT16 (&var_record->value_offset_2);
    }


  hd[hnd] = *head;


  return (0);
}


NV_INT32 las_read_header (NV_INT32 hnd, LAS_HEADER_T *head)
{
  LAS_GEOKEYDIRECTORYTAG_RECORD_T   var_record;

  return (las_read_header_var (hnd, head, &var_record));
}


NV_INT32 las_read_record_0 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_0_T *record)
{
  NV_INT32 ret;
  NV_INT64 long_pos;


  if (num > hd[hnd].number_of_point_records) return (0);

  if (num != LAS_NEXT_RECORD)
    {
      fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data + (NV_INT64) num *
                (NV_INT64) hd[hnd].point_data_record_length, SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (las_fp[hnd]);
      if (long_pos < (NV_INT64) hd[hnd].offset_to_point_data) 
        fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data, SEEK_SET);
    }


  fread (&record->x, 4, 1, las_fp[hnd]);
  fread (&record->y, 4, 1, las_fp[hnd]);
  fread (&record->z, 4, 1, las_fp[hnd]);
  fread (&record->intensity, 2, 1, las_fp[hnd]);
  fread (&record->flags, 1, 1, las_fp[hnd]);
  fread (&record->classification, 1, 1, las_fp[hnd]);
  fread (&record->scan_angle_rank, 1, 1, las_fp[hnd]);
  fread (&record->file_marker, 1, 1, las_fp[hnd]);
  ret = fread (&record->user_bit_field, 2, 1, las_fp[hnd]);


  if (swap[hnd]) swap_las_record_0 (record);

  return (ret);
}


NV_INT32 las_read_record_1 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_1_T *record)
{
  NV_INT32 ret;
  NV_INT64 long_pos;


  if (num > hd[hnd].number_of_point_records) return (0);

  if (num != LAS_NEXT_RECORD)
    {
      fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data + (NV_INT64) num *
                (NV_INT64) hd[hnd].point_data_record_length, SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (las_fp[hnd]);
      if (long_pos < (NV_INT64) hd[hnd].offset_to_point_data)
        fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data, SEEK_SET);
    }


  fread (&record->x, 4, 1, las_fp[hnd]);
  fread (&record->y, 4, 1, las_fp[hnd]);
  fread (&record->z, 4, 1, las_fp[hnd]);
  fread (&record->intensity, 2, 1, las_fp[hnd]);
  fread (&record->flags, 1, 1, las_fp[hnd]);
  fread (&record->classification, 1, 1, las_fp[hnd]);
  fread (&record->scan_angle_rank, 1, 1, las_fp[hnd]);
  fread (&record->file_marker, 1, 1, las_fp[hnd]);
  fread (&record->user_bit_field, 2, 1, las_fp[hnd]);
  ret = fread (&record->GPS_time, 8, 1, las_fp[hnd]);


  if (swap[hnd]) swap_las_record_1 (record);

  return (ret);
}


void las_write_record_0 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_0_T record)
{
  NV_INT64 long_pos;


  if (num != LAS_NEXT_RECORD)
    {
      fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data + (NV_INT64) num *
                (NV_INT64) hd[hnd].point_data_record_length, SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (las_fp[hnd]);
      if (long_pos < (NV_INT64) hd[hnd].offset_to_point_data)
        fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data, SEEK_SET);
    }


  if (swap[hnd]) swap_las_record_0 (&record);


  fwrite (&record.x, 4, 1, las_fp[hnd]);
  fwrite (&record.y, 4, 1, las_fp[hnd]);
  fwrite (&record.z, 4, 1, las_fp[hnd]);
  fwrite (&record.intensity, 2, 1, las_fp[hnd]);
  fwrite (&record.flags, 1, 1, las_fp[hnd]);
  fwrite (&record.classification, 1, 1, las_fp[hnd]);
  fwrite (&record.scan_angle_rank, 1, 1, las_fp[hnd]);
  fwrite (&record.file_marker, 1, 1, las_fp[hnd]);
  fwrite (&record.user_bit_field, 2, 1, las_fp[hnd]);
}


void las_write_record_1 (NV_INT32 hnd, NV_INT32 num, LAS_POINT_DATA_FORMAT_1_T record)
{
  NV_INT64 long_pos;


  if (num != LAS_NEXT_RECORD)
    {
      fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data + (NV_INT64) num *
                (NV_INT64) hd[hnd].point_data_record_length, SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (las_fp[hnd]);
      if (long_pos < (NV_INT64) hd[hnd].offset_to_point_data)
        fseeko64 (las_fp[hnd], (NV_INT64) hd[hnd].offset_to_point_data, SEEK_SET);
    }


  if (swap[hnd]) swap_las_record_1 (&record);


  fwrite (&record.x, 4, 1, las_fp[hnd]);
  fwrite (&record.y, 4, 1, las_fp[hnd]);
  fwrite (&record.z, 4, 1, las_fp[hnd]);
  fwrite (&record.intensity, 2, 1, las_fp[hnd]);
  fwrite (&record.flags, 1, 1, las_fp[hnd]);
  fwrite (&record.classification, 1, 1, las_fp[hnd]);
  fwrite (&record.scan_angle_rank, 1, 1, las_fp[hnd]);
  fwrite (&record.file_marker, 1, 1, las_fp[hnd]);
  fwrite (&record.user_bit_field, 2, 1, las_fp[hnd]);
  fwrite (&record.GPS_time, 8, 1, las_fp[hnd]);
}


void las_dump_record_0 (NV_INT32 hnd, LAS_POINT_DATA_FORMAT_0_T record)
{
  printf ("x : %f\n", record.x * hd[hnd].x_scale_factor + hd[hnd].x_offset);
  printf ("y : %f\n", record.y * hd[hnd].y_scale_factor + hd[hnd].y_offset);
  printf ("z : %f\n", record.z * hd[hnd].z_scale_factor + hd[hnd].z_offset);
  printf ("intensity : %d\n", record.intensity);
  printf ("return # : %d\n", record.flags & 0x07);
  printf ("# of returns : %d\n", (record.flags & 0x38) >> 3);
  printf ("scan direction flag : %d\n", (record.flags & 0x40) >> 6);
  printf ("edge of flightline flag : %d\n", (record.flags & 0x80) >> 7);
  printf ("classification : %d\n", record.classification);
  printf ("scan angle rank : %d\n", record.scan_angle_rank);
  printf ("file marker : %d\n", record.file_marker);
  printf ("user bit field : %d\n", record.user_bit_field);
}


void las_dump_record_1 (NV_INT32 hnd, LAS_POINT_DATA_FORMAT_1_T record)
{
  /*
  NV_INT32        year, day, hour, minute, month, mday;
  NV_FLOAT32      second;

  charts_cvtime (record.timestamp, &year, &day, &hour, &minute, &second);
  charts_jday2mday (year, day, &month, &mday);
  month++;

  printf ("*****************************\n");
  printf ("timestamp : %Ld    %d-%02d-%02d (%d) %02d:%02d:%05.2f\n", 
          record.timestamp, year + 1900, month, mday, day, hour, minute, second);
  */

  printf ("x : %f\n", record.x * hd[hnd].x_scale_factor + hd[hnd].x_offset);
  printf ("y : %f\n", record.y * hd[hnd].y_scale_factor + hd[hnd].y_offset);
  printf ("z : %f\n", record.z * hd[hnd].z_scale_factor + hd[hnd].z_offset);
  printf ("intensity : %d\n", record.intensity);
  printf ("return # : %d\n", record.flags & 0x07);
  printf ("# of returns : %d\n", (record.flags & 0x38) >> 3);
  printf ("scan direction flag : %d\n", (record.flags & 0x40) >> 6);
  printf ("edge of flightline flag : %d\n", (record.flags & 0x80) >> 7);
  printf ("classification : %d\n", record.classification);
  printf ("scan angle rank : %d\n", record.scan_angle_rank);
  printf ("file marker : %d\n", record.file_marker);
  printf ("user bit field : %d\n", record.user_bit_field);
  printf ("GPS time : %f\n", record.GPS_time);
}
