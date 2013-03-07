
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



#include "abe.hpp"


//  These functions store and retrieve the program settings (environment) from somewhere ;-)  On Linux they're in your home
//  directory in a directory called .config/"misc->qsettings_org"/"misc->qsettings_app".rc (at this time - 
//  .config/navo.navy.mil/abe.rc).  On Windoze they're stored in the registry (may a higher being have mercy 
//  on you if you want to change them).  On MacOSX I have no idea where they're stored but you can check the Qt docs ;-)
//  If you make a change to the way a variable is used and you want to force the defaults to be restaored just change the
//  settings_version to a newer number (I've been using the program version number from version.hpp - which you should be
//  updating EVERY time you make a change to the program!).  You don't need to change the settings_version though unless
//  you want to force the program to go back to the defaults (which can annoy your users).  So, the settings_version won't
//  usually match the program version.

NV_FLOAT64 settings_version = 1.11;


void envin (OPTIONS *options, MISC *misc)
{
  NV_FLOAT64 saved_version = 0.0;
  QString string;


  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);

  saved_version = settings.value (abe::tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults (from set_defaults.cpp) since they may have changed.

  if (settings_version != saved_version) return;

  options->main_button_icon_size = settings.value (abe::tr ("icon size"), options->main_button_icon_size).toInt ();

  options->orientation = settings.value (abe::tr ("orientation"), options->orientation).toInt ();

  options->window_x = settings.value (abe::tr ("x position"), options->window_x).toInt ();
  options->window_y = settings.value (abe::tr ("y position"), options->window_y).toInt ();


  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++) options->buttonAccel[i] = settings.value (misc->progName[i] + abe::tr ("accelerator"), options->buttonAccel[i]).toString ();


  settings.endGroup ();
}


void envout (OPTIONS *options, MISC *misc)
{
  QSettings settings (misc->qsettings_org, misc->qsettings_app);
  settings.beginGroup (misc->qsettings_app);


  settings.setValue (abe::tr ("settings version"), settings_version);

  settings.setValue (abe::tr ("main button icon size"), options->main_button_icon_size);

  settings.setValue (abe::tr ("orientation"), options->orientation);

  settings.setValue (abe::tr ("x position"), options->window_x);
  settings.setValue (abe::tr ("y position"), options->window_y);


  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++) settings.setValue (misc->progName[i] + abe::tr ("accelerator"), options->buttonAccel[i]);


  settings.endGroup ();
}
