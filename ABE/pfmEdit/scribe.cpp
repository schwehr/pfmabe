
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



#include "pfmEdit.hpp"


/*!
  The ever popular scribe contour drawing package.  Most of the functions called here are in the nvutility library
  because they're pretty much stock functions.  Only the scribe function itself ever really gets modified (based
  on what you're trying to draw contours for and what you want to draw them to).
*/

void scribe (nvMap *map, OPTIONS *options, MISC *misc, NV_FLOAT32 *ar, NV_INT32 ncc, NV_INT32 nrr, NV_FLOAT64 xorig, NV_FLOAT64 yorig)
{
  NV_INT32                index_contour, num_points, i, num_interp, bytes, width;
  NV_FLOAT64              dcontour_x[CONTOUR_POINTS], dcontour_y[CONTOUR_POINTS],
                          dcontour_z[CONTOUR_POINTS], dx, dy, cell_diag_length,
                          segment_length, x[2], y[2], z[2], *xyz_x, *xyz_y, *xyz_z;
  NV_FLOAT32              level, half_gridx, half_gridy, *contour_x, 
                          *contour_y;


  void contourMinMax (NV_FLOAT32, NV_FLOAT32);
  void contourEmphasis (NV_INT32);
  void contourMaxDensity (NV_INT32);
  void contourMaxPoints (NV_INT32);
  void contourLevels (NV_INT32 numLevels, NV_FLOAT32 *);
  NV_INT32 initContour (NV_FLOAT32, NV_INT32, NV_INT32, NV_FLOAT32 *);
  NV_INT32 getContour (NV_FLOAT32 *, NV_INT32 *, NV_INT32 *, NV_FLOAT32 *, NV_FLOAT32 *);
  void smooth_contour (NV_INT32, NV_INT32 *, NV_FLOAT64 *, NV_FLOAT64 *);



  //  Check the smoothing factor range and get the number of interpolation 
  //  points per unsmoothed contour segment

  if (options->smoothing_factor < 0) options->smoothing_factor = 0;
  if (options->smoothing_factor > 10) options->smoothing_factor = 10;


  dcontour_x[0] = xorig + (misc->x_grid_size * ncc) * 0.5;
  dcontour_y[0] = yorig + (misc->y_grid_size * nrr) * 0.5;
  dcontour_x[1] = dcontour_x[0] + misc->x_grid_size;
  dcontour_y[1] = dcontour_y[0] + misc->y_grid_size;

  map->map_to_screen (2, dcontour_x, dcontour_y, dcontour_z, x, y, z);

  dx = x[1] - x[0];
  dy = y[1] - y[0];

  cell_diag_length = sqrt (pow (dx, 2) + pow (dy, 2));

  segment_length = DEFAULT_SEGMENT_LENGTH / options->smoothing_factor;

  if (cell_diag_length > segment_length)
    {
      num_interp = (NV_INT32) ((cell_diag_length / segment_length) + 0.5);
    }
  else
    {
      num_interp = 1;
    }


  /* allocate memory for contour arrays */

  if (options->smoothing_factor > 0)
    {
      bytes = (((num_interp * (CONTOUR_POINTS - 1)) + 1) * sizeof (NV_FLOAT64));
    }
  else
    {
      bytes = (CONTOUR_POINTS * sizeof (NV_FLOAT64));
    }

  contour_x = (NV_FLOAT32 *) malloc (bytes / 2);
  contour_y = (NV_FLOAT32 *) malloc (bytes / 2);
  xyz_x = (NV_FLOAT64 *) malloc (bytes);
  xyz_y = (NV_FLOAT64 *) malloc (bytes);
  if ((xyz_z = (NV_FLOAT64 *) malloc (bytes)) == NULL)
    {
      perror (__FILE__);
      exit (-1);
    }


  //  Set the min and max contours, the index contour interval, the
  //  maximum contour density, and the number of points to be returned
  //  by the package.

  contourMinMax (misc->min_z * options->z_factor + options->z_offset, misc->max_z * options->z_factor + options->z_offset);
  contourMaxDensity (misc->maxd);
  contourMaxPoints (CONTOUR_POINTS);


  //  If the contour interval is set to 0.0 use the user defined levels.

  if (misc->abe_share->cint == 0.0) contourLevels (misc->abe_share->num_levels, misc->abe_share->contour_levels);


  //  Pass the grid array (arranged 1D) to the contouring package.

  initContour (misc->abe_share->cint, nrr, ncc, ar);


  //  Compute half of a grid cell in degrees.

  half_gridx = misc->x_grid_size * 0.5;
  half_gridy = misc->y_grid_size * 0.5;


  //  Get the contours from the package until all are drawn.

  while (getContour (&level, &index_contour, &num_points, contour_x, contour_y))
    {
      //  Convert from grid points (returned contours) to position.
                    
      for (i = 0 ; i < num_points ; i++)
        {
          dcontour_x[i] = xorig + (contour_x[i] * misc->x_grid_size) + half_gridx;
          dcontour_y[i] = yorig + (contour_y[i] * misc->y_grid_size) + half_gridy;
        }


      //  Convert from position to pixels.

      map->map_to_screen (num_points, dcontour_x, dcontour_y, dcontour_z, xyz_x, xyz_y, xyz_z);


      //  smooth out the contour (don't do this sooner for the sake of
      //  efficiency -- smooth in xy plot space)

      if (options->smoothing_factor > 0) smooth_contour (num_interp, &num_points, xyz_x, xyz_y);


      //  If this is the zero contour, thicken the line.

      width = options->contour_width;
      if (level == 0.0)
        {
          switch (width)
            {
            case 1:
              width = 3;
              break;

            case 2:
              width = 4;
              break;

            case 3:
              width = 5;
              break;
            }
        }


      //  Draw the contours.
            
      for (i = 1 ; i < num_points ; i++)
        {
          map->drawLine (NINT (xyz_x[i - 1]), NINT (xyz_y[i - 1]), NINT (xyz_x[i]), NINT (xyz_y[i]),
                         options->contour_color, width, NVFalse, Qt::SolidLine);
        }


      //  Check the event queue to see if the user wants to interrupt the contouring.

      if (qApp->hasPendingEvents ())
        {
          qApp->processEvents ();
          if (misc->drawing_canceled) break;
        }
    }


  free (contour_x);
  free (contour_y);
  free (xyz_x);
  free (xyz_y);
  free (xyz_z);

  map->flush ();
}
