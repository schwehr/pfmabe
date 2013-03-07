
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef NVUTILITY_VERSION

#define     NVUTILITY_VERSION     "PFM Software - nvutility library V2.1.22 - 11/04/11"

#endif

/*!< <pre>

    Version 1.00
    04/01/08
    Jan C. Depner

    This is the first version of the nvutility library.  April Fools!  Actually, this is just
    the first time we've had a version file.  The first version of nvutility was somewhere back
    in 1998.


    Version 1.01
    04/07/08
    Jan C. Depner

    Added bit_test and bit_set to chrtr.c.  Added get_string.c.  Added nvutility.h which contains
    includes of all of the .h and .hpp files.  Removed getopt_win.h, getopt1.c, and getopt.c (not used).
    Had to change the POINT structure name to CONTOUR_POINT in globals.hpp because POINT is
    already defined in MinGW.


    Version 1.02
    04/08/08
    Jan C. Depner

    Removed geod.c, geod.h, and ellipsoid.h.  These were a major problem in Windows and in all but one
    case they were in the application directories anyway.


    Version 1.03
    04/09/08
    Jan C. Depner

    Added registerABE.cpp, registerABE.hpp, unregisterABE.cpp, unregisterABE.hpp, changeFileRegisterABE.cpp, and
    changeFileRegisterABE.hpp.


    Version 1.04
    04/24/08
    Jan C. Depner

    Fixed a small probelm in check_target_schema.c


    Version 1.05
    04/25/08
    Jan C. Depner

    Leap year bug in inv_cvtime.


    Version 1.06
    05/20/08
    Jan C. Depner

    Modified get_area_mbr to adjust west longitudes to positive if the area crosses the dateline.  Also,
    made read_srtm_mask and read_coast deal with the 0-360 world correctly.


    Version 1.07
    06/09/08
    Jan C. Depner

    Fix one-off error in nvmap.cpp when reading the landmask.  Also, modified the comments in read_srtm_mask.c.


    Version 1.08
    06/10/08
    Jan C. Depner

    Added command capability to ABE_register.hpp.


    Version 1.09
    07/02/08
    Jan C. Depner

    Added NV_I32_COORD2 and NV_F64_COORD2 versions of drawPolygon to nvmap.cpp.


    Version 1.10
    07/14/08
    Jan C. Depner

    nvmap::resetBounds wasn't resetting initial_bounds in addtion to bounds[0].


    Version 1.11
    07/21/08
    Jan C. Depner

    get_area_mbr wasn't reading afs files properly.


    Version 1.12
    09/11/08
    Jan C. Depner

    movable_object_count was getting hosed in nvmap.cpp.


    Version 2.0.0
    09/17/08
    Jan C. Depner

    Had to make it a static library on Windoze due to my inability to make a shared version of 
    the target library.  Google for __declspec and MINGW to see what's happening.


    Version 2.0.1
    10/02/08
    Jan C. Depner

    Added drawTextClear, getZoomLevel, and clipLine functions to nvmap.cpp.  Fixed MANY dateline issues.


    Version 2.0.2
    10/08/08
    Jan C. Depner

    More dateline insanity in nvmap.cpp.  The comment looks like this:

      //  Arbitrary cutoff - if we're crossing the dateline and one point is negative and the other is >=0
      //  and either point is more than 90 degrees outside of the boundaries we're going to say that this line
      //  is completely outside of the area.  If you want to do really large areas that cross the datelin you
      //  need to use 0-360 world always.


    Version 2.0.3
    11/24/08
    Jan C. Depner

    Added squat.cpp, squat.hpp, and vec.h.  These are used for 3D OpenGL programs
    like pfm3D and pfmEdit3D.


    Version 2.0.4
    12/02/08
    Jan C. Depner

    Fixed bug in the rubberband line code of nvmap.cpp and nvpic.cpp.


    Version 2.0.5
    12/03/08
    Jan C. Depner

    Fixed check_target_schema bug.


    Version 2.0.6
    01/08/09
    Jan C. Depner

    More dateline stupidity.  This time in zoomOutPercent.


    Version 2.0.7
    02/11/09
    Jan C. Depner

    Added a couple of new flags to ABE.h.  PFMVIEW_FORCE_EDIT, PFMVIEW_FORCE_EDIT_3D, PFMEDIT3D_OPENED, and
    PFMEDIT3D_CLOSED.  See the comments in ABE.h.


    Version 2.0.8
    02/24/09
    Jan C. Depner

    Bug fix for get_geoid03.c


    Version 2.0.9
    04/08/09
    Jan C. Depner

    Modified nvmap.cpp to have an NV_BOOL "flush" argument for all drawing routines.


    Version 2.0.10
    04/27/09
    Jan C. Depner

    Added inside_polygon and inside_polygon2.


    Version 2.0.11
    05/21/09
    Jan C. Depner

    Added nvMapGl.cpp and .hpp after removing them from the pfm3D, pfmEdit3D, and bagViewer directories.


    Version 2.0.12
    05/22/09
    Jan C. Depner

    Removed the redrawMapWithCheck call from the resize event.  This should be handled by the 
    application, not the library.


    Version 2.0.13
    06/02/09
    Jan C. Depner

    Added setMovingText to nvmap.cpp.


    Version 2.0.14
    06/18/09
    Jan C. Depner

    Replaced double_bit_pack and double_bit_unpack in bit_pack.c with improved code from Will Burros (IVS).


    Version 2.0.15
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 2.0.16
    Jan C. Depner
    08/17/09

    Added a change to nvMapGL.cpp to only return fully opaque points when performing a 
    nearest point check.  This is used for slicing in pfmEdit3D.


    Version 2.0.17
    Jan C. Depner
    08/20/09

    Added drawLine to nvMapGL.cpp.  This allows you to draw a 2D line over the 3D data.


    Version 2.0.18
    Jan C. Depner
    10/28/09

    Added dummyBuffer to nvmap.cpp.  Added check_srtm_mask to read_srtm_mask.c.


    Version 2.0.19
    Jan C. Depner
    11/19/09

    Added polygon_is_rectangle and polygon_is_rectangle2 to get_area_mbr.c.


    Version 2.0.20
    Jan C. Depner
    12/02/09

    Added get_egm08.c.


    Version 2.0.21
    Jan C. Depner
    12/09/09

    Added WA_DELETE_ON_CLOSE to nvMapGL.cpp.


    Version 2.0.22
    Jan C. Depner
    12/11/09

    Added polygon_collision functions to inside_polygon.c and inside_polygon.h.


    Version 2.0.23
    Jan C. Depner
    01/07/10

    Fixed bug in read_srtm_mask.cpp.


    Version 2.0.24
    Jan C. Depner
    01/11/10

    Fixed equator bug in read_srtm_mask.cpp.


    Version 2.0.25
    Jan C. Depner
    01/19/10

    Fixed yet another equator bug in read_srtm_mask.cpp.


    Version 2.0.26
    Jan C. Depner
    02/02/10

    Increased the number of points in an AREA structure to 1000.  AFAIK this is only used in apply_tides.


    Version 2.0.27
    Jan C. Depner
    02/09/10

    Added sunshade.cpp and survey.cpp.


    Version 2.0.28
    Jan C. Depner
    03/16/10

    Added polygon_intersection.c and polygon_intersection.h.


    Version 2.0.29
    Jan C. Depner
    04/09/10

    Added NVMAP_UP_LEFT, NVMAP_UP_RIGHT, NVMAP_DOWN_LEFT, and NVMAP_DOWN_RIGHT to the possible move directions in
    nvmap.cpp, nvmap.hpp, nvMapGL.cpp, and nvMapGL.hpp.


    Version 2.0.30
    Jan C. Depner
    04/14/10

    Added scaleBox.cpp and scaleBox.hpp.  This is a Qt widget for drawing colored, numbered scale boxes.


    Version 2.0.31
    Jan C. Depner
    04/29/10

    Changed fixpos to use numeric constants instead of character strings to set type and format.  Same with rproj and read_coast.
    Changed many "NV_CHAR *" arguments to "const NV_CHAR *" arguments for gcc 4.4 compatibility (when calling with a constant).


    Version 2.0.32
    Jan C. Depner
    06/01/10

    Fixed contour default initializations in contour.cpp.


    Version 2.0.33
    Jan C. Depner
    06/07/10

    Added geo_distance.c (stolen from PFM library and made generic).


    Version 2.0.34
    Jan C. Depner
    07/07/10

    Modified scaleBox to include a flag for drawing a lock in the scaleBox (on the right).


    Version 2.0.35
    Jan C. Depner
    07/19/10

    Added inside_mbr and inside_xymbr to inside.c.


    Version 2.0.36
    Jan C. Depner
    07/22/10

    Added martin.c and convolve.c to support CHRTR2 gridEdit program.


    Version 2.1.0
    Gary Morris (USM)
    09/20/10

    Updates to merge in latest CZMIL changes from Gary Morris (USM)


    Version 2.1.1
    Jan C. Depner
    09/24/10

    Added clip_lat_lon to line_intersection.c.


    Version 2.1.2
    Jan C. Depner
    10/22/10

    Fixed memory leak in nvpic.cpp - I hope.


    Version 2.1.3
    Jan C. Depner
    11/05/10

    Finally fixed auto Z scaling in nvMapGL.  Removed warning to stderr in read_coast.c when it can't
    find coast50k.ccl since it is restricted and might not be available.


    Version 2.1.4
    Jan C. Depner
    11/30/10

    Finally added a real scale to the 3D stuff in nvMapGL.cpp.


    Version 2.1.5
    Jan C. Depner
    12/14/10

    Moved the scale to the background in nvMapGL.cpp.  Added clearMarkerPoints and setMarkerPoints to nvMapGL.cpp so 
    that we can finally mark points nicely.


    Version 2.1.6
    Jan C. Depner
    12/22/10

    Added unlock icon to scaleBox.cpp.  We now have three modes -1 = unlocked, 0 = no lock, 1 = locked.


    Version 2.1.7
    Jan C. Depner
    01/05/11

    DOH!  I missed a couple of %lf fields in printf statements (see version 2.0.15 of 07/29/09).


    Version 2.1.8
    Jan C. Depner
    01/07/11

    Added getFaux3DCoords to nvMapGL in order to get the distance, azimuth, and delta Z of the cursor when measuring in 3D
    point editors.


    Version 2.1.9
    Jan C. Depner
    01/15/11

    Fixed bug in resize code of nvMapGL.cpp.  I gotta learn to typecast ints to floats when I do math ;-)


    Version 2.1.10
    Jan C. Depner
    01/27/11

    Fixed bug in find_startup.c and find_startup_name.c.  The last directory in the PATH wasn't being searched.


    Version 2.1.11
    Jan C. Depner
    01/28/11

    Fixed a light ambience problem introduced by Gary's setRenderModeAmbience stuff.  We don't want to do that when we're
    drawing a 3D surface, as opposed to dicrete points.


    Version 2.1.12
    Gary Morris (USM)
    02/18/11

    Turned off lighting when in point mode.  It makes the colors match better when you go to a non-OpenGL program.  
    Hopefully it's also a bit faster.


    Version 2.1.13
    Jan Depner
    02/21/11

    Removed references to lat and lon from nvMapGL.cpp and nvMapGL.hpp since we're not always dealing with lat and lon.
    Sometimes it's northing and easting or it may be just X and Y.


    Version 2.1.14
    Jan Depner
    03/30/11

    Added setAutoScale to nvMapGL.cpp and nvMapGL.hpp so we can turn auto-scaling on and off at will.


    Version 2.1.15
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks for pfmView, pfmEdit(3D), and pfmEditShell have been added to ABE.h (ABE
    shared memory).


    Version 2.1.16
    Jan C. Depner
    04/21/11

    Modified get_egm_08 to return -1 for an error when trying to set the grid type.


    Version 2.1.17
    Jan C. Depner
    05/09/11

    Added getMarker3DCoords to nvMapGL.cpp so we could find the nearest "highlighted" (i.e. marker) point in
    pfmEdit3D.  The lat and lon returned by mouseMoveEvent is the nearest of all displayed points when in 
    NVMAPGL_POINT_MODE.  Due to that we needed to be able to find the nearest marker point in the slot
    connected to the nvMapGL mouseMoveSignal (slotMouseMove in pfmEdit3D).


    Version 2.1.18
    Jan C. Depner
    05/20/11

    Added pmt and apd ac zero offset thresholds to the hofWaveFilter parameters in the ABE.h file.  Note that this
    is a change to shared memory and may cause temporary problems until ipcclean is run or the syatem is rebooted.


    Version 2.1.19
    Jan C. Depner
    07/14/11

    Fixed bug in nvmap.cpp when using NO_PROJECTION.  It was still checking dateline.  DOH!


    Version 2.1.20
    Jan C. Depner
    07/22/11

    Added setSidebarUrls.cpp and .hpp.


    Version 2.1.21
    Jan C. Depner
    10/28/11

    Added getSystemInfo.cpp and .hpp.


    Version 2.1.22
    Jan C. Depner
    11/04/11

    Fixed bug in nvMapGL.cpp when coloring by something other than depth/elevation.

</pre>*/
