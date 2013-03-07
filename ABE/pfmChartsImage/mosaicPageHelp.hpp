QString cellText = 
  mosaicPage::tr ("Enter the output cell size in meters.");

QString txt_fileText = 
  mosaicPage::tr ("This is the mission parameters file.  You may select it by using the <b>Browse...</b> button to "
		  "the right.");

QString txt_fileBrowseText = 
  mosaicPage::tr ("Select the mission parameters text (.txt) file.  This file should be in the project directory "
		  "under the <b>wks</b> directory.");

QString timeText = 
  mosaicPage::tr ("Enter the TIME_OFFSET (in seconds) to be added to the time of the image before retrieving the "
                  "attitude information from the .pos or SBET file.  This is used to handle any offset between the "
                  "time the photo was taken and the time at which it was time tagged by the system.  It is usually "
                  "set to 0.1 seconds.");

QString rollText = 
  mosaicPage::tr ("Enter the camera roll bias in degrees.  This value will be automatically filled with the value "
		  "in the <b>camera_boresight_roll:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> of the "
		  "mission parameters file if that file was selected above.");

QString pitchText = 
  mosaicPage::tr ("Enter the camera pitch bias in degrees.  This value will be automatically filled with the value "
		  "in the <b>camera_boresight_pitch:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> of the "
		  "mission parameters file if that file was selected above.");

QString headingText = 
  mosaicPage::tr ("Enter the camera heading bias in degrees.  This value will be automatically filled with the value "
		  "in the <b>camera_boresight_heading:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> of the "
		  "mission parameters file if that file was selected above.");

QString focalText = 
  mosaicPage::tr ("Enter the focal length of the camera in millimeters.  This value will be automatically filled with "
		  "the value in the <b>focal_length:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> of the "
		  "mission parameters file if that file was selected above.");

QString pixelText = 
  mosaicPage::tr ("Enter the camera pixel size in microns.  This value will be automatically filled with "
		  "the value in the <b>pixel_size:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> of the "
		  "mission parameters file if that file was selected above.");

QString colText = 
  mosaicPage::tr ("Enter the camera column offset in microns (?).  This value will be automatically filled with "
		  "the first value in the <b>principal_point_offsets:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> "
		  "of the mission parameters file if that file was selected above.");

QString rowText = 
  mosaicPage::tr ("Enter the camera row offset in microns (?).  This value will be automatically filled with "
		  "the second value in the <b>principal_point_offsets:</b> entry of the <b>[DIGITAL_CAMERA_SECTION]</b> "
		  "of the mission parameters file if that file was selected above.");
