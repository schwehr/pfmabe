
QString out_fileText = 
  runPage::tr ("Enter a TIFF file name to be used for output.  If you do not append .tif to the file "
	       "it will be automatically added.");

QString out_fileBrowseText = 
  runPage::tr ("Select a preexisting TIFF file to be used for output.");

QString utmText = 
  runPage::tr ("This check box will force the output GeoTIFF file to be a UTM projected grid.");

QString geoText = 
  runPage::tr ("This check box will force the output GeoTIFF file to be an unprojected latitude/longitude grid.");

QString carisText = 
  runPage::tr ("This check box will force the output to be unblocked and use <b>PACKBITS</b> compression "
	       "because Caris can't be bothered to learn how to read a GeoTIFF standard file.<br><br>"
	       "<b><i>WARNING - This is really slow so unless you need to put this GeoTIFF into Caris "
	       "don't use it!  If you must use it, make sure that your output file is on a local disk "
	       "not an NFS mounted disk (/net/whatever).  This option WILL NOT be saved for your next "
	       "run.</i></b>");
