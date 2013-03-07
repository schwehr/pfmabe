
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



QString invText = 
  startPage::tr ("If this box is checked invalid data will be output along with valid data.  If an invalid point is output "
                 "to the LAS file its <b>User Data</b> field will be set to <b>I</b> for invalid.");

QString geoidText = 
  startPage::tr ("If this box is checked then we will attempt to correct the ellipsoidal height to an orthometric height "
                 "using the National Geodetic Survey (NGS) Geoid03 files.  The files must be stored in a directory that "
                 "is pointed to by the environment variable WVS_DIR (this is also where we store our coastline files).  The "
                 "files must be the binary versions (not ASCII) and the file names must be the following:<br><br>"
                 "<ul>"
                 "<li>g2003a01.bin</li>"
                 "<li>g2003a02.bin</li>"
                 "<li>g2003a03.bin</li>"
                 "<li>g2003a04.bin</li>"
                 "<li>g2003h01.bin</li>"
                 "<li>g2003p01.bin</li>"
                 "<li>g2003u01.bin</li>"
                 "<li>g2003u02.bin</li>"
                 "<li>g2003u03.bin</li>"
                 "<li>g2003u04.bin</li>"
                 "<li>g2003u05.bin</li>"
                 "<li>g2003u06.bin</li>"
                 "<li>g2003u07b.bin</li>"
                 "<li>g2003u08.bin</li>"
                 "</ul><br>"
                 "<b>IMPORTANT NOTE: This option only works with data collected in KGPS mode.</b>");

QString hDatumText = 
  startPage::tr ("Select the horizontal datum that was used for the data sets.  Either WGS 84 or NAD 83.");
