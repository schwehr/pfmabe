
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



#include "bagViewer.hpp"


NV_FLOAT64 settings_version = 1.12;


NV_BOOL envin (OPTIONS *options, QMainWindow *mainWindow)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (bagViewer::tr ("navo.navy.mil"), bagViewer::tr ("bagViewer"));

  settings.beginGroup (bagViewer::tr ("bagViewer"));

  saved_version = settings.value (bagViewer::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->position_form = settings.value (bagViewer::tr ("position form"), options->position_form).toInt ();

  options->exaggeration = settings.value (bagViewer::tr ("z exaggeration"), options->exaggeration).toDouble ();

  options->draw_scale = settings.value (bagViewer::tr ("draw scale flag"), options->draw_scale).toBool ();

  options->surface = settings.value (bagViewer::tr ("surface"), options->surface).toInt ();

  options->color_option = settings.value (bagViewer::tr ("color by uncertainty flag"), options->color_option).toBool ();

  options->display_tracking_list = settings.value (bagViewer::tr ("display tracking list flag"), options->display_tracking_list).toBool ();

  options->display_tracking_info = settings.value (bagViewer::tr ("display tracking list info"), options->display_tracking_info).toBool ();

  options->feature_size = (NV_FLOAT32) settings.value (bagViewer::tr ("tracking list marker size"),
                                                       (NV_FLOAT64) options->feature_size).toDouble ();

  options->input_dir = settings.value (bagViewer::tr ("input directory"), options->input_dir).toString ();
  options->image_dir = settings.value (bagViewer::tr ("image directory"), options->image_dir).toString ();

  NV_INT32 red = settings.value (bagViewer::tr ("tracker color/red"), options->tracker_color.red ()).toInt ();
  NV_INT32 green = settings.value (bagViewer::tr ("tracker color/green"), options->tracker_color.green ()).toInt ();
  NV_INT32 blue = settings.value (bagViewer::tr ("tracker color/blue"), options->tracker_color.blue ()).toInt ();
  NV_INT32 alpha = settings.value (bagViewer::tr ("tracker color/alpha"), options->tracker_color.alpha ()).toInt ();
  options->tracker_color.setRgb (red, green, blue, alpha);


  red = settings.value (bagViewer::tr ("background color/red"), options->background_color.red ()).toInt ();
  green = settings.value (bagViewer::tr ("background color/green"), options->background_color.green ()).toInt ();
  blue = settings.value (bagViewer::tr ("background color/blue"), options->background_color.blue ()).toInt ();
  alpha = settings.value (bagViewer::tr ("background color/alpha"), options->background_color.alpha ()).toInt ();
  options->background_color.setRgb (red, green, blue, alpha);


  red = settings.value (bagViewer::tr ("tracking list color/red"), options->feature_color.red ()).toInt ();
  green = settings.value (bagViewer::tr ("tracking list color/green"), options->feature_color.green ()).toInt ();
  blue = settings.value (bagViewer::tr ("tracking list color/blue"), options->feature_color.blue ()).toInt ();
  alpha = settings.value (bagViewer::tr ("tracking list color/alpha"), options->feature_color.alpha ()).toInt ();
  options->feature_color.setRgb (red, green, blue, alpha);


  red = settings.value (bagViewer::tr ("tracking list info color/red"), options->feature_info_color.red ()).toInt ();
  green = settings.value (bagViewer::tr ("tracking list info color/green"), options->feature_info_color.green ()).toInt ();
  blue = settings.value (bagViewer::tr ("tracking list info color/blue"), options->feature_info_color.blue ()).toInt ();
  alpha = settings.value (bagViewer::tr ("tracking list info color/alpha"), options->feature_info_color.alpha ()).toInt ();
  options->feature_info_color.setRgb (red, green, blue, alpha);


  red = settings.value (bagViewer::tr ("scale color/red"), options->scale_color.red ()).toInt ();
  green = settings.value (bagViewer::tr ("scale color/green"), options->scale_color.green ()).toInt ();
  blue = settings.value (bagViewer::tr ("scale color/blue"), options->scale_color.blue ()).toInt ();
  alpha = settings.value (bagViewer::tr ("scale color/alpha"), options->scale_color.alpha ()).toInt ();
  options->scale_color.setRgb (red, green, blue, alpha);


  mainWindow->restoreState (settings.value (bagViewer::tr ("main window state")).toByteArray (), 
                            (NV_INT32) (settings_version * 100.0));

  mainWindow->restoreGeometry (settings.value (bagViewer::tr ("main window geometry")).toByteArray ());

  settings.endGroup ();

  return (NVTrue);
}


void envout (OPTIONS *options, QMainWindow *mainWindow)
{
  QSettings settings (bagViewer::tr ("navo.navy.mil"), bagViewer::tr ("bagViewer"));

  settings.beginGroup (bagViewer::tr ("bagViewer"));


  settings.setValue (bagViewer::tr ("settings version"), settings_version);


  settings.setValue (bagViewer::tr ("position form"), options->position_form);

  settings.setValue (bagViewer::tr ("z exaggeration"), options->exaggeration);

  settings.setValue (bagViewer::tr ("draw scale flag"), options->draw_scale);

  settings.setValue (bagViewer::tr ("surface"), options->surface);

  settings.setValue (bagViewer::tr ("color by uncertainty flag"), options->color_option);

  settings.setValue (bagViewer::tr ("display tracking list flag"), options->display_tracking_list);

  settings.setValue (bagViewer::tr ("display tracking list info"), options->display_tracking_info);

  settings.setValue (bagViewer::tr ("tracking list marker size"), (NV_FLOAT64) options->feature_size);

  settings.setValue (bagViewer::tr ("input directory"), options->input_dir);
  settings.setValue (bagViewer::tr ("image directory"), options->image_dir);

  settings.setValue (bagViewer::tr ("background color/red"), options->background_color.red ());
  settings.setValue (bagViewer::tr ("background color/green"), options->background_color.green ());
  settings.setValue (bagViewer::tr ("background color/blue"), options->background_color.blue ());
  settings.setValue (bagViewer::tr ("background color/alpha"), options->background_color.alpha ());


  settings.setValue (bagViewer::tr ("tracker color/red"), options->tracker_color.red ());
  settings.setValue (bagViewer::tr ("tracker color/green"), options->tracker_color.green ());
  settings.setValue (bagViewer::tr ("tracker color/blue"), options->tracker_color.blue ());
  settings.setValue (bagViewer::tr ("tracker color/alpha"), options->tracker_color.alpha ());


  settings.setValue (bagViewer::tr ("tracking list color/red"), options->feature_color.red ());
  settings.setValue (bagViewer::tr ("tracking list color/green"), options->feature_color.green ());
  settings.setValue (bagViewer::tr ("tracking list color/blue"), options->feature_color.blue ());
  settings.setValue (bagViewer::tr ("tracking list color/alpha"), options->feature_color.alpha ());


  settings.setValue (bagViewer::tr ("tracking list info color/red"), options->feature_info_color.red ());
  settings.setValue (bagViewer::tr ("tracking list info color/green"), options->feature_info_color.green ());
  settings.setValue (bagViewer::tr ("tracking list info color/blue"), options->feature_info_color.blue ());
  settings.setValue (bagViewer::tr ("tracking list info color/alpha"), options->feature_info_color.alpha ());


  settings.setValue (bagViewer::tr ("scale color/red"), options->scale_color.red ());
  settings.setValue (bagViewer::tr ("scale color/green"), options->scale_color.green ());
  settings.setValue (bagViewer::tr ("scale color/blue"), options->scale_color.blue ());
  settings.setValue (bagViewer::tr ("scale color/alpha"), options->scale_color.alpha ());


  settings.setValue (bagViewer::tr ("main window state"), mainWindow->saveState ((NV_INT32) (settings_version * 100.0)));

  settings.setValue (bagViewer::tr ("main window geometry"), mainWindow->saveGeometry ());

  settings.endGroup ();
}
