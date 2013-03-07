
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

#define     VERSION     "PFM Software - waveformMonitor V3.17 - 01/06/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    04/05/05

    First working version.


    Version 1.01
    Jan C. Depner
    04/15/05

    Added error message for file open errors (other than command line).  Happy tax day :-(


    Version 1.02
    Jan C. Depner
    04/19/05

    Changed all of the QVBox's to QVBoxLayout's.  This allows the dialogs to auto
    size correctly.  I'm learning ;-)


    Version 1.03
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.1
    Jan C. Depner
    05/18/05

    Modified so that it could be started geoSwath (--geoSwath PPID argument) as well as PFM_ABE.


    Version 1.11
    Jan C. Depner
    05/31/05

    Fixed bug - wasn't setting share->modcode when trying to swap.


    Version 1.2
    Jan C. Depner
    06/30/05

    Added ability to display wave forms as lines or dots.


    Version 1.21
    Jan C. Depner
    08/24/05

    Fix display of reported and corrected depths.


    Version 1.22
    Jan C. Depner
    03/28/06

    Replaced QVGroupBox with QGroupBox and removed QHBox and QVBox to prepare for Qt 4.


    Version 1.23
    Jan C. Depner
    04/10/06

    Added WhatsThis button to prefs and extended dialogs.


    Version 2.0
    Jan C. Depner
    04/12/07

    Qt 4 port.


    Version 2.01
    Jan C. Depner
    05/30/07

    Added alpha values to saved colors.


    Version 2.02
    Jan C. Depner
    08/03/07

    Correctly compute backslope by looking for next rise to end slope computation area.


    Version 2.03
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 3.00
    Jan C. Depner
    09/20/07

    Switched from POSIX shared memory to QSharedMemory.


    Version 3.01
    Jan C. Depner
    10/23/07

    Now honors the CHILD_PROCESS_FORCE_EXIT key.


    Version 3.02
    Jan C. Depner
    01/04/08

    Now uses the parent process ID of the bin viewer plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 3.03
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 3.04
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 3.05
    Jan C. Depner
    04/28/08

    Use save_wave instead of wave_data when computing slopes for the extended display.


    Version 3.06
    Jan C. Depner
    07/15/08

    Removed pfmShare shared memory usage and replaced with abeShare.


    Version 3.07
    Jan C. Depner
    03/20/09

    Reinstated the program and renamed it to waveformMonitor.  This was at the request of the ACE.  The new
    waveMonitor (from USM) eats up too much screen real estate.  Also removed the extended dialog since
    that functionality is being replaced by chartsMonitor.  Added ability to support WLF format.


    Version 3.08
    Jan C. Depner
    04/08/09

    Changes to deal with "flush" argument on all nvmapp.cpp (nvutility library) drawing functions.


    Version 3.09
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 3.10
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 3.11
    Jan C. Depner
    06/15/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 3.12
    Jan C. Depner
    07/29/09

    Added ac_zero_level lines.


    Version 3.13
    Jan C. Depner
    08/06/09

    Added selected point marker on waveforms for WLF data.


    Version 3.14
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 3.15
    Jan C. Depner
    09/14/09

    Fix the uncorrected vs corrected depth display in KGPS data.  Changed icon to something a little more
    representative.


    Version 3.16
    Jan C. Depner
    09/16/09

    Set killed flag in abe_share when program exits other than from kill switch from parent.


    Version 3.17
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.

*/
