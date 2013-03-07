
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



QString pfm_fileText = 
  startPage::tr ("Use the browse button to select an input PFM list or handle file.  You cannot modify the text in the "
                  "<b>PFM File</b> text window.  The reason for this is that the file must exist in order for the "
                  "program to run.  Note also that the <b>Next</b> button will not work until you select an input "
                  "PFM file.  When one is selected the default output file name will be supplied in the <b>Output "
                  "GeoTIFF File</b> text window.  The GeoTIFF file name can be edited since it may be a new file or a "
                  "pre-existing file.");

QString pfm_fileBrowseText = 
  startPage::tr ("Use this button to select the input PFM list or handle file");

QString output_fileText = 
  startPage::tr ("You may enter a new file name to be used for the output GeoTIFF file or modify the default file name "
                  "that was placed here when you selected the input PFM file.");

QString output_fileBrowseText = 
  startPage::tr ("Use this button to select a pre-existing GeoTIFF file to be overwritten.");

QString area_fileText = 
  startPage::tr ("You may enter an area file name to be used to limit the area of the PFM file from which to generate "
                  "the GeoTIFF.  The area file may be in ISS-60 format (*.ARE) or generic format (*.are).");

QString area_fileBrowseText = 
  startPage::tr ("Use this button to select an optional area file.");

