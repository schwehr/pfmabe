
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



/*****************************************************************************\

    This program is public domain software that was developed by 
    the U.S. Naval Oceanographic Office.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

\*****************************************************************************/


QString abeText = 
  abe::tr ("The abe program is just a simple PFM Area-Based Editor program launcher.  Just press a button to start a program.  "
           "Help is available on each of the program buttons by selecting the <b>What's This</b> button in the Help menu "
           "and then clicking on the program of interest.");

QString abeAboutText = 
  abe::tr ("<center><br>abe<br><br>"
           "Author : Jan C. Depner (jan.depner@navy.mil)<br>"
           "Date : 19 January 2010</center><br><br>"
           "The abe program is just a simple PFM Area-Based Editor program launcher.  Just press a button to start a program.  "
           "You can also drag and drop files on the buttons.  For example, you can drag and drop a .pfm file on the "
           "pfmView button to start the pfmView program using that .pfm file.  "
           "Help is available on each of the program buttons by selecting the <b>What's This</b> button in the Help menu "
           "and then clicking on the program of interest.");

QString acknowledgementsText = 
  abe::tr ("<center><br>abe was built using some, if not all, of the following Open Source libraries:"
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
