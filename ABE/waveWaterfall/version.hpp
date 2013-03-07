
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

#define     VERSION     "PFM Software - waveWaterfall V1.33 - 01/06/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    11/23/07

    First working version.


    Version 1.01
    Jan C. Depner
    01/04/08

    Now uses the parent process ID of the bin viewer plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 1.02
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 1.03
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 1.04
    Jan C. Depner
    07/15/08

    Removed pfmShare shared memory usage and replaced with abeShare.


    Version 1.05
    Jan C. Depner
    11/03/08

    Converted to use new mwShare.multi... type records in the ABE_SHARE structure.


    Version 1.06
    Jan C. Depner
    11/16/08

    Lock trackCursor during snapshot of waveforms.


    Version 1.07
    Jan C. Depner
    11/28/08

    Added kill_switch option.


    Version 1.20
    Jan C. Depner
    03/13/09

    Added ability to handle WLF data.  Friday the 13th - Oh No!


    Version 1.21
    Jan C. Depner
    03/25/09

    Waveforms from the same line as the first are colored the same as the first line.


    Version 1.22
    Jan C. Depner
    04/08/09

    Changes to deal with "flush" argument on all nvmapp.cpp (nvutility library) drawing functions.


    Version 1.23
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 1.24
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 1.25
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 1.26
    Jan C. Depner
    06/15/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 1.27
    Jan C. Depner
    08/06/09

    Added waveform selected point mark for WLF data.


    Version 1.28
    Jan C. Depner
    09/10/09

    Made window icon dependent on data type (APD or PMT).


    Version 1.29
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 1.30
    Jan C. Depner
    09/16/09

    Set killed flag in abe_share when program exits other than from kill switch from parent.


    Version 1.31
    Jan C. Depner
    11/17/09

    Color coded the status bar backgrounds to match the waveforms.


    Version 1.32
    Jan C. Depner
    08/30/10

    Fixed APD and PMT location settings problem.


    Version 1.33
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.

*/
