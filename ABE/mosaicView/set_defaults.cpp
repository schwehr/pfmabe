#include "mosaicView.hpp"


void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore)
{
  //  Set Defaults so that if keys don't for envin the parms are defined.

  if (!restore)
    {
      misc->poly_count = 0;
      misc->feature = NULL;
      misc->nearest_feature = -1;
      misc->feature_search_string = "";

      memset (&misc->bfd_header, 0, sizeof (BFDATA_HEADER));

      misc->bfd_open = NVFalse;
    }


  options->coast = NVFalse;
  options->landmask = NVFalse;
  options->startup_message = NVTrue;
  options->position_form = 0;
  options->zoom_percent = 10;
  options->width = 1024;
  options->height = 768;
  options->window_x = 0;
  options->window_y = 0;
  options->marker_color = QColor (255, 0, 0, 128);
  options->coast_color = QColor (255, 255, 0);
  options->landmask_color = QColor (170, 85, 0);
  options->display_feature = NVFalse;
  options->display_children = NVFalse;
  options->display_feature_info = NVFalse;
  options->display_feature_poly = NVFalse;
  options->rect_color = QColor (255, 255, 255);
  options->feature_color = QColor (255, 255, 255);
  options->feature_info_color = QColor (255, 255, 255);
  options->feature_poly_color = QColor (255, 255, 0, 255);
  options->highlight_color = QColor (255, 0, 0);
  options->coast_color = QColor (255, 255, 0);
  options->landmask_color = QColor (170, 85, 0);
  options->feature_diameter = 20.0;
  options->last_rock_feature_desc = 4;
  options->last_offshore_feature_desc = 17;
  options->last_light_feature_desc = 9;
  options->last_lidar_feature_desc = 0;
  options->last_feature_description = "";
  options->last_feature_remarks = "";
  options->screenshot_delay = 2;
}
