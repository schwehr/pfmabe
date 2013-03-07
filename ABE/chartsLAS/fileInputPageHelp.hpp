QString fileInputBrowseText = 
  fileInputPage::tr ("This button allows you to select multiple input files.  You may use this button as many times as "
                     "needed to define all of the desired input files.  This may also be used in conjunction with the "
                     "<b>Browse Directories</b> button.  All selected files are added to the <b>Input files</b> list.  "
                     "The input file list may be edited by hand if needed.  Right click in the window to get edit "
                     "options.");

QString fileDirBrowseText = 
  fileInputPage::tr ("This button allows you to select multiple input files by specifiying a top level directory.  Using "
                     "the <b>Directory file mask</b> field as the filter, all files below the selected directory will be "
                     "searched for matching files.  These files will be added to the <b>Input files</b> list.  This may "
                     "also be used in conjunction with the <b>Browse Input Files</b> button.  You may use this button as many "
                     "times as needed to define all of the desired input files.  The input file list may be edited by hand "
                     "if needed.  Right click in the window to get edit options.<br><br>"
                     "<b>WARNING: *.hof LIDAR files that have a filename like this - 03DS03044_012_031205_0028_TA_05272.hof "
                     "will not be loaded using <i>Browse Directories</i>!  These are, by definition, timing lines.  If you "
                     "wish to load a timing line use the <i>Browse Input Files</i> button.</b>");

QString fileFileMaskText = 
  fileInputPage::tr ("This is the file search filter for the <b>Browse Directories</b> button.  You may select a filter "
                     "from the existing list or type in your own special filter.  This filter is used in searching "
                     "directory trees when using the <b>Browse Directories</b> button.");


QString fileInputFilesText = 
  fileInputPage::tr ("This is a list of all files that will be read to create the LAS or ASCII text output files.  You may "
                     "edit the list or even add files by hand (although why anyone would do this is beyond me).  There is "
                     "limited text editor functionality available for this list.  Right click to get a menu of functions "
                     "including undo, redo, cut, paste, and others.");
