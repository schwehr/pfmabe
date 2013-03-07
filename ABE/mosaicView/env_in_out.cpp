#include "mosaicView.hpp"


NV_FLOAT64 settings_version = 2.00;


void envin (OPTIONS *options, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (mosaicView::tr ("navo.navy.mil"), mosaicView::tr ("mosaicView"));

  settings.beginGroup (mosaicView::tr ("mosaicView"));

  saved_version = settings.value (mosaicView::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->coast = settings.value (mosaicView::tr ("coast flag"), options->coast).toBool ();
  options->landmask = settings.value (mosaicView::tr ("landmask flag"), options->landmask).toBool ();

  options->position_form = settings.value (mosaicView::tr ("position form"), options->position_form).toInt ();

  options->zoom_percent = settings.value (mosaicView::tr ("zoom percentage"), options->zoom_percent).toInt ();

  NV_INT32 red = settings.value (mosaicView::tr ("marker color/red"), options->marker_color.red ()).toInt ();
  NV_INT32 green = settings.value (mosaicView::tr ("marker color/green"), options->marker_color.green ()).toInt ();
  NV_INT32 blue = settings.value (mosaicView::tr ("marker color/blue"), options->marker_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (mosaicView::tr ("marker color/alpha"), options->marker_color.alpha ()).toInt ();
  options->marker_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("coast color/red"), options->coast_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("coast color/green"), options->coast_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("coast color/blue"), options->coast_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("coast color/alpha"), options->coast_color.alpha ()).toInt ();
  options->coast_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("landmask color/red"), options->landmask_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("landmask color/green"), options->landmask_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("landmask color/blue"), options->landmask_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("landmask color/alpha"), options->landmask_color.alpha ()).toInt ();
  options->landmask_color.setRgb (red, green, blue, alpha);

  
  red = settings.value (mosaicView::tr ("feature color/red"), options->feature_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("feature color/green"), options->feature_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("feature color/blue"), options->feature_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("feature color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("feature info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("feature info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("feature info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("feature info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("feature poly color/red"), options->feature_poly_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("feature poly color/green"), options->feature_poly_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("feature poly color/blue"), options->feature_poly_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ()).toInt ();
  options->feature_poly_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("highlight color/red"), options->highlight_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("highlight color/green"), options->highlight_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("highlight color/blue"), options->highlight_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("highlight color/alpha"), options->highlight_color.alpha ()).toInt ();
  options->highlight_color.setRgb (red, green, blue, alpha);


  red = settings.value (mosaicView::tr ("rect color/red"), options->rect_color.red ()).toInt ();
  green = settings.value (mosaicView::tr ("rect color/green"), options->rect_color.green ()).toInt ();
  blue = settings.value (mosaicView::tr ("rect color/blue"), options->rect_color.blue ()).toInt ();
  alpha = settings.value (mosaicView::tr ("rect color/alpha"), options->rect_color.alpha ()).toInt ();
  options->rect_color.setRgb (red, green, blue, alpha);

  

  options->display_feature = settings.value (mosaicView::tr ("view feature positions"), 
                                            options->display_feature).toBool ();

  options->display_children = settings.value (mosaicView::tr ("view sub-feature positions"), 
                                              options->display_children).toBool ();

  options->display_feature_info = settings.value (mosaicView::tr ("view feature info"), 
                                                 options->display_feature_info).toBool ();

  options->display_feature_poly = settings.value (mosaicView::tr ("view feature polygon"), 
                                                 options->display_feature_poly).toBool ();

  options->feature_diameter = (NV_FLOAT32) settings.value (mosaicView::tr ("feature diameter"), 
                                                          (NV_FLOAT64) options->feature_diameter).toDouble ();


  options->last_rock_feature_desc = settings.value (mosaicView::tr ("last rock feature descriptor index"), 
                                                   options->last_rock_feature_desc).toInt ();

  options->last_offshore_feature_desc = 
    settings.value (mosaicView::tr ("last offshore feature descriptor index"), 
                    options->last_offshore_feature_desc).toInt ();

  options->last_light_feature_desc = settings.value (mosaicView::tr ("last light feature descriptor index"), 
                                                    options->last_light_feature_desc).toInt ();

  options->last_lidar_feature_desc = settings.value (mosaicView::tr ("last lidar feature descriptor index"), 
                                                    options->last_lidar_feature_desc).toInt ();

  options->last_feature_description = settings.value (mosaicView::tr ("last feature description"), 
                                                      options->last_feature_description).toString ();

  options->last_feature_remarks = settings.value (mosaicView::tr ("last feature remarks"), 
                                                  options->last_feature_remarks).toString ();

  options->feature_diameter = (NV_FLOAT32) settings.value (mosaicView::tr ("feature diameter"), 
                                                          (NV_FLOAT64) options->feature_diameter).toDouble ();

  options->screenshot_delay = settings.value (mosaicView::tr ("screenshot delay"), options->screenshot_delay).toInt ();

  options->new_feature = settings.value (mosaicView::tr ("new feature counter"), options->new_feature).toInt ();

  options->startup_message = settings.value (mosaicView::tr ("startup message flag"), options->startup_message).toBool ();

  mainWindow->restoreState (settings.value (mosaicView::tr ("main window state")).toByteArray (), 
                            NINT (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (mosaicView::tr ("main window geometry")).toByteArray ());

  settings.endGroup ();
}


void envout (OPTIONS *options, QMainWindow *mainWindow)
{
  QSettings settings (mosaicView::tr ("navo.navy.mil"), mosaicView::tr ("mosaicView"));

  settings.beginGroup (mosaicView::tr ("mosaicView"));


  settings.setValue (mosaicView::tr ("settings version"), settings_version);


  settings.setValue (mosaicView::tr ("coast flag"), options->coast);
  settings.setValue (mosaicView::tr ("landmask flag"), options->landmask);

  settings.setValue (mosaicView::tr ("position form"), options->position_form);

  settings.setValue (mosaicView::tr ("zoom percentage"), options->zoom_percent);


  settings.setValue (mosaicView::tr ("marker color/red"), options->marker_color.red ());
  settings.setValue (mosaicView::tr ("marker color/green"), options->marker_color.green ());
  settings.setValue (mosaicView::tr ("marker color/blue"), options->marker_color.blue ());
  settings.setValue (mosaicView::tr ("marker color/alpha"), options->marker_color.alpha ());


  settings.setValue (mosaicView::tr ("coast color/red"), options->coast_color.red ());
  settings.setValue (mosaicView::tr ("coast color/green"), options->coast_color.green ());
  settings.setValue (mosaicView::tr ("coast color/blue"), options->coast_color.blue ());
  settings.setValue (mosaicView::tr ("coast color/alpha"), options->coast_color.alpha ());


  settings.setValue (mosaicView::tr ("landmask color/red"), options->landmask_color.red ());
  settings.setValue (mosaicView::tr ("landmask color/green"), options->landmask_color.green ());
  settings.setValue (mosaicView::tr ("landmask color/blue"), options->landmask_color.blue ());
  settings.setValue (mosaicView::tr ("landmask color/alpha"), options->landmask_color.alpha ());


  settings.setValue (mosaicView::tr ("rect color/red"), options->rect_color.red ());
  settings.setValue (mosaicView::tr ("rect color/green"), options->rect_color.green ());
  settings.setValue (mosaicView::tr ("rect color/blue"), options->rect_color.blue ());
  settings.setValue (mosaicView::tr ("rect color/alpha"), options->rect_color.alpha ());


  settings.setValue (mosaicView::tr ("feature color/red"), options->feature_color.red ());
  settings.setValue (mosaicView::tr ("feature color/green"), options->feature_color.green ());
  settings.setValue (mosaicView::tr ("feature color/blue"), options->feature_color.blue ());
  settings.setValue (mosaicView::tr ("feature color/alpha"), options->feature_color.alpha ());


  settings.setValue (mosaicView::tr ("feature info color/red"), options->feature_info_color.red ());
  settings.setValue (mosaicView::tr ("feature info color/green"), options->feature_info_color.green ());
  settings.setValue (mosaicView::tr ("feature info color/blue"), options->feature_info_color.blue ());
  settings.setValue (mosaicView::tr ("feature info color/alpha"), options->feature_info_color.alpha ());


  settings.setValue (mosaicView::tr ("feature poly color/red"), options->feature_poly_color.red ());
  settings.setValue (mosaicView::tr ("feature poly color/green"), options->feature_poly_color.green ());
  settings.setValue (mosaicView::tr ("feature poly color/blue"), options->feature_poly_color.blue ());
  settings.setValue (mosaicView::tr ("feature poly color/alpha"), options->feature_poly_color.alpha ());


  settings.setValue (mosaicView::tr ("highlight color/red"), options->highlight_color.red ());
  settings.setValue (mosaicView::tr ("highlight color/green"), options->highlight_color.green ());
  settings.setValue (mosaicView::tr ("highlight color/blue"), options->highlight_color.blue ());
  settings.setValue (mosaicView::tr ("highlight color/alpha"), options->highlight_color.alpha ());


  settings.setValue (mosaicView::tr ("view feature positions"), options->display_feature);
  settings.setValue (mosaicView::tr ("view feature info"), options->display_feature_info);
  settings.setValue (mosaicView::tr ("view feature polygon"), options->display_feature_poly);

  settings.setValue (mosaicView::tr ("feature diameter"), (NV_FLOAT64) options->feature_diameter);

  settings.setValue (mosaicView::tr ("last rock feature descriptor index"), options->last_rock_feature_desc);

  settings.setValue (mosaicView::tr ("last offshore feature descriptor index"), options->last_offshore_feature_desc);

  settings.setValue (mosaicView::tr ("last light feature descriptor index"), options->last_light_feature_desc);

  settings.setValue (mosaicView::tr ("last lidar feature descriptor index"), options->last_lidar_feature_desc);

  settings.setValue (mosaicView::tr ("feature diameter"), (NV_FLOAT64) options->feature_diameter);

  settings.setValue (mosaicView::tr ("screenshot delay"), options->screenshot_delay);

  settings.setValue (mosaicView::tr ("new feature counter"), options->new_feature);

  settings.setValue (mosaicView::tr ("startup message flag"), options->startup_message);

  settings.setValue (mosaicView::tr ("main window state"), mainWindow->saveState (NINT (settings_version * 100.0)));

  settings.setValue (mosaicView::tr ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
