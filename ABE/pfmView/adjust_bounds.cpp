
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


//!  Adjust the displayed area bounds to nearest grid point.

void adjust_bounds (MISC *misc, NV_INT32 pfm)
{
  misc->displayed_area[pfm].min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
    (NINT ((misc->total_displayed_area.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
	   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
  misc->displayed_area[pfm].max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
    (NINT ((misc->total_displayed_area.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
	   misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) * 
    misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
  misc->displayed_area[pfm].min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
    (NINT ((misc->total_displayed_area.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
	   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
  misc->displayed_area[pfm].max_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
    (NINT ((misc->total_displayed_area.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
	   misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) *
    misc->abe_share->open_args[pfm].head.x_bin_size_degrees;


  misc->displayed_area[pfm].min_y = qMax (misc->displayed_area[pfm].min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
  misc->displayed_area[pfm].max_y = qMin (misc->displayed_area[pfm].max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
  misc->displayed_area[pfm].min_x = qMax (misc->displayed_area[pfm].min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
  misc->displayed_area[pfm].max_x = qMin (misc->displayed_area[pfm].max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);


  misc->displayed_area_width[pfm] = (NINT ((misc->displayed_area[pfm].max_x - misc->displayed_area[pfm].min_x) / 
                                           misc->abe_share->open_args[pfm].head.x_bin_size_degrees));
  misc->displayed_area_height[pfm] = (NINT ((misc->displayed_area[pfm].max_y - misc->displayed_area[pfm].min_y) / 
                                            misc->abe_share->open_args[pfm].head.y_bin_size_degrees));
  misc->displayed_area_row[pfm] = NINT ((misc->displayed_area[pfm].min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) / 
                                        misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
  misc->displayed_area_column[pfm] = NINT ((misc->displayed_area[pfm].min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) / 
                                           misc->abe_share->open_args[pfm].head.x_bin_size_degrees);

  if (misc->displayed_area_column[pfm] + misc->displayed_area_width[pfm] > misc->abe_share->open_args[pfm].head.bin_width)
    {
      misc->displayed_area_width[pfm] = misc->abe_share->open_args[pfm].head.bin_width - misc->displayed_area_column[pfm];
      misc->displayed_area[pfm].max_x = misc->abe_share->open_args[pfm].head.mbr.max_x;
    }

  if (misc->displayed_area_row[pfm] + misc->displayed_area_height[pfm] > misc->abe_share->open_args[pfm].head.bin_height)
    {
      misc->displayed_area_height[pfm] = misc->abe_share->open_args[pfm].head.bin_height - misc->displayed_area_row[pfm];
      misc->displayed_area[pfm].max_y = misc->abe_share->open_args[pfm].head.mbr.max_y;
    }
}

