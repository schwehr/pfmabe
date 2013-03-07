
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



#include "pfmEdit.hpp"

/***************************************************************************/
/*!

  - Module Name:        overlay_flag

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Draw flag locations in the GC and the pixmap.

  - Arguments:

  - Return Value:       void

****************************************************************************/

void overlayFlag (nvMap *map, OPTIONS *options, MISC *misc, NV_INT32 type)
{
  NV_INT32        x, y, z;
  NV_FLOAT64      dep, nx[2], ny[2];
  QString         feature_search_string = QString (misc->abe_share->feature_search_string);


  NV_BOOL checkFeature (nvMap *map, MISC *misc, QString feature_search_string, NV_INT32 ftr, NV_BOOL *highlight, QString *feature_info);



  if (type == PFM_SELECTED_FEATURE)
    {
      for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
        {
          if (options->display_man_invalid || options->display_flt_invalid || misc->feature[i].confidence_level)
            {
              if (!check_bounds (map, options, misc, misc->feature[i].longitude, misc->feature[i].latitude, 
                                 misc->feature[i].depth, PFM_USER, NVFalse, 0, NVFalse, misc->slice))
                {
                  //  Check for the type of feature display

                  if (options->display_feature == 1 || (options->display_feature == 2 && misc->feature[i].confidence_level != 5) ||
                      (options->display_feature == 3 && misc->feature[i].confidence_level == 5))
                    {
                      //  Check the feature for the feature search string and highlighting.

                      QString feature_info;
                      NV_BOOL highlight;
                      if (checkFeature (map, misc, feature_search_string, i, &highlight, &feature_info))
                        {
                          //  Check to see if this is a child record.

                          if (options->display_children || !misc->feature[i].parent_record)
                            {
                              dep = misc->feature[i].depth;
                              if (!misc->view)
                                {
                                  map->map_to_screen (1, &misc->feature[i].longitude, &misc->feature[i].latitude, &dep, &x, &y, &z);
                                }
                              else
                                {
                                  scale_view (map, misc, misc->feature[i].longitude, misc->feature[i].latitude, dep, &x, &y, &z);
                                }


                              misc->visible_feature_count++;


                              //  Go through here if we want to display feature info or polygon.

                              if (options->display_feature_info || options->display_feature_poly)
                                {
                                  if (options->display_feature_info && !feature_info.isEmpty ())
                                    map->drawText (feature_info, x + 6, y + 3, options->feature_info_color, NVFalse);


                                  //  Feature polygons (only in plan view).

                                  if (!misc->view && options->display_feature_poly && misc->feature[i].poly_count)
                                    {
                                      BFDATA_POLYGON bfd_polygon;

                                      binaryFeatureData_read_polygon (misc->bfd_handle, misc->feature[i].record_number, &bfd_polygon);


                                      //  Draw a line from the center of the associated feature to the first polygon
                                      //  point that is in the displayed area so we'll know which feature goes with
                                      //  which polygon.

                                      for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                        {
                                          if (bfd_polygon.longitude[j] >= misc->displayed_area.min_x &&
                                              bfd_polygon.longitude[j] <= misc->displayed_area.max_x &&
                                              bfd_polygon.latitude[j] >= misc->displayed_area.min_y &&
                                              bfd_polygon.latitude[j] <= misc->displayed_area.max_y)
                                            {
                                              map->drawLine (misc->feature[i].longitude, misc->feature[i].latitude, bfd_polygon.longitude[j],
                                                             bfd_polygon.latitude[j], options->feature_color, 2, NVFalse,
                                                             Qt::DotLine);
                                              break;
                                            }
                                        }


                                      //  Mark each polygon vertex.

                                      for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                        {
                                          if (bfd_polygon.longitude[j] >= misc->displayed_area.min_x &&
                                              bfd_polygon.longitude[j] <= misc->displayed_area.max_x &&
                                              bfd_polygon.latitude[j] >= misc->displayed_area.min_y &&
                                              bfd_polygon.latitude[j] <= misc->displayed_area.max_y)
                                            {
                                              NV_FLOAT64 pos_z = 0.0;
                                              NV_INT32 map_x, map_y, map_z;
                                              map->map_to_screen (1, &bfd_polygon.longitude[j], &bfd_polygon.latitude[j], &pos_z, &map_x, &map_y, &map_z);
                                              drawx (map, map_x, map_y, options->feature_color, 99999, NVFalse);
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


                                              if (clip (&nx[0], &ny[0], &nx[1], &ny[1], misc->displayed_area))
                                                {
                                                  map->drawLine (nx[0], ny[0], nx[1], ny[1], options->feature_poly_color, 2, NVFalse,
                                                                 Qt::SolidLine);
                                                }
                                            }
                                        }
                                    }
                                }


                              QColor featureColor;
                              if (highlight)
                                {
                                  featureColor = options->feature_highlight_color;
                                }
                              else
                                {
                                  featureColor = options->feature_color;
                                  if (misc->feature[i].confidence_level == 5) featureColor = options->verified_feature_color;
                                }

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
    }
  else if (type == FILTER_MASK)
    {
      //  Don't display these if we're in the middle of a filtering operation.

      if (!misc->filter_kill_count)
        {
          QColor mask = options->contour_color;
          mask.setAlpha (128);

          for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
            {
              //  If we are displaying and editing only a single line, only get those points that 
              //  are in that line.

              if (!check_bounds (map, options, misc, i, NVTrue, misc->slice) && (!misc->num_lines || check_line (misc, misc->data[i].line)))
                {
                  if (misc->data[i].fmask)
                    {
                      dep = misc->data[i].z;
                      if (!misc->view)
                        {
                          map->map_to_screen (1, &misc->data[i].x, &misc->data[i].y, &dep, &x, &y, &z);
                        }
                      else
                        {
                          scale_view (map, misc, misc->data[i].x, misc->data[i].y, dep, &x, &y, &z);
                        }

                      drawx (map, x, y, mask, FILTER_MASK, NVFalse);
                    }
                }
            }
        }
    }

  map->flush ();
}
