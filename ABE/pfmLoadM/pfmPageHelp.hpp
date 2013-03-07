
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



QString pfm_fileText = 
  pfmPage::tr ("This is the name of the PFM handle file.  You may either type in the name or use the "
               "<b>Browse...</b> button to select a preexisting file (or create a PFM structure in a directory "
               "other than the current working directory).<br><br>"
               "If you do not fully qualify the PFM name (for example: "
               "/data1/datasets/sftf/sftf_final on Linux or N:\\sftf\\sftf_final on Windows) then the PFM structure "
               "will be created in the current working directory.  If you started the program from a desktop "
               "shortcut or from a menu the current working directory will be your home directory.  You almost never "
               "want to do that ;-)<br><br>"
               "  If you type in the name of an existing "
               "PFM handle file (with or without the .pfm extension) all of the required settings will no longer "
               "be modifiable.  This will also occur if you select a preexisting PFM file using the <b>Browse...</b> "
               "button.  Note that old style (pre 4.5) PFM list files are not supported by this loader.<br><br>"
               "<b>NOTE: You may define up to 30 PFM structures to be loaded using the same input files.  Simply click "
               "on the <i>Next</i> button to define another PFM structure.  This is useful for areas that consist of "
               "a number of sheets that were surveyed all at once.  When you are finished defining PFM structures to "
               "load, click <i>Next</i> to go to the next page, then click <i>Finish</i> to move to the <i>Global "
               "Options</i> page.</b>");

QString pfm_fileBrowseText = 
  pfmPage::tr ("Use this button to select a preexisting PFM structure or to create a PFM in a different directory "
               "than the current working directory.  When you select a preexisting PFM file all of the required "
               "settings will no longer be modifiable.  When creating a new PFM you do not need to add .pfm to the name.  "
               "Note that old style (pre 4.5) PFM list files are not supported by this loader.");

QString mBinSizeText = 
  pfmPage::tr ("Enter the desired PFM bin size in meters.  Normally, this size is chosen based on the desired "
               "resolution of the end product.  A good rule of thumb is to take the sonar footprint size or LIDAR "
               "spot density at the average depth in the area and double it.  Using this bin size generates bins of "
               "approximately the requested size in meters.  If you set bin size this way it overrides the geographic "
               "bin size.");

QString gBinSizeText = 
  pfmPage::tr ("Enter the desired PFM bin size in decimal minutes of latitude.  Using this bin size generates bins of "
               "exactly the desired size in minutes of latitude and longitude.  North of 64N and south of 64S the bins "
               "will be exactly the entered value in latitude but will be the equivalent distance in the longitudinal "
               "direction at the north or south boundary of the area (depending on hemisphere).  For example, an area "
               "that extends from 80N to 83N with a bin size setting of 0.1 minutes would have and approximate "
               "longitudinal bin size of 0.5759 minutes.  If you set bin size this way it overrides the meter bin size.");

QString minDepthText = 
  pfmPage::tr ("Enter the estimated minimum depth for the area that is being loaded.  This value is used to limit "
               "the amount of storage required for the data.  If you think you may add DEM (like DTED) data to the "
               "file at some later time or deeper depth data than what you have at present make sure to allow for "
               "that.  WARNING - any sounding less than this minimum depth or greater than the maximum depth will be "
               "automatically flagged as filter edited and stored back into the input files as invalid when unloaded.");

QString maxDepthText = 
  pfmPage::tr ("Enter the estimated maximum depth for the area that is being loaded.  This value is used to limit "
               "the amount of storage required for the data.  If you think you may add DEM (like DTED) data to the "
               "file at some later time or deeper depth data than what you have at present make sure to allow for "
               "that.  WARNING - any sounding less than this minimum depth or greater than the maximum depth will be "
               "automatically flagged as filter edited and stored back into the input files as invalid when unloaded.");

QString precisionText = 
  pfmPage::tr ("Each of the soundings that is loaded into the PFM structure is stored as a scaled integer.  To do this "
               "we must set a precision that is converted to a multiplier or scale factor.  The precision of the data "
               "in the PFM structure will be dependent on this scale factor.  In theory you should be able to use any "
               "precision you want to get the exact scale factor but in practice we have found that the three values "
               "given here are sufficient to handle the desired ranges.  0.01 gives one centimeter precision, 0.10 "
               "gives decimeter precision, and 1.00 yields one meter precision.  For shallow water you would normally "
               "use 0.01, for medium depths, 0.1, and for deep water, 1.0.  If in doubt just leave it set to 0.01 "
               "since the only result will be a somewhat larger PFM structure.");

QString areaText = 
  pfmPage::tr ("This is the area file name that can be selected using the <b>Browse...</b> button.  This field is not "
               "editable since the area file is required and must be preexisting.  The area file may be in the ISS-60 "
               "survey planner area format (.ARE) or it may be in the NAVO generic area format (.are).  Generic "
               "format files contain a simple list of polygon points.  The points may be in any of the following "
               "formats:"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
               "The lat and lon must be entered one per line, separated by a comma.  You do not need to repeat the "
               "first point, the polygon will be closed automatically.  The file selection dialog box will "
               "automatically place you in the directory pointed to by the $U_SPROOT environment variable (if "
               "present).");

QString areaBrowseText = 
  pfmPage::tr ("Use this button to select an area file.  The area file may be in the ISS-60 survey planner area "
               "format (.ARE) or it may be in the NAVO generic area format (.are).  Generic format files contain a "
               "simple list of polygon points.  The points may be in any of the following formats:"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
               "The lat and lon must be entered one per line, separated by a comma.  You do not need to repeat the "
               "first point, the polygon will be closed automatically.  The file selection dialog box will "
               "automatically place you in the directory pointed to by the $U_SPROOT environment variable (if "
               "present).");

QString area_mapText = 
  pfmPage::tr ("Use this button to run the <b>areaCheck</b> program in order to create an area file.  If an area file "
               "has already been selected then you will be able to edit it using areaCheck.  If you create an area file "
               "it will be named the same as your PFM handle file with an added extension of <b>.are</b>.  You will not "
               "be able to create an area file unless you have specified a PFM file name.");

QString area_PFMText = 
  pfmPage::tr ("Use this button to use the area that is defined internally in a pre-existing PFM structure.  An area file "
               "will be created with the dame name as your PFM handle file with an added extension of <b>.are</b>.  You will not "
               "be able to create an area file unless you have specified a PFM file name.");

QString mosaicText = 
  pfmPage::tr ("This is the associated UNISIPS sidescan mosaic file if one is available.");

QString mosaicBrowseText = 
  pfmPage::tr ("Use this button to select the associated UNISIPS sidescan mosaic file if one is available.  In "
               "practice we seldom use the UNISIPS mosaic files anymore as we can now load UNISIPS sidescan data "
               "files directly into the PFM structure (if you do this be sure to load them as a separate group and "
               "set the <b>Mark data as reference</b> flag since the depth data is usually not sufficiently "
               "accurate).  Normally we will leave this field set to <b>NONE</b>.");

QString featureText = 
  pfmPage::tr ("This is the associated Binary Feature Data (BFD) file if one is available.");

QString featureBrowseText = 
  pfmPage::tr ("Use this button to select a Binary Feature Data (BFD) file.  These files are usually created using "
               "the <b>build_feature</b> program but may also be created later in the pfmEdit program by adding a "
               "feature (the file will automatically be created and named PFM_FILE_NAME.bfd).  If you don't have "
               "a feature file just leave this set to <b>NONE</b>.");

QString stdText = 
  pfmPage::tr ("Set the number of standard deviations used in the area based covariance filter.  Normal filtering is "
               "usually set at 2.4, heavy (<b><i>MP</i></b>) filtering is usually set at 2.0.");

QString featureRadiusText = 
  pfmPage::tr ("This is the radius in meters around known features (from the Binary Feature Data file) that will "
               "not be filtered.");

QString applyFilterText = 
  pfmPage::tr ("Set this check box if you wish to perform area based filtering.  It is not advisable to filter data to "
               "be used for hydrographic products unless you also set the <b>Deep Filter Only</b> button.");

QString deepFilterText = 
  pfmPage::tr ("Set this check box to only filter values that are of greater depth than the average surface.  This is "
               "usually set by default as it is safer than removing both shoal and deep flyers.");

QString area_nsewText = 
  pfmPage::tr ("Press this button to bring up a dialog for defining an area with the north, south, east, and west "
	       "boundaries.");

QString northText = 
  pfmPage::tr ("Set the northern boundary of the area.  Allowable formats are:<br><br>"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
	       "<br><br>");

QString southText = 
  pfmPage::tr ("Set the southern boundary of the area.  Allowable formats are:<br><br>"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
	       "<br><br>");

QString eastText = 
  pfmPage::tr ("Set the eastern boundary of the area.  Allowable formats are:<br><br>"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
	       "<br><br>");

QString westText = 
  pfmPage::tr ("Set the western boundary of the area.  Allowable formats are:<br><br>"
               "<ul>"
               "<li>Hemisphere Degrees Minutes Seconds.decimal</li>"
               "<li>Hemisphere Degrees Minutes.decimal</li>"
               "<li>Hemisphere Degrees.decimal</li>"
               "<li>Sign Degrees Minutes Seconds.decimal</li>"
               "<li>Sign Degrees Minutes.decimal</li>"
               "<li>Sign Degrees.decimal</li>"
               "</ul>"
	       "<br><br>");

QString applyNSEWText = 
  pfmPage::tr ("Press this button to accept the values entered in the boundary fields and close the dialog.  "
	       "Note that all four fields must be filled.");

QString closeNSEWText = 
  pfmPage::tr ("Press this button to discard the values entered and close the dialog.");
