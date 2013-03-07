
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
