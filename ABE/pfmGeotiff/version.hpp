
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

#define     VERSION     "PFM Software - pfmGeotiff V5.26 - 11/03/11"

#endif

/*!< <pre>

    Version 1.0
    Jan C. Depner
    07/07/03

    First working version.


    Version 1.1
    Jan C. Depner
    07/23/03

    Had to switch to TOPLEFT orientation because neither CARIS nor FLEDERMAUS 
    could handle BOTLEFT.


    Version 1.2
    Jan C. Depner
    08/19/03

    Went to PIXELSCALE in addition to the geotie points since Fledermaus, etc
    can't handle real geotiepoints.  Also fixed bug in TOPLEFT implementation.


    Version 1.3
    Jan C. Depner
    08/27/03

    Added alpha channel transparency option.


    Version 2.0
    Jan C. Depner
    12/11/03

    Now able to output multiple geotiffs representing coverage for each of the
    different data types.


    Version 3.0
    Jan C. Depner
    03/02/04

    Qt Wizard version.  Also, fixed a bug that caused the minimum value to 
    always be 0.0.  More importantly, I get my new Gibson ES-135 today :D


    Version 3.13
    Jan C. Depner
    08/13/04

    Changes to deal with data topo and hydro data in the same set.  Turnover is at
    zero.  This is the Friday the 13th version ;-)


    Version 3.4
    Jan C. Depner
    09/07/04

    Now reads .ARE or .are file to only generate subset GeoTIFF.


    Version 3.51
    Jan C. Depner
    02/14/05

    Fixed a "stupid" when not building using an area file... Doh!  Happy Valentine's
    Day!


    Version 3.52
    Jan C. Depner
    02/25/05

    Added slot for modifying the output file name manually.  Switched to 
    open_existing_pfm_file from open_pfm_file.


    Version 3.53
    Jan C. Depner
    08/18/05

    Added restart colormap button so that you can optionally make the colors continuous across the
    zero boundary.


    Version 3.54
    Jan C. Depner
    10/26/05

    Now uses the PFM 4.6 handle file or list file instead of the PFM directory name.


    Version 3.6
    Jan C. Depner
    02/03/06

    Added envin and envout.  Check for null value in min bin.


    Version 3.61
    Jan C. Depner
    03/29/06

    Replaced QVox, QHBox with QVBoxLayout, QHBoxLayout to prepare for Qt 4.


    Version 3.62
    Jan C. Depner
    06/05/06

    Removed get_area_mbr.cpp.  Moved to utility.


    Version 3.7
    Jan C. Depner
    06/13/06

    Name change from pfm2geotiff to pfmGeotiff to match naming convention for PFM_ABE
    Qt programs.


    Version 3.8
    Jan C. Depner
    06/28/07

    Fixed transparency and projection data.


    Version 4.0
    Jan C. Depner
    07/02/07

    Ported to Qt4.


    Version 4.1
    Jan C. Depner
    07/04/07

    Added live update of sample data set when changing color and
    sunshading parameters.  Happy Birthday America!


    Version 4.11
    Jan C. Depner
    07/27/07

    Added background color on start and end color labels in the image parameters page.


    Version 4.12
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 5.00
    Jan C. Depner
    10/03/07

    Switched to GDAL instead of TIFF/GEOTIFF/JPEG.


    Version 5.01
    Jan C. Depner
    12/22/07

    Handle command line PFM file.


    Version 5.02
    Jan C. Depner
    04/07/08

    Replaced single .h and .hpp files from utility library with include of nvutility.h and nvutility.hpp


    Version 5.03
    Jan C. Depner
    04/30/08

    Added QFile->close () after reading the sample data.


    Version 5.04
    Jan C. Depner
    01/29/09

    Set checkpoint to 0 prior to calling open_existing_pfm_file.


    Version 5.05
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 5.10
    Jan C. Depner
    06/02/09

    Ignores pfmView generated GSF files (pfmView_GSF) for coverage output.


    Version 5.20
    Jan C. Depner
    11/03/09

    Added option to output 32 bit floating point GeoTIFF instead of color coded, sunshaded.


    Version 5.21
    Jan C. Depner
    08/27/10

    Set the proper NoData value for greyscale.


    Version 5.22
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 5.23
    Jan C. Depner
    06/27/11

    Save all directories used by the QFileDialogs.  Add current working directory to the sidebar for all QFileDialogs.


    Version 5.24
    Jan C. Depner
    07/22/11

    Using setSidebarUrls function from nvutility to make sure that current working directory (.) and
    last used directory are in the sidebar URL list of QFileDialogs.


    Version 5.25
    Jan C. Depner
    09/27/11

    Added NAVD88 height option.


    Version 5.26
    Jan C. Depner
    11/03/11

    Added option to include or exclude interpolated contours.  Added option to set
    smoothing level.  Fixed bug in contour generation.  Outputs empty ESRI DBF shape file
    for contours so that Arc can handle the files nicely.

</pre>*/
