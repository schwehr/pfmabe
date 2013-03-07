
using namespace std;  // Windoze bullshit - God forbid they should follow a standard


QString bGrpText = 
  Prefs::tr ("Select the format in which you want all geographic positions to be displayed.");

QString backgroundColorText = 
  Prefs::tr ("Click this button to change the color that is used as NULL background.  After "
             "selecting a color the map will be redrawn.");

QString coastColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot the coasts.  After "
             "selecting a color the map will be redrawn.");

QString maskColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot the landmask.  After "
             "selecting a color the map will be redrawn.");

QString colorGrpText = 
  Prefs::tr ("Click a button to change the associated plotting color.  After selecting a color the map will be redrawn.");

QString sunAzText = 
  Prefs::tr ("Set the sun angle.  Sun angle is the direction from which the sun appears to be "
             "illuminating the sun shaded surface.  The default value is 30 degrees which would be "
             "from the upper right of the display.");

QString sunElText = 
  Prefs::tr ("Set the sun elevation.  Sun elevation is the apparent elevation of the sun above the "
             "horizon for the sun shaded surface.  The default value is 30 degrees.  Acceptable "
             "values are from 0.0 (totally dark) to 90.0 (directly overhead).");

QString sunExText = 
  Prefs::tr ("Set the sun shading Z exaggeration.  This is the apparent exaggeration of the surface "
             "in the Z direction.  The default is 1.0 (no exaggeration).");

QString stopLightText = 
  Prefs::tr ("If selected, data that is stored in MINMAX type .trk files will be displayed using stop light colors (red, "
             "yellow, green).  The values at which the colors change are specified in the <b>Green to yellow value</b> "
             "and <b>Yellow to reed value</b> locations.  If this button is not selected then data from MINMAX .trk files "
             "will be displayed in colors ranging from blue to red with blue representing the minimum and red representing "
             "the max.");

QString minStopText = 
  Prefs::tr ("This is the value below which data from MINMAX .trk files will be displayed in green.  This value is meaningless "
             "if the <b>Use stoplight colors</b> check box is not selected.");

QString maxStopText = 
  Prefs::tr ("This is the value above which data from MINMAX .trk files will be displayed in red.  This value is meaningless "
             "if the <b>Use stoplight colors</b> check box is not selected.");

QString highlightColorText = 
  Prefs::tr ("Click this button to change the color that is used to highlight selected lines or areas.  After selecting "
             "a color the map will be redrawn.");

QString restoreDefaultsText = 
  Prefs::tr ("Click this button to restore all of the preferences to the original default values and close "
	     "the dialog.  This includes <b><i>all</i></b> of the system preferences, not just those in the "
             "preferences dialog.");

QString closePrefsText = 
  Prefs::tr ("Click this button to discard all preference changes and close the dialog.  The main map and "
             "will not be redrawn.");

QString applyPrefsText = 
  Prefs::tr ("Click this button to apply all changes and close the dialog.  The main map map will "
             "be redrawn.");
