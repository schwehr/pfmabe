
/*********************************************************************************************

    This program is public domain software that was developed by the U.S. Naval Oceanographic
    Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.

*********************************************************************************************/


/*  C/Motif pfm_extract replaced by C++/Qt pfmExtract on 06/02/05.  All comments for pfm_extract
    have been left in for historical (hysterical ???) purposes.  JCD  */


#ifndef VERSION

#define     VERSION     "PFM Software - pfmExtract V3.17 - 11/03/11"

#endif

/*

    Dave Fabre, Neptune Sciences
    21 apr 2000 (v0.2) -    correct CTD/XBT indicator
                            if the comment has "CT" or "XB" then we can tell
                            otherwise, we can't tell
    11 may 2000 (v0.4) -    put in the option to output 2 forms,
                            keyword:value & column with comma separators.
    21 jun 2000 (v0.5) -    outputting both forms & outputting
                            even if the getvals fail and outputting shoals stuff.



    Version 0.6
    Jan C. Depner
    06/28/00

    Added check for file number out of bounds.  Also used indent to clean up
    source code.


    Version 1.0
    Jan C. Depner
    09/04/00

    Replaced call to read_depth_record_index with read_depth_array_index.


    Version 1.1
    Jan C. Depner
    02/21/01

    Passing scale to open_pfm_file as a pointer.


    Version 1.2
    Jan C. Depner
    03/27/01

    Fixed contour output.


    Version 1.3
    Jan C. Depner
    06/01/01

    Don't let it unload files marked as "deleted".


    Version 1.4
    Jan C. Depner
    06/20/01

    Pass structure args to open_pfm_file.


    Version 2.0
    Jan C. Depner
    07/19/01
 
    4.0 PFM library changes.


    Version 2.01
    Jan C. Depner
    10/15/02
 
    Handles single-beam files (sba, sbb, etc).


    Version 2.1
    Jan C. Depner
    11/13/02
 
    Removed the .mge file (no longer used).  Added .cnt file for contours
    for the .crs file.  Added smoothing of contours.  Changed .crs format
    to better match what they need in N43 for Caris.


    Version 2.11
    Jan C. Depner
    11/27/02
 
    Changed .crs format (again) to better match what they need in N43 for 
    Caris.


    Version 2.2
    Jan C. Depner
    08/19/03
 
    Added brain-dead DNC stair_stepped contours.  Added CHARTS HOF file 
    support.


    Version 2.3
    Jan C. Depner
    09/15/03
 
    Replaced the .ext format (that nobody was using) with ASCII lat,lon,depth.
    This may be a bit more useful.


    Version 3.0
    Jan C. Depner
    09/22/04
 
    Added -a option to extract only for defined area.


    Version 3.01
    Jan C. Depner
    10/20/04
 
    Fixed -a option for polygon - DOH!


    Version 3.02
    Jan C. Depner
    11/23/04
 
    Handle old hof/tof lidar record count (starts at 0 instead of the, now 
    standard, 1).


    Version 3.03
    Jan C. Depner
    12/16/04
 
    Changed Usage message for PFM 4.5 directory input.


    Version 3.1
    Dave Fabre
    02/22/05
 
    Get beam positions from the GSF files to avoid UTM coordinates in current
    version of CUBE.


    Version 3.11
    Jan C. Depner
    02/25/05

    Switched to open_existing_pfm_file from open_pfm_file.


    Version 3.12
    Jan C. Depner
    03/04/05

    Fix return from open_existing_pfm_file.


******************* pfmExtract comments *********************


    Version 1.0
    Jan C. Depner
    06/02/05

    This is the first version of the C++/Qt replacement for pfm_extract.
    I have gone back to 1.0 but didn't want to lose the earlier documentation
    of the C/Motif pfm_extract program.


    Version 1.01
    Jan C. Depner
    10/26/05

    Now uses the PFM 4.6 handle file or list file instead of the PFM directory name.


    Version 1.02
    Jan C. Depner
    02/03/06

    Make sure we don't use points from deleted files.


    Version 1.03
    Jan C. Depner
    03/28/06

    Replaced QVBox and QHBox with QVBoxLayout and QHBoxLayout in preparation for Qt 4.


    Version 1.04
    Jan C. Depner
    04/10/06

    Fixed the context sensitive help button.  Now saves geometry.


    Version 1.05
    Jan C. Depner
    06/05/06

    Removed inside.cpp and get_area_mbr.cpp.  Moved to utility.


    Version 2.00
    Jan C. Depner
    07/23/07

    Qt 4 port.


    Version 2.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 2.02
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 2.03
    Jan C. Depner
    04/08/08

    Added geod.c, geod.h, and ellipsoid.h and cleaned up the code a bit.


    Version 2.04
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 2.05
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 3.00
    Jan C. Depner
    06/08/09

    This program is now the GUI version of the old pfm2rdp program.  Since NAVO has stopped using pfm_ss to 
    select soundings we don't need to be able to extract them.


    Version 3.01
    Jan C. Depner
    07/02/09

    Added depth cutoff and datum shift options.


    Version 3.02
    Jan C. Depner
    03/16/10

    Clear the LLZ header prior to setting fields.


    Version 3.03
    Jan C. Depner
    07/07/10

    Clear the LLZ header prior to setting fields.


    Version 3.10
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 3.11
    Jan C. Depner
    09/09/10

    Added ESRI's POINTZ SHAPEFILE as an output option.


    Version 3.12
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 3.13
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 3.14
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 3.15
    Jan C. Depner
    08/01/11

    Added orthometric correction option.


    Version 3.16
    Jan C. Depner
    08/18/11

    Added exclude land option.


    Version 3.17
    Jan C. Depner
    11/03/11

    Fixed geoid03 application bug.

*/
