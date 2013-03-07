#include "pfm3D.hpp"

void paint_surface (pfm3D *parent, MISC *misc, nvMapGL *map, NV_BOOL reset)
{
  static NV_F64_XYMBR prev_mbr = {-999.0, -999.0, -999.0, -999.0};
  static NV_FLOAT64 prev_min_z = CHRTRNULL, prev_max_z = -CHRTRNULL;
  static NV_INT32 prev_display_GeoTIFF = 0;
  NV_FLOAT32 **data;
  NV_F64_XYMBC mbc;


  void adjust_bounds (MISC *misc, NV_INT32 pfm);
  QImage *geotiff (NV_F64_XYMBR mbr, NV_CHAR *geotiff_file, NV_F64_XYMBR *geotiff_mbr);
  void geotiff_clear ();



  //  Make an MBC from the MBR and the data.

  mbc.min_x = misc->abe_share->viewer_displayed_area.min_x;
  mbc.min_y = misc->abe_share->viewer_displayed_area.min_y;
  mbc.max_x = misc->abe_share->viewer_displayed_area.max_x;
  mbc.max_y = misc->abe_share->viewer_displayed_area.max_y;
  mbc.max_z = -999999999.0;
  mbc.min_z = 999999999.0;


  //  First we have to compute the minimum bounding cube for all of the available PFMs

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
          if (misc->pfm_handle[pfm] >= 0) close_pfm_file (misc->pfm_handle[pfm]);

          if ((misc->pfm_handle[pfm] = open_existing_pfm_file (&misc->abe_share->open_args[pfm])) < 0)
            {
              QMessageBox::warning (parent, pfm3D::tr ("Open PFM Structure"), pfm3D::tr ("The file ") + 
                                    QDir::toNativeSeparators (QString (misc->abe_share->open_args[pfm].list_path)) + 
                                    pfm3D::tr (" is not a PFM handle or list file or there was an error reading the file.") +
                                    pfm3D::tr ("  The error message returned was:\n\n") +
                                    QString (pfm_error_str (pfm_error)));
              parent->clean_exit (-1);
            }

          if (misc->abe_share->open_args[pfm].head.proj_data.projection)
            {
              QMessageBox::warning (parent, pfm3D::tr ("Open PFM Structure"), pfm3D::tr ("Sorry, pfm3D does not handle projected data."));
              close_pfm_file (misc->pfm_handle[pfm]);
              printf ("0\n");
              parent->clean_exit (-1);
            }


          //  Adjust bounds to nearest grid point (compute displayed_area).

          adjust_bounds (misc, pfm);


          //  If none of the area is in the displayed area the width and/or height will be 0 or negative.

          if (misc->displayed_area_width > 0 && misc->displayed_area_height > 0)
            {
              BIN_RECORD *bin;

              bin = (BIN_RECORD *) malloc (sizeof (BIN_RECORD) * misc->displayed_area_width);
              if (bin == NULL)
                {
                  perror ("Allocating bin memory in paint_surface");
                  parent->clean_exit (-1);
                }


              for (NV_INT32 i = 0 ; i < misc->displayed_area_height ; i++)
                {
                  read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width, misc->displayed_area_row + i, misc->displayed_area_column, bin);

                  NV_FLOAT32 z = -misc->abe_share->open_args[pfm].head.null_depth;

                  for (NV_INT32 j = 0 ; j < misc->displayed_area_width ; j++)
                    {
                      switch (misc->abe_share->layer_type)
                        {
                        case AVERAGE_FILTERED_DEPTH:
                        default:
                          if (bin[j].validity & (PFM_DATA | PFM_INTERPOLATED)) z = -bin[j].avg_filtered_depth;
                          break;

                        case MIN_FILTERED_DEPTH:
                          if (bin[j].validity & PFM_DATA) z = -bin[j].min_filtered_depth;
                          break;

                        case MAX_FILTERED_DEPTH:
                          if (bin[j].validity & PFM_DATA) z = -bin[j].max_filtered_depth;
                          break;

                        case AVERAGE_DEPTH:
                          if (bin[j].num_soundings) z = -bin[j].avg_depth;
                          break;

                        case MIN_DEPTH:
                          if (bin[j].num_soundings) z = -bin[j].min_depth;
                          break;

                        case MAX_DEPTH:
                          if (bin[j].num_soundings) z = -bin[j].max_depth;
                          break;
                        }

                      if (z != -misc->abe_share->open_args[pfm].head.null_depth)
                        {
                          mbc.max_z = qMax (mbc.max_z, (NV_FLOAT64) z);
                          mbc.min_z = qMin (mbc.min_z, (NV_FLOAT64) z);
                        }
                    }
                }

              free (bin);
            }
        }
    }


  //  Have to set the bounds prior to doing anything

  if (prev_mbr.min_x != misc->abe_share->viewer_displayed_area.min_x || prev_mbr.min_y != misc->abe_share->viewer_displayed_area.min_y ||
      prev_mbr.max_x != misc->abe_share->viewer_displayed_area.max_x || prev_mbr.max_y != misc->abe_share->viewer_displayed_area.max_y || 
      mbc.min_z != prev_min_z || mbc.max_z != prev_max_z)
    {
      map->setBounds (mbc);


      if (reset)
        {
          //  Get the map center so we can reset the view.

          map->getMapCenter (&misc->map_center_x, &misc->map_center_y, &misc->map_center_z);


          map->setMapCenter (misc->map_center_x, misc->map_center_y, misc->map_center_z);

          map->resetPOV ();
        }
    }


  //  If we want to display a GeoTIFF we must load the texture, unless it (or the displayed area) hasn't changed.

  if (misc->display_GeoTIFF && ((prev_mbr.min_x != misc->abe_share->viewer_displayed_area.min_x ||
                                 prev_mbr.min_y != misc->abe_share->viewer_displayed_area.min_y ||
                                 prev_mbr.max_x != misc->abe_share->viewer_displayed_area.max_x ||
                                 prev_mbr.max_y != misc->abe_share->viewer_displayed_area.max_y ||
                                 mbc.min_z != prev_min_z || mbc.max_z != prev_max_z ||
                                 misc->display_GeoTIFF != prev_display_GeoTIFF) || misc->GeoTIFF_init))
    {
      NV_F64_XYMBR geotiff_mbr;
      QImage *subImage = geotiff (misc->abe_share->viewer_displayed_area, misc->GeoTIFF_name, &geotiff_mbr);
      if (subImage != NULL)
        {
          map->setGeotiffTexture (subImage, geotiff_mbr, misc->display_GeoTIFF);
          geotiff_clear ();
          misc->GeoTIFF_init = NVFalse;
        }
      prev_display_GeoTIFF = misc->display_GeoTIFF;
    }
  prev_mbr = misc->abe_share->viewer_displayed_area;
  prev_min_z = mbc.min_z;
  prev_max_z = mbc.max_z;


  //  Clear ALL of the data layers prior to loading.

  for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++) map->clearDataLayer (pfm);


  //  Now we load each of the PFMs (setDataLayer) using the minimum bounding cube as the limits.

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
          //  Adjust bounds to nearest grid point (compute displayed_area).

          adjust_bounds (misc, pfm);


          //  Make sure that each PFM is actually in the desired area.  If it's not then either the width
          //  or the height will be 0 or negative.

          if (misc->displayed_area_height > 0 && misc->displayed_area_width > 0)
            {
              data = (NV_FLOAT32 **) malloc (sizeof (NV_FLOAT32 *) * misc->displayed_area_height);
              if (data == NULL) 
                {
                  perror ("Allocating data memory in paint_surface");
                  parent->clean_exit (-1);
                }


              BIN_RECORD *bin;

              bin = (BIN_RECORD *) malloc (sizeof (BIN_RECORD) * misc->displayed_area_width);
              if (bin == NULL)
                {
                  perror ("Allocating bin memory in paint_surface");
                  parent->clean_exit (-1);
                }


              for (NV_INT32 i = 0 ; i < misc->displayed_area_height ; i++)
                {
                  data[i] = (NV_FLOAT32 *) malloc (sizeof (NV_FLOAT32) * misc->displayed_area_width);
                  if (data[i] == NULL) 
                    {
                      perror ("Allocating data[i] memory in paint_surface");
                      parent->clean_exit (-1);
                    }


                  read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width, misc->displayed_area_row + i, misc->displayed_area_column, bin);

                  for (NV_INT32 j = 0 ; j < misc->displayed_area_width ; j++)
                    {
                      data[i][j] = -misc->abe_share->open_args[pfm].head.null_depth;

                      switch (misc->abe_share->layer_type)
                        {
                        case AVERAGE_FILTERED_DEPTH:
                        default:
                          if (bin[j].validity & (PFM_DATA | PFM_INTERPOLATED)) data[i][j] = -bin[j].avg_filtered_depth;
                          break;

                        case MIN_FILTERED_DEPTH:
                          if (bin[j].validity & PFM_DATA) data[i][j] = -bin[j].min_filtered_depth;
                          break;

                        case MAX_FILTERED_DEPTH:
                          if (bin[j].validity & PFM_DATA) data[i][j] = -bin[j].max_filtered_depth;
                          break;

                        case AVERAGE_DEPTH:
                          if (bin[j].num_soundings) data[i][j] = -bin[j].avg_depth;
                          break;

                        case MIN_DEPTH:
                          if (bin[j].num_soundings) data[i][j] = -bin[j].min_depth;
                          break;
                          
                        case MAX_DEPTH:
                          if (bin[j].num_soundings) data[i][j] = -bin[j].max_depth;
                          break;
                        }
                    }
                }


              free (bin);


              NV_INT32 layer_type = 0;
              switch (misc->abe_share->layer_type)
                {
                case AVERAGE_FILTERED_DEPTH:
                case AVERAGE_DEPTH:
                  layer_type = 0;
                  break;

                case MIN_FILTERED_DEPTH:
                case MIN_DEPTH:
                  layer_type = -1;
                  break;

                case MAX_FILTERED_DEPTH:
                case MAX_DEPTH:
                  layer_type = 1;
                  break;
                }

              map->setDataLayer (pfm, data, NULL, 0, 0, misc->displayed_area_height, misc->displayed_area_width, 
                                 misc->abe_share->open_args[pfm].head.y_bin_size_degrees, misc->abe_share->open_args[pfm].head.x_bin_size_degrees,
                                 -misc->abe_share->open_args[pfm].head.null_depth, misc->displayed_area, layer_type);

              for (NV_INT32 i = 0 ; i < misc->displayed_area_height ; i++) free (data[i]);

              free (data);
            }
        }
    }
}
