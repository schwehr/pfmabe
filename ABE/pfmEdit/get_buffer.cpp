
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

  - Module Name:        get_buffer

  - Programmer(s):      Jan C. Depner

  - Date Written:       May 1999

  - Purpose:            Read the point cloud data for the area defined in
                        shared memory (passed from pfmView or pfmEditShell).
                        Populate the shared memory point cloud buffer with
                        the data.  We also get unique data types and set some
                        flags to indicate if particular data types are present.

  - Return Value:       void

****************************************************************************/

void get_buffer (MISC *misc)
{
  NV_I32_COORD2   coord;
  NV_INT32        recnum;
  NV_F64_COORD2   xy;
  NV_BOOL         found;
  DEPTH_RECORD    *depth;


  //  Check for the max number of index attributes for any of the PFM files.  Compute average bin size
  //  for all of the PFM files.

  misc->max_attr = 0;
  misc->x_grid_size = 0.0;
  misc->y_grid_size = 0.0;

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      misc->max_attr = qMax (misc->abe_share->open_args[pfm].head.num_ndx_attr, misc->max_attr);

      misc->x_grid_size += misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
      misc->y_grid_size += misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
    }


  misc->x_grid_size /= (NV_FLOAT64) misc->abe_share->pfm_count;
  misc->y_grid_size /= (NV_FLOAT64) misc->abe_share->pfm_count;


  //  Clear some important variables

  memset (misc->line_number, 0, sizeof (misc->line_number));
  misc->line_count = 0;
  misc->abe_share->point_cloud_count = 0;

  misc->min_z = 99999999.0;
  misc->max_z = -99999999.0;
  NV_INT16 data_type_lut[MAX_PFM_FILES][PFM_MAX_FILES];


  misc->unique_count = 0;


  //  Build a lookup table for the file types and get the unique data types.

  NV_INT32 point_count = 0;
  memset (data_type_lut, 0, MAX_PFM_FILES * PFM_MAX_FILES * sizeof (NV_INT16));

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  File types.

      NV_INT32 last = get_next_list_file_number (misc->pfm_handle[pfm]);

      for (NV_INT32 list = 0 ; list < last ; list++)
        {
          NV_CHAR tmp[512];

          read_list_file (misc->pfm_handle[pfm], list, tmp, &data_type_lut[pfm][list]);


          //  Unique data types.

          NV_BOOL hit = NVFalse;
          for (NV_INT32 i = 0 ; i < misc->unique_count ; i++)
            {
              if (misc->unique_type[i] == data_type_lut[pfm][list])
                {
                  hit = NVTrue;
                  break;
                }
            }

          if (!hit)
            {
              misc->unique_type[misc->unique_count] = data_type_lut[pfm][list];
              misc->unique_count++;
            }
        }


      //  Check for LIDAR and/or GSF data so we can determine what to do with LIDAR and/or GSF tool bar buttons.

      for (NV_INT32 i = 0 ; i < misc->unique_count ; i++)
        {
          if (misc->unique_type[i] == PFM_GSF_DATA) misc->gsf_present = NVTrue;

          if (misc->unique_type[i] == PFM_CHARTS_HOF_DATA) misc->hof_present = NVTrue;

          if (misc->unique_type[i] == PFM_CHARTS_HOF_DATA || misc->unique_type[i] == PFM_SHOALS_1K_DATA || misc->unique_type[i] == PFM_WLF_DATA ||
              misc->unique_type[i] == PFM_HAWKEYE_HYDRO_DATA || misc->unique_type[i] == PFM_CZMIL_DATA) misc->hydro_lidar_present = NVTrue;

          if (misc->unique_type[i] == PFM_CHARTS_HOF_DATA || misc->unique_type[i] == PFM_SHOALS_1K_DATA ||
              misc->unique_type[i] == PFM_SHOALS_TOF_DATA || misc->unique_type[i] == PFM_WLF_DATA ||
              misc->unique_type[i] == PFM_HAWKEYE_HYDRO_DATA || misc->unique_type[i] == PFM_HAWKEYE_TOPO_DATA ||
              misc->unique_type[i] == PFM_CZMIL_DATA) misc->lidar_present = NVTrue;
        }


      //  Read the buffer.

      xy.y = misc->abe_share->edit_area.min_y;
      xy.x = misc->abe_share->edit_area.min_x;

      compute_index_ptr (xy, &misc->ll[pfm], &misc->abe_share->open_args[pfm].head);

      if (misc->ll[pfm].x < 0) misc->ll[pfm].x = 0;
      if (misc->ll[pfm].y < 0) misc->ll[pfm].y = 0;


      xy.y = misc->abe_share->edit_area.max_y;
      xy.x = misc->abe_share->edit_area.max_x;
        
      compute_index_ptr (xy, &misc->ur[pfm], &misc->abe_share->open_args[pfm].head);

      if (misc->ur[pfm].x > misc->abe_share->open_args[pfm].head.bin_width - 1) misc->ur[pfm].x = misc->abe_share->open_args[pfm].head.bin_width - 1;
      if (misc->ur[pfm].y > misc->abe_share->open_args[pfm].head.bin_height - 1) misc->ur[pfm].y = misc->abe_share->open_args[pfm].head.bin_height - 1;


      misc->null_val[pfm] = misc->abe_share->open_args[pfm].head.null_depth;


      //  Determine the floating point format for the attributes.

      for (NV_INT32 i = 0 ; i < misc->max_attr ; i++)
        {
          NV_INT32 decimals = (NV_INT32) (log10 (misc->abe_share->open_args[pfm].head.ndx_attr_scale[i]) + 0.05);
          sprintf (misc->attr_format[pfm][i], "%%.%df", decimals);
        }


      //  In order to determine the data size up front (so we can put it in shared memory for other processes to access) we
      //  have to pre-scan the bin records to get the number of points in each.  This shouldn't take too long though and it
      //  saves us a ton of memory usage in ancillary programs.

      NV_INT32 row_width = (misc->ur[pfm].x - misc->ll[pfm].x) + 1;


      if (row_width >= 1)
        {
          BIN_RECORD *row = (BIN_RECORD *) calloc (row_width, sizeof (BIN_RECORD));
          if (row == NULL)
            {
              perror (pfmEdit::tr ("Allocating row in get_buffer").toAscii ());
              exit (-1);
            }


          for (NV_INT32 i = misc->ll[pfm].y ; i <= misc->ur[pfm].y ; i++)
            {
              read_bin_row (misc->pfm_handle[pfm], row_width, i, misc->ll[pfm].x, row);

              NV_FLOAT64 slat = misc->abe_share->open_args[pfm].head.mbr.min_y + (NV_FLOAT64) i * misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
              NV_FLOAT64 nlat = slat + misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

              for (NV_INT32 j = 0 ; j < row_width ; j++)
                {
                  //  For polygons we can eliminate bins that are completely outside of the polygon.  For rectangles we're just going
                  //  to live with the extra slop around the edges.

                  if (misc->abe_share->polygon_count)
                    {
                      NV_FLOAT64 wlon = misc->abe_share->open_args[pfm].head.mbr.min_x + (NV_FLOAT64) (j + misc->ll[pfm].x) *
                        misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
                      NV_FLOAT64 elon = wlon + misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

                      if (inside_polygon2 (misc->abe_share->polygon_x, misc->abe_share->polygon_y, misc->abe_share->polygon_count, wlon, slat) ||
                          inside_polygon2 (misc->abe_share->polygon_x, misc->abe_share->polygon_y, misc->abe_share->polygon_count, wlon, nlat) ||
                          inside_polygon2 (misc->abe_share->polygon_x, misc->abe_share->polygon_y, misc->abe_share->polygon_count, elon, nlat) ||
                          inside_polygon2 (misc->abe_share->polygon_x, misc->abe_share->polygon_y, misc->abe_share->polygon_count, elon, slat))
                        {
                          point_count += row[j].num_soundings;
                        }
                    }
                  else
                    {
                      point_count += row[j].num_soundings;
                    }
                }
            }

          free (row);
        }
    }


  //  Check to see if the point cloud memory already exists (due to a previous editor crashed).  If so, attach it, delete it, and then create it again.
  //  The key is the parent process ID (the one used for the main ABE shared memory area) plus "_abe_pfmEdit".  

  QString key;
  key.sprintf ("%d_abe_pfmEdit", misc->abe_share->ppid);

  misc->dataShare = new QSharedMemory (key);

  if (!misc->dataShare->create (sizeof (POINT_CLOUD) * point_count, QSharedMemory::ReadWrite))
    {
      misc->dataShare->attach (QSharedMemory::ReadWrite);
      misc->dataShare->detach ();

      if (!misc->dataShare->create (sizeof (POINT_CLOUD) * point_count, QSharedMemory::ReadWrite))
        {
          QMessageBox::critical (0, pfmEdit::tr ("pfmEdit3D get_buffer"),
                                 pfmEdit::tr ("Unable to allocate %1 points (%2 bytes) of shared data memory.  Reason:%3").arg
                                 (point_count) .arg (sizeof (POINT_CLOUD) * point_count).arg
                                 (misc->dataShare->errorString ()));
          fprintf (stderr, "Unable to allocate %d points of shared data memory in get_buffer.c.  Reason:%s", point_count,
                   strerror (errno));
          exit (-1);
        }
    }

  misc->data = (POINT_CLOUD *) misc->dataShare->data ();


  //  Go ahead and lock it.  We'll unlock it if we need to share with an external program.

  misc->dataShare->lock ();


  //  Now we have to load the points that are inside our area of interest.

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      for (NV_INT32 i = misc->ll[pfm].y ; i <= misc->ur[pfm].y ; i++)
        {
          coord.y = i;
          for (NV_INT32 j = misc->ll[pfm].x ; j <= misc->ur[pfm].x ; j++)
            {
              coord.x = j;

              if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
                {
                  for (NV_INT32 k = 0 ; k < recnum ; k++)
                    {
                      //  DO NOT use data marked as PFM_DELETED.

                      if (!(depth[k].validity & PFM_DELETED))
                        {
                          //  If the polygon count is zero we are loading a rectangle so we want to check against that.
                          //  Otherwise it's a polygon so we need to check against that.

                          NV_BOOL in = NVFalse;
                          if (misc->abe_share->polygon_count)
                            {
                              if (inside_polygon2 (misc->abe_share->polygon_x, misc->abe_share->polygon_y, misc->abe_share->polygon_count,
                                                   depth[k].xyz.x, depth[k].xyz.y)) in = NVTrue;
                            }
                          else
                            {
                              if (depth[k].xyz.x >= misc->abe_share->edit_area.min_x && depth[k].xyz.x <= misc->abe_share->edit_area.max_x &&
                                  depth[k].xyz.y >= misc->abe_share->edit_area.min_y && depth[k].xyz.y <= misc->abe_share->edit_area.max_y) in = NVTrue;
                            }


                          if (in)
                            {
                              for (NV_INT32 m = 0 ; m < misc->max_attr ; m++)
                                {
                                  misc->data[misc->abe_share->point_cloud_count].attr[m] = 0.0;
                                  if (misc->abe_share->open_args[pfm].head.num_ndx_attr) misc->data[misc->abe_share->point_cloud_count].attr[m] = depth[k].attr[m];
                                }

                              misc->data[misc->abe_share->point_cloud_count].x = depth[k].xyz.x;
                              misc->data[misc->abe_share->point_cloud_count].y = depth[k].xyz.y;
                              misc->data[misc->abe_share->point_cloud_count].z = depth[k].xyz.z;


                              //  Compute min and max Z values for displayed data (valid or not).

                              misc->min_z = qMin (misc->data[misc->abe_share->point_cloud_count].z, misc->min_z);
                              misc->max_z = qMax (misc->data[misc->abe_share->point_cloud_count].z, misc->max_z);


                              misc->data[misc->abe_share->point_cloud_count].herr = depth[k].horizontal_error;
                              misc->data[misc->abe_share->point_cloud_count].verr = depth[k].vertical_error;
                              misc->data[misc->abe_share->point_cloud_count].val = depth[k].validity;
                              misc->data[misc->abe_share->point_cloud_count].pfm = pfm;
                              misc->data[misc->abe_share->point_cloud_count].file = depth[k].file_number;
                              misc->data[misc->abe_share->point_cloud_count].type = data_type_lut[pfm][depth[k].file_number];


                              //  We need the line numbers to be unique because we compare line numbers
                              //  to line numbers in various places in the code.  They won't be unique if we're 
                              //  displaying multiple PFM files so we're cheating and adding the PFM number
                              //  times SHARED_LINE_MULT to make them unique and still be able to determine which
                              //  PFM they are associated with.  Clever, no ;-)

                              misc->data[misc->abe_share->point_cloud_count].line = pfm * SHARED_LINE_MULT + depth[k].line_number;


                              misc->data[misc->abe_share->point_cloud_count].rec = depth[k].ping_number;
                              misc->data[misc->abe_share->point_cloud_count].sub = depth[k].beam_number;
                              misc->data[misc->abe_share->point_cloud_count].addr = depth[k].address.block;
                              misc->data[misc->abe_share->point_cloud_count].pos = depth[k].address.record;
                              misc->data[misc->abe_share->point_cloud_count].oval = depth[k].validity;
                              misc->data[misc->abe_share->point_cloud_count].xcoord = coord.x;
                              misc->data[misc->abe_share->point_cloud_count].ycoord = coord.y;
                              misc->data[misc->abe_share->point_cloud_count].exflag = NVFalse;
                              misc->data[misc->abe_share->point_cloud_count].mask = NVFalse;
                              misc->data[misc->abe_share->point_cloud_count].fmask = NVFalse;


                              //  Check for "reference data".

                              if (depth[k].validity & PFM_REFERENCE) misc->reference_flag = NVTrue;


                              //  Define which lines are loaded in the edit window.

                              found = NVFalse;
                              for (NV_INT32 m = 0 ; m < misc->line_count ; m++)
                                {
                                  if (misc->data[misc->abe_share->point_cloud_count].line == misc->line_number[m])
                                    {
                                      found = NVTrue;
                                      break;
                                    }
                                }

                              if (!found)
                                {
                                  misc->line_number[misc->line_count] = misc->data[misc->abe_share->point_cloud_count].line;

                                  misc->line_type[misc->line_count] = misc->data[misc->abe_share->point_cloud_count].type;

                                  misc->line_count++;
                                }

                              misc->abe_share->point_cloud_count++;
                            }
                        }
                    }
                  free (depth);
                }
            }
        }


      //  Get any line shift information.

      QString file;
      FILE *fp;
      NV_CHAR string[2048];

      file.sprintf ("%s/%s", misc->abe_share->open_args[pfm].list_path, gen_basename (misc->abe_share->open_args[pfm].list_path));


      //  Check to see if this is a new PFM structure.

      if ((fp = fopen (file.toAscii (), "r")) != NULL)
        {
          fclose (fp);
          file.sprintf ("%s/%s.line_shift", misc->abe_share->open_args[pfm].list_path, 
			gen_basename (misc->abe_share->open_args[pfm].list_path));
        }
      else
        {
          file.sprintf ("%s.line_shift", misc->abe_share->open_args[pfm].list_path);
        }

      if ((fp = fopen (file.toAscii (), "r")) != NULL)
        {
          NV_FLOAT32 temp = 0.0;
          NV_INT32 file_num, line;
          NV_CHAR tmp[1024];
          while (fgets (string, sizeof (string), fp) != NULL)
            {
              sscanf (string, "%d,%d,%f,%s", &file_num, &line, &temp, tmp);

              misc->line_shift[pfm][line] = temp;

              for (NV_INT32 i = 0 ; i < misc->line_count ; i++)
                {
                  if (line == misc->line_number[i] % SHARED_LINE_MULT)
                    {
                      misc->shifted[pfm] = NVTrue;

                      for (NV_INT32 j = 0 ; j < misc->abe_share->point_cloud_count ; j++)
                        {
                          if (misc->data[j].line % SHARED_LINE_MULT == line) misc->data[j].z += temp;
                        }

                      break;
                    }
                }
            }

          fclose (fp);
        }
    }


  if (misc->line_count == 1)
    {
      misc->line_interval = 1;
    }
  else
    {
      misc->line_interval = ((NUMSHADES * 2) - 1) / (misc->line_count - 1);
    }


  //  Since the lines are loaded into the misc->line_number array based on when they were read in we're
  //  going to try to separate the line colors somewhat by moving the lines around in the line_number array.
  //  Hopefully this will give us better color separation in color by line mode.

  NV_INT32 *ln, *lt;

  ln = (NV_INT32 *) calloc (misc->line_count, sizeof (NV_INT32));
  if (ln == NULL)
    {
      QMessageBox::critical (0, pfmEdit::tr ("pfmEdit get_buffer"), pfmEdit::tr ("Unable to allocate line number array"));
      exit (-1);
    }

  lt = (NV_INT32 *) calloc (misc->line_count, sizeof (NV_INT32));
  if (lt == NULL)
    {
      QMessageBox::critical (0, pfmEdit::tr ("pfmEdit get_buffer"), pfmEdit::tr ("Unable to allocate line type array"));
      exit (-1);
    }

  for (NV_INT32 i = 0, j = 0, k = misc->line_count - 1 ; i < misc->line_count ; i++)
    {
      if (i % 2)
        {
          ln[k] = misc->line_number[i];
          lt[k] = misc->line_type[i];
          k--;
        }
      else
        {
          ln[j] = misc->line_number[i];
          lt[j] = misc->line_type[i];
          j++;
        }
    }

  for (NV_INT32 i = 0 ; i < misc->line_count ; i++)
    {
      misc->line_number[i] = ln[i];
      misc->line_type[i] = lt[i];
    }

  free (ln);
  free (lt);


  //  Define the bounds + 2%

  NV_FLOAT64 width_2 = (misc->abe_share->edit_area.max_x - misc->abe_share->edit_area.min_x) * 0.02;
  NV_FLOAT64 height_2 = (misc->abe_share->edit_area.max_y - misc->abe_share->edit_area.min_y) * 0.02;
  misc->abe_share->edit_area.min_y -= height_2;
  misc->abe_share->edit_area.max_y += height_2;
  misc->abe_share->edit_area.min_x -= width_2;
  misc->abe_share->edit_area.max_x += width_2;


  misc->displayed_area.min_x = misc->abe_share->edit_area.min_x;
  misc->displayed_area.max_x = misc->abe_share->edit_area.max_x;
  misc->displayed_area.min_y = misc->abe_share->edit_area.min_y;
  misc->displayed_area.max_y = misc->abe_share->edit_area.max_y;


  //  Try to open the feature file and read the features into memory.

  if (strcmp (misc->abe_share->open_args[0].target_path, "NONE"))
    {
      if (misc->bfd_open) binaryFeatureData_close_file (misc->bfd_handle);
      misc->bfd_open = NVFalse;

      if ((misc->bfd_handle = binaryFeatureData_open_file (misc->abe_share->open_args[0].target_path, &misc->bfd_header, BFDATA_UPDATE)) >= 0)
        {
          if (binaryFeatureData_read_all_short_features (misc->bfd_handle, &misc->feature) < 0)
            {
              QString msg = QString (binaryFeatureData_strerror ());
              QMessageBox::warning (0, pfmEdit::tr ("pfmEdit"), pfmEdit::tr ("Unable to read feature records\nReason: ") + msg);
              binaryFeatureData_close_file (misc->bfd_handle);
            }
          else
            {
              misc->bfd_open = NVTrue;
            }
        }
    }
}
