
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


typedef struct
{
  DEPTH_RECORD      *dep;
  BIN_RECORD        bin;
} ROW_RECORD;


typedef struct
{
  NV_FLOAT64        lat;
  NV_FLOAT64        lon;
  NV_FLOAT64        poly_y[2000];
  NV_FLOAT64        poly_x[2000];
  NV_INT32          poly_count;
} FEATURE_RECORD;


static ROW_RECORD     *row[3] = {NULL, NULL, NULL};
static NV_INT32       prev_coord_y = -1;
static NV_INT32       row_num[3];
static FEATURE_RECORD *feature = NULL;
static NV_INT32       feature_count = 0;
static NV_FLOAT32     feature_radius;


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
                        - depth_record    =   depth record array
                        - bin             =   bin record

  - Return Value:       Number of valid points

****************************************************************************/

void compute_bin_values (DEPTH_RECORD *depth_record, BIN_RECORD *bin)
{
  NV_FLOAT64 sum_filtered = 0.0, sum2_filtered = 0.0;
  NV_INT32 filtered_count = 0;


  for (NV_U_INT32 i = 0 ; i < bin->num_soundings ; i++)
    {
      //  DO NOT use records marked as file deleted.

      if (!(depth_record[i].validity & PFM_DELETED))
        {
          if (!(depth_record[i].validity & PFM_INVAL))
            {
              sum_filtered += depth_record[i].xyz.z;
              sum2_filtered += SQR (depth_record[i].xyz.z);

              filtered_count++;
            }
        }
    }


  if (!filtered_count)
    {
      bin->validity &= ~PFM_DATA;
    }
  else
    {
      bin->avg_filtered_depth = sum_filtered / (NV_FLOAT64) filtered_count; 
      if (filtered_count > 1)
        {
          bin->standard_dev = sqrt ((sum2_filtered - ((NV_FLOAT64) filtered_count * 
                                                      (pow ((NV_FLOAT64) bin->avg_filtered_depth, 2.0)))) / 
                                    ((NV_FLOAT64) filtered_count - 1.0));
        }
      else
        {
          bin->standard_dev = 0.0;
        }
      bin->validity |= PFM_DATA;
    }

  return;
}



/********************************************************************/
/*!

 - Function Name :  InitializeAreaFilter

 - Description : Perform any initialization specific to the area based
                 filter here.

 - Inputs : None

 - Returns : None

 - Error Conditions : None

********************************************************************/

void InitializeAreaFilter (NV_INT32 pfm_handle, NV_INT32 width)
{
  //  Initialize the three row buffer for the simple filter.

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      row[i] = (ROW_RECORD *) malloc (width * sizeof (ROW_RECORD));

      if (row[i] == NULL)
        {
          perror ("Allocating memory in InitializeAreaFilter");
          exit (-1);
        }


      for (NV_INT32 j = 0 ; j < width ; j++)
        {
          row[i][j].bin.coord.y = i;
          row[i][j].bin.coord.x = j;

          if (!read_bin_depth_array_index (pfm_handle, &row[i][j].bin, &row[i][j].dep))
            {
              compute_bin_values (row[i][j].dep, &row[i][j].bin);
            }
          else
            {
              row[i][j].dep = NULL;
            }
        }

      row_num[i] = i;
    }
  prev_coord_y = 0;
}



/********************************************************************/
/*!

 - Function Name : AreaFilter

 - Description :   Buffers three rows of PFM data and filters it.

 - Inputs :
                   - pfm_handle         =   handle to open PFM file.
                   - coord              =   current cell coordinate that we are processing.
                   - bin_record         =   bin_record about cell we are processing. 
                   - bin_header         =   bin header
                   - standard_deviation =   standard deviation value
                   - bin_diagonal       =   diagonal distance between bins in meters

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

NV_INT32 AreaFilter (NV_INT32 pfm_handle, NV_I32_COORD2 *coord, BIN_HEADER bin_header, OPTIONS *options, MISC *misc,
                     NV_FLOAT64 bin_diagonal, NV_INT32 start_col, NV_INT32 end_col, NV_FLOAT64 *mx, NV_FLOAT64 *my,
                     NV_INT32 poly_count)
{
  BIN_RECORD bin_record;
  memset (&bin_record, 0, sizeof (BIN_RECORD));

  NV_FLOAT64 avgsum = 0.0;
  NV_FLOAT64 stdsum = 0.0;
  NV_FLOAT64 sum2 = 0.0;
  NV_INT32 sumcount = 0;
  NV_INT32 start_y, end_y, start_k;
  NV_INT32 width = end_col - start_col;


  //  Slide the 3 row buffer down one row.

  if (coord->y != prev_coord_y && coord->y > 1 && coord->y < bin_header.bin_height - 1)
    {
      //  If we moved up more than one row, read all three rows.  Otherwise, slide the rows down one row.

      end_y = coord->y + 1;
      if (coord->y - prev_coord_y == 1)
        {
          for (NV_INT32 i = start_col ; i < end_col ; i++)
            {
              NV_INT32 index = i - start_col;
              if (row[0][index].dep != NULL) free (row[0][index].dep);
              if (row[1][index].dep != NULL)
                {
                  row[0][index].dep = (DEPTH_RECORD *) calloc (row[1][index].bin.num_soundings, sizeof (DEPTH_RECORD));
                  for (NV_U_INT32 j = 0 ; j < row[1][index].bin.num_soundings ; j++)
                    {
                      row[0][index].dep[j] = row[1][index].dep[j];
                    }
                }
              else
                {
                  row[0][index].dep = NULL;
                }
              row[0][index].bin = row[1][index].bin;


              if (row[1][index].dep != NULL) free (row[1][index].dep);
              if (row[2][index].dep != NULL)
                {
                  row[1][index].dep = (DEPTH_RECORD *) calloc (row[2][index].bin.num_soundings, sizeof (DEPTH_RECORD));
                  for (NV_U_INT32 j = 0 ; j < row[2][index].bin.num_soundings ; j++)
                    {
                      row[1][index].dep[j] = row[2][index].dep[j];
                    }
                }
              else
                {
                  row[1][index].dep = NULL;
                }
              row[1][index].bin = row[2][index].bin;
            }

          row_num[0] = row_num[1];
          row_num[1] = row_num[2];

          start_y = end_y;
          start_k = 2;
        }
      else
        {
          start_y = coord->y - 1;
          start_k = 0;
        }

      NV_INT32 k = start_k;
      for (NV_INT32 i = start_y ; i <= end_y ; i++, k++)
        {
          for (NV_INT32 j = start_col ; j < end_col ; j++)
            {
              NV_INT32 index = j - start_col;
              if (row[k][index].dep != NULL) free (row[k][index].dep);
            }
          free (row[k]);


          row[k] = (ROW_RECORD *) malloc (width * sizeof (ROW_RECORD));

          if (row[k] == NULL)
            {
              perror ("Allocating memory in AreaFilter");
              exit (-1);
            }

          for (NV_INT32 j = start_col ; j < end_col ; j++)
            {
              NV_INT32 index = j - start_col;
              row[k][index].bin.coord.y = i;
              row[k][index].bin.coord.x = j;

              if (!read_bin_depth_array_index (pfm_handle, &row[k][index].bin, &row[k][index].dep))
                {
                  compute_bin_values (row[k][index].dep, &row[k][index].bin);
                }
              else
                {
                  row[k][index].dep = NULL;
                }
            }

          row_num[k] = i;
        }
      prev_coord_y = coord->y;
    }


  //  Get the information from the 8 cells surrounding this cell (and save the center cell data).

  NV_INT32 m = 0;
  NV_INT32 n = 0;
  NV_INT32 k = 0;

  for (NV_INT32 i = coord->y - 1 ; i <= coord->y + 1 ; i++, k++)
    {
      for (NV_INT32 j = coord->x - 1 ; j <= coord->x + 1 ; j++)
        {
          NV_INT32 index = j - start_col;


          //  Make sure each cell is in the area (edge effect).

          if (i >= 1 && i < bin_header.bin_height - 1 && index >= 0 && index < width)
            {
              //  If this is the center of the 9 block cell, save the block coordinates.

              if (i == coord->y && j == coord->x)
                {
                  //  Set the central bin record indices and contents to be used later

                  m = k;
                  n = index;
                  bin_record = row[m][n].bin;


                  //  If the center cell has no valid data, return.

                  if (!(bin_record.validity & PFM_DATA)) 
                    {
                      //  Recompute the bin record based on the contents of the depth array.

                      compute_bin_values (row[m][n].dep, &row[m][n].bin);

                      return (0);
                    }
                }

              if (row[k][index].bin.validity & PFM_DATA)
                {
                  avgsum += row[k][index].bin.avg_filtered_depth;
                  stdsum += row[k][index].bin.standard_dev;
                  sum2 += SQR (row[k][index].bin.avg_filtered_depth);

                  sumcount++;
                }
            }
        }
    }


  //  Make sure that at least 5 of the surrounding cells have valid data.  This saves us from deleting a lot of edge points
  //  without any real evidence that they were bad.

  if (sumcount < 5) return (0);


  //  Compute the eight slopes from the center cell to find out if it's flat enough to use the average of the
  //  standard deviations or if we need to use the standard deviation of the averages.

  NV_BOOL flat = NVTrue;
  NV_FLOAT64 dx, slope;

  for (NV_INT32 i = coord->y - 1, k = 0 ; i <= coord->y + 1 ; i++, k++)
    {
      for (NV_INT32 j = coord->x - 1 ; j <= coord->x + 1 ; j++)
        {
          NV_INT32 index = j - start_col;


          //  Make sure each cell is in the area (edge effect).

          if (i >= 1 && i < bin_header.bin_height - 1 && index >= 0 && index < width)
            {
              if (k != m || index != n)
                {
                  if (row[k][index].bin.validity & PFM_DATA)
                    {
                      if ((abs (bin_record.coord.x - row[k][index].bin.coord.x) + 
                           abs (bin_record.coord.y - row[k][index].bin.coord.y)) > 1)
                        {
                          dx = bin_diagonal;
                        }
                      else
                        {
                          dx = bin_header.bin_size_xy;
                        }

                      slope = (fabs (bin_record.avg_filtered_depth - row[k][index].bin.avg_filtered_depth)) / dx;

                      if (slope > 1.0)
                        {
                          flat = NVFalse;
                          break;
                        }
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


  for (NV_U_INT32 i = 0 ; i < row[m][n].bin.num_soundings ; i++)
    {
      NV_FLOAT64 depth = row[m][n].dep[i].xyz.z;


      //  Only check those that haven't been checked before.

      if (!(row[m][n].dep[i].validity & (PFM_INVAL | PFM_DELETED)))
        {
          //  Check in both directions first

          if (fabs (depth - avg) >= BinSigmaFilter)
            {

              //  Check the deep filter only flag and, if set, check in the deep direction only

              if (!options->deep_filter_only || (depth - avg) >= BinSigmaFilter)
                {
                  row[m][n].dep[i].validity |= PFM_MANUALLY_INVAL;


                  //  Only update the record if the point is within the polygon

                  if (inside_polygon2 (mx, my, poly_count, row[m][n].dep[i].xyz.x, row[m][n].dep[i].xyz.y))
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
                              pfm_geo_distance (pfm_handle, feature[j].lat, feature[j].lon, row[m][n].dep[i].xyz.y, 
                                                row[m][n].dep[i].xyz.x, &dist);

                              if (dist < feature_radius)
                                {
                                  filter_it = NVFalse;
                                  break;
                                }

                              if (feature[j].poly_count && inside_polygon2 (feature[j].poly_x, feature[j].poly_y, feature[j].poly_count,
                                                                            row[m][n].dep[i].xyz.x, row[m][n].dep[i].xyz.y))
                                {
                                  filter_it = NVFalse;
                                  break;
                                }
                            }
                        }


                      //  Check the masked areas.

                      for (NV_INT32 j = 0 ; j < misc->poly_filter_mask_count ; j++)
                        {
                          if (inside (misc->poly_filter_mask[j].x, misc->poly_filter_mask[j].y, misc->poly_filter_mask[j].count,
                                      row[m][n].dep[i].xyz.x, row[m][n].dep[i].xyz.y))
                            {
                              filter_it = NVFalse;
                              break;
                            }
                        }


                      if (filter_it)
                        {
                          NV_INT32 rc = update_depth_record_index (pfm_handle, &row[m][n].dep[i]);
                          if (rc != SUCCESS)
                            {
                              fprintf (stderr, "Error on depth status update.\n");
                              fprintf (stderr, "%s\n", pfm_error_str (rc));
                              fflush (stderr);
                            }
                        }
                    }
                }
            }
        }

      count++;
    }


  //  Recompute the bin record based on the modified contents of the depth array.

  compute_bin_values (row[m][n].dep, &row[m][n].bin);


  return (0);
}



void filterPolyArea (OPTIONS *options, MISC *misc, NV_FLOAT64 *mx, NV_FLOAT64 *my, NV_INT32 count)
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
                          feature = (FEATURE_RECORD *) realloc (feature, (feature_count + 1) * sizeof (FEATURE_RECORD));

                          if (feature == NULL)
                            {
                              perror ("Allocating feature memory in pfmFilter.cpp");
                              exit (-1);
                            }

                          feature[feature_count].lat = misc->feature[i].latitude;
                          feature[feature_count].lon = misc->feature[i].longitude;
                          feature[feature_count].poly_count = 0;

                          if (misc->feature[i].poly_count && misc->feature[i].poly_type)
                            {
                              feature[feature_count].poly_count = misc->feature[i].poly_count;

                              BFDATA_POLYGON bfd_polygon;

                              binaryFeatureData_read_polygon (misc->bfd_handle, i, &bfd_polygon);

                              for (NV_U_INT32 j = 0 ; j < misc->feature[i].poly_count ; j++)
                                {
                                  feature[feature_count].poly_y[j] = bfd_polygon.latitude[j];
                                  feature[feature_count].poly_x[j] = bfd_polygon.longitude[j];
                                }
                            }

                          feature_count++;
                        }
                    }
                }
            }
        }


      //  Compute the bin diagonal distance for the filter.

      NV_FLOAT64 bin_diagonal;
      if (fabs (misc->abe_share->open_args[pfm].head.x_bin_size_degrees -
                misc->abe_share->open_args[pfm].head.y_bin_size_degrees) < 0.000001)
        {
          NV_FLOAT64 az;
          if (misc->abe_share->open_args[pfm].head.mbr.min_y <= 0.0)
            {
              invgp (NV_A0, NV_B0, misc->abe_share->open_args[pfm].head.mbr.max_y,
                     misc->abe_share->open_args[pfm].head.mbr.min_x, 
                     misc->abe_share->open_args[pfm].head.mbr.max_y -
                     (misc->abe_share->open_args[pfm].head.y_bin_size_degrees), 
                     misc->abe_share->open_args[pfm].head.mbr.min_x +
                     (misc->abe_share->open_args[pfm].head.x_bin_size_degrees), 
                     &bin_diagonal, &az);
            }
          else
            {
              invgp (NV_A0, NV_B0, misc->abe_share->open_args[pfm].head.mbr.min_y,
                     misc->abe_share->open_args[pfm].head.mbr.min_x, 
                     misc->abe_share->open_args[pfm].head.mbr.min_y +
                     (misc->abe_share->open_args[pfm].head.y_bin_size_degrees), 
                     misc->abe_share->open_args[pfm].head.mbr.min_x +
                     (misc->abe_share->open_args[pfm].head.x_bin_size_degrees), 
                     &bin_diagonal, &az);
            }
        }
      else
        {
          bin_diagonal = 2.0 * sqrt (misc->abe_share->open_args[pfm].head.bin_size_xy);
        }


      //  Figure out the width, start row, start column, end row, and end column

      NV_F64_XYMBR bounds;

      bounds.min_x = 999999.0;
      bounds.min_y = 999999.0;
      bounds.max_x = -999999.0;
      bounds.max_y = -999999.0;
      for (NV_INT32 i = 0 ; i < count ; i++)
        {
          if (misc->dateline && mx[i] < 0.0) mx[i] += 360.0;

          bounds.min_x = qMin (bounds.min_x, mx[i]);
          bounds.min_y = qMin (bounds.min_y, my[i]);
          bounds.max_x = qMax (bounds.max_x, mx[i]);
          bounds.max_y = qMax (bounds.max_y, my[i]);
        }


      //  Adjust input bounds to nearest grid point.

      bounds.min_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
        (NINT ((bounds.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
               misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) * 
        misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

      bounds.max_y = misc->abe_share->open_args[pfm].head.mbr.min_y + 
        (NINT ((bounds.max_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
               misc->abe_share->open_args[pfm].head.y_bin_size_degrees)) * 
        misc->abe_share->open_args[pfm].head.y_bin_size_degrees;

      bounds.min_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
        (NINT ((bounds.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
               misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) * 
        misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

      bounds.max_x = misc->abe_share->open_args[pfm].head.mbr.min_x + 
        (NINT ((bounds.max_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
               misc->abe_share->open_args[pfm].head.x_bin_size_degrees)) * 
        misc->abe_share->open_args[pfm].head.x_bin_size_degrees;

      bounds.min_y = qMax (bounds.min_y, misc->abe_share->open_args[pfm].head.mbr.min_y);
      bounds.max_y = qMin (bounds.max_y, misc->abe_share->open_args[pfm].head.mbr.max_y);
      bounds.min_x = qMax (bounds.min_x, misc->abe_share->open_args[pfm].head.mbr.min_x);
      bounds.max_x = qMin (bounds.max_x, misc->abe_share->open_args[pfm].head.mbr.max_x);


      NV_INT32 width = (NINT ((bounds.max_x - bounds.min_x) / misc->abe_share->open_args[pfm].head.x_bin_size_degrees));
      NV_INT32 height = (NINT ((bounds.max_y - bounds.min_y) / misc->abe_share->open_args[pfm].head.y_bin_size_degrees));
      NV_INT32 irow = NINT ((bounds.min_y - misc->abe_share->open_args[pfm].head.mbr.min_y) /
                            misc->abe_share->open_args[pfm].head.y_bin_size_degrees);
      NV_INT32 column = NINT ((bounds.min_x - misc->abe_share->open_args[pfm].head.mbr.min_x) /
                              misc->abe_share->open_args[pfm].head.x_bin_size_degrees);


      //  We have to be able to grab 1 cell around the selected cell so we want to make sure we don't start at 0
      //  or end at bin_width or bin_height.

      if (irow < 1) irow = 1;
      if (column < 1) column = 1;

      if (column + width >= misc->abe_share->open_args[pfm].head.bin_width - 1)
        {
          width = misc->abe_share->open_args[pfm].head.bin_width - column - 2;
          bounds.max_x = misc->abe_share->open_args[pfm].head.mbr.max_x;
        }

      if (irow + height >= misc->abe_share->open_args[pfm].head.bin_height - 1)
        {
          height = misc->abe_share->open_args[pfm].head.bin_height - irow - 2;
          bounds.max_y = misc->abe_share->open_args[pfm].head.mbr.max_y;
        }


      NV_INT32 start_y = irow;
      NV_INT32 end_y = irow + height;
      NV_INT32 start_x = column;
      NV_INT32 end_x = column + width;


      InitializeAreaFilter (misc->pfm_handle[pfm], width);


      misc->statusProg->setRange (0, height);
      misc->statusProgLabel->setText (pfmView::tr (" Filtering "));
      misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc->statusProgLabel->setPalette (misc->statusProgPalette);
      misc->statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      NV_I32_COORD2 coord;
      for (NV_INT32 i = start_y ; i < end_y ; i++)
        {
          coord.y = i;

          misc->statusProg->setValue (i - start_y);
          qApp->processEvents ();


          for (NV_INT32 j = start_x ; j < end_x ; j++)
            {
              coord.x = j;

              AreaFilter (misc->pfm_handle[pfm], &coord, misc->abe_share->open_args[pfm].head, options, misc, bin_diagonal,
                          start_x, end_x, mx, my, count);
            }
        }


      misc->statusProg->reset ();
      misc->statusProg->setTextVisible (FALSE);
      qApp->processEvents();


      //  Free the memory used by the filter.

      for (NV_INT32 i = 0 ; i < 3 ; i++)
        {
          if (row[i] != NULL) 
            {
              for (NV_INT32 j = 0 ; j < width ; j++)
                {
                  if (row[i][j].dep) free (row[i][j].dep);
                }
              free (row[i]);
            }
        }


      //  Free the feature memory (if any)

      if (feature_count)
        {
          free (feature);
          feature = NULL;
          feature_count = 0;
        }


      misc->statusProg->setRange (0, height);
      misc->statusProgLabel->setText (pfmView::tr (" Recomputing bins "));
      misc->statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      //  Recompute the bin values

      BIN_RECORD bin;
      for (NV_INT32 i = start_y ; i < end_y ; i++)
        {
          coord.y = i;

          misc->statusProg->setValue (i - start_y);
          qApp->processEvents ();


          for (NV_INT32 j = start_x ; j < end_x ; j++)
            {
              coord.x = j;

              recompute_bin_values_index (misc->pfm_handle[pfm], coord, &bin, 0);
            }
        }


      misc->statusProg->reset ();
      misc->statusProg->setTextVisible (FALSE);
      qApp->processEvents();


      //  If the average surface is a MISP surface, recompute that also

      remisp (misc, options, &bounds);
    }
}
