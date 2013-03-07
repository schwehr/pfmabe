#include "geoSwath3D.hpp"


NV_FLOAT64 settings_version = 3.10;


//  Note to self - I'm not saving the contour intervals because they always get passed in from pfmView

NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString buttonHotKey[HOTKEYS];
  QString progHotKey[NUMPROGS];


  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);


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


      QString string = misc->buttonText[i] + geoSwath3D::tr (" hot key");
      options->buttonAccel[i] = settings.value (string, options->buttonAccel[i]).toString ();
    }

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      //  Save the defaults so we can check them later.

      progHotKey[i] = options->hotkey[i];


      QString name = options->name[i] + geoSwath3D::tr (" Hotkey");
      options->hotkey[i] = settings.value (name, options->hotkey[i]).toString ();

      name = options->name[i] + geoSwath3D::tr (" Action Key");
      options->action[i] = settings.value (name, options->action[i]).toString ();

      name = options->name[i] + geoSwath3D::tr (" State");
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
      QMessageBox::warning (0, geoSwath3D::tr ("geoSwath3D settings"),
                            geoSwath3D::tr ("Hot key conflicts have been detected when reaing program settings!<br><br>") +
                            geoSwath3D::tr ("Some of your hotkeys have been reset to default values!"));
    }


  saved_version = settings.value (geoSwath3D::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->flag_index = settings.value (geoSwath3D::tr ("flag index"), options->flag_index).toInt ();

  options->zoom_percent = settings.value (geoSwath3D::tr ("zoom percentage"), options->zoom_percent).toInt ();

  options->exaggeration = settings.value (geoSwath3D::tr ("Z exaggeration"), options->exaggeration).toDouble ();

  options->zx_rotation = (NV_FLOAT32) settings.value (geoSwath3D::tr ("ZX rotation"), (NV_FLOAT64) options->zx_rotation).toDouble ();

  options->y_rotation = (NV_FLOAT32) settings.value (geoSwath3D::tr ("Y rotation"), (NV_FLOAT64) options->y_rotation).toDouble ();

  options->position_form = settings.value (geoSwath3D::tr ("position form"), options->position_form).toInt ();

  options->point_size = settings.value (geoSwath3D::tr ("point size"), options->point_size).toInt ();

  options->point_limit = settings.value (geoSwath3D::tr ("point limit"), options->point_limit).toInt ();

  options->slice_percent = settings.value (geoSwath3D::tr ("slice percentage"), options->slice_percent).toInt ();

  options->slice_alpha = settings.value (geoSwath3D::tr ("slice alpha"), options->slice_alpha).toInt ();

  options->min_window_size = (NV_FLOAT32) settings.value (geoSwath3D::tr ("minimum Z window size"),
                                                          (NV_FLOAT64) options->min_window_size).toDouble ();

  options->iho_min_window = settings.value (geoSwath3D::tr ("IHO min window"), options->iho_min_window).toInt ();

  options->function = settings.value (geoSwath3D::tr ("function"), options->function).toInt ();

  NV_INT32 red = settings.value (geoSwath3D::tr ("edit color/red"), options->edit_color.red ()).toInt ();
  NV_INT32 green = settings.value (geoSwath3D::tr ("edit color/green"), options->edit_color.green ()).toInt ();
  NV_INT32 blue = settings.value (geoSwath3D::tr ("edit color/blue"), options->edit_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (geoSwath3D::tr ("edit color/alpha"), options->edit_color.alpha ()).toInt ();
  options->edit_color.setRgb (red, green, blue, alpha);

  red = settings.value (geoSwath3D::tr ("tracker color/red"), options->tracker_color.red ()).toInt ();
  green = settings.value (geoSwath3D::tr ("tracker color/green"), options->tracker_color.green ()).toInt ();
  blue = settings.value (geoSwath3D::tr ("tracker color/blue"), options->tracker_color.blue ()).toInt ();
  alpha = settings.value (geoSwath3D::tr ("tracker color/alpha"), options->tracker_color.alpha ()).toInt ();
  options->tracker_color.setRgb (red, green, blue, alpha);

  red = settings.value (geoSwath3D::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (geoSwath3D::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (geoSwath3D::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (geoSwath3D::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);

  red = settings.value (geoSwath3D::tr ("scale color/red"), options->scale_color.red ()).toInt ();
  green = settings.value (geoSwath3D::tr ("scale color/green"), options->scale_color.green ()).toInt ();
  blue = settings.value (geoSwath3D::tr ("scale color/blue"), options->scale_color.blue ()).toInt ();
  alpha = settings.value (geoSwath3D::tr ("scale color/alpha"), options->scale_color.alpha ()).toInt ();
  options->scale_color.setRgb (red, green, blue, alpha);


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      QString string;
      string.sprintf (geoSwath3D::tr ("Wave color %02d").toAscii (), i);
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


  options->draw_scale = settings.value (geoSwath3D::tr ("draw scale flag"), options->draw_scale).toBool ();

  options->filterSTD = settings.value (geoSwath3D::tr ("filter standard deviation"), options->filterSTD).toDouble ();

  options->deep_filter_only = settings.value (geoSwath3D::tr ("deep filter only flag"), options->deep_filter_only).toBool ();

  options->undo_levels = settings.value (geoSwath3D::tr ("undo levels"), options->undo_levels).toInt ();

  options->input_dir = settings.value (geoSwath3D::tr ("input directory"), options->input_dir).toString ();

  options->inputFilter = settings.value (geoSwath3D::tr ("input filter"), options->inputFilter).toString ();


  mainWindow->restoreState (settings.value (geoSwath3D::tr ("main window state")).toByteArray (),
			    NINT (settings_version * 100.0L));

  mainWindow->restoreGeometry (settings.value (geoSwath3D::tr ("main window geometry")).toByteArray ());


  settings.endGroup ();


  return (NVTrue);
}


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      QString string = misc->buttonText[i] + geoSwath3D::tr (" hot key");
      settings.setValue (string, options->buttonAccel[i]);
    }

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      QString name = options->name[i] + geoSwath3D::tr (" Hotkey");
      settings.setValue (name, options->hotkey[i]);

      name = options->name[i] + geoSwath3D::tr (" Action Key");
      settings.setValue (name, options->action[i]);

      name = options->name[i] + geoSwath3D::tr (" State");
      settings.setValue (name, options->state[i]);
    }

  settings.setValue (geoSwath3D::tr ("settings version"), settings_version);

  settings.setValue (geoSwath3D::tr ("flag index"), options->flag_index);

  settings.setValue (geoSwath3D::tr ("zoom percentage"), options->zoom_percent);

  settings.setValue (geoSwath3D::tr ("Z exaggeration"), options->exaggeration);

  settings.setValue (geoSwath3D::tr ("ZX rotation"), (NV_FLOAT64) options->zx_rotation);

  settings.setValue (geoSwath3D::tr ("Y rotation"), (NV_FLOAT64) options->y_rotation);

  settings.setValue (geoSwath3D::tr ("main button icon size"), options->main_button_icon_size);

  settings.setValue (geoSwath3D::tr ("position form"), options->position_form);

  settings.setValue (geoSwath3D::tr ("point size"), options->point_size);

  settings.setValue (geoSwath3D::tr ("point limit"), options->point_limit);

  settings.setValue (geoSwath3D::tr ("slice percentage"), options->slice_percent);

  settings.setValue (geoSwath3D::tr ("slice alpha"), options->slice_alpha);

  settings.setValue (geoSwath3D::tr ("minimum Z window size"), (NV_FLOAT64) options->min_window_size);

  settings.setValue (geoSwath3D::tr ("IHO min window"), options->iho_min_window);

  settings.setValue (geoSwath3D::tr ("function"), options->function);

  settings.setValue (geoSwath3D::tr ("edit color/red"), options->edit_color.red ());
  settings.setValue (geoSwath3D::tr ("edit color/green"), options->edit_color.green ());
  settings.setValue (geoSwath3D::tr ("edit color/blue"), options->edit_color.blue ());
  settings.setValue (geoSwath3D::tr ("edit color/alpha"), options->edit_color.alpha ());

  settings.setValue (geoSwath3D::tr ("tracker color/red"), options->tracker_color.red ());
  settings.setValue (geoSwath3D::tr ("tracker color/green"), options->tracker_color.green ());
  settings.setValue (geoSwath3D::tr ("tracker color/blue"), options->tracker_color.blue ());
  settings.setValue (geoSwath3D::tr ("tracker color/alpha"), options->tracker_color.alpha ());

  settings.setValue (geoSwath3D::tr ("background color/red"), options->background_color.red ());
  settings.setValue (geoSwath3D::tr ("background color/green"), options->background_color.green ());
  settings.setValue (geoSwath3D::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (geoSwath3D::tr ("background color/alpha"), options->background_color.alpha ());

  settings.setValue (geoSwath3D::tr ("scale color/red"), options->scale_color.red ());
  settings.setValue (geoSwath3D::tr ("scale color/green"), options->scale_color.green ());
  settings.setValue (geoSwath3D::tr ("scale color/blue"), options->scale_color.blue ());
  settings.setValue (geoSwath3D::tr ("scale color/alpha"), options->scale_color.alpha ());


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      QString string;
      string.sprintf (geoSwath3D::tr ("Wave color %02d").toAscii (), i);
      settings.setValue (string + "/red", options->waveColor[i].red ());
      settings.setValue (string + "/green", options->waveColor[i].green ());
      settings.setValue (string + "/blue", options->waveColor[i].blue ());
      settings.setValue (string + "/alpha", options->waveColor[i].alpha ());
    }


  settings.setValue (geoSwath3D::tr ("draw scale flag"), options->draw_scale);

  settings.setValue (geoSwath3D::tr ("deep filter only flag"), options->deep_filter_only);

  settings.setValue (geoSwath3D::tr ("filter standard deviation"), options->filterSTD);

  settings.setValue (geoSwath3D::tr ("undo levels"), options->undo_levels);

  settings.setValue (geoSwath3D::tr ("input directory"), options->input_dir);

  settings.setValue (geoSwath3D::tr ("input filter"), options->inputFilter);


  settings.setValue (geoSwath3D::tr ("main window state"), mainWindow->saveState (NINT (settings_version * 100.0L)));

  settings.setValue (geoSwath3D::tr ("main window geometry"), mainWindow->saveGeometry ());


  settings.endGroup ();
}
