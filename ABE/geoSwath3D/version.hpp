
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

#define     VERSION     "PFM Software - geoSwath3D V3.14 - 07/22/11"

#endif

/*

    Version 1.00
    Jan C. Depner
    01/01/10

    First working version.


    Version 1.01
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 1.02
    Jan C. Depner
    06/25/10

    Added support for HAWKEYE CSS Generic Binary Output Format data. Also, added support for displaying
    PFM_HAWKEYE_DATA in the chartsMonitor ancillary program.


    Version 1.03
    Jan C. Depner
    06/28/10

    Changed chartsMonitor to lidarMonitor due to multiple lidar support.


    Version 1.04
    Jan C. Depner
    07/06/10

    Fixed the ever elusive "deep flyer not showing up" problem.  Unfortunately I waited 3 months to fix it after I fixed it in
    pfmEdit3D.  I forgot about geoSwath3d having the same problem.  DOH!


    Version 1.05
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 2.00
    Jan C. Depner
    11/05/10

    Finally fixed auto Z scaling in nvMapGL (nvutility library).  Also added minimum Z extents to preferences.


    Version 3.00
    Jan C. Depner
    12/08/10

    Added 2D tracker and auto filter options like in pfmEdit3D.  Cleaned up the button and ancillary program hotkey handling.


    Version 3.01
    Jan C. Depner
    12/09/10

    Fixed the slice and size bar buttons (finally).


    Version 3.02
    Jan C. Depner
    12/14/10

    We can finally mark data points in a somewhat proper way.  They don't disappear when you rotate or zoom.


    Version 3.03
    Jan C. Depner
    12/16/10

    Can now highlight and un-highlight in polygons.  Also, pressing the Enter key when data is highlighted will
    invalidate the highlighted points.


    Version 3.04
    Jan C. Depner
    01/05/11

    Added hotkey check in the preferences dialog to prevent conflicts.  I really should have done this a long time
    ago but it was way down on the priority list ;-)


    Version 3.05
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 3.06
    Jan C. Depner
    01/15/11

    Added an exaggeration scrollbar to the left side of the window.


    Version 3.07
    Jan C. Depner
    01/16/11

    Positioned non-modal popup dialogs in the center of the screen.


    Version 3.08
    Jan C. Depner
    02/24/11

    Fixed a badly stupid error in io_data.cpp for GSF that was only showing up in 32 bit Windows XP for some unknown reason.
    It should have hosed things on all platforms but seemed to sneak through on all of the others (including 64 bit Windows 7).


    Version 3.09
    Jan C. Depner
    02/25/11

    Switched to using PFM_HAWKEYE_HYDRO_DATA and PFM_HAWKEYE_TOPO_DATA to replace the deprecated PFM_HAWKEYE_DATA.


    Version 3.10
    Jan C. Depner
    03/28/11

    Allow user to define the key for invalidating filter highlighted or manually highlighted data.  Try to save the user's
    hot key definitions even across setting version changes.


    Version 3.11
    Jan C. Depner
    04/14/11

    Removed tofWaterKill ancillary program.


    Version 3.12
    Jan C. Depner
    06/16/11

    Removed HMPS_SPARE_1 flag check since we don't do swath filtering anymore and it's been removed from hmpsflag.h.


    Version 3.13
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.
    Removed the . on the command line option since the current working directory is in the sidebar.


    Version 3.14
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.

*/
