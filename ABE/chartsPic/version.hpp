
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

#define     VERSION     "PFM Software - chartsPic V4.27 - 01/06/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    09/09/04

    First working version.


    Version 1.1
    Jan C. Depner
    10/13/04

    Add system 2 roll, pitch, heading parameters.  Check for sbet file
    prior to checking for pos file.  Fix resize problem.


    Version 1.2
    Jan C. Depner
    10/29/04

    Added Linux only display of rotated image (separate window, no cursor
    tracking, requires ImageMagick);


    Version 1.3
    Jan C. Depner
    04/01/05

    Added --actionkey00 and --actionkey01 options (from pfmEdit).  --actionkey01
    will generate a rotated picture when the associated action key is pressed
    in pfmEdit.  Also, now shows NULL value photos.


    Version 1.31
    Jan C. Depner
    04/19/05

    Changed all of the QVBox's to QVBoxLayout's.  This allows the dialogs to auto
    size correctly.  I'm learning ;-)


    Version 1.32
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.4
    Jan C. Depner
    05/18/05

    Modified so that it could be started geoSwath (--geoSwath PPID argument) as well as PFM_ABE.


    Version 1.5
    Jan C. Depner
    05/23/05

    Added Brighten and Darken action keys.  Added Freeze toggle button.


    Version 2.0
    Jan C. Depner
    12/29/05

    Now able to display pictures rotated to North up.


    Version 2.1
    Jan C. Depner
    01/03/06

    No longer making temporary jpg file to store image.  Using Qt::Pixmap.loadFromData.  Happy 28 freaking
    years at NAVO.  Sheesh, I'm old ;-)


    Version 2.2
    Jan C. Depner
    03/07/06

    Added option for saving the snippet using file name sent in ABE_SHARE from pfmEdit.


    Version 3.0
    Jan C. Depner
    04/06/06

    Massive Qt changes - new cursors, left click to finish operations, right click menu in some modes, fixed 
    stretch, replaced QVBox, QHBox and others with layouts.


    Version 3.01
    Jan C. Depner
    06/14/06

    Saves heading to ABE_SHARE when dumping a target snippet.


    Version 3.02
    Jan C. Depner
    09/08/06

    Switched to overrideCursor for wait state.


    Version 3.03
    Jan C. Depner
    09/12/06

    Check zoom_level in pic.cpp so it doesn't try to grab tons of memory.


    Version 3.04
    Jan C. Depner
    09/29/06

    Make default filename for saving scaled picture the dtg and position of picture.


    Version 3.05
    Jan C. Depner
    01/25/07

    Try to open upper case SBET file if lower case isn't found.  GCS stupidly changes
    case on the SBET file when it mods it.  DOH!


    Version 4.0
    Jan C. Depner
    04/11/07

    Qt 4 port.


    Version 4.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 4.02
    Jan C. Depner
    09/05/07

    Make sure picture is redrawn each time the picture changes (heading problem).


    Version 4.03
    Jan C. Depner
    10/05/07

    Change the timer to check every 250 milliseconds.  This really takes the load off of the processor
    and it's still just as responsive.


    Version 4.04
    Jan C. Depner
    10/15/07

    Set the shared memory key to 0 after each redraw so that the caller will know it has been started.


    Version 4.05
    Jan C. Depner
    11/01/07

    Added "stickpins".


    Version 4.06
    Jan C. Depner
    12/07/07

    Got rid of annoying QMessageBox error messages and moved them to the status bar.  Tora, tora, tora!


    Version 4.07
    Jan C. Depner
    01/04/08

    Now uses the parent process ID of the bin viewer plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 4.08
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 4.09
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 4.10
    Jan C. Depner
    07/15/08

    Removed pfmShare shared memory usage and replaced with abeShare.


    Version 4.11
    Jan C. Depner
    10/30/08

    Mods to support new waveMonitor changes to ABE.h.


    Version 4.12
    Jan C. Depner
    11/28/08

    Added kill_switch option.


    Version 4.20
    Jan C. Depner
    03/13/09

    Added ability to handle WLF data.  Friday the 13th - Oh No!


    Version 4.21
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 4.22
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 4.23
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 4.24
    Jan C. Depner
    06/15/09

    Use WLF sensor position and attitude data instead of POS or SBET if it is present in the WLF record.


    Version 4.25
    Jan C. Depner
    09/16/09

    Set killed flag in abe_share when program exits other than from kill switch from parent.


    Version 4.26
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 4.27
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.

*/
