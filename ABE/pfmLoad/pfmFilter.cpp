
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



/* System includes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cmath>

/* Local Includes. */
#include "pfmFilter.hpp"
#include "hmpsflag.h"


/* Local Definitions. */

typedef struct
{
    DEPTH_RECORD      *dep;
    BIN_RECORD        bin;
} ROW_RECORD;
  
static ROW_RECORD     *row[3] = {NULL, NULL, NULL};
static NV_INT32       prev_coord_y = -1;
static NV_INT32       row_num[3];


unsigned int          BadSoundings, GoodSoundings, TotalSoundings;



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
*   Arguments:          depth_record    -   depth record array              *
*                       bin             -   bin record                      *
*                                                                           *
*   Return Value:       Number of valid points                              *
*                                                                           *
\***************************************************************************/

void compute_bin_values (DEPTH_RECORD *depth_record, BIN_RECORD *bin)
{
    NV_INT32            filtered_count;
    NV_FLOAT64          sum_filtered, sum2_filtered;
    NV_INT16            validity;



    sum_filtered = 0.0;
    sum2_filtered = 0.0;
    filtered_count = 0;


    for (NV_U_INT32 i = 0 ; i < bin->num_soundings ; i++)
    {
        validity = depth_record[i].validity;


        /*  DO NOT use records marked as file deleted.  */

        if (!(validity & PFM_DELETED))
        {
            if (!(validity & PFM_INVAL))
            {
                sum_filtered += depth_record[i].xyz.z;
                sum2_filtered += (depth_record[i].xyz.z * 
                    depth_record[i].xyz.z);

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
            bin->standard_dev = sqrt ((sum2_filtered - 
                ((NV_FLOAT64) filtered_count * 
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


/********************************************************************
 *
 * Function Name :  InitializeAreaFilter
 *
 * Description : Preform any initialization specific to the area based
 *               filter here.
 *
 * Inputs : None
 *
 * Returns : None
 *
 * Error Conditions : None
 *
 ********************************************************************/
void InitializeAreaFilter (PFM_DEFINITION *pfm_def)
{
    static NV_INT32       prev_width = 0;



    /*  Clear counters showing filter statistics.  */

    BadSoundings = 0;
    GoodSoundings = 0;
    TotalSoundings = 0;


    /*  Initialize the three row buffer for the simple filter.  */

    for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
        if (row[i] != NULL) 
        {
            for (NV_INT32 j = 0 ; j < prev_width ; j++)
            {
                if (row[i][j].dep) free (row[i][j].dep);
            }
            free (row[i]);
        }

        row[i] = (ROW_RECORD *) malloc (pfm_def->open_args.head.bin_width * 
            sizeof (ROW_RECORD));

        if (row[i] == NULL)
        {
            perror ("Allocating memory in InitializeAreaFilter");
            exit (-1);
        }


        for (NV_INT32 j = 0 ; j < pfm_def->open_args.head.bin_width ; j++)
        {
            row[i][j].bin.coord.y = i;
            row[i][j].bin.coord.x = j;

            if (!read_bin_depth_array_index (pfm_def->hnd, &row[i][j].bin, &row[i][j].dep))
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
    prev_width = pfm_def->open_args.head.bin_width;
}


/********************************************************************
 *
 * Function Name :
 *
 * Description :
 *
 * Inputs :
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/
void FinalizeAreaFilter (FILTER_SUMMARY *summary)
{
  summary->TotalSoundings = TotalSoundings;
  summary->GoodSoundings = GoodSoundings;
  summary->BadSoundings = BadSoundings;
}



/********************************************************************
 *
 * Function Name : AreaFilter
 *
 * Description : This function is called by the final loop in pfmLoader
 *               main. Based on the information within the cell, it weeds
 *               out additional problems, and gathers statistics.
 *
 * Inputs : pfm_handle - handle to open PFM file.
 *          coord      - current cell coordinate that we are processing.
 *          bin_record - bin_record about cell we are processing. 
 *          k          - filter argument array index
 *
 * Returns :
 *
 * Error Conditions :
 *
 ********************************************************************/

void AreaFilter (NV_I32_COORD2 *coord, BIN_RECORD *bin_record, PFM_DEFINITION *pfm_def, 
                 NV_FLOAT64 bin_diagonal, FILE *errfp)
{
  NV_INT32               m = 0, n = 0, start_y, end_y, start_k, rc, sumcount, pointcount;
  NV_U_INT32             validity;
  NV_FLOAT64             depth, dx = 0.0, x, y, BinSigmaFilter, sum2, avgsum, stdsum, avg, std, slope;
  NV_BOOL                flat;



  avgsum = 0.0;
  stdsum = 0.0;
  sum2 = 0.0;
  sumcount = 0;
  pointcount = 0;


  /*  Slide the 3 row buffer down one row.  */

  if (coord->y != prev_coord_y && coord->y > 1 && coord->y < pfm_def->open_args.head.bin_height - 1)
    {
      /*  If we moved up more than one row, read all three rows.  Otherwise,
          slide the rows down one row.  */

      end_y = coord->y + 1;
      if (coord->y - prev_coord_y == 1)
        {
          for (NV_INT32 i = 0 ; i < pfm_def->open_args.head.bin_width ; i++)
            {
              if (row[0][i].dep != NULL) free (row[0][i].dep);
              if (row[1][i].dep != NULL)
                {
                  row[0][i].dep = (DEPTH_RECORD *) calloc (row[1][i].bin.num_soundings, sizeof (DEPTH_RECORD));
                  for (NV_U_INT32 j = 0 ; j < row[1][i].bin.num_soundings ; j++)
                    {
                      row[0][i].dep[j] = row[1][i].dep[j];
                    }
                }
              else
                {
                  row[0][i].dep = NULL;
                }
              row[0][i].bin = row[1][i].bin;


              if (row[1][i].dep != NULL) free (row[1][i].dep);
              if (row[2][i].dep != NULL)
                {
                  row[1][i].dep = (DEPTH_RECORD *) calloc (row[2][i].bin.num_soundings, sizeof (DEPTH_RECORD));
                  for (NV_U_INT32 j = 0 ; j < row[2][i].bin.num_soundings ; j++)
                    {
                      row[1][i].dep[j] = row[2][i].dep[j];
                    }
                }
              else
                {
                  row[1][i].dep = NULL;
                }
              row[1][i].bin = row[2][i].bin;
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


      for (NV_INT32 i = start_y, k = start_k ; i <= end_y ; i++, k++)
        {
          for (NV_INT32 j = 0 ; j < pfm_def->open_args.head.bin_width ; j++)
            {
              if (row[k][j].dep != NULL) free (row[k][j].dep);
            }
          free (row[k]);


          row[k] = (ROW_RECORD *) malloc (pfm_def->open_args.head.bin_width * sizeof (ROW_RECORD));

          if (row[k] == NULL)
            {
              perror ("Allocating memory in GetAllDepthRecords");
              exit (-1);
            }

          for (NV_INT32 j = 0 ; j < pfm_def->open_args.head.bin_width ; j++)
            {
              row[k][j].bin.coord.y = i;
              row[k][j].bin.coord.x = j;

              if (!read_bin_depth_array_index (pfm_def->hnd, &row[k][j].bin, &row[k][j].dep))
                {
                  compute_bin_values (row[k][j].dep, &row[k][j].bin);
                }
              else
                {
                  row[k][j].dep = NULL;
                }
            }

          row_num[k] = i;
        }
      prev_coord_y = coord->y;
    }


  /*  Get the information from the 8 cells surrounding this cell.  */

  for (NV_INT32 i = coord->y - 1, k = 0 ; i <= coord->y + 1 ; i++, k++)
    {
      for (NV_INT32 j = coord->x - 1 ; j <= coord->x + 1 ; j++)
        {
          /*  Make sure each cell is in the area (edge effect).  */

          if (i >= 0 && i < pfm_def->open_args.head.bin_height && j >= 0 && j < pfm_def->open_args.head.bin_width)
            {
              /*  If this is the center of the 9 block cell, save the block coordinates.  */

              if (i == coord->y && j == coord->x)
                {
                  m = k;
                  n = j;
                  *bin_record = row[m][n].bin;


                  /*  If the center cell has no valid data, return.  */

                  if (!(bin_record->validity & PFM_DATA)) 
                    {
                      /*  Recompute the bin record based on the contents of the depth array.  */

                      bin_record->validity &= ~PFM_CHECKED;
                      recompute_bin_values_from_depth_index (pfm_def->hnd, bin_record, PFM_CHECKED, row[m][n].dep);


                      /*  Save the recomputed bin data.  */

                      row[m][n].bin = *bin_record;

                      return;
                    }
                }

              if (row[k][j].bin.validity & PFM_DATA)
                {
                  avgsum += row[k][j].bin.avg_filtered_depth;
                  stdsum += row[k][j].bin.standard_dev;
                  sum2 += (row[k][j].bin.avg_filtered_depth * row[k][j].bin.avg_filtered_depth);

                  sumcount++;

                  pointcount += row[k][j].bin.num_soundings;
                }
            }
        }
    }


  y = bin_record->xy.y;
  x = bin_record->xy.x;


  //  Make sure that at least 5 of the surrounding cells have valid data.  This saves us from deleting a lot of edge points
  //  without any real evidence that they were bad.

  if (sumcount < 5) return;


  /*  Compute the eight slopes from the center cell to find out if it's flat enough to use the average of the standard deviations
      or if we need to use the standard deviation of the averages.  We use a reference slope of 1 degree to determine which we 
      need to use.  */

  flat = NVTrue;
  for (NV_INT32 i = coord->y - 1, k = 0 ; i <= coord->y + 1 ; i++, k++)
    {
      for (NV_INT32 j = coord->x - 1 ; j <= coord->x + 1 ; j++)
        {
          /*  Make sure each cell is in the area (edge effect).  */

          if (i >= 0 && i < pfm_def->open_args.head.bin_height && j >= 0 && j < pfm_def->open_args.head.bin_width)
            {
              if (k != m || j != n)
                {
                  if (row[k][j].bin.validity & PFM_DATA)
                    {
                      if ((abs (bin_record->coord.x - row[k][j].bin.coord.x) + abs (bin_record->coord.y -
                                                                                    row[k][j].bin.coord.y)) > 1)
                        {
                          dx = bin_diagonal;
                        }
                      else
                        {
                          if (pfm_def->open_args.head.bin_size_xy == 0.0)
                            {
                              dx = bin_diagonal;
                            }
                          else
                            {
                              dx = pfm_def->open_args.head.bin_size_xy;
                            }
                        }

                      slope = (fabs (bin_record->avg_filtered_depth - row[k][j].bin.avg_filtered_depth)) / dx;

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


  /*
   * For a given bin record, give me the depth data. 
   */

  /*  If the slope is low (< 1 degree) we'll use an average of the cell standard deviations to beat the depths against.
      Otherwise, we'll compute the standard deviation from the cell averages.  Since we're using the average of the
      computed standard deviations of all of the nine cells or the standard deviations of the averages of all nine
      cells we multiply the resulting standard deviation (?) by two to get a reasonable result, otherwise the standard
      deviation surface is too smooth and we end up cutting out too much good data.  I must admit I arrived at these
      numbers by playing with the filter using em3000 shallow water data and em121a deep water data but they appear to
      work properly.  This way three sigma seems to cut out what you would expect three sigma to cut out.  If you
      leave it as is it cuts out about 30%.  This is called empirically determining a value (From Nero's famous
      statement "I'm the emperor and I can do what I damn well please, now hand me my fiddle.").   JCD  */

  avg = avgsum / (NV_FLOAT64) sumcount; 
  if (flat || sumcount < 2)
    {
      std = (stdsum / (NV_FLOAT64) sumcount) * 2.0;
    }
  else
    {
      std = (sqrt ((sum2 - ((NV_FLOAT64) sumcount * (avg * avg))) / ((NV_FLOAT64) sumcount - 1.0))) * 2.0;
    }

  BinSigmaFilter = pfm_def->cellstd * std;


  for (NV_U_INT32 i = 0 ; i < row[m][n].bin.num_soundings ; i++)
    {
      TotalSoundings++;

      depth = row[m][n].dep[i].xyz.z;


      /*
       * If depth record exceeds "user defined" sigma, cut it. 
       * JCD recommended 01-Feb-00, CBL installed.
       */

      /*
       * Only check those that haven't been checked before. 
       */

      if (!(row[m][n].dep[i].validity & (PFM_INVAL | PFM_DELETED)))
        {
          NV_BOOL cut = NVFalse;


          //  Check for deep filter only.

          if (pfm_def->deep_filter_only)
            {
              if (depth - avg >= BinSigmaFilter) cut = NVTrue;
            }
          else
            {
              if (fabs (depth - avg) >= BinSigmaFilter) cut = NVTrue;
            }


          if (cut)
            {
              row[m][n].dep[i].validity |= (PFM_FILTER_INVAL | PFM_MODIFIED);

              rc = update_depth_record_index (pfm_def->hnd, &row[m][n].dep[i]);
              if (rc != SUCCESS)
                {
                  fprintf (errfp, "Error on depth status update.\n");
                  fprintf (errfp, "%s\n", pfm_error_str (rc));
                }
            }
        }

      validity = row[m][n].dep[i].validity;


      if (!(validity & (PFM_INVAL | PFM_DELETED)))
        {
          GoodSoundings++;
        }
      else if (validity & (PFM_INVAL | PFM_DELETED))
        {
          BadSoundings++;
        }
    }


  /*  Recompute the bin record based on the modified contents of the depth array.  */

  bin_record->validity &= ~PFM_CHECKED;
  recompute_bin_values_from_depth_index (pfm_def->hnd, bin_record, PFM_CHECKED, row[m][n].dep);


  /*  Save the recomputed bin data so that it will be used in subsequent filtering operations.  */

  row[m][n].bin = *bin_record;


  return;
}




/*  This took WWAAAAAAYYYYYY too long but we left in the comments */

/********************************************************************
 *
 * Function Name : FeatureProcessing
 *
 * Description : After the data has been marked, let us look around
 * and see if we might have some features that are hidden as unknowns
 * or bad. This routine is based on conversations with Jan Depner, and
 * Barb Reed at NAVO. 
 *
 * The email that encapsulates the historical content of the discussion
 * is included here, these ideas were generated at the conclusion of the
 * first Destin trip.
 *
 *
 * We did a lot of testing with 1, 1.5 and 2 meter features - various speeds and
 * modes.  Dave and Roy counted the number of pings and total "hits" on each
 * feature under each of the conditions.  The conclusion is that the system
 * is definitely "seeing" the feature, but there is so much system noise
 * (especially on HSL 9), that a processor would generally remove the feature
 * with the noise.  After looking at the various ways the system "saw" the
 * features, We came up with a potential scheme of using the number of
 * consecutive pings and total number of hits to possibly use in help  cleaning
 * the data.  The spec for the EM-3000 was 5 pings on a 1 meter feature at 6
 * kts.  The system can do that, it's just that a normal processor would
 * probably edit those points out with the rest of the noise.
 *
 * ---What do you think?
 *
 * The "rules" for invalidating outliers as we see them now are (the numbers
 * may need some tweaking - have to test!):
 *
 * 1.  If the outlier is within (20 meters?) of an identified feature  
 *     - do not invalidate
 * 2.  If the outlier is within (10 meters?) of an outlier on a separate pass
 *     - do not invalidate
 * 3.  If the outliers fall in any of the following scenarios
 *     - do not invalidate
 *         2 consecutive pings with at least 10 total hits
 *         3 consecutive pings with at least 8 total hits
 *         4 consecutive pings with at least 6 total hits
 *         5 consecutive pings with at least 5 total hits
 *         >= 6 consecutive pings
 *
 * 4.  Filter the heck out the rest of the data, otherwise the features will be
 * lost in the noise!
 *
 *
 * Inputs : Binhex Header and 
 *          current load parameters. 
 *
 * Returns : 
 *
 * Error Conditions :
 *
 ********************************************************************/
