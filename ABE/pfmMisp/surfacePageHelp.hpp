
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



QString surface_typeText = 
  surfacePage::tr ("Select the data/surface type to be used to generate the Minimum Curvature Spline Interpolated surface.");

QString min_surfaceText = 
  surfacePage::tr ("You may select one data/surface type to be used to generate the MISP surface.  This button will "
                   "generate a MISP surface based on the <b>Minimum Filtered Surface</b>.");

QString max_surfaceText = 
  surfacePage::tr ("You may select one data/surface type to be used to generate the MISP surface.  This button will "
                   "generate a MISP surface based on the <b>Maximum Filtered Surface</b>.");

QString avg_surfaceText = 
  surfacePage::tr ("You may select one data/surface type to be used to generate the MISP surface.  This button will "
                   "generate a MISP surface based on all of the depths stored in the PFM structure.");

QString factorText = 
  surfacePage::tr ("The <b>Weight factor</b> is used to weigh the original input against the minimum curvature regional "
                   "grid that is created from the original input data.  The higher the value, the more weight that is "
                   "given to the input data.  The acceptable values are from 1 to 3.  A good rule of thumb for the "
                   "weight is to use 1 if the data is very random point or line data, use 2 for fairly evenly spaced "
                   "point, line, or area data, and use 3 for evenly spaced, uniform coverage.  If a smooth "
                   "representation of the field is needed then the lower values will give better results.");

QString forceText = 
  surfacePage::tr ("Select this if you wish to force the final output grid to use the original data values at the "
                   "location of those values.  This overrides the weight factor in the locations of the original "
                   "values.  The weight factor will still have some influence on the grid surface in areas near the "
                   "original values.  Generally speaking, you only want to use this option if you have set the "
                   "<b>Weight factor</b> to a value of 3.");

QString nibbleText = 
  surfacePage::tr ("Set the nibble distance in bins.  The nibbler is used to clear interpolated bins that are a set "
                   "distance from bins that contain data.  For example, if you set the nibble value to 4 then any cell "
                   "that is not within 4 bins of a bin that contains data will be cleared after the surface has been "
                   "generated.  If this is set to -1, no nibbling will be done and all empty bins will be interpolated.  "
                   "If it is set to 0, no empty bins will be interpolated.  In other words, the nibbler won't be run after "
                   "the empty celss are interpolated.  Interpolated values for empty cells will not be inserted into the "
                   "surface.");

QString clearLandText = 
  surfacePage::tr ("Select this if you wish to clear interpolated bins in areas that are marked as land in the "
                   "<b>Shuttle Radar Topography Mission (SRTM)</b> land mask.  In order to use this option you must "
                   "have the <b>SRTM_DATA</b> environment variable set to point to the directory that contains the "
                   "SRTM land mask file.");

QString replaceAllText = 
  surfacePage::tr ("Select this if you wish to replace all of the <b>Average Filtered/Edited Surface</b> bins with "
                   "MISP Surface data.  If this is not checked then only those bins that do not contain data will be "
                   "replaced.");
