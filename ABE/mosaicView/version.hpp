
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

#define     VERSION     "PFM Software - mosaicView V1.34 - 04/15/11"

#endif

/*

    Version 1.0
    Jan C. Depner
    10/05/07

    First working version.


    Version 1.01
    Jan C. Depner
    10/23/07

    Now honors the CHILD_PROCESS_FORCE_EXIT key.


    Version 1.02
    Jan C. Depner
    01/01/08

    Uses multiple movable objects as added to nvMap.


    Version 1.03
    Jan C. Depner
    01/04/08

    Now uses the parent process ID of the bin viewer plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 1.04
    Jan C. Depner
    01/14/08

    Uses check_target_schema (from utility library) to make sure that the schema in target files is correct.


    Version 1.05
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 1.06
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 1.07
    Jan C. Depner
    04/14/08

    Minor code cleanup.


    Version 1.08
    Jan C. Depner
    05/22/08

    Changed the window icon to match the Windows .ico icon.


    Version 1.09
    Jan C. Depner
    07/15/08

    Removed pfmShare shared memory usage and replaced with abeShare.


    Version 1.10
    Jan C. Depner
    08/28/08

    We were reading the targets twice and setting the cursor tracker twice on open.


    Version 1.11
    Jan C. Depner
    12/04/08

    Changed confidence level of new targets to 2 since they will be coming from a mosaic.  Also changed
    equipType to "other" (4) in the the Platform subelement.


    Version 1.12
    Jan C. Depner
    12/15/08

    Added screen capture to updateTarget.


    Version 1.20
    Jan C. Depner
    04/08/09

    Replaced support of NAVO standard target (XML) files with support for Binary Feature Data (BFD) files.


    Version 1.21
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 1.22
    Jan C. Depner
    04/15/09

    Added the ability to delete feature polygons.  Also added pencil cursor.


    Version 1.23
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 1.24
    Jan C. Depner
    04/27/09

    Replaced QColorDialog::getRgba with QColorDialog::getColor.


    Version 1.25
    Jan C. Depner
    05/21/09

    Fixed key press event screwup.  Set all QFileDialogs to use List mode instead of Detail mode.


    Version 1.26
    Jan C. Depner
    05/22/09

    Dealing with multiple resize events when "Display content in resizing windows" is 
    enabled.


    Version 1.27
    Jan C. Depner
    06/24/09

    Changed the BFD names to avoid collision with GNU Binary File Descriptor library.


    Version 1.28
    Jan C. Depner
    07/28/09

    Changed use of _sleep to Sleep on MinGW (Windows).


    Version 1.29
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 1.30
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 1.31
    Jan C. Depner
    08/17/10

    Replaced our kludgy old UTM transform with calls to the PROJ 4 library functions.  All hail the PROJ 4 developers!


    Version 1.32
    Jan C. Depner
    09/08/10

    Changes to handle Qt 4.6.3.


    Version 1.33
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.34
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.

*/
