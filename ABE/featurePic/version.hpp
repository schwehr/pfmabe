
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

#define     VERSION     "PFM Software - featurePic V2.14 - 01/06/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    06/14/06

    First working version.


    Version 1.01
    Jan C. Depner
    09/12/06

    Check zoom_level in pic.cpp so it doesn't try to grab tons of memory.


    Version 2.0
    Jan C. Depner
    04/11/07

    Qt 4 port.


    Version 2.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 2.02
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 2.03
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 2.10
    Jan C. Depner
    04/02/09

    Added support for BFD files containing images.


    Version 2.11
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 2.12
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 2.13
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 2.14
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.

*/
