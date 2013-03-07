/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


QString openText = 
  geoSwath3D::tr ("<img source=\":/icons/fileopen.xpm\"> Click this button to open an input file.");

QString exitText = 
  geoSwath3D::tr ("<img source=\":/icons/quit.xpm\"> Click this button to exit from the editor.");

QString resetText = 
  geoSwath3D::tr ("<img source=\":/icons/reset_view.xpm\"> Click this button to reset to the original scale.");


QString displayInvalidText = 
  geoSwath3D::tr ("<img source=\":/icons/displayinvalid.xpm\"> Click this button to display previously invalidated "
                  "data points.");

QString highlightPolyText = 
  geoSwath3D::tr ("<img source=\":/icons/highlight_polygon.xpm\"> Click this button to allow you to select a polygonal area "
                  "inside of which you wish to highlight data points.  When selected the cursor will become the highlight polygon cursor "
                  "<img source=\":/icons/highlight_polygon_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                  "highlight the data in the polygon simply left click again.  If, at any time during the operation, "
                  "you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.<br><br>"
                  "<b>IMPORTANT NOTE: If you would like to delete highlighted data points just press <i>Return</i> or whatever "
                  "hot key that has been set for <i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog.</b>");
QString clearPolyText = 
  geoSwath3D::tr ("<img source=\":/icons/clear_polygon.xpm\"> Click this button to allow you to select a polygonal area "
                  "inside of which you wish to un-highlight data points.  When selected the cursor will become the clear polygon cursor "
                  "<img source=\":/icons/clear_polygon_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                  "un-highlight the data in the polygon simply left click again.  If, at any time during the operation, "
                  "you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.<br><br>"
                  "<b>IMPORTANT NOTE: If you would like to delete highlighted data points just press <i>Return</i> or whatever "
                  "hot key that has been set for <i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog.</b>");
QString clearHighlightText = 
  geoSwath3D::tr ("<img source=\":/icons/clear_highlight.xpm\"> Click this button to clear all highlighted points from "
                  "the display.");

QString prefsText = 
  geoSwath3D::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  This "
                  "includes colors, minimum Z window size, position display format, and all hot keys.");


QString stopText = 
  geoSwath3D::tr ("<img source=\":/icons/stop.xpm\"> Click this button to cancel drawing of the data.  A much easier "
                  "way to do this though is to click any mouse button in the display area or press any "
                  "key on the keyboard.  The stop button is really just there so that the interface looks "
                  "similar to the viewer and also to provide a place for help on how to stop the drawing.");


QString deletePointText = 
  geoSwath3D::tr ("<img source=\":/icons/delete_point.xpm\"> Click this button to select delete subrecord/record "
                  "mode.  In this mode you can place the cursor on a subrecord (beam, shot, point) and "
                  "delete the subrecord by pressing the <Shift> key and then left clicking (or via the right click menu) "
                  "or delete the record (ping, shot) by pressing the middle mouse button.  Information about the current "
                  "point will be displayed in the status bars at the bottom of the window.<br><br>"
                  "You can also highlight a point by using the right-click popup menu.  Note that the <b>Highlight point</b> option may be "
                  "disabled if you have filter kill points up or are flagging data using any of the flag buttons "
                  "<img source=\":/icons/user_flag.xpm\"><br><br>"
                  "Hot keys and action keys are available when in this mode.  Please check the <b>Preferences<b> dialog "
                  "<img source=\":/icons/prefs.xpm\">, specifically the <b>Ancillary Programs</b> dialog, to see what action "
                  "keys are available for each ancillary program.  To find out what the keys do in the ancillary program "
                  "you must use the help in the ancillary program." 
                  "<p><b><i>IMPORTANT NOTE: Only in this mode can you use hot keys to launch ancillary "
                  "programs.</i></b></p>");
QString deleteRectText = 
  geoSwath3D::tr ("<img source=\":/icons/delete_rect.xpm\"> Click this button to allow you to select a rectangular area "
                  "to invalidate.  When selected the cursor will become the invalidate rectangle cursor "
                  "<img source=\":/icons/delete_rect_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                  "rectangle and invalidate the data left click again.  If, at any time during the operation, you wish to "
                  "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                  "select one of the menu options.");
QString deletePolyText = 
  geoSwath3D::tr ("<img source=\":/icons/delete_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "to invalidate.  When selected the cursor will become the invalidate polygon cursor "
                  "<img source=\":/icons/delete_poly_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                  "invalidate the data in the polygon simply left click again.  If, at any time during the operation, "
                  "you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.");
QString keepPolyText = 
  geoSwath3D::tr ("<img source=\":/icons/keep_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "and invalidate all data <b>outside</b> of the polygon.  When selected the cursor will "
                  "become the invalidate outside polygon cursor <img source=\":/icons/keep_poly_cursor.xpm\">.  Left "
                  "clicking on a location will cause that point to be the polygon start point.  To close the polygon and "
                  "invalidate the data outside of the polygon simply left click again.  If, at any time during the "
                  "operation, you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.");
QString restoreRectText = 
  geoSwath3D::tr ("<img source=\":/icons/restore_rect.xpm\"> Click this button to allow you to select a rectangular area "
                  "in which to restore invalidated data.  When selected the cursor will become the restore rectangle "
                  "cursor <img source=\":/icons/restore_rect_cursor.xpm\">.  Left clicking on a location will cause that "
                  "point to be the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize "
                  "the rectangle and restore the invalid data simply left click again.  If, at any time during the "
                  "operation, you wish to discard the rectangle and abort the operation simply click the middle mouse button "
                  "or right click and select one of the menu options.");
QString restorePolyText = 
  geoSwath3D::tr ("<img source=\":/icons/restore_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "in which to restore invalidated data.  When selected the cursor will become the restore polygon cursor "
                  "<img source=\":/icons/restore_poly_cursor.xpm\">.  Left clicking on a location will cause that point to "
                  "be the polygon start point.  To close the polygon and restore invalid data in the polygon simply left "
                  "click again.  If, at any time during the operation, you wish to discard the polygon and abort the "
                  "operation simply click the middle mouse button or right click and select a menu option.");

QString hotkeyPolyText = 
  geoSwath3D::tr ("<img source=\":/icons/hotkey_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "in which to run an ancillary program that is started with a hotkey.  When selected the cursor will "
                  "become the hotkey polygon cursor <img source=\":/icons/hotkey_poly_cursor.xpm\">.  Left clicking on a "
                  "location will cause that point to be the polygon start point.  Moving the cursor will draw a continuous "
                  "line.  To close the polygon in preparation for pressing a hotkey simply left click again.  After defining "
                  "the polygon just press the hotkey that is associated with the ancillary program that you wish to run.  "
                  "To see the available ancillary programs look under preferences <img source=\":/icons/prefs.xpm\">.  If, "
                  "at any time during the operation, you wish to discard the polygon and abort the operation simply click "
                  "the middle mouse button or right click and select a menu option.");

//  If you modify mapText, change the text in hotkeyHelp.cpp as well.

QString mapText = 
  geoSwath3D::tr ("The geoSwath3D program is used to display and edit data points in data files.  The currently "
                  "available file types are GSF, HOF, TOF, WLF, and HAWKEYE (.bin).  The various editing and viewing options are "
                  "initiated from the tool bar buttons.  Help is available for each button by clicking on the "
                  "What's This button <img source=\":/icons/contextHelp.xpm\"> then clicking on the item of interest.");

QString fileBarText = 
  geoSwath3D::tr ("This scroll bar can be used to move forward and backward through the file.  It is much easier "
                  "to use the Page Down and Page Up keys instead of trying to click the scroll bar arrows.  Page Down "
                  "moves forward in the file and Page Up moves back (towards the top of the file).");

QString trackMapText = 
  geoSwath3D::tr ("This is the 2D tracking map.  It will show you where your cursor is in relation to the 3D cursor position.");

QString exagBarText = 
  geoSwath3D::tr ("This scroll bar controls/monitors the vertical exaggeration of the data.  Pressing the up and down "
                  "arrow keys will change the exaggeration by 1.0.  Clicking the mouse while the cursor is in the trough "
                  "will change the value by 5.0.  Dragging the slider will display the value in the <b>Exag:</b> field in "
                  "the status area on the left but the exaggeration will not change until the slider is released.  The "
                  "maximum value is 100.0 and the minimum value is 1.0.  If the exaggeration has been set to less than 1.0 "
                  "due to large vertical data range the scroll bar will be inactive.  The exaggeration can also be changed "
                  "by pressing the <b>Ctrl</b> key and then simultaneously pressing the <b>PageUp</b> or <b>PageDown</b> button.");

QString sliceBarText = 
  geoSwath3D::tr ("This scroll bar controls/monitors the slice as it moves through the data.  Pressing the up and down "
                  "arrow keys will move the slice through the data.  The slice size will be the percent of the viewable data "
                  "that is set with the slice size scroll bar.  When you press the up arrow the view will move "
                  "into the data one slice.  When displaying a slice, any edits will only effect data that is "
                  "in full color (non-translucent).  Slicing is extremely handy in dealing with very bumpy surfaces "
                  "to allow you to clean up hidden flyers.  You can turn off slicing by right clicking and selecting "
                  "<b>Turn off slicing</b>, by double clicking anywhere in the window, or by pressing the <b>Esc</b> key.");

QString sizeText = 
  geoSwath3D::tr ("Set the slice size using this scroll bar.  To understand how slicing works try "
                  "to visualize the surface as it would be in plan view but with the bottom (leading) edge defined "
                  "by the angle of view.  The slice size will be this percentage (default is 5 percent or 1/20th) of "
                  "the displayed data.  When you press one of the up or down arrow keys or click one of the slice bar "
                  "arrow buttons the slice will move by this amount.  The range for this scroll bar is 1 to 50.");


QString transText = 
  geoSwath3D::tr ("Set the transparency value to be used for data that is not in the current slice.  If "
                  "the slider is moved to the bottom, data outside the slice will be invisible.  If it is moved to the top "
                  "the data will be almost completely opaque.  The range for this scroll bar is 0 (transparent) to 64 "
                  "(semi-transparent).");

QString geoSwath3DAboutText = 
  geoSwath3D::tr ("<center>geoSwath3D<br><br>"
                  "Author : Jan C. Depner (jan.depner@navy.mil)<br>"
                  "Date : 31 January 2005<br><br></center>"
                  "geoSwath3D is a 3D along track editor for GSF, CHARTS HOF, CHARTS TOF, WLF, and AHAB BIN "
                  "files.");

QString acknowledgementsText = 
  geoSwath3D::tr ("<center><br>geoSwath3D was built using some, if not all, of the following Open Source libraries:"
                  "<br><br></center>"
                  "<ul>"
                  "<li><a href=\"http://www.qtsoftware.com/qt\">Qt</a> - A cross-platform application and UI framework</li>"
                  "<li><a href=\"http://www.gdal.org\">GDAL</a> - Geospatial Data Abstraction Library</li>"
                  "<li><a href=\"http://trac.osgeo.org/proj\">PROJ.4</a> - Cartographic Projections Library</li>"
                  "<li><a href=\"http://xerces.apache.org/xerces-c\">XERCES</a> - Validating XML parser libary</li>"
                  "<li><a href=\"http://shapelib.maptools.org\">SHAPELIB</a> - Shapefile C Library</li>"
                  "<li><a href=\"http://www.zlib.net\">ZLIB</a> - Compression Library</li>"
                  "<li><a href=\"http://www.alglib.net\">LEASTSQUARES</a> - Least squares math library</li>"
                  "<li><a href=\"http://liblas.org\">LIBLAS</a> - LAS I/O library</li>"
                  "<li><a href=\"http://www.hdfgroup.org/HDF5\">HDF5</a> - Heirarchical Data Format library</li>"
                  "</ul><br>"
                  "<ul>"
                  "<li>Qt and SHAPELIB are licensed under the <a href=\"http://www.gnu.org/copyleft/lesser.html\">GNU LGPL</a></li>"
                  "<li>GDAL is licensed under an X/MIT style open source license</li>"
                  "<li>PROJ.4 is licensed under an MIT open source license</li>"
                  "<li>XERCES is licensed under the Apache Software License</li>"
                  "<li>ZLIB, LEASTSQUARES, LIBLAS, and HDF5 are licensed under their own open source license</li>"
                  "</ul><br><br>"
                  "Many thanks to the authors of these and all of their supporting libraries.  For more information on "
                  "each library please visit their websites using the links above.<br>"
                  "<center>Jan C. Depner<br><br></center>");

QString miscLabelText = 
  geoSwath3D::tr ("This area displays the currently active mode or information about the operation in progress.");

QString statusBarText = 
  geoSwath3D::tr ("The status bar is used to display the progress of the current operation.");

QString progStatusText = 
  geoSwath3D::tr ("The status bar is used to display the progress of the current operation.");

QString attrText = 
  geoSwath3D::tr ("Click this button to access a pulldown menu allowing you to select coloring options.");

QString flagText = 
  geoSwath3D::tr ("Click this button to access a pulldown menu allowing you to select data flagging options.  The "
                  "flagging options are:<br><br>"
                  "<ul>"
                  "<li><img source=\":/icons/suspect.xpm\"> - mark suspect data</li>"
                  "<li><img source=\":/icons/invalid.xpm\"> - mark invalid data</li>"
                  "</ul>");

QString lidarMonitorText = 
  geoSwath3D::tr ("<img source=\":/icons/lidar_monitor.png\"> Click this button to run (or kill) the lidarMonitor program.  The "
                  "lidarMonitor program will display HOF, TOF, WLF, or HAWKEYE record data for the current point nearest the "
                  "cursor in a text format.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString rmsMonitorText = 
  geoSwath3D::tr ("<img source=\":/icons/rms_monitor.png\"> Click this button to run (or kill) the rmsMonitor program.  The "
                  "rmsMonitor program will display navigation RMS data in a text format for the current HOF of TOF point nearest the "
                  "cursor.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString chartsPicText = 
  geoSwath3D::tr ("<img source=\":/icons/charts_pic.png\"> Click this button to run (or kill) the LIDAR chartsPic program.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveformMonitorText = 
  geoSwath3D::tr ("<img source=\":/icons/waveform_monitor.xpm\"> Click this button to run (or kill) the LIDAR waveformMonitor program.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveMonitorText = 
  geoSwath3D::tr ("<img source=\":/icons/wave_monitor.xpm\"> Click this button to run (or kill) the LIDAR waveMonitor program.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveWaterfallAPDText = 
  geoSwath3D::tr ("<img source=\":/icons/wave_waterfall_apd.png\"> Click this button to run (or kill) the LIDAR waveWaterfall program "
                  "in APD mode.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveWaterfallPMTText = 
  geoSwath3D::tr ("<img source=\":/icons/wave_waterfall_pmt.png\"> Click this button to run (or kill) the LIDAR waveWaterfall program "
                  "in PMT mode.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString gsfMonitorText = 
  geoSwath3D::tr ("<img source=\":/icons/gsf_monitor.png\"> Click this button to run (or kill) the gsfMonitor program.<br><br>"
                  "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                  "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString toolbarText = 
  geoSwath3D::tr ("There are seven available tool bars in the geoSwath3D program.  They are<br><br>"
                  "<ul>"
                  "<li>View tool bar - contains buttons to change the view</li>"
                  "<li>Utilities tool bar - contains buttons to modify the preferences and get context sensitive help</li>"
                  "<li>Edit tool bar - contains buttons to set editing modes</li>"
                  "<li>Mask tool bar - contains buttons to mask data inside or outside of rectangles or polygons</li>"
                  "</ul>"
                  "The tool bars may be turned on or off and relocated to any location on the screen.  You may click and "
                  "drag the tool bars using the handle to the left of (or above) the tool bar.  Right clicking in the tool "
                  "bar or in the menu bar will pop up a menu allowing you to hide or show the individual tool bars.  The "
                  "location, orientation, and visibility of the tool bars will be saved on exit.");

QString maskInsideRectText = 
  geoSwath3D::tr ("<img source=\":/icons/mask_inside_rect.xpm\"> Click this button to allow you to select a rectangular area "
                  "inside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                  "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                  "rectangle and mask the data, left click again.  If, at any time during the operation, you wish to "
                  "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                  "select one of the menu options.  To clear all masking press the mask reset button "
                  "<img source=\":/icons/mask_reset.xpm\">");
QString maskOutsideRectText = 
  geoSwath3D::tr ("<img source=\":/icons/mask_outside_rect.xpm\"> Click this button to allow you to select a rectangular area "
                  "outside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                  "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                  "rectangle and mask the data, left click again.  If, at any time during the operation, you wish to "
                  "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                  "select one of the menu options.  To clear all masking press the mask reset button "
                  "<img source=\":/icons/mask_reset.xpm\">");
QString maskInsidePolyText = 
  geoSwath3D::tr ("<img source=\":/icons/mask_inside_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "inside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                  "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                  "mask the data simply left click again.  If, at any time during the operation, "
                  "you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.  To clear all masking press the mask reset button "
                  "<img source=\":/icons/mask_reset.xpm\">");
QString maskOutsidePolyText = 
  geoSwath3D::tr ("<img source=\":/icons/mask_outside_poly.xpm\"> Click this button to allow you to select a polygonal area "
                  "outside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                  "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                  "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                  "mask the data simply left click again.  If, at any time during the operation, "
                  "you wish to discard the polygon and abort the operation simply click the middle mouse "
                  "button or right click and select a menu option.  To clear all masking press the mask reset button "
                  "<img source=\":/icons/mask_reset.xpm\">");
QString maskResetText = 
  geoSwath3D::tr ("<img source=\":/icons/mask_reset.xpm\"> Click this button to clear all data masking.");
QString measureText = 
  geoSwath3D::tr ("<img source=\":/icons/measure.xpm\"> Click this button to measure distances and depth differences.  When this "
                  "is clicked the cursor will become the measure cursor <img source=\":/icons/measure_cursor.xpm\">.  Simply click "
                  "in a location and then move the cursor.  The distance, azimuth, and deltaZ will be displayed in the "
                  "lower right corner status bar.  When you are finished click any mouse button to begin measuring again.");

QString filterText = 
  geoSwath3D::tr ("<img source=\":/icons/filter.xpm\"> Click this button to filter the data points.  The filter parameters can "
                  "be modified in the <b>Preferences</b> dialog <img source=\":/icons/preferences.xpm\"><br><br>"
                  "<b>IMPORTANT NOTE: The filter operation will show you all of the points that are to be filtered and "
                  "a confirmation dialog prior to actually invalidating the filtered points.  If you really know what you're "
                  "doing and don't want to spend the time answering the dialog you can turn off filter confirmation in the "
                  "preferences dialog.  This should not be a problem since you can always undo <img source=\":/icons/undo.png\"> "
                  "the filter operation but you'll have to pay close attention to what is getting deleted if there are "
                  "features in the area.</b>");

QString filterRectMaskText = 
  geoSwath3D::tr ("<img source=\":/icons/filter_mask_rect.xpm\"> Click this button to define temporary areas to protect (mask) from "
                  "subsequent filter operations <img source=\":/icons/filter.xpm\">.<br><br>"
                  "After clicking the button the cursor will change to the rectangle mask cursor "
                  "<img source=\":/icons/filter_mask_rect_cursor.xpm\">.  "
                  "Click the left mouse button to define a starting point for the rectangle.  Moving the mouse will draw a rectangle.  "
                  "Left click to end rectangle definition and mask the area.  To abort the operation click the middle mouse button.<br><br>"
                  "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
                  "are viewing these will be discarded.</b>");
QString filterPolyMaskText = 
  geoSwath3D::tr ("<img source=\":/icons/filter_mask_poly.xpm\"> Click this button to define temporary areas to protect (mask) from "
                  "subsequent filter operations <img source=\":/icons/filter.xpm\">.<br><br>"
                  "After clicking the button the cursor will change to the polygon mask cursor "
                  "<img source=\":/icons/filter_mask_poly_cursor.xpm\">.  "
                  "Click the left mouse button to define a starting point for a polygon.  Moving the mouse will draw a line.  "
                  "Left click to define the next vertex of the polygon.  Double click to define the last vertex of the polygon.  "
                  "To abort the operation click the middle mouse button.<br><br>"
                  "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
                  "are viewing these will be discarded.</b>");
QString clearMasksText = 
  geoSwath3D::tr ("<img source=\":/icons/clear_filter_masks.xpm\"> Click this button to clear all currently defined filter masks.");

QString setStdText = 
  geoSwath3D::tr ("<img source=\":/icons/set_filter_std.png\"> Click this button to bring up a dialog to allow you to change the "
                  "filter standard deviation value.");

QString undoText = 
  geoSwath3D::tr ("<img source=\":/icons/undo.png\"> Click this button to undo the last edit operation.  When there are no edit "
                  "operations to be undone this button will not be enabled.");

QString linkText = 
  geoSwath3D::tr ("<img source=\":/icons/unlink.xpm\"> Click this button to choose from other ABE or geographical GUI "
                  "programs to connect to.  At present the only possible applications are pfmView, geoSwath, and "
                  "areaCheck.  Hopefully, in the near future, we will be adding CNILE to the list.  The connection "
                  "allows cursor tracking and limited data exchange between the applications.  If there are no "
                  "available ABE groups to link to you can create a new group and add yourself to it so that other "
                  "applications can link to this one.");

QString unlinkText = 
  geoSwath3D::tr ("<img source=\":/icons/link.xpm\"> Click this button to disconnect from another ABE or "
                  "geographical GUI program.  This option is not available unless you have already linked to another "
                  "program.");

QString filterBoxText = 
  geoSwath3D::tr ("This tab page becomes active after running the filter <img source=\":/icons/filter.xpm\"><br><br>"
                  "When this page is activei you can adjust the results of the filter by moving the standard deviation "
                  "slider up or down.  You may also change the view of the data to get a better perspective on the points "
                  "that have been selected to be invalidated.  If there are areas of the selection that you do not want "
                  "to invalidate you may use the filter mask buttons, <img source=\":/icons/filter_mask_rect.xpm\"> and/or "
                  "<img source=\":/icons/filter_mask_poly.xpm\">, to mask out those areas.  After you are completely satisfied "
                  "with the final results of the filtering operation you may accept the results by pressing the <b>Accept</b> "
                  "button or, more simply, by just pressing the <b>Return</b> key (or whatever hot key that has been set for "
                 "<i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog).<br><br>"
                  "<b>IMPORTANT TIP: Holding down the <b>Shift</b> key and moving the mouse wheel will change the standard "
                  "deviation value.  Approximately 1 to 1.5 seconds after you stop moving the mouse wheel or release the "
                  "<b>Shift</b> key the filter will be re-run and the new filter results will be displayed.</b>");

QString stdSliderText = 
  geoSwath3D::tr ("Move this slider up to increase the amount of filtering (i.e. decrease the standard deviation value) or "
                  "move it down to decrease the amount of filtering (i.e. increase the standard deviation value).  When the "
                  "slider is released the filter is re-run on the visible data and the number of points to be rejected is "
                  "modified.<br><br>"
                  "<b>IMPORTANT TIP: Holding down the <b>Shift</b> key and moving the mouse wheel will change the standard "
                  "deviation value.  Approximately 1 to 1.5 seconds after you stop moving the mouse wheel or release the "
                  "<Shift> key the filter will be re-run and the new filter results will be displayed.</b>");
