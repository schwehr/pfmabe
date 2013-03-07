QString sourceText = 
  optionsPage::tr ("Select the source of the data to be output.  Either the minimum surface, maximum surface, average surface, or "
                   "all valid data points.");

QString min_surfaceText = 
  optionsPage::tr ("Selecting this button will extract the minimum Z value from each bin and output that value.  The value is "
                   "positioned at the actual position of the point, not the bin center position.");

QString max_surfaceText = 
  optionsPage::tr ("Selecting this button will extract the maximum Z value from each bin and output that value.  The value is "
                   "positioned at the actual position of the point, not the bin center position.");

QString avg_surfaceText = 
  optionsPage::tr ("Selecting this button will extract the average Z value from each bin and output that value.  The value is "
                   "positioned at the center of the bin.");

QString all_surfaceText = 
  optionsPage::tr ("Selecting this button will extract all Z values from the PFM.  This can create quite large files, especially if "
                   "you have selected ASCII output format.");

QString formatText = 
  optionsPage::tr ("Select the output format.  Either ASCII, LLZ, RDP, or SHP.  ASCII data will be output as LAT, LON, Z in decimal "
                   "degrees unless the UTM option is selected.  In that case it will be ZONE, EASTING, NORTHING, Z.  The SHP format is "
                   "ESRI's SHAPEFILE format.  In this case the data is output in POINTZ form.");

QString referenceText = 
  optionsPage::tr ("Check this box to allow reference data points to be included in the output.  This option is only meaningful "
                   "if you select <b>All data points</b> as the data source.");

QString checkedText = 
  optionsPage::tr ("Check this box if you only want to output data from bins that have been marked as checked or verified.");

QString flipText = 
  optionsPage::tr ("Check this box if you want to invert the sign of the Z values.  That is, from depth to elevation.");

QString utmText = 
  optionsPage::tr ("Check this box if you want to output position data in UTM coordinates.  This is only meaningful if you have "
                   "selected ASCII output format.");

QString landText = 
  optionsPage::tr ("Check this box if you want to exclude land data from the output.  Unless you invert the data, land is any value "
                   "less than 0.0.  If you invert the data, it is any value greater than 0.0.  Please note that if the data in your "
                   "PFM has not been datum shifted the output will be FUBAR if you use this option unless you set the datum shift "
                   "value in the appropriate box.");

QString uncertText = 
  optionsPage::tr ("Check this box if you want to output horizontal and vertical uncertainty data, or standard deviation data, along "
                   "with the Y, X, and Z data.  This is only meaningful if you have selected ASCII output format.<br><br>"
                   "If you select <b>All data points</b> for your data source then the output will have the horizontal uncertainty "
                   "and the vertical uncertainty following the Z value.  If you selected one of the surfaces for your data source "
                   "then the output will have the bin standard deviation following the Z value.");

QString cutText = 
  optionsPage::tr ("Set this flag to apply the depth cutoff value.");

QString cutoffText = 
  optionsPage::tr ("Set a depth value cutoff.  Any points below this value will be output as this value.  This can be used to "
                   "set a flat water surface for building DEMs.  This value is checked <b>prior</b> to the datum shift being "
                   "applied and <b>prior</b> to sign inversion.");

QString datumShiftText = 
  optionsPage::tr ("Set a datum shift value.  This value will be <b>added</b> to the data value <b>prior</b> to sign inversion.  "
                   "This can be used to shift from MSL to MLLW or MLLW to ellipsoid heights.");

QString sizeText = 
  optionsPage::tr ("Select the file size limit for the output files.  If this is set to anything other than <b>No limit</b>, files "
                   "will be broken up into multiple files with the selected max file size.");

QString geoidText = 
  optionsPage::tr ("If this box is checked then we will attempt to correct the ellipsoidal height to an orthometric height "
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
                   "<font color=\"red\"><b>IMPORTANT NOTE: This option only works with data collected in KGPS mode that has not been datum shifted.  It is "
                   "the user's responsibility to check and make sure that the data in question was collected in KGPS mode referenced "
                   "from the ellipsoid and has not been datum shifted.  This option is normally only used with LiDAR data.</b></font>");
