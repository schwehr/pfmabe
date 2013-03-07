
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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmLoadDef.hpp"
#include "attributes.hpp"


void setAllAttributes (PFM_GLOBAL *global)
{
  //  Populate the possible index attribute names and ranges.

  global->time_attribute_name = time_attribute_name;
  global->time_attribute_def[0] = time_attribute_def[0];
  global->time_attribute_def[1] = time_attribute_def[1];
  global->time_attribute_def[2] = time_attribute_def[2];

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      global->gsf_attribute_name[i] = gsf_attribute_name[i];
      global->gsf_attribute_def[i][0] = gsf_attribute_def[i][0];
      global->gsf_attribute_def[i][1] = gsf_attribute_def[i][1];
      global->gsf_attribute_def[i][2] = gsf_attribute_def[i][2];
    }

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      global->hof_attribute_name[i] = hof_attribute_name[i];
      global->hof_attribute_def[i][0] = hof_attribute_def[i][0];
      global->hof_attribute_def[i][1] = hof_attribute_def[i][1];
      global->hof_attribute_def[i][2] = hof_attribute_def[i][2];
    }

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      global->tof_attribute_name[i] = tof_attribute_name[i];
      global->tof_attribute_def[i][0] = tof_attribute_def[i][0];
      global->tof_attribute_def[i][1] = tof_attribute_def[i][1];
      global->tof_attribute_def[i][2] = tof_attribute_def[i][2];
    }

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      global->wlf_attribute_name[i] = wlf_attribute_name[i];
      global->wlf_attribute_def[i][0] = wlf_attribute_def[i][0];
      global->wlf_attribute_def[i][1] = wlf_attribute_def[i][1];
      global->wlf_attribute_def[i][2] = wlf_attribute_def[i][2];
    }

  for (NV_INT32 i = 0; i < CZMIL_ATTRIBUTES; i++)
    {
      global->czmil_attribute_name[i] = czmil_attribute_name[i];
      global->czmil_attribute_def[i][0] = czmil_attribute_def[i][0];
      global->czmil_attribute_def[i][1] = czmil_attribute_def[i][1];
      global->czmil_attribute_def[i][2] = czmil_attribute_def[i][2];
    }

  for (NV_INT32 i = 0; i < BAG_ATTRIBUTES; i++)
    {
      global->bag_attribute_name[i] = bag_attribute_name[i];
      global->bag_attribute_def[i][0] = bag_attribute_def[i][0];
      global->bag_attribute_def[i][1] = bag_attribute_def[i][1];
      global->bag_attribute_def[i][2] = bag_attribute_def[i][2];
    }

  for (NV_INT32 i = 0; i < HAWKEYE_ATTRIBUTES; i++)
    {
      global->hawkeye_attribute_name[i] = hawkeye_attribute_name[i];
      global->hawkeye_attribute_def[i][0] = hawkeye_attribute_def[i][0];
      global->hawkeye_attribute_def[i][1] = hawkeye_attribute_def[i][1];
      global->hawkeye_attribute_def[i][2] = hawkeye_attribute_def[i][2];
    }
}



void countAllAttributes (PFM_GLOBAL *global)
{
  global->attribute_count = 0;

  if (global->time_attribute_num) global->attribute_count++;

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      if (global->gsf_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      if (global->hof_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      if (global->tof_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      if (global->wlf_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      if (global->czmil_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      if (global->bag_attribute_num[i]) global->attribute_count++;
    }

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      if (global->hawkeye_attribute_num[i]) global->attribute_count++;
    }
}



NV_INT32 gsfAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      if (global.gsf_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 hofAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      if (global.hof_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 tofAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      if (global.tof_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 wlfAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      if (global.wlf_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 czmilAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      if (global.czmil_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 bagAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      if (global.bag_attribute_num[i]) count++;
    }

  return (count);
}



NV_INT32 hawkeyeAttributesSet (PFM_GLOBAL global)
{
  NV_INT32 count = 0;

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      if (global.hawkeye_attribute_num[i]) count++;
    }

  return (count);
}



void getGSFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, gsfSwathBathyPing mb_ping, NV_INT32 beam)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (NINT ((NV_FLOAT64) mb_ping.ping_time.tv_sec / 60.0));


  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.gsf_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.gsf_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = mb_ping.heading;
              break;

            case 1:
              l_attr[index] = mb_ping.pitch;
              break;

            case 2:
              l_attr[index] = mb_ping.roll;
              break;

            case 3:
              l_attr[index] = mb_ping.heave;
              break;

            case 4:
              l_attr[index] = mb_ping.course;
              break;

            case 5:
              l_attr[index] = mb_ping.speed;
              break;

            case 6:
              l_attr[index] = mb_ping.height;
              break;

            case 7:
              l_attr[index] = mb_ping.sep;
              break;

            case 8:
              l_attr[index] = mb_ping.tide_corrector;
              break;

            case 9:
              l_attr[index] = (mb_ping.ping_flags & (GSF_PING_USER_FLAG_15 | GSF_PING_USER_FLAG_14)) >> 14;
              break;

            case 10:
              if (mb_ping.across_track) l_attr[index] = mb_ping.across_track[beam];
              break;

            case 11:
              if (mb_ping.across_track) l_attr[index] = mb_ping.along_track[beam];
              break;

            case 12:
              if (mb_ping.travel_time) l_attr[index] = mb_ping.travel_time[beam];
              break;

            case 13:
              if (mb_ping.beam_angle) l_attr[index] = mb_ping.beam_angle[beam];
              break;

            case 14:
              if (mb_ping.beam_angle_forward) l_attr[index] = mb_ping.beam_angle_forward[beam];
              break;

            case 15:
              if (mb_ping.mc_amplitude) l_attr[index] = mb_ping.mc_amplitude[beam];
              break;

            case 16:
              if (mb_ping.mr_amplitude) l_attr[index] = mb_ping.mr_amplitude[beam];
              break;

            case 17:
              if (mb_ping.echo_width) l_attr[index] = mb_ping.echo_width[beam];
              break;

            case 18:
              if (mb_ping.quality_factor) l_attr[index] = mb_ping.quality_factor[beam];
              break;

            case 19:
              if (mb_ping.signal_to_noise) l_attr[index] = mb_ping.signal_to_noise[beam];
              break;

            case 20:
              if (mb_ping.receive_heave) l_attr[index] = mb_ping.receive_heave[beam];
              break;

            case 21:
              if (mb_ping.depth_error) l_attr[index] = mb_ping.depth_error[beam];
              break;

            case 22:
              if (mb_ping.across_track_error) l_attr[index] = mb_ping.across_track_error[beam];
              break;

            case 23:
              if (mb_ping.along_track_error) l_attr[index] = mb_ping.along_track_error[beam];
              break;

            case 24:
              if (mb_ping.quality_flags) l_attr[index] = (NV_FLOAT32) mb_ping.quality_flags[beam];
              break;

            case 25:
              if (mb_ping.beam_flags) l_attr[index] = (NV_FLOAT32) mb_ping.beam_flags[beam];
              break;

            case 26:
              if (mb_ping.beam_angle_forward) l_attr[index] = mb_ping.beam_angle_forward[beam];
              break;

            case 27:
              if (mb_ping.sector_number) l_attr[index] = (NV_FLOAT32) mb_ping.sector_number[beam];
              break;

            case 28:
              if (mb_ping.detection_info) l_attr[index] = (NV_FLOAT32) mb_ping.detection_info[beam];
              break;

            case 29:
              if (mb_ping.incident_beam_adj) l_attr[index] = mb_ping.incident_beam_adj[beam];
              break;

            case 30:
              if (mb_ping.system_cleaning) l_attr[index] = (NV_FLOAT32) mb_ping.system_cleaning[beam];
              break;

            case 31:
              if (mb_ping.doppler_corr) l_attr[index] = mb_ping.doppler_corr[beam];
              break;
            }
        }
    }
}



void getHOFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, HYDRO_OUTPUT_T hof)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (hof.timestamp / 60000000);


  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.hof_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.hof_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = hof.haps_version;
              break;

            case 1:
              l_attr[index] = hof.position_conf;
              break;

            case 2:
              l_attr[index] = hof.status;
              break;

            case 3:
              l_attr[index] = hof.suggested_dks;
              break;

            case 4:
              l_attr[index] = hof.suspect_status;
              break;

            case 5:
              l_attr[index] = hof.tide_status;
              break;

            case 6:
              l_attr[index] = hof.abdc;
              break;

            case 7:
              l_attr[index] = hof.sec_abdc;
              break;

            case 8:
              l_attr[index] = hof.data_type;
              break;

            case 9:
              l_attr[index] = hof.land_mode;
              break;

            case 10:
              l_attr[index] = hof.classification_status;
              break;

            case 11:
              l_attr[index] = hof.wave_height;
              break;

            case 12:
              l_attr[index] = hof.elevation;
              break;

            case 13:
              l_attr[index] = hof.topo;
              break;

            case 14:
              l_attr[index] = hof.altitude;
              break;

            case 15:
              l_attr[index] = hof.kgps_topo;
              break;

            case 16:
              l_attr[index] = hof.kgps_datum;
              break;

            case 17:
              l_attr[index] = hof.kgps_water_level;
              break;

            case 18:
              l_attr[index] = hof.k;
              break;

            case 19:
              l_attr[index] = hof.intensity;
              break;

            case 20:
              l_attr[index] = hof.bot_conf;
              break;

            case 21:
              l_attr[index] = hof.sec_bot_conf;
              break;

            case 22:
              l_attr[index] = hof.nadir_angle;
              break;

            case 23:
              l_attr[index] = hof.scanner_azimuth;
              break;

            case 24:
              l_attr[index] = hof.sfc_fom_apd;
              break;

            case 25:
              l_attr[index] = hof.sfc_fom_ir;
              break;

            case 26:
              l_attr[index] = hof.sfc_fom_ram;
              break;

            case 27:
              l_attr[index] = hof.depth_conf;
              break;

            case 28:
              l_attr[index] = hof.sec_depth_conf;
              break;

            case 29:
              l_attr[index] = hof.warnings;
              break;

            case 30:
              l_attr[index] = hof.warnings2;
              break;

            case 31:
              l_attr[index] = hof.warnings3;
              break;

            case 32:
              l_attr[index] = hof.calc_bfom_thresh_times10[0];
              break;

            case 33:
              l_attr[index] = hof.calc_bfom_thresh_times10[1];
              break;

            case 34:
              l_attr[index] = hof.calc_bot_run_required[0];
              break;

            case 35:
              l_attr[index] = hof.calc_bot_run_required[1];
              break;

            case 36:
              l_attr[index] = hof.bot_bin_first;
              break;

            case 37:
              l_attr[index] = hof.bot_bin_second;
              break;

            case 38:
              l_attr[index] = hof.bot_bin_used_pmt;
              break;

            case 39:
              l_attr[index] = hof.sec_bot_bin_used_pmt;
              break;

            case 40:
              l_attr[index] = hof.bot_bin_used_apd;
              break;

            case 41:
              l_attr[index] = hof.sec_bot_bin_used_apd;
              break;

            case 42:
              l_attr[index] = hof.bot_channel;
              break;

            case 43:
              l_attr[index] = hof.sec_bot_chan;
              break;

            case 44:
              l_attr[index] = hof.sfc_bin_apd;
              break;

            case 45:
              l_attr[index] = hof.sfc_bin_ir;
              break;

            case 46:
              l_attr[index] = hof.sfc_bin_ram;
              break;

            case 47:
              l_attr[index] = hof.sfc_channel_used;
              break;
            }
        }
    }
}



void getTOFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, TOPO_OUTPUT_T tof)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (tof.timestamp / 60000000);


  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.tof_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.tof_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = tof.classification_status;
              break;

            case 1:
              l_attr[index] = tof.altitude;
              break;

            case 2:
              l_attr[index] = tof.intensity_first;
              break;

            case 3:
              l_attr[index] = tof.intensity_last;
              break;

            case 4:
              l_attr[index] = tof.conf_first;
              break;

            case 5:
              l_attr[index] = tof.conf_last;
              break;

            case 6:
              l_attr[index] = tof.nadir_angle;
              break;

            case 7:
              l_attr[index] = tof.scanner_azimuth;
              break;

            case 8:
              l_attr[index] = tof.pos_conf;
              break;
            }
        }
    }
}



void getWLFAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, WLF_RECORD wlf)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (wlf.tv_sec / 60);


  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.wlf_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.wlf_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = wlf.water_surface;
              break;

            case 1:
              l_attr[index] = wlf.z_offset;
              break;

            case 2:
              l_attr[index] = wlf.number_of_returns;
              break;

            case 3:
              l_attr[index] = wlf.return_number;
              break;

            case 4:
              l_attr[index] = wlf.point_source;
              break;

            case 5:
              l_attr[index] = wlf.scan_angle;
              break;

            case 6:
              l_attr[index] = wlf.edge_of_flight_line;
              break;

            case 7:
              l_attr[index] = wlf.intensity;
              break;

            case 8:
              l_attr[index] = wlf.red;
              break;

            case 9:
              l_attr[index] = wlf.green;
              break;

            case 10:
              l_attr[index] = wlf.blue;
              break;

            case 11:
              l_attr[index] = wlf.reflectance;
              break;

            case 12:
              l_attr[index] = wlf.classification;
              break;
            }
        }
    }
}



void getCZMILAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, CZMIL_CXY_Data czmil)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (czmil.channel[CZMIL_DEEP_CHANNEL].timestamp / 60000000);


  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.czmil_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.czmil_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[1];
              break;

            case 1:
              l_attr[index] = czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[2];
              break;

            case 2:
              l_attr[index] = czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[3];
              break;

            case 3:
              l_attr[index] = czmil.channel[CZMIL_DEEP_CHANNEL].waveform_class;
              break;

            case 4:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_1].return_elevation[1];
              break;

            case 5:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_1].return_elevation[2];
              break;

            case 6:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_1].return_elevation[3];
              break;

            case 7:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_1].waveform_class;
              break;

            case 8:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_2].return_elevation[1];
              break;

            case 9:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_2].return_elevation[2];
              break;

            case 10:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_2].return_elevation[3];
              break;

            case 11:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_2].waveform_class;
              break;

            case 12:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_3].return_elevation[1];
              break;

            case 13:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_3].return_elevation[2];
              break;

            case 14:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_3].return_elevation[3];
              break;

            case 15:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_3].waveform_class;
              break;

            case 16:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_4].return_elevation[1];
              break;

            case 17:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_4].return_elevation[2];
              break;

            case 18:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_4].return_elevation[3];
              break;

            case 19:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_4].waveform_class;
              break;

            case 20:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_5].return_elevation[1];
              break;

            case 21:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_5].return_elevation[2];
              break;

            case 22:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_5].return_elevation[3];
              break;

            case 23:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_5].waveform_class;
              break;

            case 24:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_6].return_elevation[1];
              break;

            case 25:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_6].return_elevation[2];
              break;

            case 26:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_6].return_elevation[3];
              break;

            case 27:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_6].waveform_class;
              break;

            case 28:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_7].return_elevation[1];
              break;

            case 29:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_7].return_elevation[2];
              break;

            case 30:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_7].return_elevation[3];
              break;

            case 31:
              l_attr[index] = czmil.channel[CZMIL_SHALLOW_CHANNEL_7].waveform_class;
              break;

            case 32:
              l_attr[index] = czmil.channel[CZMIL_IR_CHANNEL].return_elevation[1];
              break;

            case 33:
              l_attr[index] = czmil.channel[CZMIL_IR_CHANNEL].return_elevation[2];
              break;

            case 34:
              l_attr[index] = czmil.channel[CZMIL_IR_CHANNEL].return_elevation[3];
              break;

            case 35:
              l_attr[index] = czmil.channel[CZMIL_IR_CHANNEL].waveform_class;
              break;
            }
        }
    }
}



void getBAGAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, BAG_RECORD bag)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = PFMWDB_NULL_TIME;


  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.bag_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.bag_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = bag.uncert;
              break;
            }
        }
    }
}



void getHAWKEYEAttributes (NV_FLOAT32 *l_attr, PFM_GLOBAL global, HAWKEYE_RECORD hawkeye)
{
  NV_INT32 index = global.time_attribute_num - 1;
  if (global.time_attribute_num) l_attr[index] = (NV_FLOAT32) (hawkeye.tv_sec / 60);


  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      NV_INT32 index = global.hawkeye_attribute_num[i] - 1;

      l_attr[index] = 0.0;

      if (global.hawkeye_attribute_num[i])
        {
          switch (i)
            {
            case 0:
              l_attr[index] = hawkeye.Surface_Altitude;            /*  Survey Point Instantaneous Surface Altitude  */
              break;

            case 1:
              l_attr[index] = hawkeye.Interpolated_Surface_Altitude; /*  Survey Point Interpolated Surface Altitude  */
              break;

            case 2:
              l_attr[index] = hawkeye.Surface_Northing_StdDev;     /*  Estimated Standard Deviation of Survey Point Surface Northing  */
              break;

            case 3:
              l_attr[index] = hawkeye.Surface_Easting_StdDev;      /*  Estimated Standard Deviation of Survey Point Surface Easting  */
              break;

            case 4:
              l_attr[index] = hawkeye.Surface_Altitude_StdDev;     /*  Estimated Standard Deviation of Survey Point Surface Altitude  */
              break;

            case 5:
              l_attr[index] = hawkeye.Point_Altitude;              /*  Survey Point Altitude  */
              break;

            case 6:
              l_attr[index] = hawkeye.Point_Northing_StdDev;       /*  Estimated Standard Deviation of Survey Point Northing  */
              break;

            case 7:
              l_attr[index] = hawkeye.Point_Easting_StdDev;        /*  Estimated Standard Deviation of Survey Point Easting  */
              break;

            case 8:
              l_attr[index] = hawkeye.Point_Altitude_StdDev;       /*  Estimated Standard Deviation of Survey Point Altitude  */
              break;

            case 9:
              l_attr[index] = hawkeye.Azimuth;                     /*  Scanner Mirror Azimuth Angle at Aircraft Direction of Movement  */
              break;

            case 10:
              l_attr[index] = hawkeye.Air_Nadir_Angle;             /*  Scanner Mirror Nadir Angle  */
              break;

            case 11:
              l_attr[index] = hawkeye.Water_Nadir_Angle;           /*  Estimated Laser Beam Nadir Angle in Water Column  */
              break;

            case 12:
              l_attr[index] = hawkeye.Slant_range;                 /*  Surface slant range  */
              break;

            case 13:
              l_attr[index] = hawkeye.Slant_Range_Comp_Model;      /*  Enumerator for the computational model used to produce the slant range  */
              break;

            case 14:
              l_attr[index] = hawkeye.Wave_Height;                 /*  Estimated Wave Height  */
              break;

            case 15:
              l_attr[index] = hawkeye.Water_Quality_Correction;    /*  The value subtracted to produce a wave height corrected surface altitude  */
              break;

            case 16:
              l_attr[index] = hawkeye.Tidal_Correction;            /*  The value subtracted to produce a tide corrected surface altitude  */
              break;

            case 17:
              l_attr[index] = hawkeye.Depth_Amplitude;             /*  The amplitude of the waveform peak attributed to this survey point  */
              break;

            case 18:
              l_attr[index] = hawkeye.Depth_Class;                 /*  The classification of this survey point  */
              break;

            case 19:
              l_attr[index] = hawkeye.Depth_Confidence;            /*  The estimated confidence of the depth or altitude value for this survey point  */
              break;

            case 20:
              l_attr[index] = hawkeye.Pixel_Index;                 /*  Index of the receiver pixel whose data was used to produce this survey point  */
              break;

            case 21:
              l_attr[index] = hawkeye.Scanner_Angle_X;             /*  Scanner angle at X-axis  */
              break;

            case 22:
              l_attr[index] = hawkeye.Scanner_Angle_Y;             /*  Scanner angle at Y-axis  */
              break;

            case 23:
              l_attr[index] = hawkeye.Aircraft_Altitude;           /*  Onboard Inertial/GPS Navigation System Altitude  */
              break;

            case 24:
              l_attr[index] = hawkeye.Aircraft_Roll;               /*  Onboard Inertial/GPS Navigation System Roll Angle  */
              break;

            case 25:
              l_attr[index] = hawkeye.Aircraft_Pitch;              /*  Onboard Inertial/GPS Navigation System Pitch Angle  */
              break;

            case 26:
              l_attr[index] = hawkeye.Aircraft_Heading;            /*  Onboard Inertial/GPS Navigation System Heading  */
              break;

            case 27:
              l_attr[index] = hawkeye.Aircraft_Northing_StdDev;    /*  Onboard Inertial/GPS Navigation System Northing Standard Deviation  */
              break;

            case 28:
              l_attr[index] = hawkeye.Aircraft_Easting_StdDev;     /*  Onboard Inertial/GPS Navigation System Easting Standard Deviation  */
              break;

            case 29:
              l_attr[index] = hawkeye.Aircraft_Altitude_StdDev;    /*  Onboard Inertial/GPS Navigation System Altitude Standard Deviation  */
              break;

            case 30:
              l_attr[index] = hawkeye.Aircraft_Roll_StdDev;        /*  Onboard Inertial/GPS Navigation System Roll Angle Standard Deviation  */
              break;

            case 31:
              l_attr[index] = hawkeye.Aircraft_Pitch_StdDev;       /*  Onboard Inertial/GPS Navigation System Pitch Angle Standard Deviation  */
              break;

            case 32:
              l_attr[index] = hawkeye.Aircraft_Heading_StdDev;     /*  Onboard Inertial/GPS Navigation System Heading Standard Deviation  */
              break;

            case 33:
              l_attr[index] = hawkeye.Extracted_Waveform_Attributes; /*  Attributes extracted/estimated during waveform processing kept as bitwise flags  */
              break;

            case 34:
              l_attr[index] = hawkeye.Receiver_Data_Used;          /*  Enumerator for the receiver whose data was used to produce the survey point  */
              break;

            case 35:
              l_attr[index] = hawkeye.Manual_Output_Screening_Flags; /*  A byte bitwise interpreted as flags assigned during manual screening of post
                                                                         processing output  */
              break;

            case 36:
              l_attr[index] = hawkeye.Waveform_Peak_Amplitude;     /*  The amplitude of the waveform processing detected peak used for this survey point  */
              break;

            case 37:
              l_attr[index] = hawkeye.Waveform_Peak_Classification; /*  The classification enumerator of the waveform processing detected peak used for
                                                                        this survey point  */
              break;

            case 38:
              l_attr[index] = hawkeye.Waveform_Peak_Contrast;      /*  The contrast of the waveform processing detected peak used for this survey point  */
              break;

            case 39:
              l_attr[index] = hawkeye.Waveform_Peak_Debug_Flags;   /*  The debug flags for the waveform processing detected peak used for this survey
                                                                       point (two bytes of bitwise flags)  */
              break;

            case 40:
              l_attr[index] = hawkeye.Waveform_Peak_Attributes;    /*  The extracted attributes of the waveform processing detected peak used for this
                                                                       survey point (two bytes of bitwise flags)  */
              break;

            case 41:
              l_attr[index] = hawkeye.Waveform_Peak_Jitter;        /*  The jitter of the waveform processing detected peak used for this survey point  */
              break;

            case 42:
              l_attr[index] = hawkeye.Waveform_Peak_Position;      /*  The waveform sample index of the waveform processing detected peak used for this
                                                                       survey point  */
              break;

            case 43:
              l_attr[index] = hawkeye.Waveform_Peak_Pulsewidth;    /*  The pulsewidth of the waveform processing detected peak used for this survey point  */
              break;

            case 44:
              l_attr[index] = hawkeye.Waveform_Peak_SNR;           /*  The signal-to-noise ratio of the waveform processing detected peak used for this
                                                                       survey point  */
              break;

            case 45:
              l_attr[index] = hawkeye.Scan_Direction_Flag;         /*  Scan Direction Flag indicating left-to-right with >0 right-to-left with <0 and
                                                                       non-computable with 0  */
              break;

            case 46:
              l_attr[index] = hawkeye.Edge_of_Flightline;          /*  A boolean flag indicating the scanning direction having shifted  */
              break;

            case 47:
              l_attr[index] = hawkeye.Scan_Angle_Rank;             /*  The angle of the laser output at the nadir rounded to nearest integer  */
              break;

	    case 48:
	      l_attr[index] = hawkeye.SelectBottomCase;            /*  SelectBottom Case is for development purposes states which case in SeletBottoms
                                                                       that is used  */
	      break;

	    case 49:
	      l_attr[index] = hawkeye.Return_Number;               /*  The index into the array of returns extracted from the current waveform  */
	      break;

	    case 50:
	      l_attr[index] = hawkeye.Number_of_Returns;           /*  The total number of returns extracted from the current waveform  */
	      break;

	    case 51:
	      l_attr[index] = hawkeye.Surface_altitude_interpolated_tide_corrected; /*  Mean surface altitude adjusted with tide correction  */
	      break;

	    case 52:
	      l_attr[index] = hawkeye.IR_polarisation_ratio;       /*  Amplitude ratio orthogonal/parallell polarisaion for infrared  */
	      break;

	    case 53:
	      l_attr[index] = hawkeye.Spatial_Quality;             /*  The spatial quality of a measurement  */
	      break;

	    case 54:
	      l_attr[index] = hawkeye.IR_AMPLITUDE;                /*  The amplitude of the parallell infrared pulse in Volts  */
	      break;

	    case 55:
	      l_attr[index] = hawkeye.Scan_Position;               /*    */
	      break;

	    case 56:
	      l_attr[index] = hawkeye.K_Lidar;                     /*  An estimate of the water column system diffuse attenuation coefficient  */
	      break;

	    case 57:
	      l_attr[index] = hawkeye.K_Lidar_Average;             /*    */
	      break;

	    case 58:
	      l_attr[index] = hawkeye.K_Lidar_Deep_Momentary;      /*    */
	      break;

	    case 59:
	      l_attr[index] = hawkeye.K_Lidar_Shallow_Momentary;   /*    */
	      break;

	    case 60:
	      l_attr[index] = hawkeye.Relative_reflectivity;       /*  Relative reflectivity  */
	      break;

	    case 61:
	      l_attr[index] = hawkeye.Waveform_Peak_Gain;          /*  Gain at point of peak  */
	      break;

	    case 62:
	      l_attr[index] = hawkeye.Adjusted_Amplitude;          /*  Peak amplitude adjusted for gain distance and pulsewidth  */
	      break;

	    case 63:
	      l_attr[index] = hawkeye.Water_Travel_Time;           /*  Travel time for pulse through water  */
	      break;

	    case 64:
	      l_attr[index] = hawkeye.Attenuation_c;               /*  Attenuation coefficient c /m  */
	      break;

	    case 65:
	      l_attr[index] = hawkeye.Absorption_a;                /*  Absorption coefficient a /m  */
	      break;

	    case 66:
	      l_attr[index] = hawkeye.Scattering_s;                /*  Scattering coefficient s /m  */
	      break;

	    case 67:
	      l_attr[index] = hawkeye.Backscattering_Bb;           /*  Backscattering coefficient Bb /m  */
	      break;
            }
        }
    }
}
