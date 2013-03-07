
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



QString minValueText = 
  lockValue::tr ("Set the minimum value at which you want to lock the scale.");

QString maxValueText = 
  lockValue::tr ("Set the maximum value at which you want to lock the scale.");

QString minSpinText = 
  lockValue::tr ("Set the Hue, Saturation, and Value (HSV) value to be used for coloring minimum data values.  The point colors "
                 "will fade from this value for minimum data values to the maximum HSV value for maximum data value.  This value "
                 "is defined on the standard HSV color wheel.  The default value is 0 (red).  The color wheel goes from 0 to 360 "
                 "but these values are limited to 0 (red) to 315 (magenta).  For reference, red is 0, green is 120, blue is 240, "
                 "and magenta is 315.<br><br>"
                 "<b>IMPORTANT NOTE: If you change the values for color-by-depth they will be passed to, and used by, pfmView.</b>");

QString maxSpinText = 
  lockValue::tr ("Set the Hue, Saturation, and Value (HSV) value to be used for coloring maximum data values.  The point colors "
                 "will fade from this value for maximum data values to the maximum HSV value for maximum data value.  This value "
                 "is defined on the standard HSV color wheel.  The default value is 0 (red).  The color wheel goes from 0 to 360 "
                 "but these values are limited to 0 (red) to 315 (magenta).  For reference, red is 0, green is 120, blue is 240, "
                 "and magenta is 315.<br><br>"
                 "<b>IMPORTANT NOTE: If you change the values for color-by-depth they will be passed to, and used by, pfmView.</b>");

QString lockText = 
  lockValue::tr ("Press this button to lock the scale value to the value selected in the value spin box.  After pressing this button "
                 "the dialog will be closed and the map will be redrawn.");

QString unlockText = 
  lockValue::tr ("Press this button to unlock the scale value.  Note that the value in the value spin box is disregarded when you "
                 "unlock the scale.  After pressing this button the dialog will be closed and the map will be redrawn.");
