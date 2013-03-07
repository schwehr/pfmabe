
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



#include "pfmEdit3D.hpp"


//!  Computes the X, Y, and Z bounds of the displayed data.

NV_BOOL compute_bounds (nvMapGL *map, OPTIONS *options, MISC *misc, NV_INT32 *viewable_count, NV_FLOAT32 *attr_min, NV_FLOAT32 *attr_max)
{
  static NV_F64_XYMBC prev_bounds = {-999.0, -999.0, -999.0, -999.0, CHRTRNULL, -CHRTRNULL};


  //  Compute the Z range of the data.

  *viewable_count = 0;
  *attr_min = CHRTRNULL;
  *attr_max = -CHRTRNULL;


  //  Compute the min and max values for defining the 3D space and optionally for coloring.

  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ;  i++)
    {
      //  If we are displaying and editing only a single line, only get those points that are in that line.

      if (!misc->num_lines || check_line (misc, misc->data[i].line))
        {
          //  We're not checking for masking here because we don't want the window to rescale or recolor if we mask.
          //  We also NEVER check for slicing here for the same reason.  The relevant options are:
          //                                                                                                 mask:--\/                          slice:---\/

          if (!check_bounds (options, misc, misc->data[i].x, misc->data[i].y, misc->data[i].z, misc->data[i].val, NVFalse, misc->data[i].pfm, NVTrue, NVFalse))
            {
              (*viewable_count)++;


              misc->bounds.min_x = qMin (misc->bounds.min_x, misc->data[i].x);
              misc->bounds.max_x = qMax (misc->bounds.max_x, misc->data[i].x);

              misc->bounds.min_y = qMin (misc->bounds.min_y, misc->data[i].y);
              misc->bounds.max_y = qMax (misc->bounds.max_y, misc->data[i].y);


              //  Don't use null data for max and min Z computation.

              if (misc->data[i].z < misc->null_val[misc->data[i].pfm])
                {
                  misc->bounds.min_z = qMin (misc->bounds.min_z, (NV_FLOAT64) -misc->data[i].z);
                  misc->bounds.max_z = qMax (misc->bounds.max_z, (NV_FLOAT64) -misc->data[i].z);
                }



              //  Compute the attribute min/max if we are coloring by horizontal error, vertical error, or attribute.

	      if (options->color_index == 2)
		{
                  if (misc->data[i].herr != 0.0)
                    {
                      *attr_min = qMin (misc->data[i].herr, *attr_min);
                      *attr_max = qMax (misc->data[i].herr, *attr_max);
                    }
		}
	      else if (options->color_index == 3)
		{
                  if (misc->data[i].verr != 0.0)
                    {
                      *attr_min = qMin (misc->data[i].verr, *attr_min);
                      *attr_max = qMax (misc->data[i].verr, *attr_max);
                    }
		}
	      else if (options->color_index >= PRE_ATTR && options->color_index < PRE_ATTR + 10)
		{
                  *attr_min = qMin (misc->data[i].attr[options->color_index - PRE_ATTR], *attr_min);
                  *attr_max = qMax (misc->data[i].attr[options->color_index - PRE_ATTR], *attr_max);
                }
            }
        }
    }


  //  Make sure we don't get smaller than our minimum allowable Z window size.

  NV_FLOAT32 win_size = options->min_window_size;
  switch (options->iho_min_window)
    {
    case 0:
      win_size = options->min_window_size;
      break;

    case 1:
      win_size = 0.25 + 0.0075 * misc->bounds.min_z;
      break;

    case 2:
      win_size = 0.5 + 0.013 * misc->bounds.min_z;
      break;

    case 3:
      win_size = 1.0 + 0.023 * misc->bounds.min_z;
      break;
    }
  map->setMinZExtents (win_size);


  if (misc->bounds.min_x != prev_bounds.min_x || misc->bounds.min_y != prev_bounds.min_y || misc->bounds.min_z != prev_bounds.min_z ||
      misc->bounds.max_x != prev_bounds.max_x || misc->bounds.max_y != prev_bounds.max_y || misc->bounds.max_z != prev_bounds.max_z)
    {
      prev_bounds = misc->aspect_bounds = misc->bounds;


      //  Assuming that the bins are approximately square in terms of distance we want to try to maintain aspect ratio
      //  when we edit a long narrow area.

      NV_FLOAT64 x_bins = (misc->aspect_bounds.max_x - misc->aspect_bounds.min_x) / misc->abe_share->open_args[0].head.x_bin_size_degrees;
      NV_FLOAT64 y_bins = (misc->aspect_bounds.max_y - misc->aspect_bounds.min_y) / misc->abe_share->open_args[0].head.y_bin_size_degrees;

      if (x_bins > y_bins)
        {
          NV_FLOAT64 center = misc->aspect_bounds.min_y + (misc->aspect_bounds.max_y - misc->aspect_bounds.min_y) / 2.0;

          misc->aspect_bounds.min_y = center - (x_bins / 2.0) * misc->abe_share->open_args[0].head.y_bin_size_degrees;
          misc->aspect_bounds.max_y = center + (x_bins / 2.0) * misc->abe_share->open_args[0].head.y_bin_size_degrees;
        }
      else
        {
          NV_FLOAT64 center = misc->aspect_bounds.min_x + (misc->aspect_bounds.max_x - misc->aspect_bounds.min_x) / 2.0;

          misc->aspect_bounds.min_x = center - (y_bins / 2.0) * misc->abe_share->open_args[0].head.x_bin_size_degrees;
          misc->aspect_bounds.max_x = center + (y_bins / 2.0) * misc->abe_share->open_args[0].head.x_bin_size_degrees;
        }

      return (NVTrue);
    }

  return (NVFalse);
}
