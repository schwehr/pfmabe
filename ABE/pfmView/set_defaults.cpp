
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


/*!
  Set defaults for all of the program's user defined options.  This called before envin and may be called from 
  prefs.cpp if requested.
*/

void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore)
{
  //  Restore is only NVFalse if we were caled from pfmView (prior to envin).  If we're called from prefs.cpp
  //  this is set to NVTrue (we only want to reset the user's options in that case).

  if (!restore)
    {
      misc->coverage_drawing = NVFalse;
      misc->poly_count = 0;
      misc->maxd = 10;
      strcpy (misc->GeoTIFF_name, "");
      misc->GeoTIFF_init = NVTrue;
      misc->GeoTIFF_open = NVFalse;
      misc->drawing_canceled = NVFalse;
      misc->drawing = NVFalse;
      misc->display_GeoTIFF = NVFalse;
      misc->output_point_count = 0;
      misc->output_points = (OUTPUT_DATA_POINTS *) NULL;
      misc->clear = NVTrue;
      misc->cov_clear = NVTrue;
      memset (&misc->bfd_header, 0, sizeof (BFDATA_HEADER));
      misc->prev_total_displayed_area.min_x = 999.0;
      misc->html_help_tag = "#view_pfm";
      misc->nearest_feature = -1;
      misc->nearest_pfm = 0;
      misc->draw_contour_level = 0.0;
      misc->color_by_attribute = 0;
      misc->linked = NVFalse;
      misc->cov_color = QColor (0, 0, 0);
      misc->cov_checked_color = QColor (220, 220, 220);
      misc->cov_verified_color = QColor (0, 255, 0);
      misc->cov_function = COV_START_DRAW_RECTANGLE;


      //  Tooltip text for the buttons that have editable accelerators

      misc->buttonText[REDRAW_KEY] = pfmView::tr("Redraw");
      misc->buttonText[REDRAW_COVERAGE_KEY] = pfmView::tr("Redraw Coverage Map");
      misc->buttonText[TOGGLE_CONTOUR_KEY] = pfmView::tr("Toggle contour drawing");
      misc->buttonText[DISPLAY_AVG_EDITED_KEY] = pfmView::tr("Display average edited depth surface");
      misc->buttonText[DISPLAY_MIN_EDITED_KEY] = pfmView::tr("Display minimum edited depth surface");
      misc->buttonText[DISPLAY_MAX_EDITED_KEY] = pfmView::tr("Display maximum edited depth surface");
      misc->buttonText[DISPLAY_AVG_UNEDITED_KEY] = pfmView::tr("Display average unedited surface");
      misc->buttonText[DISPLAY_MIN_UNEDITED_KEY] = pfmView::tr("Display minimum unedited depth surface");
      misc->buttonText[DISPLAY_MAX_UNEDITED_KEY] = pfmView::tr("Display maximum unedited depth surface");
      misc->buttonText[DISPLAY_MIN_EDITED_OTF_KEY] = pfmView::tr("Display minimum edited depth surface in on-the-fly grid");
      misc->buttonText[DISPLAY_MAX_EDITED_OTF_KEY] = pfmView::tr("Display maximum edited depth surface in on-the-fly grid");
      misc->buttonText[TOGGLE_GEOTIFF_KEY] = pfmView::tr("Toggle GeoTIFF display");


      //  Toolbutton icons for the buttons that have editable accelerators

      misc->buttonIcon[REDRAW_KEY] = QIcon (":/icons/redraw.xpm");
      misc->buttonIcon[REDRAW_COVERAGE_KEY] = QIcon (":/icons/redrawcoverage.xpm");
      misc->buttonIcon[TOGGLE_CONTOUR_KEY] = QIcon (":/icons/contour.xpm");
      misc->buttonIcon[DISPLAY_AVG_EDITED_KEY] = QIcon (":/icons/avgfilt.xpm");
      misc->buttonIcon[DISPLAY_MIN_EDITED_KEY] = QIcon (":/icons/minfilt.xpm");
      misc->buttonIcon[DISPLAY_MAX_EDITED_KEY] = QIcon (":/icons/maxfilt.xpm");
      misc->buttonIcon[DISPLAY_AVG_UNEDITED_KEY] = QIcon (":/icons/avg.xpm");
      misc->buttonIcon[DISPLAY_MIN_UNEDITED_KEY] = QIcon (":/icons/min.xpm");
      misc->buttonIcon[DISPLAY_MAX_UNEDITED_KEY] = QIcon (":/icons/max.xpm");
      misc->buttonIcon[DISPLAY_MIN_EDITED_OTF_KEY] = QIcon (":/icons/minfilt_otf.xpm");
      misc->buttonIcon[DISPLAY_MAX_EDITED_OTF_KEY] = QIcon (":/icons/maxfilt_otf.xpm");
      misc->buttonIcon[TOGGLE_GEOTIFF_KEY] = QIcon (":/icons/geo.xpm");


      for (NV_INT32 i = 0 ; i < NUM_OVERLAYS ; i++) 
        {
          misc->overlays[i].display = NVFalse;
          misc->overlays[i].file_type = 0;
        }


      //  Set up the sine and cosine values for arrow drawing.

      for (NV_INT32 i = 0 ; i < 3600 ; i++)
        {
          misc->sin_array[i] = sinf (((NV_FLOAT32) i / 10.0) * NV_DEG_TO_RAD);
          misc->cos_array[i] = cosf (((NV_FLOAT32) i / 10.0) * NV_DEG_TO_RAD);
        }

      misc->qsettings_org = pfmView::tr ("navo.navy.mil");
      misc->qsettings_app = pfmView::tr ("pfmView");

      misc->add_feature_index = -1;
      misc->add_feature_coord.x = -1;
      misc->add_feature_coord.y = -1;

      misc->poly_filter_mask_count = 0;
      misc->poly_filter_mask = NULL;
      misc->def_feature_poly = NVFalse;

      misc->width = 1100;
      misc->height = 964;
      misc->window_x = 0;
      misc->window_y = 0;

      misc->cov_width = 250;
      misc->cov_height = 250;

      misc->dateline = NVFalse;
      misc->filt_contour = NULL;
      misc->filt_contour_count = 0;

      misc->process_id = getpid ();

      misc->bfd_open = NVFalse;
      misc->otf_surface = NVFalse;
      misc->otf_grid = NULL;
      misc->tposiafps = NVFalse;


#ifdef NVWIN3X

      misc->help_browser = "C:\\Program Files\\Mozilla Firefox\\firefox.exe";

      QFileInfo br (misc->help_browser);

      if (!br.exists () || !br.isExecutable ()) misc->help_browser = "C:\\Program Files\\Internet Explorer\\iexplore.exe";

#else

      misc->help_browser = "firefox";

#endif


      //  Special case.  We don't want to show the startup message again just because we reset the defaults.

      options->startup_message = NVTrue;
      options->recent_file_count = 0;
    }


  options->coast = NVFalse;
  options->landmask = NVFalse;

  options->position_form = 0;
  options->contour = NVFalse;
  options->contour_width = 2;
  options->contour_index = 0;
  options->display_suspect = NVFalse;
  options->display_feature = 0;
  options->display_children = NVFalse;
  options->display_selected = NVFalse;
  options->display_reference = NVFalse;
  options->display_feature_info = NVFalse;
  options->display_feature_poly = NVFalse;
  options->display_minmax = NVFalse;
  strcpy (options->edit_name, "pfmEdit");
  strcpy (options->edit_name_3D, "pfmEdit3D");
  strcpy (options->unload_prog, "pfm_unload");
  strcpy (options->cube_prog, "navo_pfm_cube");
  strcpy (options->import_prog, "dump_dnc");
  options->mosaic_prog = "mosaicView [MOSAIC_FILE]";
  options->mosaic_hotkey = 'm';
  options->mosaic_actkey = "z,+,-";
  options->threeD_prog = "pfm3D";
  options->threeD_hotkey = '3';
  options->threeD_actkey = "";
  options->sunopts.azimuth = 30.0;
  options->sunopts.elevation = 30.0;
  options->sunopts.exag = 1.0;
  options->sunopts.power_cos = 1.0;
  options->sunopts.num_shades = NUMSHADES;
  options->edit_mode = RECT_EDIT_AREA_3D;
  options->auto_redraw = NVTrue;
  options->contour_filter_envelope = 10.0;
  options->smoothing_factor = 0;
  options->z_factor = 1.0;
  options->z_offset = 0.0;
  options->contour_color = QColor (255, 255, 255);
  options->feature_color = QColor (255, 255, 255);
  options->feature_info_color = QColor (255, 255, 255);
  options->feature_poly_color = QColor (255, 255, 0, 255);
  options->verified_feature_color = QColor (0, 255, 0);
  options->contour_highlight_color = QColor (255, 0, 0);
  options->coast_color = QColor (255, 255, 0);
  options->landmask_color = QColor (170, 85, 0);
  options->background_color = QColor (96, 96, 96);
  options->cov_feature_color = QColor (255, 0, 0);
  options->cov_inv_feature_color = QColor (0, 255, 0);
  options->cov_verified_feature_color = QColor (0, 169, 169);
  options->poly_filter_mask_color = QColor (255, 255, 255, 96);
  options->h_count = 4;


  //  Default to red-blue, unlocked, 0.0 for all color bands.

  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      options->min_hsv_color[i] = 0;
      options->max_hsv_color[i] = 240;
      options->min_hsv_locked[i] = NVFalse;
      options->max_hsv_locked[i] = NVFalse;
      options->min_hsv_value[i] = 0.0;
      options->max_hsv_value[i] = 0.0;
    }


  //  Now change the color by depth default to red-magenta.

  options->max_hsv_color[0] = 315;


  options->highlight = 0;
  options->highlight_percent = 0.1;
  options->chart_scale = 5000;
  options->feature_radius = 20.0;
  options->last_rock_feature_desc = 4;
  options->last_offshore_feature_desc = 17;
  options->last_light_feature_desc = 9;
  options->last_lidar_feature_desc = 0;
  options->last_feature_description = "";
  options->last_feature_remarks = "";
  options->feature_search_string = "";
  options->misp_weight = 2;
  options->misp_force_original = NVTrue;
  options->misp_replace_all = NVTrue;
  options->filterSTD = 2.4;
  options->deep_filter_only = NVFalse;
  options->overlap_percent = 5;
  options->GeoTIFF_alpha = 64;
  options->group_features = NVTrue;
  options->new_feature = 0;
  options->zero_turnover = NVTrue;
  options->feature_search_type = 0;
  options->feature_search_invert = NVFalse;
  options->stoplight = NVFalse;
  options->stoplight_min_mid = 0.0;
  options->stoplight_max_mid = 0.0;
  options->stoplight_max_color = QColor (255, 0, 0, 255);
  options->stoplight_mid_color = QColor (255, 255, 0, 255);
  options->stoplight_min_color = QColor (0, 255, 0, 255);

  options->iho = 0;
  options->capture = 5.0;
  options->queue = 11;
  options->horiz = 0.0;
  options->distance = 2.0;
  options->min_context = 0.0;
  options->max_context = 5.0;
  options->std2conf = 1;
  options->disambiguation = 0;

  options->overlay_dir = ".";
  options->output_area_dir = ".";
  options->output_points_dir = ".";
  options->input_pfm_dir = ".";
  options->geotiff_dir = ".";
  options->area_dir = ".";
  options->dnc_dir = ".";
  options->main_button_icon_size = 24;
  options->screenshot_delay = 2;

  options->otf_bin_size_meters = 2.0;


  options->cint = 5.0;
  options->layer_type = MIN_FILTERED_DEPTH;


  //  Default contour levels

  options->num_levels = 52;
  options->contour_levels[0] = -30;
  options->contour_levels[1] = -20;
  options->contour_levels[2] = -10;
  options->contour_levels[3] = -5;
  options->contour_levels[4] = -2;
  options->contour_levels[5] = 0;
  options->contour_levels[6] = 2;
  options->contour_levels[7] = 5;
  options->contour_levels[8] = 10;
  options->contour_levels[9] = 20;
  options->contour_levels[10] = 30;
  options->contour_levels[11] = 40;
  options->contour_levels[12] = 50;
  options->contour_levels[13] = 60;
  options->contour_levels[14] = 70;
  options->contour_levels[15] = 80;
  options->contour_levels[16] = 90;
  options->contour_levels[17] = 100;
  options->contour_levels[18] = 120;
  options->contour_levels[19] = 130;
  options->contour_levels[20] = 140;
  options->contour_levels[21] = 160;
  options->contour_levels[22] = 180;
  options->contour_levels[23] = 200;
  options->contour_levels[24] = 250;
  options->contour_levels[25] = 300;
  options->contour_levels[26] = 350;
  options->contour_levels[27] = 400;
  options->contour_levels[28] = 450;
  options->contour_levels[29] = 500;
  options->contour_levels[30] = 600;
  options->contour_levels[31] = 700;
  options->contour_levels[32] = 800;
  options->contour_levels[33] = 900;
  options->contour_levels[34] = 1000;
  options->contour_levels[35] = 1500;
  options->contour_levels[36] = 2000;
  options->contour_levels[37] = 2500;
  options->contour_levels[38] = 3000;
  options->contour_levels[39] = 3500;
  options->contour_levels[40] = 4000;
  options->contour_levels[41] = 4500;
  options->contour_levels[42] = 5000;
  options->contour_levels[43] = 5500;
  options->contour_levels[44] = 6000;
  options->contour_levels[45] = 6500;
  options->contour_levels[46] = 7000;
  options->contour_levels[47] = 7500;
  options->contour_levels[48] = 8000;
  options->contour_levels[49] = 8500;
  options->contour_levels[50] = 9000;
  options->contour_levels[51] = 10000;


  options->buttonAccel[REDRAW_KEY] = "Shift+r";
  options->buttonAccel[REDRAW_COVERAGE_KEY] = "Shift+m";
  options->buttonAccel[TOGGLE_CONTOUR_KEY] = "Shift+c";
  options->buttonAccel[DISPLAY_AVG_EDITED_KEY] = "F3";
  options->buttonAccel[DISPLAY_MIN_EDITED_KEY] = "F4";
  options->buttonAccel[DISPLAY_MAX_EDITED_KEY] = "F5";
  options->buttonAccel[DISPLAY_AVG_UNEDITED_KEY] = "F6";
  options->buttonAccel[DISPLAY_MIN_UNEDITED_KEY] = "F7";
  options->buttonAccel[DISPLAY_MAX_UNEDITED_KEY] = "F8";
  options->buttonAccel[DISPLAY_MIN_EDITED_OTF_KEY] = "F9";
  options->buttonAccel[DISPLAY_MAX_EDITED_OTF_KEY] = "F10";
  options->buttonAccel[TOGGLE_GEOTIFF_KEY] = "F11";
}
