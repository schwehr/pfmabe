
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

#define     VERSION     "PFM Software - chartsLAS V5.02 - 10/06/11"

#endif


/*

    Version 1.00
    Jan C. Depner
    02/24/06

    First version.


    Version 1.01
    Jan C. Depner
    03/28/06

    Replaced QHBox, QVBox, QVGroupBox with QHBoxLayout, QVBoxLayout, QGroupBox to prepare for Qt 4.


    Version 1.2
    Jan C. Depner
    05/17/06

    Added UTM northing and easting output as well as distance from ellipsoid.


    Version 1.21
    Jan C. Depner
    05/18/06

    Added UTM zone to output.


    Version 1.22
    Jan C. Depner
    06/05/06

    Removed inside.cpp and fget_coord.cpp.  Moved to utility.


    Version 1.3
    Jan C. Depner
    06/08/06

    Added option to correct from ellipsoidal to orthometric heights.


    Version 1.4
    Jan C. Depner
    08/01/06

    Fixed problem with TOF first and last possibly not being in the same area.


    Version 1.41
    Jan C. Depner
    08/04/06

    First return bug fix.


    Version 1.42
    Jan C. Depner
    09/08/06

    Switched to overrideCursor for wait state.



    Version 1.43
    Jan C. Depner
    09/22/06

    Now write out both first and last even if they are the same value and position.


    Version 1.44
    Jan C. Depner
    10/16/06

    Added time to HOF ASCII output.


    Version 1.45
    Jan C. Depner
    11/07/06

    Close the stinkin' input files - DOH!  Don't forget to vote.


    Version 1.46
    Jan C. Depner
    01/05/07

    Fixed number of returns per pulse.  Added horizontal datum choice (WGS84 or NAD83).


    Version 1.47
    Jan C. Depner
    02/07/07

    Added optional prefix for output files.


    Version 1.5
    Jan C. Depner
    03/16/07

    Progress bars only update at 20% intervals.


    Version 2.0
    Jan C. Depner
    04/19/07

    Moved qApp->processEvent outside of progress bar check.  Check against last area before 
    checking against other areas.


    Version 2.01
    Jan C. Depner
    06/19/07

    Changed output format a bit.


    Version 3.0
    Jan C. Depner
    07/16/07

    Ported to Qt4.


    Version 3.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 3.02
    Jan C. Depner
    08/31/07

    Get UTM zone from center of each area and don't recompute it per record.


    Version 3.03
    Jan C. Depner
    12/14/07

    Changed HOF ASCII header to say ELEV instead of DEPTH since it is ;-)


    Version 3.04
    Jan C. Depner
    03/10/08

    Fixed the directory browse functions so that you don't have to navigate around if you want the current working
    directory.


    Version 3.05
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 3.06
    Jan C. Depner
    04/08/08

    Corrected the directory browse location problem.


    Version 3.07
    Jan C. Depner
    04/29/08

    Replaced old area reading code with get_area_mbr.


    Version 4.00
    Jan C. Depner
    04/21/09

    Now uses liblas to write out the LAS files.


    Version 4.01
    Jan C. Depner
    04/27/09

    Uses inside_polygon2 instead of the old inside function.


    Version 4.02
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 4.03
    Jan C. Depner
    09/14/09

    Switched ASCII file I/O to LARGEFILE64.


    Version 4.04
    Jan C. Depner
    11/19/09

    Now determines if the areas are rectangles and, if so, does a (much faster) rectangle compare for the points.


    Version 5.00
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 5.01
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 5.02
    Jan C. Depner
    10/06/11

    To match a change in the TOF PFM loader we now check input files start times against 10/07/2011.
    Files prior to that date will ignore the first return if the last return was -998.0.

*/
