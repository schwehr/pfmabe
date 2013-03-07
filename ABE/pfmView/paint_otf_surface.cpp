
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


//!  This function paints the surface when using on-the-fly (OTF) binning (actual painting occurs in hatchr.cpp).

void paint_otf_surface (MISC *misc, OPTIONS *options, nvMap *map, NVMAP_DEF *mapdef, NV_BOOL cov_area_defined)
{
  NV_FLOAT64  x_bin_size_degrees;
  NV_FLOAT64  y_bin_size_degrees;
  NV_INT32 ndx = 0;
  static NV_INT32 width = 0, height = 0, size = 0;


  void hatchr (nvMap *map, OPTIONS *options, NV_BOOL clear, NV_F64_XYMBR mbr, NV_F64_XYMBR edit_mbr, NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range,
               NV_FLOAT64 x_bin_size, NV_FLOAT64 y_bin_size, NV_INT32 height, NV_INT32 start_x, NV_INT32 end_x, NV_FLOAT32 ss_null, NV_FLOAT64 cell_size_x,
               NV_FLOAT64 cell_size_y, NV_FLOAT32 *current_row, NV_FLOAT32 *next_row, NV_FLOAT32 *current_attr, NV_U_CHAR *current_flags, NV_U_BYTE alpha,
               NV_INT32 row, NV_INT32 attribute);
  void geotiff (nvMap *map, MISC *misc, OPTIONS *options);
  void overlayData (nvMap *map, MISC *misc, OPTIONS *options);
  void setScale (NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range, NV_INT32 flip, MISC *misc, OPTIONS *options, NV_BOOL min_lock, NV_BOOL max_lock);



  if (!cov_area_defined) return;


  misc->total_displayed_area.min_y = mapdef->bounds[mapdef->zoom_level].min_y;
  misc->total_displayed_area.min_x = mapdef->bounds[mapdef->zoom_level].min_x;
  misc->total_displayed_area.max_y = mapdef->bounds[mapdef->zoom_level].max_y;
  misc->total_displayed_area.max_x = mapdef->bounds[mapdef->zoom_level].max_x;


  if (misc->total_displayed_area.min_x > misc->total_displayed_area.max_x)
    {
      //  Only adjust the lons if the displayed area does not cross the
      //  dateline.
            
      if ((misc->total_displayed_area.max_x > 0.0 && misc->total_displayed_area.min_x > 0.0) || 
	  (misc->total_displayed_area.max_x < 0.0 && misc->total_displayed_area.min_x < 0.0))
	{
	  NV_FLOAT64 temp = misc->total_displayed_area.min_x;
	  misc->total_displayed_area.min_x = misc->total_displayed_area.max_x;
	  misc->total_displayed_area.max_x = temp;
	}
    }


  //  Save the displayed area for other ABE programs.  Some key off of the editor/viewer area (displayed_area)
  //  and some key off of just the viewer area (viewer_displayed_area) so, at this point, we set them
  //  both.

  misc->abe_share->displayed_area = misc->abe_share->viewer_displayed_area = misc->total_displayed_area;


  //  Compute center point for sun angle arrow and bin size computation

  NV_FLOAT64 center_map_x = misc->total_displayed_area.min_x + (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / 2.0;
  NV_FLOAT64 center_map_y = misc->total_displayed_area.min_y + (misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / 2.0;
  NV_FLOAT64 dum_map_z;
  NV_INT32 dum_z;
  map->map_to_screen (1, &center_map_x, &center_map_y, &dum_map_z, &misc->center_point.x, &misc->center_point.y, &dum_z);


  //  Compute the bin size in degrees at the center of the displayed area.

  NV_FLOAT64 new_x, new_y;
  NV_F64_XYMBR mbr;
  newgp (center_map_y, center_map_x, 0.0, options->otf_bin_size_meters, &new_y, &new_x);
  y_bin_size_degrees = misc->abe_share->otf_y_bin_size = new_y - center_map_y;
  newgp (center_map_y, center_map_x, 90.0, options->otf_bin_size_meters, &new_y, &new_x);
  x_bin_size_degrees = misc->abe_share->otf_x_bin_size = new_x - center_map_x;


  //  Fill the background with the background color so we don't have to paint the background in hatchr.
  //  Also so we can display the GeoTIFF under the data if requested.

  //  Whole area

  if (misc->clear)
    {
      //  If we asked to clear then we've probably changed the area so we want to free and allocate the grid.

      if (misc->otf_grid != NULL)
        {
          misc->otfShare->detach ();
          misc->otf_grid = NULL;
          misc->abe_share->otf_width = misc->abe_share->otf_height = 0;
        }

      mbr = misc->total_displayed_area;
    }


  //  Partial area (edited area)

  else
    {
      mbr = misc->abe_share->edit_area;
    }

  map->fillRectangle (mbr.min_x, mbr.min_y, mbr.max_x, mbr.max_y, options->background_color, NVFalse);


  NV_FLOAT32 null_depth;
  if (options->layer_type == MIN_FILTERED_DEPTH || options->layer_type == MIN_DEPTH)
    {
      null_depth = misc->abe_share->otf_null_value = CHRTRNULL;
    }
  else
    {
      null_depth = misc->abe_share->otf_null_value = -CHRTRNULL;
    }


  //  Allocate the otf grid if needed.

  if (misc->otf_grid == NULL)
    {
      //  Figure out how many bins we'll need.

      width = misc->abe_share->otf_width = NINT ((mbr.max_x - mbr.min_x) / x_bin_size_degrees) + 1;
      height = misc->abe_share->otf_height = NINT ((mbr.max_y - mbr.min_y) / y_bin_size_degrees) + 1;

      size = width * height * sizeof (OTF_GRID_RECORD);

      QString skey;
      skey.sprintf ("%d_abe_otf_grid", misc->process_id);

      misc->otfShare = new QSharedMemory (skey);

      if (!misc->otfShare->create (size, QSharedMemory::ReadWrite)) misc->otfShare->attach (QSharedMemory::ReadWrite);

      misc->otf_grid = (OTF_GRID_RECORD *) misc->otfShare->data ();

      misc->otfShare->lock ();


      //  Clear the memory block.

      memset (misc->otf_grid, 0, size);

      for (NV_INT32 i = 0 ; i < height ; i++)
        {
          for (NV_INT32 j = 0 ; j < width ; j++)
            {
              misc->otf_grid[i * width + j].min = CHRTRNULL;
              misc->otf_grid[i * width + j].max = -CHRTRNULL;
              misc->otf_grid[i * width + j].cnt = OTF_GRID_MAX;
            }
        }


      misc->displayed_area_min = CHRTRNULL;
      misc->displayed_area_max = -CHRTRNULL;
    }


  //  If we're just redrawing after an edit we need to clear the edited area.  To do this we check to see if the OTF bin is in
  //  the displayed area.  If it is, we set the bin to null depth so that it will get replaced prior to drawing.

  if (!misc->clear)
    {
      for (NV_INT32 i = 0 ; i < height ; i++)
        {
          NV_FLOAT64 bin_lat = misc->total_displayed_area.min_y + (NV_FLOAT64) i * y_bin_size_degrees;
          for (NV_INT32 j = 0 ; j < width ; j++)
            {
              NV_FLOAT64 bin_lon = misc->total_displayed_area.min_x + (NV_FLOAT64) j * x_bin_size_degrees;
              if (bin_lat >= misc->abe_share->edit_area.min_y && bin_lat < misc->abe_share->edit_area.max_y &&
                  bin_lon >= misc->abe_share->edit_area.min_x && bin_lon < misc->abe_share->edit_area.max_x)
                {
                  misc->otf_grid[i * width + j].cnt = OTF_GRID_MAX;
                  misc->otf_grid[i * width + j].min = CHRTRNULL;
                  misc->otf_grid[i * width + j].max = -CHRTRNULL;
                  misc->otf_grid[i * width + j].avg = 0.0;
                  misc->otf_grid[i * width + j].std = 0.0;
                }
            }
        }
    }


  NV_BOOL min_lock = NVFalse, max_lock = NVFalse;
  NV_INT32 attribute = 0;

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Only if we want to display it.

      if (misc->abe_share->display_pfm[pfm])
        {
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


          //  If the width or height is 0 or negative we have asked for an area outside of the PFM's MBR so we don't want to do anything.

          if (misc->displayed_area_width[pfm] > 0 && misc->displayed_area_height[pfm] > 0)
            {
              //  We only want to do attributes if we're coloring by number, stddev, avg - min, max - avg, or max - min.

              if (misc->color_by_attribute > 0 && misc->color_by_attribute < PRE_ATTR) attribute = misc->color_by_attribute;

              misc->current_row = (NV_FLOAT32 *) calloc (misc->abe_share->otf_width, sizeof (NV_FLOAT32));
              if (misc->current_row == NULL)
                {
                  perror (pfmView::tr ("Allocating current_row in paint_otf_surface").toAscii ());
                  exit (-1);
                }

              misc->next_row = (NV_FLOAT32 *) calloc (misc->abe_share->otf_width, sizeof (NV_FLOAT32));
              if (misc->next_row == NULL)
                {
                  perror (pfmView::tr ("Allocating next_row in paint_otf_surface").toAscii ());
                  exit (-1);
                }

              if (attribute)
                {
                  misc->current_attr = (NV_FLOAT32 *) calloc (misc->abe_share->otf_width, sizeof (NV_FLOAT32));
                  if (misc->current_attr == NULL)
                    {
                      perror (pfmView::tr ("Allocating current_attr in paint_otf_surface").toAscii ());
                      exit (-1);
                    }
                }


              misc->statusProg->setRange (misc->hatchr_start_y, misc->hatchr_end_y);
              QString title = pfmView::tr (" Reading %1 of %2 : ").arg (misc->abe_share->pfm_count - pfm).arg (misc->abe_share->pfm_count) +
                QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName () + " ";
              misc->statusProgLabel->setText (title);
              misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
              misc->statusProgLabel->setPalette (misc->statusProgPalette);
              misc->statusProg->setTextVisible (TRUE);
              qApp->processEvents();


              //  We only want to update the progress bar at about 20% increments.  This makes things go marginally faster.

              NV_INT32 prog_inc = (misc->hatchr_end_y - misc->hatchr_start_y) / 5;
              if (!prog_inc) prog_inc = 1;


              //  Read input data row by row.  Note that hatchr_start_y and hatchr_end_y may not be the same as the
              //  entire displayed area since we may only be redrawing a small edited portion of the display.

              NV_I32_COORD2 coord;
              DEPTH_RECORD *depth;
              NV_INT32 numrecs;
              for (NV_INT32 jj = misc->hatchr_start_y ; jj <= misc->hatchr_end_y ; jj++)
                {
                  if (!((jj - misc->hatchr_start_y) % prog_inc))
                    {
                      misc->statusProg->setValue (jj);
                      qApp->processEvents();
                    }


                  coord.y = misc->displayed_area_row[pfm] + jj;
                  for (NV_INT32 kk = misc->hatchr_start_x ; kk <= misc->hatchr_end_x ; kk++)
                    {
                      coord.x = misc->displayed_area_column[pfm] + kk;
                      if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &numrecs))
                        {
                          for (NV_INT32 mm = 0 ; mm < numrecs ; mm++)
                            {
                              //  Compute the bin

                              NV_INT32 bin_x = NINT ((depth[mm].xyz.x - misc->total_displayed_area.min_x) / x_bin_size_degrees);
                              NV_INT32 bin_y = NINT ((depth[mm].xyz.y - misc->total_displayed_area.min_y) / y_bin_size_degrees);


                              //  Make sure the point is within our otf grid.

                              if (bin_x >= 0 && bin_x < width - 1 && bin_y >= 0 && bin_y < height - 1)
                                {
                                  ndx = bin_y * width + bin_x;


                                  if (options->layer_type == MIN_FILTERED_DEPTH || options->layer_type == MAX_FILTERED_DEPTH ||
                                      options->layer_type == AVERAGE_FILTERED_DEPTH)
                                    {
                                      if (!(depth[mm].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                                        {
                                          //  Using avg as sum and std as sum of squares until all bins are loaded.

                                          misc->otf_grid[ndx].avg += depth[mm].xyz.z;
                                          misc->otf_grid[ndx].std += (depth[mm].xyz.z * depth[mm].xyz.z);
                                          misc->otf_grid[ndx].cnt++;


                                          if (misc->otf_grid[ndx].min > depth[mm].xyz.z) 
                                            {
                                              misc->otf_grid[ndx].min = depth[mm].xyz.z;
                                              misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                              misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                            }

                                          if (misc->otf_grid[ndx].max < depth[mm].xyz.z)
                                            {
                                              misc->otf_grid[ndx].max = depth[mm].xyz.z;
                                              misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                              misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                            }
                                        }
                                    }
                                  else
                                    {
                                      if (!(depth[mm].validity & (PFM_DELETED | PFM_REFERENCE)))
                                        {
                                          //  Using avg as sum and std as sum of squares until all bins are loaded.

                                          misc->otf_grid[ndx].avg += depth[mm].xyz.z;
                                          misc->otf_grid[ndx].std += (depth[mm].xyz.z * depth[mm].xyz.z);
                                          misc->otf_grid[ndx].cnt++;


                                          if (misc->otf_grid[ndx].min > depth[mm].xyz.z) 
                                            {
                                              misc->otf_grid[ndx].min = depth[mm].xyz.z;
                                              misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                              misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                            }

                                          if (misc->otf_grid[ndx].max < depth[mm].xyz.z)
                                            {
                                              misc->otf_grid[ndx].max = depth[mm].xyz.z;
                                              misc->displayed_area_min = MIN (misc->displayed_area_min, depth[mm].xyz.z);
                                              misc->displayed_area_max = MAX (misc->displayed_area_max, depth[mm].xyz.z);
                                            }
                                        }
                                    }
                                }
                            }

                          free (depth);
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

      if (misc->drawing_canceled) break;
    }


  //  Compute the average and standard deviation.

  for (NV_INT32 i = 0 ; i < misc->abe_share->otf_height ; i++)
    {
      for (NV_INT32 j = 0 ; j < misc->abe_share->otf_width ; j++)
        {
          ndx = i * misc->abe_share->otf_width + j;


          //  Only compute average and standard deviation if we have valid points.

          if (misc->otf_grid[ndx].cnt >= OTF_GRID_MAX)
            {
              misc->otf_grid[ndx].cnt -= OTF_GRID_MAX;

              if (misc->otf_grid[ndx].cnt)
                {
                  misc->otf_grid[ndx].avg = misc->otf_grid[ndx].avg / (NV_FLOAT64) misc->otf_grid[ndx].cnt;

                  if (misc->otf_grid[ndx].cnt > 1)
                    {
                      NV_FLOAT64 variance = ((misc->otf_grid[ndx].std -
                                              ((NV_FLOAT64) misc->otf_grid[ndx].cnt * (misc->otf_grid[ndx].avg * misc->otf_grid[ndx].avg))) /
                                             ((NV_FLOAT64) misc->otf_grid[ndx].cnt - 1.0));

                      if (variance >= 0)
                        {
                          misc->otf_grid[ndx].std = sqrt (variance);
                        }
                      else
                        {
                          misc->otf_grid[ndx].std = 0.0;
                        }
                    }
                  else
                    {
                      misc->otf_grid[ndx].std = 0.0;
                    }
                }
              else
                {
                  misc->otf_grid[ndx].avg = null_depth;
                  misc->otf_grid[ndx].std = 0.0;
                }
            }
        }
    }


  if (attribute)
    {
      //  Compute the min and max attributes.

      for (NV_INT32 i = 0 ; i < misc->abe_share->otf_height ; i++)
        {
          for (NV_INT32 j = 0 ; j < misc->abe_share->otf_width ; j++)
            {
              ndx = i * misc->abe_share->otf_width + j;


              //  Only compute min and max if we have valid points.

              if (misc->otf_grid[ndx].cnt)
                {
                  switch (attribute)
                    {
                    case 1:
                      misc->displayed_area_attr_min = qMin ((NV_FLOAT32) misc->otf_grid[ndx].cnt, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax ((NV_FLOAT32) misc->otf_grid[ndx].cnt, misc->displayed_area_attr_max);
                      break;

                    case 2:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].std, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].std, misc->displayed_area_attr_max);
                      break;

                    case 3:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].avg - misc->otf_grid[ndx].min, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].avg - misc->otf_grid[ndx].min, misc->displayed_area_attr_max);
                      break;

                    case 4:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].max - misc->otf_grid[ndx].avg, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].max - misc->otf_grid[ndx].avg, misc->displayed_area_attr_max);
                      break;

                    case 5:
                      misc->displayed_area_attr_min = qMin (misc->otf_grid[ndx].max - misc->otf_grid[ndx].min, misc->displayed_area_attr_min);
                      misc->displayed_area_attr_max = qMax (misc->otf_grid[ndx].max - misc->otf_grid[ndx].min, misc->displayed_area_attr_max);
                      break;
                    }
                }
            }
        }

      misc->color_min = misc->displayed_area_attr_min;
      misc->color_max = misc->displayed_area_attr_max;
    }
  else
    {
      misc->color_min = misc->displayed_area_min;
      misc->color_max = misc->displayed_area_max;
    }


  //  If the min or max hsv lock is set we need to check to see if we want to use the locked value(s).

  NV_FLOAT32 *min_z = &misc->displayed_area_min, *max_z = &misc->displayed_area_max;

  if (attribute)
    {
      min_z = &misc->displayed_area_attr_min;
      max_z = &misc->displayed_area_attr_max;
    }


  if (options->min_hsv_locked[attribute])
    {
      if (misc->color_min < options->min_hsv_value[attribute])
        {
          misc->color_min = options->min_hsv_value[attribute];
          min_lock = NVTrue;
        }
    }

  if (options->max_hsv_locked[attribute])
    {
      if (misc->color_max > options->max_hsv_value[attribute])
        {
          misc->color_max = options->max_hsv_value[attribute];
          max_lock = NVTrue;
        }
    }


  misc->color_range = misc->color_max - misc->color_min;
  if (misc->color_range == 0.0) misc->color_range = 1.0;


  misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
  if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;

  if (attribute)
    {
      misc->displayed_area_attr_range = misc->displayed_area_attr_max - misc->displayed_area_attr_min;
      if (misc->displayed_area_attr_range == 0.0) misc->displayed_area_attr_range = 1.0;
    }



  misc->color_range = misc->color_max - misc->color_min;

  misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
  if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents ();


  if (!misc->drawing_canceled)
    {
      //  Set the scale colors for the current range.

      if (attribute)
        {
          setScale (misc->displayed_area_attr_min, misc->displayed_area_attr_max, misc->displayed_area_attr_range, attribute, misc, options, min_lock, max_lock);
        }
      else
        {
          setScale (misc->displayed_area_min, misc->displayed_area_max, misc->displayed_area_range, attribute, misc, options, min_lock, max_lock);
        }


      //  Set these for pfm3D.

      misc->abe_share->otf_min_z = misc->displayed_area_min;
      misc->abe_share->otf_max_z = misc->displayed_area_max;


      //  Redraw everything

      if (misc->clear)
        {
          misc->hatchr_start_x = 0;
          misc->hatchr_end_x = width;
          misc->hatchr_start_y = 0;
          misc->hatchr_end_y = height;
        }


      //  Redraw a portion (what we edited)

      else
        {
          NV_F64_XYMBR tmp_mbr = misc->abe_share->edit_area;


          //  Adjust edit bounds to nearest grid point (with a border of 1 grid point to clean up the edges).

          tmp_mbr.min_y = misc->total_displayed_area.min_y + (NINT ((misc->abe_share->edit_area.min_y - misc->total_displayed_area.min_y) /
                                                                    y_bin_size_degrees) - 1) * y_bin_size_degrees;
          tmp_mbr.max_y = misc->total_displayed_area.min_y + (NINT ((misc->abe_share->edit_area.max_y - misc->total_displayed_area.min_y) /
                                                                    y_bin_size_degrees) + 1) * y_bin_size_degrees;
          tmp_mbr.min_x = misc->total_displayed_area.min_x + (NINT ((misc->abe_share->edit_area.min_x - misc->total_displayed_area.min_x) /
                                                                    x_bin_size_degrees) - 1) * x_bin_size_degrees;
          tmp_mbr.max_x = misc->total_displayed_area.min_x + (NINT ((misc->abe_share->edit_area.max_x - misc->total_displayed_area.min_x) /
                                                                    x_bin_size_degrees) + 1) * x_bin_size_degrees;


          tmp_mbr.min_y = qMax (tmp_mbr.min_y, misc->total_displayed_area.min_y);
          tmp_mbr.max_y = qMin (tmp_mbr.max_y, misc->total_displayed_area.max_y);
          tmp_mbr.min_x = qMax (tmp_mbr.min_x, misc->total_displayed_area.min_x);
          tmp_mbr.max_x = qMin (tmp_mbr.max_x, misc->total_displayed_area.max_x);



          misc->hatchr_start_x = NINT ((tmp_mbr.min_x - misc->total_displayed_area.min_x) / x_bin_size_degrees);
          misc->hatchr_end_x = misc->hatchr_start_x + (NINT ((tmp_mbr.max_x - tmp_mbr.min_x) / x_bin_size_degrees));

          misc->hatchr_start_y = NINT ((tmp_mbr.min_y - misc->total_displayed_area.min_y) / y_bin_size_degrees);
          misc->hatchr_end_y = misc->hatchr_start_y + (NINT ((tmp_mbr.max_y - tmp_mbr.min_y) / y_bin_size_degrees));
        }

      misc->displayed_area_range = misc->displayed_area_max - misc->displayed_area_min;
      if (misc->displayed_area_range == 0.0) misc->displayed_area_range = 1.0;

      misc->statusProg->setRange (misc->hatchr_start_y, misc->hatchr_end_y);
      misc->statusProgLabel->setText (pfmView::tr (" Painting surface "));
      misc->statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      //  We only want to update the progress bar at about 20% increments.  This makes things go marginally faster.

      NV_INT32 prog_inc = height / 5;
      if (!prog_inc) prog_inc = 1;


      for (NV_INT32 jj = misc->hatchr_start_y ; jj < misc->hatchr_end_y ; jj++)
        {
          if (!((jj - misc->hatchr_start_y) % prog_inc))
            {
              misc->statusProg->setValue (jj);
              qApp->processEvents();
            }

          if (attribute)
            {
              for (NV_INT32 kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (attribute)
                    {
                    case 1:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].cnt;
                      break;

                    case 2:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].std;
                      break;

                    case 3:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].avg - misc->otf_grid[jj * width + kk].min;
                      break;

                    case 4:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].max - misc->otf_grid[jj * width + kk].avg;
                      break;

                    case 5:
                      misc->current_attr[kk] = misc->otf_grid[jj * width + kk].max - misc->otf_grid[jj * width + kk].min;
                      break;
                    }
                }
            }

          if (jj == misc->hatchr_end_y - 1)
            {
              for (NV_INT32 kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (options->layer_type)
                    {
                    case MIN_FILTERED_DEPTH:
                    case MIN_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].min;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].min;
                      break;

                    case MAX_FILTERED_DEPTH:
                    case MAX_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].max;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].max;
                      break;

                    case AVERAGE_FILTERED_DEPTH:
                    case AVERAGE_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      misc->next_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      break;
                    }
                }
            }
          else
            {
              for (NV_INT32 kk = misc->hatchr_start_x ; kk < misc->hatchr_end_x ; kk++)
                {
                  switch (options->layer_type)
                    {
                    case MIN_FILTERED_DEPTH:
                    case MIN_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].min;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].min;
                      break;

                    case MAX_FILTERED_DEPTH:
                    case MAX_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].max;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].max;
                      break;

                    case AVERAGE_FILTERED_DEPTH:
                    case AVERAGE_DEPTH:
                      misc->current_row[kk] = misc->otf_grid[jj * width + kk].avg;
                      misc->next_row[kk] = misc->otf_grid[(jj + 1) * width + kk].avg;
                      break;
                    }
                }
            }


          //  HSV fill and sunshade.

          hatchr (map, options, misc->clear, misc->total_displayed_area, misc->abe_share->edit_area, misc->color_min, misc->color_max,
                  misc->color_range, x_bin_size_degrees, y_bin_size_degrees, misc->abe_share->otf_height, misc->hatchr_start_x, misc->hatchr_end_x,
                  null_depth, options->otf_bin_size_meters, options->otf_bin_size_meters, misc->current_row, misc->next_row, misc->current_attr,
                  NULL, misc->pfm_alpha[0], jj, attribute);


          if (qApp->hasPendingEvents ())
            {
              qApp->processEvents();
              if (misc->drawing_canceled) break;
            }
        }


      //  Flush all of the saved fillRectangles from hatchr to screen

      map->flush ();
    }


  free (misc->current_row);
  free (misc->next_row);
  if (attribute) free (misc->current_attr);

  misc->otfShare->unlock ();


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  if (!misc->drawing_canceled)
    {
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
