QString pfm_fileText = 
  startPage::tr ("Use the browse button to select an input PFM list or handle file.  You cannot modify the text in the "
                  "<b>PFM File</b> text window.  The reason for this is that the file must exist in order for the "
                  "program to run.  Note also that the <b>Next</b> button will not work until you select an input "
                  "PFM file.  When one is selected the default output file name will be supplied in the <b>Output "
                  "GeoTIFF File</b> text window.  The GeoTIFF file name can be edited since it may be a new file or a "
                  "pre-existing file.");

QString pfm_fileBrowseText = 
  startPage::tr ("Use this button to select the input PFM list or handle file");

QString output_fileText = 
  startPage::tr ("You may enter a new file name to be used for the output GeoTIFF file or modify the default file name "
                  "that was placed here when you selected the input PFM file.");

QString output_fileBrowseText = 
  startPage::tr ("Use this button to select a pre-existing GeoTIFF file to be overwritten.");

QString area_fileText = 
  startPage::tr ("You may enter an area file name to be used to limit the area of the PFM file from which to generate "
                  "the GeoTIFF.  The area file may be in ISS-60 format (*.ARE) or generic format (*.are).");

QString area_fileBrowseText = 
  startPage::tr ("Use this button to select an optional area file.");

QString sep_fileText = 
  startPage::tr ("You may enter a CHRTR2 file name to be used to define the Z0 (MSL to local datum) and Z1 (ellipsoid to "
                 "MSL) separation surfaces.");

QString sep_fileBrowseText = 
  startPage::tr ("Use this button to select an optional separation surface CHRTR2 file.");
