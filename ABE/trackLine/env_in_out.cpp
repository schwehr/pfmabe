#include "trackLine.hpp"


NV_FLOAT64 settings_version = 1.00;


void envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (trackLine::tr ("navo.navy.mil"), trackLine::tr ("trackLine"));

  settings.beginGroup (trackLine::tr ("trackLine"));

  saved_version = settings.value (trackLine::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->width = settings.value (trackLine::tr ("width"), options->width).toInt ();
  options->height = settings.value (trackLine::tr ("height"), options->height).toInt ();

  options->window_x = settings.value (trackLine::tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (trackLine::tr ("y position"), options->window_y).toInt ();

  options->inputFilter = settings.value (trackLine::tr ("input filter"), options->inputFilter).toString ();

  options->heading[TRACKLINE_GSF] = settings.value (trackLine::tr ("GSF heading limit"), options->heading[TRACKLINE_GSF]).toDouble ();
  options->speed[TRACKLINE_GSF] = settings.value (trackLine::tr ("GSF speed limit"), options->speed[TRACKLINE_GSF]).toDouble ();

  options->heading[TRACKLINE_POS] = settings.value (trackLine::tr ("POS heading limit"), options->heading[TRACKLINE_POS]).toDouble ();
  options->speed[TRACKLINE_POS] = settings.value (trackLine::tr ("POS speed limit"), options->speed[TRACKLINE_POS]).toDouble ();

  options->heading[TRACKLINE_WLF] = settings.value (trackLine::tr ("WLF heading limit"), options->heading[TRACKLINE_WLF]).toDouble ();
  options->speed[TRACKLINE_WLF] = settings.value (trackLine::tr ("WLF speed limit"), options->speed[TRACKLINE_WLF]).toDouble ();

  options->heading[TRACKLINE_HWK] = settings.value (trackLine::tr ("HWK heading limit"), options->heading[TRACKLINE_HWK]).toDouble ();
  options->speed[TRACKLINE_HWK] = settings.value (trackLine::tr ("HWK speed limit"), options->speed[TRACKLINE_HWK]).toDouble ();

  settings.endGroup ();
}


void envout (OPTIONS *options)
{
  QSettings settings (trackLine::tr ("navo.navy.mil"), trackLine::tr ("trackLine"));

  settings.beginGroup (trackLine::tr ("trackLine"));


  settings.setValue (trackLine::tr ("settings version"), settings_version);


  settings.setValue (trackLine::tr ("width"), options->width);
  settings.setValue (trackLine::tr ("height"), options->height);

  settings.setValue (trackLine::tr ("x position"), options->window_x);
  settings.setValue (trackLine::tr ("y position"), options->window_y);

  settings.setValue (trackLine::tr ("input filter"), options->inputFilter);

  settings.setValue (trackLine::tr ("GSF heading limit"), options->heading[TRACKLINE_GSF]);
  settings.setValue (trackLine::tr ("GSF speed limit"), options->speed[TRACKLINE_GSF]);

  settings.setValue (trackLine::tr ("POS heading limit"), options->heading[TRACKLINE_POS]);
  settings.setValue (trackLine::tr ("POS speed limit"), options->speed[TRACKLINE_POS]);

  settings.setValue (trackLine::tr ("WLF heading limit"), options->heading[TRACKLINE_WLF]);
  settings.setValue (trackLine::tr ("WLF speed limit"), options->speed[TRACKLINE_WLF]);

  settings.setValue (trackLine::tr ("HWK heading limit"), options->heading[TRACKLINE_HWK]);
  settings.setValue (trackLine::tr ("HWK speed limit"), options->speed[TRACKLINE_HWK]);

  settings.endGroup ();
}
