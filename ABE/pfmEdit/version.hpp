
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



/*  C/Motif pfm_edit replaced by C++/Qt pfmEdit on 03/17/05.  All comments for pfm_edit
    have been left in for historical (hysterical ???) purposes.  JCD  */


#ifndef VERSION

#ifdef OPTECH_CZMIL
#define     VERSION     "CME Software - 2D Editor V8.79 - 10/24/11"
#else
#define     VERSION     "PFM Software - pfmEdit V8.79 - 10/24/11"
#endif

#endif

/*! <pre>

    Version 1.0
    Jan C. Depner
    05/21/99


    Version 2.0
    Jan C. Depner
    02/15/00

    Went to version 2.0 of the PFM library.


    Version 2.01
    Jan C. Depner
    02/23/00

    Lots of groovy new goodies.


    Version 2.1
    Jan C. Depner
    03/29/00

    Uses version 2.1 of the pfm library.


    Version 2.2
    Jan C. Depner
    04/06/00

    Reads a runstream file from pfm_view to allow editing of a polygon from the
    viewer.  Also, can stop the plot and edit ONLY the visible points.


    Version 2.3
    Jan C. Depner
    04/10/00

    Added the ability to turn off auto redraw after an edit operation.


    Version 2.31
    Jan C. Depner
    04/23/00

    Fixed a bunch of small annoying bugs.  Added a few minor enhancements.


    Version 2.32
    Jan C. Depner
    06/22/00

    Removed Sound.  Added About Help xpm.


    Version 2.33
    Jan C. Depner
    08/20/00

    Minor changes, mostly having to do with SHOALS data.


    Version 2.34
    Jan C. Depner
    08/24/00

    Oh baby, pixmap'ed buttons and pseudo-bubble-help.


    Version 2.35
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 2.36
    Jan C. Depner
    09/17/00

    Popup menu for external viewers.  Highlight point in views.  Removed 
    filter.


    Version 2.37
    Jan C. Depner
    10/03/00

    Fixed bug in get_buffer that caused status to be incorrectly saved when
    up against edge of area.  Added cancel button to popup menu.  Made the
    active function a saveable option.


    Version 2.4
    Jan C. Depner
    10/12/00

    Modified slightly to work with the more efficient pfm_lib V2.6.


    Version 2.5
    Jan C. Depner
    12/28/00

    Added a much nicer percent spinner for the save and exit.


    Version 2.51
    Jan C. Depner
    02/21/01

    Handles Hypack xyz files.


    Version 2.52
    Jan C. Depner
    05/06/01

    Added demo version stuff and removed ACME Software references.


    Version 2.53
    Jan C. Depner
    05/13/01

    Fixed the placement of the target sonar depth.  Changed the way the 
    buttons are set as toggle or push (much nicer).


    Version 2.54
    Jan C. Depner
    06/20/01

    Changed open_pfm_file to use structure arg.  Added SHOALS out file lister.


    Version 3.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 3.1
    Jan C. Depner
    07/21/01
 
    Default unmarked data to class 1.


    Version 3.11
    Jan C. Depner
    10/19/01
 
    Fixed bug that was causing the polygon edit mode in pfm_view to not
    work.  Added menu to allow user to define specific lines to be viewed.
    Removed scroll bars from drawing area since no one was using them.  Made
    sure demo version would not write out changes.


    Version 3.12
    Jan C. Depner
    11/27/01
 
    Fixed bug in adding targets.  Needed new contact ID for each.


    Version 3.13
    Jan C. Depner
    12/11/01
 
    Turned off SELECT VIEW arrow if you press any button outside of the
    viewing area.


    Version 3.2
    Jan C. Depner
    06/06/02
 
    Supporting PFM 4.1.  Removed View CLass buttons.  Rearranged the
    buttons.  Added view of different classes in the Options menu.


    Version 3.21
    Jan C. Depner
    08/06/02
 
    Bug fix in check_bounds.  Not displaying non-flagged data.


    Version 3.22
    Jan C. Depner
    08/14/02
 
    Added printout on shared memory error.


    Version 3.23
    Jan C. Depner
    09/19/02
 
    Removed the PFM_USER_05 flag.


    Version 3.24
    Jan C. Depner
    11/06/02
 
    Removed DEMO_VERSION.


    Version 3.25
    Jan C. Depner
    11/18/02
 
    Changed the data type names to match those in the latest updated PFM
    library.


    Version 4.00
    Jan C. Depner
    05/08/03
 
    Added capability to shift the line depth for selected lines to assist in
    editing data that may have a bad predicted tide, draft, heave, or other
    Z offset problem.  Line shifts are saved in the file ~/.pfm_shift.  This
    file is removed on a line shift reset.


    Version 4.01
    Jan C. Depner
    06/01/03
 
    Fixed bug in get_buffer that was masking if we had only one marked data
    type but some data that wasn't marked at all.


    Version 4.02
    Jan C. Depner
    07/18/03
 
    Added unisips kicker to ancillary program defaults.  Added reference 
    view button.


    Version 4.1
    Jan C. Depner
    07/25/03
 
    Changed polygon functionality to continuously draw with a .1 second
    minimum time between points.


    Version 4.2
    Jan C. Depner
    09/11/03
 
    Removed the PFM_USER flag check on load.  You can still select which data
    type you want to view but you might end up with a blank screen.  We were
    having problems with this one.
    Speeded up computation of views by storing misc.rotate_angle as an integer
    in 1/10 degree units.  This is the index into the sine and cosine arrays 
    for transformations.  
    Added the ability to look at vertical slices through the data when in any
    of the alternate views (east, south, select).  This is handled by a scroll
    bar on the right hand side of the window.  
    Added a context sensitive help button to the command buttons.


    Version 4.3
    Jan C. Depner
    11/12/03
 
    Added ability to handle up to 5 data types with ancillary programs.


    Version 4.4
    Jan C. Depner
    02/24/04
 
    Switched to support of new XML target format.  Removed support for old
    HYDRO .tgt format.


    Version 4.5
    Jan C. Depner
    05/27/04
 
    Added support for [ALL RECORDS] switch on ancillary programs.  Also added
    charts_swap as 9th ancillary program.  The [ALL RECORDS] switch allows you
    to shell a program for each displayed point.  Right now it's just used for
    charts_swap which does a first and second return swap if possible.  This
    is somewhat dangerous in that you can't exit without saving at that point
    as the change is already made in the HOF file.  Oh well...  Also, added 
    arrow/page keypress for slice view.


    Version 4.6
    Jan C. Depner
    06/02/04
 
    Replaced [ALL RECORDS] with Hot key polygon action (boy that didn't last
    long).  You can now define a polygonal area (in any view) to run ancillary
    programs on.


    Version 4.61
    Jan C. Depner
    06/16/04
 
    Added ability to assign snippet files to targets.  This is handy for
    downlooking photos from LIDAR surveys.


    Version 4.62
    Jan C. Depner
    08/06/04
 
    Added filename to .pfm_shift file for use by other, non-PFM programs.


    Version 4.63
    Jan C. Depner
    08/13/04
 
    Fixed small bug with snippet display.  Added envin and envout entries for
    the snippet viewer and mosaic viewer programs.


    Version 4.7
    Jan C. Depner
    08/18/04
 
    Added ability to display GeoTIFF as underlay in plan view and handle Z
    conversion factors.  These are passed as command line arguments from
    pfm_view.  Also added new button to allow conversion to/from reference
    data.  Changed image display to use ChartsPic.  Changed the hof and tof
    list functions to both be triggered by the l key.


    Version 4.71
    Jan C. Depner
    10/21/04
 
    Fixed minor screwup in line shift code.


    Version 4.72
    Jan C. Depner
    11/04/04
 
    Fixed minor screwup in get_buffer (added 5% to bounds).  Happy birthday to me!


    Version 4.73
    Jan C. Depner
    11/23/04
 
    Handle old PFM files built with LIDAR.  Start record number was 0 instead of 1.
    Changed 5% to 2% for addition to bounds.


    Version 4.74
    Jan C. Depner
    01/18/05
 
    Doh!!!!  Forgot to change swath_m to NV_U_INT32 from NV_U_INT16.


    Version 4.75
    Jan C. Depner
    01/19/05
 
    Handles attributes better now.  Added target label option.  Supports display
    of null values (mostly for pictures over NBR in LIDAR).


    Version 4.76
    Jan C. Depner
    01/27/05
 
    Added hot keys for the major delete and restore functions.


    Version 4.77
    Jan C. Depner
    02/07/05
 
    Added -Q option to support pfmView (Qt) parent process.


    Version 4.78
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 4.79
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.


******************* pfmEdit comments *********************


    Version 1.0
    Jan C. depner
    03/17/05

    This is the first version of the C++/Qt replacement for pfm_edit.  As such, 
    I have gone back to 1.0 but didn't want to lose the earlier documentation
    of the C/Motif pfm_edit program.  Happy Saint Patrick's Day!


    Version 1.01
    Jan C. depner
    04/19/05

    Changed all of the QVBox's to QVBoxLayout's.  This allows the dialogs to auto
    size correctly.  I'm learning ;-)  Also, switched to examGSF from exammb for
    default GSF ancillary program.


    Version 1.02
    Jan C. depner
    04/27/05

    Added auto unload option and button.  If run from Fledermaus this will be on by
    default if DYNAMIC_RELOAD is set to 1 in the PFM bin header.  Otherwise it is set
    according to the QSettings read in envin or the button state.


    Version 1.03
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.04
    Jan C. Depner
    05/10/05

    Fixed a major stupid - forgot to close the PFM file - DOH!


    Version 1.05
    Jan C. Depner
    05/23/05

    Added Brighten and Darken action keys (b and d) to chartsPic default action keys.


    Version 1.06
    Jan C. Depner
    05/31/05

    Switched from crossCursor to arrowCursor.  crossCursor was too hard to see.


    Version 1.07
    Jan C. Depner
    06/08/05

    PFM_USER_NN data is now flagged instead of being displayed exclusively when using the
    01, 02, 03, and 04 buttons.  Added a No user flags button to reset.  Hot keys work in
    ADD_TARGET mode.


    Version 1.08
    Jan C. Depner
    06/29/05

    Added color by line or depth in plan view and color by line or distance from viewer in
    the alternate views.


    Version 1.09
    Jan C. Depner
    07/20/05

    Fixed bug to allow data changes to save when target has been added.


    Version 1.10
    Jan C. Depner
    07/25/05

    Fixed color by line/depth/distance tooltips.


    Version 1.11
    Jan C. Depner
    08/23/05

    Added extended help to start up browser using the Area_Based_Editor.html file.


    Version 1.12
    Jan C. Depner
    08/24/05

    Fix bug in selecting lines to display (displayLines.cpp).


    Version 1.2
    Jan C. Depner
    10/03/05

    Changed hotkeyPolygon to use new shared file for swapping (other stuff later)..  Good riddance to
    hurricane Katrina!


    Version 1.21
    Jan C. Depner
    10/26/05

    Now uses the PFM 4.6 handle file or list file instead of the PFM directory name.


    Version 1.3
    Jan C. Depner
    11/10/05

    PFM 5.0 changes (attribute display code and other goodies).


    Version 1.31
    Jan C. Depner
    01/10/06

    Tries to get snippet from same directory as target file and then falls back to full snippet
    file name.


    Version 1.32
    Jan C. Depner
    01/19/06

    Changed updateTarget to a QDialog to fix a small bug.


    Version 1.33
    Jan C. Depner
    01/29/06

    Updated map and rproj.


    Version 1.4
    Jan C. Depner
    02/10/06

    New updateTarget and targetTypes.hpp.  We now have standard types to load in the target description.


    Version 1.5
    Jan C. Depner
    03/06/06

    Removed find_startup_name.  Now use the one in utilities.  Moved ABE.hpp to utility.  Added snippet
    filename to ABE_SHARE for use by chartsPic.  When making a new target file we now make a directory
    called "targets" in the directory that the PFM file is in and then make the target file in that
    directory.  This is where we'll ask chartsPic to store the snippets via "action key" (by default a
    't').


    Version 1.51
    Jan C. Depner
    03/29/06

    Replaced QVBox, QHBox, QVGroupBox, QHGroupBox with QVBoxLayout, QHBoxLayout, QGroupBox to prepare for Qt 4.


    Version 2.0
    Jan C. Depner
    04/05/06

    Massive Qt changes - new cursors, left click to finish operations, right click menu in some modes.


    Version 2.01
    Jan C. Depner
    05/17/06

    Save the last target description to place in the description field on next add_target.


    Version 2.02
    Jan C. Depner
    06/01/06

    Fixed bug in right click popup menu function.


    Version 2.03
    Jan C. Depner
    06/05/06

    Removed inside.cpp.  Moved to utility.


    Version 2.04
    Jan C. Depner
    06/14/06

    Save the heading from chartsPic to the target file when getting an image from chartsPic.  Switched to
    targetPic to display target snippets.


    Version 2.05
    Jan C. Depner
    06/29/06

    Removed --run_required from default arguments for charts_swap call.  Receiving changes via shared_file
    since stdout from a QProcess was unreliable at best.


    Version 2.06
    Jan C. Depner
    07/07/06

    Don't open projected PFMs.


    Version 2.07
    Jan C. Depner
    08/02/06

    Added enableMapSignals so that we don't read the data three times before we display it for the first time
    (resize callbacks).  See main.cpp for minor changes.


    Version 2.08
    Jan C. Depner
    08/08/06

    Removed qApp->processEvents from preredraw.  It causes a timing problem that results in a segfault.


    Version 2.09
    Jan C. Depner
    08/25/06

    Setting Object depth as well as sonar depth when changing target depth.


    Version 2.10
    Jan C. Depner
    09/08/06

    Added ability to rotate alternate views using the arrow keys.


    Version 2.11
    Jan C. Depner
    09/13/06

    Had to change map.cpp and map.hpp to nvmap.hpp and nvmap.cpp in utility.


    Version 2.12
    Jan C. Depner
    09/29/06

    Added depth to updateTarget dialog.


    Version 2.13
    Jan C. Depner
    01/04/07

    Fixed shared file stuff when there were extra non-keyword arguments after the 
    last keyword when using --shared_file.


    Version 2.14
    Jan C. Depner
    01/16/07

    Only send heading to targetPic for UNISIPS images.  The LIDAR ones may already be rotated.


    Version 2.15
    Jan C. Depner
    01/25/07

    Make sure heading for target is 0-360 (no negatives, Wade's XML stuff freaks).


    Version 3.0
    Jan C. Depner
    04/11/07

    Qt 4 port.


    Version 3.01
    Jan C. Depner
    05/30/07

    Added alpha values to saved colors.


    Version 3.02
    Jan C. Depner
    06/25/07

    Switched to using GDAL to read TIFF info and QIMage to read TIFF data.


    Version 3.03
    Jan C. Depner
    07/31/07

    Added ability to apply offset in addition to scale.  Made size of displayed points user
    selectable.


    Version 3.04
    Jan C. Depner
    08/02/07

    Added shift + arrow keys accelerators for plan, east, and south views.  Removed accelerators
    from "view by" buttons.  Add accelerators to hot key polygon and edit target buttons.


    Version 3.05
    Jan C. Depner
    08/06/07

    Use MEDIUM_TARGET instead of SHORT_TARGET to speed up search and display of targets.


    Version 3.06
    Jan C. Depner
    08/21/07

    Mark targets as circles instead of squares.


    Version 3.07
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 3.08
    Jan C. Depner
    09/12/07

    Fixed minor bug - didn't display proper lat and lon when snapping to nearest point or target.


    Version 3.09
    Jan C. Depner
    09/17/07

    Added EGM96 button to "offset" in prefs.


    Version 3.10
    Jan C. Depner
    09/18/07

    Don't display target polygons unless in plan view.


    Version 3.11
    Jan C. Depner
    09/26/07

    Fix bug in displayLines.cpp - clear old selected lines.


    Version 3.12
    Jan C. Depner
    09/28/07

    Added GeoTIFF display toggle button.


    Version 3.13
    Jan C. Depner
    10/09/07

    Added mosaic viewer program to ancillary progs (mosaicView).


    Version 3.14
    Jan C. Depner
    10/15/07

    Tracks cursor in other ABE applications.


    Version 3.15
    Jan C. Depner
    10/30/07

    Fix hotkey polygon bug caused by allowing "no data type" hotkeys to work in any mode.


    Version 4.00
    Jan C. Depner
    12/03/07

    Replaced POSIX shared memory with QSharedMemory.  Moved abe_share and share to MISC structure.  IVS Fledermaus
    will not be able to shell this program unti we can get them to switch to QSharedMemory.  Added fflush calls
    after prints to stderr since flush is not automatic in Windows.  Honors the CHILD_PROCESS_FORCE_EXIT key.
    Modifies the set_defaults.cpp file on build to reflect the correct location of the extended help file.
    Added code to find the nearest MAX_STACK_POINTS (9) points to the cursor for use by the waveWaterfall program.


    Version 4.01
    Jan C. Depner
    12/06/07

    Added tofWaterKill to ancillary programs.  Made a nicer warning message for invalid hot key presses.


    Version 4.02
    Jan C. Depner
    12/11/07

    Changed default toolbar locations.  Changed QWidgetItem to QCheckBox to deal with bug in Qt 4.4 snapshot.


    Version 4.03
    Jan C. Depner
    12/28/07

    Uses multiple movable objects as added to nvMap.


    Version 4.10
    Jan C. Depner
    01/04/08

    Now uses the parent process ID plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 4.20
    Jan C. Depner
    01/08/08

    Ancillary programs are no longer modifiable, only hot keys and action keys.  It just got too damn 
    complicated.  Ctrl-arrow keys move the edited area in pfmView.  This forces a save of all changes.


    Version 4.21
    Jan C. Depner
    01/14/08

    Uses check_target_schema (from utility library) to make sure that the schema in target files is correct.


    Version 4.22
    Jan C. Depner
    01/16/08

    Got rid of the polygonTimer function.  Now uses the trackCursor timer to reset the polygon_timer flag since
    trackCursor is now always active.  Fixed problem in nvmap.cpp with angle for movingPath objects.


    Version 4.23
    Jan C. Depner
    01/18/08

    Added shoreline depth swap killer mode for chartsSwap ancillary program.


    Version 4.24
    Jan C. Depner
    01/23/08

    Fix problem with savng the last target description and remarks for use in the next update target dialog.


    Version 4.25
    Jan C. Depner
    03/14/08

    Added data types to the "select lines to view" dialog.


    Version 4.26
    Jan C. Depner
    03/17/08

    Added the ability to use minimum Z window vaues computed using IHO TVU based on order.  Happy Saint Patrick's Day!


    Version 4.27
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 5.00
    Jan C. Depner
    04/03/08

    Dealing with index attributes in the proper manner.


    Version 5.01
    Jan C. Depner
    04/04/08

    Had to check for attributes when clicking on color by line/depth/distance buttons otherwise
    I'd try to set the bAttr icon - which isn't there.  DOH!


    Version 5.02
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 5.03
    Jan C. Depner
    04/24/08

    Cleaned up the way extended help was being handled.


    Version 5.04
    Jan C. Depner
    06/02/08

    Added H/V errors to data and color_by_attribute.


    Version 5.05
    Jan C. Depner
    06/10/08

    Tells pfmView when we request a mosaicView zoom so that pfmView can tell any linked apps that
    a zoom has been requested.


    Version 6.00
    Jan C. Depner
    07/15/08

    Now handles multiple PFM layers from pfmView.  Stopped using the shared memory from pfm.h (now 
    use the shared memory from ABE.h).


    Version 6.01
    Jan C. Depner
    09/15/08

    Fixed attribute handling for multiple PFMs when one of the PFMs does not have data to be edited.


    Version 6.10
    Jan C. Depner
    10/31/08

    Added support for Gary Morris' new waveMonitor program and removed the waveWaterfall support since waterfall mode
    is supported in the new waveMonitor program.


    Version 6.20
    Jan C. Depner
    11/06/08

    Broke two toolbars up into 6 toolbars.  Do a better job of saving toolbar and window positions and sizes.
    Added ability to change tool button icon sizes.  Added View menu so that if you turn off all of the tool bars
    you can easily figure out how to turn them back on again (right click in the menu bar gives the same menu).
    Added Edit menu with Preferences entry.


    Version 6.21
    Jan C. Depner
    11/24/08

    Changed default action keys for waveMonitor (n is now a toggle for nearest neighbor and single waveform
    display modes).


    Version 6.22
    Jan C. Depner
    11/25/08

    Replaced charts_list with chartsMonitor and removed the "extended" (x) action key from the waveMonitor defaults.


    Version 6.23
    Jan C. Depner
    11/25/08

    Lock the file and line names when we freeze the cursor.


    Version 6.30
    Jan C. Depner
    12/02/08

    Added data masking, measure tool, kill_switch from pfmEdit3D.


    Version 6.31
    Jan C. Depner
    12/03/08

    Added the ability to shell the pfmEdit3D from here.


    Version 6.32
    Jan C. Depner
    12/04/08

    Now sets PFM_SELECTED_FEATURE on points that are associated with a target.


    Version 6.40
    Jan C. Depner
    12/08/08

    Added transparency and controls to slicing.


    Version 6.41
    Jan C. Depner
    12/12/08

    Allow hot keys in MEASURE mode.


    Version 6.42
    Jan C. Depner
    12/15/08

    Added screenshot grabber for adding and editing targets.  Also, only unfreeze if we delete the point
    or we explicitly unfreeze by pressing f or F.  Do not unfreeze on mode change.


    Version 6.43
    Jan C. Depner
    12/16/08

    Added right click menu option when in SET_REFERENCE or UNSET_REFERENCE mode to allow you to delete
    all visible, non-transparent reference points.


    Version 6.44
    Jan C. Depner
    12/22/08

    Removed transparent display of sliced data.  It just doesn't work as well in 2D as it does in 3D.


    Version 6.45
    Jan C. Depner
    01/09/09

    Fixed screwup when running the 3D editor from here.  I was losing edits already done in the
    2D editor when I went to the 3D editor.


    Version 6.46
    Jan C. Depner
    02/03/09

    Added option to consolidate targets within a target polygon.


    Version 6.47
    Jan C. Depner
    02/18/09

    Fixed the extended help feature to use the PFM_ABE environment variable to find the help file.
    Also, hard-wired the browser names in Windoze and Linux.


    Version 6.48
    Jan C. Depner
    03/13/09

    Minor change to allow waveWaterfall and chartsPic to handle WLF data.


    Version 6.49
    Jan C. Depner
    03/20/09

    Brought back the old waveMonitor as waveformMonitor at the request of the ACE.


    Version 6.50
    Jan C. Depner
    04/01/09

    Removed support for NAVO standard target (XML) files and replaced with support for
    Binary Feature Data (BFD) format files.  This is not an April Fools joke ;-)


    Version 6.51
    Jan C. Depner
    04/08/09

    Changes to deal with "flush" argument on all nvmapp.cpp (nvutility library) drawing functions.


    Version 6.52
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 6.53
    Jan C. Depner
    04/14/09

    Clear misc.poly_count after setting up 3D edit because it screwed up check_bounds after the edit was finished.


    Version 6.54
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 6.55
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 6.56
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 6.57
    Jan C. Depner
    05/22/09

    Dealing with multiple resize events when "Display content in resizing windows" is 
    enabled.


    Version 6.58
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 6.59
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 6.60
    Jan C. Depner
    07/17/09

    Fixed dateline problem with rectangles and polygons.


    Version 6.61
    Jan C. Depner
    07/28/09

    Changed use of _sleep to Sleep on MinGW (Windows).


    Version 6.62
    Jan C. Depner
    07/29/09

    Added tool tips to tool bars.
    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 6.63
    Jan C. Depner
    08/18/09

    Made mask and hide options work like the old zoom-in stuff.  That is, masked data is not used
    to determine the bounds to be displayed.


    Version 6.64
    Jan C. Depner
    08/20/09

    Added hofReturnKill program with -s option to kill Shallow Water Algorithm and/or Shoreline Depth Swapped data.
    Turn off delete type functions when displaying invalid data (added NOOP mode).
    Mark invalid points with an X when displaying invalid data.


    Version 6.65
    Jan C. Depner
    08/26/09

    Added filter masks, filter button, and area-based filtering to the editor.


    Version 6.66
    Jan C. Depner
    08/27/09

    Added message when filter doesn't find any points to delete.


    Version 6.70
    Gary Morris (USM), Jan C. Depner
    09/09/09

    Added LIDAR Attribute Viewer support.  Added LIDAR tool bar.  Added push buttons to start LIDAR monitoring
    programs (like waveMonitor) to the LIDAR tool bar.


    Version 6.71
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 6.72
    Jan C. Depner
    09/16/09

    Made new blinkTimer for mask/unmask buttons.  Added check for kill of ancillary programs that are attached
    to buttons.


    Version 6.73
    Jan C. Depner
    09/22/09

    Allow EDIT_FEATURE to be a saved edit function so that the editor may come up in that mode on start.


    Version 6.74
    Jan C. Depner
    09/23/09

    Added ability to "verify" (set confidence to 5) all displayed valid features.


    Version 6.75
    Jan C. Depner
    09/28/09

    Bug fix in code to support attributeViewer.


    Version 6.76
    Jan C. Depner
    10/22/09

    Cleaned up hot key handling.


    Version 6.77
    Jan C. Depner
    11/02/09

    Moved the attributeViewer functions to their own .cpp file.


    Version 6.78
    Jan C. Depner
    11/12/09

    Added delete records (pings) in rectangle or polygon.  Only available from the right mouse menu.
    Made data flagging work like it does in pfmEdit3D - only one thing at a time.


    Version 6.80
    Jan C. Depner
    11/18/09

    Made "color by" options match pfmEdit3D.


    Version 6.81
    Jan C. Depner
    12/02/09

    Replaced get_egm96 call with get_egm08 call.


    Version 6.82
    Jan C. Depner
    12/11/09

    Added "Edit return status:" to the printf calls that return info to pfmView.


    Version 6.90
    Jan C. Depner
    12/17/09

    Added 100 to 1,000,000 undo levels - woo hoo!


    Version 6.91
    Jan C. Depner
    12/22/09

    Fix stupid flag-invalid bug.


    Version 6.92
    Jan C. Depner
    12/28/09

    Fixed bug in slotMouseMove.  I wasn't checking to see if a rubberband construct existed prior to trying
    to drag it.  Also, added filter standard deviation button.


    Version 6.93
    Jan C. Depner
    12/29/09

    Stopped the filter from filtering points when there is not enough surrounding valid data.


    Version 6.94
    Jan C. Depner
    01/05/10

    Made filter confirmation optional.  Replaced all calls to the "inside" function with calls to
    "inside_polygon2".  This is supposed to be faster although I couldn't see any major improvement.


    Version 6.95
    Jan C. Depner
    01/06/10

    Fixed the filter so that it uses the points in memory (and their validity from whatever editing has been
    done) instead of reading the data fomr the PFM.  I just stole the filter code from pfmView and in that
    case it was OK to read from the PFM since changes made there are automatically updated in the PFM.
    In the editor the changes don't happen until you exit, so here we have to use the data in memory
    and make pseudo bins in memory to do the proper thing.


    Version 6.96
    Jan C. Depner
    01/14/10

    Fixed filter undo problem.


    Version 6.97
    Jan C. Depner
    01/26/10

    Replaced geoSwath call with gsfMonitor.  The geoSwath program was sort of useless in this context anyway.


    Version 7.00
    Jan C. Depner
    01/27/10

    Added right click menu for all non-monitor type ancillary programs when in DELETE_POINT mode or
    HOTKEY_POLYGON mode.


    Version 7.01
    Jan C. Depner
    02/17/10

    Wasn't checking against rectangle in get_buffer.  If we had a large bin size this caused all kinds of grief - the area you got
    in the editor was much larger than what you asked for.


    Version 7.02
    Jan C. Depner
    02/25/10

    Added support for running in read only mode (from pfmWDBView via pfmView).


    Version 7.03
    Jan C. Depner
    03/02/10

    Moved the filter message box to the bottom of the screen so it wouldn't get in the way.


    Version 7.10
    Jan C. Depner
    03/03/10

    Removed the ability to shell the 3D editor from the 2D editor.  It became almost impossible to save state between the
    two, especially after I added the undo levels.


    Version 7.11
    Jan C. Depner
    03/09/10

    Added special translators for time and datum attributes from PFMWDB.


    Version 7.12
    Jan C. Depner
    03/24/10

    Finally found the bug that caused the points to be plotted out of bounds when you first switch to one of the
    alternate views (i.e. not plan view).


    Version 7.13
    Jan C. Depner, Micah Tinkler
    04/02/10

    Added ability to filter mask after running the filter while the filter confirmation box is still up.  This allows you to
    look at the points the filter has picked and mask some out prior to accepting the filter (it reruns the filter and 
    regenerates the filter confirmation dialog with the new filter count).  Filter confirmation is no longer optional.
    Standard deviation can be set in the filter confirmation dialog.


    Version 7.14
    Jan C. Depner, Micah Tinkler
    04/09/10

    When the filter message dialog is displayed DELETE_POINT mode snaps to filter kill points only.  Surrounding points
    used for waveWaterfall and other ancillary programs still come from all valid points.  Replaced hot keys for 
    KEEP_POLYGON, RESTORE_RECTANGLE, and RESTORE_POLYGON with hotkeys for FILTER, RECT_FILTER_MASK, and POLY_FILTER_MASK.
    Also, made the Enter key trigger the filter message dialog Accept action if the filter message dialog is present.


    Version 7.20
    Jan C. Depner
    04/14/10

    Added optional color scale to the right of the main window.


    Version 8.00
    Jan C. Depner
    04/20/10

    Completely rearranged the GUI.  Moved the status info and color scale to the left side.  Moved the slicer to the
    right side.


    Version 8.01
    Jan C. Depner
    04/21/10

    Added ability to use Shift/mouse wheel to modify the filterMessage_dialog standard deviation value (had to play timer games to make it work right).


    Version 8.02
    Jan C. Depner
    04/22/10

    Added ability to mark files for deletion using the right click menu in DELETE_POINT mode.  Files are queued for deletion in pfmView using
    the Edit menu.


    Version 8.03
    Jan C. Depner
    05/03/10

    Added a filter results tabbed widget in the upper left of the main window.  This looks just like the one in pfmEdit3D and it
    gets rid of the separate filter message dialog.


    Version 8.04
    Jan C. Depner
    05/04/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 8.05
    Jan C. Depner
    06/03/10

    Added ability to display either manually invalid, filter invalid, or both instead of just displaying any invalid data.
    This helps a lot when processing LIDAR data since GCS invalidated points are marked as PFM_FILTER_INVAL while points removed
    in the editor or viewer by the filter (or manually) are marked as PFM_MANUALLY_INVAL.


    Version 8.06
    Jan C. Depner
    06/04/10

    Can now invalidate features in hotkey polygon mode.


    Version 8.07
    Jan C. Depner
    06/25/10

    Added support for displaying PFM_HAWKEYE_DATA in the chartsMonitor ancillary program.


    Version 8.08
    Jan C. Depner
    06/28/10

    Changed chartsMonitor to lidarMonitor due to multiple lidar support.


    Version 8.09
    Jan C. Depner
    07/06/10

    HSV min and max colors are now passed in on the command line so that they can be set by pfmView or CZMIL.  Also,
    HSV min and max locked values can be optionally passed in on the command line.


    Version 8.10
    Jan C. Depner
    07/09/10

    Fixed resize bug when first entering window in DELETE_POINT mode.


    Version 8.11
    Jan C. Depner
    09/02/10

    Added ability to use the text search string, text search invert, and text search type from pfmView.


    Version 8.12
    Jan C. Depner
    09/08/10

    Changes to handle Qt 4.6.3.


    Version 8.13
    Jan C. Depner
    09/17/10

    Fixed bug displaying feature info.


    Version 8.14
    Gary Morris (USM)
    09/20/10

    Merge of CZMIL changes.


    Version 8.15
    Jan C. Depner
    09/22/10

    Added markers to feature polygon points when displaying feature polygons.


    Version 8.16
    Jan C. Depner
    09/24/10

    Fixed contour levels dialog in prefs.


    Version 8.17
    Jan C. Depner
    10/08/10

    Removed select/unselect soundings options since we don't do that in PFM_ABE anymore.
    You can still flag them though.


    Version 8.18
    Jan C. Depner
    10/20/10

    Fixed "mask outside" so that it acts like a zoom in.


    Version 8.19
    Jan C. Depner
    12/03/10

    Added rmsMonitor to ancillary programs.  This program reads smrmsg files associated with HOF or TOF and displays the
    RMS values for the record.


    Version 8.20
    Jan C. Depner
    12/06/10

    Cleaned up buttonText, buttonAccel, buttonIcon, and button array handling.  Made selection of multiple lines  work
    in the display instead of having a list of the lines.


    Version 8.21
    Jan C. Depner
    12/09/10

    Fixed the slice and size bar buttons (finally).


    Version 8.22
    Jan C. Depner
    12/14/10

    We now allocate the memory for the highlight/mark points.  Also, added --force_auto_unload option for CZMIL.


    Version 8.23
    Jan C. Depner
    12/16/10

    Can now highlight and un-highlight in polygons.  Also, pressing the Enter key when data is highlighted will
    invalidate the highlighted points.


    Version 8.24
    Jan C. Depner
    12/20/10

    Properly define the area of interest when we start the program.  In other words, even if the user requested
    huge expanses of empty area, give it to him.


    Version 8.25
    Jan C. Depner
    01/03/11

    Removed displaySingle button and replaced with undisplaySingle (i.e. hide single).  Changed operation of 
    displayMultiple to select last line with double click instead of just ending displayMultiple mode.


    Version 8.26
    Jan C. Depner
    01/04/11

    Added new color array for color by line mode.  It now uses twice the number of colors.  The color wheel
    values go from 200 to 0/360 to 300 twice.  The first half of the color array uses value set to 255 (full color).
    The second half uses value set to 127 (darker).  This gives us a better color spread in order to differentiate
    between lines.


    Version 8.27
    Jan C. Depner
    01/05/11

    Added hotkey check in the preferences dialog to prevent conflicts.  I really should have done this a long time
    ago but it was way down on the priority list ;-)


    Version 8.28
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 8.29
    Jan C. Depner
    01/16/11

    Positioned non-modal popup dialogs in the center of the screen.


    Version 8.30
    Jan C. Depner
    02/10/11

    Removed the chartsSwap program from the list of ancillary programs.  We don't do that anymore.


    Version 8.31
    Gary Morris (USM), Jan C. Depner
    02/18/11

    Merged Gary's changes to support updates to the attributeViewer.


    Version 8.32
    Jan C. Depner
    02/22/11

    Added hawkeyeMonitor to ancillary programs.


    Version 8.33
    Jan C. Depner
    03/08/11

    Fixed up arrow slice bug on start.


    Version 8.34
    Jan C. Depner
    03/09/11

    Strip /PFMWDB:: off of input files in the ancillary programs stuff if we're working on the PFM World Data Base (PFMWDB).
    Just in case we might want to move input files to the PFMWDB directory and work on them.


    Version 8.35
    Jan C. Depner
    03/28/11

    Allow user to define the key for invalidating filter highlighted or manually highlighted data.  Try to save the user's
    hot key definitions even across setting version changes.


    Version 8.36
    Jan C. Depner
    04/06/11

    Brought back the displayLines list as a right click option when in SET_MULTIPLE mode.  Request from Paul Marin.


    Version 8.37
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks are now handled in ABE shared memory and are no longer read in as
    arguments from pfmView.


    Version 8.38
    Jan C. Depner
    04/11/11

    Properly populate the file name when creating a new feature file.  It was leaving it blank on Windows.


    Version 8.40
    Jan C. Depner
    04/14/11

    Added the externalFilter function to support the (newly added) hofWaveFilter ancillary program.  Removed tofWaterKill.


    Version 8.41
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.


    Version 8.50
    Jan C. Depner
    04/26/11

    The point cloud memory (POINT_CLOUD structure) is now allocated in shared memory.  This leaves a little slop around the 
    edges of the area to be edited and we have to pre-scan the bins to get the count but it will save a ton of memory and
    a bunch of I/O when running ancillary programs that need a large amount of data passed to them (mostly hofWaveFilter).
    There doesn't seem to be much of a hit in terms of speed when we do this.


    Version 8.51
    Gary Morris (USM)
    04/27/11

    Fixes to color handling in attributeViewerFunctions.cpp.


    Version 8.60
    Jan C. Depner
    04/28/11

    Flagging data points now makes the points "highlighted".  This allows you to delete flagged points.  You can also now
    restore highlighted invalid data points.


    Version 8.61
    Jan C. Depner
    05/05/11

    Filters (including hofWaveFilter) now honor the mask flag.  Happy Cinco de Mayo!!!


    Version 8.62
    Jan C. Depner
    05/09/11

    Fixed problem with pre-existing shared point cloud memory.  The memory may not have been detached if a previous
    editor process crashed.  Now it attaches the pre-existing memory, detaches it, and then creates a new point cloud 
    shared memory segment.  Added invert highlight function and button.


    Version 8.63
    Jan C. Depner
    05/11/11

    Don't display features that are outside of the slice.


    Version 8.64
    Jan C. Depner
    05/16/11

    Added ability to lock min/max colors and values when coloring by attribute.


    Version 8.65
    Jan C. Depner
    05/18/11

    Fixed hide and show problems (due to incorrectly determining unique data types) when more than one PFM 
    is being edited.  Moved mask responsibility from hofWaveFilter back here.  Desensitize hide and show
    "highlighted" checkboxes in hide and show menus when no points are highlighted.  Fixed return from hofReturnKill,
    it was missing the "line" value in the sscanf of the shared file.


    Version 8.66
    Jan C. Depner
    05/20/11

    Added pmt and apd ac zero offset thresholds to the hofWaveFilter parameters.


    Version 8.67
    Jan C. Depner
    05/26/11

    Added highlight record (ping, shot) right click popup menu option.


    Version 8.68
    Jan C. Depner
    06/13/11

    Fix bug with attribute filter max and min values.


    Version 8.69
    Jan C. Depner
    06/27/11

    Fix bug when editing multiple PFMs.  Added menu to allow displaying of all, unverified, verified, or no features
    (as opposed to just on/off).


    Version 8.70
    Jan C. Depner
    07/08/11

    Fixed slice bug that was causing the slice to be one slice too far along when we started slicing.


    Version 8.71
    Jan C. Depner
    07/15/11

    Fixed bug changing minimum color for color-by options.


    Version 8.72
    Jan C. Depner
    07/19/11

    Added QMessageBox output for memory allocation error in get_buffer.c.


    Version 8.73
    Jan C. Depner
    07/22/11

    Fixed ancillary program button state bug.  If pfmEdit3D was started with ancillary programs up
    (state set to 2) it was resetting the state to 1 because the QProcess wasn't associated with
    the current instantiation of pfmEdit3D.  Now it checks for the state being 2 (already running)
    as well.


    Version 8.74
    Jan C. Depner
    07/29/11

    Added input of "pfmView killed" qsetting so that pfmEdit will know that pfmView was shut down
    since the last time it was running.  This allows us to properly set the "monitor" type ancillary
    program "running" states since pfmView issues a kill command to all of the associated ancillary
    programs when it exits.


    Version 8.75
    Jan C. Depner
    08/03/11

    Added CZMIL support in get_feature_event_time.cpp.


    Version 8.76
    Gary Morris (USM)
    08/11/11
	
    Added an AV Distance Threshold toolButton tied to the Attribute Viewer running.
    Implemented a new handshake between Attribute Viewer and Editor to select user-defined attribute color codes.
    Allowed for talking to multiple type attribute viewers.
	

    Version 8.77
    Jan Depner
    08/16/11
	
    Fix data type lookup table in get_buffer.cpp when there's more than 1 PFM open.
	

    Version 8.78
    Jan Depner
    08/31/11
	
    Changed waveMonitor to CZMILwaveMonitor.
	

    Version 8.79
    Jan Depner
    10/24/11
	
    Added fix for invalid highlights not showing up when you delete data while displaying
    invalid with invalid data flagged.  Also added ability to change the edit and marker
    colors in the preferences dialog.

</pre>*/
