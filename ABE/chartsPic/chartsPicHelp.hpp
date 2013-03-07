QString rotateText = 
  chartsPic::tr ("<img source=\":/icons/rotate.xpm\"> Click this button to rotate the pictures to north up.  In most cases this "
                 "button is only used as an indicator of the <b>rotation</b> status.  Rotating the display is usually "
                 "done from within <b>pfmEdit</b> or <b>geoSwath</b> by pressing the <b>rotate</b> action key.  By "
                 "default this is the <b><i>r</i></b> key.  Look in <b>pfmEdit</b> or <b>geoSwath</b> under "
                 "<b>Preferences->Ancillary Programs</b> to see what the key is actually set to.  The default "
                 "list of keys for chartsPic is as follows:"
                 "<ul>"
                 "<li>r - display the picture in north-up rotated view</li>"
                 "<li>t - save the current picture to a file indicated in ABE_SHARE memory (only used in pfmEdit)</li>"
                 "</ul>");

QString brightenText = 
  chartsPic::tr ("Click this button to brighten the picture.");

QString darkenText = 
  chartsPic::tr ("Click this button to darken the picture.");

QString prefsText = 
  chartsPic::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  Right now "
                 "that's just the position format.  Maybe more later...");

QString quitText = 
  chartsPic::tr ("<img source=\":/icons/quit.xpm\"> Click this button to <b><em>exit</em></b> from the program.  "
                 "You can also use the <b>Quit</b> entry in the <b>File</b> menu or the optional quit key "
                 "that may have been passed from <b>pfmEdit</b> or <b>geoSwath</b>.");

QString saveText = 
  chartsPic::tr ("<img source=\":/icons/fileopen.xpm\"> Click this button to save the picture at it's original size "
                 "to a file.");

QString scaledText = 
  chartsPic::tr ("<img source=\":/icons/scaled.xpm\"> Click this button to save the picture at it's scaled/rotated "
                 "(WYSIWIG) size to a file.");

QString picText = 
  chartsPic::tr ("This is the ChartsPic program, a companion to the <b>pfmEdit</b> and/or <b>geoSwath</b> program.  "
                 "It is used for viewing CHARTS down-looking digital camera pictures.  This program is never run as "
                 "a stand-alone program but is <i><b>shelled</b></i> from other programs such as <b>pfmEdit</b> or "
                 "<b>geoSwath</b>.<br><br>"
                 "You may place <b><i>stickpins</i></b> in the picture to mark locations by left clicking in the "
                 "picture.  These will be thrown away whenever you change images.  You can save the "
                 "<b><i>stickpins</i></b> in the picture by saving the scaled picture or using the shared memory "
                 "hotkey described below.<br><br>"
                 "The box cursor will track the movement of the cursor in the <b>pfmEdit</b> or <b>geoSwath</b> window "
                 "as well as possible.  The error in position will generally be on the order of 5 meters at the center "
                 "of the picture and somewhat more toward the outer edges.  A couple of the functions/buttons have remote "
                 "action keys defined in <b>geoSwath</b> and/or <b>pfmEdit</b>.  By default these are:"
                 "<ul>"
                 "<li>r - display the picture in north-up rotated view</li>"
                 "<li>t - save the current picture to a file indicated in ABE_SHARE memory (only used in pfmEdit)</li>"
                 "</ul>"
                 "The options need to be listed in the above order in the <b>Preferences-Ancillary Programs</b> "
                 "dialog of <b>geoSwath</b> or <b>pfmEdit</b>.  For instance - r,t");

QString bGrpText = 
  chartsPic::tr ("Select the format in which you want all geographic positions to be displayed.");

QString closePrefsText = 
  chartsPic::tr ("Click this button to close the preferences dialog.");

QString acknowledgementsText = 
  chartsPic::tr ("<center><br>chartsPic was built using most, if not all, of the following Open Source libraries:"
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
