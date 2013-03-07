
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


//!  Places the hand-drawn contour values in the PFM file(s).

void writeContour (MISC *misc, NV_FLOAT32 z_factor, NV_FLOAT32 z_offset, NV_INT32 count, NV_FLOAT64 *cur_x, NV_FLOAT64 *cur_y)
{
  static NV_BOOL first = NVTrue;
  static NV_INT16 line_num[MAX_ABE_PFMS];


  if (first)
    {
      first = NVFalse;

      for (NV_INT32 pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++) line_num[pfm] = 0;
    }


  //  Loop through all of the displayed PFM layers.

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (misc->contour_in_pfm[pfm] && misc->abe_share->display_pfm[pfm] && misc->average_type[pfm])
	{
	  //  Close and then reopen (then close and reopen at the end) because add_depth_record kills use of the saved
	  //  head and tail pointers (depth_chain) in the PFM BIN structure.

	  close_pfm_file (misc->pfm_handle[pfm]);
          misc->abe_share->open_args[pfm].checkpoint = 0;
	  misc->pfm_handle[pfm] = open_existing_pfm_file (&misc->abe_share->open_args[pfm]);


	  //  Check to see if the file is already in the PFM ctl (list) file.

	  NV_INT32 file_num = -1;
	  NV_INT32 file_count = get_next_list_file_number (misc->pfm_handle[pfm]);

	  for (NV_INT16 i = 0 ; i < file_count ; i++)
	    {
	      NV_CHAR filename[512];
	      NV_INT16 type;

	      read_list_file (misc->pfm_handle[pfm], i, filename, &type);

	      if (!strcmp (filename, "pfmView_hand_drawn_contour"))
		{
		  file_num = i;
		  break;
		}
	    }


	  if (file_num == -1)
	    {
	      NV_CHAR name[128];
	      strcpy (name, "pfmView_hand_drawn_contour");
	      file_num = write_list_file (misc->pfm_handle[pfm], name, PFM_UNDEFINED_DATA);
	      line_num[pfm] = write_line_file (misc->pfm_handle[pfm], name);
	    }
          else
            {
              //  Since pfmView_hand_drawn_contour was in the list file it must also
              //  be in the line file so we need to snag the line number.

              line_num[pfm] = -1;
              NV_INT32 line_count = get_next_line_number (misc->pfm_handle[pfm]);

              for (NV_INT16 i = 0 ; i < line_count ; i++)
                {
                  NV_CHAR linename[512];

                  strcpy (linename, read_line_file (misc->pfm_handle[pfm], i));

                  if (!strcmp (linename, "pfmView_hand_drawn_contour"))
                    {
                      line_num[pfm] = i;
                      break;
                    }
                }


              //  If we didn't find the pfmView_hand_drawn_contour line name something is very wrong.

              if (line_num[pfm] == -1)
                {
                  QMessageBox::critical (0, pfmView::tr ("pfmView contour"),
                                         pfmView::tr ("Can't find the pfmView_hand_drawn_contour entry in the line file!!"));
                  return;
                }
            }


	  DEPTH_RECORD depth_record;
	  NV_I32_COORD2 min_coord, max_coord;

	  min_coord.x = 999999999;
	  min_coord.y = 999999999;
	  max_coord.x = -999999999;
	  max_coord.y = -999999999;


          //  Compute the minimum cell size in meters.  This will be our step along each contour segment so that we make sure
          //  that we place a point in all cells along the contour.

          NV_FLOAT64 min_cell_size = qMin (misc->ss_cell_size_x[pfm], misc->ss_cell_size_y[pfm]);


          //  Loop through each segment of the contour starting with [0] and [1] (Note that the loop starts at 1).

	  for (NV_INT32 i = 1 ; i < count ; i++)
	    {
              //  Save the segment end points so that we can clip them to the overall MBR without destroying the original
              //  segment ends.

              NV_FLOAT64 tmp_x[2] = {cur_x[i - 1], cur_x[i]}, tmp_y[2] = {cur_y[i - 1], cur_y[i]};


              //  Clip the segment to the overall MBR

              if (clip (&tmp_x[0], &tmp_y[0], &tmp_x[1], &tmp_y[1], misc->abe_share->open_args[pfm].head.mbr))
                {
                  //  Compute the distance and azimuth from one end of the segment to the other.

                  NV_FLOAT64 dist, az;
                  invgp (NV_A0, NV_B0, tmp_y[0], tmp_x[0], tmp_y[1], tmp_x[1], &dist, &az);


                  //  Figure out how many steps we'll have to use as we move along the segment so that we'll hit every
                  //  intervening cell.

                  NV_INT32 steps = (NV_INT32) (dist / min_cell_size);

                  NV_I32_COORD2 coord, prev_coord = {-1, -1};


                  //  Move along the segment in min_cell_size increments.

                  for (NV_INT32 j = 0 ; j <= steps ; j++)
                    {
                      NV_F64_COORD2 nxy = {tmp_x[0], tmp_y[0]};


                      compute_index_ptr (nxy, &coord, &misc->abe_share->open_args[pfm].head);


                      //  If we haven't changed cells, move on.

                      if (prev_coord.x != coord.x || prev_coord.y != coord.y)
                        {
                          //  We don't want to save points that fall in cells with previously existing valid data.

                          BIN_RECORD bin;


                          read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


                          if (!(bin.validity & PFM_DATA))
                            {
                              misc->last_saved_contour_record[pfm]++;


                              memset (&depth_record, 0, sizeof (DEPTH_RECORD));

                              depth_record.xyz.x = tmp_x[0];
                              depth_record.xyz.y = tmp_y[0];
                              depth_record.xyz.z = (misc->draw_contour_level - z_offset) / z_factor;


                              //  Use PFM_DATA (normally not used in DEPTH_RECORD) so that we can replace the value in this
                              //  bin.  Otherwise, the contours will be very jagged if we just leave the contour value in
                              //  the MISPed surface.  Hopefully, I can remove this when I get NAVO to switch to CHRTR2 for
                              //  gridding.

                              depth_record.validity = PFM_MODIFIED | PFM_DATA;
                              depth_record.beam_number = 1;
                              depth_record.ping_number = misc->last_saved_contour_record[pfm];
                              depth_record.line_number = line_num[pfm];
                              depth_record.file_number = file_num;


                              NV_F64_COORD2 nxy = {depth_record.xyz.x, depth_record.xyz.y};
                              compute_index_ptr (nxy, &depth_record.coord, &misc->abe_share->open_args[pfm].head);


                              //  If we're capturing the interpolated contours we have to beat them against the 
                              //  input polygon.

                              if (misc->function != GRAB_CONTOUR ||
                                  inside_polygon2 (misc->polygon_x, misc->polygon_y, misc->poly_count,
                                                   depth_record.xyz.x, depth_record.xyz.y))
                                {
                                  //  Load the point.

                                  NV_INT32 status;
                                  if ((status = add_depth_record_index (misc->pfm_handle[pfm], &depth_record)))
                                    {
                                      if (status != ADD_DEPTH_RECORD_OUT_OF_LIMITS_ERROR) pfm_error_exit (status);
                                    }
                                  else
                                    {
                                      min_coord.x = qMin (min_coord.x, depth_record.coord.x);
                                      min_coord.y = qMin (min_coord.y, depth_record.coord.y);
                                      max_coord.x = qMax (max_coord.x, depth_record.coord.x);
                                      max_coord.y = qMax (max_coord.y, depth_record.coord.y);
                                    }
                                }
                            }
                        }


                      //  Add min_cell_size to the X and Y positions in the "az" direction (i.e. increment the position
                      //  along the segment).

                      NV_FLOAT64 lat, lon;
                      newgp (tmp_y[0], tmp_x[0], az, min_cell_size, &lat, &lon);


                      if (misc->dateline && lon < 0.0) lon += 360.0;


                      tmp_y[0] = lat;
                      tmp_x[0] = lon;


                      //  Save the previous coordinates.

                      prev_coord = coord;
                    }


                  //  Make sure we get the last cell (tmp_y[1],tmp_x[1]).

                  NV_F64_COORD2 nxy = {tmp_x[1], tmp_y[1]};


                  compute_index_ptr (nxy, &coord, &misc->abe_share->open_args[pfm].head);


                  //  If we haven't changed cells, move on.

                  if (prev_coord.x != coord.x || prev_coord.y != coord.y)
                    {
                      //  We don't want to save points that fall in cells with previously existing valid data.

                      BIN_RECORD bin;


                      read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


                      if (!(bin.validity & PFM_DATA))
                        {
                          misc->last_saved_contour_record[pfm]++;

                          memset (&depth_record, 0, sizeof (DEPTH_RECORD));

                          depth_record.xyz.x = tmp_x[1];
                          depth_record.xyz.y = tmp_y[1];
                          depth_record.xyz.z = (misc->draw_contour_level - z_offset) / z_factor;


                          //  Use PFM_DATA (normally not used in DEPTH_RECORD) so that we can replace the value in this bin.
                          //  Otherwise, the contours will be very jagged if we just leave the contour value in the MISPed
                          //  surface.  Hopefully, I can remove this when I get NAVO to switch to CHRTR2 for gridding.

                          depth_record.validity = PFM_MODIFIED | PFM_DATA;
                          depth_record.beam_number = 1;
                          depth_record.ping_number = misc->last_saved_contour_record[pfm];
                          depth_record.line_number = line_num[pfm];
                          depth_record.file_number = file_num;


                          NV_F64_COORD2 nxy = {depth_record.xyz.x, depth_record.xyz.y};
                          compute_index_ptr (nxy, &depth_record.coord, &misc->abe_share->open_args[pfm].head);


                          //  If we're capturing the interpolated contours we have to beat them against the 
                          //  input polygon.

                          if (misc->function != GRAB_CONTOUR ||
                              inside_polygon2 (misc->polygon_x, misc->polygon_y, misc->poly_count,
                                               depth_record.xyz.x, depth_record.xyz.y))
                            {
                              //  Load the point.

                              NV_INT32 status;
                              if ((status = add_depth_record_index (misc->pfm_handle[pfm], &depth_record)))
                                {
                                  if (status != ADD_DEPTH_RECORD_OUT_OF_LIMITS_ERROR) pfm_error_exit (status);
                                }
                              else
                                {
                                  min_coord.x = qMin (min_coord.x, depth_record.coord.x);
                                  min_coord.y = qMin (min_coord.y, depth_record.coord.y);
                                  max_coord.x = qMax (max_coord.x, depth_record.coord.x);
                                  max_coord.y = qMax (max_coord.y, depth_record.coord.y);
                                }
                            }
                        }
                    }
                }
            }


	  //  Reopen so we can again use the saved depth_chain addresses in the BIN records.

	  close_pfm_file (misc->pfm_handle[pfm]);
          misc->abe_share->open_args[pfm].checkpoint = 0;
	  misc->pfm_handle[pfm] = open_existing_pfm_file (&misc->abe_share->open_args[pfm]);


	  NV_I32_COORD2 coord;
	  BIN_RECORD bin_record;

	  for (NV_INT32 i = min_coord.y ; i <= max_coord.y ; i++)
	    {
	      coord.y = i;

	      for (NV_INT32 j = min_coord.x ; j <= max_coord.x ; j++)
		{
		  coord.x = j;

		  recompute_bin_values_index (misc->pfm_handle[pfm], coord, &bin_record, 0);
		}
	    }
	}
    }
}
