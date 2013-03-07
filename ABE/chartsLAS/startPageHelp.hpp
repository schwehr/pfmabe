QString outText = 
  startPage::tr ("This is the name of the directory where you would like to put all of the output files from this "
                 "program.  Please note that if you choose an existing directory all preexisting LAS files that had been "
                 "produced by prior runs of this program will be removed prior to being rebuilt.  ASCII files will "
                 "always be appended to.  This can result in duplicate points in the ASCII files.");

QString outBrowseText = 
  startPage::tr ("This button will allow you to pick a preexisting directory in which to put the output files from "
                 "this program.  Please note that preexisting LAS files that had been "
                 "produced by prior runs of this program will be removed prior to being rebuilt.  ASCII files will "
                 "always be appended to.  This can result in duplicate points in the ASCII files.");

QString preText = 
  startPage::tr ("This is a prefix that will be added to each output file name.  The file names will consist of a combination "
                 "of the prefix, the area file name, and the input file name.  A file prefix is not required so this "
                 "field may be left blank.");

QString geoidText = 
  startPage::tr ("If this button is toggled on then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) Geoid03 files.  The files must be stored in a directory that "
                 "is pointed to by the environment variable WVS_DIR (this is also where we store our coastline files).  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2003a01.bin</li>"
                 "<li>g2003a02.bin</li>"
                 "<li>g2003a03.bin</li>"
                 "<li>g2003a04.bin</li>"
                 "<li>g2003h01.bin</li>"
                 "<li>g2003p01.bin</li>"
                 "<li>g2003u01.bin</li>"
                 "<li>g2003u02.bin</li>"
                 "<li>g2003u03.bin</li>"
                 "<li>g2003u04.bin</li>"
                 "<li>g2003u05.bin</li>"
                 "<li>g2003u06.bin</li>"
                 "<li>g2003u07b.bin</li>"
                 "<li>g2003u08.bin</li>"
                 "</ul><br>"
                 "<b>IMPORTANT NOTE: This option only works with data collected in KGPS mode.</b>");

QString hDatumText = 
  startPage::tr ("Select the horizontal datum that was used for the data sets.  Either WGS 84 or NAD 83.");
