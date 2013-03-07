#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "FileTopoOutput.h"

static NV_BOOL swap = NVFalse;


static void swap_tof_header (TOF_HEADER_T *head)
{
  NV_INT16 i;


  swap_NV_INT64 (&head->info.start_time);
  swap_NV_INT64 (&head->info.stop_time);
  swap_NV_INT64 (&head->info.file_create_time);
  swap_NV_INT64 (&head->info.file_mod_time);

  swap_NV_FLOAT64 (&head->info.min_lat);
  swap_NV_FLOAT64 (&head->info.min_long);
  swap_NV_FLOAT64 (&head->info.max_lat);
  swap_NV_FLOAT64 (&head->info.max_long);

  for (i = 0 ; i < 6 ; i++) swap_NV_INT64 (&head->info.fill_i64[i]);

  swap_NV_INT32 (&head->info.num_shots);
  swap_NV_FLOAT32 (&head->info.gcs_version);
  swap_NV_FLOAT32 (&head->info.gcs_file_version);

  swap_NV_INT32 (&head->info.header_size);
  swap_NV_INT32 (&head->info.text_block_size);
  swap_NV_INT32 (&head->info.binary_block_size);
  swap_NV_INT32 (&head->info.record_size);

  swap_NV_INT32 (&head->info.ab_system_type);
  swap_NV_INT32 (&head->info.ab_system_number);
  swap_NV_INT32 (&head->info.rep_rate);

  swap_NV_INT16 (&head->info.data_type);
  swap_NV_INT16 (&head->info.fill_short);

  for (i = 0 ; i < 11 ; i++) swap_NV_INT32 (&head->info.fill_i32[i]);

  swap_NV_INT16 ((NV_INT16 *) &head->info.flightline);

  for (i = 0 ; i < 7 ; i++) swap_NV_INT16 (&head->info.fill_i16[i]);
}


static void swap_tof_record (TOPO_OUTPUT_T *record)
{
  swap_NV_INT64 (&record->timestamp);
  swap_NV_FLOAT64 (&record->latitude_first);
  swap_NV_FLOAT64 (&record->longitude_first);
  swap_NV_FLOAT64 (&record->latitude_last);
  swap_NV_FLOAT64 (&record->longitude_last);
  swap_NV_FLOAT32 (&record->elevation_first);
  swap_NV_FLOAT32 (&record->elevation_last);
  swap_NV_FLOAT32 (&record->scanner_azimuth);
  swap_NV_FLOAT32 (&record->nadir_angle);
  swap_NV_FLOAT32 (&record->result_elevation_first);
  swap_NV_FLOAT32 (&record->result_elevation_last);
  swap_NV_FLOAT32 (&record->altitude);
  swap_NV_FLOAT32 (&record->tbdfloat);
}


FILE *open_tof_file (NV_CHAR *path)
{
  FILE *fp;


  if ((fp = fopen64 (path, "rb+")) == NULL)
    {
      perror (path);
      if ((fp = fopen64 (path, "rb")) == NULL)
        {
          perror (path);
        }
      else
        {
          fprintf (stderr, "%s opened for read only!\n", path);
        }
      fflush (stderr);
    }

  return (fp);
}


NV_INT32 tof_read_header (FILE *fp, TOF_HEADER_T *head)
{
  NV_INT64    long_pos;
  NV_CHAR     varin[1024], info[1024];
  NV_INT16    tmpi16;


  NV_CHAR *ngets (NV_CHAR *s, NV_INT32 size, FILE *stream);
  NV_INT32 big_endian ();


  swap = NVFalse;


  fseeko64 (fp, 0LL, SEEK_SET);



  /*  Read each entry.    */

  head->text.header_size = 0;
  head->text.line_min_lat = 0.0;
  head->text.line_max_lat = 0.0;
  head->text.line_min_lon = 0.0;
  head->text.line_max_lon = 0.0;
  while (ngets (varin, sizeof (varin), fp) != NULL)
    {
      if (!strcmp (varin, "EOF")) break;


      /*  Put everything to the right of the colon into 'info'.   */
            
      if (strchr (varin, ':') != NULL) strcpy (info, (NV_CHAR *) (strchr (varin, ':') + 1));


      /*  Check input for matching strings and load values if found.  */
            
      if (strstr (varin, "FileType:") != NULL) lidar_get_string (varin, head->text.file_type);

      if (strstr (varin, "EndianType:") != NULL)
        {
          if (strstr (info, "Little")) 
            {
              head->text.endian = NVFalse;
              if (big_endian ()) swap = NVTrue;
            }
          else
            {
              head->text.endian = NVTrue;
              if (!big_endian ()) swap = NVTrue;
            }
        }


      if (strstr (varin, "SoftwareVersionNumber:") != NULL) sscanf (info, "%f", &head->text.software_version);

      if (strstr (varin, "FileVersionNumber:") != NULL) sscanf (info, "%f", &head->text.file_version);


      if (strstr (varin, "Processed By:") != NULL) lidar_get_string (varin, head->text.UserName);


      if (strstr (varin, "HeaderSize:") != NULL) sscanf (info, "%d", &head->text.header_size);

      if (strstr (varin, "TextBlockSize:") != NULL) sscanf (info, "%d", &head->text.text_block_size);

      if (strstr (varin, "BinaryBlockSize:") != NULL) sscanf (info, "%d", &head->text.bin_block_size);

      if (strstr (varin, "RecordSize:") != NULL) sscanf (info, "%d", &head->text.record_size);


      if (strstr (varin, "ABSystemType:") != NULL)
        {
          sscanf (info, "%hd", &tmpi16);
          head->text.ab_system_type = (NV_BYTE) tmpi16;
        }

      if (strstr (varin, "ABSystemNumber:") != NULL)
        {
          sscanf (info, "%hd", &tmpi16);
          head->text.ab_system_number = (NV_BYTE) tmpi16;
        }

      if (strstr (varin, "SystemRepRate:") != NULL) sscanf (info, "%hd", &head->text.system_rep_rate);


      if (strstr (varin, "Project:") != NULL) lidar_get_string (varin, head->text.project);

      if (strstr (varin, "Mission:") != NULL) lidar_get_string (varin, head->text.mission);

      if (strstr (varin, "Dataset:") != NULL) lidar_get_string (varin, head->text.dataset);

      if (strstr (varin, "FlightlineNumber:") != NULL) lidar_get_string (varin, head->text.flightline_number);

      if (strstr (varin, "CodedFLNumber:") != NULL) sscanf (info, "%hd", &head->text.coded_fl_number);

      if (strstr (varin, "FlightDate:") != NULL) lidar_get_string (varin, head->text.flight_date);

      if (strstr (varin, "StartTime:") != NULL) lidar_get_string (varin, head->text.start_time);

      if (strstr (varin, "EndTime:") != NULL) lidar_get_string (varin, head->text.end_time);

      if (strstr (varin, "StartTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->text.start_timestamp);

      if (strstr (varin, "EndTimestamp:") != NULL) sscanf (info, NV_INT64_SPECIFIER, &head->text.end_timestamp);

      if (strstr (varin, "NumberShots:") != NULL) sscanf (info, "%d", &head->text.number_shots);

      if (strstr (varin, "FileCreateDate:") != NULL) lidar_get_string (varin, head->text.dataset_create_date);

      if (strstr (varin, "FileCreateTime:") != NULL) lidar_get_string (varin, head->text.dataset_create_time);

      if (strstr (varin, "LineMinLat:") != NULL) sscanf (info, "%lf", &head->text.line_min_lat);

      if (strstr (varin, "LineMaxLat:") != NULL) sscanf (info, "%lf", &head->text.line_max_lat);

      if (strstr (varin, "LineMinLong:") != NULL) sscanf (info, "%lf", &head->text.line_min_lon);

      if (strstr (varin, "LineMaxLong:") != NULL) sscanf (info, "%lf", &head->text.line_max_lon);


      long_pos = ftello64 (fp);
      if (head->text.header_size && long_pos >= head->text.header_size) break;
    }


  /*  Make sure we're past the header.  */

  fseeko64 (fp, (NV_INT64) head->text.header_size, SEEK_SET);


  return (0);
}


/*  Note that we're counting from 1 not 0.  Not my idea!  */

NV_INT32 tof_read_record (FILE *fp, NV_INT32 num, TOPO_OUTPUT_T *record)
{
  NV_INT32 ret;
  NV_INT64 long_pos;


  if (!num)
    {
      fprintf (stderr, "Zero is not a valid TOF record number\n");
      fflush (stderr);
      return (0);
    }


  if (num != TOF_NEXT_RECORD)
    {
      fseeko64 (fp, (NV_INT64) TOF_HEAD_SIZE + (NV_INT64) (num - 1) * (NV_INT64) sizeof (TOPO_OUTPUT_T), SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (fp);
      if (long_pos < TOF_HEAD_SIZE) fseeko64 (fp, (NV_INT64) TOF_HEAD_SIZE, SEEK_SET);
    }


  ret = fread (record, sizeof (TOPO_OUTPUT_T), 1, fp);


  if (swap) swap_tof_record (record);


  return (ret);
}


NV_INT32 tof_write_header (FILE *fp, TOF_HEADER_T head)
{
  fseeko64 (fp, 0LL, SEEK_SET);

  if (swap) swap_tof_header (&head);

  fwrite (&head, sizeof (TOF_HEADER_T), 1, fp);

  return (0);
}


/*  Note that we're counting from 1 not 0.  Not my idea!  */

NV_INT32 tof_write_record (FILE *fp, NV_INT32 num, TOPO_OUTPUT_T record)
{
  NV_INT32 ret;


  if (!num)
    {
      fprintf (stderr, "Zero is not a valid TOF record number\n");
      fflush (stderr);
      return (0);
    }


  if (num != TOF_NEXT_RECORD) fseeko64 (fp, (NV_INT64) TOF_HEAD_SIZE + (NV_INT64) (num - 1) * 
                                        (NV_INT64) sizeof (TOPO_OUTPUT_T), SEEK_SET);


  if (swap) swap_tof_record (&record);


  ret = fwrite (&record, sizeof (TOPO_OUTPUT_T), 1, fp);


  return (ret);
}


void tof_dump_record (TOPO_OUTPUT_T record)
{
  NV_INT32        year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  charts_cvtime (record.timestamp, &year, &day, &hour, &minute, &second);
  charts_jday2mday (year, day, &month, &mday);
  month++;

  printf ("*****************************\n");
  printf ("timestamp : ");
  printf (NV_INT64_SPECIFIER, record.timestamp);
  printf ("    %d-%02d-%02d (%d) %02d:%02d:%08.5f\n", year + 1900, month, mday, day, hour, minute, second);
  printf ("latitude first : %f\n", record.latitude_first);
  printf ("longitude first : %f\n", record.longitude_first);
  printf ("latitude last : %f\n", record.latitude_last);
  printf ("longitude last : %f\n", record.longitude_last);
  printf ("elevation first : %f\n", record.elevation_first);
  printf ("elevation last : %f\n", record.elevation_last);
  printf ("scanner azimuth : %f\n", record.scanner_azimuth);
  printf ("nadir angle : %f\n", record.nadir_angle);
  printf ("confidence first : %d\n", record.conf_first);
  printf ("confidence last : %d\n", record.conf_last);
  printf ("intensity first : %hd\n", (NV_INT16) record.intensity_first);
  printf ("intensity last : %hd\n", (NV_INT16) record.intensity_last);
  printf ("classification status : %x\n", record.classification_status);
  printf ("TBD 1 : %x\n", record.tbd_1);
  printf ("position conf : %d\n", record.pos_conf);
  printf ("TBD 2 : %x\n", record.tbd_2);
  printf ("result elevation first : %f\n", record.result_elevation_first);
  printf ("result elevation last : %f\n", record.result_elevation_last);
  printf ("altitude : %f\n", record.altitude);
  printf ("TBD float : %f\n", record.tbdfloat);
}
