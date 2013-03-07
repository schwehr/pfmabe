QString bGrpText = 
  Prefs::tr ("Select the format in which you want all geographic positions to be displayed.");

QString contourIntText = 
  Prefs::tr ("Set the contour interval to be used by pfmView and also geoSwath3D.  Setting this "
             "to 0.0 will cause the user defined contour levels to be used.  No contours will "
             "be plotted unless the plot contour button <img source=\":/icons/contour.xpm\"> has been "
             "toggled on.");

QString contourSmText = 
  Prefs::tr ("Set the contour smoothing factor by pfmView and also geoSwath3D.  This value goes "
             "from 0 (no smoothing) to 10 (max smoothing).");

QString ZFactorText = 
  Prefs::tr ("Set the scaling factor for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and geoSwath3D.  The actual values aren't changed just the displayed values.  "
             "This is useful for comparing depths to GeoTIFFs of scanned charts from NOAA (feet) "
             "or the US Navy (fathoms).");

QString offsetText = 
  Prefs::tr ("Set the offset value for contours <b><i>and</i></b> all displayed depth values "
             "in pfmView and geoSwath3D.  The actual values aren't changed just the displayed values.  "
             "This is useful for applying estimated ellipsoid offsets prior to the final datum "
             "shift being applied to the data.<br><br>"
             "<b>IMPORTANT NOTE: The offset value is <i>added</i> to the Z value and is applied <i>after</i> "
             "scaling so the units are in scaled units.</b>");

QString egmText = 
  Prefs::tr ("Press this button to retrieve the EGM08 model value of the ellipsoid to geoid (approximate mean sea "
             "level) datum offset for the point nearest the center of the minimum bounding rectangle of the displayed "
             "area.");

QString WidthText = 
  Prefs::tr ("Set the contour line width/thickness in pixels.  Index contours (if set) will be twice this size.");

QString pointSizeText = 
  Prefs::tr ("Set the size of each displayed point in pixels.");

QString pointLimitText = 
  Prefs::tr ("Set the number of points that you wish to display per section.  The minimum is 10,000 and the maximum is 500,000.  "
             "The default is 100,000.");

QString scaleText = 
  Prefs::tr ("Check this box to draw the scale on the screen.");

QString minZText = 
  Prefs::tr ("Set the minimum Z extents.  This will be in the units that the data is in (almost always meters).  The "
	     "purpose is to keep data with little Z variability from filling the screen with <b><i>fuzz</i></b> by limiting "
	     "the self scaling of the Z axis to this minimum value.<br><br>"
	     "<b>Important note: If the combo box to the left of this is set to anything other than <i>Set Manually -></i> "
	     "then the max of this value and the computed IHO TVU will be used as the minimum Z window size.</b>");

QString ihoOrderText = 
  Prefs::tr ("Set the minimum Z extents manually or based on IHO order.  If this is set to anything other than "
	     "<b>Set Manually -></b> the minimum Z extents will be set based on the S44 IHO order TVU computations.  "
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

QString rotText = 
  Prefs::tr ("Set the view rotation increment in degrees.  This value is the amount the alternate views will rotate "
             "when the left or right arrow keys are pressed.");

QString zoomPercentText = 
  Prefs::tr ("Set the percentage value for zooming the data view in or out.");

QString exagText = 
  Prefs::tr ("Set the Z exaggeration value (1.0 - 100.0).  This can be modified more easily by pressing <b>Ctrl-Page UP</b> "
             "or <b>Ctrl-Page Down</b> when viewing the main window.");


QString prefsUndoText = 
  Prefs::tr ("Set the maximum number of undo levels.  This is the number of edit operations that can be <b><i>undone</i></b>. "
             "The value ranges from a reasonable 100 to a ridiculous 1,000,000.  The higher this value is set, the more memory "
             "you use to save the undo information.");


QString killText = 
  Prefs::tr ("Check this box if you would like ancillary programs to exit when you close the geoSwath3D window and then "
             "respawn when you start it again.  This only works for the following LIDAR programs:<br><br>"
             "<ul>"
             "<li>waveMonitor</li>"
             "<li>chartsPic</li>"
             "<li>lidarMonitor</li>"
             "<li>waveWaterfall (APD)</li>"
             "<li>waveWaterfall (PMT)</li>"
             "</ul><br><br>"
             "<b>IMPORTANT NOTE: The programs should respawn in the same locations, however, if there is overlap, the "
             "order of the windows may not be preserved.  Also, if you are running Compiz (desktop effects) on Linux "
             "using dual monitors, the program locations may not restore correctly.</b>");


QString filterSTDText = 
  Prefs::tr ("Set the standard deviation value for the filter.  This value can be anywhere from 0.3 to 4.0.  "
             "A value of 2.0 or below is an extreme filter and should be used only for deep water, flat area smoothing.  "
             "A value of 2.4 standard deviations should filter about 5 percent of the data while a value of 4.0 should "
             "only filter out about 0.5 percent.");

QString dFilterText = 
  Prefs::tr ("Set this check box to indicate that data will only be filtered in the downward direction when running the "
             "filter.  Downward in this case implies a larger positive value.  The assumption is that the data we are "
             "working with consists of depths not elevations.");

QString contourLevelsText = 
  Prefs::tr ("Set individual contour levels.  Pressing this button will set the standard contour "
             "interval to 0.0 and bring up a table dialog to allow the user to insert contour "
             "levels (one per line, ascending).  The default levels are IHO standard contour "
             "levels.");

QString contoursTableText = 
  Prefs::tr ("Use this table to set individual contour levels.  Levels should be set in ascending "
             "order.");


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
             "<li>Ctrl+Alt+q</li>");

QString closeHotKeyText = 
  Prefs::tr ("Click this button to close the hot key dialog.");


QString closeContoursText = 
  Prefs::tr ("This button closes the contour level dialog.");

QString contourColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot contours.  "
             "After selecting a color the map will be redrawn.");

QString trackerColorText = 
  Prefs::tr ("Click this button to change the color that is used to plot the <i>other ABE window</i> tracker.  "
             "After selecting a color the map will be redrawn.");

QString backgroundColorText = 
  Prefs::tr ("Click this button to change the color that is used as background.  After "
             "selecting a color the map will be redrawn.");

QString scaleColorText = 
  Prefs::tr ("Click this button to change the color that is used for the scale.  After "
             "selecting a color the map will be redrawn.");

QString waveColorText = 
  Prefs::tr ("Click any of the numbered cursor buttons to change the color of the multiple cursors that "
             "are associated with the waveMonitor and waveWaterfall programs.  The <b>0</b> color will be the "
             "color of the single cursor/marker.  The waveform colors in waveMonitor and waveWaterfall will "
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
	     "action keys may be changed but they must be unique.  For information on the programs look at "
	     "<b>Hot Keys</b> in the main program's <b>Help</b> menu.");

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
             "shared memory to cause some form of action to be taken in the ancillary program and geoSwath.  Programs "
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

