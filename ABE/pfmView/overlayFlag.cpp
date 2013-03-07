
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "pfmView.hpp"


//!  This function draws the overlay flags (suspect, reference, selected, features).

void overlayFlag (nvMap *map, OPTIONS *options, MISC *misc, NV_BOOL suspect, NV_BOOL selected, NV_BOOL reference)
{
  NV_INT32            i, j, x, y, z, pixels = 0;
  NV_FLOAT64          wlon, elon, slat, nlat, dep, nx[2], ny[2];
  NV_I32_COORD2       coord;
  NV_U_INT32          val;
  QColor              tmp;


  if (!misc->clear)
    {
      slat = misc->abe_share->edit_area.min_y;
      nlat = misc->abe_share->edit_area.max_y;
      wlon = misc->abe_share->edit_area.min_x;
      elon = misc->abe_share->edit_area.max_x;
    }
  else
    {
      slat = misc->total_displayed_area.min_y;
      nlat = misc->total_displayed_area.max_y;
      wlon = misc->total_displayed_area.min_x;
      elon = misc->total_displayed_area.max_x;
    }


  //  Display suspect, selected, or reference.

  if (options->display_suspect || options->display_selected || options->display_reference)
    {
      for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
        {
          for (i = 0 ; i < misc->displayed_area_height[pfm] ; i++)
            {
              coord.y = misc->displayed_area_row[pfm] + i;
              NV_FLOAT64 lat = misc->displayed_area[pfm].min_y + ((NV_FLOAT64) i + 0.5) * misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

              for (j = 0 ; j < misc->displayed_area_width[pfm] ; j++)
                {
                  coord.x = misc->displayed_area_column[pfm] + j;
                  NV_FLOAT64 lon = misc->displayed_area[pfm].min_x + ((NV_FLOAT64) j + 0.5) * misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

                  read_bin_record_validity_index (misc->pfm_handle[pfm], coord, &val);

                  if (lat >= slat && lat <= nlat && lon >= wlon && lon <= elon)
                    {
                      if (options->display_suspect && suspect && (val & PFM_SUSPECT))
                        {
                          map->map_to_screen (1, &lon, &lat, &dep, &x, &y, &z);
                          map->drawLine (x - 6, y - 6, x + 6, y + 6, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                          map->drawLine (x - 6, y + 6, x + 6, y - 6, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                        }

                      if (options->display_selected && selected && (val & PFM_SELECTED_SOUNDING))
                        {
                          map->map_to_screen (1, &lon, &lat, &dep, &x, &y, &z);
                          map->drawLine (x - 6, y, x + 6, y, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                          map->drawLine (x, y - 6, x, y + 6, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                        }

                      if (options->display_reference && reference && (val & PFM_REFERENCE))
                        {
                          map->map_to_screen (1, &lon, &lat, &dep, &x, &y, &z);
                          map->drawLine (x - 3, y - 3, x + 3, y + 3, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                          map->drawLine (x - 3, y + 3, x + 3, y - 3, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                          map->drawLine (x - 6, y, x + 6, y, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                          map->drawLine (x, y - 6, x, y + 6, options->feature_color, LINE_WIDTH, NVFalse, Qt::SolidLine);
                        }
                    }
                }

              if (qApp->hasPendingEvents ())
                {
                  qApp->processEvents();
                  if (misc->drawing_canceled) break;
                }
            }
        }
    }


  //  Display features.

  if (options->display_feature)
    {
      if (misc->def_feature_poly)
        {
          NV_FLOAT64 dep = 0.0;
          NV_INT32 pix_x[2], pix_y[2], pix_z;
          map->map_to_screen (1, &misc->feature[0].longitude, &misc->feature[0].latitude, &dep, &pix_x[0], &pix_y[0], &pix_z);
          newgp (misc->feature[0].latitude, misc->feature[0].longitude, 0.0, misc->feature_poly_radius, &ny[0], &nx[0]);
          map->map_to_screen (1, &nx[0], &ny[0], &dep, &pix_x[1], &pix_y[1], &pix_z);

          pixels = pix_y[1] - pix_y[0] + 1;

          tmp = options->feature_poly_color;
        }
      else
        {
          pixels = 12;
          tmp = options->feature_color;
          if (misc->abe_share->layer_type == NUM_LAYERS) tmp = Qt::black;
        }


      for (i = 0 ; i < (NV_INT32) misc->bfd_header.number_of_records ; i++)
        {
          if (misc->feature[i].confidence_level || misc->abe_share->layer_type == AVERAGE_DEPTH || 
              misc->abe_share->layer_type == MIN_DEPTH || misc->abe_share->layer_type == MAX_DEPTH)
            {
              if (misc->feature[i].longitude >= wlon && misc->feature[i].longitude <= elon &&
                  misc->feature[i].latitude >= slat && misc->feature[i].latitude <= nlat)
                {
                  //  Check for the type of feature display

                  if (options->display_feature == 1 || (options->display_feature == 2 && misc->feature[i].confidence_level != 5) ||
                      (options->display_feature == 3 && misc->feature[i].confidence_level == 5))
                    {
                      //  Check the feature for the feature search string and highlighting.

                      QString feature_info;
                      NV_BOOL highlight;
                      if (checkFeature (misc, options, i, &highlight, &feature_info))
                        {
                          //  Check for defining a feature polygon (actually drawing one)

                          if (misc->def_feature_poly)
                            {
                              map->map_to_screen (1, &misc->feature[i].longitude, &misc->feature[i].latitude, &dep, &x, &y, &z);


                              //  Feature polygons.

                              if (misc->feature[i].poly_count)
                                {
                                  BFDATA_POLYGON bfd_polygon;

                                  binaryFeatureData_read_polygon (misc->bfd_handle, misc->feature[i].record_number, &bfd_polygon);


                                  //  Draw a line from the center of the associated feature to the first polygon
                                  //  point that is in the displayed area so we'll know which feature goes with
                                  //  which polygon.

                                  for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                    {
                                      if (bfd_polygon.longitude[j] >= misc->total_displayed_area.min_x &&
                                          bfd_polygon.longitude[j] <= misc->total_displayed_area.max_x &&
                                          bfd_polygon.latitude[j] >= misc->total_displayed_area.min_y &&
                                          bfd_polygon.latitude[j] <= misc->total_displayed_area.max_y)
                                        {
                                          map->drawLine (misc->feature[i].longitude, misc->feature[i].latitude, bfd_polygon.longitude[j],
                                                         bfd_polygon.latitude[j], options->feature_color, 2, NVFalse,
                                                         Qt::DotLine);
                                          break;
                                        }
                                    }


                                  if (options->feature_poly_color.alpha () < 255)
                                    {
                                      map->fillPolygon (misc->feature[i].poly_count, bfd_polygon.longitude, bfd_polygon.latitude, tmp, NVFalse);
                                    }
                                  else
                                    {
                                      //  If we're drawing a polygon, clip the lines.

                                      for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                        {
                                          NV_INT32 k = (j + 1) % misc->feature[i].poly_count;


                                          //  Don't clip the original points, clip a copy.

                                          nx[0] = bfd_polygon.longitude[j];
                                          ny[0] = bfd_polygon.latitude[j];
                                          nx[1] = bfd_polygon.longitude[k];
                                          ny[1] = bfd_polygon.latitude[k];


                                          if (clip (&nx[0], &ny[0], &nx[1], &ny[1], misc->total_displayed_area))
                                            {
                                              map->drawLine (nx[0], ny[0], nx[1], ny[1], options->feature_poly_color, 2, NVFalse, Qt::SolidLine);
                                            }
                                        }
                                    }
                                }


                              if (options->feature_poly_color.alpha () < 255)
                                {
                                  map->fillCircle (x, y, pixels, 0.0, 360.0, tmp, NVFalse);
                                }
                              else
                                {
                                  map->drawCircle (x, y, pixels, 0.0, 360.0, LINE_WIDTH, tmp, Qt::SolidLine, NVFalse);
                                }
                            }
                          else
                            {
                              QColor featureColor = options->feature_color;
                              if (misc->feature[i].confidence_level == 5) featureColor = options->verified_feature_color;


                              //  Check to see if this is a parent record or child record (only plot children if requested).

                              if (options->display_children || !misc->feature[i].parent_record)
                                {
                                  map->map_to_screen (1, &misc->feature[i].longitude, &misc->feature[i].latitude, &dep, &x, &y, &z);


                                  //  Go through here if we want to display feature info, poly, or we want to
                                  //  highlight using a feature text search.

                                  if (options->display_feature_info || options->display_feature_poly || highlight)
                                    {
                                      tmp = options->feature_info_color;
                                      if (options->display_feature_info && !feature_info.isEmpty ()) map->drawText (feature_info, x + 6, y + 3, tmp, NVFalse);

                                      if (highlight) featureColor = options->contour_highlight_color;


                                      //  Feature polygons.

                                      if (options->display_feature_poly && misc->feature[i].poly_count)
                                        {
                                          BFDATA_POLYGON bfd_polygon;

                                          binaryFeatureData_read_polygon (misc->bfd_handle, misc->feature[i].record_number, &bfd_polygon);


                                          //  Draw a line from the center of the associated feature to the first polygon
                                          //  point that is in the displayed area so we'll know which feature goes with
                                          //  which polygon.

                                          for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                            {
                                              if (bfd_polygon.longitude[j] >= misc->total_displayed_area.min_x &&
                                                  bfd_polygon.longitude[j] <= misc->total_displayed_area.max_x &&
                                                  bfd_polygon.latitude[j] >= misc->total_displayed_area.min_y &&
                                                  bfd_polygon.latitude[j] <= misc->total_displayed_area.max_y)
                                                {
                                                  map->drawLine (misc->feature[i].longitude, misc->feature[i].latitude, bfd_polygon.longitude[j],
                                                                 bfd_polygon.latitude[j], featureColor, 2, NVFalse,
                                                                 Qt::DotLine);
                                                  break;
                                                }
                                            }


                                          if (options->feature_poly_color.alpha () < 255 && misc->feature[i].poly_type)
                                            {
                                              map->fillPolygon (misc->feature[i].poly_count, bfd_polygon.longitude, bfd_polygon.latitude,
                                                                options->feature_poly_color, NVFalse);
                                            }
                                          else
                                            {
                                              //  If we're drawing a polygon, clip the lines.

                                              for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                                {
                                                  //  Don't close polylines (misc->feature[i].poly_type 0), only polygons.

                                                  if (!misc->feature[i].poly_type && j == (misc->feature[i].poly_count - 1)) break;

                                                  NV_INT32 k = (j + 1) % misc->feature[i].poly_count;


                                                  //  Don't clip the original points, clip a copy.

                                                  nx[0] = bfd_polygon.longitude[j];
                                                  ny[0] = bfd_polygon.latitude[j];
                                                  nx[1] = bfd_polygon.longitude[k];
                                                  ny[1] = bfd_polygon.latitude[k];


                                                  if (clip (&nx[0], &ny[0], &nx[1], &ny[1], misc->total_displayed_area))
                                                    {
                                                      map->drawLine (nx[0], ny[0], nx[1], ny[1],
                                                                     options->feature_poly_color, 2, NVFalse, Qt::SolidLine);
                                                    }
                                                }
                                            }
                                        }
                                    }


                                  //  Check to see if this is a child record.

                                  if (misc->feature[i].parent_record)
                                    {
                                      map->drawCircle (x, y, 8, 0.0, 360.0, LINE_WIDTH, featureColor, Qt::SolidLine, NVFalse);
                                    }
                                  else
                                    {
                                      map->drawCircle (x, y, 12, 0.0, 360.0, LINE_WIDTH, featureColor, Qt::SolidLine, NVFalse);
                                    }
                                }
                            }
                        }
                    }
                }
            }


          //  Because this can take a while we're forcing it to update every 200 features.

          if (!(i % 200))
            {
              qApp->processEvents();


              if (qApp->hasPendingEvents ())
                {
                  qApp->processEvents();
                  if (misc->drawing_canceled) break;
                }
            }
        }


      if (misc->def_feature_poly)
        {
          //  If we're defining a feature polygon and the polygon count is set, display the proposed feature polygon.

          if (misc->poly_count)
            {
              for (NV_INT32 j = 1 ; j < misc->poly_count ; j++)
                {
                  map->drawLine (misc->polygon_x[j - 1], misc->polygon_y[j - 1], misc->polygon_x[j], misc->polygon_y[j],
                                 options->contour_highlight_color, 2, NVFalse, Qt::SolidLine);
                }
              map->drawLine (misc->polygon_x[misc->poly_count - 1], misc->polygon_y[misc->poly_count - 1], misc->polygon_x[0],
                             misc->polygon_y[0], options->contour_highlight_color, 2, NVFalse, Qt::SolidLine);
            }
        }
    }
  map->flush ();
}
