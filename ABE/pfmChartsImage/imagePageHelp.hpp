QString datumText = 
  imagePage::tr ("Input the offset, in meters, from the ellipsoid to the PFM datum (this will be added to the DEM data "
                 "and the altitude).  You may press the <b>Get EGM08 offset</b> button to retrieve the EGM08 model value "
                 "of the ellipsoid to geoid (approximate mean sea level) datum offset for the point nearest the center "
                 "of the minimum bounding rectangle of the area file.<br><br>"
                 "<b>IMPORTANT NOTE: If you leave this set to zero, datum shifts will be retrieved from the HOF and/or "
                 "TOF files to be applied to the altitude.  This is the most accurate way to run it.  If no datum shifts "
                 "have been done to the HOF and/or TOF files and you set this to a value, the value will be applied to "
                 "both the data and the altitude.</b>");

QString egmText = 
  imagePage::tr ("Press this button to retrieve the EGM08 model value of the ellipsoid to geoid (approximate mean sea "
                 "level) datum offset for the point nearest the center of the minimum bounding rectangle of the area "
                 "file.");

QString casiText = 
  imagePage::tr ("Press this button to output a DEM file in ASCII xyz format for use in CASI processing.");

QString hofText = 
  imagePage::tr ("Press this button if you only want to use images that are associated with Optech Hydrographic "
                 "Output Format (HOF) data files.  This option does not affect generation of the digital elevation "
                 "model (DEM).");

QString tofText = 
  imagePage::tr ("Press this button if you only want to use images that are associated with Optech Topographic "
                 "Output Format (TOF) data files.  This option does not affect generation of the digital elevation "
                 "model (DEM).");

QString bothText = 
  imagePage::tr ("Press this button if you want to use images that are associated with Optech Hydrographic Output "
                 "Format (HOF) and Topographic Output Format (TOF) data files.  This option does not affect "
                 "generation of the digital elevation model (DEM).");

QString demText = 
  imagePage::tr ("Input the desired Digital Elevation Model grid size in meters.");

QString intervalText = 
  imagePage::tr ("Enter the desired interval between images.  Use this to thin out the number of images used "
                 "for the photo mosaic.  For example, setting this to 2 will cause the program to extract "
                 "every other picture.");

QString lowText = 
  imagePage::tr ("Set the lower direction limit for the images.  Images from lines flown with a heading of less than "
                 "this value will be excluded from the extraction process.");

QString highText = 
  imagePage::tr ("Set the upper direction limit for the images.  Images from lines flown with a heading of greater than "
                 "this value will be excluded from the extraction process.");

QString oppositeText = 
  imagePage::tr ("Check this box to include images from lines that are run in the opposite direction to the limits "
		 "set in the <b>Upper</b> and <b>Lower Line Direction Limits</b> fields.");

QString excludeText = 
  imagePage::tr ("Check this box to exclude images that are less than 150,000 bytes in size.  Files of "
                 "less than 150,000 bytes are usually either too bright or too dark to see any details.");

QString normalizeText = 
  imagePage::tr ("Check this box to run the ImageMagick <b>convert</b> program with the <b>-normalize</b> option "
                 "on the images prior to mosaicing the images.  This can take quite some time to do and is "
                 "only available on Linux systems at this time.  It can also give you some strange looking "
                 "mosaics so you probably don't want to try this except as a last resort.");

QString flipText = 
  imagePage::tr ("Check this box to invert the sign on the Z values.  If data has been stored as depths (positive "
                 "down), checking this box will cause depths to become negative and elevations to become positive.  "
                 "If the data was stored as elevations (positive up), checking this box will cause elevations to "
                 "become negative and depths to become positive.");

QString posText = 
  imagePage::tr ("Check this box to exclude images that will use .pos file navigation.  Only images that have "
                 "associated SBET or sbet navigation files will be used.");

QString srtmText = 
  imagePage::tr ("Check this box to include Shuttle Radar Topography Mission data in the DEM wherever there is no "
                 "recorded HOF or TOF data.  This data will be shifted to approximately match the datum for the "
                 "input data but will not be an exact match (not worth the effort).");
