
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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/




#ifndef VERSION

#define     VERSION     "PFM Software - bagViewer V2.14 - 07/25/11"

#endif

/*! <pre>


    Version 1.00
    Jan C. Depner
    04/24/09

    First working version.


    Version 1.01
    Jan C. Depner
    05/04/09

    Use paintEvent for "expose" events in nvMapGL.cpp.  This wasn't a problem under compiz but shows up
    under normal window managers.


    Version 1.02
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 1.03
    Jan C. Depner
    09/11/09

    Fixed getColor calls so that cancel works.  Never forget!


    Version 1.04
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 1.05
    Jan C. Depner
    08/17/10

    Removed UTM northing and easting because - who cares ;-)


    Version 1.06
    Jan C. Depner
    11/08/10

    Minor mods for auto scale changes to nvMapGL.cpp in nvutility library.


    Version 1.07
    Jan C. Depner
    11/16/10

    Added Z value to status bar display.


    Version 1.08
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 1.09
    Jan C. Depner
    01/15/11

    Added an exaggeration scrollbar to the left side of the window.


    Version 1.10
    Jan C. Depner
    01/16/11

    Positioned non-modal popup dialogs in the center of the screen.


    Version 1.11
    Jan C. Depner
    01/18/11

    Fixed light model problem.


    Version 1.12
    Jan C. Depner
    03/21/11

    Can now display the average surface in a BAG that has the enhanced surface.  Unfortunately, I've had
    to place the Average surface in the Nominal_Elevation layer since the Average optional layer has not
    yet been implemented in BAG.


    Version 2.00
    Jan C. Depner
    03/22/11

    Support for displaying UTM BAGs.  Hopefully I've got it right ;-)


    Version 2.01
    Jan C. Depner
    04/06/11

    Added support for displaying the optional Average surface from a BAG.


    Version 2.02
    Jan C. Depner
    04/15/11

    Fixed the geoTIFF reading by switching to using GDAL instead of Qt.  Hopefully Qt will get fixed eventually.


    Version 2.10
    Jan C. Depner
    04/24/11

    Removed support for displaying the optional Average surface from a BAG.  At a meeting with Steve Harrison,
    Wade Ladner, and Bill Rankin, we decided to remove the "Average" surface since it won't significantly affect model
    generation to use the enhanced surface (since most models will use larger bins with averaged enhanced points).


    Version 2.11
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 2.12
    Jan C. Depner
    06/30/11

    I forgot to initialize the mapdef initial z bounds.  This caused an intermittent segfault.


    Version 2.13
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 2.14
    Jan C. Depner
    07/25/11

    Added section to compute and display in the status bar Z0, Z1, and SEP if you have a regularly spaced
    grid of separation correctors.  This is a precursor to allowing color by Z), Z1, or SEP.

</pre>*/
