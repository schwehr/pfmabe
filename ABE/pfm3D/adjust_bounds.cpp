#include "pfm3D.hpp"

void adjust_bounds (MISC *misc, NV_INT32 pfm)
{
  //  Adjust input bounds to nearest grid point.

  misc->displayed_area.min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
    (NINT ((misc->abe_share->viewer_displayed_area.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
	   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
  misc->displayed_area.max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
    (NINT ((misc->abe_share->viewer_displayed_area.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
	   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) * 
    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
  misc->displayed_area.min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
    (NINT ((misc->abe_share->viewer_displayed_area.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
	   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
  misc->displayed_area.max_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
    (NINT ((misc->abe_share->viewer_displayed_area.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
	   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


  misc->displayed_area.min_y = qMax (misc->displayed_area.min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
  misc->displayed_area.max_y = qMin (misc->displayed_area.max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
  misc->displayed_area.min_x = qMax (misc->displayed_area.min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
  misc->displayed_area.max_x = qMin (misc->displayed_area.max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);


  misc->displayed_area_width = (NINT ((misc->displayed_area.max_x - misc->displayed_area.min_x) / 
				     misc->abe_share->open_args[pfm].head.x_bin_size_degrees));
  misc->displayed_area_height = (NINT ((misc->displayed_area.max_y - misc->displayed_area.min_y) / 
				      misc->abe_share->open_args[pfm].head.y_bin_size_degrees));
  misc->displayed_area_row = NINT ((misc->displayed_area.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) / 
				  misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
  misc->displayed_area_column = NINT ((misc->displayed_area.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) / 
				     misc->abe_share->open_args[pfm].head.x_bin_size_degrees);

  if (misc->displayed_area_column + misc->displayed_area_width > misc->abe_share->open_args[pfm].head.bin_width - 1)
    {
      misc->displayed_area_width = misc->abe_share->open_args[pfm].head.bin_width - misc->displayed_area_column - 1;
      misc->displayed_area.max_x = misc->abe_share->open_args[pfm].head.mbr.max_x;
    }

  if (misc->displayed_area_row + misc->displayed_area_height > misc->abe_share->open_args[pfm].head.bin_height - 1)
    {
      misc->displayed_area_height = misc->abe_share->open_args[pfm].head.bin_height - misc->displayed_area_row - 1;
      misc->displayed_area.max_y = misc->abe_share->open_args[pfm].head.mbr.max_y;
    }
}
