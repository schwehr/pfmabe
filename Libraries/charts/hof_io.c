#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>

#include "FileHydroOutput.h"
#include "hof_errors.h"

static NV_BOOL swap = NVFalse;


static void swap_hof_header (HOF_HEADER_T *head)
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


static void swap_hof_record (HYDRO_OUTPUT_T *record)
{
  NV_INT16 i;


  swap_NV_INT64 (&record->timestamp);
  swap_NV_INT16 (&record->haps_version);
  swap_NV_INT16 (&record->position_conf);
  swap_NV_FLOAT64 (&record->latitude);
  swap_NV_FLOAT64 (&record->longitude);
  swap_NV_FLOAT64 (&record->sec_latitude);
  swap_NV_FLOAT64 (&record->sec_longitude);
  swap_NV_FLOAT32 (&record->correct_depth);
  swap_NV_FLOAT32 (&record->correct_sec_depth);
  swap_NV_INT16 (&record->abdc);
  swap_NV_INT16 (&record->sec_abdc);

  for (i = 0 ; i < 2 ; i++) swap_NV_FLOAT32 (&record->future_use[i]);

  swap_NV_FLOAT32 (&record->tide_cor_depth);
  swap_NV_FLOAT32 (&record->reported_depth);
  swap_NV_FLOAT32 (&record->result_depth);
  swap_NV_FLOAT32 (&record->sec_depth);
  swap_NV_FLOAT32 (&record->wave_height);
  swap_NV_FLOAT32 (&record->elevation);
  swap_NV_FLOAT32 (&record->topo);
  swap_NV_FLOAT32 (&record->altitude);
  swap_NV_FLOAT32 (&record->kgps_elevation);
  swap_NV_FLOAT32 (&record->kgps_res_elev);
  swap_NV_FLOAT32 (&record->kgps_sec_elev);
  swap_NV_FLOAT32 (&record->kgps_topo);
  swap_NV_FLOAT32 (&record->kgps_datum);
  swap_NV_FLOAT32 (&record->kgps_water_level);
  swap_NV_FLOAT32 (&record->k);
  swap_NV_FLOAT32 (&record->intensity);
  swap_NV_FLOAT32 (&record->bot_conf);
  swap_NV_FLOAT32 (&record->sec_bot_conf);
  swap_NV_FLOAT32 (&record->nadir_angle);
  swap_NV_FLOAT32 (&record->scanner_azimuth);
  swap_NV_FLOAT32 (&record->sfc_fom_apd);
  swap_NV_FLOAT32 (&record->sfc_fom_ir);
  swap_NV_FLOAT32 (&record->sfc_fom_ram);
  swap_NV_FLOAT32 (&record->no_bottom_at);
  swap_NV_FLOAT32 (&record->no_bottom_at2);
  swap_NV_INT32 (&record->depth_conf);
  swap_NV_INT32 (&record->sec_depth_conf);
  swap_NV_INT32 (&record->warnings);
  swap_NV_INT32 (&record->warnings2);
  swap_NV_INT32 (&record->warnings3);

  for (i = 0 ; i < 2 ; i++) swap_NV_INT16 ((NV_INT16 *) &record->calc_bfom_thresh_times10[i]);

  swap_NV_INT16 (&record->bot_bin_first);
  swap_NV_INT16 (&record->bot_bin_second);
  swap_NV_INT16 (&record->bot_bin_used_pmt);
  swap_NV_INT16 (&record->sec_bot_bin_used_pmt);
  swap_NV_INT16 (&record->bot_bin_used_apd);
  swap_NV_INT16 (&record->sec_bot_bin_used_apd);
}


FILE *open_hof_file (NV_CHAR *path)
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
          fflush (stderr);
        }
    }

  return (fp);
}


NV_INT32 hof_read_header (FILE *fp, HOF_HEADER_T *head)
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

NV_INT32 hof_read_record (FILE *fp, NV_INT32 num, HYDRO_OUTPUT_T *record)
{
  NV_INT32 ret;
  NV_INT64 long_pos;


  if (!num)
    {
      fprintf (stderr, "Zero is not a valid HOF record number\n");
      fflush (stderr);
      return (0);
    }


  if (num != HOF_NEXT_RECORD)
    {
      fseeko64 (fp, (NV_INT64) HOF_HEAD_SIZE + (NV_INT64) (num - 1) * (NV_INT64) sizeof (HYDRO_OUTPUT_T), SEEK_SET);
    }
  else
    {
      long_pos = ftello64 (fp);
      if (long_pos < HOF_HEAD_SIZE) fseeko64 (fp, (NV_INT64) HOF_HEAD_SIZE, SEEK_SET);
    }


  ret = fread (record, sizeof (HYDRO_OUTPUT_T), 1, fp);


  if (swap) swap_hof_record (record);


  return (ret);
}


NV_INT32 hof_write_header (FILE *fp, HOF_HEADER_T head)
{
  fseeko64 (fp, 0LL, SEEK_SET);

  if (swap) swap_hof_header (&head);

  fwrite (&head, sizeof (HOF_HEADER_T), 1, fp);

  return (0);
}


/*  Note that we're counting from 1 not 0.  Not my idea!  */

NV_INT32 hof_write_record (FILE *fp, NV_INT32 num, HYDRO_OUTPUT_T record)
{
  NV_INT32 ret;


  if (!num)
    {
      fprintf (stderr, "Zero is not a valid HOF record number\n");
      fflush (stderr);
      return (0);
    }


  if (num != HOF_NEXT_RECORD) fseeko64 (fp, (NV_INT64) HOF_HEAD_SIZE + (NV_INT64) (num - 1) * 
                                        (NV_INT64) sizeof (HYDRO_OUTPUT_T), SEEK_SET);


  if (swap) swap_hof_record (&record);


  ret = fwrite (&record, sizeof (HYDRO_OUTPUT_T), 1, fp);


  return (ret);
}



/*
    This function computes 95% confidence horizontal and vertical uncertainty values based on
    information from Paul LaRoque at Optech, Toronto (29 March 2005).  More information is
    available in the spreadsheet file, CHARTS_Hydro_Topo_Accuracies_29_Mar_2005.xls

    Author : Jan C. Depner
    Date : June 02, 2008
*/

void hof_get_uncertainty (HYDRO_OUTPUT_T record, NV_FLOAT32 *h_error, NV_FLOAT32 *v_error, NV_FLOAT32 in_depth, NV_INT32 abdc)
{
  static NV_FLOAT64 horizontal_errors[2] = {2.00, 0.15};
  static NV_FLOAT32 prev_altitude = 400.0;

  NV_FLOAT32        altitude;
  NV_FLOAT64        depth;
  NV_INT32          type_index;

  NV_FLOAT64        rad_nadir, cos_rad_nadir, cos_rad_nadir_2, cos_rad_yaw, sqrt_2, tan_rad_nadir, rad_nadir_m5,
                    tan_rad_nadir_m5;
  NV_FLOAT64        height_error, roll_error, pitch_error, heading_error, scan_angle_error, antenna_error, 
                    h_calibration_error, laser_pointing_error, iho_error, propagation_error, beam_steering_error;
  NV_FLOAT64        total_aircraft_to_surface;
  NV_FLOAT64        total_platform_to_bottom;
  NV_FLOAT64        total_horizontal_error;

  NV_FLOAT64        depth_45, wave_beam_steering_error, propagation_induced_error;
  NV_FLOAT64        total_random_error;
  NV_FLOAT64        total_bias_error;


  if (in_depth <= -998.0) 
    {
      *h_error = 0.0;
      *v_error = 0.0;
      return;
    }


  if (record.altitude == 0.0)
    {
      altitude = prev_altitude;
    }
  else
    {
      altitude = record.altitude;
    }


  depth = (NV_FLOAT64) ((NV_INT32) -in_depth);
  type_index = (NV_INT32) record.data_type;


#ifdef CHARTS_DEBUG
  fprintf (stderr,"%s %d %d %f %f %f\n", __FILE__, __LINE__, type_index, in_depth, altitude, record.nadir_angle);
#endif


  /*  Horizontal error pre-computed values  */

  rad_nadir = NV_DEG_TO_RAD * record.nadir_angle;
  tan_rad_nadir = tan (rad_nadir);
  rad_nadir_m5 = (record.nadir_angle - 5.0) * NV_DEG_TO_RAD;
  tan_rad_nadir_m5 = tan (rad_nadir_m5);
  cos_rad_nadir = cos (rad_nadir);
  cos_rad_nadir_2 = cos_rad_nadir * cos_rad_nadir;
  cos_rad_yaw = cos (NV_DEG_TO_RAD * E_YAW);
  sqrt_2 = 1.41421356237;


  /*  CHARTS System Errors  */

  height_error = E_HEIGHT * tan_rad_nadir;
  roll_error = altitude * E_ROLL / cos_rad_nadir_2 * NV_DEG_TO_RAD;
  pitch_error = altitude * E_PITCH / cos_rad_nadir_2 * NV_DEG_TO_RAD;
  heading_error = sqrt (2.0 * altitude * altitude * tan_rad_nadir * tan_rad_nadir * (1.0 - cos_rad_yaw));
  scan_angle_error = sqrt_2 * altitude * E_SCAN_ANGLE / cos_rad_nadir_2 * NV_DEG_TO_RAD;
  antenna_error = E_ANTENNA * sqrt_2;
  h_calibration_error = sqrt_2 * altitude * E_H_CALIBRATION / cos_rad_nadir_2 * NV_DEG_TO_RAD;
  laser_pointing_error = altitude * E_LASER_POINTING / cos_rad_nadir_2 * NV_DEG_TO_RAD;

  total_aircraft_to_surface = sqrt (height_error * height_error + roll_error * roll_error + pitch_error * pitch_error +
				    heading_error * heading_error + scan_angle_error * scan_angle_error +
				    antenna_error * antenna_error + h_calibration_error * h_calibration_error +
				    laser_pointing_error * laser_pointing_error);

#ifdef CHARTS_DEBUG
  fprintf (stderr,"%s %d %f %f %f %f %f %f %f %f %f\n", __FILE__, __LINE__, height_error, roll_error, pitch_error,
	   heading_error, scan_angle_error, antenna_error, h_calibration_error, laser_pointing_error, total_aircraft_to_surface);
#endif


  /*  Water Parameter Errors  */

  iho_error = sqrt (0.25 + (0.013 * depth) * (0.013 * depth)) / 1.96 * tan_rad_nadir_m5;
  propagation_error = E_PROPAGATION * depth;
  beam_steering_error = (depth * 0.45 / 100.0) * tan_rad_nadir_m5;

  total_platform_to_bottom = sqrt (total_aircraft_to_surface * total_aircraft_to_surface + iho_error * iho_error +
				   propagation_error * propagation_error + beam_steering_error * beam_steering_error);

#ifdef CHARTS_DEBUG
  fprintf (stderr,"%s %d %f %f %f %f\n", __FILE__, __LINE__, iho_error, propagation_error, beam_steering_error,
	   total_platform_to_bottom);
#endif


  /*  Total Horizontal Error  */

  total_horizontal_error = sqrt (total_platform_to_bottom * total_platform_to_bottom + 
				 horizontal_errors[type_index] * horizontal_errors[type_index]);


  /*  95% Confidence Horizontal Error  */

  *h_error = (NV_FLOAT32) (total_horizontal_error * 1.96);


  /*  Land values have no vertical error estimate  */

  if (abdc == 70 || abdc == 13)
    {
      *v_error = 0.0;
    }
  else
    {
      /*  Vertical error pre-computed values  */

      depth_45 = 0.45 * depth;


      /*  Vertical random errors  */

      wave_beam_steering_error = depth * 0.45 / 100.0;

      total_random_error = sqrt (E_ALTIMETER_TIM_2 + E_CFD_2 + E_LOG_AMP_DELAY_2 + E_WAVE_HEIGHT_2 +
				 E_PULSE_LOCATION_2 + (E_ELLIPSOID_TO_LASER_KGPS_2 * (NV_FLOAT64) type_index) +
				 wave_beam_steering_error * wave_beam_steering_error);

#ifdef CHARTS_DEBUG
      fprintf (stderr,"%s %d %f %f\n", __FILE__, __LINE__, wave_beam_steering_error, total_random_error);
#endif


      /*  Vertical Bias Errors  */

      propagation_induced_error = sqrt (36.0 + depth_45 * depth_45) / 100.0;

      total_bias_error = sqrt (E_THERMAL_2 + E_V_CALIBRATION_2 + E_PMT_DELAY_2 + E_SURFACE_ORIGIN_2 +
			       propagation_induced_error * propagation_induced_error);

#ifdef CHARTS_DEBUG
      fprintf (stderr,"%s %d %f %f\n", __FILE__, __LINE__, propagation_induced_error, total_bias_error);
#endif


      /*  95% Confidence Vertical Error  */

      *v_error = (NV_FLOAT32) (fabs (total_bias_error - E_CONSTANT_BIAS) + total_random_error * 1.96);
    }
}



void hof_dump_record (HYDRO_OUTPUT_T record)
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
  printf ("haps version : %d\n", record.haps_version);
  printf ("position confidence : %d\n", record.position_conf);
  printf ("status : %x\n", record.status);
  printf ("suggested delete/keep/swap : %d\n", record.suggested_dks);
  printf ("suspect status : %x\n", record.suspect_status);
  printf ("tide status : %x\n", record.tide_status & 0x3);
  printf ("latitude : %f\n", record.latitude);
  printf ("longitude : %f\n", record.longitude);
  printf ("secondary latitude : %f\n", record.sec_latitude);
  printf ("secondary longitude : %f\n", record.sec_longitude);
  printf ("correct depth : %f\n", record.correct_depth);
  printf ("correct secondary depth : %f\n", record.correct_sec_depth);
  printf ("abbreviated depth confidence : %d\n", record.abdc);
  printf ("secondary abbreviated depth confidence : %d\n", record.sec_abdc);
  printf ("data type : %d\n", record.data_type);
  printf ("tide corrected depth : %f\n", record.tide_cor_depth);
  printf ("reported depth : %f\n", record.reported_depth);
  printf ("result depth : %f\n", record.result_depth);
  printf ("secondary depth : %f\n", record.sec_depth);
  printf ("wave height : %f\n", record.wave_height);
  printf ("elevation : %f\n", record.elevation);
  printf ("topo : %f\n", record.topo);
  printf ("altitude : %f\n", record.altitude);
  printf ("kgps elevation : %f\n", record.kgps_elevation);
  printf ("kgps result elevation : %f\n", record.kgps_res_elev);
  printf ("kgps secondary elevation : %f\n", record.kgps_sec_elev);
  printf ("kgps topo : %f\n", record.kgps_topo);
  printf ("kgps datum : %f\n", record.kgps_datum);
  printf ("kgps water level : %f\n", record.kgps_water_level);
  printf ("bottom confidence : %f\n", record.bot_conf);
  printf ("secondary bottom confidence : %f\n", record.sec_bot_conf);
  printf ("nadir angle : %f\n", record.nadir_angle);
  printf ("scanner azimuth : %f\n", record.scanner_azimuth);
  printf ("surface figure of merit apd : %f\n", record.sfc_fom_apd);
  printf ("surface figure of merit ir : %f\n", record.sfc_fom_ir);
  printf ("surface figure of merit raman : %f\n", record.sfc_fom_ram);
  printf ("depth confidence : %d\n", record.depth_conf);
  printf ("secondary depth confidence : %d\n", record.sec_depth_conf);
  printf ("warnings : %d\n", record.warnings);
  printf ("warnings2 : %d\n", record.warnings2);
  printf ("warnings3 : %d\n", record.warnings3);
  printf ("calc_bfom_thresh_times10[0] : %d\n", record.calc_bfom_thresh_times10[0]);
  printf ("calc_bfom_thresh_times10[1] : %d\n", record.calc_bfom_thresh_times10[1]);
  printf ("calc_bot_run_required[0] : %d\n", record.calc_bot_run_required[0]);
  printf ("calc_bot_run_required[1] : %d\n", record.calc_bot_run_required[1]);
  printf ("bot_bin_first : %d\n", record.bot_bin_first    );
  printf ("bot_bin_second : %d\n", record.bot_bin_second    );
  printf ("bot_bin_used_pmt : %d\n", record.bot_bin_used_pmt    );
  printf ("sec_bot_bin_used_pmt : %d\n", record.sec_bot_bin_used_pmt    );
  printf ("bot_bin_used_apd : %d\n", record.bot_bin_used_apd    );
  printf ("sec_bot_bin_used_apd : %d\n", record.sec_bot_bin_used_apd);
  printf ("bot_channel : %d\n", record.bot_channel        );
  printf ("sec_bot_chan : %d\n", record.sec_bot_chan    );
  printf ("sfc_bin_apd : %d\n", record.sfc_bin_apd        );
  printf ("sfc_bin_ir : %d\n", record.sfc_bin_ir            );
  printf ("sfc_bin_ram : %d\n", record.sfc_bin_ram        );
  printf ("sfc_channel_used : %d\n", record.sfc_channel_used    );
  printf ("ab_dep_conf : %d\n", record.ab_dep_conf);
  printf ("sec_ab_dep_conf : %d\n", record.sec_ab_dep_conf);
  printf ("kgps_abd_conf : %d\n", record.kgps_abd_conf);
  printf ("kgps_sec_abd_conf : %d\n", record.kgps_sec_abd_conf);
}
