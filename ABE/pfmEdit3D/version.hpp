
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




/*********************************************************************************************

    This program is public domain software that was developed by the U.S. Naval Oceanographic
    Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

*********************************************************************************************/


#ifndef VERSION

#ifdef OPTECH_CZMIL
#define     VERSION     "CME Software - 3D Editor V4.79 - 10/24/11"
#else
#define     VERSION     "PFM Software - pfmEdit3D V4.79 - 10/24/11"
#endif

#endif

/*! <pre>

    Version 1.00
    Jan C. Depner
    11/18/08

    First working version.


    Version 1.01
    Jan C. Depner
    11/21/08

    Added masking and a measuring tool.


    Version 1.02
    Jan C. Depner
    11/24/08

    Changed default action keys for waveMonitor (n is now a toggle for nearest neighbor and single waveform
    display modes).  Fix bug in slotMouseMove when in MEASURE mode.


    Version 1.03
    Jan C. Depner
    11/25/08

    Replaced charts_list with chartsMonitor and removed the "extended" (x) action key from the waveMonitor defaults.


    Version 1.04
    Jan C. Depner
    11/25/08

    Lock the file and line names when we freeze the cursor.


    Version 1.05
    Jan C. Depner
    11/28/08

    Fixed zoom, Z exaggeration, setting map center.  Add ability to mask by PFM_USER flag.  Made ancillary program action
    keys work as toggles.  That is, the first time you hit the key it starts the program.  The next time it kills it.
    This is only for GUI programs that save state.


    Version 1.06
    Jan C. Depner
    12/01/08

    Added ability to mask by data type if more than one data type is being displayed.


    Version 1.07
    Jan C. Depner
    12/04/08

    Now sets PFM_SELECTED_FEATURE on points that are associated with a target.  Added ability to mark 
    PFM_SELECTED_FEATURE and PFM_DESIGNATED_SOUNDING data.


    Version 1.08
    Jan C. Depner
    12/12/08

    Allow hot keys in MEASURE mode.


    Version 1.09
    Jan C. Depner
    12/15/08

    Added screenshot grabber for adding and editing targets.  Also, only unfreeze if we delete the point
    or we explicitly unfreeze by pressing f or F.  Do not unfreeze on mode change.


    Version 1.10
    Jan C. Depner
    12/16/08

    Added right click menu option when in SET_REFERENCE or UNSET_REFERENCE mode to allow you to delete
    all visible, non-transparent reference points.


    Version 1.11
    Jan C. Depner
    01/09/09

    Various and sundry changes to try to speed up display in Windoze (SUX).


    Version 1.20
    Jan C. Depner
    01/23/09

    Changed to XOR for movable objects because redrawing the point cloud every time made the response too
    slow when you were viewing a lot of points.  I saved the original, non-XOR version in nvMapGL.cpp.noXOR.


    Version 1.21
    Jan C. Depner
    01/27/09

    Removed timer from polygon drawing.


    Version 1.30
    Jan C. Depner
    02/11/09

    Added coordination between pfmView, pfmEdit3D, and pfm3D.  When pfmEdit3D is started pfm3D will unload
    it's display lists and memory unless you move the cursor back into pfm3D in which case it will reload
    until you leave again.  Lists and memory will be reloaded upon exit from pfmEdit3D.  Fixed problems with
    rubberband polygons.


    Version 1.31
    Jan C. Depner
    02/12/09

    Finally found the problem with display speed while rotating (on Windoze).  It actually had an impact on Linux
    but the disk read speed was so much better that you couldn't tell ;-)  Also, added draw scale option to prefs.
    It's not really a scale, just a box at the moment.


    Version 1.32
    Jan C. Depner
    02/18/09

    Numerous hacks to try to fix the Windoze version of nvMapGL.cpp.    Fixed the extended help feature to use the
    PFM_ABE environment variable to find the help file.  Also, hard-wired the browser names in Windoze and Linux.


    Version 1.33
    Jan C. Depner
    02/22/09

    Hopefully handling the movable objects (XORed) correctly now.  This should make the Windoze version
    a bit nicer.


    Version 1.34
    Jan C. Depner
    02/24/09

    Finally figured it out - I have to handle all of the GL_BACK and GL_FRONT buffer swapping manually.  If
    I let it auto swap it gets out of control.


    Version 1.35
    Jan C. Depner
    03/13/09

    Minor change to allow waveWaterfall and chartsPic to handle WLF data.


    Version 1.36
    Jan C. Depner
    03/20/09

    Brought back the old waveMonitor as waveformMonitor at the request of the ACE.


    Version 1.37
    Jan C. Depner
    03/25/09

    Fixed the flicker at the end of a rotate or zoom operation.  Save the last Y and ZX rotations to restore
    on startup.


    Version 1.40
    Jan C. Depner
    03/31/09

    Replaced support of NAVO standard target (XML) files with Binary Feature Data file support.


    Version 1.41
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 1.42
    Jan C. Depner
    04/14/09

    Added selectable feature marker size option.


    Version 1.43
    Jan C. Depner
    04/15/09

    Minor bug in editFeature.cpp.


    Version 1.44
    Jan C. Depner
    04/20/09

    Fixed display of NULL data.


    Version 1.45
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 1.46
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 1.47
    Jan C. Depner
    05/04/09

    Use paintEvent for "expose" events in nvMapGL.cpp.  This wasn't a problem under compiz but shows up
    under normal window managers.


    Version 1.48
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 1.49
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 1.50
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 1.51
    Jan C. Depner
    07/15/09

    Was computing the 3D bounds incorrectly.


    Version 1.52
    Jan C. Depner
    07/28/09

    Changed use of _sleep to Sleep on MinGW (Windows).


    Version 1.53
    Jan C. Depner
    07/29/09

    Added tool tips to tool bars.
    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 1.54
    Jan C. Depner
    08/20/09

    Moved flagging from "color by" menu to its own menu now that we can overlay lines in 2D.  Also, mark
    all invalid points with an X when invalid data is being displayed.  Added hofReturnKill program with
    -s option to kill Shallow Water Algorithm and/or Shoreline Depth Swapped data.


    Version 1.55
    Jan C. Depner
    08/25/09

    Added filter masks, filter button, and area-based filtering to the editor.


    Version 1.56
    Jan C. Depner
    08/27/09

    Added message when filter doesn't find any points to delete.


    Version 1.60
    Jan C. Depner
    09/10/09

    Added LIDAR tool bar.  Added push buttons to start LIDAR monitoring programs (like waveMonitor) to the
    LIDAR tool bar.


    Version 1.61
    Jan C. Depner
    09/11/09

    Added "no kill" (-n) option to command line.  This is used if pfmEdit3D is started from pfmEdit
    instead of from pfmView.  We wouldn't want pfmEdit3D to kill or respawn the programs that pfmEdit
    is in control of.  Fixed getColor calls so that cancel works.


    Version 1.62
    Jan C. Depner
    09/16/09

    Made new blinkTimer for mask/unmask buttons.  Added check for kill of ancillary programs that are attached
    to buttons.


    Version 1.63
    Jan C. Depner
    09/22/09

    Allow EDIT_FEATURE to be a saved edit function so that the editor may come up in that mode on start.


    Version 1.64
    Jan C. Depner
    09/23/09

    Added ability to "verify" (set confidence to 5) all displayed valid features.


    Version 1.65
    Jan C. Depner
    10/22/09

    Cleaned up hot key handling.


    Version 2.00
    Jan C. Depner
    11/02/09

    Added 2D tracker and attributeViewer.


    Version 2.01
    Jan C. Depner
    11/12/09

    Added delete records (pings) in rectangle or polygon.  Only available from the right mouse menu.


    Version 2.02
    Jan C. Depner
    12/01/09

    Fixed mask reset bug.


    Version 2.03
    Jan C. Depner
    12/02/09

    Replaced get_egm96 with get_egm08.


    Version 2.04
    Jan C. Depner
    12/09/09

    Fixed VirtualBox close bug.


    Version 2.05
    Jan C. Depner
    12/11/09

    Added "Edit return status:" to the printf calls that return info to pfmView.


    Version 2.10
    Jan C. Depner
    12/17/09

    Added 100 to 1,000,000 undo levels - woo hoo!


    Version 2.11
    Jan C. Depner
    12/28/09

    Fixed bug in slotMouseMove.  I wasn't checking to see if a rubberband construct existed prior to trying
    to drag it.  Also, added filter standard deviation button.


    Version 2.12
    Jan C. Depner
    12/29/09

    Stopped the filter from filtering points when there is not enough surrounding valid data.


    Version 2.13
    Jan C. Depner
    01/05/10

    Made filter confirmation optional.  Replaced all calls to the "inside" function with calls to
    "inside_polygon2".  This is supposed to be faster although I couldn't see any major improvement.


    Version 2.14
    Jan C. Depner
    01/06/10

    Fixed the filter so that it uses the points in memory (and their validity from whatever editing has been
    done) instead of reading the data from the PFM.  I just stole the filter code from pfmView and in that
    case it was OK to read from the PFM since changes made there are automatically updated in the PFM.
    In the editor the changes don't happen until you exit, so here we have to use the data in memory
    and make pseudo bins in memory to do the proper thing.


    Version 2.15
    Jan C. Depner
    01/14/10

    Fixed filter undo problem.  Also, fixed problem with single value in all depths (as when an area has been
    land masked).


    Version 2.16
    Jan C. Depner
    01/26/10

    Replaced geoSwath call with gsfMonitor.  The geoSwath program was sort of useless in this context anyway.


    Version 2.20
    Jan C. Depner
    01/28/10

    Added right click menu for all non-monitor type ancillary programs when in DELETE_POINT mode or
    HOTKEY_POLYGON mode.


    Version 2.21
    Jan C. Depner
    02/17/10

    Wasn't checking against rectangle in get_buffer.  If we had a large bin size this caused all kinds of grief - the area you got
    in the editor was much larger than what you asked for.


    Version 2.22
    Jan C. Depner
    02/25/10

    Added support for running in read only mode (from pfmWDBView via pfmView).


    Version 2.23
    Jan C. Depner
    03/02/10

    Moved the filter message box to the bottom of the screen so it wouldn't get in the way.


    Version 2.24
    Jan C. Depner
    03/09/10

    Added special translators for time and datum attributes from PFMWDB.


    Version 2.25
    Jan C. Depner
    03/25/10

    Fixed a color range problem.  Fixed redraw of 2D track map when not needed.  Fixed overlay of flags after selection.


    Version 2.26
    Jan C. Depner, Micah Tinkler
    04/02/10

    Added ability to filter mask after running the filter while the filter confirmation box is still up.  This allows you to
    look at the points the filter has picked and mask some out prior to accepting the filter (it reruns the filter and 
    regenerates the filter confirmation dialog with the new filter count).  Filter confirmation is no longer optional.
    Standard deviation can be set in the filter confirmation dialog.


    Version 2.27
    Jan C. Depner, Micah Tinkler
    04/09/10

    When the filter message dialog is displayed DELETE_POINT mode snaps to filter kill points only.  Surrounding points
    used for waveWaterfall and other ancillary programs still come from all valid points.  Replaced hot keys for 
    KEEP_POLYGON, RESTORE_RECTANGLE, and RESTORE_POLYGON with hotkeys for FILTER, RECT_FILTER_MASK, and POLY_FILTER_MASK.
    Also, made the Enter key trigger the filter message dialog Accept action if the filter message dialog is present.


    Version 2.30
    Jan C. Depner
    04/14/10

    Added optional color scale to the right of the main window.


    Version 3.00
    Jan C. Depner
    04/20/10

    Completely rearranged the GUI.  Moved the 2D tracker, status info, and color scale to the left side.  Moved the slicer to the
    right side.


    Version 3.01
    Jan C. Depner
    04/21/10

    Fixed the ever elusive "deep flyer not showing up" problem.  Added ability to use Shift/mouse wheel to modify the filterMessage_dialog
    standard deviation value (had to play timer games to make it work right).


    Version 3.02
    Jan C. Depner
    04/22/10

    Added ability to mark files for deletion using the right click menu in DELETE_POINT mode.  Files are queued for deletion in pfmView using
    the Edit menu.  Also, cursor now tracks in main window when moved in 2D tracker window.


    Version 3.03
    Jan C. Depner
    05/03/10

    Moved the filter message dialog into a notebook page that is behind the 2D Tracker.  It only comes up after you have run the
    filter.


    Version 3.04
    Jan C. Depner
    05/04/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 3.05
    Jan C. Depner
    06/03/10

    Added ability to display either manually invalid, filter invalid, or both instead of just displaying any invalid data.
    This helps a lot when processing LIDAR data since GCS invalidated points are marked as PFM_FILTER_INVAL while points removed
    in the editor or viewer by the filter (or manually) are marked as PFM_MANUALLY_INVAL.


    Version 3.06
    Jan C. Depner
    06/04/10

    Can now invalidate features in hotkey polygon mode.


    Version 3.07
    Jan C. Depner
    06/15/10

    Fixed resize bug when first entering window in DELETE_POINT mode.


    Version 3.08
    Jan C. Depner
    06/16/10

    Now changes all status information when the cursor is in the 2D tracker.


    Version 3.09
    Jan C. Depner
    06/25/10

    Added support for displaying PFM_HAWKEYE_DATA in the chartsMonitor ancillary program.


    Version 3.10
    Jan C. Depner
    06/28/10

    Changed chartsMonitor to lidarMonitor due to multiple lidar support.


    Version 3.11
    Jan C. Depner
    07/06/10

    HSV min and max colors are now passed in on the command line so that they can be set by pfmView or CZMIL.  Also,
    HSV min and max locked values can be optionally passed in on the command line.


    Version 3.12
    Jan C. Depner
    09/02/10

    Added ability to use the text search string, text search invert, and text search type from pfmView.


    Version 3.13
    Jan C. Depner
    09/08/10

    Changes to handle Qt 4.6.3.


    Version 3.14
    Jan C. Depner
    09/17/10

    Fixed bug displaying feature info.


    Version 3.15
    Gary Morris (USM)/Jan C. Depner
    09/20/10

    Merge of USM CZMIL code.


    Version 3.16
    Jan C. Depner
    10/08/10

    Removed select/unselect soundings options since we don't do that in PFM_ABE anymore.
    You can still flag them though.


    Version 3.17
    Jan C. Depner
    10/20/10

    Fixed tracker display to check for the PFM being displayed when multiple PFMs are up.


    Version 3.18
    Jan C. Depner
    10/26/10

    Fixed sign on Z value for modes other than DELETE_POINT.


    Version 4.00
    Jan C. Depner
    11/05/10

    Finally fixed auto Z scaling in nvMapGL (nvutility library).  Also added minimum Z extents to preferences.


    Version 4.01
    Jan C. Depner
    12/03/10

    Added rmsMonitor to ancillary programs.  This program reads smrmsg files associated with HOF or TOF and displays the
    RMS values for the record.


    Version 4.02
    Jan C. Depner
    12/06/10

    Cleaned up buttonText, buttonAccel, buttonIcon, and button array handling.  Made selection of multiple lines  work
    in the display instead of having a list of the lines.


    Version 4.03
    Jan C. Depner
    12/09/10

    Fixed the slice and size bar buttons (finally).


    Version 4.10
    Gary Morris (USM), Jan C. Depner
    12/13/10

    Added Gary's changes to support the attributeViewer AV_DISTANCE_TOOL.  Also, we can finally mark data points in a somewhat
    proper way.  They don't disappear when you rotate or zoom.


    Version 4.11
    Gary Morris (USM), Jan C. Depner
    12/15/10

    Can now highlight and un-highlight in polygons.  Also, pressing the Enter key when data is highlighted will
    invalidate the highlighted points.


    Version 4.12
    Jan C. Depner
    12/22/10

    Optional contours in the 2D tracker map.  Maintain aspect ratio when displaying data.


    Version 4.13
    Jan C. Depner
    12/30/10

    Ctrl + arrow keys now does a move of the area being edited the same way that the 2D editor does it.


    Version 4.14
    Jan C. Depner
    01/03/11

    Removed displaySingle button and replaced with undisplaySingle (i.e. hide single).  Changed operation of 
    displayMultiple to select last line with double click instead of just ending displayMultiple mode.


    Version 4.15
    Jan C. Depner
    01/04/11

    Added new color array for color by line mode.  It now uses twice the number of colors.  The color wheel
    values go from 200 to 0/360 to 300 twice.  The first half of the color array uses value set to 255 (full color).
    The second half uses value set to 64 (darker).  This gives us a better color spread in order to differentiate
    between lines.


    Version 4.16
    Jan C. Depner
    01/05/11

    Added hotkey check in the preferences dialog to prevent conflicts.  I really should have done this a long time
    ago but it was way down on the priority list ;-)


    Version 4.17
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 4.18
    Jan C. Depner
    01/07/11

    Added the cursor distance, azimuth, and delta Z to the measure tool output (in the status bar).


    Version 4.19
    Jan C. Depner
    01/14/11

    Added an exaggeration scrollbar to the left side of the window.


    Version 4.20
    Jan C. Depner
    01/16/11

    Positioned non-modal popup dialogs in the center of the screen.


    Version 4.21
    Jan C. Depner
    02/10/11

    Removed the chartsSwap program from the list of ancillary programs.  We don't do that anymore.


    Version 4.22
    Jan C. Depner
    02/16/11

    Added hawkeyeMonitor to ancillary programs.


    Version 4.23
    Gary Morris (USM), Jan C. Depner
    02/18/11

    Merged Gary's changes to support updates to the attributeViewer.


    Version 4.24
    Jan C. Depner
    03/09/11

    Strip /PFMWDB:: off of input files in the ancillary programs stuff if we're working on the PFM World Data Base (PFMWDB).
    Just in case we might want to move input files to the PFMWDB directory and work on them.


    Version 4.28
    Jan C. Depner
    03/28/11

    Allow user to define the key for invalidating filter highlighted or manually highlighted data.  Try to save the user's
    hot key definitions even across setting version changes.


    Version 4.29
    Jan C. Depner
    03/30/11

    Added option (in Preferences) to turn auto-scaling on or off.  Removed NOOP functions (i.e. we now allow editing when displaying
    invalid data since you'll see it in the 2D tracker).  We originally turned off editing because it confused people when data 
    didn't disappear.


    Version 4.30
    Jan C. Depner
    04/06/11

    Brought back the displayLines list as a right click option when in SET_MULTIPLE mode.  Request from Paul Marin.


    Version 4.31
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks are now handled in ABE shared memory and are no longer read in as
    arguments from pfmView.


    Version 4.32
    Jan C. Depner
    04/11/11

    Properly populate the file name when creating a new feature file.  It was leaving it blank on Windows.


    Version 4.40
    Jan C. Depner
    04/14/11

    Added the externalFilter function to support the (newly added) hofWaveFilter ancillary program.  Removed tofWaterKill.


    Version 4.50
    Jan C. Depner
    04/22/11

    The point cloud memory (POINT_CLOUD structure) is now allocated in shared memory.  This leaves a little slop around the 
    edges of the area to be edited and we have to pre-scan the bins to get the count but it will save a ton of memory and
    a bunch of I/O when running ancillary programs that need a large amount of data passed to them (mostly hofWaveFilter).
    There doesn't seem to be much of a hit in terms of speed when we do this.


    Version 4.51
    Gary Morris (USM)
    04/27/11

    Fixes to color handling in attributeViewerFunctions.cpp.


    Version 4.60
    Jan C. Depner
    04/28/11

    Flagging data points now makes the points "highlighted".  This allows you to delete flagged points.  You can also now
    restore highlighted invalid data points.


    Version 4.61
    Jan C. Depner
    05/05/11

    We no longer rescale or recolor range our data due to masking.  This actually makes sense when you think about it since the
    masked data is not invalid.  Thanx and a tip of the hat to McClain Walker (3001).  Filters (including hofWaveFilter) now
    honor the mask flag.  Happy Cinco de Mayo!!!


    Version 4.62
    Jan C. Depner
    05/09/11

    Fixed problem with pre-existing shared point cloud memory.  The memory may not have been detached if a previous
    editor process crashed.  Now it attaches the pre-existing memory, detaches it, and then creates a new point cloud 
    shared memory segment.  Fixed a serious FUBAR when looking for the nearest filter highlighted point.  Had to add
    a new function to nvMapGL.cpp in the nvutility library.  The function used in pfmEdit3D is getMarker3DCoords.
    Added invert highlight function and button.


    Version 4.63
    Jan C. Depner
    05/11/11

    Don't display features that are outside of the slice.


    Version 4.64
    Jan C. Depner
    05/13/11

    Added ability to lock min/max colors and values when coloring by attribute.


    Version 4.65
    Jan C. Depner
    05/18/11

    Fixed hide and show problems (due to incorrectly determining unique data types) when more than one PFM 
    is being edited.  Moved mask responsibility from hofWaveFilter back here.  Desensitize hide and show
    "highlighted" checkboxes in hide and show menus when no points are highlighted.  Fixed return from hofReturnKill,
    it was missing the "line" value in the sscanf of the shared file.


    Version 4.66
    Jan C. Depner
    05/20/11

    Added pmt and apd ac zero offset thresholds to the hofWaveFilter parameters.


    Version 4.67
    Jan C. Depner
    05/26/11

    Added highlight record (ping, shot) right click popup menu option.


    Version 4.68
    Jan C. Depner
    06/13/11

    Fix bug with attribute filter max and min values.


    Version 4.69
    Jan C. Depner
    06/27/11

    Fix bug when editing multiple PFMs.  Added menu to allow displaying of all, unverified, verified, or no features
    (as opposed to just on/off).


    Version 4.70
    Jan C. Depner
    07/08/11

    Fixed slice bug that was causing the slice to be one slice too far along when we started slicing.


    Version 4.71
    Jan C. Depner
    07/15/11

    Fixed bug changing minimum color for color-by options.


    Version 4.72
    Jan C. Depner
    07/19/11

    Added QMessageBox output for memory allocation error in get_buffer.c.


    Version 4.73
    Jan C. Depner
    07/22/11

    Fixed ancillary program button state bug.  If pfmEdit3D was started with ancillary programs up
    (state set to 2) it was resetting the state to 1 because the QProcess wasn't associated with
    the current instantiation of pfmEdit3D.  Now it checks for the state being 2 (already running)
    as well.


    Version 4.74
    Jan C. Depner
    07/29/11

    Added input of "pfmView killed" qsetting so that pfmEdit3D will know that pfmView was shut down
    since the last time it was running.  This allows us to properly set the "monitor" type ancillary
    program "running" states since pfmView issues a kill command to all of the associated ancillary
    programs when it exits.


    Version 4.75
    Jan C. Depner
    08/03/11

    Added CZMIL support in get_feature_event_time.cpp.


    Version 4.76
    Gary Morris (USM)
    08/11/11
	
    Added an AV Distance Threshold toolButton tied to the Attribute Viewer running.
    Implemented a new handshake between Attribute Viewer and Editor to select user-defined attribute color codes.
    Moved the selection of drawing a line for the AV Distance Threshold tool out of the track map and to the 3D map.
    Allowed for talking to multiple type attribute viewers.
	

    Version 4.77
    Jan Depner
    08/16/11
	
    Fix data type lookup table in get_buffer.cpp when there's more than 1 PFM open.
	

    Version 4.78
    Jan Depner
    08/31/11
	
    Changed waveMonitor to CZMILwaveMonitor.
	

    Version 4.79
    Jan Depner
    10/24/11
	
    Added fix for invalid highlights not showing up when you delete data while displaying
    invalid with invalid data flagged.

</pre>*/
