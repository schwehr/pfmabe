
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



#include "pfmEdit3DDef.hpp"

/***************************************************************************/
/*!

  - Module Name:        overlay_flag

  - Programmer(s):      Jan C. Depner

  - Date Written:       November 1995

  - Purpose:            Draw flag locations in the GC and the pixmap.

  - Arguments:

  - Return Value:       void

****************************************************************************/

void overlayFlag (nvMapGL *map, OPTIONS *options, MISC *misc)
{
  NV_FLOAT64          wlon, elon, slat, nlat;
  QColor              tmp;
  NV_FLOAT32          feature_half_size;
  QString             feature_search_string = QString (misc->abe_share->feature_search_string);


  NV_BOOL checkFeature (MISC *misc, QString feature_search_string, NV_INT32 ftr, NV_BOOL *highlight, QString *feature_info);



  slat = misc->abe_share->edit_area.min_y;
  nlat = misc->abe_share->edit_area.max_y;
  wlon = misc->abe_share->edit_area.min_x;
  elon = misc->abe_share->edit_area.max_x;


  misc->visible_feature_count = 0;

  map->clearFeaturePoints ();


  //  Display the features if requested.

  if (options->display_feature)
    {
      feature_half_size = options->feature_size / 2.0;


      for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
        {
          if (options->display_man_invalid || options->display_flt_invalid || misc->feature[i].confidence_level)
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
                              //  Go through here if we want to display feature info.

                              if (options->display_feature_info)
                                {
                                  tmp = options->feature_info_color;
                                  if (options->display_feature_info && !feature_info.isEmpty ()) 
                                    map->drawText (feature_info, misc->feature[i].longitude, misc->feature[i].latitude,
                                                   -misc->feature[i].depth, 0.0, 10, tmp, NVTrue);
                                }


                              QColor featureColor;
                              if (highlight)
                                {
                                  featureColor = options->feature_highlight_color;
                                }
                              else
                                {
                                  featureColor = options->feature_color;
                                  if (misc->feature[i].confidence_level == 5) featureColor = options->verified_feature_color;
                                }

                              if (misc->feature[i].parent_record)
                                {
                                  map->setFeaturePoints (misc->feature[i].longitude, misc->feature[i].latitude, -misc->feature[i].depth, featureColor, 
                                                         feature_half_size, 7, 7, NVFalse);
                                }
                              else
                                {
                                  map->setFeaturePoints (misc->feature[i].longitude, misc->feature[i].latitude, -misc->feature[i].depth, featureColor,
                                                         options->feature_size, 7, 7, NVFalse);
                                }

                              misc->visible_feature_count++;
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

      map->setFeaturePoints (0.0, 0.0, 0.0, Qt::black, 0.0, 0, 0, NVTrue);
    }


  //  Clear the marker points.

  map->clearMarkerPoints ();


  //  We can only display one group of markers (it gets too confusing to look at otherwise).  These are the filter kill points.

  if (misc->filter_kill_count)
    {
      //  Turn off highlighted points.

      if (misc->highlight_count)
        {
          free (misc->highlight);
          misc->highlight = NULL;
          misc->highlight_count = 0;
        }


      for (NV_INT32 i = 0 ; i < misc->filter_kill_count ; i++)
        {
          NV_INT32 j = misc->filter_kill_list[i];


          //  If we are displaying and editing only a single line, only get those points that 
          //  are in that line.

          if (!misc->num_lines || check_line (misc, misc->data[j].line))
            {
              if (!check_bounds (options, misc, j, NVTrue, misc->slice))
                map->setMarkerPoints (misc->data[j].x, misc->data[j].y, -misc->data[j].z, options->marker_color, 0.01, NVFalse);
            }
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0.01, NVTrue);
    }


  //  These are the attributeViewer AV_DISTANCE_TOOL points.

  else if (misc->av_dist_count)
    {
      //  Turn off highlighted points.

      if (misc->highlight_count)
        {
          free (misc->highlight);
          misc->highlight = NULL;
          misc->highlight_count = 0;
        }


      for (NV_INT32 i = 0 ; i < misc->av_dist_count ; i++)
        {
          NV_INT32 j = misc->av_dist_list[i];


          //  If we are displaying and editing only a single line, only get those points that are in that line.

          if (!misc->num_lines || check_line (misc, misc->data[j].line))
            {
              if (!check_bounds (options, misc, j, NVTrue, misc->slice))
                map->setMarkerPoints (misc->data[j].x, misc->data[j].y, -misc->data[j].z, options->marker_color, 0.01, NVFalse);
            }
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0.01, NVTrue);
    }


  //  These are highlight points

  else
    {
      for (NV_INT32 i = 0 ; i < misc->highlight_count ; i++)
        {
          NV_INT32 j = misc->highlight[i];

          if (!misc->num_lines || check_line (misc, misc->data[j].line))
            {
              if (!check_bounds (options, misc, j, NVTrue, misc->slice))
                map->setMarkerPoints (misc->data[j].x, misc->data[j].y, -misc->data[j].z, options->marker_color, 0.01, NVFalse);
            }
        }
      map->setMarkerPoints (0.0, 0.0, 0.0, Qt::black, 0.01, NVTrue);
    }
}
