
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



/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


QString exitSaveText = 
  pfmEdit3D::tr ("<img source=\":/icons/exit_save.xpm\"> Click this button to save changes to the PFM structure and "
                 "then exit from the editor.");
QString exitMaskText = 
  pfmEdit3D::tr ("<img source=\":/icons/exit_mask.xpm\"> Click this button to save changes to the PFM structure and "
                 "then exit from the editor.  In addition, the area that was edited will be marked as a filter "
                 "masked area in pfmView.");
QString exitNoSaveText = 
  pfmEdit3D::tr ("<img source=\":/icons/exit_no_save.xpm\"> Click this button to discard changes and then exit from "
                 "the editor.");
QString resetText = 
  pfmEdit3D::tr ("<img source=\":/icons/reset_view.xpm\"> Click this button to reset to the original scale and view.");


QString displayFeatureText = 
  pfmEdit3D::tr ("<img source=\":/icons/display_all_feature.png\"> Click this button to select the feature display mode.  "
               "You can highlight features that have descriptions or remarks containing specific text strings by setting "
               "the feature search string in the preferences dialog <img source=\":/icons/prefs.xpm\">.");
QString displayChildrenText = 
  pfmEdit3D::tr ("<img source=\":/icons/displayfeaturechildren.xpm\"> Click this button to display feature sub-records.  "
                 "Feature sub-records are features that have been grouped under a master feature record.  Features "
                 "can be grouped and un-grouped in the edit feature dialog of pfmView.");
QString displayFeatureInfoText = 
  pfmEdit3D::tr ("<img source=\":/icons/displayfeatureinfo.xpm\"> Click this button to write the description and "
                 "remarks fields of features next to any displayed features.  This button is "
                 "meaningless if <b>Flag Feature Data</b> is set to not display features "
                 "<img source=\":/icons/display_no_feature.png\">.");
QString verifyFeaturesText = 
  pfmEdit3D::tr ("<img source=\":/icons/verify_features.png\"> Click this button to set all valid, displayed features to a confidence "
		 "level of 5.  These will be displayed in a different color in the editor and in pfmView.  The only way to <b><i>unverify</i></b> "
		 "a feature is to edit the feature and manually set the confidence level to a lower value than 5.");


QString displayReferenceText = 
  pfmEdit3D::tr ("<img source=\":/icons/displayreference.xpm\"> Click this button to flag reference data.  Reference points "
                 "may or may not be invalid but they are not used for computing the surfaces or as selected soundings.  "
                 "Examples of data that might be flagged as reference data would include fishing nets, floating buoys, "
                 "non-permanent platforms, depth data from sidescan systems, or other data that are valid but not "
                 "normally used as part of the surfaces.");
QString displayManInvalidText = 
  pfmEdit3D::tr ("<img source=\":/icons/display_manually_invalid.png\"> Click this button to display manually invalidated "
                 "data points along with the valid data points.  If you need to differentiate between valid and invalid you "
                 "can use the data flags options to mark invalid data points.<br><br>"
                 "<b>IMPORTANT NOTE: Data that has been marked as invalid using the filter in the editors or the surface "
                 "viewer are marked as MANUALLY invalid.  Since you are reviewing the filter results this is considered manually "
                 "accepting the filter.  Points that are filtered in pfmLoad or that are marked as invalid by other "
                 "software packages (e.g. Optech's GCS) are marked as FILTER invalid.</b>");
QString displayFltInvalidText = 
  pfmEdit3D::tr ("<img source=\":/icons/display_filter_invalid.png\"> Click this button to display filter invalidated "
                 "data points along with the valid data points.  If you need to differentiate between valid and invalid you "
                 "can use the data flags options to mark invalid data points.<br><br>"
                 "<b>IMPORTANT NOTE: Data that has been marked as invalid using the filter in the editors or the surface "
                 "viewer are marked as MANUALLY invalid.  Since you are reviewing the filter results this is considered manually "
                 "accepting the filter.  Points that are filtered in pfmLoad or that are marked as invalid by other "
                 "software packages (e.g. Optech's GCS) are marked as FILTER invalid.</b>");
QString displayNullText = 
  pfmEdit3D::tr ("<img source=\":/icons/display_null.png\"> Click this button to display data points that were loaded "
                 "as NULL values.  This is primarily useful for LIDAR data but it will also display data "
                 "points that fell outside the min/max envelope on load (these were set to NULL).  This "
                 "button is only meaningful if display manually invalid <img source=\":/icons/display_manually_invalid.xpm\"> "
                 "or display filter invalid <img source=\":/icons/display_filter_invalid.xpm\"> is set to on.");

QString displayAllText = 
  pfmEdit3D::tr ("<img source=\":/icons/displayall.xpm\"> Click this button to display all lines after selecting "
                 "a single line or a subset of lines to view.");

QString undisplaySingleText = 
  pfmEdit3D::tr ("<img source=\":/icons/undisplaysingle.xpm\"> Click this button to turn off the display for a single line.  "
                 "When selected the cursor will become the ArrowCursor.  Move the box and arrow cursor to a point in the line "
                 "that you do not want to display and then left click.  The display will redraw after each left click.  This mode "
                 "will remain active until you switch to another mode.  After selecting lines to be hidden you can "
                 "revert back to displaying all data by pressing the <b>View All</b> button "
                 "<img source=\":/icons/displayall.xpm\">.");

QString displayMultipleText = 
  pfmEdit3D::tr ("<img source=\":/icons/displaylines.xpm\"> Click this button to select multiple lines to view.  "
                 "When selected the cursor will become the ArrowCursor.  Move the box and arrow cursor to a point "
                 "in a line that you want to display and then left click to select that line to view.  Double click "
                 "to select the last line to view and redraw the display.  If you decide that you don't want to display "
                 "the lines you can middle click to abort.  After selecting multiple lines to view you can revert back "
                 "to displaying all data by pressing the <b>View All</b> button <img source=\":/icons/displayall.xpm\">.<br><br>"
                 "<b>IMPORTANT NOTE: If you would like to select the lines from a list you can do so by selecting the "
                 "obvious option in the right click popup menu.</b>");

QString highlightPolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/highlight_polygon.xpm\"> Click this button to allow you to select a polygonal area "
                 "inside of which you wish to highlight data points.  When selected the cursor will become the highlight polygon cursor "
                 "<img source=\":/icons/highlight_polygon_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "highlight the data in the polygon simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.<br><br>"
                 "<b>IMPORTANT NOTE: If you would like to delete highlighted data points just press <i>Del</i> or whatever "
                 "hot key that has been set for <i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog.  Conversely, "
                 "if you have invalid data being displayed and you have highlighted some of them you can press <i>Ins</i> or "
                 "whatever hot key that has been set for <i>REJECT FILTER HIGHLIGHTED</i> in the Preferences dialog to validate "
                 "those points.  Also, if you were flagging data (i.e. highlighting by data flag) using this function will "
                 "turn off flagging but won't remove the highlights from the points that you previously flagged.  This allows you "
                 "to use data flagging to delete or restore points.</b>");
QString invertHighlightText = 
  pfmEdit3D::tr ("<img source=\":/icons/invert_highlight.png\"> Click this button to invert the highlighted selection.  That is, "
                 "all points that are currently highlighted will be un-highlighted and all points that aren't currently highlighted "
                 "will be highlighted.");
QString clearPolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/clear_polygon.xpm\"> Click this button to allow you to select a polygonal area "
                 "inside of which you wish to un-highlight data points.  When selected the cursor will become the clear polygon cursor "
                 "<img source=\":/icons/clear_polygon_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "un-highlight the data in the polygon simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.<br><br>"
                 "<b>IMPORTANT NOTE: If you would like to delete highlighted data points just press <i>Del</i> or whatever "
                 "hot key that has been set for <i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog.  Conversely, "
                 "if you have invalid data being displayed and you have highlighted some of them you can press <i>Ins</i> or "
                 "whatever hot key that has been set for <i>REJECT FILTER HIGHLIGHTED</i> in the Preferences dialog to validate "
                 "those points.  Also, if you were flagging data (i.e. highlighting by data flag) using this function will "
                 "turn off flagging but won't remove the highlights from the points that you previously flagged.  This allows you "
                 "to use data flagging to delete or restore points.</b>");
QString clearHighlightText = 
  pfmEdit3D::tr ("<img source=\":/icons/clear_highlight.xpm\"> Click this button to clear all highlighted points from "
                 "the display.<br><br>"
                 "<b>IMPORTANT NOTE: If you were flagging data (i.e. highlighting by data flag) pressing this button will "
                 "turn off flagging.</b>");

QString contourText = 
  pfmEdit3D::tr ("<img source=\":/icons/contour.xpm\"> Click this button to toggle drawing of contours.");


QString unloadText = 
  pfmEdit3D::tr ("<img source=\":/icons/unload.xpm\"> This toggle button controls whether pfmEdit3D will automatically unload "
                 "edits made to the PFM data to the original input files when you save and exit.");


QString prefsText = 
  pfmEdit3D::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  This "
                 "includes colors, contour interval, minimum Z window size, position display format, and all hot keys.");


QString stopText = 
  pfmEdit3D::tr ("<img source=\":/icons/stop.xpm\"> Click this button to cancel drawing of the data.  A much easier "
                 "way to do this though is to click any mouse button in the display area or press any "
                 "key on the keyboard.  The stop button is really just there so that the interface looks "
                 "similar to the viewer and also to provide a place for help on how to stop the drawing.");


QString deletePointText = 
  pfmEdit3D::tr ("<img source=\":/icons/delete_point.xpm\"> Click this button to select delete subrecord/record/file "
                 "mode.  In this mode you can place the cursor on a subrecord (beam, shot, point) and "
                 "delete the subrecord by pressing the <Shift> key and then left clicking (or via the right click menu), "
                 "delete the record (ping, shot) by pressing the middle mouse button, or mark the file for deletion in pfmView "
                 "using the right click menu.  Information about the current "
                 "point will be displayed in the status bars at the bottom of the window.<br><br>"
                 "You can also mark a point by using the right-click popup menu.  Note that the <b>Highlight point</b> option may be "
                 "disabled if you have filter kill points up or are flagging data using any of the flag buttons "
                 "<img source=\":/icons/user_flag.xpm\"><br><br>"
                 "Hot keys and action keys are available when in this mode.  Please check the <b>Preferences<b> dialog "
                 "<img source=\":/icons/prefs.xpm\">, specifically the <b>Ancillary Programs</b> dialog, to see what action "
                 "keys are available for each ancillary program.  To find out what the keys do in the ancillary program "
                 "you must use the help in the ancillary program.  You can also run ancillary programs by selecting the "
                 "program from the right click popup menu.  " 
                 "<p><b><i>IMPORTANT NOTE: Only in this mode, ADD_FEATURE mode <img source=\":/icons/addfeature.xpm\">, or "
                 "EDIT_FEATURE mode <img source=\":/icons/editfeature.xpm\"> can you use hot keys to launch ancillary "
                 "programs.</i></b></p>");
QString deleteRectText = 
  pfmEdit3D::tr ("<img source=\":/icons/delete_rect.xpm\"> Click this button to allow you to select a rectangular area "
                 "to invalidate.  When selected the cursor will become the invalidate rectangle cursor "
                 "<img source=\":/icons/delete_rect_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                 "rectangle and invalidate the data left click again.  If, at any time during the operation, you wish to "
                 "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                 "select one of the menu options.");
QString deletePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/delete_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "to invalidate.  When selected the cursor will become the invalidate polygon cursor "
                 "<img source=\":/icons/delete_poly_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "invalidate the data in the polygon simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.");
QString keepPolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/keep_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "and invalidate all data <b>outside</b> of the polygon.  When selected the cursor will "
                 "become the invalidate outside polygon cursor <img source=\":/icons/keep_poly_cursor.xpm\">.  Left "
                 "clicking on a location will cause that point to be the polygon start point.  To close the polygon and "
                 "invalidate the data outside of the polygon simply left click again.  If, at any time during the "
                 "operation, you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.");
QString restoreRectText = 
  pfmEdit3D::tr ("<img source=\":/icons/restore_rect.xpm\"> Click this button to allow you to select a rectangular area "
                 "in which to restore invalidated data.  When selected the cursor will become the restore rectangle "
                 "cursor <img source=\":/icons/restore_rect_cursor.xpm\">.  Left clicking on a location will cause that "
                 "point to be the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize "
                 "the rectangle and restore the invalid data simply left click again.  If, at any time during the "
                 "operation, you wish to discard the rectangle and abort the operation simply click the middle mouse button "
                 "or right click and select one of the menu options.");
QString restorePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/restore_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "in which to restore invalidated data.  When selected the cursor will become the restore polygon cursor "
                 "<img source=\":/icons/restore_poly_cursor.xpm\">.  Left clicking on a location will cause that point to "
                 "be the polygon start point.  To close the polygon and restore invalid data in the polygon simply left "
                 "click again.  If, at any time during the operation, you wish to discard the polygon and abort the "
                 "operation simply click the middle mouse button or right click and select a menu option.");

QString referencePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/reference_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "in which to set all points to reference points.  Reference points may or may not be invalid but they are "
                 "not used for computing the surfaces or as selected soundings.  Examples of data that might be flagged as "
                 "reference data would include fishing nets, floating buoys, non-permanent platforms, depth data from "
                 "sidescan systems, or other data that are valid but not normally used as part of the surfaces.  "
                 "When selected the cursor will become the set reference polygon cursor "
                 "<img source=\":/icons/reference_poly_cursor.xpm\">.  Left clicking on a location will cause that point to "
                 "be the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "set the data in the polygon to reference simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.<br><br>"
                 "<b>IMPORTANT NOTE: There is an extra right click popup menu option for this mode.  It allows you to delete "
                 "all non-masked (visible) or non-transparent reference points.  There is a stupid, Micro$oft-like <i>'Do "
                 "you really want to do this'</i> type of message when you use that option.<b>");
QString unreferencePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/unreference_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "in which to set all reference points to non-reference points.  Reference points may or may not be "
                 "invalid but they are not used for computing the surfaces or as selected soundings.  Examples of data "
                 "that might be flagged as reference data would include fishing nets, floating buoys, non-permanent "
                 "platforms, depth data from sidescan systems, or other data that are valid but not normally used as part "
                 "of the surfaces.  When selected the cursor will become the unset reference polygon cursor "
                 "<img source=\":/icons/unreference_poly_cursor.xpm\">.  Left clicking on a location will cause that point "
                 "to be the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "set the reference data in the polygon to non-reference data simply left click again.  If, at any time "
                 "during the operation, you wish to discard the polygon and abort the operation simply click the middle "
                 "mouse button or right click and select a menu option.<br><br>"
                 "<b>IMPORTANT NOTE: There is an extra right click popup menu option for this mode.  It allows you to delete "
                 "all non-masked (visible) or non-transparent reference points.  There is a stupid, Micro$oft-like <i>'Do "
                 "you really want to do this'</i> type of message when you use that option.<b>");
QString hotkeyPolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/hotkey_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "in which to run an ancillary program that is started with a hotkey.  When selected the cursor will "
                 "become the hotkey polygon cursor <img source=\":/icons/hotkey_poly_cursor.xpm\">.  Left clicking on a "
                 "location will cause that point to be the polygon start point.  Moving the cursor will draw a continuous "
                 "line.  To close the polygon in preparation for pressing a hotkey simply left click again.  After defining "
                 "the polygon just press the hotkey that is associated with the ancillary program that you wish to run or select "
                 "the program from the right click popup menu.  You can also invalidate groups of features using this function.<br>"
                 "To see the available ancillary programs look under preferences <img source=\":/icons/prefs.xpm\">.  If, "
                 "at any time during the operation, you wish to discard the polygon and abort the operation simply click "
                 "the middle mouse button or right click and select a menu option.");

QString addFeatureText = 
  pfmEdit3D::tr ("<img source=\":/icons/addfeature.xpm\"> Click this button to allow you to add a feature to an existing "
                 "BFD feature file.  If a feature file does not exist you will be given the option of creating "
                 "one.  When selected the cursor will become the add feature cursor "
                 "<img source=\":/icons/add_feature_cursor.xpm\">.  "
                 "The box cursor will snap to the nearest point.  You may place a feature by left clicking on the desired "
                 "point.  When you left click a feature edit dialog will appear for you to modify.  If, at any time "
                 "during the operation, you wish to abort the operation simply click the middle mouse button or right "
                 "click and select one of the menu options.<br><br>"
                 "Hot keys and action keys are available when in this mode.  Please check the <b>Preferences<b> dialog "
                 "<img source=\":/icons/prefs.xpm\">, specifically the <b>Ancillary Programs</b> dialog, to see what action "
                 "keys are available for each ancillary program.  To find out what the keys do in the ancillary program "
                 "you must use the help in the ancillary program." 
                 "<p><b><i>IMPORTANT NOTE: Only in this mode, DELETE_POINT mode <img source=\":/icons/delete_point.xpm\">, "
                 "or EDIT_FEATURE mode <img source=\":/icons/editfeature.xpm\"> can you use hot keys to launch ancillary "
                 "programs.</i></b></p>");
QString editFeatureText = 
  pfmEdit3D::tr ("<img source=\":/icons/editfeature.xpm\"> Click this button to allow you to edit an existing feature.  "
                 "Note that you may only edit invalid features if you have invalid data viewing "
                 "<img source=\":/icons/displayinvalid.xpm\"> toggled on.  When selected the cursor will become the "
                 "edit feature cursor <img source=\":/icons/edit_feature_cursor.xpm\">.  The box cursor will snap to the "
                 "nearest feature.  "
                 "You may edit that feature by left clicking while the box cursor is attached to the desired feature.  "
                 "When you left click a feature edit dialog will appear for you to modify.  If, at any time during the "
                 "operation, you wish to abort the operation simply click the middle mouse button or right click and "
                 "select one of the menu options.<br><br>"
                 "Hot keys and action keys are available when in this mode.  Please check the <b>Preferences<b> dialog "
                 "<img source=\":/icons/prefs.xpm\">, specifically the <b>Ancillary Programs</b> dialog, to see what action "
                 "keys are available for each ancillary program.  To find out what the keys do in the ancillary program "
                 "you must use the help in the ancillary program.  Note that the 6th option for <b>ChartsPic</b> will "
                 "<b>ONLY</b> work in this mode.  The 6th option is used to get <b>chartsPic</b> to save a downlooking "
                 "image associated with a CHARTS HOF or TOF file.  The 6th option is normally <b><i>t</i></b> but may "
                 "have been changed by the user."
                 "<p><b><i>IMPORTANT NOTE: Only in this mode, DELETE_POINT mode <img source=\":/icons/delete_point.xpm\">, "
                 "or ADD_FEATURE mode <img source=\":/icons/addfeature.xpm\"> can you use hot keys to launch ancillary "
                 "programs.</i></b></p>");
QString moveFeatureText = 
  pfmEdit3D::tr ("<img source=\":/icons/movefeature.xpm\"> Click this button to allow you to move an existing feature to "
                 "another location.  When selected the cursor will become the move feature cursor "
                 "<img source=\":/icons/move_feature_cursor.xpm\">.  The box cursor will snap to the nearest feature.  "
                 "Acquire that feature by left clicking while the box cursor is attached to the desired feature.  After left "
                 "clicking the box cursor will then snap to the nearest point instead of the nearest feature.  To attach the "
                 "feature to that point simply left click again.  If, at any time during the operation, you wish to abort "
                 "the operation simply click the middle mouse button or right clicking and selecting an option from the "
                 "menu.");
QString deleteFeatureText = 
  pfmEdit3D::tr ("<img source=\":/icons/deletefeature.xpm\"> Click this button to allow you to invalidate an existing "
                 "feature.  This function will set the feature confidence value to 0.  When selected the cursor will become "
                 "the delete feature cursor <img source=\":/icons/delete_feature_cursor.xpm\">.  The box cursor will snap to "
                 "the nearest feature.  You may delete that feature by left clicking while the box cursor is attached to the "
                 "desired feature.  If, at any time during the operation, you wish to abort the operation simply click the "
                 "middle mouse button or right clicking and selectin an option from the menu.");


//  If you modify mapText, change the text in hotkeyHelp.cpp as well.

QString mapText = 
  pfmEdit3D::tr ("The pfmEdit3D program is used to display and edit data points that have been stored "
                 "in a PFM structure.  This program is launched from the pfmView program and is "
                 "not run from the command line.  The various editing and viewing options are "
                 "initiated from the tool bar buttons.  Help is available for each button by "
                 "clicking on the What's This button <img source=\":/icons/contextHelp.xpm\"> then clicking "
                 "on the item of interest.<br><br>"
                 "Point of view is controlled by holding down the <b>Ctrl</b> key and clicking and dragging with the "
                 "left mouse button.  It can also be controlled by holding down the <b>Ctrl</b> key and using the "
                 "left, right, up, and down arrow keys.  Zoom is controlled by holding down the <b>Ctrl</b> key and "
                 "using the mouse wheel or holding down the <b>Ctrl</b> key, pressing the right mouse button, and moving "
                 "the cursor up or down.  To reposition the center of the view just place the cursor at the desired "
                 "location, hold down the <b>Ctrl</b> key, and click the middle mouse button.  Z exaggeration can be "
                 "changed by pressing <b>Ctrl-Page Up</b> or <b>Ctrl-Page Down</b> or by editing "
                 "the exaggeration value in the Preferences dialog <img source=\":/icons/prefs.xpm\"><br><br><br>"
                 "Slicing of the data is initiated by using the mouse wheel, dragging the slider "
                 "in the scroll bar on the right of the display, by clicking the up or down arrow keys (without "
                 "holding down the <b>Ctrl</b> key), or pressing the up and down arrow buttons in the slice scroll bar.  "
                 "<br><br>"
                 "<b>IMPORTANT NOTE: When in DELETE_POINT mode <img source=\":/icons/delete_point.xpm\">, "
                 "ADD_FEATURE mode <img source=\":/icons/addfeature.xpm\">, or EDIT_FEATURE mode "
                 "<img source=\":/icons/editfeature.xpm\"> hot keys and action keys will be active.  Use the Hotkeys Help "
                 "entry in the Help menu to see what all of the hotkeys and action keys are.  These keys may be changed "
                 "in the Preferences dialog if needed.");

QString trackMapText = 
  pfmEdit3D::tr ("This is the 2D tracking map.  It will show you where your cursor is in relation to the 3D cursor position.");

QString exagBarText = 
  pfmEdit3D::tr ("This scroll bar controls/monitors the vertical exaggeration of the data.  Pressing the up and down "
                 "arrow keys will change the exaggeration by 1.0.  Clicking the mouse while the cursor is in the trough "
                 "will change the value by 5.0.  Dragging the slider will display the value in the <b>Exag:</b> field in "
                 "the status area on the left but the exaggeration will not change until the slider is released.  The "
                 "maximum value is 100.0 and the minimum value is 1.0.  If the exaggeration has been set to less than 1.0 "
                 "due to large vertical data range the scroll bar will be inactive.  The exaggeration can also be changed "
                 "by pressing the <b>Ctrl</b> key and then simultaneously pressing the <b>PageUp</b> or <b>PageDown</b> button.");

QString sliceBarText = 
  pfmEdit3D::tr ("This scroll bar controls/monitors the slice as it moves through the data.  Pressing the up and down "
                 "arrow keys will move the slice through the data.  The slice size will be the percent of the viewable data "
                 "that is set with the slice size scroll bar.  When you press the up arrow the view will move "
                 "into the data one slice.  When displaying a slice, any edits will only effect data that is "
                 "in full color (non-translucent).  Slicing is extremely handy in dealing with very bumpy surfaces "
                 "to allow you to clean up hidden flyers.  You can turn off slicing by right clicking and selecting "
                 "<b>Turn off slicing</b>, by double clicking anywhere in the window, or by pressing the <b>Esc</b> key.");

QString sizeText = 
  pfmEdit3D::tr ("Set the slice size using this scroll bar.  To understand how slicing works try "
                 "to visualize the surface as it would be in plan view but with the bottom (leading) edge defined "
                 "by the angle of view.  The slice size will be this percentage (default is 5 percent or 1/20th) of "
                 "the displayed data.  When you press one of the up or down arrow keys or click one of the slice bar "
                 "arrow buttons the slice will move by this amount.  The range for this scroll bar is 1 to 50.");


QString transText = 
  pfmEdit3D::tr ("Set the transparency value to be used for data that is not in the current slice.  If "
                 "the slider is moved to the bottom, data outside the slice will be invisible.  If it is moved to the top "
                 "the data will be almost completely opaque.  The range for this scroll bar is 0 (transparent) to 64 "
                 "(semi-transparent).");

QString colorScaleBoxText = 
  pfmEdit3D::tr ("The color scale is an index for the color ranges.  The scale can be turned off in the preferences dialog.<br><br>"
                 "<b>IMPORTANT NOTE: When the color scheme is color by line the color scale is not applicable and will be set to "
                 "all white with no values.</b>");


QString pfmEdit3DAboutText = 
  pfmEdit3D::tr ("<center>pfmEdit3D<br><br>"
                 "Author : Jan C. Depner (jan.depner@navy.mil)<br>"
                 "Date : 31 January 2005<br><br>"
                 "The History Of PFM<br><br></center>"
                 "PFM, or Pure File Magic, was conceived on a recording trip to Nashville in early 1996.  "
                 "The design was refined over the next year or so by the usual band of suspects.  The "
                 "purpose of this little piece of work was to allow hydrographers to geographically view "
                 "minimum, maximum, and average binned surfaces, of whatever bin size they chose, and "
                 "then allow them to edit the original depth data.  After editing the depth data, the "
                 "bins would be recomputed and the binned surface redisplayed.  The idea being that the "
                 "hydrographer could view the min or max binned surface to find flyers and then just "
                 "edit those areas that required it.  In addition to the manual viewing and hand editing, "
                 "the PFM format is helpful for automatic filtering in places where data from different "
                 "files overlaps.  Also, there is a hook to a mosaic imagery file that can contain "
                 "sidescan mosaic data.  pfmEdit3D is a very simple Qt binned surface viewer.  After "
                 "all of the editing is finished, the status information can be loaded back into the "
                 "original raw input data files.<br><br>"
                 "The author (Evil Twin) would like to acknowledge the contributions to the PFM design "
                 "of the usual band of suspects:"
                 "<ul>"
                 "<li>Commissioner Gordon - Jim Hammack, NAVO</li>"
                 "<li>Fabio               - Dave Fabre, Neptune Sciences, Inc.</li>"
                 "<li>Oh Yes              - Becky Martinolich, NAVO</li>"
                 "</ul><br><br><br>"
                 "<center>Jan '<i>Evil Twin</i>' Depner<br>"
                 "<a href=\"http://en.wikipedia.org/wiki/Naval_Oceanographic_Office\">Naval Oceanographic Office</a><br>"
                 "jan.depner@navy.mil<br></center>");

QString acknowledgementsText = 
  pfmEdit3D::tr ("<center><br>pfmEdit3D was built using some, if not all, of the following Open Source libraries:"
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
  pfmEdit3D::tr ("This area displays the currently active mode or information about the operation in progress.");

QString statusBarText = 
  pfmEdit3D::tr ("The status bar is used to display the progress of the current operation.");

QString progStatusText = 
  pfmEdit3D::tr ("The status bar is used to display the progress of the current operation.");

QString attrText = 
  pfmEdit3D::tr ("Click this button to access a pulldown menu allowing you to select coloring options.  There will always be "
                 "at least four options available.  These are:<br><br>"
                 "<ul>"
                 "<li><img source=\":/icons/color_by_depth.xpm\"> - color by depth</li>"
                 "<li><img source=\":/icons/color_by_line.xpm\"> - color by line</li>"
                 "<li><img source=\":/icons/horiz.xpm\"> - color by horizontal uncertainty</li>"
                 "<li><img source=\":/icons/vert.xpm\"> - color by vertical uncertainty</li>"
                 "</ul><br><br>"
                 "In addition to the above options there may be up to ten more numbered "
                 "color options.  These correspond to PFM attributes which may "
                 "be in the displayed PFM data and will look like this - <img source=\":/icons/attr04.xpm\">");

QString flagText = 
  pfmEdit3D::tr ("Click this button to access a pulldown menu allowing you to select data flagging options.  The "
                 "flagging options are:<br><br>"
                 "<ul>"
                 "<li><img source=\":/icons/suspect.xpm\"> - mark suspect data</li>"
                 "<li><img source=\":/icons/selected.xpm\"> - mark selected soundings</li>"
                 "<li><img source=\":/icons/feature.xpm\"> - mark selected feature soundings</li>"
                 "<li><img source=\":/icons/designated.xpm\"> - mark hydrographer designated soundings</li>"
                 "<li><img source=\":/icons/user_flag01.xpm\"> - mark PFM_USER_01 flagged data</li>"
                 "<li><img source=\":/icons/user_flag02.xpm\"> - mark PFM_USER_02 flagged data</li>"
                 "<li><img source=\":/icons/user_flag03.xpm\"> - mark PFM_USER_03 flagged data</li>"
                 "<li><img source=\":/icons/user_flag04.xpm\"> - mark PFM_USER_04 flagged data</li>"
                 "<li><img source=\":/icons/user_flag05.xpm\"> - mark PFM_USER_05 flagged data</li>"
                 "</ul><br><br>"
                 "The PFM_USER_NN flags may be disabled if the PFM file being viewed does not have valid "
                 "user flags.");

QString hideText = 
  pfmEdit3D::tr ("Click this button to access a pulldown menu allowing you to select flagged data to be hidden.  "
		 "There are five options available.  These are:<br><br>"
                 "<ul>"
                 "<li>hide PFM_USER_01 flagged data</li>"
                 "<li>hide PFM_USER_02 flagged data</li>"
                 "<li>hide PFM_USER_03 flagged data</li>"
                 "<li>hide PFM_USER_04 flagged data</li>"
                 "<li>hide PFM_USER_05 flagged data</li>"
                 "</ul><br><br>"
                 "The PFM_USER_NN flags may be disabled if the PFM file being viewed does not have valid "
                 "user flags.  More than one data type may be selected to be hidden.<br><br>"
		 "<b>IMPORTANT NOTE: Selecting any of these options <i>DOES NOT</i> cause data that doesn't "
		 "contain matching PFM_USER flags to be unmasked (i.e. shown).  If a point is already masked "
		 "it will remain masked.  Also, once you hide something you can't get it back until you reset "
                 "or click a show button.  This is so you can combine data type/flag hiding with masking.</b>");


QString showText = 
  pfmEdit3D::tr ("Click this button to access a pulldown menu allowing you to select flagged data to be shown.  "
		 "There are five options available.  These are:<br><br>"
                 "<ul>"
                 "<li>show PFM_USER_01 flagged data</li>"
                 "<li>show PFM_USER_02 flagged data</li>"
                 "<li>show PFM_USER_03 flagged data</li>"
                 "<li>show PFM_USER_04 flagged data</li>"
                 "<li>show PFM_USER_05 flagged data</li>"
                 "</ul><br><br>"
                 "The PFM_USER_NN flags may be disabled if the PFM file being viewed does not have valid "
                 "user flags.  More than one data type may be selected to be hidden.<br><br>"
		 "<b>IMPORTANT NOTE: Selecting any of these options will hide any data that does not "
		 "match the PFM_USER flag.  Think of this as a <i>'show only these types'</i> option.</b>");

QString attributeViewerText = 
  pfmEdit3D::tr ("<img source=\":/icons/attributeviewer.png\"> Click this button to run (or kill) the LIDAR Attribute Viewer program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString distanceThreshText = 
  pfmEdit3D::tr ("<img source=\":/icons/distance_threshold.xpm\"> Click this button to access the Attribute Viewer's Distance Threshold tool.  "
                 "This tool will allow the user to draw a line within the 3D environment and filter all of the shots within a distance radius.  "
                 "Only two dimensions (lat, lon) are used for this calculation<br>"
                 "<b>IMPORTANT NOTE: There is not hotkey for this button as it is not an external process. </b>");

QString lidarMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/lidar_monitor.png\"> Click this button to run (or kill) the lidarMonitor program.  The "
                 "lidarMonitor program will display HOF, TOF, WLF, CZMIL, or HAWKEYE record data for the current point nearest the "
                 "cursor in a text format.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString rmsMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/rms_monitor.png\"> Click this button to run (or kill) the rmsMonitor program.  The "
                 "rmsMonitor program will display navigation RMS data in a text format for the current HOF of TOF point nearest the "
                 "cursor.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString chartsPicText = 
  pfmEdit3D::tr ("<img source=\":/icons/charts_pic.png\"> Click this button to run (or kill) the LIDAR chartsPic program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveformMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/waveform_monitor.xpm\"> Click this button to run (or kill) the LIDAR waveformMonitor program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString CZMILwaveMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/wave_monitor.xpm\"> Click this button to run (or kill) the LIDAR CZMILwaveMonitor program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveWaterfallAPDText = 
  pfmEdit3D::tr ("<img source=\":/icons/wave_waterfall_apd.png\"> Click this button to run (or kill) the LIDAR waveWaterfall program "
                 "in APD mode.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString waveWaterfallPMTText = 
  pfmEdit3D::tr ("<img source=\":/icons/wave_waterfall_pmt.png\"> Click this button to run (or kill) the LIDAR waveWaterfall program "
                 "in PMT mode.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString hawkeyeMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/hawkeye_monitor.png\"> Click this button to run (or kill) the LIDAR hawkeyeMonitor program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString gsfMonitorText = 
  pfmEdit3D::tr ("<img source=\":/icons/gsf_monitor.png\"> Click this button to run (or kill) the gsfMonitor program.<br><br>"
                 "<b>IMPORTANT NOTE: The hotkey for this button can be changed by changing the associated ancillary program hotkey "
                 "in the Preferences <img source=\":/icons/preferences.xpm\"> dialog.</b>");

QString layerPrefsText = 
  pfmEdit3D::tr ("Selecting this menu item will cause the layer preferences dialog to appear.  In this dialog you can "
                 "turn display of the layers (i.e. PFM files) on or off.");

QString toolbarText = 
  pfmEdit3D::tr ("There are seven available tool bars in the pfmEdit3D program.  They are<br><br>"
                 "<ul>"
                 "<li>View tool bar - contains buttons to change the view</li>"
                 "<li>Feature tool bar - contains buttons to view and modify the features</li>"
                 "<li>Reference tool bar - contains buttons to view and change reference status (also view suspect)</li>"
                 "<li>Selected tool bar - contains buttons to view and change selected soundings</li>"
                 "<li>Utilities tool bar - contains buttons to modify the preferences and get context sensitive help</li>"
                 "<li>Edit tool bar - contains buttons to set editing modes</li>"
                 "<li>Mask tool bar - contains buttons to mask data inside or outside of rectangles or polygons</li>"
                 "</ul>"
                 "The tool bars may be turned on or off and relocated to any location on the screen.  You may click and "
                 "drag the tool bars using the handle to the left of (or above) the tool bar.  Right clicking in the tool "
                 "bar or in the menu bar will pop up a menu allowing you to hide or show the individual tool bars.  The "
                 "location, orientation, and visibility of the tool bars will be saved on exit.");

QString maskInsideRectText = 
  pfmEdit3D::tr ("<img source=\":/icons/mask_inside_rect.xpm\"> Click this button to allow you to select a rectangular area "
                 "inside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                 "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                 "rectangle and mask the data, left click again.  If, at any time during the operation, you wish to "
                 "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                 "select one of the menu options.  To clear all masking press the mask reset button "
                 "<img source=\":/icons/mask_reset.xpm\">");
QString maskOutsideRectText = 
  pfmEdit3D::tr ("<img source=\":/icons/mask_outside_rect.xpm\"> Click this button to allow you to select a rectangular area "
                 "outside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                 "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the rectangle anchor point.  Moving the cursor will cause a rectangle to appear.  To finalize the "
                 "rectangle and mask the data, left click again.  If, at any time during the operation, you wish to "
                 "discard the rectangle and abort the operation simply click the middle mouse button or right click and "
                 "select one of the menu options.  To clear all masking press the mask reset button "
                 "<img source=\":/icons/mask_reset.xpm\">");
QString maskInsidePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/mask_inside_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "inside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                 "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "mask the data simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.  To clear all masking press the mask reset button "
                 "<img source=\":/icons/mask_reset.xpm\">");
QString maskOutsidePolyText = 
  pfmEdit3D::tr ("<img source=\":/icons/mask_outside_poly.xpm\"> Click this button to allow you to select a polygonal area "
                 "outside of which you want to hide the data.  When selected the cursor will become the mask cursor "
                 "<img source=\":/icons/mask_cursor.xpm\">.  Left clicking on a location will cause that point to be "
                 "the polygon start point.  Moving the cursor will draw a continuous line.  To close the polygon and "
                 "mask the data simply left click again.  If, at any time during the operation, "
                 "you wish to discard the polygon and abort the operation simply click the middle mouse "
                 "button or right click and select a menu option.  To clear all masking press the mask reset button "
                 "<img source=\":/icons/mask_reset.xpm\">");
QString maskResetText = 
  pfmEdit3D::tr ("<img source=\":/icons/mask_reset.xpm\"> Click this button to clear all data masking.");

QString measureText = 
  pfmEdit3D::tr ("<img source=\":/icons/measure.xpm\"> Click this button to measure distances and depth differences.  When this button "
                 "is clicked the cursor will become the measure cursor <img source=\":/icons/measure_cursor.xpm\">.  Position the "
                 "box/crosshair on a point and then move the cursor.    The distance in meters, azimuth in degrees, and the difference "
                 "in Z in meters will be displayed in the status bar.  When you are finished click the left mouse button.<br><br>"
                 "<b>IMPORTANT NOTE: There are two sets of distance, azimuth, and deltaZ values.  They are labeled Nearest Point and "
                 "Cursor.  The nearest point is the one that the box/crosshair snaps to.  The cursor position is a bit more "
                 "complicated.  The cursor position is computed by assuming that the cursor  is actually in a plane that is parallel to the "
                 "screen face and at the depth of the anchor point (the starting point of the line, not the nearest point).  This gives "
                 "the user the ability to measure approximate distances without having to use the position of the nearest point to "
                 "compute an absolute distance.<br><br>"
                 "<font color=\"#ff0000\">TIP: To minimize Z distortion when trying to get approximate distances press the Reset button "
                 "<img source=\":/icons/reset_view.xpm\"> and then use the left and right arrow keys to rotate the view.</font></b>");

QString filterText = 
  pfmEdit3D::tr ("<img source=\":/icons/filter.xpm\"> Click this button to filter the data points.  The filter parameters can "
                 "be modified in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br><br>"
                 "<b>IMPORTANT NOTE: The filter operation will mark all of the points that are to be filtered and display "
                 "a confirmation dialog prior to actually invalidating the filtered points.<br><br>"
                 "<b><font color=\"#ff0000\">TIP: After running the filter, while the confirmation dialog is visible, you can still mask out any "
                 "points that you don't want to filter and the confirmation dialog will be updated to reflect the change.  You can also adjust the "
                 "filter standard deviation in the confirmation dialog which will re-run the filter.  You can use <i>Shift</i> and the mouse wheel to "
                 "change the value of the standard deviation slider.  In approximately 1 to 1.5 seconds after you quit scrolling the mouse wheel "
                 "or release the <i>Shift</i> key the filter will re-run.  Pressing the <i>Del</i> key (or whatever hot key that has been set for "
                 "<i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog) while the filter confirmation dialog is present will automatically "
                 "<i>Accept</i> the filter operation.  Thanks, and a tip of the hat to <font color=\"#000000\">Micah Tinkler</font> for the filtering "
                 "in the editor idea and especially for the idea of being able to mask <i><font color=\"#000000\">after</font></i> the filter has been "
                 "run.</font></b>");

QString attrFilterText = 
  pfmEdit3D::tr ("<img source=\":/icons/attr_filter.xpm\"> Click this button to filter the data points based on a specified attribute value.  "
                 "This is a band pass/fail filter.  That is, points inside/outside of the set range are filtered.<br><br>"
                 "<b>IMPORTANT NOTE: The filter operation will mark all of the points that are to be filtered and display "
                 "a confirmation dialog prior to actually invalidating the filtered points.  Setting the <i>Filter Max</i> slider value less than the "
                 "<i>Filter Min</i> slider value will cause all points <i>inside</i> the range to be marked for invalidation.  Normally, points outside "
                 "the range will be marked for invalidation.<br><br>"
                 "<b><font color=\"#ff0000\">TIP: After running the filter, while the confirmation dialog is visible, you can still mask out any "
                 "points that you don't want to filter and the confirmation dialog will be updated to reflect the change.  You can also adjust the "
                 "filter range values in the confirmation dialog which will re-run the filter.  You can use the sliders in the filter "
                 "confirmation dialog to adjust the min and max range values.  In approximately 1 to 1.5 seconds after you quit adjusting the "
                 "sliders the filter will be re-run.  Pressing the <i>Del</i> key (or whatever hot key that has been set for "
                 "<i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog) while the filter confirmation dialog is present will automatically "
                 "<i>Accept</i> the filter operation.</font></b>");

QString HOFWaveFilterText = 
  pfmEdit3D::tr ("<img source=\":/icons/hof_filter.png\"> Click this button to filter CHARTS HOF data points using a proximity based "
                 "waveform filter.  The filter consists of three passes.  The first pass invalidates selected points that have a low fore "
                 "or back slope.  The second pass uses the search radius combined with the horizontal uncertainty and vertical uncertainty "
                 "to eliminate from filtering any valid points that have a valid point from another line within the Hockey Puck of "
                 "Confidence (TM).  The final pass looks for rising waveforms in both PMT and APD (if applicable) in the waveforms of "
                 "valid or invalid adjacent points (within the radius and Z limits).  The search width defines how far before and after "
                 "the selected bin to search for a rise.  The rise threshold defines how many consecutive rise points are considered "
                 "significant.<br><br>"
                 "<b>IMPORTANT NOTE: This button will turn off slicing, un-hide any hidden data, and clear all masks (except filter masks) "
                 "prior to running the filter.</b>");

QString filterRectMaskText = 
  pfmEdit3D::tr ("<img source=\":/icons/filter_mask_rect.xpm\"> Click this button to define temporary areas to protect (mask) from "
                 "subsequent (or current) filter operations <img source=\":/icons/filter.xpm\">.<br><br>"
                 "After clicking the button the cursor will change to the rectangle mask cursor "
                 "<img source=\":/icons/filter_mask_rect_cursor.xpm\">.  "
                 "Click the left mouse button to define a starting point for the rectangle.  Moving the mouse will draw a rectangle.  "
                 "Left click to end rectangle definition and mask the area.  To abort the operation click the middle mouse button.<br><br>"
                 "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
                 "are viewing these will be discarded.</b>");
QString filterPolyMaskText = 
  pfmEdit3D::tr ("<img source=\":/icons/filter_mask_poly.xpm\"> Click this button to define temporary areas to protect (mask) from "
                 "subsequent (or current) filter operations <img source=\":/icons/filter.xpm\">.<br><br>"
                 "After clicking the button the cursor will change to the polygon mask cursor "
                 "<img source=\":/icons/filter_mask_poly_cursor.xpm\">.  "
                 "Click the left mouse button to define a starting point for a polygon.  Move the mouse to define the polygon to "
                 "be masked.  Left click again to define the last vertex of the polygon.  "
                 "To abort the operation click the middle mouse button.<br><br>"
                 "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
                 "are viewing these will be discarded.</b>");
QString clearMasksText = 
  pfmEdit3D::tr ("<img source=\":/icons/clear_filter_masks.xpm\"> Click this button to clear all currently defined filter masks.");

QString undoText = 
  pfmEdit3D::tr ("<img source=\":/icons/undo.png\"> Click this button to undo the last edit operation.  When there are no edit "
               "operations to be undone this button will not be enabled.");

QString filterBoxText = 
  pfmEdit3D::tr ("This tab page becomes active after running the filter <img source=\":/icons/filter.xpm\"><br><br>"
                 "When this page is active you can adjust the results of the filter by moving the standard deviation "
                 "slider up or down.  You may also change the view of the data to get a better perspective on the points "
                 "that have been selected to be invalidated.  If there are areas of the selection that you do not want "
                 "to invalidate you may use the filter mask buttons, <img source=\":/icons/filter_mask_rect.xpm\"> and/or "
                 "<img source=\":/icons/filter_mask_poly.xpm\">, to mask out those areas.  After you are completely satisfied "
                 "with the final results of the filtering operation you may accept the results by pressing the <b>Accept</b> "
                 "button or, more simply, by just pressing the <b>Del</b> key (or whatever hot key that has been set for "
                 "<i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog).<br><br>"
                 "<b>IMPORTANT TIP: Holding down the <b>Shift</b> key and moving the mouse wheel will change the standard "
                 "deviation value.  Approximately 1 to 1.5 seconds after you stop moving the mouse wheel or release the "
                 "<b>Shift</b> key the filter will be re-run and the new filter results will be displayed.</b>");

QString attrFilterBoxText = 
  pfmEdit3D::tr ("This tab page becomes active after running the attribute filter <img source=\":/icons/attr_filter.xpm\"><br><br>"
                 "When this page is active you can adjust the results of the filter by moving the minimum and maximum range "
                 "sliders up or down.  You may also change the view of the data to get a better perspective on the points "
                 "that have been selected to be invalidated.  If there are areas of the selection that you do not want "
                 "to invalidate you may use the filter mask buttons, <img source=\":/icons/filter_mask_rect.xpm\"> and/or "
                 "<img source=\":/icons/filter_mask_poly.xpm\">, to mask out those areas.  After you are completely satisfied "
                 "with the final results of the filtering operation you may accept the results by pressing the <b>Accept</b> "
                 "button or, more simply, by just pressing the <b>Del</b> key(or whatever hot key that has been set for "
                 "<i>DELETE FILTER HIGHLIGHTED</i> in the Preferences dialog).<br><br>"
                 "<b>IMPORTANT TIP: Setting the minimum slider value to a larger value causes the filter to invalidate points "
                 "<i>inside</i> the range instead of outside the range.</b>");

QString stdSliderText = 
  pfmEdit3D::tr ("Move this slider up to increase the amount of filtering (i.e. decrease the standard deviation value) or "
                 "move it down to decrease the amount of filtering (i.e. increase the standard deviation value).  When the "
                 "slider is released the filter is re-run on the visible data and the number of points to be rejected is "
                 "modified.<br><br>"
                 "<b>IMPORTANT TIP: Holding down the <b>Shift</b> key and moving the mouse wheel will change the standard "
                 "deviation value.  Approximately 1 to 1.5 seconds after you stop moving the mouse wheel or release the "
                 "<Shift> key the filter will be re-run and the new filter results will be displayed.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.</b>");

QString minSliderText = 
  pfmEdit3D::tr ("Move this slider up/down to increase/decrease the minimum value of the range for attribute filtering.  If the value is set to less "
                 "than the maximum slider value, points less than this value will be marked for deletion.  If it is set greater than the "
                 "maximum slider value, points greater than this value but less than the maximum value will be marked for deletion.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");

QString maxSliderText = 
  pfmEdit3D::tr ("Move this slider up/down to increase/decrease the maximum value of the range for attribute filtering.  If the value is set to "
                 "greater than the minimum slider value, points greater than this value will be marked for deletion.  If it is set to less than the "
                 "minimum slider value, points less than this value but greater than the minimum value will be marked for deletion.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");

QString srSliderText = 
  pfmEdit3D::tr ("Move this slider left/right to decrease/increase the search radius used by the hofWaveFilter program when searching for "
                 "nearby points.  Decreasing this value makes the filter more aggressive.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");
QString swSliderText = 
  pfmEdit3D::tr ("Move this slider left/right to decrease/increase the search width used by the hofWaveFilter program when searching for "
                 "rising values on nearby waveforms.  Decreasing this value makes the filter more aggressive.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");
QString rtSliderText = 
  pfmEdit3D::tr ("Move this slider left/right to decrease/increase the rise threshold used by the hofWaveFilter program when searching for "
                 "rising values on nearby waveforms.  Increasing this value makes the filter more aggressive.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");
QString pmtSliderText = 
  pfmEdit3D::tr ("Move this slider left/right to decrease/increase the PMT AC zero offset threshold.  If a point in the HOF waveform "
                 "is not at least this value above the PMT AC zero offset it will be marked as invalid.  Increasing this value makes "
                 "the filter more aggressive.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");
QString apdSliderText = 
  pfmEdit3D::tr ("Move this slider left/right to decrease/increase the APD AC zero offset threshold.  If a point in the HOF waveform "
                 "is not at least this value above the APD AC zero offset it will be marked as invalid.  Increasing this value makes "
                 "the filter more aggressive.<br><br>"
                 "You can also type the value you want into the text box and press <i>Return</i>.");
