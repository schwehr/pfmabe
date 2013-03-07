#include "selectThread.hpp"

selectThread::selectThread (QObject *parent)
  : QThread(parent)
{
}



selectThread::~selectThread ()
{
}



void selectThread::select (OPTIONS *op, NV_INT32 ph, PFM_OPEN_ARGS *oa, FEATURE **f, NV_INT32 p, NV_FLOAT64 ss)
{
  QMutexLocker locker (&mutex);

  l_options = op;
  l_pfm_handle = ph;
  l_open_args = *oa;
  l_features = f;
  l_pass = p;
  l_start_size = ss;

  if (!isRunning ()) start ();
}



void selectThread::run ()
{
  NV_INT32            percent = 0, old_percent = -1, numrecs, num_cols[3], **depth_count, **min_index, features_count;
  NV_U_INT32          hpc_lines[1024];
  NV_F64_COORD2       xy, order[2] = {{1.0, 0.05}, {2.0, 0.10}};
  NV_I32_COORD2       coord;
  NV_BOOL             oct_hit[8];
  NV_F64_COORD3       oct_pos[8];
  POINT_DATA          ***depth = NULL;
  DEPTH_RECORD        *in_depth = NULL;
  NV_FLOAT64          lat, lon, x_bin_size_degrees[3], x_meter_degrees[3], x_two_thirds_bin_degrees[3], min_dist[8];
  QString             string;


  mutex.lock ();

  OPTIONS *options = l_options;
  NV_INT32 pfm_handle = l_pfm_handle;
  PFM_OPEN_ARGS open_args = l_open_args;
  FEATURE **features = l_features;
  NV_INT32 pass = l_pass;
  NV_FLOAT64 start_size = l_start_size;

  mutex.unlock ();


  //qDebug () << __LINE__ << pass;


  //  Allocate the base memory pointers.

  depth = (POINT_DATA ***) calloc (3, sizeof (POINT_DATA **));

  if (depth == NULL)
    {
      perror ("Allocating depth array");
      exit (-1);
    }

  depth_count = (NV_INT32 **) calloc (3, sizeof (NV_INT32 *));

  if (depth_count == NULL)
    {
      perror ("Allocating depth_count array");
      exit (-1);
    }

  min_index = (NV_INT32 **) calloc (3, sizeof (NV_INT32 *));

  if (min_index == NULL)
    {
      perror ("Allocating min_index array");
      exit (-1);
    }


  NV_FLOAT64 bin_size = pow (2.0, (NV_FLOAT64) pass) * start_size;


  features_count = 0;


  //  Figure out what bin_size meters in the north/south direction is in latitude degrees.  We're making the assumption that
  //  curvature is insignificant for distances of 1.5, 3, 6, 12, 24... meters.

  newgp (options->mbr.min_y, options->mbr.min_x, 0.0, 1.0, &lat, &lon);

  NV_FLOAT64 y_meter_degrees = lat - options->mbr.min_y;

  NV_FLOAT64 y_bin_size_degrees = y_meter_degrees * bin_size;

  NV_FLOAT64 y_two_thirds_bin_degrees = y_bin_size_degrees * 2.0 / 3.0;

  NV_FLOAT64 two_thirds_bin_meters = bin_size * 2.0 / 3.0;

  NV_FLOAT64 bin_overlap_meters = two_thirds_bin_meters / 2.0;


  NV_INT32 num_rows = (NV_INT32) ((options->mbr.max_y - options->mbr.min_y) / y_two_thirds_bin_degrees) + 1;


  //  Compute the maximum size of the bin arrays (width / bin_size + 1).  This will either be on the southern edge or
  //  the northern edge depending on hemisphere.  Odds are, it won't matter but you never know.

  if (options->mbr.min_y >= 0.0)
    {
      newgp (options->mbr.min_y, options->mbr.min_x, 90.0, 1.0, &lat, &lon);
    }
  else
    {
      newgp (options->mbr.max_y, options->mbr.min_x, 90.0, 1.0, &lat, &lon);
    }


  //  Figure out what bin_size meters in the east/west direction is in longitude degrees.

  x_meter_degrees[0] = lon - options->mbr.min_x;

  x_bin_size_degrees[0] = x_meter_degrees[0] * bin_size;

  x_two_thirds_bin_degrees[0] = x_bin_size_degrees[0] * 2.0 / 3.0;


  NV_INT32 save_size = (NV_INT32) ((options->mbr.max_x - options->mbr.min_x) / x_two_thirds_bin_degrees[0]) + 1;


  //  Allocate the bin arrays.

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      depth[i] = (POINT_DATA **) calloc (save_size, sizeof (POINT_DATA *));

      if (depth[i] == NULL)
        {
          fprintf(stderr, "%s %d %d\n",__FILE__,__LINE__,save_size);
          perror ("Allocating depth[i] array");
          exit (-1);
        }


      depth_count[i] = (NV_INT32 *) calloc (save_size, sizeof (NV_INT32));

      if (depth_count[i] == NULL)
        {
          perror ("Allocating depth_count[i] array");
          exit (-1);
        }


      min_index[i] = (NV_INT32 *) calloc (save_size, sizeof (NV_INT32));

      if (min_index[i] == NULL)
        {
          perror ("Allocating min_index[i] array");
          exit (-1);
        }


      //  Just in case NULL is not the same as 0.

      for (NV_INT32 j = 0 ; j < save_size ; j++) depth[i][j] = NULL;
    }


  //  Pre-load the first three rows of data.

  //qDebug () << __LINE__ << pass;
  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      //  startlat is the latitude of the bottom of all three rows.

      NV_FLOAT64 startlat = options->mbr.min_y + (NV_FLOAT64) (i / 3) * y_two_thirds_bin_degrees;


      //  newlat is the latitude of the bottom of the current row.

      NV_FLOAT64 newlat = options->mbr.min_y + (NV_FLOAT64) i * y_two_thirds_bin_degrees;


      newgp (newlat, options->mbr.min_x, 90.0, 1.0, &lat, &lon);

      x_meter_degrees[i] = lon - options->mbr.min_x;

      x_bin_size_degrees[i] = x_meter_degrees[i] * bin_size;

      x_two_thirds_bin_degrees[i] = x_bin_size_degrees[i] * 2.0 / 3.0;

      num_cols[i] = (NV_INT32) ((options->mbr.max_x - options->mbr.min_x) / x_two_thirds_bin_degrees[i]) + 1;


      //  Compute the start and end rows (in the PFM) for the input depth records.  Since we're making rows of bin_size sized
      //  bins for this, and we don't know where these bins will be in the PFM, we have to figure out how many
      //  rows of PFM bins we need to read to fill our bin_size bins.

      xy.x = options->mbr.min_x;
      xy.y = newlat;

      compute_index_ptr (xy, &coord, &open_args.head);

      NV_INT32 start_col = coord.x;
      NV_INT32 start_row = coord.y;


      xy.x = options->mbr.max_x;
      xy.y = newlat + y_bin_size_degrees;

      compute_index_ptr (xy, &coord, &open_args.head);

      NV_INT32 end_col = coord.x;
      NV_INT32 end_row = coord.y;

      //qDebug () << __LINE__ << pass << i << start_col << end_col << start_row << end_row;

      for (NV_INT32 j = start_row ; j <= end_row ; j++)
        {
          coord.y = j;

          for (NV_INT32 k = start_col ; k <= end_col ; k++)
            {
              coord.x = k;


              BIN_RECORD bin;
              read_bin_record_index (pfm_handle, coord, &bin);

              if (bin.num_soundings)
                {
                  //  Read the depth chain for the selected PFM bin.

                  //qDebug () << __LINE__ << pass << j << k;

                  if (!read_depth_array_index (pfm_handle, coord, &in_depth, &numrecs))
                    {
                      //qDebug () << __LINE__ << numrecs << pass;
                      for (NV_INT32 m = 0 ; m < numrecs ; m++)
                        {
                          //  Don't use invalid, deleted, or reference points.

                          if (!(in_depth[m].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                            {
                              //  Add in the offset (if any).

                              in_depth[m].xyz.z += l_options->offset;


                              //  Check for above zero inclusion.

                              if (options->zero || in_depth[m].xyz.z >= 0.0)
                                {
                                  //  The points may fall in 1, 2, or 4 bins due to overlap.  We have to figure out which
                                  //  third of the bin, in both X and Y, that the point is in so that we can tell how many
                                  //  total bins it will fall in.

                                  NV_FLOAT64 x_meter = (in_depth[m].xyz.x - options->mbr.min_x) / x_meter_degrees[i];
                                  NV_FLOAT64 y_meter = (in_depth[m].xyz.y - startlat) / y_meter_degrees;


                                  //  These are the indices for the main bin that this point falls in (lower 2/3 of bin determines
                                  //  main bin).

                                  NV_INT32 col = (NV_INT32) (x_meter / two_thirds_bin_meters);
                                  NV_INT32 row = (NV_INT32) (y_meter / two_thirds_bin_meters);


                                  //  Make sure we're inside our latest latitude band.

                                  if (x_meter >= 0.0 && y_meter >= 0.0 && row >= 0 && row <= 3 && col >= 0 && col <= (num_cols[i] - 1))
                                    {
#ifdef __PFM_FEATURE_DEBUG__
                                      if (col >= save_size)
                                        {
                                          fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,col,save_size);
                                          exit (-1);
                                        }
#endif

                                      //  If the row == 3 then this point MAY be in the top third of the top row so we want to check it
                                      //  and add it to row 2 if it is.

                                      if (row < 3 || (row == 3 && fmod (y_meter, two_thirds_bin_meters) < bin_overlap_meters))
                                        {
                                          NV_BOOL top_row = NVFalse;
                                          if (row == 3)
                                            {
                                              top_row = NVTrue;
                                              row = 2;
                                            }

                                          depth[row][col] = (POINT_DATA *) realloc (depth[row][col], (depth_count[row][col] + 1) * sizeof (POINT_DATA));

                                          if (depth[row][col] == NULL)
                                            {
                                              perror ("Allocating depth[row][col] array");
                                              exit (-1);
                                            }

                                          depth[row][col][depth_count[row][col]].xyz = in_depth[m].xyz;
                                          depth[row][col][depth_count[row][col]].l = in_depth[m].line_number;
                                          depth[row][col][depth_count[row][col]].h = in_depth[m].horizontal_error;
                                          depth[row][col][depth_count[row][col]].v = in_depth[m].vertical_error;

                                          depth_count[row][col]++;


                                          //  Check for X overlap.

                                          NV_BOOL x_overlap = NVFalse;
                                          NV_BOOL y_overlap = NVFalse;
                                          if (fmod (x_meter, two_thirds_bin_meters) < bin_overlap_meters)
                                            {
                                              //  If the main bin is not at 0, we have a point in the previous bin.

                                              if (col)
                                                {
                                                  x_overlap = NVTrue;

                                                  depth[row][col - 1] = (POINT_DATA *) realloc (depth[row][col - 1], (depth_count[row][col - 1] + 1) *
                                                                                                sizeof (POINT_DATA));

                                                  if (depth[row][col - 1] == NULL)
                                                    {
                                                      perror ("Allocating depth[row][col - 1] array");
                                                      exit (-1);
                                                    }

                                                  depth[row][col - 1][depth_count[row][col - 1]].xyz = in_depth[m].xyz;
                                                  depth[row][col - 1][depth_count[row][col - 1]].l = in_depth[m].line_number;
                                                  depth[row][col - 1][depth_count[row][col - 1]].h = in_depth[m].horizontal_error;
                                                  depth[row][col - 1][depth_count[row][col - 1]].v = in_depth[m].vertical_error;

                                                  depth_count[row][col - 1]++;
                                                }
                                            }


                                          //  Check for Y overlap (only if not in top third of top row).

                                          if (!top_row && fmod (y_meter, two_thirds_bin_meters) < bin_overlap_meters)
                                            {
                                              //  If the main bin is not at 0, we have a point in the previous bin.

                                              if (row)
                                                {
                                                  y_overlap = NVTrue;

                                                  depth[row - 1][col] = (POINT_DATA *) realloc (depth[row - 1][col], (depth_count[row - 1][col] + 1) *
                                                                                                sizeof (POINT_DATA));

                                                  if (depth[row - 1][col] == NULL)
                                                    {
                                                      perror ("Allocating depth[row - 1][col] array");
                                                      exit (-1);
                                                    }

                                                  depth[row - 1][col][depth_count[row - 1][col]].xyz = in_depth[m].xyz;
                                                  depth[row - 1][col][depth_count[row - 1][col]].l = in_depth[m].line_number;
                                                  depth[row - 1][col][depth_count[row - 1][col]].h = in_depth[m].horizontal_error;
                                                  depth[row - 1][col][depth_count[row - 1][col]].v = in_depth[m].vertical_error;

                                                  depth_count[row - 1][col]++;
                                                }
                                            }


                                          //  If both X and Y had overlap then we also need to add it to the bin left and below this
                                          //  bin.

                                          if (x_overlap && y_overlap && row && col)
                                            {
                                              depth[row - 1][col - 1] = (POINT_DATA *) realloc (depth[row - 1][col - 1], (depth_count[row - 1][col - 1] + 1) *
                                                                                                sizeof (POINT_DATA));

                                              if (depth[row - 1][col - 1] == NULL)
                                                {
                                                  perror ("Allocating depth[row - 1][col - 1] array");
                                                  exit (-1);
                                                }

                                              depth[row - 1][col - 1][depth_count[row - 1][col - 1]].xyz = in_depth[m].xyz;
                                              depth[row - 1][col - 1][depth_count[row - 1][col - 1]].l = in_depth[m].line_number;
                                              depth[row - 1][col - 1][depth_count[row - 1][col - 1]].h = in_depth[m].horizontal_error;
                                              depth[row - 1][col - 1][depth_count[row - 1][col - 1]].v = in_depth[m].vertical_error;

                                              depth_count[row - 1][col - 1]++;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                      free (in_depth);
                    }
                }
            }
        }
    }
  //qDebug () << __LINE__ << pass;


  //  Compute mins for the first three rows.

  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      for (NV_INT32 j = 0 ; j < num_cols[i] ; j++)
        {
          //  Find the minimum depth.

          NV_FLOAT64 min_depth = 999999999.0;
          min_index[i][j] = -1;

          for (NV_INT32 k = 0 ; k < depth_count[i][j] ; k++)
            {

#ifdef __PFM_FEATURE_DEBUG__
              if (j >= save_size)
                {
                  fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,j,save_size);
                  exit (-1);
                }
#endif

              if (depth[i][j][k].xyz.z <= min_depth)
                {
                  min_depth = depth[i][j][k].xyz.z;
                  min_index[i][j] = k;
                }
            }
        }
    }

  //qDebug () << __LINE__ << pass;

  //  Here we loop through the entire PFM looking for minimums in each bin and comparing them to maximums in
  //  8 octants to see if we have a possible feature.  Once per row of bin_size sized bins (with 1/3 bin_size 
  //  overlap in both X and Y directions).

  for (NV_INT32 i = 0 , p = 0; i < num_rows ; i++)
    {
      if (!i)
        {
          p = 0;
        }
      else if (i == num_rows - 2)
        {
          p = 2;
        }
      else
        {
          p = 1;
        }

      for (NV_INT32 j = 0 ; j < num_cols[p] ; j++)
        {
          NV_INT32 start_row = qMax (p - 1, 0);
          NV_INT32 end_row = qMin (p + 1, 2);

          NV_INT32 start_col = qMax (j - 1, 0);
          NV_INT32 end_col = qMin (j + 1, num_cols[p] - 1);


          //  Check to make sure that we found some points.

          if (min_index[p][j] >= 0 && depth_count[p][j])
            {

#ifdef __PFM_FEATURE_DEBUG__
              if (j >= save_size)
                {
                  fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,j,save_size);
                  exit (-1);
                }
#endif
              lat = depth[p][j][min_index[p][j]].xyz.y;
              lon = depth[p][j][min_index[p][j]].xyz.x;
              NV_FLOAT64 min_depth = depth[p][j][min_index[p][j]].xyz.z;
              NV_FLOAT32 h = depth[p][j][min_index[p][j]].h;
              NV_FLOAT32 v = depth[p][j][min_index[p][j]].v;


              memset (oct_hit, 0, sizeof (oct_hit));
              memset (oct_pos, 0, sizeof (oct_pos));
              for (NV_INT32 oct = 0 ; oct < 8 ; oct++) min_dist[oct] = 9999999999999999.0L;
              NV_INT32 oct_hits = 0;

              NV_FLOAT64 cut = order[options->order].x;
              if (min_depth > 40.0) cut = min_depth * order[options->order].y;

              for (NV_INT32 k = start_row ; k <= end_row ; k++)
                {
                  for (NV_INT32 m = start_col ; m <= end_col ; m++)
                    {
                      //  Now, determine octant in relation to the minimum for each depth point.

                      for (NV_INT32 n = 0 ; n < depth_count[k][m] ; n++)
                        {

#ifdef __PFM_FEATURE_DEBUG__
                          if (m >= save_size)
                            {
                              fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,m,save_size);
                              exit (-1);
                            }
#endif

                          //  Only if we exceed the order[options->order] do we compute the octant.

                          if (depth[k][m][n].xyz.z - min_depth >= cut)
                            {
                              //  Compute the distance between the points to see if we're outside the combined horizontal
                              //  uncertainty of the two points.

                              NV_FLOAT64 dist;

                              pfm_geo_distance (pfm_handle, lat, lon, depth[k][m][n].xyz.y, depth[k][m][n].xyz.x, &dist);


                              if (dist >= depth[k][m][n].h + h)
                                {
                                  NV_INT32 octant = get_octant (lon, lat, depth[k][m][n].xyz.x, depth[k][m][n].xyz.y);


                                  //  If we haven't hit this octant before, mark it as a hit and increment the octant count.

                                  if (!oct_hit[octant])
                                    {
                                      oct_hits++;
                                      oct_hit[octant] = NVTrue;


                                      //  We will break out of the loop if we reach 8 octant hits but only if we aren't generating
                                      //  estimated polygon shapes.

                                      if (!options->output_polygons && oct_hits >= 8) break;
                                    }


                                  //  Find the closest octant points that exceed our cutoff.  This will allow us
                                  //  to come up with an estimated shape for the feature.

                                  if (options->output_polygons && dist < min_dist[octant])
                                    {
                                      min_dist[octant] = dist;
                                      oct_pos[octant] = depth[k][m][n].xyz;
                                    }
                                }
                            }
                        }


                      //  We will break out of the loop if we reach 8 octant hits but only if we aren't generating
                      //  estimated polygon shapes.

                      if (!options->output_polygons && oct_hits >= 8) break;
                    }


                  //  We will break out of the loop if we reach 8 octant hits but only if we aren't generating
                  //  estimated polygon shapes.

                  if (!options->output_polygons && oct_hits >= 8) break;
                }


              //  If we got a hit in all 8 octants we are, obviously, good to go.  However, if we got hits in
              //  the following combinations of two octants we will also call it good:
              //
              //                 0,1   2,3   4,5   6,7   7,0   1,2   3,4   5,6
              //
              //  This is like breaking it up into 4 quadrants of 90 degrees each and then shifting 45 degrees
              //  and breaking that up into 4 quadrants of 90 degrees each.  This is what allows us to find 
              //  ridges or pipelines.

              if (oct_hits >= 8 || ((oct_hit[0] || oct_hit[1]) && (oct_hit[2] || oct_hit[3]) &&
                                    (oct_hit[4] || oct_hit[5]) && (oct_hit[6] || oct_hit[7]) &&
                                    (oct_hit[7] || oct_hit[0]) && (oct_hit[1] || oct_hit[2]) &&
                                    (oct_hit[3] || oct_hit[4]) && (oct_hit[5] || oct_hit[6])))
                {
                  features[pass] = (FEATURE *) realloc (features[pass], (features_count + 1) * sizeof (FEATURE));

                  if (features[pass] == NULL)
                    {
                      perror ("Allocating features[pass] array");
                      exit (-1);
                    }

                  features[pass][features_count].x = lon;
                  features[pass][features_count].y = lat;
                  features[pass][features_count].z = min_depth;
                  features[pass][features_count].h = h;
                  features[pass][features_count].v = v;
                  features[pass][features_count].confidence = 3;


                  //  Save the 6 to 8 closest points to the possible feature in each of 8 octants.

                  for (NV_INT32 oct = 0 ; oct < 8 ; oct++)
                    {
                      features[pass][features_count].oct_hit[oct] = oct_hit[oct];
                      features[pass][features_count].oct_pos[oct] = oct_pos[oct];
                    }


                  //  Compute the confidence based on the HPC (Hockey Puck of Confidence (TM)).

                  NV_FLOAT32 v_cut = features[pass][features_count].v * 2.0;
                  NV_FLOAT32 h_cut = features[pass][features_count].h * 2.0;

                  if (options->hpc)
                    {
                      NV_INT32 hpc_count = 0;
                      NV_INT32 hpc_line_count = 0;
                      for (NV_INT32 k = start_row ; k <= end_row ; k++)
                        {
                          for (NV_INT32 m = start_col ; m <= end_col ; m++)
                            {
                              //  Now, determine octant in relation to the minimum for each depth point.

                              for (NV_INT32 n = 0 ; n < depth_count[k][m] ; n++)
                                {
                                  if (fabs (depth[k][m][n].xyz.z - features[pass][features_count].z) <= v_cut)
                                    {
                                      pfm_geo_distance (pfm_handle, features[pass][features_count].y, features[pass][features_count].x,
                                                        depth[k][m][n].xyz.y, depth[k][m][n].xyz.x, &cut);

                                      if (cut <= h_cut)
                                        {
                                          NV_BOOL hit = NVFalse;
                                          for (NV_INT32 p = 0 ; p < hpc_line_count ; p++)
                                            {
                                              if (depth[k][m][n].l == hpc_lines[p])
                                                {
                                                  hit = NVTrue;
                                                  break;
                                                }
                                            }


                                          hpc_count++;


                                          if (!hit)
                                            {
                                              hpc_lines[hpc_line_count] = depth[k][m][n].l;
                                              hpc_line_count++;

                                              if (hpc_count > 2 && hpc_line_count > 1)
                                                {
                                                  features[pass][features_count].confidence = 5;
                                                  break;
                                                }
                                            }
                                        }
                                    }
                                }

                              if (hpc_count > 2 && hpc_line_count > 1) break;
                            }

                          if (hpc_count > 2 && hpc_line_count > 1) break;
                        }
                    }


                  features_count++;
                  //qDebug () << __LINE__ << features_count << pass;
                }
            }
        }


      //  Move everything down one row (in the arrays) and then get the next row.   We only want to move up a row after
      //  we have dealt with the first row.

      if (i)
        {
          for (NV_INT32 j = 1 ; j < 3 ; j++)
            {
              for (NV_INT32 k = 0 ; k < num_cols[j] ; k++)
                {

#ifdef __PFM_FEATURE_DEBUG__
                  if (k >= save_size)
                    {
                      fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,k,save_size);
                      exit (-1);
                    }
#endif

                  if (depth_count[j][k])
                    {
                      depth[j - 1][k] = (POINT_DATA *) realloc (depth[j - 1][k], depth_count[j][k] * sizeof (POINT_DATA));

                      if (depth[j - 1][k] == NULL)
                        {
                          perror ("Allocating depth[j - 1][k] array");
                          exit (-1);
                        }


                      for (NV_INT32 m = 0 ; m < depth_count[j][k] ; m++) depth[j - 1][k][m] = depth[j][k][m];

                      min_index[j - 1][k] = min_index[j][k];
                      depth_count[j - 1][k] = depth_count[j][k];
                    }
                  else
                    {
                      depth[j - 1][k] = NULL;
                      min_index[j - 1][k] = -1;
                      depth_count[j - 1][k] = 0;
                    }
                }

              x_meter_degrees[j - 1] = x_meter_degrees[j];
              x_bin_size_degrees[j - 1] = x_bin_size_degrees[j];
              x_two_thirds_bin_degrees[j - 1] = x_two_thirds_bin_degrees[j];
              num_cols[j - 1] = num_cols[j];
            }


          //  Clear the new [2] arrays.

          for (NV_INT32 k = 0 ; k < num_cols[2] ; k++)
            {

#ifdef __PFM_FEATURE_DEBUG__
              if (k >= save_size)
                {
                  fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,k,save_size);
                  exit (-1);
                }
#endif

              if (depth[2][k])
                {
                  free (depth[2][k]);
                }
              depth[2][k] = NULL;
              depth_count[2][k] = 0;
              min_index[2][k] = -1;
            }


          //  This is the beginning of the next row which is based on two thirds of the bin size.

          NV_FLOAT64 newlat = options->mbr.min_y + (NV_FLOAT64) (i + (3 - p)) * y_two_thirds_bin_degrees;

          newgp (newlat, options->mbr.min_x, 90.0, 1.0, &lat, &lon);

          x_meter_degrees[2] = lon - options->mbr.min_x;

          x_bin_size_degrees[2] = x_meter_degrees[2] * bin_size;

          x_two_thirds_bin_degrees[2] = x_bin_size_degrees[2] * 2.0 / 3.0;

          num_cols[2] = (NV_INT32) ((options->mbr.max_x - options->mbr.min_x) / x_two_thirds_bin_degrees[2]) + 1;


          //  Compute the start and end rows (in the PFM) for the input depth records.  Since we're making a row of bin_size sized
          //  bins for this, and we don't know where these bins will be in the PFM, we have to figure out how many
          //  rows of PFM bins we need to read to fill our bin_size bins.

          xy.x = options->mbr.min_x;
          xy.y = newlat;

          compute_index_ptr (xy, &coord, &open_args.head);

          NV_INT32 start_col = coord.x;
          NV_INT32 start_row = coord.y;


          xy.x = options->mbr.max_x;
          xy.y = newlat + y_bin_size_degrees;

          compute_index_ptr (xy, &coord, &open_args.head);

          NV_INT32 end_col = coord.x;
          NV_INT32 end_row = qMin (coord.y, open_args.head.bin_height - 1);


          //  This is where we want to compute the row number from since we want data to be stored in the [2] array.
          //  We only want to add data to the [2] array since the data in the overlap area of the [1] array has
          //  already been loaded.  We have redundant reads due to this but this way we don't have to run through array
          //  [1] looking for data in the overlap area with array [2].

          NV_FLOAT64 startlat = newlat - (2.0 * y_two_thirds_bin_degrees);


          for (NV_INT32 j = start_row ; j <= end_row ; j++)
            {
              coord.y = j;

              for (NV_INT32 k = start_col ; k <= end_col ; k++)
                {
                  coord.x = k;


                  BIN_RECORD bin;
                  read_bin_record_index (pfm_handle, coord, &bin);

                  if (bin.num_soundings)
                    {
                      if (!read_depth_array_index (pfm_handle, coord, &in_depth, &numrecs))
                        {
                          for (NV_INT32 m = 0 ; m < numrecs ; m++)
                            {
                              //  Don't use invalid, deleted, or reference points.

                              if (!(in_depth[m].validity & (PFM_INVAL | PFM_DELETED | PFM_REFERENCE)))
                                {
                                  //  Add in the offset (if any).

                                  in_depth[m].xyz.z += l_options->offset;


                                  //  Check for above zero inclusion.

                                  if (options->zero || in_depth[m].xyz.z >= 0.0)
                                    {
                                      //  The points may fall in 1, 2, or 4 bins due to overlap.  We have to figure out which
                                      //  third of the bin, in both X and Y, that the point is in so that we can tell how many
                                      //  total bins it will fall in.

                                      NV_FLOAT64 x_meter = (in_depth[m].xyz.x - options->mbr.min_x) / x_meter_degrees[2];
                                      NV_FLOAT64 y_meter = (in_depth[m].xyz.y - startlat) / y_meter_degrees;


                                      //  These are the indices for the main bin that this point falls in.

                                      NV_INT32 col = (NV_INT32) (x_meter / two_thirds_bin_meters);
                                      NV_INT32 row = (NV_INT32) (y_meter / two_thirds_bin_meters);


                                      //  Make sure we're only in the [2] array since we've already loaded the [1] array.
                                      //  Yeah, I know, if I was clever I could get 1/3 of the data from the [1] array.
                                      //  That would be a major PITA.

                                      if (x_meter >= 0.0 && (row == 2 || row == 3) && col >= 0 && col <= (num_cols[2] - 1))
                                        {

#ifdef __PFM_FEATURE_DEBUG__
                                          if (col >= save_size)
                                            {
                                              fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,col,save_size);
                                              exit (-1);
                                            }
#endif

                                          //  If the row == 3 then this point MAY be in the top third of the top row so we want to check it
                                          //  and add it to row 2 if it is.

                                          if (row < 3 || (row == 3 && fmod (y_meter, two_thirds_bin_meters) < bin_overlap_meters))
                                            {
                                              NV_BOOL top_row = NVFalse;
                                              if (row == 3)
                                                {
                                                  top_row = NVTrue;
                                                  row = 2;
                                                }

                                              depth[row][col] = (POINT_DATA *) realloc (depth[row][col], (depth_count[row][col] + 1) *
                                                                                        sizeof (POINT_DATA));

                                              if (depth[row][col] == NULL)
                                                {
                                                  perror ("Allocating depth[row][col] array");
                                                  exit (-1);
                                                }

                                              depth[row][col][depth_count[row][col]].xyz = in_depth[m].xyz;
                                              depth[row][col][depth_count[row][col]].l = in_depth[m].line_number;
                                              depth[row][col][depth_count[row][col]].h = in_depth[m].horizontal_error;
                                              depth[row][col][depth_count[row][col]].v = in_depth[m].vertical_error;

                                              depth_count[row][col]++;


                                              //  Check for X overlap.

                                              NV_BOOL x_overlap = NVFalse;
                                              NV_BOOL y_overlap = NVFalse;
                                              if (fmod (x_meter, two_thirds_bin_meters) < bin_overlap_meters)
                                                {
                                                  //  If the main bin is not at 0, we have a point in the previous bin.

                                                  if (col)
                                                    {
                                                      x_overlap = NVTrue;

                                                      depth[row][col - 1] = (POINT_DATA *) realloc (depth[row][col - 1], (depth_count[row][col - 1] + 1) *
                                                                                                    sizeof (POINT_DATA));

                                                      if (depth[row][col - 1] == NULL)
                                                        {
                                                          perror ("Allocating depth[row][col - 1] array");
                                                          exit (-1);
                                                        }

                                                      depth[row][col - 1][depth_count[row][col - 1]].xyz = in_depth[m].xyz;
                                                      depth[row][col - 1][depth_count[row][col - 1]].l = in_depth[m].line_number;
                                                      depth[row][col - 1][depth_count[row][col - 1]].h = in_depth[m].horizontal_error;
                                                      depth[row][col - 1][depth_count[row][col - 1]].v = in_depth[m].vertical_error;

                                                      depth_count[row][col - 1]++;
                                                    }
                                                }


                                              //  Check for Y overlap (only if not in top third of top row).

                                              if (!top_row && fmod (y_meter, two_thirds_bin_meters) < bin_overlap_meters)
                                                {
                                                  //  If the main bin is not at 0, we have a point in the previous bin.

                                                  if (row)
                                                    {
                                                      y_overlap = NVTrue;

                                                      depth[row - 1][col] = (POINT_DATA *) realloc (depth[row - 1][col], (depth_count[row - 1][col] + 1) *
                                                                                                    sizeof (POINT_DATA));

                                                      if (depth[row - 1][col] == NULL)
                                                        {
                                                          perror ("Allocating depth[row - 1][col] array");
                                                          exit (-1);
                                                        }

                                                      depth[row - 1][col][depth_count[row - 1][col]].xyz = in_depth[m].xyz;
                                                      depth[row - 1][col][depth_count[row - 1][col]].l = in_depth[m].line_number;
                                                      depth[row - 1][col][depth_count[row - 1][col]].h = in_depth[m].horizontal_error;
                                                      depth[row - 1][col][depth_count[row - 1][col]].v = in_depth[m].vertical_error;

                                                      depth_count[row - 1][col]++;
                                                    }
                                                }


                                              //  If both X and Y had overlap then we also need to add it to the bin left and below
                                              //  this bin.

                                              if (x_overlap && y_overlap && row && col)
                                                {
                                                  depth[row - 1][col - 1] = (POINT_DATA *) realloc (depth[row - 1][col - 1], (depth_count[row - 1][col - 1] + 1) *
                                                                                                    sizeof (POINT_DATA));

                                                  if (depth[row - 1][col - 1] == NULL)
                                                    {
                                                      perror ("Allocating depth[row - 1][col - 1] array");
                                                      exit (-1);
                                                    }

                                                  depth[row - 1][col - 1][depth_count[row - 1][col - 1]].xyz = in_depth[m].xyz;
                                                  depth[row - 1][col - 1][depth_count[row - 1][col - 1]].l = in_depth[m].line_number;
                                                  depth[row - 1][col - 1][depth_count[row - 1][col - 1]].h = in_depth[m].horizontal_error;
                                                  depth[row - 1][col - 1][depth_count[row - 1][col - 1]].v = in_depth[m].vertical_error;

                                                  depth_count[row - 1][col - 1]++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                          free (in_depth);
                        }
                    }
                }
            }


          //  Compute mins for the new row.

          for (NV_INT32 j = 0 ; j < num_cols[2] ; j++)
            {
              //  Find the minimum depth.

              NV_FLOAT64 min_depth = 999999999.0;
              min_index[2][j] = -1;

              for (NV_INT32 k = 0 ; k < depth_count[2][j] ; k++)
                {

#ifdef __PFM_FEATURE_DEBUG__
                  if (j >= save_size)
                    {
                      fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,j,save_size);
                      exit (-1);
                    }
#endif

                  if (depth[2][j][k].xyz.z < min_depth)
                    {
                      min_depth = depth[2][j][k].xyz.z;
                      min_index[2][j] = k;
                    }
                }
            }
        }


      percent = NINT (((NV_FLOAT32) i / (NV_FLOAT32) num_rows) * 100.0);
      if (percent != old_percent)
        {
          emit percentValue (percent, pass);
          qApp->processEvents ();

          old_percent = percent;
        }
    }


  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      for (NV_INT32 j = 0 ; j < num_cols[i] ; j++)
        {

#ifdef __PFM_FEATURE_DEBUG__
          if (j >= save_size)
            {
              fprintf(stderr,"%s %d %d %d\n",__FILE__,__LINE__,j,save_size);
              exit (-1);
            }
#endif

          if (depth[i][j]) free (depth[i][j]);
        }

      if (depth_count[i]) free (depth_count[i]);
      if (min_index[i]) free (min_index[i]);

      if (depth[i]) free (depth[i]);
    }


  //qDebug () << __LINE__ << features_count << pass;

  emit completed (features_count, pass);
  qApp->processEvents ();
}



/*

    This returns the octant that x1, y1 is in, in relation to x0, y0.  Sort of like this:

                                            |
                                    \    7  |  0    /
                                      \     |     /
                                    6   \   |   /    1
                                          \ | /
                                ------------*------------
                                          / | \
                                    5   /   |   \    2
                                      /     |     \
                                    /    4  |  3    \
                                            |

    Where the asterisk (*) represents the point x0, y0.

*/

NV_U_BYTE selectThread::get_octant (NV_FLOAT64 x0, NV_FLOAT64 y0, NV_FLOAT64 x1, NV_FLOAT64 y1)
{
  NV_FLOAT32 slope;


  //  Either directly north or directly south.

  if (x1 - x0 == 0.0)
    {
      //  North.

      if (y1 - y0 > 0.0) return (0);


      //  South.

      return (4);
    }
  else
    {
      slope = (y1 - y0) / (x1 - x0);


      //  North hemisphere or directly east or west.

      if (x1 - x0 >= 0.0)
        {
          if (slope > 1.0) return (0);

          if (slope >= 0.0) return (1);

          if (slope < -1.0) return (3);

          return (2);
        }


      //  South hemisphere.

      else
        {
          if (slope > 1.0) return (4);

          if (slope > 0.0) return (5);

          if (slope > -1.0) return (6);

          return (7);
        }
    }


  //  Should never get here.  This is just to remove warnings at compile time.

  return (0);
}
