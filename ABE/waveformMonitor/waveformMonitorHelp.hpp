QString prefsText = 
  waveformMonitor::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  This includes "
                       "position format and the colors.");
QString modeText = 
  waveformMonitor::tr ("<img source=\":/icons/mode_line.xpm\"> <img source=\":/icons/mode_dot.xpm\"> Click this button to toggle between line and dot drawing "
                       "modes for the wave display.  When selected the waves are drawn as lines, when unselected the waves are drawn "
                       "as unconnected dots.");

QString quitText = 
  waveformMonitor::tr ("<img source=\":/icons/quit.xpm\"> Click this button to <b><em>exit</em></b> from the program.  "
                       "You can also use the <b>Quit</b> entry in the <b>File</b> menu.");
QString mapText = 
  waveformMonitor::tr ("This is the waveformMonitor program, a companion to the pfmEdit program for viewing Charts "
                       "LIDAR HOF waveforms.  There are a number of action keys that may be pressed while in the "
                       "pfmEdit window.  The default actions and key values are :<br><br>"
                       "Swap primary and secondary depth - s<br>"
                       "Toggle display of APD waveform - 1<br>"
                       "Toggle display of PMT waveform - 2<br>"
                       "Toggle display of IR waveform - 3<br>"
                       "Toggle display of RAMAN waveform - 4<br><br>"
                       "The actual key values may be changed in pfmEdit using the Preferences->Ancillary Programs "
                       "dialog.  Help is available on all fields in waveformMonitor using the What's This pointer.");

QString bGrpText = 
  waveformMonitor::tr ("Select the format in which you want all geographic positions to be displayed.");

QString closePrefsText = 
  waveformMonitor::tr ("Click this button to close the preferences dialog.");

QString restoreDefaultsText = 
  waveformMonitor::tr ("Click this button to restore colors, size, and position format to the default settings.");

QString nameLabelText = 
  waveformMonitor::tr ("This is the name of the input file from which this point was taken");

QString dateLabelText = 
  waveformMonitor::tr ("This is the date and time of the point.  The fields are, in order, year, month, day, "
                       "(julian day), hour, minutes, and second.");

QString recordLabelText = 
  waveformMonitor::tr ("This is the record number in the input file of this point");

QString dataTypeText = 
  waveformMonitor::tr ("This will either be DGPS or KGPS for differential or kinematic respectively.");

QString correctDepthText = 
  waveformMonitor::tr ("This is the final depth after tide (DGPS) or datum (KGPS) corrections.  Note that these "
                       "corrections may not have been made yet.  Check the tide/datum field.");

QString secDepthText = 
  waveformMonitor::tr ("This is the secondary corrected depth (if available).  Check the tide/datum field to see "
                       "if tide/datum corrections have been applied.");

QString reportedDepthText = 
  waveformMonitor::tr ("This is the depth without the tide/datum correction.  This can come from either the "
                       "kgps_elevation field or the result_depth field depending on whether the survey was done "
                       "in KGPS or DGPS mode respectively.");

QString tideLabelText = 
  waveformMonitor::tr ("This is the tide correction if in DGPS mode or the datum correction if in KGPS mode.  This "
                       "may be 0.0.");

QString waveHeightText = 
  waveformMonitor::tr ("This is the estimated wave height.");

QString abdcLabelText = 
  waveformMonitor::tr ("This is the abbreviated depth confidence.  It is either from ab_dep_conf or kgps_abd_conf "
                       "depending on the survey mode (DGPS or KGPS respectively).  Any value below 70 is "
                       "considered unacceptable.  Some values are hard-wired.  13 indicates land of unknown "
                       "elevation, 70 indicates land of known elevation, 72 indicates shoreline depth swapped "
                       "data.");

QString sabdcLabelText = 
  waveformMonitor::tr ("This is the secondary abbreviated depth confidence.  It is either from sec_ab_dep_conf or "
                       "kgps_sec_abd_conf depending on the survey mode (DGPS or KGPS respectively).  Any value "
                       "below 70 is considered unacceptable.  Some values are hard-wired.  13 indicates land of "
                       "unknown elevation, 70 indicates land of known elevation, 72 indicates shoreline depth "
                       "swapped data.");

QString botBinLabelText = 
  waveformMonitor::tr ("This is the bottom channel and bin used to derive the depth.  The channel will either be "
                       "APD or PMT.  The bin is the index into the wave array.  This will be indicated by a "
                       "(usually) green marker on the wave plot.");

QString secBotBinLabelText = 
  waveformMonitor::tr ("This is the bottom channel and bin used to derive the secondary depth.  The channel will "
                       "either be APD or PMT.  The bin is the index into the wave array.  This will be indicated "
                       "by a (usually) red marker on the wave plot.");

QString sfcBinLabelText = 
  waveformMonitor::tr ("This is the surface channel and bin used to derive the secondary depth.  The channel will "
                       "either be IR, RAMAN, or (rarely) APD.  The bin is the index into the wave array.  This "
                       "will be indicated by a (usually) yellow marker on the wave plot.");

QString fullConfText = 
  waveformMonitor::tr ("Full depth confidence or GG Conf.  Each digit of the full depth confidence is used for "
                       "specific information:<br><br>"
                       "digit 1 - State index (2-4) else (7-9) if killer flag is set<br>"
                       "digit 2 - Mode index (0-3) else (5-8) if BFOM is less than the bottom threshold<br>"
                       "digit 3 - Surface channel used (0-3) else (5-8) if non-killer flag is set<br>"
                       "digit 4 - Wave height in decimeters, valid only when mode index (digit 2) is 0<br>"
                       "digit 5 - SFOM (surface figure of merit) / 10<br>"
                       "digit 6 - Bottom confidence, values greater than or equal to 9 are set to 9<br><br>"
                       "State index settings:<br><br>"
                       "0 - Waiting for first pulse with a valid surface return (internal)<br>"
                       "1 - Filling buffer<br>"
                       "2 - Buffers filled, burst mode for first portion of buffer<br>"
                       "3 - Steady state, one in/one out<br>"
                       "4 - Termination, burst mode for last portion of buffer (caused by insufficient valid data in "
                       "the buffer or reaching the end of a flightline)<br><br>"
                       "Mode index settings:<br><br>"
                       "0 - Normal, wave heights are calculated (this gives the smallest depth error)<br>"
                       "1 - Prediction mode, invalid surface (uses mean altitude)<br>"
                       "2 - Surface return but bad buffer or wave corrector OFF<br>"
                       "3 - No surface, bad buffer, no output<br><br>"
                       "All real-time depths are mode 2 in the aircraft as they are not wave corrected.  Real-time data "
                       "is specifically for go/no go determination.<br><br>"
                       "Ideally, the first two digits of the full depth confidence should be 30 for good data.");

QString secFullConfText = 
  waveformMonitor::tr ("Secondary full depth confidence or GG Conf.  Each digit of the full depth confidence is used for "
                       "specific information:<br><br>"
                       "digit 1 - State index (2-4) else (7-9) if killer flag is set<br>"
                       "digit 2 - Mode index (0-3) else (5-8) if BFOM is less than the bottom threshold<br>"
                       "digit 3 - Surface channel used (0-3) else (5-8) if non-killer flag is set<br>"
                       "digit 4 - Wave height in decimeters, valid only when mode index (digit 2) is 0<br>"
                       "digit 5 - SFOM (surface figure of merit) / 10<br>"
                       "digit 6 - Bottom confidence, values greater than or equal to 9 are set to 9<br><br>"
                       "State index settings:<br><br>"
                       "0 - Waiting for first pulse with a valid surface return (internal)<br>"
                       "1 - Filling buffer<br>"
                       "2 - Buffers filled, burst mode for first portion of buffer<br>"
                       "3 - Steady state, one in/one out<br>"
                       "4 - Termination, burst mode for last portion of buffer (caused by insufficient valid data in "
                       "the buffer or reaching the end of a flightline)<br><br>"
                       "Mode index settings:<br><br>"
                       "0 - Normal, wave heights are calculated (this gives the smallest depth error)<br>"
                       "1 - Prediction mode, invalid surface (uses mean altitude)<br>"
                       "2 - Surface return but bad buffer or wave corrector OFF<br>"
                       "3 - No surface, bad buffer, no output<br><br>"
                       "All real-time depths are mode 2 in the aircraft as they are not wave corrected.  Real-time data "
                       "is specifically for go/no go determination.<br><br>"
                       "Ideally, the first two digits of the full depth confidence should be 30 for good data.");

QString bfomThreshText = 
  waveformMonitor::tr ("Bottom figure of merit threshold times 10.  This value (unitless) is the threshold for "
                       "the bottom signal to noise ratio in the shallow channel.  A peak is not accepted unless it "
                       "exceeds this value, which is nominally set to 6.0.");

QString secBfomThreshText = 
  waveformMonitor::tr ("Secondary bottom figure of merit threshold times 10.  This value (unitless) is the threshold for "
                       "the bottom signal to noise ratio in the deep channel.  A peak is not accepted unless it "
                       "exceeds this value, which is nominally set to 7.0.");

QString sigStrengthText = 
  waveformMonitor::tr ("Bottom signal strength.  Waiting for more info from Jeff Croucher (Fugro)");

QString secSigStrengthText = 
  waveformMonitor::tr ("Secondary bottom signal strength.  Waiting for more info from Jeff Croucher (AKA "
                       "Crouching Gerbil)");


QString haps_versionText = 
  waveformMonitor::tr ("HAPS version.  Waiting for more info from Jeff Croucher (Fugro)");

QString position_confText = 
  waveformMonitor::tr ("Position confidence.  Waiting for more info from Jeff Croucher (Fugro)");

QString suggested_dksText = 
  waveformMonitor::tr ("Suggested <b><i>Delete, Kept, Swapped</i></b> flag.  Waiting for more info from Jeff "
                       "Croucher (Fugro)");

QString suspect_statusText = 
  waveformMonitor::tr ("Suspect <b><i>Delete, Kept, Swapped</i></b> flag.  Waiting for more info from Jeff "
                       "Croucher (Fugro)");

QString tide_statusText = 
  waveformMonitor::tr ("This will either be <b><i>Predicted</i></b>, <b><i>Observed</i></b>, "
                       "<b><i>Verified</i></b>, or (if in KGPS mode) <b><i>N/A</i></b>");

QString topoText = 
  waveformMonitor::tr ("Waiting for more info from Jeff Croucher (Fugro)");

QString latitudeText = 
  waveformMonitor::tr ("Latitude of the point.");

QString longitudeText = 
  waveformMonitor::tr ("Longitude of the point.");

QString sec_latitudeText = 
  waveformMonitor::tr ("Latitude of the secondary point, if available.");

QString sec_longitudeText = 
  waveformMonitor::tr ("Longitude of the secondary point, if available.");

QString altitudeText = 
  waveformMonitor::tr ("Aircraft altitude.");

QString kgps_water_levelText = 
  waveformMonitor::tr ("Waiting for more info from Jeff Croucher (Fugro)");

QString nadir_angleText = 
  waveformMonitor::tr ("This is the angle of the laser off nadir.  This should be very close to 20 degrees.");

QString scanner_azimuthText = 
  waveformMonitor::tr ("This is the laser scanner angle.  It will sweep from approximately -60 degrees on the "
                       "left side to +60 degrees on the right.");

QString sfc_fom_apdText = 
  waveformMonitor::tr ("Surface signal strength for the APD channel.  Waiting for more info from Jeff Croucher "
                       "(Fugro)");

QString sfc_fom_irText = 
  waveformMonitor::tr ("Surface signal strength for the IR channel.  Waiting for more info from Jeff Croucher "
                       "(Fugro)");

QString sfc_fom_ramText = 
  waveformMonitor::tr ("Surface signal strength for the RAMAN channel.  Waiting for more info from Jeff Croucher "
                       "(Fugro)");

QString warningsText = 
  waveformMonitor::tr ("Coded bit warnings.  Waiting for more info from Jeff Croucher (Fugro)");

QString warnings2Text = 
  waveformMonitor::tr ("Coded bit warnings.  Waiting for more info from Jeff Croucher (Fugro)");

QString warnings3Text = 
  waveformMonitor::tr ("Coded bit warnings.  Waiting for more info from Jeff Croucher (Fugro)");

QString calc_bot_run0Text = 
  waveformMonitor::tr ("Calculated HAP for primary.  Waiting for more info from Jeff Croucher (AKA Crouching Gerbil)");

QString calc_bot_run1Text = 
  waveformMonitor::tr ("Calculated HAP for secondary.  Waiting for more info from Jeff Croucher (AKA Crouching Gerbil)");

QString primary_runText = 
  waveformMonitor::tr ("Run length of the primary run using a minimum run length of 6.  This run length is not based on the same  "
                       "algorithms that Optech uses.  We do not count 0 rise as a run point.  We also don't vary the minimum run "
                       "length.  Because of this we may not detect some runs that GCS does.  This value is mainly used for checking "
                       "settings for the charts_swap smart swap option.");

QString primary_slopeText = 
  waveformMonitor::tr ("Pseudo-slope value for the primary run.  Since the units used in the rise over run calculation are not the same "
                       "this is not a real slope value.  Empirically we have found that a pseudo-slope of 1.0 or greater is significant.  "
                       "A primary run with a slope of less than 1.0 is usually not considered valid.  The run length used to detect a "
                       "run is not based on the same algorithms that Optech uses.  We do not count 0 rise as a run point.  We also don't "
                       "vary the minimum run length.  Because of this we may not detect some runs that GCS does.  This value is mainly "
                       "used for checking settings for the charts_swap smart swap option.");

QString primary_backslopeText = 
  waveformMonitor::tr ("Pseudo-slope value of the falling side of the run for the primary run.  Since the units used in the rise over run "
                       "calculation are not the same this is not a real slope value.  Empirically we have found that a pseudo-slope of 1.0 "
                       "or greater is significant.  A primary run with a slope of less than 1.0 is usually not considered valid.  The "
                       "run length used to detect a run is not based on the same algorithms that Optech uses.  We do not count 0 rise as a "
                       "run point.  We also don't vary the minimum run length.  Because of this we may not detect some runs that GCS "
                       "does.  This value is mainly used for checking settings for the charts_swap smart swap option.");

QString secondary_runText = 
  waveformMonitor::tr ("Run length of the secondary run using a minimum run length of 6.  This run length is not based on the same  "
                       "algorithms that Optech uses.  We do not count 0 rise as a run point.  We also don't vary the minimum run "
                       "length.  Because of this we may not detect some runs that GCS does.  This value is mainly used for checking "
                       "settings for the charts_swap smart swap option.");

QString secondary_slopeText = 
  waveformMonitor::tr ("Pseudo-slope value for the secondary run.  Since the units used in the rise over run calculation are not the same "
                       "this is not a real slope value.  Empirically we have found that a pseudo-slope of 1.0 or greater is significant.  "
                       "A secondary run with a slope of less than 1.0 is usually not considered valid.  The run length used to detect a "
                       "run is not based on the same algorithms that Optech uses.  We do not count 0 rise as a run point.  We also don't "
                       "vary the minimum run length.  Because of this we may not detect some runs that GCS does.  This value is mainly "
                       "used for checking settings for the charts_swap smart swap option.");

QString secondary_backslopeText = 
  waveformMonitor::tr ("Pseudo-slope value of the falling side of the run for the secondary run.  Since the units used in the rise over run "
                       "calculation are not the same this is not a real slope value.  Empirically we have found that a pseudo-slope of 1.0 "
                       "or greater is significant.  A secondary run with a slope of less than 1.0 is usually not considered valid.  The "
                       "run length used to detect a run is not based on the same algorithms that Optech uses.  We do not count 0 rise as a "
                       "run point.  We also don't vary the minimum run length.  Because of this we may not detect some runs that GCS "
                       "does.  This value is mainly used for checking settings for the charts_swap smart swap option.");

QString acknowledgementsText = 
  waveformMonitor::tr ("<center><br>waveformMonitor was built using some of the following Open Source libraries:"
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
