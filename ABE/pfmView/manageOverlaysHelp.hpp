
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



QString overlaysGridText = 
  manageOverlays::tr ("Select files for overlaying on the main display.  The following file "
                      "extensions are recognized:"
                      "<ul>"
                      "<li>.ARE           -      ISS60 area file format</li>"
                      "<li>.are           -      generic area file format</li>"
                      "<li>.afs           -      Army Corps area files (lon,lat)</li>"
                      "<li>.ZNE           -      ISS60 tide or SV zone file format</li>"
                      "<li>.zne           -      ISS60 tide or SV zone file format</li>"
                      "<li>.tdz           -      generic tide zone file format</li>"
                      "<li>.srv           -      ISS60 survey plan file format</li>"
                      "<li>.trk           -      decimated track file format</li>"
                      "<li>.pts,.yxz,.txt -      generic point format</li>"
                      "<li>.shp           -      ESRI SHAPE files (line or polygon only)</li>"
                      "</ul>"
                      "The ISS60 file formats are documented in the ISS60 User Manual.  The generic "
                      "file formats conform to the following:<br><br>"
                      "Geographic positions are entered as a lat, lon pair separated by a comma.  A lat "
                      "or lon may be in any of the following formats (degrees, minutes, and seconds must "
                      "be separated by a space or tab) :"
                      "<ul>"
                      "<li>Hemisphere Degrees decimal                 : S 28.4532</li>"
                      "<li>Hemisphere Degrees minutes decimal         : S 28 27.192</li>"
                      "<li>Hemisphere Degrees minutes seconds decimal : S 28 27 11.52</li>"
                      "<li>Signed Degrees decimal                 : -28.4532</li>"
                      "<li>Signed Degrees minutes decimal         : -28 27.192</li>"
                      "<li>Signed Degrees minutes seconds decimal : -28 27 11.52</li>"
                      "</ul>"
                      "West longitude and south latitude are negative :<br><br>"
                      "Ex. : -28 27 11.52 = S28 27 11.52 = s 28 27.192<br><br>"
                      "The generic bin point format contains the following <b>comma-separated</b> fields:"
                      "<ul>"
                      "<li>Latitude</li>"
                      "<li>Longitude</li>"
                      "<li>Description</li>"
                      "</ul>"
                      "The decimated track file format is as follows:<br><br>"
                      "file #,ping #,lat,lon,azimuth,distance<br><br>"
                      "For the purposes of this program azimuth and distance are ignored.  Example:<br><br>"
                      "3,9103,30.165371600,-88.749665800,353.748880,30.254914<br><br>"
                      "Prior to each change of file number is a line containing the file name in this "
                      "format:<br><br>"
                      "FILE ### = /whatever/whatever/blah/blah/yackity/smackity");

QString applyMOText = 
  manageOverlays::tr ("This button applies all changes made to the overlays dialog and causes the "
                      "main display to be redrawn.");

QString closeMOText = 
  manageOverlays::tr ("This button closes the overlays dialog.  Changes will not be applied unless the "
                      "Apply button has been pressed.");
