#include "io_data.hpp"


static NV_INT32             type, curr_record, end_record;
static FILE                 *fp;
static NV_CHAR              filename[1024];
static NV_BOOL              no_edit;


//  CHARTS

static HYDRO_OUTPUT_T       hof_record;
static TOPO_OUTPUT_T        tof_record;
static HOF_HEADER_T         hof_header;
static TOF_HEADER_T         tof_header;
static NV_INT32             year, day, hour, minute;
static NV_FLOAT32           second;


//  WLF

static NV_INT32             wlf_handle;
static WLF_RECORD           wlf_record;
static WLF_HEADER           wlf_header;


//  HAWKEYE

static NV_INT32             hawkeye_handle;
static HAWKEYE_RECORD       hawkeye_record;
static HAWKEYE_META_HEADER  *hawkeye_meta_header;
static HAWKEYE_CONTENTS_HEADER *hawkeye_contents_header;


//  GSF

static NV_INT32             gsf_handle;
static gsfDataID            gsf_data_id;
static gsfRecords           gsf_records;
static NV_INT32             last_rec = -1;



static QProgressBar *indexProg;


static void indexProgress (int state, int percent)
{
  static NV_BOOL second = NVFalse;

  if (state == 1) second = NVFalse;

  if (!second && state == 2) 
    {
      indexProg->reset ();
      second = NVTrue;
    }
      
  if (state == 2 || percent < 100) indexProg->setValue (percent);

  qApp->processEvents ();
}



static void allocate_record (POINT_DATA *data, NV_INT32 num)
{
  if ((data->x = (NV_FLOAT64 *) realloc (data->x, (num + 1) * sizeof (NV_FLOAT64))) == NULL ||
      (data->y = (NV_FLOAT64 *) realloc (data->y, (num + 1) * sizeof (NV_FLOAT64))) == NULL ||
      (data->z = (NV_FLOAT32 *) realloc (data->z, (num + 1) * sizeof (NV_FLOAT32))) == NULL ||
      (data->val = (NV_U_INT32 *) realloc (data->val, (num + 1) * sizeof (NV_U_INT32))) == NULL ||
      (data->oval = (NV_U_INT32 *) realloc (data->oval, (num + 1) * sizeof (NV_U_INT32))) == NULL ||
      (data->rec = (NV_U_INT32 *) realloc (data->rec, (num + 1) * sizeof (NV_U_INT32))) == NULL ||
      (data->sub = (NV_INT32 *) realloc (data->sub, (num + 1) * sizeof (NV_INT32))) == NULL ||
      (data->time = (NV_FLOAT64 *) realloc (data->time, (num + 1) * sizeof (NV_FLOAT64))) == NULL ||
      (data->mask = (NV_BOOL *) realloc (data->mask, (num + 1) * sizeof (NV_BOOL))) == NULL ||
      (data->fmask = (NV_BOOL *) realloc (data->fmask, (num + 1) * sizeof (NV_BOOL))) == NULL)
    {
      fprintf (stderr, "Error allocating memory for data, exiting\n");
      exit (-1);
    }
}



NV_INT32 io_data_open (MISC *misc, OPTIONS *options)
{
  time_t sec;
  long nsec;


  strcpy (filename, misc->file.toAscii ());

  misc->data_type = -1;

  no_edit = misc->no_edit;


  //  Check for HOF file.

  if (misc->file.endsWith (".hof"))
    {
      if ((fp = open_hof_file (filename)) == NULL) return (-1);

      hof_read_header (fp, &hof_header);

      if (strstr (hof_header.text.file_type, "Hydrographic Output File"))
        {
          misc->data_type = type = PFM_CHARTS_HOF_DATA;
          misc->lidar_present = misc->hydro_lidar_present = NVTrue;
        }
      else
        {
          fclose (fp);
          return (-1);
        }


      fclose (fp);
    }


  //  Check for TOF file.

  if (misc->file.endsWith (".tof"))
    {
      if ((fp = open_tof_file (filename)) == NULL) return (-1);

      tof_read_header (fp, &tof_header);

      if (strstr (tof_header.text.file_type, "Topographic Output File"))
        {
          misc->data_type = type = PFM_SHOALS_TOF_DATA;
          misc->lidar_present = NVTrue;
          misc->hydro_lidar_present = NVFalse;
        }
      else
        {
          fclose (fp);
          return (-1);
        }

      fclose (fp);
    }


  //  Check for WLF file.

  if (misc->file.endsWith (".wlf") || misc->file.endsWith (".whf") || misc->file.endsWith (".wtf"))
    {
      if ((wlf_handle = wlf_open_file (filename, &wlf_header, WLF_UPDATE)) >= 0)
        {
          misc->data_type = type = PFM_WLF_DATA;

          if (misc->file.endsWith (".whf"))
            {
              misc->lidar_present = misc->hydro_lidar_present = NVTrue;
            }
          else
            {
              misc->lidar_present = NVTrue;
              misc->hydro_lidar_present = NVFalse;
            }

          wlf_close_file (wlf_handle);
        }
    }


  //  Check for HAWKEYE file.

  if (misc->data_type < 0 && (hawkeye_handle = hawkeye_open_file (filename, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_UPDATE)) >= 0)
    {
      if (hawkeye_contents_header->data_type == HAWKEYE_HYDRO_BIN_DATA)
        {
          misc->data_type = type = PFM_HAWKEYE_HYDRO_DATA;
        }
      else
        {
          misc->data_type = type = PFM_HAWKEYE_TOPO_DATA;
        }


      //  Check for the required fields.

      if (hawkeye_contents_header->data_type != HAWKEYE_HYDRO_BIN_DATA)
        {
          if (!hawkeye_contents_header->available.Timestamp || !hawkeye_contents_header->available.Point_Latitude ||
              !hawkeye_contents_header->available.Point_Longitude || !hawkeye_contents_header->available.Point_Altitude ||
              !hawkeye_contents_header->available.Manual_Output_Screening_Flags || !hawkeye_contents_header->available.Return_Number)
            {
              QMessageBox::critical (0, "geoSwath3D", QString ("Missing required point record fields in HAWKEYE file %1").arg (filename));
              hawkeye_close_file (hawkeye_handle);
              return (-1);
            }

          misc->lidar_present = misc->hydro_lidar_present = NVTrue;
        }
      else
        {
          if (!hawkeye_contents_header->available.Timestamp || !hawkeye_contents_header->available.Point_Latitude ||
              !hawkeye_contents_header->available.Point_Longitude || !hawkeye_contents_header->available.Depth ||
              !hawkeye_contents_header->available.Depth_Confidence || !hawkeye_contents_header->available.Manual_Output_Screening_Flags ||
              !hawkeye_contents_header->available.Return_Number)
            {
              QMessageBox::critical (0, "geoSwath3D", QString ("Missing required point record fields in HAWKEYE file %1").arg (filename));
              hawkeye_close_file (hawkeye_handle);
              return (-1);
            }
          misc->lidar_present = NVTrue;
        }


      hawkeye_close_file (hawkeye_handle);
    }


  if (misc->data_type == PFM_CHARTS_HOF_DATA)
    {
      if ((fp = open_hof_file (filename)) == NULL) return (-1);


      hof_read_header (fp, &hof_header);

      misc->num_subrecords = 2;
      misc->num_records = options->point_limit / misc->num_subrecords;

      charts_cvtime (hof_header.text.start_timestamp, &year, &day, &hour, &minute, &second);
      misc->start_time = normtime (year, day, hour, minute, second);

      charts_cvtime (hof_header.text.end_timestamp, &year, &day, &hour, &minute, &second);
      misc->end_time = normtime (year, day, hour, minute, second);

      misc->total_records = hof_header.text.number_shots;

      misc->start_record = 1;
      misc->end_record = end_record = misc->total_records;

      curr_record = 1;
    }
  else if (misc->data_type == PFM_SHOALS_TOF_DATA)
    {
      if ((fp = open_tof_file (filename)) == NULL) return (-1);

      tof_read_header (fp, &tof_header);


      misc->num_subrecords = 2;
      misc->num_records = options->point_limit / misc->num_subrecords;

      charts_cvtime (tof_header.text.start_timestamp, &year, &day, &hour, &minute, &second);
      misc->start_time = normtime (year, day, hour, minute, second);

      charts_cvtime (tof_header.text.end_timestamp, &year, &day, &hour, &minute, &second);
      misc->end_time = normtime (year, day, hour, minute, second);

      misc->total_records = tof_header.text.number_shots;

      misc->start_record = 1;
      misc->end_record = end_record = misc->total_records;

      curr_record = 1;
    }
  else if (misc->data_type == PFM_WLF_DATA)
    {
      if ((wlf_handle = wlf_open_file (filename, &wlf_header, WLF_UPDATE)) < 0) return (-1);

      misc->num_subrecords = 2;
      misc->num_records = options->point_limit / misc->num_subrecords;

      wlf_read_record (wlf_handle, 0, &wlf_record, NVFalse, NULL);

      cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);
      misc->start_time = normtime (year, day, hour, minute, second);


      wlf_read_record (wlf_handle, wlf_header.number_of_records - 1, &wlf_record, NVFalse, NULL);

      cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);
      misc->end_time = normtime (year, day, hour, minute, second);


      misc->total_records = wlf_header.number_of_records;

      misc->start_record = 0;
      misc->end_record = end_record = misc->total_records - 1;

      curr_record = 0;
    }
  else if (misc->data_type == PFM_HAWKEYE_HYDRO_DATA || misc->data_type == PFM_HAWKEYE_TOPO_DATA)
    {
      if ((hawkeye_handle = hawkeye_open_file (filename, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_UPDATE)) < 0) return (-1);

      misc->num_subrecords = 1;
      misc->num_records = options->point_limit / misc->num_subrecords;

      hawkeye_read_record (hawkeye_handle, 0, &hawkeye_record);

      cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);
      misc->start_time = normtime (year, day, hour, minute, second);


      hawkeye_read_record (hawkeye_handle, hawkeye_contents_header->NbrOfPointRecords - 1, &hawkeye_record);

      cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);
      misc->end_time = normtime (year, day, hour, minute, second);


      misc->total_records = hawkeye_contents_header->NbrOfPointRecords;

      misc->start_record = 0;
      misc->end_record = end_record = misc->total_records - 1;

      curr_record = 0;
    }
  else
    {
      indexProg = misc->statusProg;
      indexProg->setRange (0, 100);

      gsf_register_progress_callback (indexProgress);

      if (gsfOpen (filename, GSF_UPDATE_INDEX, &gsf_handle)) return (-1);


      //  Read one record to get the number of beams available.

      gsf_data_id.record_number = 1;
      if ((gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_records, NULL, 0)) != -1)
        {
          misc->num_subrecords = gsf_records.mb_ping.number_beams;
        }
      else
        {
          misc->num_subrecords = 1024;
        }
      misc->num_records = options->point_limit / misc->num_subrecords;


      indexProg->setValue (100);


      misc->data_type = type = PFM_GSF_DATA;


      misc->gsf_present = NVTrue;


      gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, 1, &sec, &nsec);
      misc->start_time = norm_posix_time (sec, nsec);
        
      misc->total_records = gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, -1, &sec, &nsec) + 1;
      misc->end_time = norm_posix_time (sec, nsec);

      misc->start_record = 1;
      misc->end_record = end_record = misc->total_records;

      curr_record = 1;
    }


  strcpy (misc->abe_share->nearest_filename, filename);


  return (0);
}




void io_data_close ()
{
  switch (type)
    {
    case PFM_CHARTS_HOF_DATA:
    case PFM_SHOALS_TOF_DATA:
      fclose (fp);
      break;

    case PFM_WLF_DATA:
      wlf_close_file (wlf_handle);
      break;

    case PFM_HAWKEYE_HYDRO_DATA:
    case PFM_HAWKEYE_TOPO_DATA:
      hawkeye_close_file (hawkeye_handle);
      break;

    case PFM_GSF_DATA:

      //  Flush the last write buffer.

      if (last_rec != -1 && !no_edit)
        {
          gsf_data_id.record_number = last_rec;
          gsfWrite (gsf_handle, &gsf_data_id, &gsf_records);
          last_rec = -1;
        }
      gsfClose (gsf_handle);
      break;
    }
}


    

void io_data_read (NV_INT32 num_records, POINT_DATA *data, QProgressBar *prog)
{
  NV_INT32 i, j, k;
  NV_FLOAT64 lat, lon, ang1, ang2, nlat, nlon, lateral, lat2, lon2;
  NV_BOOL nominal = NVFalse;


  if (num_records == 0)
    {
      printf("\n\n***YOU MUST OPEN A FILE BEFORE SCROLLING FORWARD***\n\n");
      exit(-1);
    }
        


  NV_INT32 end = qMin (curr_record + num_records, end_record) - curr_record;
  
  k = 0;
  prog->setRange (0, end);
  NV_INT32 inc = end / 10;

  for (j = 0 ; j < end ; j++)
    {
      if (!(j % inc)) prog->setValue (j);

      switch (type)
        {
        case PFM_SHOALS_TOF_DATA:
          tof_read_record (fp, curr_record, &tof_record);


          //  First return.

          if (tof_record.elevation_first != -998.0)
            {
              allocate_record (data, k);

              data->y[k] = tof_record.latitude_first;
              data->x[k] = tof_record.longitude_first;
              data->z[k] = -tof_record.elevation_first;

              charts_cvtime (tof_record.timestamp, &year, &day, &hour, &minute, &second);

              data->time[k] = normtime (year, day, hour, minute, second);
              data->val[k] = 0;
              data->rec[k] = curr_record;
              data->sub[k] = 0;
              data->mask[k] = 0;
              data->fmask[k] = NVFalse;

              if (tof_record.conf_first < 50) data->val[k] = PFM_FILTER_INVAL;
              k++;
            }


            if (tof_record.elevation_last != -998.0) 
              {
                allocate_record (data, k);

                data->y[k] = tof_record.latitude_last;
                data->x[k] = tof_record.longitude_last;
                data->z[k] = -tof_record.elevation_last;

                data->time[k] = normtime (year, day, hour, minute, second);
                data->val[k] = 0;
                data->rec[k] = curr_record;
                data->sub[k] = 1;
                data->mask[k] = 0;
                data->fmask[k] = NVFalse;

                if (tof_record.conf_last < 50) data->val[k] = PFM_FILTER_INVAL;
                k++;
              }
            curr_record++;
            break;


          case PFM_CHARTS_HOF_DATA:
            hof_read_record (fp, curr_record, &hof_record);

            if (hof_record.abdc)
              {
                allocate_record (data, k);

                data->y[k] = hof_record.latitude;
                data->x[k] = hof_record.longitude;
                data->z[k] = -hof_record.correct_depth;

                charts_cvtime (hof_record.timestamp, &year, &day, &hour, &minute, &second);

                data->time[k] = normtime (year, day, hour, minute, second);
                data->val[k] = 0;
                data->rec[k] = curr_record;
                data->sub[k] = 0;
                data->mask[k] = 0;
                data->fmask[k] = NVFalse;

                if (hof_record.status & AU_STATUS_DELETED_BIT) data->val[k] = PFM_MANUALLY_INVAL;
                if (hof_record.abdc < 70) data->val[k] = PFM_FILTER_INVAL;


                if (hof_record.suspect_status & SUSPECT_STATUS_SUSPECT_BIT) data->val[k] |= PFM_SUSPECT;
                if (hof_record.suspect_status & SUSPECT_STATUS_FEATURE_BIT) data->val[k] |= PFM_SELECTED_FEATURE;


                if (hof_record.data_type) data->val[k] |= PFM_USER_01;
                if (hof_record.status & AU_STATUS_SWAPPED_BIT) data->val[k] |= PFM_USER_02;
                if (hof_record.abdc == 70) data->val[k] |= PFM_USER_03;
                k++;
              }


            if (hof_record.correct_sec_depth != -998.0)
              {
                allocate_record (data, k);

                data->y[k] = hof_record.sec_latitude;
                data->x[k] = hof_record.sec_longitude;
                data->z[k] = -hof_record.correct_sec_depth;

                data->time[k] = normtime (year, day, hour, minute, second);
                data->val[k] = 0;
                data->rec[k] = curr_record;
                data->sub[k] = 1;
                data->mask[k] = 0;
                data->fmask[k] = NVFalse;

                if (hof_record.sec_abdc < 70) data->val[k] = PFM_FILTER_INVAL;

                k++;
              }


            curr_record++;
            break;


          case PFM_WLF_DATA:
            if (wlf_read_record (wlf_handle, curr_record, &wlf_record, NVFalse, NULL) < 0)
              {
                wlf_perror ();
              }
            else
              {
                allocate_record (data, k);

                data->y[k] = wlf_record.y;
                data->x[k] = wlf_record.x;
                data->z[k] = -wlf_record.z;

                cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);

                data->time[k] = normtime (year, day, hour, minute, second);
                data->val[k] = 0;
                data->rec[k] = curr_record;
                data->sub[k] = wlf_record.return_number;
                data->mask[k] = 0;
                data->fmask[k] = NVFalse;

                if (wlf_record.status)
                  {
                    if (wlf_record.status & WLF_MANUALLY_INVAL) data->val[k] = PFM_MANUALLY_INVAL;
                    if (wlf_record.status & WLF_FILTER_INVAL) data->val[k] = PFM_FILTER_INVAL;
                    if (wlf_record.status & WLF_SUSPECT) data->val[k] |= PFM_SUSPECT;
                    if (wlf_record.status & WLF_SELECTED_FEATURE) data->val[k] |= PFM_SELECTED_FEATURE;
                    if (wlf_record.status & WLF_SELECTED_SOUNDING) data->val[k] |= PFM_SELECTED_SOUNDING;
                    if (wlf_record.status & WLF_DESIGNATED_SOUNDING) data->val[k] |= PFM_DESIGNATED_SOUNDING;
                  }
                k++;


                curr_record++;
              }
            break;


          case PFM_HAWKEYE_HYDRO_DATA:
          case PFM_HAWKEYE_TOPO_DATA:
            if (hawkeye_read_record (hawkeye_handle, curr_record, &hawkeye_record) < 0)
              {
                hawkeye_perror ();
              }
            else
              {
                allocate_record (data, k);

                data->y[k] = hawkeye_record.Point_Latitude;
                data->x[k] = hawkeye_record.Point_Longitude;

                if (type == PFM_HAWKEYE_HYDRO_DATA)
                  {
                    data->z[k] = -hawkeye_record.Depth;

                    if (hawkeye_record.Depth_Confidence <= 0.0) data->val[k] = PFM_FILTER_INVAL;
                  }
                else
                  {
                    data->z[k] = hawkeye_record.Point_Altitude;
                  }

                cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);

                data->time[k] = normtime (year, day, hour, minute, second);
                data->val[k] = 0;
                data->rec[k] = curr_record;
                data->sub[k] = hawkeye_record.Return_Number;
                data->mask[k] = 0;
                data->fmask[k] = NVFalse;

                if (hawkeye_record.Manual_Output_Screening_Flags & HAWKEYE_INVALID) data->val[k] = PFM_MANUALLY_INVAL;

                k++;


                curr_record++;
              }
            break;


          case PFM_GSF_DATA:

            //  Flush the last write buffer.
    
            if (last_rec != -1)
              {
                gsf_data_id.record_number = last_rec;
                gsfWrite (gsf_handle, &gsf_data_id, &gsf_records);
                last_rec = -1;
              }
        

            //  Read a GSF record.
        
            gsf_data_id.record_number = curr_record;
            if ((gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_records, NULL, 0)) != -1)
              {
                if (!(gsf_records.mb_ping.ping_flags & GSF_IGNORE_PING))
                  {
                    //  If all true depth values are zero or there is no true depth field use nominal depth.

                    if (gsf_records.mb_ping.depth != NULL)
                      {
                        NV_BOOL hit = NVFalse;
                        for (i = 0 ; i < gsf_records.mb_ping.number_beams ; i++)
                          {
                            if (gsf_records.mb_ping.depth[i] != 0.0)
                              {
                                hit = NVTrue;
                                break;
                              }
                          }

                        if (hit && gsf_records.mb_ping.nominal_depth != NULL)
                          {
                            nominal = NVFalse;
                          }
                        else
                          {
                            nominal = NVTrue;
                          }
                      }
                    else
                      {
                        nominal = NVTrue;
                      }


                    //  Snag the lat, lon, and heading.
            
                    lat  = gsf_records.mb_ping.latitude;
                    lon  = gsf_records.mb_ping.longitude;
                    ang1 = gsf_records.mb_ping.heading + 90.0;
                    ang2 = gsf_records.mb_ping.heading;


                    //  Check for the ever popular bad nav data.

                    if (lat <= 90.0 && lon <= 180.0)
                      {
                        //  Georeference each beam.
            
                        for (i = 0 ; i < gsf_records.mb_ping.number_beams ; i++)
                          {
                            if (!check_flag (gsf_records.mb_ping.beam_flags[i], HMPS_IGNORE_NULL_BEAM))
                              {
                                //  If the across track array is present then use it.

                                nlat = lat;
                                nlon = lon;
                                if (gsf_records.mb_ping.across_track != (NV_FLOAT64 *) NULL)
                                  {
                                    lateral = gsf_records.mb_ping.across_track[i];
                                    newgp (lat, lon, ang1, lateral, &nlat, &nlon);
                                  }


                                //  If the along track array is present then use it.

                                if (gsf_records.mb_ping.along_track != (NV_FLOAT64 *) NULL)
                                  {
                                    lat2=nlat;
                                    lon2=nlon;
                                    lateral = gsf_records.mb_ping.along_track[i];
                                    newgp (lat2, lon2, ang2, lateral, &nlat, &nlon);
                                  }


                                allocate_record (data, k);


                                //  Load the arrays.

                                data->y[k] = nlat;
                                data->x[k] = nlon;
                                if (nominal && gsf_records.mb_ping.nominal_depth != NULL)
                                  {
                                    data->z[k] = gsf_records.mb_ping.nominal_depth[i];
                                  }
                                else
                                  {
                                    data->z[k] = gsf_records.mb_ping.depth[i];
                                  }
                                data->time[k] = norm_posix_time (gsf_records.mb_ping.ping_time.tv_sec, gsf_records.mb_ping.ping_time.tv_nsec);
                                data->val[k] = 0;
                                data->rec[k] = curr_record;
                                data->sub[k] = i;
                                data->mask[k] = 0;
                                data->fmask[k] = NVFalse;

                                gsf_to_pfm_flags (&data->val[k], gsf_records.mb_ping.beam_flags[i]);

                                k++;
                              }
                          }
                      }
                  }
              }
            curr_record++;

            break;
        }
    }


  prog->setValue (end);
  qApp->processEvents ();

  data->count = k;
}




void io_data_write (NV_U_INT32 val, NV_INT32 rec, NV_INT32 sub, NV_FLOAT64 time)
{
    NV_INT64      timestamp;


    if (no_edit) return;


    switch (type)
      {
      case PFM_SHOALS_TOF_DATA:

        //  Read a record.  You don't have enough information to rebuild the record so you have 
        //  to read it.

        tof_read_record (fp, rec, &tof_record);
        timestamp = tof_record.timestamp;

        charts_cvtime (timestamp, &year, &day, &hour, &minute, &second);


        //  Test the time passed back from the program against the one just read in
        //  for the same rec.  If they don't match, you've got problems.
        
        if (fabs (normtime (year, day, hour, minute, second) - time) > 0.000001)
          {
            printf ("Time mismatch in io_data_write %f %f %d\n",
                    normtime (year, day, hour, minute, second), time, rec);
            exit (-1);
          }


        if (sub)
          {
            if (val & PFM_INVAL)
              {
                if (tof_record.conf_last > 0) tof_record.conf_last = -tof_record.conf_last;
              }
            else
              {
                if (tof_record.conf_last < 0) tof_record.conf_last = -tof_record.conf_last;
              }
          }
        else
          {
            if (val & PFM_INVAL)
              {
                if (tof_record.conf_first > 0) tof_record.conf_first = -tof_record.conf_first;
              }
            else
              {
                if (tof_record.conf_first < 0) tof_record.conf_first = -tof_record.conf_first;
              }
          }


        tof_write_record (fp, rec, tof_record);

        break;


      case PFM_CHARTS_HOF_DATA:

        //  Read a record.  You don't have enough information to rebuild the record so you have 
        //  to read it.

        hof_read_record (fp, rec, &hof_record);
        timestamp = hof_record.timestamp;


        charts_cvtime (timestamp, &year, &day, &hour, &minute, &second);


        //  Test the time passed back from the program against the one just read in
        //  for the same rec.  If they don't match, you've got problems.
        
        if (fabs (normtime (year, day, hour, minute, second) - time) > 0.000001)
          {
            printf ("Time mismatch in io_data_write %f %f %d\n",
                    normtime (year, day, hour, minute, second), time, rec);
            exit (-1);
          }

        /*  HOF uses the lower three bits of the status field for status thusly :

        bit 0 = deleted    (1) 
        bit 1 = kept       (2) 
        bit 2 = swapped    (4)      
        */

        hof_record.status = 0;
        if (val & PFM_INVAL) hof_record.status |= AU_STATUS_DELETED_BIT;


        hof_record.suspect_status = 0;
        if (val & PFM_SUSPECT) hof_record.suspect_status = SUSPECT_STATUS_SUSPECT_BIT;
        if (val & PFM_SELECTED_FEATURE) hof_record.suspect_status |= SUSPECT_STATUS_FEATURE_BIT;


        //  Another thing with the hof data - if the invalid bit is set we also set the abdc to a 
        //  negative (or vice-versa).

        if (val & PFM_INVAL)
          {
            if (hof_record.abdc > 0) hof_record.abdc = -hof_record.abdc;
          }
        else
          {
            if (hof_record.abdc < 0) hof_record.abdc = -hof_record.abdc;
          }

        hof_write_record (fp, rec, hof_record);

        break;


      case PFM_WLF_DATA:

        //  Read a record.  You don't have enough information to rebuild the record so you have 
        //  to read it.

        wlf_read_record (wlf_handle, rec, &wlf_record, NVFalse, NULL);

        cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);


        //  Test the time passed back from the program against the one just read in
        //  for the same rec.  If they don't match, you've got problems.
        
        if (fabs (normtime (year, day, hour, minute, second) - time) > 0.000001)
          {
            printf ("Time mismatch in io_data_write %f %f %d\n",
                    normtime (year, day, hour, minute, second), time, rec);
            exit (-1);
          }


        if (val & WLF_MANUALLY_INVAL) wlf_record.status = PFM_MANUALLY_INVAL;
        if (val & WLF_FILTER_INVAL) wlf_record.status = PFM_FILTER_INVAL;
        if (val & WLF_SUSPECT) wlf_record.status |= PFM_SUSPECT;
        if (val & WLF_SELECTED_FEATURE) wlf_record.status |= PFM_SELECTED_FEATURE;
        if (val & WLF_SELECTED_SOUNDING) wlf_record.status |= PFM_SELECTED_SOUNDING;
        if (val & WLF_DESIGNATED_SOUNDING) wlf_record.status |= PFM_DESIGNATED_SOUNDING;

        wlf_update_record (wlf_handle, rec, wlf_record);

        break;


      case PFM_HAWKEYE_HYDRO_DATA:
      case PFM_HAWKEYE_TOPO_DATA:

        //  Read a record.  You don't have enough information to rebuild the record so you have to read it.

        hawkeye_read_record (hawkeye_handle, rec, &hawkeye_record);

        cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);


        //  Test the time passed back from the program against the one just read in
        //  for the same rec.  If they don't match, you've got problems.
        
        if (fabs (normtime (year, day, hour, minute, second) - time) > 0.000001)
          {
            printf ("Time mismatch in io_data_write %f %f %d\n", normtime (year, day, hour, minute, second), time, rec);
            exit (-1);
          }


        if (val & PFM_INVAL)
          {
            hawkeye_record.Manual_Output_Screening_Flags = HAWKEYE_INVALID;
          }
        else
          {
            hawkeye_record.Manual_Output_Screening_Flags = HAWKEYE_VALID;
          }

        hawkeye_update_record (hawkeye_handle, rec, hawkeye_record);

        break;


      case PFM_GSF_DATA:

        //  If the rec number has changed, flush the last write buffer.
    
        if (rec != last_rec)
          {
            //  Don't write the first time through.
        
            if (last_rec != -1)
              {
                gsf_data_id.record_number = last_rec;
                gsfWrite (gsf_handle, &gsf_data_id, &gsf_records);
              }


            //  Read a record.  You don't have enough information to rebuild the record so you 
            //  have to read it.

            gsf_data_id.record_number = rec;
            if ((gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_records, NULL, 0)) == -1)
              {
                fprintf (stderr, "Invalid rec number in io_data_write %d\n", rec);
                exit (-1);
              }

            //  Save the last rec number.
        
            last_rec = rec;
          }


        //  Test the time passed back from the program against the one just read in
        //  for the same rec.  If they don't match, you've got problems.
        
        if (fabs (norm_posix_time (gsf_records.mb_ping.ping_time.tv_sec, 
                                   gsf_records.mb_ping.ping_time.tv_nsec) - time) > 0.000001)
          {
            printf ("Time mismatch in io_data_write %f %f %d\n",
                    norm_posix_time (gsf_records.mb_ping.ping_time.tv_sec,
                                     gsf_records.mb_ping.ping_time.tv_nsec), time, rec);
            exit (-1);
          }


        //  Convert the PFM validity data to GSF validity data (we're buffering by ping so we'll write it out later).

        //  Don't do anything if the entire ping was bad.
    
        if (!(gsf_records.mb_ping.ping_flags & GSF_IGNORE_PING))
          {
            //  Set the beam flags.  Don't write the record yet, we'll do that if/when the record changes

            pfm_to_gsf_flags (val, &gsf_records.mb_ping.beam_flags[sub]);
        }
        break;
    }
}



NV_INT32 get_record ()
{
    /*  REQUIRED    */
    /*  Return the current record number in the swath file. */
    
    return (curr_record);
}



void set_record (NV_INT32 record)
{
    /*  REQUIRED    */
    /*  Set the current record number or position to the specified record.  */
    
    curr_record = record;
}



NV_FLOAT64 rec_time (NV_INT32 record)
{
  time_t sec;
  long nsec;
  HYDRO_OUTPUT_T hof;
  TOPO_OUTPUT_T tof;
  NV_FLOAT64 time = 0.0;


  switch (type)
    {
    case PFM_SHOALS_TOF_DATA:
      tof_read_record (fp, record, &tof);

      charts_cvtime (tof.timestamp, &year, &day, &hour, &minute, &second);

      time = normtime (year, day, hour, minute, second);
      break;

    case PFM_CHARTS_HOF_DATA:
      hof_read_record (fp, record, &hof);

      charts_cvtime (hof.timestamp, &year, &day, &hour, &minute, &second);

      time = normtime (year, day, hour, minute, second);
      break;

    case PFM_GSF_DATA:
      gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, record, &sec, &nsec);
      time = norm_posix_time (sec, nsec);
      break;
    }

  return (time);
}
