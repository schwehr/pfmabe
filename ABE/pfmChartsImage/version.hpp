
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

#define     VERSION     "PFM Software - pfmChartsImage V7.17 - 07/22/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    10/26/04

    First working version.


    Version 1.01
    Jan C. Depner
    11/05/04

    Changed sign on heading for ERDAS, output R,P instead of P,R.


    Version 1.02
    Jan C. Depner
    12/16/04

    Changed Usage message for PFM 4.5 directory input.


    Version 1.03
    Jan C. Depner
    01/14/05

    Output DEM in UTM coordinates.


    Version 1.04
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 1.1
    Jan C. Depner
    03/14/05

    Now handles HOF files for those rare occasions when you go over
    land but don't have TOF files.


    Version 2.0
    Jan C. Depner
    08/09/05

    Hopefully we now have everything right for ERDAS to do the photo mosaicing.


    Version 2.01
    Jan C. Depner
    08/12/05

    Added -n option to override limits on too dark or light pictures.


    Version 2.02
    Jan C. Depner
    10/12/05

    Added -m option to decimate the number of pictures output (Every mth picture).


    Version 2.03
    Jan C. Depner
    10/26/05

    Changed usage for PFM 4.6 handle file use.


    Version 2.04
    Jan C. Depner
    01/03/06

    Minor change to image_read_record call.


    Version 2.05
    Jan C. Depner
    06/05/06

    Removed inside.c, get_area_mbr.c, ellpfns.c, projfns.c, and tmfns.c.  Moved to utility.


    Version 2.1
    Jan C. Depner
    07/31/06

    Added pos time offset option.  Changed name of pos.txt output file to pos.dat.  Added area file name to
    beginning of pos.dat and dem.dat (areaname_pos.dat).  Added flip sign option.  Fixed the omega, phi, kappa
    computations and the interpolation in the charts pos_io function...  DOH!!!


    Version 2.11
    Jan C. Depner
    10/16/06

    Added zone to end of DEM and .txt files.


    Version 2.2
    Jan C. Depner
    10/17/06

    Added JAM output option.


    Version 2.21
    Jan C. Depner
    01/25/07

    Check for upper case SBET file name to correct for GCS screwup.


    Version 3.00
    Jan C. Depner
    06/18/07

    Check for filename in .kin file actually being correct.  Fix my seriously major
    screwup trying to find the related image records.


    Version 4.00
    Jan C. Depner
    06/28/07

    Use MISP to create a DEM of any grid size.  Set water values to 0.0


    Version 5.00
    Jan C. Depner
    07/03/07

    Ported to a Qt wizard (Qt4).


    Version 5.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 6.00
    Jan C. Depner
    08/29/07

    Now shells Ray Seyfarth's mosaic program and gdalwarp to create the mosaic and convert it to
    latlong from utm.  Two years since Katrina and we still don't have our stuff together :-(


    Version 6.01
    Jan C. Depner
    09/05/07

    Removed JAM option and replaced with .pos exclusion options.


    Version 6.02
    Jan C. Depner
    09/25/07

    Added normalize option.


    Version 6.10
    Jan C. Depner
    09/28/07

    Added brain-dead Caris option for output.


    Version 6.11
    Jan C. Depner
    10/11/07

    Fixed "free" bug.  Also, now uses image_read_record_recnum to get images.


    Version 6.20
    Jan C. Depner
    11/13/07

    New version of mosaic now allows output in utm or geographic therefor, removed GDALWARP stuff
    and added utm/geographic option.


    Version 6.21
    Jan C. Depner
    12/14/07

    Changed QWidgetItem to QCheckBox in inputPage.cpp to deal with a bug in Qt 4.4 snapshot.


    Version 6.22
    Jan C. Depner
    12/20/07

    Handle command line PFM file name.


    Version 6.23
    Jan C. Depner
    12/21/07

    Fix a number of insidious bugs in pfmChartsIMage.cpp and inputPage.cpp (caused by version 6.21 changes).


    Version 6.24
    Jan C. Depner
    01/13/08

    Keeps hits for NULL data since the picture should be OK anyway.


    Version 6.25
    Jan C. Depner
    01/31/08

    Switched DEM logic for EGM96 datum shift - I had it backwards... again... DOH!


    Version 6.26
    Jan C. Depner
    02/07/08

    Added ability to include lines in the opposite direction to the line direction limits.


    Version 6.27
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 6.28
    Jan C. Depner
    04/08/08

    Added ability to set the level for the old "zero" area file (now "level" area file).


    Version 6.29
    Jan C. Depner
    04/24/08

    Output the level value to the list widget.


    Version 7.00
    Jan C. Depner
    05/23/08

    Allow user to go back and run mosaic multiple times with different photos as input.  Also,
    moved the output file format stuff to the run page and automatically pick a file name.


    Version 7.01
    Jan C. Depner
    07/16/08

    Check for availablility of SRTM data.


    Version 7.02
    Chris Macon
    07/22/08

    Added UTM convergence computation to phi, omega, kappa computation.  Also, don't use PFM_DELETED points.


    Version 7.03
    Jan Depner
    04/08/09

    Changed limits on datum spin box to -10000.0 to 10000.0.


    Version 7.04
    Chris Macon
    04/09/09

    Added "Flip" option detection when shifting the SRTM data to ellipsoid heights
    Added a check to eliminate "0" values from being added into the misp surface


    Version 7.05
    Chris Macon
    04/10/09

    Fixed SRTM single point read problem.  Now generates DEM from all valid data points
    instead of just the bins from one of the surfaces.


    Version 7.06
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 7.07
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 7.08
    Jan C. Depner
    06/16/09

    Replaced closing message box with output to wCheckList.


    Version 7.09
    Jan C. Depner
    06/20/09

    Fixed bug dealing with PFM_CHARTS_HOF_DATA and PFM_SHOALS_1K_DATA.


    Version 7.10
    Jan C. Depner
    12/02/09

    Replaced get_egm96 with get_egm08.


    Version 7.11
    Jan C. Depner
    04/29/10

    Even though running this on Windows is pointless I made the code portable by replacing %lld with NV_INT64_SPECIFIER.


    Version 7.12
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 7.13
    Jan C. Depner
    10/04/10

    Skip image records with bad time stamps (up to 100).


    Version 7.14
    Jan C. Depner
    12/17/10

    Cleaned up layouts.  Added option to output ASCII xyz DEM for CASI processing.


    Version 7.15
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 7.16
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 7.17
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.

*/
