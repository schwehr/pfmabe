
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


NV_FLOAT64 settings_version = 8.50;


/*!
  These functions store and retrieve the program settings (environment) from somewhere ;-)  On Linux they're in your home
  directory in a directory called .config/"misc->qsettings_org"/"misc->qsettings_app".rc (at this time - 
  .config/navo.navy.mil/pfmView.rc).  On Windoze they're stored in the registry (may a higher being have mercy 
  on you if you want to change them).  On MacOSX I have no idea where they're stored but you can check the Qt docs ;-)
  If you make a change to the way a variable is used and you want to force the defaults to be restored just change the
  settings_version to a newer number (I've been using the program version number from version.hpp - which you should be
  updating EVERY time you make a change to the program!).  You don't need to change the settings_version though unless
  you want to force the program to go back to the defaults (which can annoy your users).  So, the settings_version won't
  usually match the program version.
*/

NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;
  QString buttonHotKey[HOTKEYS];


  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);


  //  Recent files, directories, and hot keys should never be affected by the settings version so we want to load them even if the settings version
  //  has changed.

  options->recent_file_count = settings.value (pfmView::tr ("recent file count"), options->recent_file_count).toInt ();

  for (NV_INT32 i = 0 ; i < options->recent_file_count ; i++)
    {
      string.sprintf (pfmView::tr ("recent file %d").toAscii (), i);
      options->recentFile[i] = settings.value (string, options->recentFile[i]).toString ();
    }

  options->overlay_dir = settings.value (pfmView::tr ("overlay directory"), options->overlay_dir).toString ();
  options->output_area_dir = settings.value (pfmView::tr ("output area directory"), options->output_area_dir).toString ();
  options->output_points_dir = settings.value (pfmView::tr ("output points directory"), options->output_points_dir).toString ();
  options->input_pfm_dir = settings.value (pfmView::tr ("input pfm directory"), options->input_pfm_dir).toString ();
  options->geotiff_dir = settings.value (pfmView::tr ("geotiff directory"), options->geotiff_dir).toString ();
  options->area_dir = settings.value (pfmView::tr ("area directory"), options->area_dir).toString ();
  options->dnc_dir = settings.value (pfmView::tr ("dnc directory"), options->dnc_dir).toString ();


  //  We're trying to save the hot keys since these are a pain for the user to change back if they've made major changes 
  //  to them.  We want to use whatever the user had saved even if we change the settings version.  We will still have to
  //  check for hot key conflicts though since there might be new keys with new deefault values.  To do this we're going
  //  to save the default hot keys (set in set_defaults.cpp) and then do a comparison.  If there are conflicts we'll replace 
  //  the user's setting with the default and then issue a warning.

  NV_BOOL hotkey_conflict = NVFalse;

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      //  Save the default so we can check it later.

      buttonHotKey[i] = options->buttonAccel[i];


      QString string = misc->buttonText[i] + pfmView::tr (" hot key");
      options->buttonAccel[i] = settings.value (string, options->buttonAccel[i]).toString ();
    }


  //  Make sure we have no hotkey duplications.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (options->buttonAccel[i].toUpper () == options->buttonAccel[j].toUpper ())
            {
              hotkey_conflict = NVTrue;
              options->buttonAccel[i] = buttonHotKey[i];
              options->buttonAccel[j] = buttonHotKey[j];
            }
        }
    }


  if (hotkey_conflict)
    {
      QMessageBox::warning (0, pfmView::tr ("pfmView settings"),
                            pfmView::tr ("Hot key conflicts have been detected when reading program settings!<br><br>") +
                            pfmView::tr ("Some of your hotkeys have been reset to default values!"));
    }


  saved_version = settings.value (pfmView::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults (from set_defaults.cpp) since they may have changed.

  if (settings_version != saved_version) return (NVFalse);;


  options->contour = settings.value (pfmView::tr ("contour flag"), options->contour).toBool ();

  options->group_features = settings.value (pfmView::tr ("group features flag"), options->group_features).toBool ();

  options->contour_width = settings.value (pfmView::tr ("contour width"), options->contour_width).toInt ();

  options->contour_index = settings.value (pfmView::tr ("contour index"), options->contour_index).toInt ();


  options->coast = settings.value (pfmView::tr ("coast flag"), options->coast).toBool ();
  options->landmask = settings.value (pfmView::tr ("landmask flag"), options->landmask).toBool ();
  options->GeoTIFF_alpha = settings.value (pfmView::tr ("geotiff alpha"), options->GeoTIFF_alpha).toInt ();


  options->filterSTD = settings.value (pfmView::tr ("filter standard deviation"), options->filterSTD).toDouble ();

  options->deep_filter_only = settings.value (pfmView::tr ("deep filter only flag"), options->deep_filter_only).toBool ();

  options->misp_weight = settings.value (pfmView::tr ("misp weight"), options->misp_weight).toInt ();
  options->misp_force_original = settings.value (pfmView::tr ("misp force original flag"), options->misp_force_original).toBool ();
  options->misp_replace_all = settings.value (pfmView::tr ("misp replace all flag"), options->misp_replace_all).toBool ();

  options->position_form = settings.value (pfmView::tr ("position form"), options->position_form).toInt ();

  NV_INT32 red = settings.value (pfmView::tr ("contour color/red"), options->contour_color.red ()).toInt ();
  NV_INT32 green = settings.value (pfmView::tr ("contour color/green"), options->contour_color.green ()).toInt ();
  NV_INT32 blue = settings.value (pfmView::tr ("contour color/blue"), options->contour_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (pfmView::tr ("contour color/alpha"), options->contour_color.alpha ()).toInt ();
  options->contour_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("feature color/red"), options->feature_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("feature color/green"), options->feature_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("feature color/blue"), options->feature_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("feature color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("feature info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("feature info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("feature info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("feature info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("feature poly color/red"), options->feature_poly_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("feature poly color/green"), options->feature_poly_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("feature poly color/blue"), options->feature_poly_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ()).toInt ();
  options->feature_poly_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("verified feature color/red"), options->verified_feature_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("verified feature color/green"), options->verified_feature_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("verified feature color/blue"), options->verified_feature_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("verified feature color/alpha"), options->verified_feature_color.alpha ()).toInt ();
  options->verified_feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("contour highlight color/red"), options->contour_highlight_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("contour highlight color/green"), options->contour_highlight_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("contour highlight color/blue"), options->contour_highlight_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("contour highlight color/alpha"), 
                          options->contour_highlight_color.alpha ()).toInt ();
  options->contour_highlight_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("coast color/red"), options->coast_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("coast color/green"), options->coast_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("coast color/blue"), options->coast_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("coast color/alpha"), options->coast_color.alpha ()).toInt ();
  options->coast_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("landmask color/red"), options->landmask_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("landmask color/green"), options->landmask_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("landmask color/blue"), options->landmask_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("landmask color/alpha"), options->landmask_color.alpha ()).toInt ();
  options->landmask_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("filter mask color/red"), options->poly_filter_mask_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("filter mask color/green"), options->poly_filter_mask_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("filter mask color/blue"), options->poly_filter_mask_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("filter mask color/alpha"), options->poly_filter_mask_color.alpha ()).toInt ();
  options->poly_filter_mask_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (pfmView::tr ("coverage feature color/red"), options->cov_feature_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("coverage feature color/green"), options->cov_feature_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("coverage feature color/blue"), options->cov_feature_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("coverage feature color/alpha"), options->cov_feature_color.alpha ()).toInt ();
  options->cov_feature_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (pfmView::tr ("coverage invalid feature color/red"), options->cov_inv_feature_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("coverage invalid feature color/green"), options->cov_inv_feature_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("coverage invalid feature color/blue"), options->cov_inv_feature_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("coverage invalid feature color/alpha"), options->cov_inv_feature_color.alpha ()).toInt ();
  options->cov_inv_feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfmView::tr ("coverage verified feature color/red"), options->cov_verified_feature_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("coverage verified feature color/green"), options->cov_verified_feature_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("coverage verified feature color/blue"), options->cov_verified_feature_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("coverage verified feature color/alpha"), options->cov_verified_feature_color.alpha ()).toInt ();
  options->cov_verified_feature_color.setRgb (red, green, blue, alpha);


  options->stoplight = settings.value (pfmView::tr ("stoplight flag"), options->stoplight).toBool ();
  options->stoplight_min_mid = (NV_FLOAT64) settings.value (pfmView::tr ("stoplight min to mid crossover value"),
                                                            (NV_FLOAT64) options->stoplight_min_mid).toDouble ();
  options->stoplight_max_mid = (NV_FLOAT64) settings.value (pfmView::tr ("stoplight max to mid crossover value"),
                                                            (NV_FLOAT64) options->stoplight_max_mid).toDouble ();

  red = settings.value (pfmView::tr ("stoplight min color/red"), options->stoplight_min_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("stoplight min color/green"), options->stoplight_min_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("stoplight min color/blue"), options->stoplight_min_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("stoplight min color/alpha"), options->stoplight_min_color.alpha ()).toInt ();
  options->stoplight_min_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmView::tr ("stoplight mid color/red"), options->stoplight_mid_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("stoplight mid color/green"), options->stoplight_mid_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("stoplight mid color/blue"), options->stoplight_mid_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("stoplight mid color/alpha"), options->stoplight_mid_color.alpha ()).toInt ();
  options->stoplight_mid_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmView::tr ("stoplight max color/red"), options->stoplight_max_color.red ()).toInt ();
  green = settings.value (pfmView::tr ("stoplight max color/green"), options->stoplight_max_color.green ()).toInt ();
  blue = settings.value (pfmView::tr ("stoplight max color/blue"), options->stoplight_max_color.blue ()).toInt ();
  alpha = settings.value (pfmView::tr ("stoplight max color/alpha"), options->stoplight_max_color.alpha ()).toInt ();
  options->stoplight_max_color.setRgb (red, green, blue, alpha);


  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      string = pfmView::tr ("minimum hsv color value %1").arg (i);
      options->min_hsv_color[i] = settings.value (string, options->min_hsv_color[i]).toInt ();
      string = pfmView::tr ("maximum hsv color value %1").arg (i);
      options->max_hsv_color[i] = settings.value (string, options->max_hsv_color[i]).toInt ();
      string = pfmView::tr ("minimum hsv locked flag %1").arg (i);
      options->min_hsv_locked[i] = settings.value (string, options->min_hsv_locked[i]).toBool ();
      string = pfmView::tr ("maximum hsv locked flag %1").arg (i);
      options->max_hsv_locked[i] = settings.value (string, options->max_hsv_locked[i]).toBool ();
      string = pfmView::tr ("minimum hsv locked value %1").arg (i);
      options->min_hsv_value[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->min_hsv_value[i]).toDouble ();
      string = pfmView::tr ("maximum hsv locked value %1").arg (i);
      options->max_hsv_value[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->max_hsv_value[i]).toDouble ();
    }


  QString en = settings.value (pfmView::tr ("editor name"), QString (options->edit_name)).toString ();
  strcpy (options->edit_name, en.toAscii ());
  en = settings.value (pfmView::tr ("3D editor name"), QString (options->edit_name_3D)).toString ();
  strcpy (options->edit_name_3D, en.toAscii ());


  options->sunopts.sun.x = settings.value (pfmView::tr ("sunopts sun x"), options->sunopts.sun.x).toDouble ();

  options->sunopts.sun.y = settings.value (pfmView::tr ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.sun.z = settings.value (pfmView::tr ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.azimuth = settings.value (pfmView::tr ("sunopts azimuth"), options->sunopts.azimuth).toDouble ();

  options->sunopts.elevation = settings.value (pfmView::tr ("sunopts elevation"), options->sunopts.elevation).toDouble ();

  options->sunopts.exag = settings.value (pfmView::tr ("sunopts exag"), options->sunopts.exag).toDouble ();

  options->sunopts.power_cos = settings.value (pfmView::tr ("sunopts power cosine"), options->sunopts.power_cos).toDouble ();

  options->sunopts.num_shades = settings.value (pfmView::tr ("sunopts number of shades"), options->sunopts.num_shades).toInt ();


  options->display_suspect = settings.value (pfmView::tr ("view suspect sounding positions"), options->display_suspect).toBool ();

  options->display_feature = settings.value (pfmView::tr ("view feature positions"), options->display_feature).toInt ();
  options->display_children = settings.value (pfmView::tr ("view feature children positions"), options->display_children).toBool ();
  options->display_selected = settings.value (pfmView::tr ("view selected sounding positions"), options->display_selected).toBool ();

  options->display_reference = settings.value (pfmView::tr ("view reference data positions"), options->display_reference).toBool ();

  options->display_feature_info = settings.value (pfmView::tr ("view feature info"), options->display_feature_info).toBool ();

  options->display_feature_poly = settings.value (pfmView::tr ("view feature polygon"), options->display_feature_poly).toBool ();

  options->feature_search_string = settings.value (pfmView::tr ("feature search string"), options->feature_search_string).toString ();

  options->display_minmax = settings.value (pfmView::tr ("view min and max"), options->display_minmax).toBool ();


  options->auto_redraw = settings.value (pfmView::tr ("auto redraw"), options->auto_redraw).toBool ();

  options->edit_mode = settings.value (pfmView::tr ("edit mode [0,1]"), options->edit_mode).toInt ();


  QString up = settings.value (pfmView::tr ("unload program name"), QString (options->unload_prog)).toString ();
  strcpy (options->unload_prog, up.toAscii ());


  QString im = settings.value (pfmView::tr ("import program name"), QString (options->import_prog)).toString ();
  strcpy (options->import_prog, im.toAscii ());


  options->contour_filter_envelope = (NV_FLOAT32) settings.value (pfmView::tr ("contour filter envelope"), 
                                                                  (NV_FLOAT64) options->contour_filter_envelope).toDouble ();

  options->smoothing_factor = settings.value (pfmView::tr ("contour smoothing factor"), options->smoothing_factor).toInt ();

  options->z_factor = (NV_FLOAT32) settings.value (pfmView::tr ("depth scaling factor"), (NV_FLOAT64) options->z_factor).toDouble ();

  options->z_offset = (NV_FLOAT32) settings.value (pfmView::tr ("depth offset value"), (NV_FLOAT64) options->z_offset).toDouble ();

  options->zero_turnover = settings.value (pfmView::tr ("zero turnover flag"), options->zero_turnover).toBool ();

  options->highlight = settings.value (pfmView::tr ("highlight"), options->highlight).toInt ();
  options->highlight_percent = (NV_FLOAT32) settings.value (pfmView::tr ("highlight depth percentage"), (NV_FLOAT64) options->highlight_percent).toDouble ();

  options->chart_scale = settings.value (pfmView::tr ("chart scale"), options->chart_scale).toInt ();


  options->cint = (NV_FLOAT32) settings.value (pfmView::tr ("contour interval"), (NV_FLOAT64) options->cint).toDouble ();
  options->layer_type = settings.value (pfmView::tr ("binned layer type"), options->layer_type).toInt ();


  options->last_rock_feature_desc = settings.value (pfmView::tr ("last rock feature descriptor index"), options->last_rock_feature_desc).toInt ();

  options->last_offshore_feature_desc = settings.value (pfmView::tr ("last offshore feature descriptor index"), options->last_offshore_feature_desc).toInt ();

  options->last_light_feature_desc = settings.value (pfmView::tr ("last light feature descriptor index"), options->last_light_feature_desc).toInt ();

  options->last_lidar_feature_desc = settings.value (pfmView::tr ("last lidar feature descriptor index"), options->last_lidar_feature_desc).toInt ();

  options->last_feature_description = settings.value (pfmView::tr ("last feature description"), options->last_feature_description).toString ();

  options->last_feature_remarks = settings.value (pfmView::tr ("last feature remarks"), options->last_feature_remarks).toString ();

  options->feature_search_invert = settings.value (pfmView::tr ("invert feature search"), options->feature_search_invert).toBool ();

  options->feature_search_type = settings.value (pfmView::tr ("invert feature type"), options->feature_search_type).toInt ();

  options->overlap_percent = settings.value (pfmView::tr ("window overlap percentage"), options->overlap_percent).toInt ();

  options->num_levels = settings.value (pfmView::tr ("contour levels"), options->num_levels).toInt ();

  for (NV_INT32 i = 0 ; i < options->num_levels ; i++)
    {
      string.sprintf (pfmView::tr ("contour level %d").toAscii (), i);
      options->contour_levels[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->contour_levels[i]).toDouble ();
    }


  options->iho = settings.value (pfmView::tr ("CUBE IHO order"), options->iho).toInt ();
  options->capture = settings.value (pfmView::tr ("CUBE capture percentage"), options->capture).toDouble ();
  options->queue = settings.value (pfmView::tr ("CUBE queue length"), options->queue).toInt ();
  options->horiz = settings.value (pfmView::tr ("CUBE orizontal position uncertainty"), options->horiz).toDouble ();
  options->distance = settings.value (pfmView::tr ("CUBE distance exponent"), options->distance).toDouble ();
  options->min_context = settings.value (pfmView::tr ("CUBE minimum context"), options->min_context).toDouble ();
  options->max_context = settings.value (pfmView::tr ("CUBE maximum context"), options->max_context).toDouble ();
  options->disambiguation = settings.value (pfmView::tr ("CUBE disambiguation method"), options->disambiguation).toInt ();


  options->feature_radius = (NV_FLOAT32) settings.value (pfmView::tr ("feature filter exclusion radius"), (NV_FLOAT64) options->feature_radius).toDouble ();

  options->screenshot_delay = settings.value (pfmView::tr ("screenshot delay"), options->screenshot_delay).toInt ();

  options->otf_bin_size_meters = (NV_FLOAT32) settings.value (pfmView::tr ("otf bin size"), (NV_FLOAT64) options->otf_bin_size_meters).toDouble ();

  options->h_count = settings.value (pfmView::tr ("highlight bin count"), options->h_count).toInt ();

  options->new_feature = settings.value (pfmView::tr ("new feature counter"), options->new_feature).toInt ();
  options->startup_message = settings.value (pfmView::tr ("startup message flag"), options->startup_message).toBool ();


  mainWindow->restoreState (settings.value (pfmView::tr ("main window state")).toByteArray (), (NV_INT32) (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (pfmView::tr ("main window geometry")).toByteArray ());

  settings.endGroup ();


  //  We need to get the mosaicView program and hotkeys from the pfmEdit3D settings.
  //  Note that we never save these.  Only pfmEdit3D will be allowed to change these values.

  QSettings settings2 (pfmView::tr ("navo.navy.mil"), pfmView::tr ("pfmEdit3D"));

  settings2.beginGroup (pfmView::tr ("pfmEdit3D"));


  options->mosaic_prog = settings2.value (pfmView::tr ("mosaic viewer"), options->mosaic_prog).toString ();

  options->mosaic_hotkey = settings2.value (pfmView::tr ("mosaic viewer hot key"), options->mosaic_hotkey).toString ();
  options->mosaic_actkey = settings2.value (pfmView::tr ("mosaic viewer action keys"), options->mosaic_actkey).toString ();

  settings2.endGroup ();

  return (NVTrue);
}


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  QString string;

  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);


  settings.setValue (pfmView::tr ("recent file count"), options->recent_file_count);

  for (NV_INT32 i = 0 ; i < options->recent_file_count ; i++)
    {
      string.sprintf (pfmView::tr ("recent file %d").toAscii (), i);
      settings.setValue (string, options->recentFile[i]);
    }

  settings.setValue (pfmView::tr ("overlay directory"), options->overlay_dir);
  settings.setValue (pfmView::tr ("output area directory"), options->output_area_dir);
  settings.setValue (pfmView::tr ("output points directory"), options->output_points_dir);
  settings.setValue (pfmView::tr ("input pfm directory"), options->input_pfm_dir);
  settings.setValue (pfmView::tr ("geotiff directory"), options->geotiff_dir);
  settings.setValue (pfmView::tr ("area directory"), options->area_dir);
  settings.setValue (pfmView::tr ("dnc directory"), options->dnc_dir);


  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      string = misc->buttonText[i] + pfmView::tr (" hot key");
      settings.setValue (string, options->buttonAccel[i]);
    }


  settings.setValue (pfmView::tr ("settings version"), settings_version);

  settings.setValue (pfmView::tr ("contour flag"), options->contour);

  settings.setValue (pfmView::tr ("group features flag"), options->group_features);

  settings.setValue (pfmView::tr ("contour width"), options->contour_width);

  settings.setValue (pfmView::tr ("contour index"), options->contour_index);


  settings.setValue (pfmView::tr ("coast flag"), options->coast);
  settings.setValue (pfmView::tr ("landmask flag"), options->landmask);
  settings.setValue (pfmView::tr ("geotiff alpha"), options->GeoTIFF_alpha);

  settings.setValue (pfmView::tr ("main button icon size"), options->main_button_icon_size);

  settings.setValue (pfmView::tr ("misp weight"), options->misp_weight);
  settings.setValue (pfmView::tr ("misp force original flag"), options->misp_force_original);
  settings.setValue (pfmView::tr ("misp replace all flag"), options->misp_replace_all);

  settings.setValue (pfmView::tr ("deep filter only flag"), options->deep_filter_only);

  settings.setValue (pfmView::tr ("filter standard deviation"), options->filterSTD);

  settings.setValue (pfmView::tr ("position form"), options->position_form);


  settings.setValue (pfmView::tr ("contour color/red"), options->contour_color.red ());
  settings.setValue (pfmView::tr ("contour color/green"), options->contour_color.green ());
  settings.setValue (pfmView::tr ("contour color/blue"), options->contour_color.blue ());
  settings.setValue (pfmView::tr ("contour color/alpha"), options->contour_color.alpha ());


  settings.setValue (pfmView::tr ("feature color/red"), options->feature_color.red ());
  settings.setValue (pfmView::tr ("feature color/green"), options->feature_color.green ());
  settings.setValue (pfmView::tr ("feature color/blue"), options->feature_color.blue ());
  settings.setValue (pfmView::tr ("feature color/alpha"), options->feature_color.alpha ());


  settings.setValue (pfmView::tr ("feature info color/red"), options->feature_info_color.red ());
  settings.setValue (pfmView::tr ("feature info color/green"), options->feature_info_color.green ());
  settings.setValue (pfmView::tr ("feature info color/blue"), options->feature_info_color.blue ());
  settings.setValue (pfmView::tr ("feature info color/alpha"), options->feature_info_color.alpha ());


  settings.setValue (pfmView::tr ("feature poly color/red"), options->feature_poly_color.red ());
  settings.setValue (pfmView::tr ("feature poly color/green"), options->feature_poly_color.green ());
  settings.setValue (pfmView::tr ("feature poly color/blue"), options->feature_poly_color.blue ());
  settings.setValue (pfmView::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ());


  settings.setValue (pfmView::tr ("verified feature color/red"), options->verified_feature_color.red ());
  settings.setValue (pfmView::tr ("verified feature color/green"), options->verified_feature_color.green ());
  settings.setValue (pfmView::tr ("verified feature color/blue"), options->verified_feature_color.blue ());
  settings.setValue (pfmView::tr ("verified feature color/alpha"), options->verified_feature_color.alpha ());


  settings.setValue (pfmView::tr ("contour highlight color/red"), options->contour_highlight_color.red ());
  settings.setValue (pfmView::tr ("contour highlight color/green"), options->contour_highlight_color.green ());
  settings.setValue (pfmView::tr ("contour highlight color/blue"), options->contour_highlight_color.blue ());
  settings.setValue (pfmView::tr ("contour highlight color/alpha"), options->contour_highlight_color.alpha ());


  settings.setValue (pfmView::tr ("coast color/red"), options->coast_color.red ());
  settings.setValue (pfmView::tr ("coast color/green"), options->coast_color.green ());
  settings.setValue (pfmView::tr ("coast color/blue"), options->coast_color.blue ());
  settings.setValue (pfmView::tr ("coast color/alpha"), options->coast_color.alpha ());


  settings.setValue (pfmView::tr ("landmask color/red"), options->landmask_color.red ());
  settings.setValue (pfmView::tr ("landmask color/green"), options->landmask_color.green ());
  settings.setValue (pfmView::tr ("landmask color/blue"), options->landmask_color.blue ());
  settings.setValue (pfmView::tr ("landmask color/alpha"), options->landmask_color.alpha ());


  settings.setValue (pfmView::tr ("background color/red"), options->background_color.red ());
  settings.setValue (pfmView::tr ("background color/green"), options->background_color.green ());
  settings.setValue (pfmView::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (pfmView::tr ("background color/alpha"), options->background_color.alpha ());


  settings.setValue (pfmView::tr ("filter mask color/red"), options->poly_filter_mask_color.red ());
  settings.setValue (pfmView::tr ("filter mask color/green"), options->poly_filter_mask_color.green ());
  settings.setValue (pfmView::tr ("filter mask color/blue"), options->poly_filter_mask_color.blue ());
  settings.setValue (pfmView::tr ("filter mask color/alpha"), options->poly_filter_mask_color.alpha ());


  settings.setValue (pfmView::tr ("coverage feature color/red"), options->cov_feature_color.red ());
  settings.setValue (pfmView::tr ("coverage feature color/green"), options->cov_feature_color.green ());
  settings.setValue (pfmView::tr ("coverage feature color/blue"), options->cov_feature_color.blue ());
  settings.setValue (pfmView::tr ("coverage feature color/alpha"), options->cov_feature_color.alpha ());


  settings.setValue (pfmView::tr ("coverage invalid feature color/red"), options->cov_inv_feature_color.red ());
  settings.setValue (pfmView::tr ("coverage invalid feature color/green"), options->cov_inv_feature_color.green ());
  settings.setValue (pfmView::tr ("coverage invalid feature color/blue"), options->cov_inv_feature_color.blue ());
  settings.setValue (pfmView::tr ("coverage invalid feature color/alpha"), options->cov_inv_feature_color.alpha ());


  settings.setValue (pfmView::tr ("coverage verified feature color/red"), options->cov_verified_feature_color.red ());
  settings.setValue (pfmView::tr ("coverage verified feature color/green"), options->cov_verified_feature_color.green ());
  settings.setValue (pfmView::tr ("coverage verified feature color/blue"), options->cov_verified_feature_color.blue ());
  settings.setValue (pfmView::tr ("coverage verified feature color/alpha"), options->cov_verified_feature_color.alpha ());


  settings.setValue (pfmView::tr ("stoplight flag"), options->stoplight);
  settings.setValue (pfmView::tr ("stoplight min to mid crossover value"), (NV_FLOAT64) options->stoplight_min_mid);
  settings.setValue (pfmView::tr ("stoplight max to mid crossover value"), (NV_FLOAT64) options->stoplight_max_mid);

  settings.setValue (pfmView::tr ("stoplight min color/red"), options->stoplight_min_color.red ());
  settings.setValue (pfmView::tr ("stoplight min color/green"), options->stoplight_min_color.green ());
  settings.setValue (pfmView::tr ("stoplight min color/blue"), options->stoplight_min_color.blue ());
  settings.setValue (pfmView::tr ("stoplight min color/alpha"), options->stoplight_min_color.alpha ());

  settings.setValue (pfmView::tr ("stoplight mid color/red"), options->stoplight_mid_color.red ());
  settings.setValue (pfmView::tr ("stoplight mid color/green"), options->stoplight_mid_color.green ());
  settings.setValue (pfmView::tr ("stoplight mid color/blue"), options->stoplight_mid_color.blue ());
  settings.setValue (pfmView::tr ("stoplight mid color/alpha"), options->stoplight_mid_color.alpha ());

  settings.setValue (pfmView::tr ("stoplight max color/red"), options->stoplight_max_color.red ());
  settings.setValue (pfmView::tr ("stoplight max color/green"), options->stoplight_max_color.green ());
  settings.setValue (pfmView::tr ("stoplight max color/blue"), options->stoplight_max_color.blue ());
  settings.setValue (pfmView::tr ("stoplight max color/alpha"), options->stoplight_max_color.alpha ());

  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      string = pfmView::tr ("minimum hsv color value %1").arg (i);
      settings.setValue (string, options->min_hsv_color[i]);
      string = pfmView::tr ("maximum hsv color value %1").arg (i);
      settings.setValue (string, options->max_hsv_color[i]);
      string = pfmView::tr ("minimum hsv locked flag %1").arg (i);
      settings.setValue (string, options->min_hsv_locked[i]);
      string = pfmView::tr ("maximum hsv locked flag %1").arg (i);
      settings.setValue (string, options->max_hsv_locked[i]);
      string = pfmView::tr ("minimum hsv locked value %1").arg (i);
      settings.setValue (string, (NV_FLOAT64) options->min_hsv_value[i]);
      string = pfmView::tr ("maximum hsv locked value %1").arg (i);
      settings.setValue (string, (NV_FLOAT64) options->max_hsv_value[i]);
    }


  settings.setValue (pfmView::tr ("editor name"), QString (options->edit_name));
  settings.setValue (pfmView::tr ("3D editor name"), QString (options->edit_name_3D));


  settings.setValue (pfmView::tr ("sunopts sun x"), options->sunopts.sun.x);
  settings.setValue (pfmView::tr ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (pfmView::tr ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (pfmView::tr ("sunopts azimuth"), options->sunopts.azimuth);
  settings.setValue (pfmView::tr ("sunopts elevation"), options->sunopts.elevation);
  settings.setValue (pfmView::tr ("sunopts exag"), options->sunopts.exag);
  settings.setValue (pfmView::tr ("sunopts power cosine"), options->sunopts.power_cos);
  settings.setValue (pfmView::tr ("sunopts number of shades"), options->sunopts.num_shades);


  settings.setValue (pfmView::tr ("view suspect sounding positions"), options->display_suspect);
  settings.setValue (pfmView::tr ("view feature positions"), options->display_feature);
  settings.setValue (pfmView::tr ("view sub-feature positions"), options->display_children);
  settings.setValue (pfmView::tr ("view selected sounding positions"), options->display_selected);
  settings.setValue (pfmView::tr ("view reference data positions"), options->display_reference);
  settings.setValue (pfmView::tr ("view feature info"), options->display_feature_info);
  settings.setValue (pfmView::tr ("view feature polygon"), options->display_feature_poly);
  settings.setValue (pfmView::tr ("feature search string"), options->feature_search_string);
  settings.setValue (pfmView::tr ("view min and max"), options->display_minmax);

  settings.setValue (pfmView::tr ("auto redraw"), options->auto_redraw);

  settings.setValue (pfmView::tr ("edit mode [0,1]"), options->edit_mode);


  settings.setValue (pfmView::tr ("unload program name"), QString (options->unload_prog));

  settings.setValue (pfmView::tr ("import program name"), QString (options->import_prog));


  settings.setValue (pfmView::tr ("contour smoothing factor"), options->smoothing_factor);

  settings.setValue (pfmView::tr ("contour filter envelope"), (NV_FLOAT64) options->contour_filter_envelope);

  settings.setValue (pfmView::tr ("depth scaling factor"), (NV_FLOAT64) options->z_factor);

  settings.setValue (pfmView::tr ("depth offset value"), (NV_FLOAT64) options->z_offset);

  settings.setValue (pfmView::tr ("zero turnover flag"), options->zero_turnover);

  settings.setValue (pfmView::tr ("highlight"), options->highlight);
  settings.setValue (pfmView::tr ("highlight depth percentage"), (NV_FLOAT64) options->highlight_percent);

  settings.setValue (pfmView::tr ("chart scale"), (NV_FLOAT64) options->chart_scale);

  settings.setValue (pfmView::tr ("contour interval"), options->cint);

  settings.setValue (pfmView::tr ("binned layer type"), options->layer_type);


  settings.setValue (pfmView::tr ("feature filter exclusion radius"), (NV_FLOAT64) options->feature_radius);

  settings.setValue (pfmView::tr ("last rock feature descriptor index"), options->last_rock_feature_desc);

  settings.setValue (pfmView::tr ("last offshore feature descriptor index"), options->last_offshore_feature_desc);

  settings.setValue (pfmView::tr ("last light feature descriptor index"), options->last_light_feature_desc);

  settings.setValue (pfmView::tr ("last lidar feature descriptor index"), options->last_lidar_feature_desc);

  settings.setValue (pfmView::tr ("last feature description"), options->last_feature_description);

  settings.setValue (pfmView::tr ("last feature remarks"), options->last_feature_remarks);

  settings.setValue (pfmView::tr ("window overlap percentage"), options->overlap_percent);

  settings.setValue (pfmView::tr ("invert feature search"), options->feature_search_invert);

  settings.setValue (pfmView::tr ("invert feature type"), options->feature_search_type);


  settings.setValue (pfmView::tr ("contour levels"), options->num_levels);

  for (NV_INT32 i = 0 ; i < options->num_levels ; i++)
    {
      string.sprintf (pfmView::tr ("contour level %d").toAscii (), i);
      settings.setValue (string, options->contour_levels[i]);
    }


  settings.setValue (pfmView::tr ("CUBE IHO order"), options->iho);
  settings.setValue (pfmView::tr ("CUBE capture percentage"), options->capture);
  settings.setValue (pfmView::tr ("CUBE queue length"), options->queue);
  settings.setValue (pfmView::tr ("CUBE orizontal position uncertainty"), options->horiz);
  settings.setValue (pfmView::tr ("CUBE distance exponent"), options->distance);
  settings.setValue (pfmView::tr ("CUBE minimum context"), options->min_context);
  settings.setValue (pfmView::tr ("CUBE maximum context"), options->max_context);
  settings.setValue (pfmView::tr ("CUBE disambiguation method"), options->disambiguation);


  settings.setValue (pfmView::tr ("screenshot delay"), options->screenshot_delay);

  settings.setValue (pfmView::tr ("otf bin size"), (NV_FLOAT64) options->otf_bin_size_meters);

  settings.setValue (pfmView::tr ("highlight bin count"), options->h_count);

  settings.setValue (pfmView::tr ("new feature counter"), options->new_feature);
  settings.setValue (pfmView::tr ("startup message flag"), options->startup_message);


  settings.setValue (pfmView::tr ("main window state"), mainWindow->saveState ((NV_INT32) (settings_version * 100.0)));

  settings.setValue (pfmView::tr ("main window geometry"), mainWindow->saveGeometry ());


  settings.endGroup ();


  //  We need to set a value in the pfmEdit and pfmEdit3D qsettings to let them know that pfmView has
  //  left the building.  This will allow them to properly set the running state of any "monitor" type
  //  ancillary programs.  The only time this matters is if you have ancillary programs running from
  //  one of the editors and you have left the editor and then shut down pfmView.  In that case 
  //  the editor will come back up next time thinking that the ancillary programs are still running
  //  and set their button states incorrectly.

  QSettings settings2 (pfmView::tr ("navo.navy.mil"), pfmView::tr ("pfmEdit"));

  settings2.beginGroup (pfmView::tr ("pfmEdit"));


  settings2.setValue (pfmView::tr ("pfmView killed"), 1);

  settings2.endGroup ();

  QSettings settings3 (pfmView::tr ("navo.navy.mil"), pfmView::tr ("pfmEdit3D"));

  settings3.beginGroup (pfmView::tr ("pfmEdit3D"));


  settings3.setValue (pfmView::tr ("pfmView killed"), 1);

  settings3.endGroup ();
}
