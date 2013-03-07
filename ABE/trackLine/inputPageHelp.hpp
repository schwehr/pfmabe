QString formatText = 
  inputPage::tr ("<br><br>The following file formats are accepted by the trackLine program:<br><br>"
                 "<ul>"
                 "<li>GSF (any extension but usually .dNN where NN represents a 2 digit number)</li>"
                 "<li>CHARTS POS/SBET (extension .pos, .out, .POS, .OUT)</li>"
                 "<li>Waveform LIDAR Format (extensions .wlf, .wtf, .whf - only if platform position is available)</li>"
                 "<li>AHAB Hawkeye (extension .bin - only if platform position is available)</li>"
                 "<ul>");

QString inputBrowseText = 
  inputPage::tr ("This button allows you to select multiple input files.  You may use this button as many times as "
		 "needed to define all of the desired input files.  This may also be used in conjunction with the "
		 "<b>Browse Directories</b> button.  All selected files are added to the <b>Input files</b> list.") + formatText;

QString dirBrowseText = 
  inputPage::tr ("This button allows you to select multiple input files by specifiying a top level directory.  Using "
		 "the <b>Directory file mask</b> field as the filter, all files below the selected directory will be "
		 "searched for matching files.  These files will be added to the <b>Input files</b> list.  This may "
		 "also be used in conjunction with the <b>Browse Files</b> button.  You may use this button as many "
		 "times as needed to define all of the desired input files.") + formatText;

QString fileMaskText = 
  inputPage::tr ("This is the file search filter for the <b>Browse Directories</b> button.  You may select a filter "
                "from the existing list or type in your own special filter.  This filter is used in searching "
                "directory trees when using the <b>Browse Directories</b> button.") + formatText;

QString inputFilesText = 
  inputPage::tr ("This is a list of all files that will be input to trackLine program.  You may edit the list or even "
                "add files by hand (although why anyone would do this is beyond me).  There is limited text editor "
                "functionality available for this list.  Right click to get a menu of functions including undo, redo, "
                "cut, paste, and others.  If you make a mistake and add duplicate files using the <b>Browse "
                "Directories</b> button or even the <b>Browse Files</b> button, fear not.  The input file list is sorted "
                "and duplicates are culled out before the trackline generation process is started.") + formatText;
