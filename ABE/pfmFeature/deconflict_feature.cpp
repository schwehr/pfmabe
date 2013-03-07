#include "pfmFeature.hpp"


//  This is the sort function for qsort.

static NV_INT32 compare_depths (const void *a, const void *b)
{
    NV_F64_COORD3 *sa = (NV_F64_COORD3 *)(a);
    NV_F64_COORD3 *sb = (NV_F64_COORD3 *)(b);

    return (sa->z < sb->z ? 0 : 1);
}



//  Deconflict the possible features.

NV_INT32 deconflict_feature (NV_F64_COORD2 order, NV_INT32 passes, NV_INT32 pfm_handle, OPTIONS *options,
                            NV_INT32 *features_count, FEATURE **features, RUN_PROGRESS *progress)
{
  NV_INT32             percent = 0, old_percent = -1;
  NV_FLOAT64           bin_size[2], cut;
  QString              string;


  //  IMPORTANT ASSUMPTION: For the 1.5 or 3.0 meter bin size we are assuming that the object may be as small as a one or 
  //  two meter cube.  Given this assumption we will search for conflicts at either 1.414 or 2.83 meters.  This is the 
  //  diagonal of a one or two meter square.  For the larger features we will use the diagonal of the bin size.  Is this 
  //  correct?  What does blue sound like?  How hot is tweed?  Who the hell knows!


  NV_INT32 decon_count = 0;


  //  We need to sort our "passes" arrays of possible features by depth.

  for (NV_INT32 i = 0 ; i < passes ; i++)
    {
      if (features_count[i]) qsort (features[i], features_count[i], sizeof (FEATURE), compare_depths);
    }



  //  If we had an area file we want to invalidate any points that fall outside our area.

  if (options->polygon_count)
    {
      for (NV_INT32 i = 0 ; i < passes ; i++)
        {
          for (NV_INT32 j = 0 ; j < features_count[i] ; j++)
            {
              if (!inside (options->polygon_x, options->polygon_y, options->polygon_count, features[i][j].x, features[i][j].y))
                {
                  features[i][j].z = -14000.0;
                  decon_count++;
                }
            }
        }
    }


  //  Now deconflict the features in each array with the other features in the same array.  The rationale is:
  //
  //  1) take shoalest feature
  //  2) compare to all other features
  //  3) if another feature is within bin_size * 1.414 meters (3, 6, or 12 meters) of the shoaler feature, mark the feature as invalid
  //  4) wash, rinse, repeat

  for (NV_INT32 i = 0 ; i < passes ; i++)
    {
      bin_size[0] = pow (2.0, (NV_FLOAT64) i) * 1.5 * order.x;


      //  For the first pass of a special order or order one deconfliction we want to limit the search radius
      //  to the diagonal of the cube.  For subsequent passes or order two we use twice the diagonal of the bin size.

      NV_FLOAT64 radius = bin_size[0] * 1.414;
      if (!i && order.x <= 2.0) radius = order.x * 1.414;


      string.sprintf (pfmFeature::tr ("Feature deconfliction, pass %d of %d, %.3f meter search radius").toAscii (), i + 1, passes, radius);
      progress->dbox->setTitle (string);
      qApp->processEvents ();


      if (features_count[i] > 1)
        {
          for (NV_INT32 j = 0 ; j < features_count[i] ; j++)
            {
              if (features[i][j].z > -13000.0)
                {
                  for (NV_INT32 k = j + 1 ; k < features_count[i] ; k++)
                    {
                      if (features[i][k].z > -13000.0)
                        {
                          pfm_geo_distance (pfm_handle, features[i][j].y, features[i][j].x, features[i][k].y, features[i][k].x, &cut);

                          if (cut < radius)
                            {
                              decon_count++;
                              features[i][k].z = -14000.0;
                            }
                        }
                    }
                }


              percent = NINT (((NV_FLOAT32) j / (NV_FLOAT32) features_count[i]) * 100.0);
              if (percent != old_percent)
                {
                  progress->dbar->setValue (percent);
                  old_percent = percent;
                }

              qApp->processEvents ();
            }

          progress->dbar->setValue (100);
          qApp->processEvents ();
        }
    }


  //  Deconflict all bin sizes against all other bin sizes.

  for (NV_INT32 i = 0 ; i < passes ; i++)
    {
      //  Make sure we have some possible features in this bin size.

      if (features_count[i])
        {
          //  Compute the bin size (starts at 1.5 or 3.0 and doubles going up - 1.5, 3, 6, 12, 24...)

          bin_size[0] = pow (2.0, (NV_FLOAT64) i) * 1.5 * order.x;


          //  Inner loop.

          for (NV_INT32 j = 0 ; j < passes ; j++)
            {
              //  Make sure we have some possible features in this bin size.

              if (j != i && features_count[j])
                {
                  //  Compute the bin size (starts at 1.5 or 3.0 and doubles going up - 3, 6, 12, 24...)

                  bin_size[1] = pow (2.0, (NV_FLOAT64) j) * 1.5 * order.x;


                  //  We only want to compare smaller bin possible features to larger bin possible features.

                  if (bin_size[0] < bin_size[1])
                    {
                      string.sprintf (pfmFeature::tr ("Feature deconfliction, pass %d of %d, %.1fm bin vs %.1fm bin").toAscii (),
				      i + 1, passes, bin_size[0], bin_size[1]);
                      progress->dbox->setTitle (string);
                      qApp->processEvents ();


                      //  For the first pass of a special order or order one deconfliction we want to limit the search radius
                      //  to the diagonal of the cube.  For subsequent passes or order two we use twice the diagonal of the bin
                      //  size.  Note that we're using the [i] loop to determine the search radius.

                      NV_FLOAT64 radius = bin_size[0] * 1.414;
                      if (!i && order.x <= 2.0) radius = order.x * 1.414;


                      //  Loop through the first possible feature list (remember, it's sorted shoalest to deepest).

                      for (NV_INT32 k = 0 ; k < features_count[i] ; k++)
                        {
                          //  Make sure the possible feature is valid (we may have invalidated it in a previous pass).

                          if (features[i][k].z > -13000.0)
                            {
                              //  Loop through the second possible feature list.

                              for (NV_INT32 m = 0 ; m < features_count[j] ; m++)
                                {
                                  //  Make sure the possible feature is valid.

                                  if (features[j][m].z > -13000.0)
                                    {
                                      //  Get the distance between the two possible features.  We use pfm_geo_distance because it is
                                      //  about 8 times faster than computing the actual distance.  The difference in most cases is
                                      //  on the order of a millimeter.

                                      pfm_geo_distance (pfm_handle, features[i][k].y, features[i][k].x, features[j][m].y,
                                                        features[j][m].x, &cut);


                                      //  If the distance is less than our deconflict radius we might need to get rid of one of these.

                                      if (cut < radius)
                                        {
                                          //  If the second list feature is deeper we want to invalidate it otherwise we do nothing.

                                          decon_count++;
                                          if (features[j][m].z >= features[i][k].z)
                                            {
                                              features[j][m].z = -14000.0;
                                            }
                                          else
                                            {
                                              features[i][k].z = -14000.0;
                                            }
                                        }
                                    }
                                }
                            }

                          percent = NINT (((NV_FLOAT32) k / (NV_FLOAT32) features_count[i]) * 100.0);
                          if (percent != old_percent)
                            {
                              progress->dbar->setValue (percent);
                              old_percent = percent;
                            }

                          qApp->processEvents ();
                        }
                    }

                  progress->dbar->setValue (100);
                  qApp->processEvents ();
                }
            }
        }
    }


  //  If we had an exclusion area file we want to invalidate any points that fall inside our exclusion area.

  if (options->ex_polygon_count)
    {
      for (NV_INT32 i = 0 ; i < passes ; i++)
        {
          for (NV_INT32 j = 0 ; j < features_count[i] ; j++)
            {
              if (features[i][j].z > -13000.0 && inside (options->ex_polygon_x, options->ex_polygon_y, options->ex_polygon_count,
                                                         features[i][j].x, features[i][j].y))
                {
                  features[i][j].z = -14000.0;
                  decon_count++;
                }
            }
        }
    }


  return (decon_count);
}
