#include "pfmFeature.hpp"
#include "version.hpp"


void get_feature_event_time (NV_INT32 pfm_handle, DEPTH_RECORD depth, time_t *tv_sec, long *tv_nsec);


void build_feature (NV_INT32 pfm_handle, PFM_OPEN_ARGS open_args, NV_INT32 iho_order, NV_INT32 passes, OPTIONS *options,
                    QString feature_file, NV_INT32 *features_count, FEATURE **features, RUN_PROGRESS *progress)
{
  NV_INT32            bfd_handle, year, day, hour, minute, percent = 0, old_percent = -1;
  NV_FLOAT32          second;
  QString             title;
  NV_BOOL             new_file = NVFalse;
  BFDATA_HEADER       bfd_header;
  BFDATA_RECORD       bfd_record;
  BFDATA_POLYGON      bfd_poly;
  NV_I32_COORD2       coord;
  DEPTH_RECORD        *in_depth = NULL;



  //  Make sure that we can open and write to the output .bfd file.

  NV_CHAR ftr[512];
  strcpy (ftr, feature_file.toAscii ());

  if (options->replace)
    {
      memset (&bfd_header, 0, sizeof (BFDATA_HEADER));

      strcpy (bfd_header.creation_software, VERSION);

      if ((bfd_handle = binaryFeatureData_create_file (ftr, bfd_header)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::critical (0, pfmFeature::tr ("pfmFeature"), pfmFeature::tr ("Unable to create feature file.\nReason : ") + msg);
          exit (-1);
        }

      new_file = NVTrue;
    }
  else
    {
      if ((bfd_handle = binaryFeatureData_open_file (ftr, &bfd_header, BFDATA_UPDATE)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::critical (0, pfmFeature::tr ("pfmFeature"), pfmFeature::tr ("Unable to open feature file.\nReason : ") + msg);
          exit (-1);
        }
    }


    NV_FLOAT64 start_size = 1.5 * pow (2.0, (NV_FLOAT64) iho_order);


    time_t current_time = time (&current_time);

    cvtime ((time_t) current_time, 0, &year, &day, &hour, &minute, &second);
    year += 1900;


    NV_INT32 count = 0;
    for (NV_INT32 i = 0 ; i < passes ; i++)
      {
        NV_FLOAT64 bin_size = pow (2.0, (NV_FLOAT64) i) * start_size;

        title.sprintf (pfmFeature::tr ("Writing features, pass %d of %d, %.1f bin size").toAscii (), i + 1, passes, bin_size);
        progress->wbox->setTitle (title);
        qApp->processEvents ();


        old_percent = -1;
        percent = 0;


        for (NV_INT32 j = 0 ; j < features_count[i] ; j++)
          {
            memset (&bfd_record, 0, sizeof (BFDATA_RECORD));

            if (features[i][j].z > -13000.0)
              {
                //  Find the event time from the file pointed to in the PFM file.

                NV_F64_COORD2 xy = {features[i][j].x, features[i][j].y};
                compute_index_ptr (xy, &coord, &open_args.head);
                NV_INT32 numrecs;

                if (!read_depth_array_index (pfm_handle, coord, &in_depth, &numrecs))
                  {
                    for (NV_INT32 m = 0 ; m < numrecs ; m++)
                      {
                        //  Don't use invalid, deleted, or reference points.

                        if (!(in_depth[m].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                          {
                            if (fabs (in_depth[m].xyz.z - features[i][j].z) < 0.005 && 
                                fabs (in_depth[m].xyz.x - features[i][j].x) < 0.000001 && 
                                fabs (in_depth[m].xyz.y - features[i][j].y) < 0.000001)
                              {
                                //  Add PFM_SELECTED_FEATURE to the validity for the selected point

                                in_depth[m].validity |= PFM_SELECTED_FEATURE;

                                update_depth_record_index (pfm_handle, &in_depth[m]);


                                get_feature_event_time (pfm_handle, in_depth[m], &bfd_record.event_tv_sec, &bfd_record.event_tv_nsec);
                                cvtime (bfd_record.event_tv_sec, bfd_record.event_tv_nsec, &year, &day, &hour, &minute, &second);
                                break;
                              }
                          }
                      }
                    free (in_depth);
                  }
                else
                  {
                    inv_cvtime (year - 1900, day, hour, minute, 0.0, &bfd_record.event_tv_sec, &bfd_record.event_tv_nsec);
                  }

                sprintf (bfd_record.contact_id, "%02d%03d%02d%02d%05d", year % 100, day, hour, minute, count++);
                bfd_record.contact_id[14] = 0;


                //  Subtract off the offset (if any) prior to putting the depth in the feature.

                bfd_record.depth = (NV_FLOAT32) features[i][j].z - options->offset;


                strcpy (bfd_record.description, options->description.toAscii ());


                //  Find the octant position farthest from the shoal (feature) point.

                NV_FLOAT64 max_dist = -1.0;
                NV_INT32 oct_cnt = 0;

                for (NV_INT32 oct = 0 ; oct < 8 ; oct++)
                  {
                    if (features[i][j].oct_hit[oct])
                      {
                        if (options->output_polygons)
                          {
                            NV_FLOAT64 dist;
                            pfm_geo_distance (pfm_handle, features[i][j].y, features[i][j].x, features[i][j].oct_pos[oct].y, features[i][j].oct_pos[oct].x, &dist);

                            max_dist = qMax (dist, max_dist);
                          }
                        oct_cnt++;
                      }
                  }

                        
                switch (iho_order)
                  {
                  case 0:
                    if (options->output_polygons)
                      {
                        sprintf (bfd_record.remarks, "Created with pfmFeature, IHO Special order, bin size %.0f, octants %d, H/V %.2f/%.2f, #%d, max dist %.1f",
                                 bin_size, oct_cnt, features[i][j].h, features[i][j].v, count - 1, max_dist);
                      }
                    else
                      {
                        sprintf (bfd_record.remarks, "Created with pfmFeature, IHO Special order, bin size %.0f, octants %d, H/V %.2f/%.2f, #%d",
                                 bin_size, oct_cnt, features[i][j].h, features[i][j].v, count - 1);
                      }
                    break;

                  case 1:
                    if (options->output_polygons)
                      {
                        sprintf (bfd_record.remarks, "Created with pfmFeature, IHO Order 1, bin size %.0f, octants %d, H/V %.2f/%.2f, #%d, max dist %.1f",
                                 bin_size, oct_cnt, features[i][j].h, features[i][j].v, count - 1, max_dist);
                      }
                    else
                      {
                        sprintf (bfd_record.remarks, "Created with pfmFeature, IHO Order 1, bin size %.0f, octants %d, H/V %.2f/%.2f, #%d",
                                 bin_size, oct_cnt, features[i][j].h, features[i][j].v, count - 1);
                      }
                    break;
                  }


                bfd_record.confidence_level = features[i][j].confidence;


                bfd_record.latitude = features[i][j].y;
                bfd_record.longitude = features[i][j].x;


                //  Build a feature polygon from the nearest octant trigger points that exceed the combined horizontal uncertainty.
                //  This is only done if we have specified the (hidden) --polygon option on the command line.

                if (options->output_polygons)
                  {
                    bfd_record.poly_count = 0;
                    bfd_record.poly_type = 1;
                    for (NV_INT32 oct = 0 ; oct < 8 ; oct++)
                      {
                        if (features[i][j].oct_hit[oct])
                          {
                            bfd_poly.latitude[bfd_record.poly_count] = features[i][j].oct_pos[oct].y;
                            bfd_poly.longitude[bfd_record.poly_count] = features[i][j].oct_pos[oct].x;
                            bfd_record.poly_count++;
                          }
                      }
                  }


                strcpy (bfd_record.analyst_activity, "NAVOCEANO BHY");

                bfd_record.equip_type = 3;

                bfd_record.nav_system = 1;

                bfd_record.platform_type = 4;

                bfd_record.sonar_type = 3;

                if (binaryFeatureData_write_record (bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_poly, NULL) < 0)
                  {
                    binaryFeatureData_perror ();
                    exit (-1);
                  }
              }


            percent = NINT (((NV_FLOAT32) j / (NV_FLOAT32) features_count[i]) * 100.0);
            if (percent != old_percent)
              {
                progress->wbar->setValue (percent);
                old_percent = percent;
              }

            qApp->processEvents ();
          }
      }


    binaryFeatureData_close_file (bfd_handle);


    //  If we didn't find any features (an admittedly rare occurrence) we need to get rid of the files and put out a message.

    if (!count)
      {
        //  Remove the .bfd file.

        remove (ftr);


        //  Remove the .bfa file.

        ftr[strlen (ftr) - 1] = 'a';
        remove (ftr);


        QMessageBox::information (0, pfmFeature::tr ("pfmFeature"), pfmFeature::tr ("No features were detected.\nFeature file not created."));
      }
    else
      {
        //  Make this the PFMs feature file.

        update_target_file (pfm_handle, open_args.list_path, ftr);
      }


    progress->wbar->setValue (100);
    qApp->processEvents ();
}
