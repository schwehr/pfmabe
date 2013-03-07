
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



QString openText = 
  pfmView::tr ("<img source=\":/icons/fileopen.xpm\"> Click this button to open a PFM structure.");
QString closePFMText = 
  pfmView::tr ("Selecting this menu item will close the current top layer PFM file.  All other "
	       "available layers will be moved up by one level.");
QString geotiffText = 
  pfmView::tr ("<img source=\":/icons/geotiffopen.xpm\"> Click this button to open a GeoTIFF file.");

QString importText = 
  pfmView::tr ("You may import DNC hazards, obstructions, lights, and buoys to the associated feature file for the top layer PFM.  "
               "If there is no associated feature file then one will be created.  You must select a DNC directory "
               "(not a file) to be imported.  Features will be created with a remark that they were derived from "
               "a DNC.  The feature search string (in the preferences dialog <img source=\":/icons/prefs.xpm\">) "
               "will be set to <b>dnc</b> so that these features will be shown in the <b>Highlight</b> color.");

QString editRectText = 
  pfmView::tr ("<img source=\":/icons/edit_rect.xpm\"> Click this button to switch the default mode to rectangle "
               "edit.<br><br>"
               "After clicking the button the cursor will change to the edit rectangle cursor "
               "<img source=\":/icons/edit_rect_cursor.xpm\">.  Click the left mouse button to define a starting point "
               "for a rectangle.  Move the mouse to define the edit bounds.  Left click again to begin the edit "
               "operation.  To abort the operation click the middle mouse button.<br><br> "
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or polygon edit "
               "<img source=\":/icons/edit_poly.xpm\">) is selected.</b>");
QString editPolyText = 
  pfmView::tr ("<img source=\":/icons/edit_poly.xpm\"> Click this button to switch the default mode to polygon "
               "edit.<br><br>"
	       "After clicking the button the cursor will change to the edit polygon cursor "
               "<img source=\":/icons/edit_poly_cursor.xpm\">.  Click the left mouse button to define a starting point "
               "for a polygon.  Moving the mouse will draw a line.  Left click to define the next vertex of the polygon.  "
               "Double click to define the last vertex of the polygon and begin the edit operation.  "
	       "To abort the operation click the middle mouse button.<br><br> "
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");
QString filterRectText = 
  pfmView::tr ("<img source=\":/icons/filter_rect.xpm\"> Click this button to switch the default mode to rectangle "
               "filter.<br><br>"
	       "After clicking the button the cursor will change to the filter rectangle cursor "
               "<img source=\":/icons/filter_rect_cursor.xpm\">.  Click the left mouse button to define a starting point for a "
               "rectangle.  Moving the mouse will draw a rectangle.  Left click to end rectangle definition and begin the "
               "filter operation.  To abort the operation click the middle mouse button.  There are two values that affect "
               "the filter, they are the standard deviation value and the <b>deep filter only</b> flag.  When filtering, standard "
               "deviation values are computed for each bin and the surrounding 8 bins.  If the difference between an "
               "individual depth and the computed average surface (using all 9 bins) exceeds the computed standard "
               "deviation times the number of filter standard deviations allowed in either the upward or downward "
               "direction then that point will be filtered out.  If the <b>deep filter only</b> flag is set then only "
               "values that exceed the standard deviation in the downward (positive) direction will be filtered out.  "
               "These values may be set in the preferences dialog <img source=\":/icons/prefs.xpm\"> or using the right "
               "click popup menu.<br><br>"
               "<b>IMPORTANT NOTE: The filter invalidates actual sounding data in the PFM structure (just like deleting "
               "in the editor).  Data within the <i>Feature exclusion radius</i> of a feature will not be "
	       "filtered.  This value is set in the <i>Preferences</i> dialog <img source=\":/icons/prefs.xpm\">.  "
	       "In addition, data within a feature polygon <i>whose attached feature position is within the filter "
	       "area</i> will not be filtered.  If you don't include the attached feature in the filter area then the "
	       "feature polygon will be ignored.  Also, you may mask areas using either the Rectangular Filter Mask "
               "<img source=\":/icons/filter_mask_rect.xpm\"> button or the Polygonal Filter Mask "
               "<img source=\":/icons/filter_mask_poly.xpm\"> button.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");
QString filterPolyText = 
  pfmView::tr ("<img source=\":/icons/filter_poly.xpm\"> Click this button to switch the default mode to polygon "
               "filter.<br><br>"
	       "After clicking the button the cursor will change to the filter polygon cursor "
               "<img source=\":/icons/filter_poly_cursor.xpm\">.  Click the left mouse button to define a starting point for a "
               "polygon.  Moving the mouse will draw a line.  Left click to define the next vertex of the polygon.  "
               "Double click to define the last vertex of the polygon and begin the filter operation.  "
	       "To abort the operation click the middle mouse button.  There are two values that affect the filter, they "
               "are the standard deviation value and the <b>deep filter only</b> flag.  When filtering, standard "
               "deviation values are computed for each bin and the surrounding 8 bins.  If the difference between an "
               "individual depth and the computed average surface (using all 9 bins) exceeds the computed standard "
               "deviation times the number of filter standard deviations allowed in either the upward or downward "
               "direction then that point will be filtered out.  If the <b>deep filter only</b> flag is set then only "
               "values that exceed the standard deviation in the downward (positive) direction will be filtered out.  "
               "These values may be set in the preferences dialog <img source=\":/icons/prefs.xpm\"> or using the right "
               "click popup menu.<br><br>"
               "<b>IMPORTANT NOTE: The filter invalidates actual sounding data in the PFM structure (just like deleting "
               "in the editor).  Data within the <i>Feature exclusion radius</i> of a feature will not be "
	       "filtered.  This value is set in the <i>Preferences</i> dialog <img source=\":/icons/prefs.xpm\">.  "
	       "In addition, data within a feature polygon <i>whose attached feature position is within the filter "
	       "area</i> will not be filtered.  If you don't include the attached feature in the filter area then the "
	       "feature polygon will be ignored.  Also, you may mask areas using either the Rectangular Filter Mask "
               "<img source=\":/icons/filter_mask_rect.xpm\"> button or the Polygonal Filter Mask "
               "<img source=\":/icons/filter_mask_poly.xpm\"> button.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");
QString filterRectMaskText = 
  pfmView::tr ("<img source=\":/icons/filter_mask_rect.xpm\"> Click this button to define temporary areas to protect (mask) from "
               "subsequent filter operations <img source=\":/icons/filter_rect.xpm\">.<br><br>"
	       "After clicking the button the cursor will change to the rectangle mask cursor "
               "<img source=\":/icons/filter_mask_rect_cursor.xpm\">.  "
               "Click the left mouse button to define a starting point for the rectangle.  Moving the mouse will draw a rectangle.  "
               "Left click to end rectangle definition and mask the area.  To abort the operation click the middle mouse button.<br><br>"
               "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
               "are viewing these will be discarded.</b>");
QString filterPolyMaskText = 
  pfmView::tr ("<img source=\":/icons/filter_mask_poly.xpm\"> Click this button to define temporary areas to protect (mask) from "
               "subsequent filter operations <img source=\":/icons/filter_poly.xpm\">.<br><br>"
	       "After clicking the button the cursor will change to the polygon mask cursor "
               "<img source=\":/icons/filter_mask_poly_cursor.xpm\">.  "
               "Click the left mouse button to define a starting point for a polygon.  Moving the mouse will draw a line.  "
               "Left click to define the next vertex of the polygon.  Double click to define the last vertex of the polygon.  "
               "To abort the operation click the middle mouse button.<br><br>"
               "<b>IMPORTANT NOTE: The filter mask areas are only temporary.  If you change the area of the PFM that you "
               "are viewing these will be discarded.</b>");
QString clearMasksText = 
  pfmView::tr ("<img source=\":/icons/clear_filter_masks.xpm\"> Click this button to clear all currently defined filter masks.  "
               "The masks will also be cleared if you change the area you are viewing.");
QString addFeatureText = 
  pfmView::tr ("<img source=\":/icons/addfeature.xpm\"> Click this button to add a feature to the top layer PFM.  When selected the cursor will "
               "become a crosshair.  Left clicking on the display will add a feature at the shoalest point within 20 meters of "
               "the crosshair.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like edit feature <img source=\":/icons/editfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString deleteFeatureText = 
  pfmView::tr ("<img source=\":/icons/deletefeature.xpm\"> Click this button to enter delete feature mode.  When selected "
               "the cursor will become the delete feature cursor <img source=\":/icons/delete_feature_cursor.xpm\">.  As "
               "the cursor is moved the box cursor will snap to the nearest feature.  Left clicking will delete the feature "
               "under the box cursor.  Actually what happens is that a confidence value of zero is assigned to the "
               "selected feature.  Features with a confidence value of zero are visible when viewing any of the unedited "
               "surfaces.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString editFeatureText = 
  pfmView::tr ("<img source=\":/icons/editfeature.xpm\"> Click this button to enter edit feature mode.  When selected the "
               "cursor will become the edit feature cursor <img source=\":/icons/edit_feature_cursor.xpm\">.  As the cursor "
               "is moved the box cursor will snap to the nearest feature.  Left clicking will bring up the edit feature "
               "dialog.<br><br>"
               "<b>Cool feature: You can right click and select <i>Highlight features (text search)</i> to highlight "
               "features where the description or remarks contain the text string (verbatim, case insensitive).  This "
               "can be handy for finding all of the features on screen that are of a certain type.</b><br><br>" 
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString remispText = 
  pfmView::tr ("<img source=\":/icons/remisp_icon.xpm\"> Click this button to switch the default mode to "
               "<b>regrid rectangular area</b>.<br><br>"
               "After clicking the button the cursor will change to the regrid cursor "
               "<img source=\":/icons/remisp_cursor.xpm\">.  Click the left mouse button to define a starting point for a "
               "rectangle.  Move the mouse to define the edit bounds.  Left click again to begin the regrid operation.  "
               "To abort the operation click the middle mouse button.  This button is only active if you have run "
               "pfmMisp to create a MISP grid surface in the PFM structure and you selected the <b>Replace all bins</b> "
               "option.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString remispFilterText = 
  pfmView::tr ("<img source=\":/icons/remisp_filter_icon.xpm\"> Click this button to switch the default mode to remisp/filter "
               "mode.<br><br>"
	       "After clicking the button the cursor will change to the filter polygon cursor "
               "<img source=\":/icons/filter_poly_cursor.xpm\">.  Click the left mouse button to define a starting point for a "
               "polygon.  Moving the mouse will draw a line.  Left click to define the next vertex of the polygon.  "
               "Double click to define the last vertex of the polygon and begin the filter operation.  "
	       "To abort the operation click the middle mouse button.<br><br>"
               "<b>IMPORTANT NOTE: The filter invalidates actual sounding data in the PFM structure (just like deleting "
               "in the editor).  The way this filter works is that contours drawn in the <b>draw contour for filtering </b> "
               "mode will be used to generate a surface within this polygon.  The actual data within the polygon will be "
               "compared with the surface and one (and only one) data point that is nearest to the surface value in the "
               "bin will be left as valid.  All other points will be marked invalid.  If there are no points in the bin within "
               "the contour filter envelope (that can be set in preferences dialog <img source=\":/icons/prefs.xpm\">) then all "
               "of the data points in the bin will be marked as invalid.  After the data is filtered the area will be remisp'ed.</b><br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString drawContourText = 
  pfmView::tr ("<img source=\":/icons/draw_contour.xpm\"> Click this button to switch the default mode to <b>draw "
               "contour</b>.  This will allow you to draw a contour which can then be placed into the PFM structure.<br><br>"
               "There are three ways to define the depth value that you wish to draw as a contour.  The easiest is to "
               "move the draw contour cursor <img source=\":/icons/draw_contour_cursor.xpm\"> near a contour and click "
               "the center mouse button to grab the value of the nearest contour.  You may also use the <b>Grab nearest "
               "contour depth for contour</b> menu entry from the right click popup menu to grab the nearest contour "
               "value.  The third method is to use the <b>Set depth for contour</b> menu entry from the right click popup "
               "menu to manually set the value.  Once you have set the contour value, contours of exactly that value that "
               "are displayed on the screen will be redrawn highlighted in the current display.  The chosen value will "
               "be displayed in the status bar at the bottom of the window.  Note that manually entering a value that "
               "is not an exact contour value will not cause any contour highlighting.  As long as you are in <b>draw "
               "contour</b> mode the chosen contour level will be highlighted whenever it is redrawn.<br><br>"
               "At this point you may move the cursor to the desired location and left click/release to begin drawing the "
               "contour.   To end the contour you may left click/release again.  If, at any time during the operation, "
               "you wish to discard the contour and abort the operation simply click the middle mouse button or right "
               "click and select the appropriate menu option.  This button is only active if you have run "
               "pfmMisp to create a MISP grid surface in the PFM structure and you selected the <b>Replace all bins</b> "
               "option.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b><br><br>"
               "<b>IMPORTANT NOTE : Drawn contours will not place data in cells containing already existing, valid "
               "data.  If you want to draw contours through existing, valid data you must first invalidate that data.");

QString grabContourText = 
  pfmView::tr ("<img source=\":/icons/grab_contour.xpm\"> Click this button to switch the default mode to <b>capture "
               "interpolated contour</b> mode.  This will allow you to define a polygon to capture the interpolated "
               "contours and paste the contour value into the PFM file.  You can use this to avoid having to hand draw "
               "contours through an area in which you feel that the interpolated contours are correct.<br><br>"
	       "After clicking the button the cursor will change to the capture contour cursor "
               "<img source=\":/icons/grab_contour_cursor.xpm\">.  Click the left mouse button to define a starting point "
               "for a polygon.  Moving the mouse will draw a line.  Left click to define the next vertex of the polygon.  "
               "Double click to define the last vertex of the polygon and begin the capture operation.  "
	       "To abort the operation click the middle mouse button.<br><br> "
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b>");

QString drawContourFilterText = 
  pfmView::tr ("<img source=\":/icons/draw_contour_filter.xpm\"> Click this button to switch the default mode to <b>draw "
               "contour for filtering</b>.  This will allow you to draw one or more contours that will be used to define "
               "a surface for filtering the underlying PFM data.  There are three ways to define the depth value that you "
               "wish to draw as a contour.  The easiest is to move the draw contour cursor "
               "<img source=\":/icons/draw_contour_cursor.xpm\"> near a contour and click "
               "the center mouse button to grab the value of the nearest contour.  You may also use the <b>Grab nearest "
               "contour depth for contour</b> menu entry from the right click popup menu to grab the nearest contour "
               "value.  The third method is to use the <b>Set depth for contour</b> menu entry from the right click popup "
               "menu to manually set the value.  Once you have set the contour value, contours of exactly that value that "
               "are displayed on the screen will be redrawn highlighted in the current display.  The chosen value will "
               "be displayed in the status bar at the bottom of the window.  Note that manually entering a value that "
               "is not an exact contour value will not cause any contour highlighting.  As long as you are in <b>draw "
               "contour for filtering</b> mode the chosen contour level will be highlighted whenever it is redrawn.<br><br>"
               "At this point you may move the cursor to the desired location and left click/release to begin drawing the "
               "contour.   To end the contour you may left click/release again.  If, at any time during the operation, "
               "you wish to discard the contour and abort the operation simply click the middle mouse button or right "
               "click and select the appropriate menu option.  This button is only active if you have run "
               "pfmMisp to create a MISP grid surface in the PFM structure and you selected the <b>Replace all bins</b> "
               "option.<br><br>"
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or rectangle edit "
               "<img source=\":/icons/edit_rect.xpm\">) is selected.</b><br><br>"
               "<b>IMPORTANT NOTE : In order to use the contours that were drawn you must use the remisp/filter mode "
               "<img source=\":/icons/remisp_filter_icon.xpm\"> to perform the remisp and filtering.  The contours will remain "
               " in place until you do this, until you press this button again, or until you clear the contours with the "
               "clear filter contours button <img source=\":/icons/clear_filter_contours.xpm\">.</b>");

QString clearFilterContoursText = 
  pfmView::tr ("<img source=\":/icons/clear_filter_contours.xpm\"> Click this button to clear any contours that were drawn "
               "to be used in the contour filter mode <img source=\":/icons/draw_contour_filter.xpm\">");

QString zoomOutText = 
  pfmView::tr ("<img source=\":/icons/icon_zoomout.xpm\"> Click this button to zoom out one level.");
QString zoomInText = 
  pfmView::tr ("<img source=\":/icons/icon_zoomin.xpm\"> Click this button to define a rectangular zoom area.  "
               "After clicking the button the cursor will change to the zoomCursor "
               "<img source=\":/icons/zoom_cursor.xpm\">.  Click the left mouse button to define a starting point for a "
               "rectangle.  Move the mouse to define the zoom in bounds.  Left click again to finish the operation.  To "
               "abort the zoom in operation click the middle mouse button.<br><br> "
               "<b>NOTE : This is a momentary button.  After zooming in or clicking the middle mouse button "
               "the active function will revert to the last edit function such as rectangular edit mode "
               "<img source=\":/icons/edit_rect.xpm\"></b>");
QString redrawText = 
  pfmView::tr ("<img source=\":/icons/redraw.xpm\"> Click this button to redraw the displayed area.");
QString autoRedrawText = 
  pfmView::tr ("<img source=\":/icons/autoredraw.xpm\"> Click this button to force a redraw of the edited area "
               "after each edit operation.");

QString redrawCoverageText = 
  pfmView::tr ("<img source=\":/icons/redrawcoverage.xpm\"> Click this button to redraw the coverage map.");
QString autoRedrawCoverageText = 
  pfmView::tr ("<img source=\":/icons/autoredrawcoverage.xpm\"> Click this button to force a redraw of the "
               "entire coverage map whenever the boundaries of the displayed area are changed.");

QString setCheckedText = 
  pfmView::tr ("<img source=\":/icons/setchecked.xpm\"> Click this button to access a popup menu allowing you to set the "
               "bins in the displayed area of all of the PFM layers to <b>checked</b>, <b>unchecked</b>, <b>verified</b>, or <b>unverified</b>.");

QString displaySuspectText = 
  pfmView::tr ("<img source=\":/icons/displaysuspect.xpm\"> Click this button to flag bins that contain data "
               "that is marked as suspect.");
QString displayFeatureText = 
  pfmView::tr ("<img source=\":/icons/display_all_feature.png\"> Click this button to select the feature display mode.  "
               "You can highlight features that have descriptions or remarks containing specific text strings by setting "
               "the feature search string in the preferences dialog <img source=\":/icons/prefs.xpm\">.");
QString displayChildrenText = 
  pfmView::tr ("<img source=\":/icons/displayfeaturechildren.xpm\"> Click this button to display feature sub-records.  "
               "Feature sub-records are features that have been grouped under a master feature record.  Features "
               "can be grouped and un-grouped in the edit feature dialog.");
QString displayFeatureInfoText = 
  pfmView::tr ("<img source=\":/icons/displayfeatureinfo.xpm\"> Click this button to write the description and "
               "remarks fields of features next to any displayed features.  This button is "
               "meaningless if <b>Flag Feature Data</b> is set to not display features "
               "<img source=\":/icons/display_no_feature.png\">.");
QString displayFeaturePolyText = 
  pfmView::tr ("<img source=\":/icons/displayfeaturepoly.xpm\"> Click this button to draw any associated polygonal "
               "areas with the displayed features.  This button is meaningless if <b>Flag Feature Data</b> is set to not display features "
               "<img source=\":/icons/display_no_feature.png\">.");
QString displaySelectedText = 
  pfmView::tr ("<img source=\":/icons/displayselected.xpm\"> Click this button to flag bins that contain "
               "soundings that are marked as selected.");
QString displayReferenceText = 
  pfmView::tr ("<img source=\":/icons/displayreference.xpm\"> Click this button to flag bins that contain data "
               "that is marked as reference data.");
QString displayMinMaxText = 
  pfmView::tr ("<img source=\":/icons/display_min_max.xpm\"> Click this button to flag the minimum, maximum, and "
               "max standard deviation bins.  Bins will be marked with a circle for the minimum, a "
               "square for the maximum, and a diamond for the max standard deviation.");

QString unloadText = 
  pfmView::tr ("<img source=\":/icons/unload.xpm\"> Click this button to bring up a menu to allow you to unload "
               "back to the original input files all edits that have been done in the displayed area or the entire "
               "PFM.  Note that if you have more than one PFM open only the top layer PFM will be unloaded.");

QString prefsText = 
  pfmView::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  This "
               "includes colors, contour intervals, contour smoothing factor, scale factor, "
               "sunshading options, coastline options, and position display format.");



QString stopText = 
  pfmView::tr ("<img source=\":/icons/stop.xpm\"> Click this button to cancel drawing of the surface.");

QString highlightText = 
  pfmView::tr ("Click this button to highlight parts of the display depending on data type or various data conditions.  "
	       "This button may display any of the following icons depending on the option chosen:"
	       "<ul>"
	       "<li><img source=\":/icons/highlight_none.xpm\">  No highlighting</li>"
	       "<li><img source=\":/icons/highlight_all.xpm\">  Highlight all data (essentially a gray scale display)</li>"
	       "<li><img source=\":/icons/highlight_checked.xpm\">  Highlight data that is set to checked or verified</li>"
	       "<li><img source=\":/icons/highlight01.xpm\">  Highlight cells containing PFM_USER_01 flagged data</li>"
	       "<li><img source=\":/icons/highlight02.xpm\">  Highlight cells containing PFM_USER_02 flagged data</li>"
	       "<li><img source=\":/icons/highlight03.xpm\">  Highlight cells containing PFM_USER_03 flagged data</li>"
	       "<li><img source=\":/icons/highlight04.xpm\">  Highlight cells containing PFM_USER_04 flagged data</li>"
	       "<li><img source=\":/icons/highlight05.xpm\">  Highlight cells containing PFM_USER_05 flagged data</li>"
	       "<li><img source=\":/icons/highlight_int.xpm\">  Highlight cells that have only interpolated (MISP) data</li>"
	       "<li><img source=\":/icons/highlight_mult.xpm\">  Highlight cells containing valid or invalid data from more than one line</li>"
	       "<li><img source=\":/icons/highlight_count.xpm\">  Highlight cells that contain more valid points than the set value</li>"
	       "<li><img source=\":/icons/highlight_IHO_S.xpm\">  Highlight cells with points that might not meet IHO Special Order criteria</li>"
	       "<li><img source=\":/icons/highlight_IHO_1.xpm\">  Highlight cells with points that might not meet IHO Order 1 criteria</li>"
	       "<li><img source=\":/icons/highlight_IHO_2.xpm\">  Highlight cells with points that might not meet IHO Order 2 criteria</li>"
	       "<li><img source=\":/icons/highlight_percent.xpm\">  Highlight cells with points that fall outside of the stated percentage of depth</li>"
	       "</ul>"
	       "<br>Notes: The IHO and percentage options compare each point in a bin with the average edited depth "
	       "of the bin to determine if any of the points fall outside of the selected IHO order or percentage of depth.  "
	       "The IHO highlighting does not include vertical error information nor does it take in to account slope in the "
	       "bin.  As such, unhighlighted areas should not be construed to meet the selected IHO criteria.  "
	       "This is merely a tool used to indicate where there <b><i>might</i></b> be data problems.  The IHO options will "
	       "only work between 0.0 and 200.0 meters while the percentage option will only work in "
	       "depths greater than 20.0 meters.  In order for these to have any meaning the data must have been "
	       "corrected to the final datum or an approximation thereof.  Percentage of depth is mostly useful "
	       "for deep water and it is usually set to the sonar system's stated accuracy.  For example, the Simrad "
	       "EM121A is supposed to be accurate to 0.10 percent of depth.  The percentage value and/or the valid bin count value "
               "can be changed in the preferences <img source=\":/icons/prefs.xpm\"> dialog.<br>"
	       "<b>Important note: The options below the menu separator force the program to read all of the actual "
	       "data points (not just the bins).  This can take quite a bit of time depending on the amount of data in "
	       "the displayed area of the PFM.</b>");

QString stoplightText = 
  pfmView::tr ("<img source=\":/icons/stoplight.xpm\"> Click this button to toggle display using stoplight colors.  "
               "Stoplight colors can be defined in the preferences dialog <img source=\":/icons/stoplight.xpm\">.");

QString contourText = 
  pfmView::tr ("<img source=\":/icons/contour.xpm\"> Click this button to toggle drawing of contours.");

QString sunshadingText = 
  pfmView::tr ("<img source=\":/icons/sunshading.xpm\"> Click this button to toggle between sunshaded and "
               "non-sunshaded views.");

QString sunangleText = 
  pfmView::tr ("<img source=\":/icons/sunangle.xpm\"> Click this button to set the sun angle.  After clicking "
               "the button, an arrow will appear that points toward the center of the display area.  "
               "Move the mouse to line up the arrow in the desired direction and then left click to apply the new "
               "sun angle.  The sun angle will be displayed in the status bar (far right) when moving the mouse.  To "
               "abort the operation click the center button.<br><br> "
               "<b>IMPORTANT NOTE : This is a momentary button.  After clicking the right or middle mouse button "
               "the active function will revert to the last edit function such as rectangular edit mode "
               "<img source=\":/icons/edit_rect.xpm\"></b>");

QString coastText = 
  pfmView::tr ("<img source=\":/icons/coast.xpm\"> Click this button to toggle the display of coastline.");

QString maskText = 
  pfmView::tr ("<img source=\":/icons/landmask.xpm\"> Click this button to toggle the display of the landmask on and "
               "off.  Note that the landmask will only be plotted if the displayed area is less than 5 degrees in both "
               "latitude and longitude.  The SRTM_DATA environment variable must also be set to point to the "
               "directory containing the srtm_mask.clm file.");

QString avgFiltText = 
  pfmView::tr ("<img source=\":/icons/avgfilt.xpm\"> Click this button to display a surface made up of the "
               "average of all of the edited (valid) points in the bins.<br><br>"
               "<b>IMPORTANT NOTE: If the button is the CUBE surface button, <img source=\":/icons/cube_surface.png\">, "
               "the displayed surface will not be the average surface, it will be the CUBE surface.");
QString minFiltText = 
  pfmView::tr ("<img source=\":/icons/minfilt.xpm\"> Click this button to display a surface made up of the "
               "minimum edited (valid) point in the bins.");
QString maxFiltText = 
  pfmView::tr ("<img source=\":/icons/maxfilt.xpm\"> Click this button to display a surface made up of the "
               "maximum edited (valid) point in the bins.");
QString avgText = 
  pfmView::tr ("<img source=\":/icons/avg.xpm\"> Click this button to display a surface made up of an average of "
               "all of the points (valid and invalid) in the bins.");
QString minText = 
  pfmView::tr ("<img source=\":/icons/min.xpm\"> Click this button to display a surface made up of the minimum "
               "(valid or invalid) point in the bins.");
QString maxText = 
  pfmView::tr ("<img source=\":/icons/max.xpm\"> Click this button to display a surface made up of the maximum "
               "(valid or invalid) point in the bins.");
QString minFiltOtfText = 
  pfmView::tr ("<img source=\":/icons/minfilt_otf.xpm\"> Click this button to display a surface made up of the "
               "minimum edited (valid) point in on-the-fly gridded bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString maxFiltOtfText = 
  pfmView::tr ("<img source=\":/icons/maxfilt_otf.xpm\"> Click this button to display a surface made up of the "
               "maximum edited (valid) point in the bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString avgFiltOtfText = 
  pfmView::tr ("<img source=\":/icons/avgfilt_otf.xpm\"> Click this button to display a surface made up of the "
               "average edited (valid) data in the bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString minOtfText = 
  pfmView::tr ("<img source=\":/icons/min_otf.xpm\"> Click this button to display a surface made up of the "
               "minimum edited or unedited (valid or manually invalid) point in on-the-fly gridded bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString maxOtfText = 
  pfmView::tr ("<img source=\":/icons/max_otf.xpm\"> Click this button to display a surface made up of the "
               "maximum edited or unedited (valid or manually invalid) point in the bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString avgOtfText = 
  pfmView::tr ("<img source=\":/icons/avg_otf.xpm\"> Click this button to display a surface made up of the "
               "average edited or unedited (valid or invalid) data in the bins.  This surface will be binned on-the-fly into a grid "
               "that has bin sizes defined in the <b>Preferences</b> dialog <img source=\":/icons/prefs.xpm\"><br>"
               "or by using the <b>Set on-the-fly gridding bin size</b> button <img source=\":/icons/set_otf_size.xpm\">.  "
               "You can color OTF bins by attribute as long as you only select the number, standard deviation, "
               "avg minus min, max minus avg, or max minus min attribute.<br><br>"
               "<b>IMPORTANT NOTE: When you use on-the-fly gridding the display of contours, GeoTIFFs, and "
               "highlighting will be turned off.  The purpose of the on-the-fly gridding is to allow you to see "
               "discrepencies in the minimum, maximum, or average surfaces that may not be visible at larger or smaller "
               "bin sizes.  To do this you really don't want any contours or GeoTIFFs messing up your view.  The "
               "highlighting is turned off because most of that deals with information in the PFM bins.<br><br>"
               "OTHER IMPORTANT NOTE: Scenarios in which you might want to use on-the-fly gridding:<br>"
               "<ul>"
               "<li>You have a very large area PFM at a larger bin size (in order to handle it more efficiently) but "
               "you believe there might be discrepencies in the data that you can't see at the larger bin size.</li>"
               "<li>You have an area that is mostly deep water so you have used a larger bin size (to get enough points "
               "for a full surface) but there is a small part of the area that is shallow.  You can zoom in to the "
               "shallow area and then use on-the-fly gridding at a smaller bin size to see the detail in the surfaces.</li>"
               "<li>You have an area that is mostly shallow water so you have used a smaller bin size (to get better "
               "detail in the surfaces) but you have a small part of the area that is deep.  You can zoom in to the "
               "deep area and then use on-the-fly gridding at a larger bin size to get a full surface so you can "
               "see any outliers.</li>"
               "</ul><br>");
QString setOtfBinText = 
  pfmView::tr ("<img source=\":/icons/set_otf_size.xpm\"> Click this button to change the on-the-fly bin size.");
QString verifyFeaturesText = 
  pfmView::tr ("<img source=\":/icons/verify_features.png\"> Click this button to set all valid, displayed features to a confidence "
	       "level of 5.  These will be displayed in a different color in the editors and in pfmView.  The only way to <b><i>unverify</i></b> "
	       "a feature is to edit the feature and manually set the confidence level to a lower value than 5.");
QString geoText = 
  pfmView::tr ("<img source=\":/icons/geo_off.xpm\"> This is a tristate button.  Click it to change the way GeoTIFFs "
               "are displayed.  The three states are:<br><br>"
               "<img source=\":/icons/geo_off.xpm\"><br><br>"
               "GeoTIFF will not be displayed.<br><br>"
               "<img source=\":/icons/geo.xpm\"><br><br>"
               "GeoTIFF will be displayed over the data with the transparency set in the preferences dialog.<br><br>"
               "<img source=\":/icons/geo_under.xpm\"><br><br>"
               "GeoTIFF will be displayed under the data.");


QString attrText = 
  pfmView::tr ("Click this button access a pulldown menu allowing you to color the displayed surface by the value of the "
	       "associated attribute.  This button will display one of the following icons when not coloring by depth:<br><br>"
               "<ul>"
               "<li>number of soundings icon <img source=\":/icons/number.xpm\"></li>"
               "<li>standard deviation icon <img source=\":/icons/stddev.xpm\"></li>"
               "<li>average minus minimum icon <img source=\":/icons/avg_min_diff.xpm\"></li>"
               "<li>maximumn minus average icon <img source=\":/icons/max_avg_diff.xpm\"></li>"
               "<li>maximum minus minimum icon <img source=\":/icons/max_min_diff.xpm\"></li>"
               "<li>bin attribute 1 icon <img source=\":/icons/attr01.xpm\"></li>"
               "<li>bin attribute 2 icon <img source=\":/icons/attr02.xpm\"></li>"
               "<li>bin attribute 3 icon <img source=\":/icons/attr03.xpm\"></li>"
               "<li>bin attribute 4 icon <img source=\":/icons/attr04.xpm\"></li>"
               "<li>bin attribute 5 icon <img source=\":/icons/attr05.xpm\"></li>"
               "<li>bin attribute 6 icon <img source=\":/icons/attr06.xpm\"></li>"
               "<li>bin attribute 7 icon <img source=\":/icons/attr07.xpm\"></li>"
               "<li>bin attribute 8 icon <img source=\":/icons/attr08.xpm\"></li>"
               "<li>bin attribute 9 icon <img source=\":/icons/attr09.xpm\"></li>"
               "<li>bin attribute 10 icon <img source=\":/icons/attr10.xpm\"></li>"
               "</ul><br><br>"
               "If you are coloring by depth you will see the color by depth icon <img source=\":/icons/color_by_depth.xpm\">.");

QString covMenuText = 
  pfmView::tr ("You may select an area to view by clicking and dragging on the Coverage Map with the left mouse "
               "button.  After defining the area, click the left mouse button again and the binned data will be "
               "displayed in the main window.  To discard the area click the middle mouse button or right click to "
               "select an option from the menu.  You may drag a pre-existing rectangle using <b>Ctrl-left click</b> or "
               "by selecting <b>Drag rectangle</b> from the right click popup menu, clicking the open hand cursor on or "
               "near the existing rectangle, dragging the new rectangle to the area of interest, and then clicking again "
               "to display the new area.  As with most other operations, you can discard the operation by clicking the "
               "middle mouse button.");

QString covText = 
  pfmView::tr ("This is the coverage map.  Areas that do not contain data will be displayed in white.  Areas with data "
               "will be displayed as black, gray, or green depending on the state of the data.  Gray areas represent "
               "data that has been <b>checked</b>.  Green areas represent data that has been <b>verified</b>.  Black "
               "areas represent data that has been neither <b>checked</b> nor <b>verified</b>.<br><br>"
               "The PFM area boundary (which may not be a rectangle) will be displayed in semi-transparent green.  "
               "If you are displaying multiple PFM layers then only the top level PFM boundary will be displayed in "
               "semi-transparent green.  The other PFM boundaries will be displayed in semi-transparent red.<br><br>") +
               covMenuText;

QString pfmLabelText = 
  pfmView::tr ("This is the name of the PFM layer that the cursor is moving over in the main window or the coverage "
               "map.  If multiple PFM layers are being displayed and there are overlapping areas then this will be "
               "the name of the topmost PFM layer.");

QString mapText = 
  pfmView::tr ("The pfmView program is used to display geographically binned data that is stored in a PFM structure.  "
               "Loading a file is as simple as clicking on the open file button "
               "<img source=\":/icons/fileopen.xpm\"> and then selecting a PFM file.  "
               "After doing this the coverage will be displayed in the Coverage Map.  You may select an area to view "
               "by clicking and dragging on the Coverage Map with the left mouse button.  After defining "
               "the area, click the left mouse button again and the binned data will be displayed in the main "
               "window.  You may change the area being viewed by either defining a new area in the coverage "
               "map or pressing one of the arrow/keypad keys.  If you use the arrow/keypad keys there will be a five "
               "percent overlap between the new window and the old window.  PgUp, PgDn, Home, and End move the map in the "
               "coresponding diagonal directions.  Context sensitive help is available for all actions by using the What's This button "
               "<img source=\":/icons/contextHelp.xpm\">.<br><br>"
               "The default action for the program is to start the edit process either in rectangle "
               "<img source=\":/icons/edit_rect.xpm\"> or polygon <img source=\":/icons/edit_poly.xpm\"> edit mode.  In "
               "rectangle mode place the cursor on the upper left corner of the area to be edited.  The left mouse "
               "button should be pressed to start the area definition.  Moving the cursor in any "
               "direction will cause a rubber-band rectangle to appear.  The rubber-band rectangle "
               "should be expanded to the desired size.  When the rectangle is correct, press the left "
               "mouse button to edit the data for the defined area of the bin file.  To discard the "
               "edit operation, press the middle mouse button (left and right buttons together on a "
               "two-button mouse).  In polygon mode start "
               "defining a polygon by clicking in the viewing area.  A rubber-band line will stretch "
               "from the selected location.  When the next desired position is reached, click the left "
               "mouse button again to anchor the next point.  When you have defined the last point in "
               "the polygon you can double-clicking the left mouse button to close the polygon.  Discard the "
               "polygon by clicking the middle mouse button.  After the left mouse button is pressed "
               "in rectangle mode or double-clicked in polygon mode the pfmEdit program will be run for the selected "
               "rectangular or polygonal area.<br><br>"
               "When the user exits the pfmEdit program the pfmView window will be redrawn if there "
               "were any changes to the underlying data, the binned surface being viewed is a "
               "<b>filtered</b> surface, and the auto redraw <img source=\":/icons/autoredraw.xpm\"> button is "
               "selected.<br><br>");

QString pfmViewAboutText = 
  pfmView::tr ("<center><br>pfmView<br><br>"
               "Author : Jan C. Depner (depnerj@navo.navy.mil)<br>"
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
               "files overlaps.  Also, there is a hook to a GeoTIFF mosaic file that can contain "
               "sidescan or photo mosaic data.  pfmView is a very simple Qt binned surface viewer.  After "
               "all of the editing is finished, the status information can be loaded back into the "
               "original raw input data files.<br><br>"
               "The author (Evil Twin) would like to acknowledge the contributions to the PFM design "
               "of the usual band of suspects:"
               "<ul>"
               "<li>Commissioner Gordon - Jim Hammack, NAVO</li>"
               "<li>Fabio               - Dave Fabre, Neptune Sciences, Inc.</li>"
               "<li>Oh Yes              - Becky Martinolich, NAVO</li>"
               "</ul><br><br><br>"
               "<center>Jan 'Evil Twin' Depner<br>"
               "<a href=\"http://en.wikipedia.org/wiki/Naval_Oceanographic_Office\">Naval Oceanographic Office</a><br>"
               "jan.depner@navy.mil<br></center>");

QString acknowledgementsText = 
  pfmView::tr ("<center><br>pfmView was built using some, if not all, of the following Open Source libraries:"
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
  pfmView::tr ("This area displays the currently active mode or information about the operation in progress.");

QString statusBarText = 
  pfmView::tr ("The status bar is used to display the progress of the current operation.");

QString attrStatusText = 
  pfmView::tr ("This status bar displays information about the bin attributes that are associated with each bin.");

QString progStatusText = 
  pfmView::tr ("The status bar is used to display the progress of the current operation.");

QString fileCheckedText = 
  pfmView::tr ("This menu entry sets the <b><i>entire file for each displayed PFM layer</i></b> to CHECKED.  For a large file(s) this "
               "can take a fairly long time.");

QString fileUncheckedText = 
  pfmView::tr ("This menu entry sets the <b><i>entire file for each displayed PFM layer</i></b> to UNCHECKED.  For a large file(s) "
               "this can take a fairly long time.");

QString fileVerifiedText = 
  pfmView::tr ("This menu entry sets the <b><i>entire file for each displayed PFM layer</i></b> to VERIFIED.  For a large file(s) "
               "this can take a fairly long time.");

QString fileUnverifiedText = 
  pfmView::tr ("This menu entry sets the <b><i>entire file for each displayed PFM layer</i></b> to UNVERIFIED.  For a large file(s) "
               "this can take a fairly long time.");

QString restoreDeleteText = 
  pfmView::tr ("This menu entry pops up the Restore/Delete Files dialog.  This dialog allows you to "
               "delete or restore input files in the top layer (0) PFM structure.  If you delete a file it will automatically "
               "be renamed with <b>.pfmView_deleted</b> appended to the name.  This will be reversed when you restore a file.");

QString deleteQueueText = 
  pfmView::tr ("This menu entry pops up the Delete Queue Files dialog.  This dialog allows you to delete files that have been placed on "
               "the delete file queue by pfmEdit or pfmEdit3D (right click menu in DELETE_POINT mode).  This works on all PFM layers.  "
               "Note that all files deleted using this method will be renamed with <b>.pfmView_deleted</b> appended to the name.");

QString changePathText = 
  pfmView::tr ("This menu entry pops up the Change Input File Paths dialog.  This dialog allows you to "
               "change the input file paths in the PFM control (*.ctl) file in the event that you have "
	       "moved the input files since the PFM structure was built.");

QString selectOverlaysText = 
  pfmView::tr ("This menu entry pops up the Select Overlays dialog.  The dialog allows you to select "
               "files for overlaying on the main display.  The following file extensions are "
               "recognized:"
               "<ul>"
               "<li>.ARE      -      ISS60 area file format</li>"
               "<li>.are      -      generic area file format</li>"
               "<li>.afs      -      Army Corps area file format (lon,lat)</li>"
               "<li>.ZNE      -      ISS60 tide or SV zone file format</li>"
               "<li>.zne      -      ISS60 tide or SV zone file format</li>"
               "<li>.tdz      -      generic tide zone file format</li>"
               "<li>.srv      -      ISS60 survey plan file format</li>"
               "<li>.trk      -      decimated track file format</li>"
               "<li>.pts      -      generic bin point format</li>"
               "</ul>"
               "The ISS60 file formats are documented in the ISS60 User Manual.  The generic file "
               "formats conform to the following:<br><br>"
               "Geographic positions are entered as a lat, lon pair separated by a comma.  A lat "
               "or lon may be in any of the following formats (degrees, minutes, and seconds must "
               "be separated by a space or tab):"
               "<ul>"
               "<li>Hemisphere Degrees decimal                 : S 28.4532</li>"
               "<li>Hemisphere Degrees minutes decimal         : S 28 27.192</li>"
               "<li>Hemisphere Degrees minutes seconds decimal : S 28 27 11.52</li>"
               "<li>Signed Degrees decimal                 : -28.4532</li>"
               "<li>Signed Degrees minutes decimal         : -28 27.192</li>"
               "<li>Signed Degrees minutes seconds decimal : -28 27 11.52</li>"
               "</ul>"
               "West longitude and south latitude are negative :<br><br>"
               "Ex. : -28 27 11.52 = S28 27 11.52 = s 28 27.192<br><br>"
               "The generic bin point format contains the following comma-separated fields:"
               "<ul>"
               "<li>Latitude</li>"
               "<li>Longitude</li>"
               "<li>Description</li>"
               "</ul>"
               "The decimated track file format is as follows:<br><br>"
               "file #,ping #,lat,lon,azimuth,distance<br><br>"
               "For the purposes of this program azimuth and distance are ignored.  Example:<br><br>"
               "3,9103,30.165371600,-88.749665800,353.748880,30.254914<br><br>"
               "Prior to each change of file number is a line containing the file name in this "
               "format:<br><br>"
               "FILE ### = /whatever/whatever/blah/blah/yackity/smackity");

QString findFeaturesText = 
  pfmView::tr ("This menu option pops up the Find Feature dialog.  This dialog allows you to find "
               "specific features in the coverage map and, optionally, to move the main display to be "
               "centered on a specific feature.");

QString definePolygonText = 
  pfmView::tr ("This menu option pops up the Define Feature Polygon dialog.  The dialog allows you to construct "
               "a polygon surrounding a group of features.  The features will be grouped in the associated "
               "Binary Feature Data (BFD) file.");

QString changeFeatureText = 
  pfmView::tr ("This menu option allows you to select a different feature file for this PFM structure.");

QString changeMosaicText = 
  pfmView::tr ("This menu option allows you to select a different mosaic file for this PFM structure.<br><br>"
               "<b>IMPORTANT NOTE: It is not a good idea to use GeoTIFF files that are much larger than 100MB "
               "since the entire file will be held in memory and the response time can be very slow.</b>");

QString startMosaicText = 
  pfmView::tr ("This menu option allows you to start the mosaic file viewer application.  The mosaic viewer "
               "program (default: mosaicView) and it's hot key arguments can only be set or modified in the "
               "<b>pfmEdit</b> preferences dialog.  The default hot keys are:<br>"
               "<ul>"
               "<li>z : zoom to the area being viewed in pfmView or pfmEdit</li>"
               "<li>+ : zoom in by 10%</li>"
               "<li>- : zoom out by 10%</li>"
               "</ul><br>"
	       "Note that pressing the <b>z<b> key will alos cause other linked ABE applications to zoom "
	       "to the current minimum bounding rectangle being displayed in pfmView.  Applications "
	       "are linked using the <img source=\":/icons/unlink.xpm\"> button.");

QString start3DText = 
  pfmView::tr ("This menu option allows you to start the 3D PFM bin surface viewer application.");

QString displayPFMHeaderText = 
  pfmView::tr ("This menu option pops up the Display PFM Header dialog.  This dialog displays the "
               "ASCII text PFM structure header.");

QString outputDataPointsText = 
  pfmView::tr ("This menu option allows the user to select points to be output to an ASCII file.  "
               "When selected the cursor will become the cross hair cursor.  Left clicking on "
               "a location will collect the position and depths of that location.  Up to 200 points may be "
               "selected.  To select the last point double click on a location.  At that time a file "
               "selection dialog will pop up.  Enter a file name or use a preexisting file.  Files "
               "must have a .pts extension.  If you do not put one on the file name it will be appended "
               "automatically.  If, at any time during the operation you wish to discard the points and "
               "abort the operation simply click the middle mouse button.  The format of the output "
               "file is:<br><br>"
               "latitude, longitude, Point #N average depth minimum depth maximum depth<br>");

QString zoomToAreaText = 
  pfmView::tr ("This menu option allows the user to select an area file and have the view zoom to the "
               "area file boundaries.  This is handy if you would like to compare sections of two different "
               "PFM structures.  Otherwise it's kind of pointless <img source=\":/icons/smile.xpm\">");

QString defineRectAreaText = 
  pfmView::tr ("This menu option allows the user to define a rectangular area and output it to a "
               "generic area file (*.are).  When selected the cursor will become the diagonal sizing "
               "cursor.  Left clicking on a location will cause that point to be the rectangle anchor "
               "point.  Moving the cursor will cause a rectangle to appear.  To finalize the rectangle "
               "left click again.  At that time a file selection dialog will pop up.  Enter a "
               "file name or use a preexisting file.  Files must have a .are extension.  If you do not "
               "put one on the file name it will be appended automatically.  If, at any time during the "
               "operation you wish to discard the rectangle and abort the operation simply click the "
               "middle mouse button.");

QString definePolyAreaText = 
  pfmView::tr ("This menu option allows the user to define a polygonal area and output it to a generic "
               "area file (*.are).  When selected the cursor will become the arrow cursor.  Left "
               "clicking on a location will cause that point to be the first polygon vertex.  Moving the "
               "cursor will cause a line to appear.  Select the next vertex by left clicking on a "
               "location again.  Select as many points as needed.  To select the last point double click "
               "on its location.  At that time a file selection dialog will pop up.  Enter a file name "
               "or use a preexisting file.  Files must have a .are extension.  If you do not put one on "
               "the file name it will be appended automatically.  If, at any time during the operation "
               "you wish to discard the polygon and abort the operation simply click the middle mouse "
               "button.");

QString defineFeaturePolyAreaText = 
  pfmView::tr ("Left clicking on a location will cause that point to be the first polygon vertex.  Moving "
               "the cursor will cause a line to appear.  Select the next vertex by left clicking on a "
               "location again.  Select as many points as needed.  To select the last point double click "
               "on its location or right click to bring up the popup menu and select "
               "<b>Accept polygon</b>.  You can discard the polygon at any time by either clicking the "
               "center mouse button or selecting <b>Discard polygon</b> from the right-click popup menu.  "
               "After either accepting or discarding the polygon the feature dialog will reappear.  If you "
               "have accepted the polygon the length and orientation will be recomputed using the "
               "long axis of the polygonal area.  If you have discarded the polygon these fields will "
               "remain unchanged.");

QString exportImageText = 
  pfmView::tr ("Export the displayed view to an image file.  Everything displayed on the screen with the exception "
	       "of moving objects (marker, cursors, rubberband rectangels, etc.) will be saved to the image file.  The "
	       "available formats are:<br><br>"
	       "<ul>"
	       "<li>JPG</li>"
	       "<li>PNG</li>"
	       "<li>GeoTIFF</li>"
	       "<li>BMP</li>"
	       "<li>XPM</li>"
	       "</ul>"
	       "<br><br><b>Important note: If you type in a name and place one of the extensions on the name (.jpg, "
	       ".png, .tif, .bmp, or .xpm) or if you select a preexisting file with one of those extensions, that "
	       "format will be used.  If you don't place an extension on the file name, the extension selected in the "
	       "<i>Files of type:</i> menu will be appended and used.</b>");

QString linkText = 
  pfmView::tr ("<img source=\":/icons/unlink.xpm\"> Click this button to choose from other ABE or geographical GUI "
	       "programs to connect to.  At present the only possible applications are pfmView, geoSwath, and "
	       "areaCheck.  Hopefully, in the near future, we will be adding CNILE to the list.  The connection "
	       "allows cursor tracking and limited data exchange between the applications.  If there are no "
	       "available ABE groups to link to you can create a new group and add yourself to it so that other "
	       "applications can link to this one.");

QString unlinkText = 
  pfmView::tr ("<img source=\":/icons/link.xpm\"> Click this button to disconnect from another ABE or "
	       "geographical GUI program.  This option is not available unless you have already linked to another "
	       "program.");

QString highText = 
  pfmView::tr ("<img source=\":/icons/high.xpm\"> Click this button to select the highest (shoalest) point inside a user defined "
               "rectangle.  The point will be added to the Binary Feature Data (BFD) file associated with the current, top-level "
               "PFM.  The point will be taken from the average or MISP edited surface regardless of the surface being displayed.  "
               "If there is no associated BFD file, one will be created.  The point can be deleted using the <b>Delete Feature</b> "
               "<img source=\":/icons/deletefeature.xpm\"> option.<br><br>"
               "After clicking the button the cursor will change to the high point cursor "
               "<img source=\":/icons/high_cursor.xpm\">.  Click the left mouse button to define a starting point "
               "for a rectangle.  Move the mouse to define the rectangle bounds.  Left click again to select the shoalest point.  To "
               "abort the operation click the middle mouse button.<br><br> "
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or polygon edit "
               "<img source=\":/icons/edit_poly.xpm\">) is selected.</b>");

QString lowText = 
  pfmView::tr ("<img source=\":/icons/low.xpm\"> Click this button to select the lowest (deepest) point inside a user defined "
               "rectangle.  The point will be added to the Binary Feature Data (BFD) file associated with the current, top-level "
               "PFM.  The point will be taken from the average or MISP edited surface regardless of the surface being displayed.  "
               "If there is no associated BFD file, one will be created.  The point can be deleted using the <b>Delete Feature</b> "
               "<img source=\":/icons/deletefeature.xpm\"> option.<br><br>"
               "After clicking the button the cursor will change to the low point cursor "
               "<img source=\":/icons/low_cursor.xpm\">.  Click the left mouse button to define a starting point "
               "for a rectangle.  Move the mouse to define the rectangle bounds.  Left click again to select the deepest point.  To "
               "abort the operation click the middle mouse button.<br><br> "
	       "<b>NOTE: This mode will remain active unless one of the other modes "
               "(like add feature <img source=\":/icons/addfeature.xpm\"> or polygon edit "
               "<img source=\":/icons/edit_poly.xpm\">) is selected.</b>");

QString cubeText = 
  pfmView::tr ("<img source=\":/icons/cube.xpm\"> Click this button to run the CUBE process on the displayed area.  "
	       "Depending on the size of the area this could take a bit of time.");

QString layerText = 
  pfmView::tr ("Selecting one of the layers will cause it to be the top level layer.  The top level layer is the "
	       "only layer on which certain actions are allowed.  For instance, running CUBE, setting areas to "
	       "checked/verified, and displaying features.  An easier method to bring a PFM to the top level is "
	       "to <b>Ctrl-Left-Click</b> when the mouse is over the desired PFM.");

QString layerPrefsText = 
  pfmView::tr ("Selecting this menu item will cause the layer preferences dialog to appear.  In this dialog you can "
	       "turn layers on or off, set their transparency level (alpha value), set their coverage color, remove "
	       "the layer, and reorder the layers.");

QString toolbarText = 
  pfmView::tr ("There are seven available tool bars in the pfmEdit program.  They are<br><br>"
               "<ul>"
               "<li>Edit tool bar - contains buttons to set editing modes</li>"
               "<li>View tool bar - contains buttons to change the view</li>"
               "<li>Surface tool bar - contains buttons to change the PFM surface that you are viewing</li>"
               "<li>Flag Data tool bar - contains buttons to flag data based on attributes (e.g. reference, suspect)</li>"
               "<li>Feature tool bar - contains buttons to view and modify the features</li>"
               "<li>MISP/CUBE tool bar - contains buttons to interact with MISP and CUBE algorithms (only if MISP or CUBE has been run)</li>"
               "<li>Utilities tool bar - contains buttons to modify the preferences and get context sensitive help</li>"
               "</ul>"
               "The tool bars may be turned on or off and relocated to any location on the screen.  You may click and "
               "drag the tool bars using the handle to the left of (or above) the tool bar.  Right clicking in the tool "
               "bar or in the menu bar will pop up a menu allowing you to hide or show the individual tool bars.  The "
               "location, orientation, and visibility of the tool bars will be saved on exit.");
