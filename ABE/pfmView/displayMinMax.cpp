
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



/*!
  This function places markers on the minimum, maximum, and max standard deviation value in the displayed area.
  The actual values used are computed in compute_layer_min_max.cpp which is called from paint_surface.cpp.
*/

void displayMinMax (nvMap *map, OPTIONS *options, MISC *misc)
{
  NV_FLOAT64          x[3], y[3], dz = 0.0;
  NV_INT32            cx, cy, cz, px[4], py[4];


  NV_INT32 min_pfm = misc->displayed_area_min_pfm;
  NV_INT32 max_pfm = misc->displayed_area_max_pfm;
  NV_INT32 std_pfm = misc->displayed_area_std_pfm;

  NV_I32_COORD2 min_coord = misc->displayed_area_min_coord;
  NV_I32_COORD2 max_coord = misc->displayed_area_max_coord;
  NV_I32_COORD2 std_coord = misc->displayed_area_std_coord;


  //  Minimum value.

  y[0] = misc->displayed_area[min_pfm].min_y + ((NV_FLOAT64) (min_coord.y - misc->displayed_area_row[min_pfm]) + 0.5) * 
    misc->abe_share->open_args[min_pfm].head.y_bin_size_degrees;
  x[0] = misc->displayed_area[min_pfm].min_x + ((NV_FLOAT64) (min_coord.x - misc->displayed_area_column[min_pfm])+ 0.5) * 
    misc->abe_share->open_args[min_pfm].head.x_bin_size_degrees;


  //  Maximum value.

  y[1] = misc->displayed_area[max_pfm].min_y + ((NV_FLOAT64) (max_coord.y - misc->displayed_area_row[max_pfm]) + 0.5) * 
    misc->abe_share->open_args[max_pfm].head.y_bin_size_degrees;
  x[1] = misc->displayed_area[max_pfm].min_x + ((NV_FLOAT64) (max_coord.x - misc->displayed_area_column[max_pfm]) + 0.5) * 
    misc->abe_share->open_args[max_pfm].head.x_bin_size_degrees;


  //  Maximum standard deviation.

  y[2] = misc->displayed_area[std_pfm].min_y + ((NV_FLOAT64) (std_coord.y - misc->displayed_area_row[std_pfm]) + 0.5) * 
    misc->abe_share->open_args[std_pfm].head.y_bin_size_degrees;
  x[2] = misc->displayed_area[std_pfm].min_x + ((NV_FLOAT64) (std_coord.x - misc->displayed_area_column[std_pfm]) + 0.5) * 
    misc->abe_share->open_args[std_pfm].head.x_bin_size_degrees;


  map->fillCircle (x[0], y[0], 15, 0.0, 360.0, options->contour_color, NVTrue);
  map->drawCircle (x[0], y[0], 16, 0.0, 360.0, 1, Qt::black, Qt::SolidLine, NVTrue);

  map->fillRectangle (x[1], y[1], 15, 15, options->contour_color, NVTrue);
  map->drawRectangle (x[1], y[1], 15, 15, Qt::black, 1, Qt::SolidLine, NVTrue);

  map->map_to_screen (1, &x[2], &y[2], &dz, &cx, &cy, &cz);
  px[0] = cx;
  py[0] = cy - 7;
  px[1] = cx - 7;
  py[1] = cy;
  px[2] = cx;
  py[2] = cy + 7;
  px[3] = cx + 7;
  py[3] = cy;
  map->fillPolygon (4, px, py, options->contour_color, NVTrue);
  map->drawPolygon (4, px, py, Qt::black, 1, NVTrue, Qt::SolidLine, NVTrue);
}
