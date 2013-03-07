#include "areaCheck.hpp"


NV_FLOAT64 settings_version = 5.01;


NV_BOOL envin (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;


  QSettings settings (areaCheck::tr ("navo.navy.mil"), areaCheck::tr ("areaCheck"));


  settings.beginGroup (areaCheck::tr ("areaCheck"));


  saved_version = settings.value (areaCheck::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->coast = settings.value (areaCheck::tr ("coastline flag"), options->coast).toBool ();

  options->display_children = settings.value (areaCheck::tr ("sub-feature flag"), options->display_children).toBool ();
  options->display_feature_info = settings.value (areaCheck::tr ("feature info flag"), options->display_feature_info).toBool ();
  options->display_feature_poly = settings.value (areaCheck::tr ("feature polygon flag"), options->display_feature_poly).toBool ();

  options->inputFilter = settings.value (areaCheck::tr ("input filter"), options->inputFilter).toString ();
  options->outputFilter = settings.value (areaCheck::tr ("output filter"), options->outputFilter).toString ();

  options->position_form = settings.value (areaCheck::tr ("position form"), options->position_form).toInt ();

  NV_INT32 red = settings.value (areaCheck::tr ("coast color/red"), options->coast_color.red ()).toInt ();
  NV_INT32 green = settings.value (areaCheck::tr ("coast color/green"), options->coast_color.green ()).toInt ();
  NV_INT32 blue = settings.value (areaCheck::tr ("coast color/blue"), options->coast_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (areaCheck::tr ("coast color/alpha"), options->coast_color.alpha ()).toInt ();
  options->coast_color.setRgb (red, green, blue, alpha);


  red = settings.value (areaCheck::tr ("landmask color/red"), options->mask_color.red ()).toInt ();
  green = settings.value (areaCheck::tr ("landmask color/green"), options->mask_color.green ()).toInt ();
  blue = settings.value (areaCheck::tr ("landmask color/blue"), options->mask_color.blue ()).toInt ();
  alpha = settings.value (areaCheck::tr ("landmask color/alpha"), options->mask_color.alpha ()).toInt ();
  options->mask_color.setRgb (red, green, blue, alpha);


  red = settings.value (areaCheck::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (areaCheck::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (areaCheck::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (areaCheck::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);


  for (NV_INT32 i = 0 ; i < NUM_TYPES ; i++)
    {
      QString name = misc->type_name[i] + areaCheck::tr (" color/");

      red = settings.value (name + areaCheck::tr ("red"), options->color[i].red ()).toInt ();
      green = settings.value (name + areaCheck::tr ("green"), options->color[i].green ()).toInt ();
      blue = settings.value (name + areaCheck::tr ("blue"), options->color[i].blue ()).toInt ();
      alpha = settings.value (name + areaCheck::tr ("alpha"), options->color[i].alpha ()).toInt ();
      options->color[i].setRgb (red, green, blue, alpha);
    }
  
  
  red = settings.value (areaCheck::tr ("highlight color/red"), options->highlight_color.red ()).toInt ();
  green = settings.value (areaCheck::tr ("highlight color/green"), options->highlight_color.green ()).toInt ();
  blue = settings.value (areaCheck::tr ("highlight color/blue"), options->highlight_color.blue ()).toInt ();
  alpha = settings.value (areaCheck::tr ("highlight color/alpha"), options->highlight_color.alpha ()).toInt ();
  options->highlight_color.setRgb (red, green, blue, alpha);

  
  options->sunopts.sun.x = settings.value (areaCheck::tr ("sunopts sun x"), options->sunopts.sun.x).toDouble ();

  options->sunopts.sun.y = settings.value (areaCheck::tr ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.sun.z = settings.value (areaCheck::tr ("sunopts sun y"), options->sunopts.sun.y).toDouble ();

  options->sunopts.azimuth = settings.value (areaCheck::tr ("sunopts azimuth"), options->sunopts.azimuth).toDouble ();

  options->sunopts.elevation = settings.value (areaCheck::tr ("sunopts elevation"), 
                                               options->sunopts.elevation).toDouble ();

  options->sunopts.exag = settings.value (areaCheck::tr ("sunopts exag"), options->sunopts.exag).toDouble ();

  options->sunopts.power_cos = settings.value (areaCheck::tr ("sunopts power cosine"), 
                                               options->sunopts.power_cos).toDouble ();

  options->sunopts.num_shades = settings.value (areaCheck::tr ("sunopts number of shades"), 
                                                options->sunopts.num_shades).toInt ();


  options->stoplight = settings.value (areaCheck::tr ("stoplight"), options->stoplight).toBool ();

  options->minstop = (NV_FLOAT32) settings.value (areaCheck::tr ("min stoplight"), 
                                                  (NV_FLOAT64) options->minstop).toDouble ();
  options->maxstop = (NV_FLOAT32) settings.value (areaCheck::tr ("max stoplight"),
                                                  (NV_FLOAT64) options->maxstop).toDouble ();

  options->input_dir = settings.value (areaCheck::tr ("input directory"), options->input_dir).toString ();
  options->output_dir = settings.value (areaCheck::tr ("output directory"), options->output_dir).toString ();

  mainWindow->restoreState (settings.value (areaCheck::tr ("main window state")).toByteArray (), 
                            NINT (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (areaCheck::tr ("main window geometry")).toByteArray ());

  settings.endGroup ();

  return (NVTrue);
}


void envout (OPTIONS *options, MISC *misc, QMainWindow *mainWindow)
{
  QSettings settings (areaCheck::tr ("navo.navy.mil"), areaCheck::tr ("areaCheck"));


  settings.beginGroup (areaCheck::tr ("areaCheck"));


  settings.setValue (areaCheck::tr ("settings version"), settings_version);

  settings.setValue (areaCheck::tr ("coastline flag"), options->coast);

  settings.setValue (areaCheck::tr ("sub-feature flag"), options->display_children);
  settings.setValue (areaCheck::tr ("feature info flag"), options->display_feature_info);
  settings.setValue (areaCheck::tr ("feature polygon flag"), options->display_feature_poly);


  settings.setValue (areaCheck::tr ("input filter"), options->inputFilter);
  settings.setValue (areaCheck::tr ("output filter"), options->outputFilter);

  settings.setValue (areaCheck::tr ("position form"), options->position_form);


  settings.setValue (areaCheck::tr ("coast color/red"), options->coast_color.red ());
  settings.setValue (areaCheck::tr ("coast color/green"), options->coast_color.green ());
  settings.setValue (areaCheck::tr ("coast color/blue"), options->coast_color.blue ());
  settings.setValue (areaCheck::tr ("coast color/alpha"), options->coast_color.alpha ());


  settings.setValue (areaCheck::tr ("landmask color/red"), options->mask_color.red ());
  settings.setValue (areaCheck::tr ("landmask color/green"), options->mask_color.green ());
  settings.setValue (areaCheck::tr ("landmask color/blue"), options->mask_color.blue ());
  settings.setValue (areaCheck::tr ("landmask color/alpha"), options->mask_color.alpha ());


  settings.setValue (areaCheck::tr ("background color/red"), options->background_color.red ());
  settings.setValue (areaCheck::tr ("background color/green"), options->background_color.green ());
  settings.setValue (areaCheck::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (areaCheck::tr ("background color/alpha"), options->background_color.alpha ());

  
  for (NV_INT32 i = 0 ; i < NUM_TYPES ; i++)
    {
      QString name = misc->type_name[i] + areaCheck::tr (" color/");

      settings.setValue (name + areaCheck::tr ("red"), options->color[i].red ());
      settings.setValue (name + areaCheck::tr ("green"), options->color[i].green ());
      settings.setValue (name + areaCheck::tr ("blue"), options->color[i].blue ());
      settings.setValue (name + areaCheck::tr ("alpha"), options->color[i].alpha ());
    }

  
  settings.setValue (areaCheck::tr ("highlight color/red"), options->highlight_color.red ());
  settings.setValue (areaCheck::tr ("highlight color/green"), options->highlight_color.green ());
  settings.setValue (areaCheck::tr ("highlight color/blue"), options->highlight_color.blue ());
  settings.setValue (areaCheck::tr ("highlight color/alpha"), options->highlight_color.alpha ());

  
  settings.setValue (areaCheck::tr ("sunopts sun x"), options->sunopts.sun.x);
  settings.setValue (areaCheck::tr ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (areaCheck::tr ("sunopts sun y"), options->sunopts.sun.y);
  settings.setValue (areaCheck::tr ("sunopts azimuth"), options->sunopts.azimuth);
  settings.setValue (areaCheck::tr ("sunopts elevation"), options->sunopts.elevation);
  settings.setValue (areaCheck::tr ("sunopts exag"), options->sunopts.exag);
  settings.setValue (areaCheck::tr ("sunopts power cosine"), options->sunopts.power_cos);
  settings.setValue (areaCheck::tr ("sunopts number of shades"), options->sunopts.num_shades);


  settings.setValue (areaCheck::tr ("stoplight"), options->stoplight);

  settings.setValue (areaCheck::tr ("min stoplight"), (NV_FLOAT64) options->minstop);
  settings.setValue (areaCheck::tr ("max stoplight"), (NV_FLOAT64) options->maxstop);

  settings.setValue (areaCheck::tr ("input directory"), options->input_dir);
  settings.setValue (areaCheck::tr ("output directory"), options->output_dir);

  settings.setValue (areaCheck::tr ("main window state"), mainWindow->saveState (NINT (settings_version * 100.0)));

  settings.setValue (areaCheck::tr ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
