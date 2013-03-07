
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
#include "libshp/shapefil.h"

#define  CONTOUR_LABEL_HEIGHT               12  //  in pixels
#define  HALF_CONTOUR_LABEL_HEIGHT          CONTOUR_LABEL_HEIGHT / 2


/*!
  The ever popular scribe contour drawing package.  Most of the functions called here are in the nvutility library
  because they're pretty much stock functions.  Only the scribe function itself ever really gets modified (based
  on what you're trying to draw contours for and what you want to draw them to).
*/

void scribe (nvMap *map, OPTIONS *options, MISC *misc, NV_INT32 pfm, NV_FLOAT32 *ar, NV_FLOAT32 highlight)
{
  NV_INT32                index_contour, num_points, num_interp, bytes, width, ncc, nrr, digits, cont_index_1,
                          cont_index_2, win_index_1 = 0, win_index_2, label_len_chars, label_x = 0, label_y = 0;
  NV_FLOAT64              dcontour_x[CONTOUR_POINTS], dcontour_y[CONTOUR_POINTS], dcontour_z[CONTOUR_POINTS], dx, dy,
                          cell_diag_length, xorig, yorig, segment_length, x[2], y[2], z[2], *xyz_x, *xyz_y, *xyz_z, dist;
  NV_FLOAT32              level, half_gridx, half_gridy, *contour_x, *contour_y, save_draw_contour_level = 0.0;
  QString                 label = "";
  NV_BOOL                 capture = NVFalse;



  void contourMinMax (NV_FLOAT32, NV_FLOAT32);
  void contourEmphasis (NV_INT32);
  void contourMaxDensity (NV_INT32);
  void contourMaxPoints (NV_INT32);
  void contourLevels (NV_INT32 numLevels, NV_FLOAT32 *);
  NV_INT32 initContour (NV_FLOAT32, NV_INT32, NV_INT32, NV_FLOAT32 *);
  NV_INT32 getContour (NV_FLOAT32 *, NV_INT32 *, NV_INT32 *, NV_FLOAT32 *, NV_FLOAT32 *);
  void smooth_contour (NV_INT32, NV_INT32 *, NV_FLOAT64 *, NV_FLOAT64 *);
  void writeContour (MISC *misc, NV_FLOAT32 z_factor, NV_FLOAT32 z_offset, NV_INT32 count, NV_FLOAT64 *cur_x,
                     NV_FLOAT64 *cur_y);


  //  If we're capturing the contours we need to save the original draw contour level.  We also 
  //  need to lie to the writeContour function and tell it that the contour is in ALL pfm's.
  //  writeContour will sort it out anyway.

  if (misc->function == GRAB_CONTOUR && misc->poly_count)
    {
      save_draw_contour_level = misc->draw_contour_level;
      for (NV_INT32 gpfm = 0 ; gpfm < misc->abe_share->pfm_count ; gpfm++) misc->contour_in_pfm[gpfm] = NVTrue;
      capture = NVTrue;
    }


  ncc = misc->displayed_area_width[pfm];
  nrr = misc->displayed_area_height[pfm];
  xorig = misc->displayed_area[pfm].min_x;
  yorig = misc->displayed_area[pfm].min_y + (misc->hatchr_start_y * misc->abe_share->open_args[pfm].head.y_bin_size_degrees);


  //  Check the smoothing factor range and get the number of interpolation 
  //  points per unsmoothed contour segment

  if (options->smoothing_factor < 0) options->smoothing_factor = 0;
  if (options->smoothing_factor > 10) options->smoothing_factor = 10;


  dcontour_x[0] = xorig + (misc->abe_share->open_args[pfm].head.x_bin_size_degrees * ncc) * 0.5;
  dcontour_y[0] = yorig + (misc->abe_share->open_args[pfm].head.y_bin_size_degrees * nrr) * 0.5;
  dcontour_x[1] = dcontour_x[0] + misc->abe_share->open_args[pfm].head.x_bin_size_degrees;
  dcontour_y[1] = dcontour_y[0] + misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

  map->map_to_screen (2, dcontour_x, dcontour_y, dcontour_z, x, y, z);

  dx = x[1] - x[0];
  dy = y[1] - y[0];

  cell_diag_length = sqrt (pow (dx, 2) + pow (dy, 2));

  num_interp = 1;
  if (options->smoothing_factor)
    {
      segment_length = DEFAULT_SEGMENT_LENGTH / options->smoothing_factor;

      if (cell_diag_length > segment_length)
        {
          num_interp = (NV_INT32) ((cell_diag_length / segment_length) + 0.5);
        }
      else
        {
          num_interp = 1;
        }
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


  //  Set the min and max contours, the index contour interval, the maximum contour density, and the number of points to be returned
  //  by the package.

  contourMinMax (misc->abe_share->open_args[pfm].head.min_depth * options->z_factor + options->z_offset, 
                 misc->abe_share->open_args[pfm].head.max_depth * options->z_factor + options->z_offset);
  contourMaxDensity (misc->maxd);
  contourMaxPoints (CONTOUR_POINTS);


  //  If the contour interval is set to 0.0 use the user defined levels.

  if (misc->abe_share->cint == 0.0) 
    {
      contourLevels (misc->abe_share->num_levels, misc->abe_share->contour_levels);
      contourEmphasis (9999999);
    }
  else
    {
      if (options->contour_index)
        {
          contourEmphasis (options->contour_index);
        }
      else
        {
          contourEmphasis (9999999);
        }
    }


  //  Pass the grid array (arranged 1D) to the contouring package.

  initContour (misc->abe_share->cint, nrr, ncc, ar);


  //  Compute half of a grid cell in degrees.

  half_gridx = misc->abe_share->open_args[pfm].head.x_bin_size_degrees * 0.5;
  half_gridy = misc->abe_share->open_args[pfm].head.y_bin_size_degrees * 0.5;


  //  Get the contours from the package until all are drawn.

  while (getContour (&level, &index_contour, &num_points, contour_x, contour_y))
    {
      //  Convert from grid points (returned contours) to position.

      for (NV_INT32 i = 0 ; i < num_points ; i++)
        {
          dcontour_x[i] = xorig + (contour_x[i] * misc->abe_share->open_args[pfm].head.x_bin_size_degrees) + half_gridx;
          dcontour_y[i] = yorig + (contour_y[i] * misc->abe_share->open_args[pfm].head.y_bin_size_degrees) + half_gridy; 
        }


      //  Convert from position to pixels.

      map->map_to_screen (num_points, dcontour_x, dcontour_y, dcontour_z, xyz_x, xyz_y, xyz_z);


      //  smooth out the contour (don't do this sooner for the sake of efficiency -- smooth in xy plot space)

      if (options->smoothing_factor > 0) smooth_contour (num_interp, &num_points, xyz_x, xyz_y);


      QColor cnt = options->contour_color;
      if (level == highlight) cnt = options->contour_highlight_color;


      //  Label index contours

      if (index_contour)
        {
          width = options->contour_width * 2;

          cont_index_1 = 0;
          cont_index_2 = num_points;
          label_x = 0;


          // determine start/end indices. If < 80 pts in contour line, skip labeling

          //if (num_points <= 80)


          // Cheating here - I don't want to do contour labeling at present but I may do it in the future so we're
          // looking for an impossible answer.  See line above for the proper check.

          if (num_points <= 1000000000)
            {
              win_index_2 = 1;
            }
          else
            {
              // initalize window indices to contour indices

              win_index_1 = (cont_index_1 + cont_index_2 + 1) / 2;


              // determine length of label and angle at which to plot it.
              // length of label is number of digits to left of decimal plus number of decimal places desired

              digits = NINT (log10f (level + 0.5));
              label_len_chars = digits;	// number of chars

              NV_FLOAT64 wln = (NV_FLOAT64) label_len_chars * CONTOUR_LABEL_HEIGHT;

              NV_FLOAT64 label_len = wln * 1.50;

              win_index_2 = win_index_1 + 1;

              if (win_index_2 >= cont_index_2)
                {
                  win_index_2 = cont_index_1;
                }
              else
                {
                  do
                    {
                      NV_FLOAT64 index_diff_x = xyz_x[win_index_1] - xyz_x[win_index_2];
                      NV_FLOAT64 index_diff_y = xyz_y[win_index_1] - xyz_y[win_index_2];
                      dist = sqrt ((index_diff_x * index_diff_x) + (index_diff_y * index_diff_y));


                      win_index_2++;


                      //  if at end of contour and haven't reached correct dist yet break out and draw contour
                      //  without a label

                      if (win_index_2 > num_points) break;

                    } while (dist < label_len);


                  if (win_index_2 > num_points)
                    {
                      win_index_2 = 1;
                    }
                  else
                    {
                      //  plot first part of contour

                      for (NV_INT32 i = 1 ; i < win_index_1 ; i++)
                        {
                          map->drawLine (NINT (xyz_x[i - 1]), NINT (xyz_y[i - 1]), NINT (xyz_x[i]), NINT (xyz_y[i]), cnt, width, NVFalse, Qt::SolidLine);
                        }


                      //  calculate x/y positions for label

                      label_x = NINT (xyz_x[win_index_1] + (xyz_x[win_index_2] - xyz_x[win_index_1]) / 2.0 - label_len / 2.0);
                      label_y = NINT (xyz_y[win_index_1] + (xyz_y[win_index_2] - xyz_y[win_index_1]) / 2.0 + HALF_CONTOUR_LABEL_HEIGHT);


                      //  plot label

                      NV_CHAR format[20];

                      sprintf (format, " %%%dd ", digits);

                      label.sprintf (format, NINT (level));
                    }
                }
            }


          for (NV_INT32 i = win_index_2 ; i < num_points ; i++)
            {
              map->drawLine (NINT (xyz_x[i - 1]), NINT (xyz_y[i - 1]), NINT (xyz_x[i]), NINT (xyz_y[i]), cnt, width, NVFalse, Qt::SolidLine);
            }



          /*  Convert from grid points (returned contours) to position.  Contours are output as elevations, not depths.  */

          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              dcontour_x[i] = xorig + (contour_x[i] * misc->abe_share->open_args[pfm].head.x_bin_size_degrees) + half_gridx;
              dcontour_y[i] = yorig + (contour_y[i] * misc->abe_share->open_args[pfm].head.y_bin_size_degrees) + half_gridy;
              dcontour_z[i] = level;
            }


          if (label_x) map->drawText (label, label_x, label_y, 90.0, CONTOUR_LABEL_HEIGHT, options->contour_color, NVFalse);
        }
      else
        {
          width = options->contour_width;


          //  Draw the contours.

          for (NV_INT32 i = 1 ; i < num_points ; i++)
            {
              map->drawLine (NINT (xyz_x[i - 1]), NINT (xyz_y[i - 1]), NINT (xyz_x[i]), NINT (xyz_y[i]), cnt, width, NVFalse, Qt::SolidLine);
            }
        }


      //  If we're capturing the contours we have to convert them back to positions and then check them against the 
      //  bounding polygon.

      if (capture)
        {
          for (NV_INT32 i = 0 ; i < num_points ; i++)
            map->screen_to_map (num_points, dcontour_x, dcontour_y, dcontour_z, xyz_x, xyz_y, xyz_z);

          misc->draw_contour_level = level;
          writeContour (misc, options->z_factor, options->z_offset, num_points, dcontour_x, dcontour_y);
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


  //  If we were capturing the contours we need to restore the original draw contour level.

  if (capture)
    {
      misc->draw_contour_level = save_draw_contour_level;
      for (NV_INT32 gpfm = 0 ; gpfm < misc->abe_share->pfm_count ; gpfm++) misc->contour_in_pfm[gpfm] = NVFalse;
    }


  map->flush ();
}
