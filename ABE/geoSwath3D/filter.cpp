#include "geoSwath3D.hpp"


typedef struct
{
  NV_U_INT32        rec;
  NV_U_INT32        val;
} REC_VAL;


typedef struct
{
  REC_VAL           *rv;
  NV_U_INT32        num_soundings;
  NV_FLOAT32        standard_dev;
  NV_FLOAT32        avg_filtered_depth;
  NV_BOOL           data;
} BIN_DEPTH;


static BIN_DEPTH    **bin_depth;
NV_F64_XYMBR        bounds;


#define SQR(x) ((x)*(x))


/***************************************************************************\
*                                                                           *
*   Module Name:        compute_bin_values                                  *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       December 2000                                       *
*                                                                           *
*   Purpose:            Recomputes the bin record values from the depth     *
*                       records.  Only computes the average filtered value, *
*                       the standard deviation, and the number of valid     *
*                       depths.                                             *
*                                                                           *
*   Arguments:          bin_depth       -   bin_depth record                *
*                                                                           *
*   Return Value:       Number of valid points                              *
*                                                                           *
\***************************************************************************/

void compute_bin_values (BIN_DEPTH *bin_depth, POINT_DATA *data)
{
  NV_FLOAT64 sum_filtered = 0.0, sum2_filtered = 0.0;
  NV_INT32 filtered_count = 0;


  for (NV_U_INT32 i = 0 ; i < bin_depth->num_soundings ; i++)
    {
      //  DO NOT use records marked as file deleted.

      if (!(bin_depth->rv[i].val & PFM_DELETED))
        {
          if (!(bin_depth->rv[i].val & PFM_INVAL))
            {
              sum_filtered += data->z[bin_depth->rv[i].rec];
              sum2_filtered += SQR (data->z[bin_depth->rv[i].rec]);

              filtered_count++;
            }
        }
    }


  if (!filtered_count)
    {
      bin_depth->data = NVFalse;
    }
  else
    {
      bin_depth->avg_filtered_depth = sum_filtered / (NV_FLOAT64) filtered_count; 
      if (filtered_count > 1)
        {
          bin_depth->standard_dev = sqrt ((sum2_filtered - ((NV_FLOAT64) filtered_count * (pow ((NV_FLOAT64) bin_depth->avg_filtered_depth, 2.0)))) / 
                                          ((NV_FLOAT64) filtered_count - 1.0));
        }
      else
        {
          bin_depth->standard_dev = 0.0;
        }
      bin_depth->data = NVTrue;
    }

  return;
}



/********************************************************************
 *
 * Function Name :  InitializeAreaFilter
 *
 * Description : Perform any initialization specific to the area based
 *               filter here.
 *
 ********************************************************************/

NV_BOOL InitializeAreaFilter (NV_INT32 width, NV_INT32 height, POINT_DATA *data)
{
  //  Allocate the memory for all of the bins.

  bin_depth = (BIN_DEPTH **) calloc (height, sizeof (BIN_DEPTH *));

  if (bin_depth == NULL)
    {
      QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D filter"), geoSwath3D::tr ("Unable to allocate memory for filtering!"));
      return (NVFalse);
    }

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      bin_depth[i] = (BIN_DEPTH *) calloc (width, sizeof (BIN_DEPTH));

      if (bin_depth[i] == NULL)
        {
          free (bin_depth);
          QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D filter"), geoSwath3D::tr ("Unable to allocate memory for filtering!"));
          return (NVFalse);
        }
    }


  //  Populate all of the bins with depth data.

  for (NV_INT32 i = 0 ; i < data->count ; i++)
    {
      NV_INT32 row = NINT ((data->y[i] - bounds.min_y) / data->y_grid_size);
      NV_INT32 col = NINT ((data->x[i] - bounds.min_x) / data->x_grid_size);


      //  This should never happen.

      if (row < 0 || col < 0)
        {
          for (NV_INT32 j = 0 ; j < height ; j++)
            {
              if (bin_depth[j] != NULL) 
                {
                  for (NV_INT32 k = 0 ; k < width ; k++)
                    {
                      if (bin_depth[j][k].rv) free (bin_depth[j][k].rv);
                    }
                  free (bin_depth[j]);
                }
            }
          return (NVFalse);
        }

      NV_INT32 recnum = bin_depth[row][col].num_soundings;

      bin_depth[row][col].rv = (REC_VAL *) realloc (bin_depth[row][col].rv, (recnum + 1) * sizeof (REC_VAL));


      //  Make sure we allocated the memory.

      if (bin_depth[row][col].rv == NULL)
        {
          for (NV_INT32 j = 0 ; j < height ; j++)
            {
              if (bin_depth[j] != NULL) 
                {
                  for (NV_INT32 k = 0 ; k < width ; k++)
                    {
                      if (bin_depth[j][k].rv) free (bin_depth[j][k].rv);
                    }
                  free (bin_depth[j]);
                }
            }

          QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D filter"), geoSwath3D::tr ("Unable to allocate memory for filtering!"));
          return (NVFalse);
        }

      bin_depth[row][col].rv[recnum].val = data->val[i];
      bin_depth[row][col].rv[recnum].rec = i;

      bin_depth[row][col].num_soundings++;
    }


  //  Now compute the average and standard deviation for the bin.

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      for (NV_INT32 j = 0 ; j < width ; j++)
        {
          compute_bin_values (&bin_depth[i][j], data);
        }
    }

  return (NVTrue);
}



/********************************************************************
 *
 * Function Name : AreaFilter
 *
 * Description : Buffers three rows of PFM data and filters it.
 *
 ********************************************************************/

NV_BOOL AreaFilter (OPTIONS *options, MISC *misc, NV_FLOAT64 bin_diagonal, NV_INT32 row, NV_INT32 col, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 poly_count, POINT_DATA *data)
{
  //  If the center cell has no valid data, return.

  if (!bin_depth[row][col].data) return (NVFalse);


  NV_FLOAT64 avgsum = 0.0;
  NV_FLOAT64 stdsum = 0.0;
  NV_FLOAT64 sum2 = 0.0;
  NV_INT32 sumcount = 0;


  //  Get the information from the 8 cells surrounding this cell (and save the center cell data).

  for (NV_INT32 i = row - 1 ; i <= row + 1 ; i++)
    {
      for (NV_INT32 j = col - 1 ; j <= col + 1 ; j++)
        {
          if (bin_depth[i][j].data)
            {
              avgsum += bin_depth[i][j].avg_filtered_depth;
              stdsum += bin_depth[i][j].standard_dev;
              sum2 += SQR (bin_depth[i][j].avg_filtered_depth);

              sumcount++;
            }
        }
    }


  //  Make sure that at least 5 of the surrounding cells have valid data.  This saves us from deleting a lot of edge points
  //  without any real evidence that they were bad.

  if (sumcount < 5) return (NVFalse);


  //  Compute the eight slopes from the center cell to find out if it's flat enough to use the average of the
  //  standard deviations or if we need to use the standard deviation of the averages.

  NV_BOOL flat = NVTrue;
  NV_FLOAT64 dx, slope;

  for (NV_INT32 i = row - 1, k = 0 ; i <= row + 1 ; i++, k++)
    {
      for (NV_INT32 j = col - 1 ; j <= col ; j++)
        {
          if (i != row || j != col)
            {
              if (bin_depth[i][j].data)
                {
                  dx = bin_diagonal;

                  slope = (fabs (bin_depth[row][col].avg_filtered_depth - bin_depth[i][j].avg_filtered_depth)) / dx;

                  if (slope > 1.0)
                    {
                      flat = NVFalse;
                      break;
                    }
                }
            }
        }
    }


  NV_INT32 count = 0;

  /*
      If the slope is low (< 1 degree) we'll use an average of the cell standard deviations to beat the 
      depths against.  Otherwise, we'll compute the standard deviation from the cell averages.  Since 
      we're using the average of the computed standard deviations of all of the nine cells or the 
      standard deviations of the averages of all nine cells we multiply the resulting standard
      deviation (?) by two to get a reasonable result, otherwise the standard deviation surface is too
      smooth and we end up cutting out too much good data.  I must admit I arrived at these numbers by
      playing with the filter using em3000 shallow water data and em121a deep water data but they appear
      to work properly.  This way three sigma seems to cut out what you would expect three sigma to cut
      out.  If you leave it as is it cuts out about 30%.  This is called empirically determining a value
      (From Nero's famous statement "I'm the emperor and I can do what I damn well please, now hand me
      my fiddle.").   JCD
  */

  NV_FLOAT64 avg = avgsum / (NV_FLOAT64) sumcount;
  NV_FLOAT64 std;
  if (flat)
    {
      std = (stdsum / (NV_FLOAT64) sumcount) * 2.0;
    }
  else
    {
      std = (sqrt ((sum2 - ((NV_FLOAT64) sumcount * SQR (avg))) / ((NV_FLOAT64) sumcount - 1.0))) * 2.0;
    }

  NV_FLOAT64 BinSigmaFilter = options->filterSTD * std;


  for (NV_U_INT32 i = 0 ; i < bin_depth[row][col].num_soundings ; i++)
    {
      //  Only check those that haven't been checked before.

      if (!(bin_depth[row][col].rv[i].val & (PFM_INVAL | PFM_DELETED)))
        {
          NV_FLOAT64 depth = data->z[bin_depth[row][col].rv[i].rec];


          //  Check in both directions first

          if (fabs (depth - avg) >= BinSigmaFilter)
            {
              //  Check the deep filter only flag and, if set, check in the deep direction only

              if (!data->fmask[bin_depth[row][col].rv[i].rec] && (!options->deep_filter_only || (depth - avg) >= BinSigmaFilter))
                {
                  bin_depth[row][col].rv[i].val |= PFM_FILTER_INVAL;


                  //  Only update the record if the point is within the polygon

                  if (inside_polygon2 (mx, my, poly_count, data->x[bin_depth[row][col].rv[i].rec], data->y[bin_depth[row][col].rv[i].rec]))
                    {
                      //  Check for all of the limits on this point.
                      if (!check_bounds (options, misc, data, bin_depth[row][col].rv[i].rec, NVFalse, misc->slice))
                        {
                          misc->filter_kill_list = (NV_INT32 *) realloc (misc->filter_kill_list, (misc->filter_kill_count + 1) * sizeof (NV_INT32));

                          if (misc->filter_kill_list == NULL)
                            {
                              perror ("Allocating misc->filter_kill_list memory in pfmFilter");
                              exit (-1);
                            }

                          misc->filter_kill_list[misc->filter_kill_count] = bin_depth[row][col].rv[i].rec;
                          misc->filter_kill_count++;
                        }
                    }
                }
            }
        }

      count++;
    }


  //  Recompute the bin record based on the modified contents of the depth array.

  compute_bin_values (&bin_depth[row][col], data);


  return (NVTrue);
}



NV_BOOL filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count, POINT_DATA *data)
{
  //  Compute the bin diagonal distance for the filter.

  NV_FLOAT64 bin_diagonal = sqrt (SQR (data->x_grid_size) + SQR (data->y_grid_size));


  //  Figure out the width, start row, start column, end row, and end column using the bin sizes from the level 0 PFM.

  bounds.min_x = 999999.0;
  bounds.min_y = 999999.0;
  bounds.max_x = -999999.0;
  bounds.max_y = -999999.0;
  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      bounds.min_x = qMin (bounds.min_x, mx[i]);
      bounds.min_y = qMin (bounds.min_y, my[i]);
      bounds.max_x = qMax (bounds.max_x, mx[i]);
      bounds.max_y = qMax (bounds.max_y, my[i]);
    }


  //  Add one grid cell distance around the area just to make sure we get everythin

  bounds.min_y -= data->y_grid_size;
  bounds.max_y += data->y_grid_size;
  bounds.min_x -= data->x_grid_size;
  bounds.max_x += data->x_grid_size;


  NV_INT32 width = (NINT ((bounds.max_x - bounds.min_x) / data->x_grid_size)) + 1;
  NV_INT32 height = (NINT ((bounds.max_y - bounds.min_y) / data->y_grid_size)) + 1;


  if (!InitializeAreaFilter (width, height, data)) return (NVFalse);
      

  misc->statusProgLabel->setText (geoSwath3D::tr ("Filtering..."));
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, height);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  //  Note that we're not filtering the edge cells since we won't have surrounding data.

  for (NV_INT32 i = 1 ; i < height - 1 ; i++)
    {
      misc->statusProg->setValue (i);


      for (NV_INT32 j = 1 ; j < width - 1 ; j++)
        {
          AreaFilter (options, misc, bin_diagonal, i, j, mx, my, count, data);
        }


      //  Check the event queue to see if the user wants to interrupt the filter.  Only 
      //  check every 10th row so it doesn't slow things down too much.  */

      if (!(i % 10))
        {
          if (qApp->hasPendingEvents ())
            {
              qApp->processEvents();
              if (misc->drawing_canceled)
                {
                  misc->filter_kill_count = 0;
                  misc->statusProg->reset ();
                  misc->statusProg->setRange (0, 100);
                  misc->statusProg->setValue (0);
                  misc->statusProgLabel->setVisible (FALSE);
                  misc->statusProg->setTextVisible (FALSE);
                  misc->drawing_canceled = NVFalse;
                  qApp->processEvents();
                  return (NVFalse);
                }
            }
        }
    }

  misc->statusProg->reset ();
  misc->statusProg->setRange (0, 100);
  misc->statusProg->setValue (0);
  misc->statusProgLabel->setVisible (FALSE);
  misc->statusProg->setTextVisible (FALSE);

  qApp->processEvents();


  //  Free the memory used by the filter.

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      if (bin_depth[i] != NULL) 
        {
          for (NV_INT32 j = 0 ; j < width ; j++)
            {
              if (bin_depth[i][j].rv) free (bin_depth[i][j].rv);
            }
          free (bin_depth[i]);
        }
    }

  return (NVTrue);
}
