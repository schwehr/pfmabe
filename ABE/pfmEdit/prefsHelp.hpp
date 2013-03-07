
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



QString bGrpText = 
  Prefs::tr ("Select the format in which you want all geographic positions to be displayed.");

QString contourIntText = 
  Prefs::tr ("Set the contour interval to be used by pfmView and also pfmEdit.  Setting this "
             "to 0.0 will cause the user defined contour levels to be used.  No contours will "
             "be plotted unless the plot contour button <img source=\":/icons/contour.xpm\"> has been "
             "toggled on.");

QString contourSmText = 
  Prefs::tr ("Set the contour smoothing factor by pfmView and also pfmEdit.  This value goes "
             "from 0 (no smoothing) to 10 (max smoothing).");

QString ZFactorText = 
  Prefs::tr ("Set the scaling factor for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and pfmEdit.  The actual values aren't changed just the displayed values.  "
             "This is useful for comparing depths to GeoTIFFs of scanned charts from NOAA (feet) "
             "or the US Navy (fathoms).");

QString offsetText = 
  Prefs::tr ("Set the offset value for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and pfmEdit.  The actual values aren't changed just the displayed values.  "
             "This is useful for applying estimated ellipsoid offsets prior to the final datum "
             "shift being applied to the data.<br><br>"
             "<b>IMPORTANT NOTE: The offset value is <i>added</i> to the Z value and is applied <i>after</i> "
             "scaling so the units are in scaled units.</b>");

QString egmText = 
  Prefs::tr ("Press this button to retrieve the EGM08 model value of the ellipsoid to geoid (approximate mean sea "
             "level) datum offset for the point nearest the center of the minimum bounding rectangle of the displayed "
             "area.  Since the posts in the EGM08 model are 15 minutes (approximately 15 nautical miles) apart, "
             "there will probably only be one post in the vicinity.");

QString WidthText = 
  Prefs::tr ("Set the contour line width/thickness in pixels.  Index contours (if set) will be twice this size.");

QString pointSizeText = 
  Prefs::tr ("Set the size of each displayed point in pixels.");

QString minZText = 
  Prefs::tr ("Set the minimum Z window size.  This will be in the units that the data is in (almost always meters).  The "
	     "purpose is to keep data with little Z variability from filling the screen with <b><i>fuzz</i></b> by limiting "
	     "the self scaling of the Z axis to this minimum value.<br><br>"
	     "<b>Important note: If the combo box to the left of this is set to anything other than <i>Set Manually -></i> "
	     "then the max of this value and the computed IHO TVU will be used as the minimum Z window size.</b>");

QString ihoOrderText = 
  Prefs::tr ("Set the minimum Z window size manually or based on IHO order.  If this is set to anything other than "
	     "<b>Set Manually -></b> the minimum Z window size will be set based on the S44 IHO order TVU computations.  "
	     "These are:<br><br>"
	     "<ul>"
	     "<li>Special Order   -    0.25m + 0.0075 * depth</li>"
	     "<li>Order 1a/1b     -    0.50m + 0.0130 * depth</li>"
	     "<li>Order 2         -    1.00m + 0.0230 * depth</li>"
	     "</ul>"
	     "<br><br>The depth value used in the above equations will be the minimum depth displayed.<br><br>"
	     "<b>Important note: The maximum of the computed TVU and the manually set minimum Z window size will be used "
	     "so that, in very shallow water, we don't shut the window down too far.  If you always want to use the computed TVU "
	     "set the minimum value to 0.1 or some other very small number.</b>");

QString iconSizeText = 
  Prefs::tr ("Set the size of the main window buttons.  The options are 16, 20, 24, 28, or 32 pixels.<br><br>"
             "<b>IMPORTANT NOTE: The icons size will change when you click the <i>Apply</i> button but "
             "the locations will not be entirely correct until you have exited and restarted the program.</b>");

QString sliceText = 
  Prefs::tr ("Set the slice percentage for views other than plan view.  To understand how slicing works try "
             "to visualize the surface as it would be in plan view but with the bottom (leading) edge defined "
             "by the view (either east, south, or any selected view).  The slice will be this percentage "
             "(default is 0.05 or 1/20th) of the displayed data at the bottom (leading) edge of the data.  "
             "When you press <b>Page Up</b> the slice will move into the data one slice.  When displaying a "
             "slice any edits will only effect visible data.  This is extremely handy in dealing with very "
             "bumpy surfaces to allow you do clean up hidden flyers.");


QString rotText = 
  Prefs::tr ("Set the view rotation increment in degrees.  This value is the amount the alternate views will rotate "
             "when the left or right arrow keys are pressed.");


QString overlapText = 
  Prefs::tr ("Set the window overlap percentage that will be used when you use the Ctrl-arrow keys to move the "
	     "edit area.");


QString prefsUndoText = 
  Prefs::tr ("Set the maximum number of undo levels.  This is the number of edit operations that can be <b><i>undone</i></b>. "
             "The value ranges from a reasonable 100 to a ridiculous 1,000,000.  The higher this value is set, the more memory "
             "you use to save the undo information.");


QString killText = 
  Prefs::tr ("Check this box if you would like ancillary programs to exit when you close the pfmEdit window and then "
             "respawn when you start it again.  This only works for the following LIDAR programs:<br><br>"
             "<ul>"
             "<li>waveformMonitor</li>"
             "<li>chartsPic</li>"
             "<li>waveWaterfall (APD)</li>"
             "<li>waveWaterfall (PMT)</li>"
             "<li>CZMILwaveMonitor</li>"
             "</ul><br><br>"
             "<b>IMPORTANT NOTE: The programs should respawn in the same locations, however, if there is overlap, the "
             "order of the windows may not be preserved.  Also, if you are running Compiz (desktop effects) on Linux "
             "using dual monitors, the program locations may not restore correctly.</b>");


QString filterSTDText = 
  Prefs::tr ("Set the standard deviation value for the filter.  This value can be anywhere from 0.3 to 4.0.  "
             "A value of 2.0 or below is an extreme filter and should be used only for deep water, flat area smoothing.  "
             "A value of 2.4 standard deviations should filter about 5 percent of the data while a value of 4.0 should "
             "only filter out about .5 percent.");

QString dFilterText = 
  Prefs::tr ("Set this check box to indicate that data will only be filtered in the downward direction when running the "
             "filter.  Downward in this case implies a larger positive value.  The assumption is that the data we are "
             "working with consists of depths not elevations.");

QString colorScaleText = 
  Prefs::tr ("Set this check box to display the color scale on the right side of the main window.");

QString featureRadiusText = 
  Prefs::tr ("Set the distance around a feature within which filtering will not be allowed.  Setting this to 0 turns "
	     "off <b>ALL</b> feature protection during filtering.  This includes the feature polygon exclusion.");

QString contourLevelsText = 
  Prefs::tr ("Set individual contour levels.  Pressing this button will set the standard contour "
             "interval to 0.0 and bring up a table dialog to allow the user to insert contour "
             "levels (one per line, ascending).  The default levels are IHO standard contour "
             "levels.");

QString contoursTableText = 
  Prefs::tr ("Use this table to set individual contour levels.  Levels should be set in ascending "
             "order.");


QString prefsGeotiffText = 
  Prefs::tr ("Click this button to pop up a dialog with tools to allow you to change the amount of "
             "transparency used for a GeoTIFF overlay.");

QString transparencyText = 
  Prefs::tr ("This dialog is used to set the amount of transparency used in the GeoTIFF overlay.  Move the "
             "slider to change the value.  When the slider is changed the picture will be redrawn.  Setting this to "
             "<b>Invisible</b> disables drwaing of the GeoTIFF layer.");

QString hotKeysText = 
  Prefs::tr ("Click this button to change the accelerator key sequences associated with some of "
             "the function buttons.");

QString dataTypeTableText = 
  Prefs::tr ("Select the data types that the command will work on.  If you want the command to work "
             "regardless of the data type of the current point (for instance, in the case of the "
             "mosaic file) choose Undefined.");

QString closeDataTypesText = 
  Prefs::tr ("Click this button to close the data types dialog.");

QString hotKeyTableText = 
  Prefs::tr ("Accelerator key sequences (hot keys) for some of the buttons in the main window may be changed "
             "in this dialog.  Up to four comma separated key values with modifiers may be used.  The format is "
             "illustrated by the following examples:<br><br>"
             "<ul>"
             "<li>q</li>"
             "<li>Shift+q</li>"
             "<li>Ctrl+q</li>"
             "<li>Meta+q</li>"
             "<li>Ctrl+Alt+q</li>"
             "</ul>");

QString closeHotKeyText = 
  Prefs::tr ("Click this button to close the hot key dialog.");


QString closeContoursText = 
  Prefs::tr ("This button closes the contour level dialog.");

QString colorText = 
  Prefs::tr ("Set color options in this tab.");

QString conText = 
  Prefs::tr ("Set contour and depth display options in this tab");

QString kboxText = 
  Prefs::tr ("Set the hot keys in this tab.");

QString filtText = 
  Prefs::tr ("Set the statistical filter options in this tab");

QString miscText = 
  Prefs::tr ("This tab contains settings for a number of miscellaneous options including the number of undo levels, "
             "the feature size, and the rotation increment.");

QString contourColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot contours.  "
             "After selecting a color the map will be redrawn.");

QString trackerColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot the <i>other ABE window</i> tracker.  "
             "After selecting a color the map will be redrawn.");

QString backgroundColorText = 
  Prefs::tr ("Click this button to change the color that is used as NULL background.  After "
             "selecting a color the map will be redrawn.");

QString refColorText = 
  Prefs::tr ("Click this button to change the color that is used to represent reference or NULL data.  After "
             "selecting a color the map will be redrawn.");

QString featureColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot features.  "
             "After selecting a color the map will be redrawn.");

QString featureInfoColorText = 
  Prefs::tr ("Click this button to change the color that is used to write the feature information "
             "next to the featuresplot features.  After selecting a color the map will be redrawn.");

QString featurePolyColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot the polygonal areas that "
             "are associated with specific features.  Note that if you set the alpha channel to "
             "less than 255 the area will be plotted as a filled polygon with the semi-transparency "
             "that is set for the alpha channel (0 = transparent, 255 = opaque).  After selecting a "
             "color the map will be redrawn.");

QString highFeatureColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot highlighted features.  The features that will be highlighted "
             "are determined by the text search, text search invert, and text search type that are set in pfmView.  "
             "After selecting a color the map will be redrawn.");

QString verFeatureColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot verified features (confidence = 5).  "
             "After selecting a color the map will be redrawn.");

QString waveColorText = 
  Prefs::tr ("Click any of the numbered cursor buttons to change the color of the multiple cursors that "
             "are associated with the CZMILwaveMonitor and waveWaterfall programs.  The <b>0</b> color will be the "
             "color of the single cursor/marker.  The waveform colors in CZMILwaveMonitor and waveWaterfall will "
             "use these colors as well.");

QString restoreDefaultsText = 
  Prefs::tr ("Click this button to restore all of the preferences to the original default values.  "
             "This includes <b><i>all</i></b> of the system preferences, not just those in the "
             "preferences dialog.");

QString closePrefsText = 
  Prefs::tr ("Click this button to discard all preference changes and close the dialog.  The main map and "
             "coverage map will not be redrawn.");

QString applyPrefsText = 
  Prefs::tr ("Click this button to apply all changes and close the dialog.  The main map and coverage map will "
             "be redrawn.");

QString ancillaryProgramsText = 
  Prefs::tr ("Click this button to change ancillary programs, hot keys, action keys, and data types associated "
             "with each program.");

QString programText = 
  Prefs::tr ("This is the external program that will be run when you press the associated hot key.  The hot keys and "
	     "action keys may be changed but they must be unique.<br><br>");

QString hotkeyText = 
  Prefs::tr ("The hot key is the single key that will initiate the associated program.  These must be unique.  Keys with modifiers may be used.  The format is "
             "illustrated by the following examples:<br><br>"
             "<ul>"
             "<li>q</li>"
             "<li>Shift+q</li>"
             "<li>Ctrl+q</li>"
             "<li>Meta+q</li>"
             "<li>Ctrl+Alt+q</li>"
             "</ul>");

QString actionkeyText = 
  Prefs::tr ("The action key is a single key or a group of comma (,) separated single keys that will be passed to the "
             "associated program as command line arguments of the form --actionkeyNN=key.  Action keys are used via "
             "shared memory to cause some form of action to be taken in the ancillary program and pfmEdit.  Programs "
             "that use action keys are specially built to access PFM shared memory.  The following are examples of action keys:<br><br>"
             "<ul>"
             "<li>q</li>"
             "<li>Shift+q</li>"
             "<li>Ctrl+q</li>"
             "<li>Meta+q</li>"
             "<li>Ctrl+Alt+q</li>"
             "<li>q,s,1,2,3,4</li>"
             "<li>Ctrl+q,Alt+s</li>"
             "</ul><br><br>"
             "<b>IMPORTANT NOTE: If you change the action keys for an ancillary program that is "
             "already running you must kill the ancillary program and restart it for the new action keys "
             "to take effect.</b>");

QString closeAncillaryProgramText = 
  Prefs::tr ("Click this button to close the ancillary programs dialog.");

