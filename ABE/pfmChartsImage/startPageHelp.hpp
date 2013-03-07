QString pfm_fileText = 
  startPage::tr ("Use the browse button to select an input PFM list or handle file.  You cannot modify the text in the "
                 "<b>PFM File</b> text window.  The reason for this is that the file must exist in order for the "
                 "program to run.  Note also that the <b>Next</b> button will not be enabled until you select an input "
                 "PFM file.");

QString pfm_fileBrowseText = 
  startPage::tr ("Use this button to select the input PFM list or handle file");

QString area_fileText = 
  startPage::tr ("You must enter an area file name to be used to limit the area of the PFM file for which you "
                 "will extract images.  The area file may be in ISS-60 format (*.ARE), generic format (*.are), "
                 "or Army Corps format (*.afs).");

QString area_fileBrowseText = 
  startPage::tr ("Use this button to select an area file.");

QString image_dirText = 
  startPage::tr ("This is the image file directory name.  This will be automatically generated from the input "
                 "PFM file name and the input area name.  You may modify this name if you want (although why you "
                 "would want to is beyond me).  If you check the <b>Remove image directory</b> check box, this "
                 "directory and its contents will be removed when the GeoTIFF mosaic files (UTM and geographic) "
                 "have been generated.");

QString removeText = 
  startPage::tr ("If you check this box, the <b>Image directory</b> and its contents will be removed when "
                 "the GeoTIFF mosaic files (UTM and geographic) have been generated.");

QString level_fileText = 
  startPage::tr ("You may enter an area file name that covers an area that you wish to set to a specific level in the "
                 "DEM file.  The area file may be in ISS-60 format (*.ARE), generic format (*.are), or Army Corps "
		 "format (*.afs).");

QString level_fileBrowseText = 
  startPage::tr ("Use this button to select an optional area file that will be set to the indicated level value in the "
		 "DEM.");

QString levelText = 
  startPage::tr ("Set the value that will be used in the DEM for the selected area file.");
