
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



#include "remisp.hpp"


#define         EPS    1.0e-3         // epsilon criteria for finding match
#define         FILTER 9


/*!
  This function runs the MISP operation on the selected area.  This is rather complicated and
  it's only used if you have run pfmMisp on the file.
*/

remisp::remisp (MISC *mi, OPTIONS *op, NV_F64_XYMBR *m)
{
  NV_F64_COORD3      *xyz_array = NULL;
  NV_INT32           out_count = 0, misp_weight;
  NV_I32_COORD2      coord;
  BIN_RECORD         bin;
  DEPTH_RECORD       *depth;
  NV_INT32           recnum;
  NV_F64_COORD3      xyz;
  NV_F64_XYMBR       grid_mbr[MAX_ABE_PFMS];
  NV_INT32           gridcols[MAX_ABE_PFMS], gridrows[MAX_ABE_PFMS], col_filter[MAX_ABE_PFMS], row_filter[MAX_ABE_PFMS];
  NV_FLOAT64         half_x[MAX_ABE_PFMS], half_y[MAX_ABE_PFMS];
  NV_BOOL            land_mask_flag = NVFalse;
  QString            progText;
  gridThread         grid_thread;


  misc = mi;
  options = op;
  mbr = m;

  misp_weight = options->misp_weight;
  complete = NVFalse;


  //  First we retrieve all valid data points from all available PFM layers.

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      land_mask_flag = NVFalse;


      if (misc->abe_share->display_pfm[pfm] && misc->average_type[pfm])
	{
          //  Check for the land mask flag in PFM_USER_05 in any of the PFM layers.

          if (!strcmp (misc->abe_share->open_args[pfm].head.user_flag_name[4], "Land masked point")) land_mask_flag = NVTrue;


	  //  Adjust the MBR to the nearest grid point.  Note that part or all of the MBR may be outside
	  //  of the PFM bounds if we are handling more than one layer.

	  grid_mbr[pfm].min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
	    (NINT ((mbr->min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) / 
		   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

	  grid_mbr[pfm].max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
	    (NINT ((mbr->max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
		   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

	  grid_mbr[pfm].min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
	    (NINT ((mbr->min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
		   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

	  grid_mbr[pfm].max_x = misc->abe_share->open_args[pfm].head.mbr.min_x +
	    (NINT ((mbr->max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
		   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
	    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


	  //  Add the filter border to the MBR

	  grid_mbr[pfm].min_x -= ((NV_FLOAT64) FILTER * misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  grid_mbr[pfm].min_y -= ((NV_FLOAT64) FILTER * misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
	  grid_mbr[pfm].max_x += ((NV_FLOAT64) FILTER * misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  grid_mbr[pfm].max_y += ((NV_FLOAT64) FILTER * misc->abe_share->open_args[pfm].head.y_bin_size_degrees);


	  //  Number of rows and columns in the area

	  gridcols[pfm] = NINT ((grid_mbr[pfm].max_x - grid_mbr[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees);
	  gridrows[pfm] = NINT ((grid_mbr[pfm].max_y - grid_mbr[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees);


	  row_filter[pfm] = gridrows[pfm] - FILTER;
	  col_filter[pfm] = gridcols[pfm] - FILTER;



          misc->statusProg->setRange (0, gridrows[pfm]);
	  progText = tr (" Reading PFM data from ") +
            QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm") + " ";
          misc->statusProgLabel->setText (progText);
          misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
          misc->statusProgLabel->setPalette (misc->statusProgPalette);
          misc->statusProg->setTextVisible (TRUE);
          qApp->processEvents();


	  half_x[pfm] = misc->abe_share->open_args[pfm].head.x_bin_size_degrees * 0.5;
	  half_y[pfm] = misc->abe_share->open_args[pfm].head.y_bin_size_degrees * 0.5;


	  //  Get the data that falls in the MBR from the current PFM and save it to memory.

	  for (NV_INT32 i = 0 ; i < gridrows[pfm] ; i++)
	    {
	      //  Compute the latitude of the center of the bin.

	      NV_F64_COORD2 xy;
	      xy.y = grid_mbr[pfm].min_y + i * misc->abe_share->open_args[pfm].head.y_bin_size_degrees + half_y[pfm];

	      misc->statusProg->setValue (i);
	      qApp->processEvents();

	      for (NV_INT32 j = 0 ; j < gridcols[pfm] ; j++)
		{
		  //  Compute the longitude of the center of the bin.

		  xy.x = grid_mbr[pfm].min_x + j * misc->abe_share->open_args[pfm].head.x_bin_size_degrees + half_x[pfm];


		  //  Get the PFM coordinates of the center of the bin

		  compute_index_ptr (xy, &coord, &misc->abe_share->open_args[pfm].head);


		  //  Make sure that we are inside of the PFM

		  if (coord.x >= 0 && coord.x < misc->abe_share->open_args[pfm].head.bin_width &&
		      coord.y >= 0 && coord.y < misc->abe_share->open_args[pfm].head.bin_height)
		    {
		      read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


		      //  If we have soundings in the bin, go get them

		      if (bin.num_soundings)
			{
			  if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
			    {
			      NV_BOOL found = NVFalse;
			      for (NV_INT32 k = 0 ; k < recnum ; k++)
				{
				  switch (misc->average_type[pfm])
				    {
				      //  Use every point (this is normally the case).

				    case 0:
				    case 2:
				    case 5:
				      if (!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
					add_point (&xyz_array, depth[k].xyz, &out_count);
				      break;


				      //  Use the minimum value in the bin.

				    case 1:
				    case 4:
				      if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) && 
					  fabs (depth[k].xyz.z - bin.min_filtered_depth) < EPS)
					{
					  add_point (&xyz_array, depth[k].xyz, &out_count);
					  found = NVTrue;
					}
				      break;
    

				      //  Use the maximum value in the bin.
    
				    case 3:
				    case 6:
				      if ((!(depth[k].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE))) && 
					  fabs (depth[k].xyz.z - bin.max_filtered_depth) < EPS)
					{
					  add_point (&xyz_array, depth[k].xyz, &out_count);
					  found = NVTrue;
					}
				      break;
				    }
				  if (found) break;
				}
			      free (depth);
			    }
			}
		      else
			{
			  //  If there was no depth data in the border area (filter size) we want to load previously
			  //  interpolated (MISP/SURF gridded) data in order to smoothly edge match to existing interpolated data
			  //  outside of the MBR.

			  if (bin.validity & PFM_INTERPOLATED)
			    {
                              if (i < FILTER || j < FILTER || i >= row_filter[pfm] || j >= col_filter[pfm])
                                {
                                  xyz.x = bin.xy.x;
                                  xyz.y = bin.xy.y;
                                  xyz.z = bin.avg_filtered_depth;
                                  add_point (&xyz_array, xyz, &out_count);
                                }
			    }
			}
		    }
		}
	    }
	}
    }


  //  Don't process if we found no points.

  if (!out_count) return;


  //  We have all of the input data (from any visible PFM) in memory so now we run MISP or SURF for each PFM layer that 
  //  has had MISP or SURF run on it before (average_type > 0, this is checked in pfmView.cpp).

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      //  Only deal with those that are being displayed and have been gridded.

      if (misc->abe_share->display_pfm[pfm] && misc->average_type[pfm])
	{
	  //  We're going to let MISP/SURF handle everything in zero based units of the bin size.  That is, we subtract off the
	  //  west lon from longitudes then divide by the grid size in the X direction.  We do the same with the latitude using
	  //  the south latitude.  This will give us values that range from 0.0 to gridcols[pfm] in longitude and 0.0 to gridrows[pfm]
	  //  in latitude.

	  NV_F64_XYMBR new_mbr;
	  new_mbr.min_x = 0.0;
	  new_mbr.min_y = 0.0;
	  new_mbr.max_x = (NV_FLOAT64) gridcols[pfm];
	  new_mbr.max_y = (NV_FLOAT64) gridrows[pfm];


	  //  Initialize the MISP engine.

          if (options->misp_force_original) misp_weight = -misp_weight;
          misp_init (1.0, 1.0, 0.05, 4, 20.0, 20, 999999.0, -999999.0, misp_weight, new_mbr);

          progText = tr (" Loading data into MISP for ") +
            QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm") + " ";


          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, out_count);
	  qApp->processEvents();


	  //  Don't allow the progress bar to slow things down too much.

	  NV_INT32 inc = out_count / 20;
          if (!inc) inc = 1;


	  for (NV_INT32 i = 0 ; i < out_count ; i++)
	    {
	      if (!(out_count % inc))
		{
		  misc->statusProg->setValue (i);
		  qApp->processEvents();
		}

	      xyz.x = (xyz_array[i].x - grid_mbr[pfm].min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
	      xyz.y = (xyz_array[i].y - grid_mbr[pfm].min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
	      xyz.z = xyz_array[i].z;
              misp_load (xyz);
	    }


          progText = tr (" Computing MISP surface for ") +
            QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm") + " ";


          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, 0);
          misc->statusProg->setValue (-1);
          qApp->processEvents ();


          //  We're starting the grid processing concurrently using a thread.  Note that we're using the Qt::DirectConnection type
          //  for the signal/slot connections.  This causes all of the signals emitted from the thread to be serviced immediately.
          //  Why are we running misp_proc in a thread???  Because it's the only way to get the stupid progress bar to update so
          //  that the user will know that the damn program is still running.  Sheesh!

          complete = NVFalse;
          connect (&grid_thread, SIGNAL (completed ()), this, SLOT (slotGridCompleted ()), Qt::DirectConnection);

          grid_thread.grid (misc->average_type[pfm]);


          //  We can't move on until the thread is complete but we want to keep our progress bar updated.  This is a bit tricky 
          //  because you can't update the progress bar from within slots connected to thread signals.  Those slots are considered part
          //  of the mispThread and not part of the GUI thread.  When the thread is finished we move on to the retrieval step.

          while (!complete)
            {
#ifdef NVWIN3X
              Sleep (50);
#else
              usleep (50000);
#endif

              qApp->processEvents ();
            }


          progText = tr (" Retrieving MISP data for ") +
            QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm") + " ";

          misc->statusProgLabel->setText (progText);
	  misc->statusProg->setRange (0, gridrows[pfm]);
	  qApp->processEvents();


          //  Allocating one more than gridcols[pfm] due to constraints of old chrtr (see comments in misp_funcs.c)

          NV_FLOAT32 *array = (NV_FLOAT32 *) malloc ((gridcols[pfm] + 1) * sizeof (NV_FLOAT32));

          if (array == NULL)
            {
              perror ("Allocating array in remisp");
              exit (-1);
            }


          //  This is where we stuff the new interpolated surface back in to the PFM.

          for (NV_INT32 i = 0 ; i < gridrows[pfm] ; i++)
            {
              misc->statusProg->setValue (i);
              qApp->processEvents();


              if (!misp_rtrv (array)) break;


              //  Only use data that aren't in the filter border

              if (i >= FILTER && i <= row_filter[pfm])
                {
                  //  Compute the latitude of the center of the bin.

                  NV_F64_COORD2 xy;
                  xy.y = grid_mbr[pfm].min_y + i * misc->abe_share->open_args[pfm].head.y_bin_size_degrees + half_y[pfm];


                  for (NV_INT32 j = 0 ; j < gridcols[pfm] ; j++)
                    {
                      //  Only use data that aren't in the filter border

                      if (j >= FILTER && j <= col_filter[pfm])
                        {
                          //  Compute the longitude of the center of the bin.

                          xy.x = grid_mbr[pfm].min_x + j * misc->abe_share->open_args[pfm].head.x_bin_size_degrees + half_x[pfm];


                          //  Make sure we're inside the PFM bounds.  Note that we're not using the MBR.  The reason
                          //  being that the PFM bounds may be a polygon.

                          if (bin_inside_ptr (&misc->abe_share->open_args[pfm].head, xy))
                            {
                              NV_FLOAT32 value = array[j];


                              //  If we exceeded the PFM limits we have to set it to the null depth.

                              if (array[j] > misc->abe_share->open_args[pfm].max_depth ||
                                  array[j] < -misc->abe_share->open_args[pfm].offset)
                                value = misc->abe_share->open_args[pfm].head.null_depth;


                              //  Get the PFM coordinates of the center of the bin

                              compute_index_ptr (xy, &coord, &misc->abe_share->open_args[pfm].head);


                              //  Read the record.

                              read_bin_record_index (misc->pfm_handle[pfm], coord, &bin);


                              //  This is a special case.  We don't want to replace land masked bins unless the land mask point
                              //  has been deleted.

                              if (!(bin.validity & PFM_DATA) && (!land_mask_flag || !(bin.validity & PFM_USER_05)))
                                {
                                  //  Mark the record as interpolated.

                                  bin.validity |= PFM_INTERPOLATED;


                                  //  If there was a land mask point in this bin and it has been deleted, unset the 
                                  //  PFM_USER_05 flag.

                                  if (bin.validity & PFM_USER_05) bin.validity &= ~PFM_USER_05;


                                  //  Write the record back out.

                                  bin.avg_filtered_depth = value;
                                  write_bin_record_index (misc->pfm_handle[pfm], &bin);
                                }
                              else
                                {
                                  if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
                                    {
                                      NV_BOOL found = NVFalse;
                                      for (NV_INT32 k = 0 ; k < recnum ; k++)
                                        {
                                          if (depth[k].validity & PFM_DATA)
                                            {
                                              found = NVTrue;
                                              depth[k].xyz.z = bin.avg_filtered_depth;

                                              update_depth_record_index (misc->pfm_handle[pfm], &depth[k]);
                                            }
                                        }


                                      //  This is a cell with a hand-drawn contour in it.

                                      if (found)
                                        {
                                          bin.min_filtered_depth = bin.max_filtered_depth = bin.avg_filtered_depth = bin.min_depth =
                                            bin.max_depth = bin.avg_depth = value;


                                          //  Write the bin record back out.

                                          write_bin_record_index (misc->pfm_handle[pfm], &bin);
                                        }


                                      //  This is a normal cell.

                                      else
                                        {
                                          //  Write the record back out if we have misp_replace_all set.

                                          if (options->misp_replace_all)
                                            {
                                              bin.avg_filtered_depth = value;
                                              write_bin_record_index (misc->pfm_handle[pfm], &bin);
                                            }
                                        }

                                      free (depth);
                                    }
                                }
                            }
                        }
                    }
                }
            }

          free (array);
        }
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();


  free (xyz_array);
}



remisp::~remisp ()
{
}



void 
remisp::add_point (NV_F64_COORD3 **xyz_array, NV_F64_COORD3 xyz, NV_INT32 *count)
{
  *xyz_array = (NV_F64_COORD3 *) realloc (*xyz_array, (*count + 1) * sizeof (NV_F64_COORD3));
  if (*xyz_array == NULL)
    {
      perror (tr ("Allocating xyz_array in remisp.").toAscii ());
      exit (-1);
    }


  (*xyz_array)[*count] = xyz;

  (*count)++;
}



void remisp::slotGridCompleted ()
{
  complete = NVTrue;
}
