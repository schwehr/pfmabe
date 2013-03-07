
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



QString fSizeText = 
  Prefs::tr ("Set the tracking list feature marker size.  The default is 0.001.");

QString bGrpText = 
  Prefs::tr ("Select the format in which you want all geographic positions to be displayed.");

QString backgroundColorText = 
  Prefs::tr ("Click this button to change the color that is used as background.  After "
             "selecting a color the map will be redrawn.");

QString trackerColorText = 
  Prefs::tr ("Click this button to change the color that is used for the tracking cursor.  "
             "After selecting a color the map will be redrawn.");

QString featureColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot tracking list points.  "
             "After selecting a color the map will be redrawn.");

QString featureInfoColorText = 
  Prefs::tr ("Click this button to change the color that is used to write the tracking list information "
             "next to the tracking list markers.  After selecting a color the map will be redrawn.");

QString scaleColorText = 
  Prefs::tr ("Click this button to change the color that is used for the scale.  After "
             "selecting a color the map will be redrawn.");

QString scaleText = 
  Prefs::tr ("Check this box to draw the scale on the screen.");

QString exagText = 
  Prefs::tr ("Set the Z exaggeration value.  The default is 6.0.");

QString restoreDefaultsText = 
  Prefs::tr ("Click this button to restore all of the preferences to the original default values.");

QString closePrefsText = 
  Prefs::tr ("Click this button to discard all preference changes and close the dialog.");

QString applyPrefsText = 
  Prefs::tr ("Click this button to apply all changes and close the dialog.");
