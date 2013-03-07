
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

#define     VERSION     "PFM Software - examGSF V2.21 - 10/21/11"

#endif

/*! <pre>

    Version 1.00
    Jan C. Depner
    04/15/05

    First working version.  Happy tax day :-(


    Version 1.01
    Jan C. Depner
    04/22/05

    Added unit choice in preferences and profiles for data and SVP.


    Version 1.02
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.03
    Jan C. Depner
    05/11/05

    Added a few warning messages for using silly units of measure.


    Version 1.04
    Jan C. Depner
    05/26/05

    Added record and time info in status bar when sliding scroll bar.


    Version 1.05
    Jan C. Depner
    07/27/05

    Fixed plotting bug in dataProfile.


    Version 1.1
    Jan C. Depner
    08/17/05

    Added profile option for select column headers in the main view.  Also, fixed small 
    bug - don't want to check cross track array as it might be a single-beam.


    Version 1.11
    Jan C. Depner
    08/24/05

    Fixed off-by-one error in beamRecs.cpp.


    Version 1.12
    Jan C. Depner
    03/10/06

    Fixed tide type field.


    Version 1.2
    Jan C. Depner
    04/06/06

    Fixed profile paintimg problem.  Replaced QVBox, QHBox, QVGroupBox, QHGroupBox with QVBoxLayout, QHBoxLayout,
    QGroupBox to prepare for Qt 4.


    Version 1.21
    Jan C. Depner
    08/08/06

    Added file mask to input dialog.


    Version 2.0
    Jan C. Depner
    04/23/07

    Qt 4 port.


    Version 2.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 2.10
    Jan C. Depner
    10/24/07

    Fixed a number of size issues and added close and help buttons to all dialogs.  This is mostly for Windoze but
    it also makes it nicer in Linux.


    Version 2.11
    Jan C. Depner
    12/22/07

    Added ability to read file on command line without --file option.


    Version 2.12
    Jan C. Depner
    04/01/08

    Added acknowledgements to the Help pulldown menu.


    Version 2.13
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 2.14
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 2.15
    Jan C. Depner
    04/23/09

    Changed the acknowledgements help to include Qt and a couple of others.


    Version 2.16
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 2.17
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 2.18
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 2.19
    Jan C. Depner
    06/24/11

    Save and restore last input directory.


    Version 2.20
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 2.21
    Jan C. Depner
    10/21/11

    Added some missing arrays from the gsfSwathBathyPing record.

</pre>*/
