
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


//!  Computes the minimum and maximum X, Y, and Z for all of the displayed layers (PFM files).

NV_BOOL compute_layer_min_max (MISC *misc, OPTIONS *options)
{
  void adjust_bounds (MISC *misc, NV_INT32 pfm);
  void setScale (NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range, NV_INT32 attribute, MISC *misc, OPTIONS *options, NV_BOOL min_lock, NV_BOOL max_lock);



  misc->displayed_area_min = 999999999.0;
  misc->displayed_area_max = -999999999.0;
  misc->displayed_area_std = -999999999.0;
  misc->displayed_area_attr_min = 999999999.0;
  misc->displayed_area_attr_max = -999999999.0;


  NV_INT32 attribute = 0;
  NV_BOOL min_lock = NVFalse, max_lock = NVFalse;


  //  Pre-read the data to get the min and max (since most disk systems cache this isn't much of a performance hit).
  //  The loop runs from max to min so that we end up with the final bounds defined by PFM layer 0.

  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      //  Don't use it to compute min/max if we're not displaying it.

      if (misc->abe_share->display_pfm[pfm])
        {
          //  Adjust input bounds to nearest grid point.

          adjust_bounds (misc, pfm);


	  //  If the width or height is 0 or negative we have asked for an area outside of the PFM's MBR so we don't want to 
	  //  do anything.

	  if (misc->displayed_area_width[pfm] > 0 && misc->displayed_area_height[pfm] > 0)
	    {
	      //  We only want to do attributes if we're coloring by number or stddev, or if we're on the top level.

	      if (misc->color_by_attribute == 1 || misc->color_by_attribute == 2 || (!pfm && misc->color_by_attribute)) 
                attribute = misc->color_by_attribute;


	      //  Allocate the needed arrays.

	      BIN_RECORD *current_record = (BIN_RECORD *) calloc (misc->displayed_area_width[pfm], sizeof (BIN_RECORD));
	      if (current_record == NULL)
                {
                  perror (pfmView::tr ("Allocating current_record in compute_layer_min_max").toAscii ());
                  exit (-1);
                }

	      misc->current_row = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
	      if (misc->current_row == NULL)
                {
                  perror (pfmView::tr ("Allocating current_row in compute_layer_min_max").toAscii ());
                  exit (-1);
                }

	      if (attribute)
                {
                  misc->current_attr = (NV_FLOAT32 *) calloc (misc->displayed_area_width[pfm], sizeof (NV_FLOAT32));
                  if (misc->current_attr == NULL)
                    {
                      perror (pfmView::tr ("Allocating current_attr in compute_layer_min_max").toAscii ());
                      exit (-1);
                    }
                }

	      misc->current_flags = (NV_U_CHAR *) calloc (misc->displayed_area_width[pfm], sizeof (NV_CHAR));
              if (misc->current_flags == NULL)
                {
                  perror (pfmView::tr ("Allocating current_flags in compute_layer_min_max").toAscii ());
                  exit (-1);
                }


              misc->statusProg->setRange (0, misc->displayed_area_height[pfm]);
              QString title = pfmView::tr (" Loading %1 of %2 : ").arg (misc->abe_share->pfm_count - pfm).arg (misc->abe_share->pfm_count) +
                QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName () + " ";
              misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
              misc->statusProgLabel->setPalette (misc->statusProgPalette);
              misc->statusProgLabel->setText (title);
              misc->statusProg->setTextVisible (TRUE);
              qApp->processEvents();


              //  We only want to update the progress bar at about 20% increments.  This makes things go
              //  marginally faster.

              NV_INT32 prog_inc = misc->displayed_area_height[pfm] / 5;
              if (!prog_inc) prog_inc = 1;


              for (NV_INT32 i = 0 ; i < misc->displayed_area_height[pfm] ; i++)
                {
                  if (!(i % prog_inc))
                    {
                      misc->statusProg->setValue (i);
                      qApp->processEvents();
                    }


                  read_bin_row (misc->pfm_handle[pfm], misc->displayed_area_width[pfm], misc->displayed_area_row[pfm] + i, 
                                misc->displayed_area_column[pfm], current_record);


                  loadArrays (misc->abe_share->layer_type, misc->displayed_area_width[pfm], current_record, misc->current_row,
                              misc->current_attr, attribute, misc->current_flags, H_NONE, options->h_count,
                              misc->pfm_handle[pfm], misc->abe_share->open_args[pfm], 0.0, misc->surface_val);


                  for (NV_INT32 j = 0 ; j < misc->displayed_area_width[pfm] ; j++)
                    {
                      //  Allow AVERAGE_FILTERED_DEPTH in order to get PFM_INTERPOLATED values (see loadArrays)

                      if (current_record[j].num_soundings || misc->abe_share->layer_type == AVERAGE_FILTERED_DEPTH)
                        {
                          if (misc->current_row[j] < misc->abe_share->open_args[pfm].head.null_depth)
                            {
                              //  Since the user may have turned on display of minimum, maximum, and max standard deviation
                              //  we're going to save these so we don't have to read through the data again to find the 
                              //  locations.

                              if (misc->current_row[j] < misc->displayed_area_min)
                                {
                                  misc->displayed_area_min = misc->current_row[j];
                                  misc->displayed_area_min_coord.x = misc->displayed_area_column[pfm] + j;
                                  misc->displayed_area_min_coord.y = misc->displayed_area_row[pfm] + i;
                                  misc->displayed_area_min_pfm = pfm;
                                }

                              if (misc->current_row[j] > misc->displayed_area_max) 
                                {
                                  misc->displayed_area_max = misc->current_row[j];
                                  misc->displayed_area_max_coord.x = misc->displayed_area_column[pfm] + j;
                                  misc->displayed_area_max_coord.y = misc->displayed_area_row[pfm] + i;
                                  misc->displayed_area_max_pfm = pfm;
                                }

                              if (current_record[j].standard_dev > misc->displayed_area_std) 
                                {
                                  misc->displayed_area_std = current_record[j].standard_dev;
                                  misc->displayed_area_std_coord.x = misc->displayed_area_column[pfm] + j;
                                  misc->displayed_area_std_coord.y = misc->displayed_area_row[pfm] + i;
                                  misc->displayed_area_std_pfm = pfm;
                                }


                              if (attribute)
                                {
                                  misc->displayed_area_attr_min = qMin (misc->current_attr[j], misc->displayed_area_attr_min);
                                  misc->displayed_area_attr_max = qMax (misc->current_attr[j], misc->displayed_area_attr_max);
                                }
                            }
                        }
                    }
                }


              if (attribute)
                {
                  misc->color_min = misc->displayed_area_attr_min;
                  misc->color_max = misc->displayed_area_attr_max;
                }
              else
                {
                  misc->color_min = misc->displayed_area_min;
                  misc->color_max = misc->displayed_area_max;
                }


              misc->statusProg->setValue (misc->displayed_area_height[pfm]);
              qApp->processEvents();


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


	      //  Free allocated memory.

	      free (current_record);
	      free (misc->current_row);
	      if (attribute) free (misc->current_attr);
	      free (misc->current_flags);
            }
        }
    }


  //  Set the scale colors for the current range.

  if (attribute)
    {
      setScale (misc->displayed_area_attr_min, misc->displayed_area_attr_max, misc->displayed_area_attr_range, attribute, misc, options, min_lock, max_lock);
    }
  else
    {
      setScale (misc->displayed_area_min, misc->displayed_area_max, misc->displayed_area_range, attribute, misc, options, min_lock, max_lock);
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents ();

  return (NVTrue);
}
