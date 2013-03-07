
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



#include "pfmEditShell.hpp"


//  We need to import a subset of the pfmView settings if we're going to bypass it and shell the editor.  Note that
//  we never save anything from this program.

NV_FLOAT64 settings_version = 7.80;


NV_BOOL envin (OPTIONS *options, MISC *misc)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);

  saved_version = settings.value (pfmEditShell::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults (from set_defaults.cpp) since they may have changed.

  if (settings_version != saved_version) return (NVFalse);


  options->position_form = settings.value (pfmEditShell::tr ("position form"), options->position_form).toInt ();

  options->min_hsv_color = settings.value (pfmEditShell::tr ("minimum hsv color value"), options->min_hsv_color).toInt ();
  options->max_hsv_color = settings.value (pfmEditShell::tr ("maximum hsv color value"), options->max_hsv_color).toInt ();
  options->min_hsv_locked = settings.value (pfmEditShell::tr ("minimum hsv locked flag"), options->min_hsv_locked).toBool ();
  options->max_hsv_locked = settings.value (pfmEditShell::tr ("maximum hsv locked flag"), options->max_hsv_locked).toBool ();
  options->min_hsv_value = (NV_FLOAT32) settings.value (pfmEditShell::tr ("minimum hsv locked value"), (NV_FLOAT64) options->min_hsv_value).toDouble ();
  options->max_hsv_value = (NV_FLOAT32) settings.value (pfmEditShell::tr ("maximum hsv locked value"), (NV_FLOAT64) options->max_hsv_value).toDouble ();


  QString en = settings.value (pfmEditShell::tr ("editor name"), QString (options->edit_name)).toString ();
  strcpy (options->edit_name, en.toAscii ());
  en = settings.value (pfmEditShell::tr ("3D editor name"), QString (options->edit_name_3D)).toString ();
  strcpy (options->edit_name_3D, en.toAscii ());


  options->feature_search_string = settings.value (pfmEditShell::tr ("feature search string"), options->feature_search_string).toString ();

  options->smoothing_factor = settings.value (pfmEditShell::tr ("contour smoothing factor"), options->smoothing_factor).toInt ();

  options->z_factor = (NV_FLOAT32) settings.value (pfmEditShell::tr ("depth scaling factor"), (NV_FLOAT64) options->z_factor).toDouble ();

  options->z_offset = (NV_FLOAT32) settings.value (pfmEditShell::tr ("depth offset value"), (NV_FLOAT64) options->z_offset).toDouble ();

  options->cint = (NV_FLOAT32) settings.value (pfmEditShell::tr ("contour interval"), (NV_FLOAT64) options->cint).toDouble ();
  options->layer_type = settings.value (pfmEditShell::tr ("binned layer type"), options->layer_type).toInt ();


  options->num_levels = settings.value (pfmEditShell::tr ("contour levels"), options->num_levels).toInt ();

  for (NV_INT32 i = 0 ; i < options->num_levels ; i++)
    {
      string.sprintf (pfmEditShell::tr ("contour level %d").toAscii (), i);
      options->contour_levels[i] = (NV_FLOAT32) settings.value (string, (NV_FLOAT64) options->contour_levels[i]).toDouble ();
    }


  settings.endGroup ();


  return (NVTrue);
}
