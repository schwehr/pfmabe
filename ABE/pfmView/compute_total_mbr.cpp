
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



#include "pfmViewDef.hpp"


//!  Computes the minimum bounding rectangle (MBR) for all of the PFM areas combined.

void compute_total_mbr (MISC *misc)
{
  //  Check for dateline crossing.

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (misc->abe_share->open_args[pfm].head.mbr.max_x > 180.0 && misc->abe_share->open_args[pfm].head.mbr.min_x < 180.0)
        {
          misc->dateline = NVTrue;
          break;
        }
    }


  misc->total_mbr.min_y = 999.0;
  misc->total_mbr.min_x = 999.0;
  misc->total_mbr.max_y = -999.0;
  misc->total_mbr.max_x = -999.0;

  NV_INT32 max_x_bin_pfm = 0, max_y_bin_pfm = 0;
  NV_FLOAT64 max_x_bin = 0.0, max_y_bin = 0.0;

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (misc->dateline && misc->abe_share->open_args[pfm].head.mbr.max_x < 0.0) misc->abe_share->open_args[pfm].head.mbr.max_x += 360.0;

      misc->total_mbr.min_y = qMin (misc->total_mbr.min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
      misc->total_mbr.min_x = qMin (misc->total_mbr.min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
      misc->total_mbr.max_y = qMax (misc->total_mbr.max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
      misc->total_mbr.max_x = qMax (misc->total_mbr.max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);

      if (max_x_bin < misc->abe_share->open_args[pfm].head.x_bin_size_degrees)
	{
	  max_x_bin_pfm = pfm;
	  max_x_bin = misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
	}

      if (max_y_bin < misc->abe_share->open_args[pfm].head.y_bin_size_degrees)
	{
	  max_y_bin_pfm = pfm;
	  max_y_bin = misc->abe_share->open_args[pfm].head.y_bin_size_degrees;
	}
    }


  //  Add 5 grid points as a border around the total MBR.  This gives us a bit of leeway on the borders.

  misc->total_mbr.min_y -= 5.0 * misc->abe_share->open_args[max_y_bin_pfm].head.y_bin_size_degrees;
  misc->total_mbr.min_x -= 5.0 * misc->abe_share->open_args[max_x_bin_pfm].head.x_bin_size_degrees;
  misc->total_mbr.max_y += 5.0 * misc->abe_share->open_args[max_y_bin_pfm].head.y_bin_size_degrees;
  misc->total_mbr.max_x += 5.0 * misc->abe_share->open_args[max_x_bin_pfm].head.x_bin_size_degrees;


  //  Adjust the displayed area if it's outside the total MBR bounds.

  if (misc->total_displayed_area.min_y < misc->total_mbr.min_y) misc->total_displayed_area.min_y = misc->total_mbr.min_y;
  if (misc->total_displayed_area.min_x < misc->total_mbr.min_x) misc->total_displayed_area.min_x = misc->total_mbr.min_x;
  if (misc->total_displayed_area.max_y > misc->total_mbr.max_y) misc->total_displayed_area.max_y = misc->total_mbr.max_y;
  if (misc->total_displayed_area.max_x > misc->total_mbr.max_x) misc->total_displayed_area.max_x = misc->total_mbr.max_x;
}
