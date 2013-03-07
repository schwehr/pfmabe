
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


//!  Hatchr paints the surface.  I've tried to optimize the painting where possible.

void hatchr (nvMap *map, OPTIONS *options, NV_BOOL clear, NV_F64_XYMBR mbr, NV_F64_XYMBR edit_mbr, NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range,
             NV_FLOAT64 x_bin_size, NV_FLOAT64 y_bin_size, NV_INT32 height, NV_INT32 start_x, NV_INT32 end_x, NV_FLOAT32 ss_null, NV_FLOAT64 cell_size_x,
             NV_FLOAT64 cell_size_y, NV_FLOAT32 *current_row, NV_FLOAT32 *next_row, NV_FLOAT32 *current_attr, NV_U_CHAR *current_flags, NV_U_BYTE alpha,
             NV_INT32 row, NV_INT32 attribute)
{
  NV_FLOAT64          cur_y[2] = {0.0, 0.0}, cur_x[2] = {0.0, 0.0}, y;
  NV_FLOAT32          shade_factor;
  static NV_INT32     i, h_index = 0, s_index = 0, numhues = NUMHUES - 1;
  QColor              temp, prev;
  NV_INT32            ptr;
  NV_BOOL             first;


  y = mbr.min_y + row * y_bin_size;


  //  We added one grid point to clean up the edges (see pfmView.cpp)

  if (!clear && (y < (edit_mbr.min_y - y_bin_size) || y > (edit_mbr.max_y + y_bin_size))) return;


  //  Skip the last row for sunshading.

  if (row >= height) return;


  first = NVTrue;


  cur_y[0] = mbr.min_y + row * y_bin_size;
  cur_y[1] = cur_y[0] + y_bin_size;


  //  Loop for the width of the displayed/edited area.

  for (i = start_x ; i < end_x ; i++)
    {
      //  Set the HSV fill color.

      if (attribute && current_attr != NULL)
	{
          if (options->stoplight)
            {
              if (current_attr[i] > options->stoplight_max_mid)
                {
                  h_index = options->stoplight_max_index;
                }
              else if (current_attr[i] < options->stoplight_min_mid)
                {
                  h_index = options->stoplight_min_index;
                }
              else
                {
                  h_index = options->stoplight_mid_index;
                }
            }
          else
            {
              h_index = (NV_INT32) (numhues - fabs ((current_attr[i] - max_z) / range * numhues));
            }


          //  Check for out of range data.

          if (current_attr[i] < min_z)
            {
              h_index = 0;
            }
          else if (current_attr[i] > max_z)
            {
              h_index = numhues - 1;
            }
	}
      else
	{
          if (options->stoplight)
            {
              if (current_row[i] * options->z_factor + options->z_offset < options->stoplight_min_mid)
                {
                  h_index = options->stoplight_max_index;
                }
              else if (current_row[i] * options->z_factor + options->z_offset > options->stoplight_max_mid)
                {
                  h_index = options->stoplight_min_index;
                }
              else
                {
                  h_index = options->stoplight_mid_index;
                }
            }
          else
            {
              if (options->zero_turnover && min_z < 0.0 && max_z >= 0.0)
                {
                  if (current_row[i] <= 0.0)
                    {
                      h_index = (NV_INT32) (numhues - fabs ((current_row[i] - min_z) / (-min_z) * numhues));
                    }
                  else
                    {
                      h_index = (NV_INT32) (numhues - fabs (current_row[i] / max_z * numhues));
                    }
                }
              else
                {
                  h_index = (NV_INT32) (numhues - fabs ((current_row[i] - min_z) / range * numhues));
                }


              //  Check for out of range data.

              if (current_row[i] < min_z)
                {
                  h_index = numhues - 1;
                }
              else if (current_row[i] > max_z)
                {
                  h_index = 0;
                }
            }
	}


      //  Clear nulls

      if (current_row[i] == ss_null) h_index = -2; 


      //  Highlight if the bin is marked as PFM_USER_??, checked, or multiple coverage.

      ptr = 0;
      if (h_index >= 0 && options->highlight && current_flags != NULL && current_flags[i]) ptr = 1;


      //  Shade the cell.

      s_index = NUMSHADES - 1;
      if (h_index >= 0)
        {
          shade_factor = sunshade_null (current_row[i], next_row[i], next_row[i + 1], ss_null, &options->sunopts, cell_size_x, cell_size_y);


          //  The shade_factor will sometimes come back just slightly larger than 1.0.

          if (shade_factor < 0.0) shade_factor = 0.0;

          if (shade_factor > 1.0) shade_factor = 1.0;

          s_index = (NV_INT32) ((NUMSHADES - 1) * shade_factor + 0.5);
        }


      //  Select the color if not null.

      if (h_index >= 0)
        {
          temp = options->color_array[ptr][h_index][s_index];
	  temp.setAlpha (alpha);
        }
      else
        {
          //  Set to background for null depth.  Really doesn't matter since we plot these transparently anyway.

          temp = options->background_color;
	  temp.setAlpha (0);
        }


      if (prev != temp || i == end_x - 1)
        {

          //  Compute longitude.
    
          if (first) 
            {
              cur_x[0] = mbr.min_x + i * x_bin_size;
              cur_x[1] = cur_x[0];
            }
          else
            {
              cur_x[1] = mbr.min_x + (i + 1) * x_bin_size;
            }


          if (first)
            {
              prev = temp;

              first = NVFalse;
            }
          else
            {
              map->fillRectangle (cur_x[0], cur_y[0], cur_x[1], cur_y[1], prev, NVFalse);

              prev = temp;


              //  Compute x position.

              cur_x[0] = mbr.min_x + i * x_bin_size;
            }
        }
    }
}
