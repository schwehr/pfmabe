
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



//!  This file contains help documentation for the changePath.cpp file.

QString fileListText = 
  changePath::tr ("This is a list of the input files for the selected PFM structure.  You may copy and paste part "
		  "of the path from any of these file names to the <b>From :</b> field below.  Enter a replacement "
		  "string in the <b>To :</b> field then press the apply button to make the change.  Note that the "
		  "changes are not made to the PFM control file until the <b>Accept</b> button is pressed.  You may "
		  "discard all changes by pressing the <b>Discard</b> button.");

QString fromText = 
  changePath::tr ("Enter a string that is part of the above file names that is to be changed to the string that is "
		  "entered in the <b>To :</b> field.");

QString toText = 
  changePath::tr ("Enter a string that will replace in the above file names the string that is entered in the "
		  "<b>From :</b> field.");

QString dirBrowseText = 
  changePath::tr ("This button will allow you to select a directory that contains the moved input files.  Once the "
		  "directory is chosen it will be placed in the <b>To :</b> field so that the <b>Apply</b> button "
		  "will replace the string in the <b>To :</b> field with the fully qualified directory path.");

QString applyIFPText = 
  changePath::tr ("This button applies to the above file names the change indicated in the <b>From :</b> and <b>To :</b> "
		  "fields.");

QString invertIFPText = 
  changePath::tr ("If there is a valid .pfm_cfg file in the PATH and it has [SUBSTITUTE PATH] variables defined this "
		  "button will switch between the Windows and UNIX version of the file names.<br><br>"
		  "<b>IMPORTANT NOTE: The file names in the .ctl file must match up with the [SUBSTITUTE PATH] names "
		  "in the .pfm_cfg file before anything will happen.</b><br><br>"
		  "Thanx and a tip of the hat to Heath Harwood for the UNIX/Windows path substitution idea.");

QString acceptIFPText = 
  changePath::tr ("This button causes all changes made in the above file names to be made to the PFM structure's "
		  "control (.ctl) file.  Prior to modifying the control file all of the file names will be checked to "
		  "make sure that they exist.  This button will also close the Change Input File Paths dialog if all of "
		  "the files exist.");

QString closeIFPText = 
  changePath::tr ("This button discards all changes made to the above file names and closes the Change Input File Paths "
		  "dialog.");
