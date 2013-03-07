
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


//!  This function paints the selected PFM binned surface (actual painting occurs in hatchr.cpp).

void paint_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, NV_BOOL cov_area_defined)
{
  static NV_FLOAT32 prev_min = CHRTRNULL, prev_max = -CHRTRNULL;


  NV_FLOAT32 *next_attr = NULL;
  NV_U_CHAR *next_flags = NULL;

  
  NV_BOOL compute_layer_min_max (MISC *misc, OPTIONS *options);
  void hatchr (nvMap *map, OPTIONS *options, NV_BOOL clear, NV_F64_XYMBR mbr, NV_F64_XYMBR edit_mbr, NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range,
               NV_FLOAT64 x_bin_size, NV_FLOAT64 y_bin_size, NV_INT32 height, NV_INT32 start_x, NV_INT32 end_x, NV_FLOAT32 ss_null, NV_FLOAT64 cell_size_x,
               NV_FLOAT64 cell_size_y, NV_FLOAT32 *current_row, NV_FLOAT32 *next_row, NV_FLOAT32 *current_attr, NV_U_CHAR *current_flags, NV_U_BYTE alpha,
               NV_INT32 row, NV_INT32 attribute);
  void geotiff (nvMap *map, MISC *misc, OPTIONS *options);
  void overlayData (nvMap *map, MISC *misc, OPTIONS *options);


  if (!cov_area_defined) return;


  misc->total_displayed_area.min_y = mapdef->bounds[mapdef->zoom_level].min_y;
  misc->total_displayed_area.min_x = mapdef->bounds[mapdef->zoom_level].min_x;
  misc->total_displayed_area.max_y = mapdef->bounds[mapdef->zoom_level].max_y;
  misc->total_displayed_area.max_x = mapdef->bounds[mapdef->zoom_level].max_x;


  if (misc->total_displayed_area.min_x > misc->total_displayed_area.max_x)
    {
      //  Only adjust the lons if the displayed area does not cross the dateline.
            
      if ((misc->total_displayed_area.max_x > 0.0 && misc->total_displayed_area.min_x > 0.0) || 
	  (misc->total_displayed_area.max_x < 0.0 && misc->total_displayed_area.min_x < 0.0))
	{
	  NV_FLOAT64 temp = misc->total_displayed_area.min_x;
	  misc->total_displayed_area.min_x = misc->total_displayed_area.max_x;
	  misc->total_displayed_area.max_x = temp;
	}
    }


  //  Save the displayed area for other ABE programs.  Some key off of the editor/viewer area (displayed_area)
  //  and some key off of just the viewer area (viewer_displayed_area) so, at this point, we set them both.

  misc->abe_share->displayed_area = misc->abe_share->viewer_displayed_area = misc->total_displayed_area;


  //  Compute center point for sun angle arrow

  NV_FLOAT64 center_map_x = misc->total_displayed_area.min_x + (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / 2.0;
  NV_FLOAT64 center_map_y = misc->total_displayed_area.min_y + (misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / 2.0;
  NV_FLOAT64 dum_map_z;
  NV_INT32 dum_z;
  map->map_to_screen (1, &center_map_x, &center_map_y, &dum_map_z, &misc->center_point.x, &misc->center_point.y, &dum_z);


  //  Compute the min and max for the displayed area (using values from all PFM layers).

  if (!compute_layer_min_max (misc, options)) return;


  //  Check to see if we want to force clearing because the min and/or max value changed.

  if (misc->color_by_attribute)
    {
      if (prev_min != misc->displayed_area_attr_min || prev_max != misc->displayed_area_attr_max) misc->clear = NVTrue;
      prev_min = misc->displayed_area_attr_min;
      prev_max = misc->displayed_area_attr_max;
    }
  else
    {
      if (prev_min != misc->displayed_area_min || prev_max != misc->displayed_area_max) misc->clear = NVTrue;
      prev_min = misc->displayed_area_min;
      prev_max = misc->displayed_area_max;
    }


  //  Fill the background with the background color so we don't have to paint the background in hatchr.
  //  Also so we can display the GeoTIFF under the data if requested.

  //  Whole area

  if (misc->clear)
    {
      map->fillRectangle (misc->total_displayed_area.min_x, misc->total_displayed_area.min_y,
                          misc->total_displayed_area.max_x, misc->total_displayed_area.max_y,
                          options->background_color, NVFalse);
    }


  //  Partial area (edited area)

  else
    {
      map->fillRectangle (misc->abe_share->edit_area.min_x, misc->abe_share->edit_area.min_y,
                          misc->abe_share->edit_area.max_x, misc->abe_share->edit_area.max_y,
                          options->background_color, NVFalse);
    }


  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
          //  Adjust bounds to nearest grid point

          adjust_bounds (misc, pfm);


          //  Redraw everything

          if (misc->clear)
            {
              misc->hatchr_start_x = 0;
              misc->hatchr_end_x = misc->displayed_area_width[pfm];
              misc->hatchr_start_y = 0;
              misc->hatchr_end_y = misc->displayed_area_height[pfm];
            }


          //  Redraw a portion (what we edited)

          else
            {
              NV_F64_XYMBR tmp_mbr = misc->abe_share->edit_area;


              //  Adjust edit bounds to nearest grid point (with a border of 1 grid point to clean up the edges).

              tmp_mbr.min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
                (NINT ((misc->abe_share->edit_area.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
                       misc->abe_share->open_args[pfm].head.y_bin_size_degrees) - 1) *
                misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
              tmp_mbr.max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
                (NINT ((misc->abe_share->edit_area.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
                       misc->abe_share->open_args[pfm].head.y_bin_size_degrees) + 1) * 
                misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
              tmp_mbr.min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
                (NINT ((misc->abe_share->edit_area.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
                       misc->abe_share->open_args[pfm].head.x_bin_size_degrees) - 1) *
                misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
              tmp_mbr.max_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
                (NINT ((misc->abe_share->edit_area.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
                       misc->abe_share->open_args[pfm].head.x_bin_size_degrees) + 1) *
                misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


              tmp_mbr.min_y = qMax (tmp_mbr.min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
              tmp_mbr.max_y = qMin (tmp_mbr.max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
              tmp_mbr.min_x = qMax (tmp_mbr.min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
              tmp_mbr.max_x = qMin (tmp_mbr.max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);



              misc->hatchr_start_x = NINT ((tmp_mbr.min_x - misc->displayed_area[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
              misc->hatchr_end_x = misc->hatchr_start_x + (NINT ((tmp_mbr.max_x - tmp_mbr.min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees));

              misc->hatchr_start_y = NINT ((tmp_mbr.min_y - misc->displayed_area[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
              misc->hatchr_end_y = misc->hatchr_start_y + (NINT ((tmp_mbr.max_y - tmp_mbr.min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees));
            }


          //  If the width or height is 0 we have asked for an area outside of the PFM's MBR so we don't want to do anything.

          if (misc->displayed_area_width[pfm] > 0 && misc->displayed_area_height[pfm] > 0)
            {
              //  If misc->display_GeoTIFF is -1 we want to draw the GeoTIFF first.

              if (pfm == misc->abe_share->pfm_count - 1 && misc->display_GeoTIFF < 0) geotiff (map, misc, options);


              //  The only time we don't want to go through here is when we have an open GeoTIFF file, we are displaying
              //  the GeoTIFF surface, and the GeoTIFF alpha value (transparency) is 255.

              if (misc->display_GeoTIFF <= 0 || options->GeoTIFF_alpha < 255)
                {
                  //  We only want to do attributes if we're coloring by non-PFM attributes (like number or stddev), or if we're on the top level.

                  NV_INT32 attribute = 0;
                  if ((misc->color_by_attribute > 0 && misc->color_by_attribute < PRE_ATTR) || (!pfm && misc->color_by_attribute))
                    attribute = misc->color_by_attribute;


                  //  Allocate the needed arrays.

                  NV_INT32 size;
                  NV_FLOAT32 *ar = NULL;

                  if (options->contour)
                    {
                      size = (misc->displayed_area_width[pfm] + 1) * (misc->displayed_area_height[pfm] + 1);

                      ar = (NV_FLOAT32 *) calloc (size, sizeof (NV_FLOAT32));

                      if (ar == NULL)
                        {
                          perror (pfmView::tr ("Allocating ar array in paint_surface").toAscii ());
                          exit (-1);
                        }
                    }


                  BIN_RECORD *current_record = (BIN_RECORD *) calloc (misc->displayed_area_width[pfm], sizeof (BIN_RECORD));
                  if (current_record == NULL)
                    {
                      perror (pfmView::tr ("Allocating current_record in paint_surface").toAscii ());
                      exit (-1);
                    }

                  misc->next_row = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
                  if (misc->next_row == NULL)
                    {
                      perror (pfmView::tr ("Allocating next_row in paint_surface").toAscii ());
                      exit (-1);
                    }

                  if (attribute)
                    {
                      next_attr = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
                      if (next_attr == NULL)
                        {
                          perror (pfmView::tr ("Allocating next_attr in paint_surface").toAscii ());
                          exit (-1);
                        }
                    }

                  next_flags = (NV_U_CHAR *) calloc (misc->displayed_area_width[pfm], sizeof (NV_CHAR));
                  if (next_flags == NULL)
                    {
                      perror (pfmView::tr ("Allocating next_flags in paint_surface").toAscii ());
                      exit (-1);
                    }

                  misc->current_row = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
                  if (misc->current_row == NULL)
                    {
                      perror (pfmView::tr ("Allocating current_row in paint_surface").toAscii ());
                      exit (-1);
                    }

                  if (attribute)
                    {
                      misc->current_attr = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
                      if (misc->current_attr == NULL)
                        {
                          perror (pfmView::tr ("Allocating current_attr in paint_surface").toAscii ());
                          exit (-1);
                        }
                    }


                  misc->current_flags = (NV_U_CHAR *) calloc (misc->displayed_area_width[pfm], sizeof (NV_CHAR));
                  if (misc->current_flags == NULL)
                    {
                      perror (pfmView::tr ("Allocating current_flags in paint_surface").toAscii ());
                      exit (-1);
                    }


                  //  Read input data row by row.  Note that hatchr_start_y and hatchr_end_y may not be the same as the
                  //  entire displayed area since we may only be redrawing a small edited portion of the display.

                  for (NV_INT32 jj = misc->hatchr_start_y ; jj <= misc->hatchr_end_y ; jj++)
                    {
                      //  Read data from disk.

                      if (jj == misc->hatchr_start_y)
                        {
                          read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width[pfm], misc->displayed_area_row[pfm] + jj, 
                                        misc->displayed_area_column[pfm], current_record);

                          loadArrays (misc->abe_share->layer_type, misc->displayed_area_width[pfm], current_record, misc->current_row, 
                                      misc->current_attr, attribute, misc->current_flags, options->highlight, options->h_count, misc->pfm_handle[pfm],
                                      misc->abe_share->open_args[pfm], options->highlight_percent, misc->surface_val);

                          memcpy (misc->next_row, misc->current_row, misc->displayed_area_width[pfm] * sizeof (NV_FLOAT32));

                          if (attribute) memcpy (next_attr, misc->current_attr, misc->displayed_area_width[pfm] * sizeof (NV_FLOAT32));

                          memcpy (next_flags, misc->current_flags, misc->displayed_area_width[pfm] * sizeof (NV_CHAR));
                        }
                      else
                        {
                          memcpy (misc->current_row, misc->next_row, misc->displayed_area_width[pfm] * sizeof (NV_FLOAT32));

                          if (attribute) memcpy (misc->current_attr, next_attr, misc->displayed_area_width[pfm] * sizeof (NV_FLOAT32));

                          memcpy (misc->current_flags, next_flags, misc->displayed_area_width[pfm] * sizeof (NV_CHAR));


                          //  If not at top edge, read another row.

                          if (jj < misc->abe_share->open_args[pfm].head.bin_height) 
                            {
                              read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width[pfm], misc->displayed_area_row[pfm] + jj, 
                                            misc->displayed_area_column[pfm], current_record);

                              loadArrays (misc->abe_share->layer_type, misc->displayed_area_width[pfm], current_record, misc->next_row,
                                          next_attr, attribute, next_flags, options->highlight, options->h_count, misc->pfm_handle[pfm],
                                          misc->abe_share->open_args[pfm], options->highlight_percent, misc->surface_val);
                            }
                        }


                      //  HSV fill and sunshade.

                      hatchr (map, options, misc->clear, misc->displayed_area[pfm], misc->abe_share->edit_area, misc->color_min, misc->color_max,
                              misc->color_range, misc->abe_share->open_args[pfm].head.x_bin_size_degrees, misc->abe_share->open_args[pfm].head.y_bin_size_degrees,
                              misc->abe_share->open_args[pfm].head.bin_height, misc->hatchr_start_x, misc->hatchr_end_x,
                              misc->abe_share->open_args[pfm].head.null_depth, misc->ss_cell_size_x[pfm], misc->ss_cell_size_y[pfm], misc->current_row,
                              misc->next_row, misc->current_attr, misc->current_flags, misc->pfm_alpha[pfm], jj, attribute);


                      if (options->contour)
                        {
                          for (NV_INT32 i = 0 ; i < misc->displayed_area_width[pfm] ; i++)
                            {
                              //  We use next_row for sunshading on all but the first row to send to scribe.

                              if (i)
                                {
                                  ar[(jj - misc->hatchr_start_y) * misc->displayed_area_width[pfm] + i] = misc->next_row[i] * 
                                    options->z_factor + options->z_offset;
                                }
                              else
                                {
                                  ar[(jj - misc->hatchr_start_y) * misc->displayed_area_width[pfm] + i] = misc->current_row[i] * 
                                    options->z_factor + options->z_offset;
                                }
                            }
                        }

                      if (qApp->hasPendingEvents ())
                        {
                          qApp->processEvents();
                          if (misc->drawing_canceled) break;
                        }
                    }


                  //  Flush all of the saved fillRectangles from hatchr to screen

                  map->flush ();


                  //  Free allocated memory.

                  free (current_record);
                  free (misc->next_row);
                  if (attribute) free (next_attr);
                  free (next_flags);
                  free (misc->current_row);
                  if (attribute) free (misc->current_attr);
                  free (misc->current_flags);


                  if (!misc->drawing_canceled)
                    {
                      //  If you asked for contours, do it.

                      if (options->contour)
                        {
                          if (misc->function == DRAW_CONTOUR)
                            {
                              scribe (map, options, misc, pfm, ar, misc->draw_contour_level);
                            }
                          else
                            {
                              scribe (map, options, misc, pfm, ar, 99999.0);
                            }
                        }
                    }


                  //  Free allocated scribe memory.

                  if (options->contour) free (ar);
                }


              //  Issue a warning when the user is displaying more bins than pixels

              if (!misc->tposiafps && (misc->clear && (misc->displayed_area_width[pfm] > mapdef->draw_width ||
                                                       misc->displayed_area_height[pfm] > mapdef->draw_height)))
                {
                  QString warning_message = pfmView::tr ("Number of bins displayed exceeds number of pixels.\n");
                  warning_message += pfmView::tr ("Some data masking will occur.\n");
                  warning_message += pfmView::tr ("Increase window size or decrease area size.\n\n");
                  warning_message +=
                    QString (pfmView::tr ("Bin width  = %1  ,  Pixel width  = %2\n")).arg (misc->displayed_area_width[pfm]).arg (mapdef->draw_width);
                  warning_message +=
                    QString (pfmView::tr ("Bin height = %1  ,  Pixel height = %2\n")).arg (misc->displayed_area_height[pfm]).arg (mapdef->draw_height);

                  QMessageBox msgBox (QMessageBox::Warning, pfmView::tr ("pfmView TPOSIAFPS"), warning_message);

                  msgBox.setStandardButtons (QMessageBox::Close);
                  msgBox.setDefaultButton (QMessageBox::Close);

                  QCheckBox dontShow (pfmView::tr ("I understand that I can't see everything due to resolution but don't show this again!"), &msgBox);
                  msgBox.addButton (&dontShow, QMessageBox::HelpRole);
                  msgBox.exec ();
                  misc->tposiafps = ((bool) dontShow.checkState ());
                }
            }
        }
    }


  if (!misc->drawing_canceled)
    {
      //  Display GeoTIFF data if available.

      if (misc->display_GeoTIFF == 1) geotiff (map, misc, options);


      //  Overlay any files that have been requested.

      overlayData (map, misc, options);


      //  Display suspect, feature, selected, or reference info if enabled.

      if (options->display_suspect || options->display_feature || options->display_selected || options->display_reference)
        overlayFlag (map, options, misc, NVTrue, NVTrue, NVTrue);


      //  Display filter masks if any are defined.

      for (NV_INT32 i = 0 ; i < misc->poly_filter_mask_count ; i++)
        {
          if (!misc->poly_filter_mask[i].displayed)
            {
              //  Check to see if this mask (that we are going to redraw) overlaps another (that we weren't planning on redrawing).
              //  If so, set the flag to redraw the other one.

              for (NV_INT32 j = 0 ; j < misc->poly_filter_mask_count ; j++)
                {
                  if (j != i && misc->poly_filter_mask[j].displayed)
                    {
                      if (polygon_collision2 (misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y, misc->poly_filter_mask[i].count,
                                              misc->poly_filter_mask[j].x, misc->poly_filter_mask[j].y, misc->poly_filter_mask[j].count))
                        misc->poly_filter_mask[j].displayed = NVFalse;
                    }
                }
            }
        }


      for (NV_INT32 i = 0 ; i < misc->poly_filter_mask_count ; i++)
        {
          if (!misc->poly_filter_mask[i].displayed)
            {
              if (options->poly_filter_mask_color.alpha () < 255)
                {
                  map->fillPolygon (misc->poly_filter_mask[i].count, misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y, 
                                    options->poly_filter_mask_color, NVTrue);
                }
              else
                {
                  //  We don't have to worry about clipping this because moving the area discards the mask areas.

                  map->drawPolygon (misc->poly_filter_mask[i].count, misc->poly_filter_mask[i].x, misc->poly_filter_mask[i].y,
                                    options->poly_filter_mask_color, 2, NVTrue, Qt::SolidLine, NVTrue);
                }
              misc->poly_filter_mask[i].displayed = NVTrue;
            }
        }


      if (options->landmask && misc->total_displayed_area.max_x - misc->total_displayed_area.min_x < 5.0 &&
          misc->total_displayed_area.max_y - misc->total_displayed_area.min_y < 5.0)
        {
          map->setLandmask (NVTrue);
        }
      else
        {
          map->setLandmask (NVFalse);
        }


      //  Flag minimum, maximum, and maximum std dev if enabled.

      if (options->display_minmax) displayMinMax (map, options, misc);
    }
}
