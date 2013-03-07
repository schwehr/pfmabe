
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
  surfacePage::tr ("Select the surface type to be used to generate the GeoTIFF.");

QString min_surfaceText = 
  surfacePage::tr ("You may select one surface type to be output as a GeoTIFF.  This button will generate a GeoTIFF based "
                   "on the <b>Minimum Filtered Surface</b>.");

QString max_surfaceText = 
  surfacePage::tr ("You may select one surface type to be output as a GeoTIFF.  This button will generate a GeoTIFF based "
                   "on the <b>Maximum Filtered Surface</b>.");

QString avg_surfaceText = 
  surfacePage::tr ("You may select one surface type to be output as a GeoTIFF.  This button will generate a GeoTIFF based "
                   "on the <b>Average Filtered or MISP Surface</b>.  A MISP surface can be by running the "
                   "<b><i>pfm_misp</i></b> program.");

QString cov_surfaceText = 
  surfacePage::tr ("You may select one surface type to be output as a GeoTIFF.  This button will generate one or more "
                   "GeoTIFFs that are based on the coverage by data type.  For example, you may want to see what parts of "
                   "an area are covered by <b><i>PFM_GSF_DATA</i></b>, <b><i>PFM_SHOALS_OUT_DATA</i></b>, and "
                   "< b><i>PFM_SHOALS_1K_DATA</i></b>.  If those are the only data types that are used in the input PFM "
                   "file then three GeoTIFFs will be generated, one for each data type.  The output files will be named "
                   "with the base name (from the <b>Output GeoTIFF File</b> text window) appended with "
                   "typeNN_coverage.tif (where NN is the data type number).  <b>WARNING</b> - this option can take a "
                   "very long time because all of the individual soundings must be read in order to determine coverage.");

QString transparentText = 
  surfacePage::tr ("Selecting this option will cause all empty cells to be set to transparent (alpha channel 0) in the "
                   "output GeoTIFF.  If you do not select this then all empty cells will be blank.  Using a screen viewer "
                   "(like <b>Electric Eyes</b> [ee]) will display empty cells as black.  In <b>CARIS</b> empty cells will "
                   "be white.  You will definitely want to use transparent background if you are going to "
                   "put more than one GeoTIFF in <b>CARIS</b> or <b>Fledermaus</b>.  If you don't use the transparent "
                   "background the empty cells of one GeoTIFF will obscure the other GeoTIFF(s).");

QString packbitsText = 
  surfacePage::tr ("This check box will force the output to be unblocked and use <b>PACKBITS</b> compression instead of the "
                   "more efficient (and faster) LZW compression.<br><br>"
                   "<b><i>WARNING - This is really slow so unless you absolutely have to use packbits compression (older "
                   "versions of Caris may require it, newer versions of Caris may support LZW compression) don't use it!  "
                   "If you must use it, make sure that your output file is on a local disk not an NFS mounted disk "
                   "(/net/whatever).");

QString greyText = 
  surfacePage::tr ("This check box will force the output to be 32 bit floating point elevation values.  When checked, the transparent "
                   "option is ignored and the color setting page will be disabled.<br><br>"
                   "<b>IMPORTANT NOTE: The data will be output as elevations, not depths.  That is, positive Z is up not down.</b>");

QString unitsText = 
  surfacePage::tr ("Select the units in which you would like to output the data.  Internally all data is stored in meters.  For sonar data, "
                   "the internal values may have been computed using a sound velocity profile which would give <b><i>true</i></b> depth or "
                   "they might have been computed using a <b><i>nominal</i></b> sound speed of 1500 meters per second.  If you select fathoms "
                   "as the output units then the geoTIFF and the (optional) contours will be converted to fathoms using the normal conversion "
                   "factor of 1.8288 meters per fathom.");

QString dumbText = 
  surfacePage::tr ("Checking this box indicates that you want to convert <b><i>nominal</i></b> depths in meters collected at 1500 meters per second to "
                   "<b><i>nominal</i></b> depths in fathoms converted to a sound speed of 4800 feet per second.  The conversion factor used will "
                   "be 1.875 meters per fathom.  Note that you can only select this option if you are writing the data in fathoms.<br><br>"
                   "<b>IMPORTANT NOTE: If you were not using <i>nominal</i> depth when you built this PFM do not select this option.</b>");

QString inText = 
  surfacePage::tr ("Checking this box indicates that you want to include contours that are generated over interpolated data.");

QString navd88Text = 
  surfacePage::tr ("Checking this box indicates that you want to convert WGS84 ellipsoid elevations (or depths) to NAVD88 elevations (or "
                   "depths).  This is done by subtracting (or adding) the GEOID03 value for the location from (to) the elevation (depth).");

QString elevText = 
  surfacePage::tr ("Checking this box indicates that you want the data to be output as elevation instead of depth.  This is assuming that the "
                   "PFM was loaded with depth data (positive Z down).  If it was loaded with elevation data (positive Z up) then this will convert "
                   "it to depth before writing the geoTIFF (and optional contour) file(s).");

QString intervalText = 
  surfacePage::tr ("Select a contour interval for use in generating a ESRI SHAPE files containing the contours.  If you set this value to 0.0 "
                   "no contour files will be generated.  Contours will be in the units selected for the geoTIFF.  Contours will not be generated "
                   "if you selected <b><i>Coverage</i></b> as the surface type.  The name of the ESRI SHAPE file will be the same as the geoTIFF "
                   "file but with a .shp extension.");

QString smoothingText = 
  surfacePage::tr ("Select the contour smoothing level.  Setting this to anything other than 0 can increase the size of "
                   "your contour SHAPE file by orders of magnitude (especially if you choose 10).");
