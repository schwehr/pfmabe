#include "areaCheck.hpp"


void hatchr (NV_INT32 k, NV_INT32 file_number, nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT32 *current_row, 
	     NV_FLOAT32 *next_row, NV_INT32 start_row, NV_INT32 row, NV_INT32 cols, NV_FLOAT64 min_z,
	     NV_FLOAT64 max_z, NV_F64_XYMBR mbr)
{
  NV_FLOAT64          cur_y[2] = {0.0, 0.0}, cur_x[2] = {0.0, 0.0};
  NV_FLOAT32          shade_factor;
  static NV_INT32     i, h_index = 0, s_index = 0, numhues = NUMHUES - 1;
  QColor              temp, prev;
  NV_BOOL             first;


  first = NVTrue;


  cur_y[0] = mbr.min_y + (row - start_row) * misc->overlays[k][file_number].y_bin_size_degrees;
  cur_y[1] = cur_y[0] + misc->overlays[k][file_number].y_bin_size_degrees;


  //  Loop for the width of the displayed/edited area.

  for (i = 0 ; i < cols ; i++)
    {
      //  Set the HSV fill color.

      if (min_z < 0.0 && max_z >= 0.0)
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
	  NV_FLOAT32 range = max_z - min_z;
	  if (range == 0.0) range = 1.0;

          h_index = (NV_INT32) (numhues - fabs ((current_row[i] - min_z) / range * numhues));
        }


      //  Clear nulls

      if (current_row[i] >= CHRTRNULL) h_index = -2; 


      //  Select the color if not null.

      if (h_index >= 0)
        {
          shade_factor = sunshade_null (current_row[i], next_row[i], next_row[i + 1], CHRTRNULL, 
                                        &options->sunopts, misc->overlays[k][file_number].x_bin_size_meters,
                                        misc->overlays[k][file_number].y_bin_size_meters);


          //  The shade_factor will sometimes come back just slightly larger than 1.0.

          if (shade_factor < 0.0) shade_factor = 0.0;

          if (shade_factor > 1.0) shade_factor = 1.0;

          s_index = (NV_INT32) ((NUMSHADES - 1) * shade_factor + 0.5);

          temp = misc->color_array[h_index][s_index];
        }
      else
        {
          //  Set to background for null depth.  We don't actually plot these since we've blanked the
          //  area to be redrawn.

          temp = options->background_color;

	  s_index = NUMSHADES - 1;
        }


      if (prev != temp || i == cols - 1)
        {
          //  Compute longitude.
    
          if (first) 
            {
              cur_x[0] = mbr.min_x + i * misc->overlays[k][file_number].x_bin_size_degrees;
              cur_x[1] = cur_x[0];
            }
          else
            {
              cur_x[1] = mbr.min_x + (i + 1) * misc->overlays[k][file_number].x_bin_size_degrees;
            }


          if (first)
            {
              prev = temp;

              first = NVFalse;
            }
          else
            {
              if (prev != options->background_color) map->fillRectangle (cur_x[0], cur_y[0], cur_x[1], cur_y[1], prev, NVFalse);

              prev = temp;


              //  Compute x position.

              cur_x[0] = mbr.min_x + i * misc->overlays[k][file_number].x_bin_size_degrees;
            }
        }
    }
}
