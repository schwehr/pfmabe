
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



QString prefsText = 
  examGSF::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  Right now "
               "that's just the position format and the depth units.  Maybe more later...");

QString quitText = 
  examGSF::tr ("<img source=\":/icons/quit.xpm\"> Click this button to <b><em>exit</em></b> from the program.  "
               "You can also use the <b>Quit</b> entry in the <b>File</b> menu.");

QString openText = 
  examGSF::tr ("<img source=\":/icons/fileopen.xpm\"> Click this button to open a new GSF file.  "
               "You can also use the <b>Open</b> entry in the <b>File</b> menu.");

QString otherText = 
  examGSF::tr ("<img source=\":/icons/other.xpm\"> Click this button to toggle the display of the dialog containing "
               "GSF records of types other than GSF_RECORD_SWATH_BATHYMETRY_PING (multibeam ping records).");

QString gsfTableText = 
  examGSF::tr ("These are GSF_RECORD_SWATH_BATHYMETRY_PING records.  If you wish to see GSF records other "
               "than GSF_RECORD_SWATH_BATHYMETRY_PING records use the <img source=\":/icons/other.xpm\"> button.  If you "
               "wish to view information about all of the beams in a particular ping click on the record number on the "
               "left of the window.");

QString closePrefsText = 
  examGSF::tr ("Click this button to close the preferences dialog.");

QString bGrpText = 
  examGSF::tr ("Select the format in which you want all geographic positions to be displayed.");

QString dGrpText = 
  examGSF::tr ("Select the units of choice for depth measurements.  Other than meters you may select archaic "
               "units of measure such as feet, fathoms, or cubits (Roman).  The willett is a very modern unit "
               "of measure named after Craig Willett.  It is equal to 6.096 centimeters.  The genesis of the "
               "willett is beyond the scope of this help text.  For further information contact Craig Willett, "
               "Terry Duvieilh, Becky Martinolich, or Jan Depner.  If other archaic units of measure based on "
               "the king's anatomy are required it would be easy to add them to the list at some point in the "
               "future.  Inches, yards, chains, rods, furlongs, leagues, miles, or others are very simple to "
               "add to the list.  For information call 1-800-555-1212.");

QString scrollBarText = 
  examGSF::tr ("This is a standard scroll bar for moving through the file.  You can drag, click in the trough, "
               "use the arrow buttons, the mouse wheel, or use the <b>Page Up</b> or <b>Page Down</b> buttons.");

QString feetText = 
  examGSF::tr ("Verily and forsooth thou hast left ye olde default unit of measure set to feet.  The king would "
               "be proud!  However, forasmuch as this is the twenty-and-first century, ye might prefer to use the "
               "preferences dialog <img source=\":/icons/prefs.xpm\"> and change this to meters.");

QString fathomText = 
  examGSF::tr ("Aaaarrr, ye scurvy bilge rat, ye've left the default unit o' measure set to fathoms.  P'raps "
               "ye've been swingin' the lead line a bit o'er much lately.  Or, 'tis possible that yer "
               "workin' fer the Navy.  Since we've finally dragged our carcasses into the twenty-first century "
               "ye might give a thought to using the preferences dialog <img source=\":/icons/prefs.xpm\"> to change this "
               "to meters.  Oh, and if ye are with the Navy, keep a weather eye out fer that mangy sea dog "
               "Lieutenant Maynard.");

QString cubitText = 
  examGSF::tr ("Noah....  Noah....  is that you?  You've left your default units of measure set to cubits.  "
               "We're done with the rain already - switch your default units of measure to meters using the "
               "preferences dialog <img source=\":/icons/prefs.xpm\">.");

QString willettText = 
  examGSF::tr ("Willetts as the default unit of measure?!  Come on, that was supposed to be a joke (an inside "
               "joke at that).  Nobody uses willetts to measure (with the possible exception of Craig).  Use "
               "the preferences dialog <img source=\":/icons/prefs.xpm\"> to switch this back to meters.");

QString acknowledgementsText = 
  examGSF::tr ("<center><br>examGSF was built using some of the following Open Source libraries:"
               "<br><br></center>"
               "<ul>"
               "<li><a href=\"http://www.qtsoftware.com/qt\">Qt</a> - A cross-platform application and UI framework</li>"
               "<li><a href=\"http://www.gdal.org\">GDAL</a> - Geospatial Data Abstraction Library</li>"
               "<li><a href=\"http://trac.osgeo.org/proj\">PROJ.4</a> - Cartographic Projections Library</li>"
               "<li><a href=\"http://xerces.apache.org/xerces-c\">XERCES</a> - Validating XML parser libary</li>"
               "<li><a href=\"http://shapelib.maptools.org\">SHAPELIB</a> - Shapefile C Library</li>"
               "<li><a href=\"http://www.zlib.net\">ZLIB</a> - Compression Library</li>"
               "<li><a href=\"http://www.alglib.net\">LEASTSQUARES</a> - Least squares math library</li>"
               "<li><a href=\"http://liblas.org\">LIBLAS</a> - LAS I/O library</li>"
               "<li><a href=\"http://www.hdfgroup.org/HDF5\">HDF5</a> - Heirarchical Data Format library</li>"
               "</ul><br>"
               "<ul>"
               "<li>Qt and SHAPELIB are licensed under the <a href=\"http://www.gnu.org/copyleft/lesser.html\">GNU LGPL</a></li>"
               "<li>GDAL is licensed under an X/MIT style open source license</li>"
               "<li>PROJ.4 is licensed under an MIT open source license</li>"
               "<li>XERCES is licensed under the Apache Software License</li>"
               "<li>ZLIB, LEASTSQUARES, LIBLAS, and HDF5 are licensed under their own open source license</li>"
               "</ul><br><br>"
               "Many thanks to the authors of these and all of their supporting libraries.  For more information on "
               "each library please visit their websites using the links above.<br>"
               "<center>Jan C. Depner<br><br></center>");
