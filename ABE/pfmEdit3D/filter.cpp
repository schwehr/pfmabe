
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


typedef struct
{
  NV_FLOAT64        lat;
  NV_FLOAT64        lon;
  NV_FLOAT64        poly_y[2000];
  NV_FLOAT64        poly_x[2000];
  NV_INT32          poly_count;
} FEATURE_RECORD;


static BIN_DEPTH      **bin_depth;
NV_F64_XYMBR          bounds;
static FEATURE_RECORD *feature = NULL;
static NV_INT32       feature_count = 0;
static NV_FLOAT32     feature_radius;
static NV_FLOAT64     x_bin_size, y_bin_size;


#define SQR(x) ((x)*(x))


/***************************************************************************/
/*!

  - Module Name:        compute_bin_values

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 2000

  - Purpose:            Recomputes the bin record values from the depth
                        records.  Only computes the average filtered value,
                        the standard deviation, and the number of valid
                        depths.

  - Arguments:
                        - bin_depth       =   bin_depth record

  - Return Value:
                        - Number of valid points

****************************************************************************/

void compute_bin_values (BIN_DEPTH *bin_depth, MISC *misc)
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
              sum_filtered += misc->data[bin_depth->rv[i].rec].z;
              sum2_filtered += SQR (misc->data[bin_depth->rv[i].rec].z);

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



/********************************************************************/
/*!
 
 - Function Name :  InitializeAreaFilter
 
 - Description : Perform any initialization specific to the area based
                 filter here.
 
*********************************************************************/

NV_BOOL InitializeAreaFilter (NV_INT32 width, NV_INT32 height, MISC *misc)
{
  //  Allocate the memory for all of the bins.

  bin_depth = (BIN_DEPTH **) calloc (height, sizeof (BIN_DEPTH *));

  if (bin_depth == NULL)
    {
      QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D filter"), pfmEdit3D::tr ("Unable to allocate memory for filtering!"));
      return (NVFalse);
    }

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      bin_depth[i] = (BIN_DEPTH *) calloc (width, sizeof (BIN_DEPTH));

      if (bin_depth[i] == NULL)
        {
          free (bin_depth);
          QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D filter"), pfmEdit3D::tr ("Unable to allocate memory for filtering!"));
          return (NVFalse);
        }
    }


  //  Populate all of the bins with depth data.

  for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
    {
      NV_INT32 row = NINT ((misc->data[i].y - bounds.min_y) / y_bin_size);
      NV_INT32 col = NINT ((misc->data[i].x - bounds.min_x) / x_bin_size);


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

          QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D filter"), pfmEdit3D::tr ("Unable to allocate memory for filtering!"));
          return (NVFalse);
        }

      bin_depth[row][col].rv[recnum].val = misc->data[i].val;
      bin_depth[row][col].rv[recnum].rec = i;

      bin_depth[row][col].num_soundings++;
    }


  //  Now compute the average and standard deviation for the bin.

  for (NV_INT32 i = 0 ; i < height ; i++)
    {
      for (NV_INT32 j = 0 ; j < width ; j++)
        {
          compute_bin_values (&bin_depth[i][j], misc);
        }
    }

  return (NVTrue);
}



/********************************************************************/
/*!

 - Function Name : AreaFilter
 
 - Description :   Buffers three rows of PFM data and filters it.
 
 - Inputs :
                   - pfm          = PFM layer number
                   - coord        = current cell coordinate that we are processing.
                   - bin_diagonal = diagonal distance between bins in meters
 
 - Method :        If the slope is low (< 1 degree) we'll use an average of the cell standard
                   deviations to beat the depths against.  Otherwise, we'll compute the
                   standard deviation from the cell averages.  Since we're using the average
                   of the computed standard deviations of all of the nine cells or the 
                   standard deviations of the averages of all nine cells we multiply the
                   resulting standard deviation (?) by two to get a reasonable result,
                   otherwise the standard deviation surface is too smooth and we end up
                   cutting out too much good data.  I must admit I arrived at these numbers
                   by playing with the filter using em3000 shallow water data and em121a deep
                   water data but they appear to work properly.  This way three sigma seems
                   to cut out what you would expect three sigma to cut out.  If you leave it
                   as is it cuts out about 30%.  This is called empirically determining a
                   value (From Nero's famous statement "I'm the emperor and I can do what I
                   damn well please, now hand me my fiddle.").   JCD

********************************************************************/

NV_BOOL AreaFilter (OPTIONS *options, MISC *misc, NV_FLOAT64 bin_diagonal, NV_INT32 row, NV_INT32 col, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 poly_count)
{
  //  If the center cell has no valid data, return.

  if (!bin_depth[row][col].data) return (NVFalse);


  BIN_HEADER bin_header = misc->abe_share->open_args[0].head;

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
          NV_FLOAT64 depth = misc->data[bin_depth[row][col].rv[i].rec].z;


          //  Check in both directions first

          if (fabs (depth - avg) >= BinSigmaFilter)
            {
              //  Check the deep filter only flag and, if set, check in the deep direction only

              if (!misc->data[bin_depth[row][col].rv[i].rec].fmask && (!options->deep_filter_only || (depth - avg) >= BinSigmaFilter))
                {
                  bin_depth[row][col].rv[i].val |= PFM_MANUALLY_INVAL;


                  //  Only update the record if the point is within the polygon

                  if (inside_polygon2 (mx, my, poly_count, misc->data[bin_depth[row][col].rv[i].rec].x, misc->data[bin_depth[row][col].rv[i].rec].y))
                    {
                      //  Check for all of the limits on this point.

                      if (!check_bounds (options, misc, misc->data[bin_depth[row][col].rv[i].rec].x, misc->data[bin_depth[row][col].rv[i].rec].y, 
                                         misc->data[bin_depth[row][col].rv[i].rec].z, NVFalse, misc->data[bin_depth[row][col].rv[i].rec].mask,
                                         misc->data[bin_depth[row][col].rv[i].rec].pfm, NVFalse, misc->slice))
                        {
                          //  Check the point against the features (if any).  Only update the point if it is not within 
                          //  options->feature_radius of a feature and is not within any feature polygon whose feature is
                          //  in the filter area.  Also check against any temporary filter masked areas.

                          NV_BOOL filter_it = NVTrue;
                          if (feature_count)
                            {
                              for (NV_INT32 j = 0 ; j < feature_count ; j++)
                                {
                                  NV_FLOAT64 dist;

                                  geo_distance (feature[j].lat, feature[j].lon, misc->data[bin_depth[row][col].rv[i].rec].y, 
                                                misc->data[bin_depth[row][col].rv[i].rec].x, &dist);

                                  if (dist < feature_radius)
                                    {
                                      filter_it = NVFalse;
                                      break;
                                    }

                                  if (feature[j].poly_count && inside_polygon2 (feature[j].poly_x, feature[j].poly_y, feature[j].poly_count,
                                                                                misc->data[bin_depth[row][col].rv[i].rec].x,
                                                                                misc->data[bin_depth[row][col].rv[i].rec].y))
                                    {
                                      filter_it = NVFalse;
                                      break;
                                    }
                                }
                            }


                          if (filter_it)
                            {
                              misc->filter_kill_list = (NV_INT32 *) realloc (misc->filter_kill_list, (misc->filter_kill_count + 1) * sizeof (NV_INT32));

                              if (misc->filter_kill_list == NULL)
                                {
                                  perror ("Allocating misc->filter_kill_list memory in filter.cpp");
                                  exit (-1);
                                }

                              misc->filter_kill_list[misc->filter_kill_count] = bin_depth[row][col].rv[i].rec;
                              misc->filter_kill_count++;
                            }
                        }
                    }
                }
            }
        }

      count++;
    }


  //  Recompute the bin record based on the modified contents of the depth array.

  compute_bin_values (&bin_depth[row][col], misc);


  return (NVTrue);
}



//!  This kicks off the pseudo-statistical filtering of a polygonal area.

NV_BOOL filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
{
  for (NV_INT32 pfm = misc->abe_share->pfm_count - 1 ; pfm >= 0 ; pfm--)
    {
      if (strcmp (misc->abe_share->open_args[pfm].target_path, "NONE"))
        {
          //  Get the feature information for those features inside the area.

          if (options->feature_radius > 0.0)
            {
              if (misc->bfd_open)
                {
                  feature_radius = options->feature_radius;
                  feature_count = 0;

                  for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
                    {
                      if (inside_polygon2 (mx, my, count, misc->feature[i].longitude, misc->feature[i].latitude))
                        {
                          if (misc->feature[i].poly_count && misc->feature[i].poly_type)
                            {
                              feature = (FEATURE_RECORD *) realloc (feature, (feature_count + 1) * sizeof (FEATURE_RECORD));

                              if (feature == NULL)
                                {
                                  perror ("Allocating feature memory in filter.cpp");
                                  exit (-1);
                                }

                              feature[feature_count].lat = misc->feature[i].latitude;
                              feature[feature_count].lon = misc->feature[i].longitude;
                              feature[feature_count].poly_count = misc->feature[i].poly_count;

                              BFDATA_POLYGON bfd_polygon;

                              binaryFeatureData_read_polygon (misc->bfd_handle, i, &bfd_polygon);

                              for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                {
                                  feature[feature_count].poly_y[j] = bfd_polygon.latitude[j];
                                  feature[feature_count].poly_x[j] = bfd_polygon.longitude[j];
                                }

                              feature_count++;
                            }
                        }
                    }
                }
            }
        }
    }


  //  Check to see if we're using OTF bins in pfmView.  If we are we want to use that bin size for filtering, not the average PFM bin size.

  if (misc->abe_share->otf_width)
    {
      x_bin_size = misc->abe_share->otf_x_bin_size;
      y_bin_size = misc->abe_share->otf_y_bin_size;
    }
  else
    {
      x_bin_size = misc->avg_x_bin_size_degrees;
      y_bin_size = misc->avg_y_bin_size_degrees;
    }


  //  Compute the bin diagonal distance for the filter.

  NV_FLOAT64 bin_diagonal;
  NV_FLOAT64 az;
  if (misc->abe_share->open_args[0].head.mbr.min_y <= 0.0)
    {
      invgp (NV_A0, NV_B0, misc->abe_share->open_args[0].head.mbr.max_y, misc->abe_share->open_args[0].head.mbr.min_x, 
             misc->abe_share->open_args[0].head.mbr.max_y - (y_bin_size), 
             misc->abe_share->open_args[0].head.mbr.min_x + (x_bin_size), &bin_diagonal, &az);
    }
  else
    {
      invgp (NV_A0, NV_B0, misc->abe_share->open_args[0].head.mbr.min_y, misc->abe_share->open_args[0].head.mbr.min_x, 
             misc->abe_share->open_args[0].head.mbr.min_y + (y_bin_size), 
             misc->abe_share->open_args[0].head.mbr.min_x + (x_bin_size), &bin_diagonal, &az);
    }


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

  bounds.min_y -= y_bin_size;
  bounds.max_y += y_bin_size;
  bounds.min_x -= x_bin_size;
  bounds.max_x += x_bin_size;


  NV_INT32 width = (NINT ((bounds.max_x - bounds.min_x) / x_bin_size)) + 1;
  NV_INT32 height = (NINT ((bounds.max_y - bounds.min_y) / y_bin_size)) + 1;


  if (!InitializeAreaFilter (width, height, misc))
    {
      if (feature_count)
        {
          free (feature);
          feature = NULL;
          feature_count = 0;
        }
      return (NVFalse);
    }
      

  misc->statusProgLabel->setText (pfmEdit3D::tr ("Filtering..."));
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
          AreaFilter (options, misc, bin_diagonal, i, j, mx, my, count);
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


  //  Free the feature memory (if any)

  if (feature_count)
    {
      free (feature);
      feature = NULL;
      feature_count = 0;
    }


  return (NVTrue);
}
