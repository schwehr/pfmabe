
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



QString inputBrowseText = 
  inputPage::tr ("This button allows you to select multiple input files.  You may use this button as many times as "
                 "needed to define all of the desired input files.  This may also be used in conjunction with the "
                 "<b>Browse Directories</b> button.  All selected files are added to the <b>Input files</b> list.<br><br>"
                 "<b>WARNING: Files that have already been loaded into <i>ANY<i> of the preexisting PFM structures "
                 "will be removed from this list when the load is started!</b>");

QString dirBrowseText = 
  inputPage::tr ("This button allows you to select multiple input files by specifiying a top level directory.  Using "
                 "the <b>Directory file mask</b> field as the filter, all files below the selected directory will be "
                 "searched for matching files.  These files will be added to the <b>Input files</b> list.  This may "
                 "also be used in conjunction with the <b>Browse Files</b> button.  You may use this button as many "
                 "times as needed to define all of the desired input files.<br><br>"
                 "<b>WARNING: Files that have already been loaded into <i>ANY<i> of the preexisting PFM structures "
                 "will be removed from this list when the load is started!  Also, cheating just a bit, *.hof LIDAR "
                 "files that have a filename like this - 03DS03044_012_031205_0028_TA_05272.hof will not be "
                 "loaded using <i>Browse Directories</i>!  These are, by definition, timing lines.  If you wish to load "
                 "a timing line use the <i>Browse Files</i> button.</b>");

QString pfmBrowseText = 
  inputPage::tr ("This button allows you to select a preexisting PFM structure.  All input files in the PFM structure "
                 "will be added to the input file list.  This can be handy if you want to create a duplicate PFM "
                 "structure with the same input files but different options.  It can also be used for testing unload "
                 "of edited points - edit the existing PFM, unload the edits, then build a new PFM structure using the "
                 "same input files.<br><br>"
                 "<b>IMPORTANT NOTE: Make sure that the files in the existing PFM have a proper path.</b>");

QString fileMaskText = 
  inputPage::tr ("This is the file search filter for the <b>Browse Directories</b> button.  You may select a filter "
                 "from the existing list or type in your own special filter.  This filter is used in searching "
                 "directory trees when using the <b>Browse Directories</b> button.");

QString inputFilesText = 
  inputPage::tr ("This is a list of all files that will be input to the PFM structure.  You may edit the list or even "
                 "add files by hand (although why anyone would do this is beyond me).  There is limited text editor "
                 "functionality available for this list.  Right click to get a menu of functions including undo, redo, "
                 "cut, paste, and others.");
