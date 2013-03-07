QString pfm_fileText = 
  startPage::tr ("Use the browse button to select an input PFM list or handle file.  You cannot modify the text in "
                 "the <b>PFM File</b> text window.  The reason for this is that the file must exist in order for the "
                 "program to run.  Note also that the <b>Next</b> button will not be enabled until you select an input "
                 "PFM file.  When one is selected the default output base name will be supplied in the <b>Output file "
                 "base name</b> text window.");

QString pfm_fileBrowseText = 
  startPage::tr ("Use this button to select the input PFM list or handle file");

QString output_fileText = 
  startPage::tr ("This is the base name for the output files.  The assumption is that the files will be placed in the "
                 "current working directory.  If you want to place them elsewhere you must specify a complete path such "
                 "as <b><i>/data1/datasets/palau/sheets/sheet02</i></b>.  If you do not enter a basename then the pfm "
                 "file name will be used.");

QString area_fileText = 
  startPage::tr ("You may enter an area file name to be used to limit the area of the PFM file from which to extract "
                 "data.  The area file may be in ISS-60 format (*.ARE), generic format (*.are), or USACE format (*.afs).");

QString area_fileBrowseText = 
  startPage::tr ("Use this button to select an optional area file.");
