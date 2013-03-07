#include "pfm3D.hpp"


void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc)
{
  NV_FLOAT64          wlon, elon, slat, nlat;
  NV_FLOAT32          feature_half_size;
  QColor              tmp;
  QString             feature_search_string = QString (misc->abe_share->feature_search_string);


  NV_BOOL checkFeature (MISC *misc, QString feature_search_string, NV_INT32 ftr, NV_BOOL *highlight, QString *feature_info);



  slat = misc->abe_share->viewer_displayed_area.min_y;
  nlat = misc->abe_share->viewer_displayed_area.max_y;
  wlon = misc->abe_share->viewer_displayed_area.min_x;
  elon = misc->abe_share->viewer_displayed_area.max_x;


  map->clearFeaturePoints ();


  //  Display features.

  if (options->display_feature)
    {
      feature_half_size = options->feature_size / 2.0;

      for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
        {
          if (misc->feature[i].confidence_level || misc->abe_share->layer_type == AVERAGE_DEPTH || 
              misc->abe_share->layer_type == MIN_DEPTH || misc->abe_share->layer_type == MAX_DEPTH)
            {
              if (misc->feature[i].longitude >= wlon && misc->feature[i].longitude <= elon &&
                  misc->feature[i].latitude >= slat && misc->feature[i].latitude <= nlat)
                {
                  //  Check for the type of feature display

                  if (options->display_feature == 1 || (options->display_feature == 2 && misc->feature[i].confidence_level != 5) ||
                      (options->display_feature == 3 && misc->feature[i].confidence_level == 5))
                    {
                      //  Check the feature for the feature search string and highlighting.

                      QString feature_info;
                      NV_BOOL highlight;
                      if (checkFeature (misc, feature_search_string, i, &highlight, &feature_info))
                        {
                          if (options->display_children || !misc->feature[i].parent_record)
                            {
                              tmp = options->feature_color;


                              //  Go through here if we want to display feature info or we want to
                              //  highlight using a feature text search.

                              if (options->display_feature_info || highlight)
                                {
                                  tmp = options->feature_info_color;
                                  if (options->display_feature_info && !feature_info.isEmpty ()) 
                                    map->drawText (feature_info, misc->feature[i].longitude, misc->feature[i].latitude, -misc->feature[i].depth, 
                                                   0.0, 10, tmp, NVFalse);


                                  if (highlight)
                                    {
                                      tmp = options->highlight_color;
                                    }
                                  else
                                    {
                                      tmp = options->feature_color;
                                    }
                                }

                              if (misc->feature[i].parent_record)
                                {
                                  map->setFeaturePoints (misc->feature[i].longitude, misc->feature[i].latitude, -misc->feature[i].depth,
                                                         tmp, feature_half_size, 7, 7, NVFalse);
                                }
                              else
                                {
                                  map->setFeaturePoints (misc->feature[i].longitude, misc->feature[i].latitude, -misc->feature[i].depth,
                                                         tmp, options->feature_size, 7, 7, NVFalse);
                                }
                            }
                        }
                    }
                }
            }

          if (qApp->hasPendingEvents ())
            {
              qApp->processEvents();
              if (misc->drawing_canceled) break;
            }
        }

      map->setFeaturePoints (0.0, 0.0, 0.0, Qt::black, 0.00, 0, 0, NVTrue);
    }
  else
    {
      map->flush ();
    }
}
