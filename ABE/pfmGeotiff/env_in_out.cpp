
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



#include "pfmGeotiff.hpp"


NV_FLOAT64 settings_version = 5.25;


void envin (OPTIONS *options)
{
  NV_FLOAT64 saved_version = 0.0;


  QSettings settings (pfmGeotiff::tr ("navo.navy.mil"), pfmGeotiff::tr ("pfmGeotiff"));

  settings.beginGroup (pfmGeotiff::tr ("pfmGeotiff"));

  saved_version = settings.value (pfmGeotiff::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options->transparent = settings.value (pfmGeotiff::tr ("transparent"), options->transparent).toBool ();

  options->packbits = settings.value (pfmGeotiff::tr ("packbits compression flag"), options->packbits).toBool ();

  options->grey = settings.value (pfmGeotiff::tr ("32 bit floating point format"), options->grey).toBool ();

  options->restart = settings.value (pfmGeotiff::tr ("restart"), options->restart).toBool ();

  options->azimuth = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("azimuth"), (NV_FLOAT64) options->azimuth).toDouble ();

  options->elevation = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("elevation"), (NV_FLOAT64) options->elevation).toDouble ();

  options->exaggeration = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("exaggeration"), (NV_FLOAT64) options->exaggeration).toDouble ();

  options->saturation = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("saturation"), (NV_FLOAT64) options->saturation).toDouble ();

  options->value = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("value"), (NV_FLOAT64) options->value).toDouble ();

  options->start_hsv = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("start_hsv"), (NV_FLOAT64) options->start_hsv).toDouble ();

  options->end_hsv = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("end_hsv"), (NV_FLOAT64) options->end_hsv).toDouble ();

  options->surface = settings.value (pfmGeotiff::tr ("surface"), options->surface).toInt ();

  options->units = settings.value (pfmGeotiff::tr ("units"), options->units).toInt ();

  options->dumb = settings.value (pfmGeotiff::tr ("4800 ft per second flag"), options->dumb).toBool ();

  options->intrp = settings.value (pfmGeotiff::tr ("interpolated contours"), options->intrp).toBool ();

  options->smoothing_factor = settings.value (pfmGeotiff::tr ("smoothing level"), options->smoothing_factor).toBool ();

  options->navd88 = settings.value (pfmGeotiff::tr ("NAVD88 flag"), options->navd88).toBool ();

  options->elev = settings.value (pfmGeotiff::tr ("elevation flag"), options->elev).toBool ();

  options->cint = (NV_FLOAT32) settings.value (pfmGeotiff::tr ("contour interval"), (NV_FLOAT64) options->cint).toDouble ();

  options->input_dir = settings.value (pfmGeotiff::tr ("input directory"), options->input_dir).toString ();
  options->output_dir = settings.value (pfmGeotiff::tr ("output directory"), options->output_dir).toString ();
  options->area_dir = settings.value (pfmGeotiff::tr ("area directory"), options->area_dir).toString ();

  options->window_width = settings.value (pfmGeotiff::tr ("width"), options->window_width).toInt ();
  options->window_height = settings.value (pfmGeotiff::tr ("height"), options->window_height).toInt ();
  options->window_x = settings.value (pfmGeotiff::tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (pfmGeotiff::tr ("y position"), options->window_y).toInt ();

  settings.endGroup ();
}


void envout (OPTIONS *options)
{
  QSettings settings (pfmGeotiff::tr ("navo.navy.mil"), pfmGeotiff::tr ("pfmGeotiff"));

  settings.beginGroup (pfmGeotiff::tr ("pfmGeotiff"));


  settings.setValue (pfmGeotiff::tr ("settings version"), settings_version);

  settings.setValue (pfmGeotiff::tr ("transparent"), options->transparent);

  settings.setValue (pfmGeotiff::tr ("packbits compression flag"), options->packbits);

  settings.setValue (pfmGeotiff::tr ("32 bit floating point format"), options->grey);

  settings.setValue (pfmGeotiff::tr ("restart"), options->restart);

  settings.setValue (pfmGeotiff::tr ("azimuth"), (NV_FLOAT64) options->azimuth);

  settings.setValue (pfmGeotiff::tr ("elevation"), (NV_FLOAT64) options->elevation);

  settings.setValue (pfmGeotiff::tr ("exaggeration"), (NV_FLOAT64) options->exaggeration);

  settings.setValue (pfmGeotiff::tr ("saturation"), (NV_FLOAT64) options->saturation);

  settings.setValue (pfmGeotiff::tr ("value"), (NV_FLOAT64) options->value);

  settings.setValue (pfmGeotiff::tr ("start_hsv"), (NV_FLOAT64) options->start_hsv);

  settings.setValue (pfmGeotiff::tr ("end_hsv"), (NV_FLOAT64) options->end_hsv);

  settings.setValue (pfmGeotiff::tr ("surface"), options->surface);

  settings.setValue (pfmGeotiff::tr ("units"), options->units);

  settings.setValue (pfmGeotiff::tr ("4800 ft per second flag"), options->dumb);

  settings.setValue (pfmGeotiff::tr ("interpolated contours"), options->intrp);

  settings.setValue (pfmGeotiff::tr ("smoothing level"), options->smoothing_factor);

  settings.setValue (pfmGeotiff::tr ("NAVD88 flag"), options->navd88);

  settings.setValue (pfmGeotiff::tr ("elevation flag"), options->elev);

  settings.setValue (pfmGeotiff::tr ("contour interval"), options->cint);

  settings.setValue (pfmGeotiff::tr ("input directory"), options->input_dir);
  settings.setValue (pfmGeotiff::tr ("output directory"), options->output_dir);
  settings.setValue (pfmGeotiff::tr ("area directory"), options->area_dir);

  settings.setValue (pfmGeotiff::tr ("width"), options->window_width);
  settings.setValue (pfmGeotiff::tr ("height"), options->window_height);
  settings.setValue (pfmGeotiff::tr ("x position"), options->window_x);
  settings.setValue (pfmGeotiff::tr ("y position"), options->window_y);

  settings.endGroup ();
}
