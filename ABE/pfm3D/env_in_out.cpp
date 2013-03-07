#include "pfm3D.hpp"


NV_FLOAT64 settings_version = 2.30;


NV_BOOL envin (OPTIONS *options, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (pfm3D::tr ("navo.navy.mil"), pfm3D::tr ("pfm3D"));

  settings.beginGroup (pfm3D::tr ("pfm3D"));

  saved_version = settings.value (pfm3D::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->edit_mode = settings.value (pfm3D::tr ("edit mode"), options->edit_mode).toInt ();

  options->position_form = settings.value (pfm3D::tr ("position form"), options->position_form).toInt ();

  options->zoom_percent = settings.value (pfm3D::tr ("zoom percentage"), options->zoom_percent).toInt ();

  options->exaggeration = settings.value (pfm3D::tr ("z exaggeration"), options->exaggeration).toDouble ();

  options->draw_scale = settings.value (pfm3D::tr ("draw scale flag"), options->draw_scale).toBool ();

  options->feature_size = (NV_FLOAT32) settings.value (pfm3D::tr ("feature size"), options->feature_size).toDouble ();

  NV_INT32 red = settings.value (pfm3D::tr ("feature color/red"), options->feature_color.red ()).toInt ();
  NV_INT32 green = settings.value (pfm3D::tr ("feature color/green"), options->feature_color.green ()).toInt ();
  NV_INT32 blue = settings.value (pfm3D::tr ("feature color/blue"), options->feature_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (pfm3D::tr ("feature color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfm3D::tr ("feature info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("feature info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("feature info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("feature info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfm3D::tr ("feature highlight color/red"), options->highlight_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("feature highlight color/green"), options->highlight_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("feature highlight color/blue"), options->highlight_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("feature highlight color/alpha"), options->highlight_color.alpha ()).toInt ();
  options->highlight_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfm3D::tr ("tracker color/red"), options->tracker_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("tracker color/green"), options->tracker_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("tracker color/blue"), options->tracker_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("tracker color/alpha"), options->tracker_color.alpha ()).toInt ();
  options->tracker_color.setRgb (red, green, blue, alpha);


  red = settings.value (pfm3D::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfm3D::tr ("edit color/red"), options->edit_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("edit color/green"), options->edit_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("edit color/blue"), options->edit_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("edit color/alpha"), options->edit_color.alpha ()).toInt ();
  options->edit_color.setRgb (red, green, blue, alpha);

  red = settings.value (pfm3D::tr ("scale color/red"), options->scale_color.red ()).toInt ();
  green = settings.value (pfm3D::tr ("scale color/green"), options->scale_color.green ()).toInt ();
  blue = settings.value (pfm3D::tr ("scale color/blue"), options->scale_color.blue ()).toInt ();
  alpha = settings.value (pfm3D::tr ("scale color/alpha"), options->scale_color.alpha ()).toInt ();
  options->scale_color.setRgb (red, green, blue, alpha);


  options->display_feature = settings.value (pfm3D::tr ("view feature positions"), options->display_feature).toInt ();

  options->display_children = settings.value (pfm3D::tr ("view sub-feature positions"), options->display_children).toBool ();

  options->display_feature_info = settings.value (pfm3D::tr ("view feature info"), options->display_feature_info).toBool ();

  mainWindow->restoreState (settings.value (pfm3D::tr ("main window state")).toByteArray (), (NV_INT32) (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (pfm3D::tr ("main window geometry")).toByteArray ());

  settings.endGroup ();

  return (NVTrue);
}


void envout (OPTIONS *options, QMainWindow *mainWindow)
{
  QSettings settings (pfm3D::tr ("navo.navy.mil"), pfm3D::tr ("pfm3D"));

  settings.beginGroup (pfm3D::tr ("pfm3D"));


  settings.setValue (pfm3D::tr ("settings version"), settings_version);


  settings.setValue (pfm3D::tr ("edit mode"), options->edit_mode);

  settings.setValue (pfm3D::tr ("position form"), options->position_form);

  settings.setValue (pfm3D::tr ("zoom percentage"), options->zoom_percent);

  settings.setValue (pfm3D::tr ("z exaggeration"), options->exaggeration);

  settings.setValue (pfm3D::tr ("draw scale flag"), options->draw_scale);

  settings.setValue (pfm3D::tr ("feature size"), (NV_FLOAT64) options->feature_size);

  settings.setValue (pfm3D::tr ("background color/red"), options->background_color.red ());
  settings.setValue (pfm3D::tr ("background color/green"), options->background_color.green ());
  settings.setValue (pfm3D::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (pfm3D::tr ("background color/alpha"), options->background_color.alpha ());

  settings.setValue (pfm3D::tr ("feature color/red"), options->feature_color.red ());
  settings.setValue (pfm3D::tr ("feature color/green"), options->feature_color.green ());
  settings.setValue (pfm3D::tr ("feature color/blue"), options->feature_color.blue ());
  settings.setValue (pfm3D::tr ("feature color/alpha"), options->feature_color.alpha ());


  settings.setValue (pfm3D::tr ("feature info color/red"), options->feature_info_color.red ());
  settings.setValue (pfm3D::tr ("feature info color/green"), options->feature_info_color.green ());
  settings.setValue (pfm3D::tr ("feature info color/blue"), options->feature_info_color.blue ());
  settings.setValue (pfm3D::tr ("feature info color/alpha"), options->feature_info_color.alpha ());


  settings.setValue (pfm3D::tr ("feature highlight color/red"), options->highlight_color.red ());
  settings.setValue (pfm3D::tr ("feature highlight color/green"), options->highlight_color.green ());
  settings.setValue (pfm3D::tr ("feature highlight color/blue"), options->highlight_color.blue ());
  settings.setValue (pfm3D::tr ("feature highlight color/alpha"), options->highlight_color.alpha ());


  settings.setValue (pfm3D::tr ("tracker color/red"), options->tracker_color.red ());
  settings.setValue (pfm3D::tr ("tracker color/green"), options->tracker_color.green ());
  settings.setValue (pfm3D::tr ("tracker color/blue"), options->tracker_color.blue ());
  settings.setValue (pfm3D::tr ("tracker color/alpha"), options->tracker_color.alpha ());


  settings.setValue (pfm3D::tr ("edit color/red"), options->edit_color.red ());
  settings.setValue (pfm3D::tr ("edit color/green"), options->edit_color.green ());
  settings.setValue (pfm3D::tr ("edit color/blue"), options->edit_color.blue ());
  settings.setValue (pfm3D::tr ("edit color/alpha"), options->edit_color.alpha ());


  settings.setValue (pfm3D::tr ("view feature positions"), options->display_feature);
  settings.setValue (pfm3D::tr ("view sub-feature positions"), options->display_children);
  settings.setValue (pfm3D::tr ("view feature info"), options->display_feature_info);

  settings.setValue (pfm3D::tr ("main window state"), mainWindow->saveState ((NV_INT32) (settings_version * 100.0)));

  settings.setValue (pfm3D::tr ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
