QString prefsText = 
  CZMILwaveMonitor::tr ("<img source=\":/icons/prefs.xpm\"> Click this button to change program preferences.  This includes "
                   "position format and the colors.");
QString modeText = 
  CZMILwaveMonitor::tr ("<img source=\":/icons/mode_line.xpm\"> <img source=\":/icons/mode_dot.xpm\"> Click this button to "
                   "toggle between line and dot drawing modes for the wave display.  When selected the waves are drawn "
                   "as lines, when unselected the waves are drawn as unconnected dots.");

QString horVerText = CZMILwaveMonitor::tr ("<img source =\":/icons/verArrow.png\"><img source=\":/icons/horArrow.png\">Click "
                                      "this button to toggle between displaying the waveform signatures either vertically "
                                      "or horizontally.  When selected, the signatures run vertical, when unselected, the "
                                      "signatures run horizontal.");

QString intMeterText = CZMILwaveMonitor::tr ("<img source=\":/icons/ColumnChart.png\"> Click this button to activate an "
                                        "interactive count meter that will slice through a bin of all waveform signatures "
                                        "(APD, IR, RAMAN, and PMT).  The intensity meter provides information about the "
                                        "counts of all waveforms at a time bin of interest.");

QString quitText = 
  CZMILwaveMonitor::tr ("<img source=\":/icons/quit.xpm\"> Click this button to <b><em>exit</em></b> from the program.  "
                   "You can also use the <b>Quit</b> entry in the <b>File</b> menu.");
QString mapText = 
  CZMILwaveMonitor::tr ("This is the CZMILwaveMonitor program, a companion to the pfmEdit program for viewing Charts "
                   "LIDAR HOF waveforms.  There are a number of action keys that may be pressed while in the "
                   "pfmEdit window.  The default actions and key values are :<br><br>"
                   "<ul>"
                   "<li>n - Toggle between nearest neighbor and single waveform display mode</li>"
                   "<li>1 - Toggle display of PMT waveform</li>"
                   "<li>2 - Toggle display of APD waveform</li>"
                   "<li>3 - Toggle display of TBD waveform</li>"
                   "<li>4 - Toggle display of Raman waveform</li>"
                   "<li>5 - Toggle display of IR waveform</li>"
                   "<li>f - Toggle freeze of the waveform (freezes all external programs from pfmEdit)</li>"
                   "<li>F - Toggle freeze of all of the waveforms (freezes all external programs from pfmEdit)</li>"
                   "</ul>"
                   "The actual key values may be changed in pfmEdit using the Preferences->Ancillary Programs "
                   "dialog.  Help is available on all fields in CZMILwaveMonitor using the What's This pointer.");

QString bGrpText = 
  CZMILwaveMonitor::tr ("Select the format in which you want all geographic positions to be displayed.");

QString closePrefsText = 
  CZMILwaveMonitor::tr ("Click this button to close the preferences dialog.");

QString restoreDefaultsText = 
  CZMILwaveMonitor::tr ("Click this button to restore colors, size, and position format to the default settings.");

QString nameLabelText = 
  CZMILwaveMonitor::tr ("This is the name of the input file from which this point was taken");

QString correctDepthText = 
  CZMILwaveMonitor::tr ("This is the final depth after tide (DGPS) or datum (KGPS) corrections.  Note that these "
                   "corrections may not have been made yet.  Check the tide/datum field.");

QString simDepthText =
	CZMILwaveMonitor::tr("This field is simply the simulated depth field.");
	
QString simDepth1Text =
	CZMILwaveMonitor::tr("This field is simply the simulated depth1 field.");

QString simDepth2Text =
	CZMILwaveMonitor::tr("This field is simply the simulated depth2 field.");

QString simKGPSElevText =
	CZMILwaveMonitor::tr("This field is simply the simulated KGPS elevation field.");

QString reportedDepthText = 
  CZMILwaveMonitor::tr ("This is the depth without the tide/datum correction.  This can come from either the "
                   "kgps_elevation field or the result_depth field depending on whether the survey was done "
                   "in KGPS or DGPS mode respectively.");

QString abdcLabelText = 
  CZMILwaveMonitor::tr ("This is the abbreviated depth confidence.  It is either from ab_dep_conf or kgps_abd_conf "
                   "depending on the survey mode (DGPS or KGPS respectively).  Any value below 70 is "
                   "considered unacceptable.  Some values are hard-wired.  13 indicates land of unknown "
                   "elevation, 70 indicates land of known elevation, 72 indicates shoreline depth swapped "
                   "data.");

QString bfomThreshText = 
  CZMILwaveMonitor::tr ("Bottom figure of merit threshold times 10.  This value (unitless) is the threshold for "
                   "the bottom signal to noise ratio in the shallow channel.  A peak is not accepted unless it "
                   "exceeds this value, which is nominally set to 6.0.");
