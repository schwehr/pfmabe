
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



#include "pfmEdit.hpp"


NV_FLOAT64 settings_version = 8.35;


/*!
  - These functions store and retrieve the program settings (environment) from somewhere ;-)  On Linux they're in your home
    directory in a directory called .config/"misc->qsettings_org"/"misc->qsettings_app".rc (at this time - 
    .config/navo.navy.mil/pfmEdit.rc).  On Windoze they're stored in the registry (may a higher being have mercy 
    on you if you want to change them).  On MacOSX I have no idea where they're stored but you can check the Qt docs ;-)
    If you make a change to the way a variable is used and you want to force the defaults to be restored just change the
    settings_version to a newer number (I've been using the program version number from version.hpp - which you should be
    updating EVERY time you make a change to the program!).  You don't need to change the settings_version though unless
    you want to force the program to go back to the defaults (which can annoy your users).  So, the settings_version won't
    usually match the program version.

    - Note to self : I'm not saving the contour intervals because they always get passed in from pfmView
*/

NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString buttonHotKey[HOTKEYS];
  QString progHotKey[NUMPROGS];


  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);


  //  We're trying to save the hot keys since these are a PITA for the user to change back if they've made major changes 
  //  to them.  We want to use whatever the user had saved even if we change the settings version.  We will still have to
  //  check for hot key conflicts though since there might be new keys with new deefault values.  To do this we're going
  //  to save the default hot keys (set in set_defaults.cpp) and then do a comparison.  If there are conflicts we'll replace 
  //  the user's setting with the default and then issue a warning.

  NV_BOOL hotkey_conflict = NVFalse;

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      //  Save the default so we can check it later.

      buttonHotKey[i] = options->buttonAccel[i];


      QString string = misc->buttonText[i] + pfmEdit::tr (" hot key");
      options->buttonAccel[i] = settings.value (string, options->buttonAccel[i]).toString ();
    }

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      //  Save the default so we can check it later.

      progHotKey[i] = options->hotkey[i];


      QString name = options->name[i] + pfmEdit::tr (" Hotkey");
      options->hotkey[i] = settings.value (name, options->hotkey[i]).toString ();


      //  This fixes a screwup that I made in an earlier version.  I can probably take it out in a few months.
      //  I shouldn't have used "Return" as a hot key since it is used for internal signals on some widgets.
      //  JCD 05/10/11

      if (options->hotkey[i] == "Return") options->hotkey[i] = progHotKey[i];


      name = options->name[i] + pfmEdit::tr (" Action Key");
      options->action[i] = settings.value (name, options->action[i]).toString ();

      name = options->name[i] + pfmEdit::tr (" State");
      options->state[i] = settings.value (name, options->state[i]).toInt ();
    }


  //  Make sure we have no hotkey duplications.  First, the buttons against the buttons.

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


  //  Next, the programs against the programs.

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < NUMPROGS ; j++)
        {
          if (options->hotkey[i].toUpper () == options->hotkey[j].toUpper ())
            {
              hotkey_conflict = NVTrue;
              options->hotkey[i] = progHotKey[i];
              options->hotkey[j] = progHotKey[j];
            }
        }
    }


  //  Finally, the buttons against the programs.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = 0 ; j < NUMPROGS ; j++)
        {
          if (options->buttonAccel[i].toUpper () == options->hotkey[j].toUpper ())
            {
              hotkey_conflict = NVTrue;
              options->buttonAccel[i] = buttonHotKey[i];
              options->hotkey[j] = progHotKey[j];
            }
        }
    }


  if (hotkey_conflict)
    {
      QMessageBox::warning (0, pfmEdit::tr ("pfmEdit settings"),
                            pfmEdit::tr ("Hot key conflicts have been detected when reading program settings!<br><br>") +
                            pfmEdit::tr ("Some of your hotkeys have been reset to default values!"));
    }


  saved_version = settings.value (pfmEdit::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->flag_index = settings.value (pfmEdit::tr ("flag index"), options->flag_index).toInt ();

  options->display_contours = settings.value (pfmEdit::tr ("contour flag"), options->display_contours).toBool ();

  options->display_GeoTIFF = settings.value (pfmEdit::tr ("display GeoTIFF"), options->display_GeoTIFF).toBool ();

  options->smoothing_factor = settings.value (pfmEdit::tr ("smoothing factor"), options->smoothing_factor).toInt ();

  options->point_size = settings.value (pfmEdit::tr ("point size"), options->point_size).toInt ();

  options->contour_width = settings.value (pfmEdit::tr ("contour width"), options->contour_width).toInt ();

  options->slice_percent = settings.value (pfmEdit::tr ("slice percentage"), options->slice_percent).toInt ();

  options->slice_alpha = settings.value (pfmEdit::tr ("slice alpha"), options->slice_alpha).toInt ();

  options->rotation_increment = (NV_FLOAT32) settings.value (pfmEdit::tr ("rotation increment"), 
                                                             (NV_FLOAT64) options->rotation_increment).toDouble ();

  options->min_window_size = (NV_FLOAT32) settings.value (pfmEdit::tr ("minimum Z window size"),
                                                          (NV_FLOAT64) options->min_window_size).toDouble ();

  options->iho_min_window = settings.value (pfmEdit::tr ("IHO min window"), options->iho_min_window).toInt ();

  options->function = settings.value (pfmEdit::tr ("function"), options->function).toInt ();

  NV_INT32 red = settings.value (pfmEdit::tr ("contour color/red"), options->contour_color.red ()).toInt ();
  NV_INT32 green = settings.value (pfmEdit::tr ("contour color/green"), options->contour_color.green ()).toInt ();
  NV_INT32 blue = settings.value (pfmEdit::tr ("contour color/blue"), options->contour_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (pfmEdit::tr ("contour color/alpha"), options->contour_color.alpha ()).toInt ();
  options->contour_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("tracker color/red"), options->tracker_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("tracker color/green"), options->tracker_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("tracker color/blue"), options->tracker_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("tracker color/alpha"), options->tracker_color.alpha ()).toInt ();
  options->tracker_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("feature color/red"), options->feature_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("feature color/green"), options->feature_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("feature color/blue"), options->feature_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("feature color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("feature info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("feature info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("feature info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("feature info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("feature poly color/red"), options->feature_poly_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("feature poly color/green"), options->feature_poly_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("feature poly color/blue"), options->feature_poly_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ()).toInt ();
  options->feature_poly_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("highlighted feature color/red"), options->feature_highlight_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("highlighted feature color/green"), options->feature_highlight_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("highlighted feature color/blue"), options->feature_highlight_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("highlighted feature color/alpha"), options->feature_highlight_color.alpha ()).toInt ();
  options->feature_highlight_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfmEdit::tr ("verified feature color/red"), options->verified_feature_color.red ()).toInt ();
  green = settings.value (pfmEdit::tr ("verified feature color/green"), options->verified_feature_color.green ()).toInt ();
  blue = settings.value (pfmEdit::tr ("verified feature color/blue"), options->verified_feature_color.blue ()).toInt ();
  alpha = settings.value (pfmEdit::tr ("verified feature color/alpha"), options->verified_feature_color.alpha ()).toInt ();
  options->verified_feature_color.setRgb (red, green, blue, alpha);


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      QString string = pfmEdit::tr ("Wave color %1").arg (i);
      red = settings.value (string + "/red", options->waveColor[i].red ()).toInt ();
      green = settings.value (string + "/green", options->waveColor[i].green ()).toInt ();
      blue = settings.value (string + "/blue", options->waveColor[i].blue ()).toInt ();
      alpha = settings.value (string + "/alpha", options->waveColor[i].alpha ()).toInt ();
      options->waveColor[i].setRgb (red, green, blue, alpha);

      misc->abe_share->mwShare.multiColors[i].r = red;
      misc->abe_share->mwShare.multiColors[i].g = green;
      misc->abe_share->mwShare.multiColors[i].b = blue;
      misc->abe_share->mwShare.multiColors[i].a = alpha;
    }


  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      QString string = pfmEdit::tr ("minimum hsv color value %1").arg (i);
      options->min_hsv_color[i] = settings.value (string, options->min_hsv_color[i]).toInt ();
      string = pfmEdit::tr ("maximum hsv color value %1").arg (i);
      options->max_hsv_color[i] = settings.value (string, options->max_hsv_color[i]).toInt ();
      string = pfmEdit::tr ("minimum hsv locked flag %1").arg (i);
      options->min_hsv_locked[i] = settings.value (string, options->min_hsv_locked[i]).toBool ();
      string = pfmEdit::tr ("maximum hsv locked flag %1").arg (i);
      options->max_hsv_locked[i] = settings.value (string, options->max_hsv_locked[i]).toBool ();
      string = pfmEdit::tr ("minimum hsv locked value %1").arg (i);
      options->min_hsv_value[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->min_hsv_value[i]).toDouble ();
      string = pfmEdit::tr ("maximum hsv locked value %1").arg (i);
      options->max_hsv_value[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->max_hsv_value[i]).toDouble ();
    }


  options->GeoTIFF_alpha = settings.value (pfmEdit::tr ("geotiff alpha"), options->GeoTIFF_alpha).toInt ();

  options->hofWaveFilter_search_radius = settings.value (pfmEdit::tr ("hofWaveFilter search radius"), options->hofWaveFilter_search_radius).toDouble ();
  options->hofWaveFilter_search_width = settings.value (pfmEdit::tr ("hofWaveFilter search width"), options->hofWaveFilter_search_width).toInt ();
  options->hofWaveFilter_rise_threshold = settings.value (pfmEdit::tr ("hofWaveFilter rise threshold"), options->hofWaveFilter_rise_threshold).toInt ();
  options->hofWaveFilter_pmt_ac_zero_offset_required = settings.value (pfmEdit::tr ("hofWaveFilter PMT AC zero offset required"),
                                                                       options->hofWaveFilter_pmt_ac_zero_offset_required).toInt ();
  options->hofWaveFilter_apd_ac_zero_offset_required = settings.value (pfmEdit::tr ("hofWaveFilter APD AC zero offset required"),
                                                                       options->hofWaveFilter_apd_ac_zero_offset_required).toInt ();

  options->display_feature = settings.value (pfmEdit::tr ("view feature positions"), options->display_feature).toInt ();

  options->display_children = settings.value (pfmEdit::tr ("view sub-feature positions"), options->display_children).toBool ();

  options->display_feature_info = settings.value (pfmEdit::tr ("view feature info"), options->display_feature_info).toBool ();

  options->display_feature_poly = settings.value (pfmEdit::tr ("view feature polygon"), options->display_feature_poly).toBool ();

  options->screenshot_delay = settings.value (pfmEdit::tr ("screenshot delay"), options->screenshot_delay).toInt ();

  options->display_reference = settings.value (pfmEdit::tr ("view reference data positions"), options->display_reference).toBool ();

  options->auto_redraw = settings.value (pfmEdit::tr ("auto redraw"), options->auto_redraw).toBool ();

  options->unload_prog = settings.value (pfmEdit::tr ("unload program name"), options->unload_prog).toString ();
  options->auto_unload = settings.value (pfmEdit::tr ("auto unload flag"), options->auto_unload).toBool ();


  options->last_rock_feature_desc = settings.value (pfmEdit::tr ("last rock feature descriptor index"), options->last_rock_feature_desc).toInt ();

  options->last_offshore_feature_desc = settings.value (pfmEdit::tr ("last offshore feature descriptor index"), options->last_offshore_feature_desc).toInt ();

  options->last_light_feature_desc = settings.value (pfmEdit::tr ("last light feature descriptor index"), options->last_light_feature_desc).toInt ();

  options->last_lidar_feature_desc = settings.value (pfmEdit::tr ("last lidar feature descriptor index"), options->last_lidar_feature_desc).toInt ();

  settings.setValue (pfmEdit::tr ("last feature description"), options->last_feature_description);

  settings.setValue (pfmEdit::tr ("last feature remarks"), options->last_feature_remarks);

  options->overlap_percent = settings.value (pfmEdit::tr ("overlap percent"), options->overlap_percent).toInt ();

  options->display_grid = settings.value (pfmEdit::tr ("display grid flag"), options->display_grid).toBool ();

  options->plan_view_by_line = settings.value (pfmEdit::tr ("plan view color by line flag"), options->plan_view_by_line).toBool ();
  options->alt_view_by_line = settings.value (pfmEdit::tr ("alternate view color by line flag"), options->alt_view_by_line).toBool ();

  options->filterSTD = settings.value (pfmEdit::tr ("filter standard deviation"), options->filterSTD).toDouble ();

  options->deep_filter_only = settings.value (pfmEdit::tr ("deep filter only flag"), options->deep_filter_only).toBool ();

  options->feature_radius = (NV_FLOAT32) settings.value (pfmEdit::tr ("feature filter exclusion radius"), (NV_FLOAT64) options->feature_radius).toDouble ();

  options->undo_levels = settings.value (pfmEdit::tr ("undo levels"), options->undo_levels).toInt ();

  options->kill_and_respawn = settings.value (pfmEdit::tr ("kill and respawn flag"), options->kill_and_respawn).toBool ();

  for (NV_INT32 i = 0 ; i < NUM_ATTR + PRE_ATTR ; i++)
    {
      QString name = pfmEdit::tr ("Attribute %1 range minimum").arg (i, 2, 10, QChar ('0'));
      options->attr_filter_range[i][0] = (NV_FLOAT32) settings.value (name, (NV_FLOAT64) options->attr_filter_range[i][0]).toDouble ();

      name = pfmEdit::tr ("Attribute %1 range maximum").arg (i, 2, 10, QChar ('0'));
      options->attr_filter_range[i][1] = (NV_FLOAT32) settings.value (name, (NV_FLOAT64) options->attr_filter_range[i][1]).toDouble ();
    }


  options->avInterfaceBoxSize = settings.value (pfmEdit::tr ("av interface box size"), 0).toInt ();
  options->distThresh = (NV_FLOAT32) settings.value (pfmEdit::tr ("distance threshhold"), 4.0).toDouble ();


  //  This is the killed flag that can be set when pfmView exits.  If pfmView was shut down since the last time
  //  this program was running we may have ancillary program states set to "running" (2) even though pfmView
  //  killed them.

  NV_INT32 pfmView_killed = 0;
  pfmView_killed = settings.value (pfmEdit::tr ("pfmView killed"), pfmView_killed).toInt();


  //  Now we set the ancillary program states if pfmView was killed (possibly with ancillary programs running).

  if (pfmView_killed)
    {
      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          //  If it was anything other than 0 we want to make sure it's set to 1.

          if (options->state[i]) options->state[i] = 1;
        }
    }


  mainWindow->restoreState (settings.value (pfmEdit::tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (pfmEdit::tr ("main window geometry")).toByteArray ());


  settings.endGroup ();


  return (NVTrue);
}


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      QString string = misc->buttonText[i] + pfmEdit::tr (" hot key");
      settings.setValue (string, options->buttonAccel[i]);
    }

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      QString name = options->name[i] + pfmEdit::tr (" Hotkey");
      settings.setValue (name, options->hotkey[i]);

      name = options->name[i] + pfmEdit::tr (" Action Key");
      settings.setValue (name, options->action[i]);

      name = options->name[i] + pfmEdit::tr (" State");
      settings.setValue (name, options->state[i]);
    }

  settings.setValue (pfmEdit::tr ("settings version"), settings_version);

  settings.setValue (pfmEdit::tr ("flag index"), options->flag_index);

  settings.setValue (pfmEdit::tr ("contour flag"), options->display_contours);

  settings.setValue (pfmEdit::tr ("display GeoTIFF"), options->display_GeoTIFF);

  settings.setValue (pfmEdit::tr ("contour width"), options->contour_width);

  settings.setValue (pfmEdit::tr ("smoothing factor"), options->smoothing_factor);

  settings.setValue (pfmEdit::tr ("main button icon size"), options->main_button_icon_size);

  settings.setValue (pfmEdit::tr ("point size"), options->point_size);

  settings.setValue (pfmEdit::tr ("slice percentage"), options->slice_percent);

  settings.setValue (pfmEdit::tr ("slice alpha"), options->slice_alpha);

  settings.setValue (pfmEdit::tr ("rotation increment"), (NV_FLOAT64) options->rotation_increment);

  settings.setValue (pfmEdit::tr ("minimum Z window size"), (NV_FLOAT64) options->min_window_size);

  settings.setValue (pfmEdit::tr ("IHO min window"), options->iho_min_window);

  settings.setValue (pfmEdit::tr ("function"), options->function);

  settings.setValue (pfmEdit::tr ("contour color/red"), options->contour_color.red ());
  settings.setValue (pfmEdit::tr ("contour color/green"), options->contour_color.green ());
  settings.setValue (pfmEdit::tr ("contour color/blue"), options->contour_color.blue ());
  settings.setValue (pfmEdit::tr ("contour color/alpha"), options->contour_color.alpha ());

  settings.setValue (pfmEdit::tr ("tracker color/red"), options->tracker_color.red ());
  settings.setValue (pfmEdit::tr ("tracker color/green"), options->tracker_color.green ());
  settings.setValue (pfmEdit::tr ("tracker color/blue"), options->tracker_color.blue ());
  settings.setValue (pfmEdit::tr ("tracker color/alpha"), options->tracker_color.alpha ());

  settings.setValue (pfmEdit::tr ("background color/red"), options->background_color.red ());
  settings.setValue (pfmEdit::tr ("background color/green"), options->background_color.green ());
  settings.setValue (pfmEdit::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (pfmEdit::tr ("background color/alpha"), options->background_color.alpha ());

  settings.setValue (pfmEdit::tr ("feature color/red"), options->feature_color.red ());
  settings.setValue (pfmEdit::tr ("feature color/green"), options->feature_color.green ());
  settings.setValue (pfmEdit::tr ("feature color/blue"), options->feature_color.blue ());
  settings.setValue (pfmEdit::tr ("feature color/alpha"), options->feature_color.alpha ());

  settings.setValue (pfmEdit::tr ("feature info color/red"), options->feature_info_color.red ());
  settings.setValue (pfmEdit::tr ("feature info color/green"), options->feature_info_color.green ());
  settings.setValue (pfmEdit::tr ("feature info color/blue"), options->feature_info_color.blue ());
  settings.setValue (pfmEdit::tr ("feature info color/alpha"), options->feature_info_color.alpha ());

  settings.setValue (pfmEdit::tr ("feature poly color/red"), options->feature_poly_color.red ());
  settings.setValue (pfmEdit::tr ("feature poly color/green"), options->feature_poly_color.green ());
  settings.setValue (pfmEdit::tr ("feature poly color/blue"), options->feature_poly_color.blue ());
  settings.setValue (pfmEdit::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ());

  settings.setValue (pfmEdit::tr ("highlighted feature color/red"), options->feature_highlight_color.red ());
  settings.setValue (pfmEdit::tr ("highlighted feature color/green"), options->feature_highlight_color.green ());
  settings.setValue (pfmEdit::tr ("highlighted feature color/blue"), options->feature_highlight_color.blue ());
  settings.setValue (pfmEdit::tr ("highlighted feature color/alpha"), options->feature_highlight_color.alpha ());

  settings.setValue (pfmEdit::tr ("verified feature color/red"), options->verified_feature_color.red ());
  settings.setValue (pfmEdit::tr ("verified feature color/green"), options->verified_feature_color.green ());
  settings.setValue (pfmEdit::tr ("verified feature color/blue"), options->verified_feature_color.blue ());
  settings.setValue (pfmEdit::tr ("verified feature color/alpha"), options->verified_feature_color.alpha ());


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      QString string = pfmEdit::tr ("Wave color %1").arg (i, 2, 10, QChar ('0'));
      settings.setValue (string + "/red", options->waveColor[i].red ());
      settings.setValue (string + "/green", options->waveColor[i].green ());
      settings.setValue (string + "/blue", options->waveColor[i].blue ());
      settings.setValue (string + "/alpha", options->waveColor[i].alpha ());
    }


  for (NV_INT32 i = 0 ; i < NUM_HSV ; i++)
    {
      QString string = pfmEdit::tr ("minimum hsv color value %1").arg (i);
      settings.setValue (string, options->min_hsv_color[i]);
      string = pfmEdit::tr ("maximum hsv color value %1").arg (i);
      settings.setValue (string, options->max_hsv_color[i]);
      string = pfmEdit::tr ("minimum hsv locked flag %1").arg (i);
      settings.setValue (string, options->min_hsv_locked[i]);
      string = pfmEdit::tr ("maximum hsv locked flag %1").arg (i);
      settings.setValue (string, options->max_hsv_locked[i]);
      string = pfmEdit::tr ("minimum hsv locked value %1").arg (i);
      settings.setValue (string, (NV_FLOAT64) options->min_hsv_value[i]);
      string = pfmEdit::tr ("maximum hsv locked value %1").arg (i);
      settings.setValue (string, (NV_FLOAT64) options->max_hsv_value[i]);
    }


  settings.setValue (pfmEdit::tr ("geotiff alpha"), options->GeoTIFF_alpha);

  settings.setValue (pfmEdit::tr ("hofWaveFilter search radius"), options->hofWaveFilter_search_radius);
  settings.setValue (pfmEdit::tr ("hofWaveFilter search width"), options->hofWaveFilter_search_width);
  settings.setValue (pfmEdit::tr ("hofWaveFilter rise threshold"), options->hofWaveFilter_rise_threshold);
  settings.setValue (pfmEdit::tr ("hofWaveFilter PMT AC zero offset required"), options->hofWaveFilter_pmt_ac_zero_offset_required);
  settings.setValue (pfmEdit::tr ("hofWaveFilter APD AC zero offset required"), options->hofWaveFilter_apd_ac_zero_offset_required);

  settings.setValue (pfmEdit::tr ("view feature positions"), options->display_feature);
  settings.setValue (pfmEdit::tr ("view sub-feature positions"), options->display_children);
  settings.setValue (pfmEdit::tr ("view feature info"), options->display_feature_info);
  settings.setValue (pfmEdit::tr ("view feature polygon"), options->display_feature_poly);
  settings.setValue (pfmEdit::tr ("screenshot delay"), options->screenshot_delay);
  settings.setValue (pfmEdit::tr ("view reference data positions"), options->display_reference);

  settings.setValue (pfmEdit::tr ("auto redraw"), options->auto_redraw);

  settings.setValue (pfmEdit::tr ("unload program name"), options->unload_prog);
  settings.setValue (pfmEdit::tr ("auto unload flag"), options->auto_unload);

  settings.setValue (pfmEdit::tr ("last rock feature descriptor index"), options->last_rock_feature_desc);

  settings.setValue (pfmEdit::tr ("last offshore feature descriptor index"), options->last_offshore_feature_desc);

  settings.setValue (pfmEdit::tr ("last light feature descriptor index"), options->last_light_feature_desc);

  settings.setValue (pfmEdit::tr ("last lidar feature descriptor index"), options->last_lidar_feature_desc);

  settings.setValue (pfmEdit::tr ("last feature description"), options->last_feature_description);

  settings.setValue (pfmEdit::tr ("last feature remarks"), options->last_feature_remarks);

  settings.setValue (pfmEdit::tr ("overlap percent"), options->overlap_percent);

  settings.setValue (pfmEdit::tr ("display grid flag"), options->display_grid);

  settings.setValue (pfmEdit::tr ("plan view color by line flag"), options->plan_view_by_line);
  settings.setValue (pfmEdit::tr ("alternate view color by line flag"), options->alt_view_by_line);

  settings.setValue (pfmEdit::tr ("deep filter only flag"), options->deep_filter_only);

  settings.setValue (pfmEdit::tr ("filter standard deviation"), options->filterSTD);

  settings.setValue (pfmEdit::tr ("feature filter exclusion radius"), (NV_FLOAT64) options->feature_radius);

  settings.setValue (pfmEdit::tr ("undo levels"), options->undo_levels);

  settings.setValue (pfmEdit::tr ("kill and respawn flag"), options->kill_and_respawn);

  for (NV_INT32 i = 0 ; i < NUM_ATTR + PRE_ATTR ; i++)
    {
      QString name = pfmEdit::tr ("Attribute %1 range minimum").arg (i, 2, 10, QChar ('0'));
      settings.setValue (name, (NV_FLOAT64) options->attr_filter_range[i][0]);

      name = pfmEdit::tr ("Attribute %1 range maximum").arg (i, 2, 10, QChar ('0'));
      settings.setValue (name, (NV_FLOAT64) options->attr_filter_range[i][1]);
    }

  settings.setValue (pfmEdit::tr ("av interface box size"), options->avInterfaceBoxSize);
  settings.setValue (pfmEdit::tr ("distance threshhold"), options->distThresh);


  //  This is the killed flag that can be set when pfmView exits.  We always set this to 0 since, if this program is running,
  //  pfmView must be running.

  settings.setValue (pfmEdit::tr ("pfmView killed"), 0);


  settings.setValue (pfmEdit::tr ("main window state"), mainWindow->saveState (NINT (settings_version * 100.0)));

  settings.setValue (pfmEdit::tr ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
