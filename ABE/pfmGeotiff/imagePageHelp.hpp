
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



QString restartText = 
  imagePage::tr ("Selecting this option will cause the colormap to go from minimum to maximum (depending on direction) "
                 "when crossing the zero boundary.  If not selected then the colormap will be continuous from the "
                 "minimum to the maximum value.  Click the button to see the effect in the sample data to the right.  "
                 "Note that if you are using one of the gray scales this has no effect.");

QString sunAzText = 
  imagePage::tr ("This is the sun azimuth.  The range is 0.0 to 360.0.");

QString sunElText = 
  imagePage::tr ("This is the sun elevation.  The range is from 0.0 (on the horizon) to 90.0 (directly overhead).");

QString sunExText = 
  imagePage::tr ("This is the vertical exaggeration.  This is used to give a better view of terrain features.  As an "
                 "example, the default vertical exaggeration in <b>IVS' Fledermaus</b> is 6.0.");

QString saturationText = 
  imagePage::tr ("This is the color saturation.  The range is 0.0 to 1.0.  Basically, 0.0 is grayscale and 1.0 is full "
                 "color.");

QString valueText = 
  imagePage::tr ("This is the color value.  The range is 0.0 (white) to 1.0 (full color).");

QString start_hueText = 
  imagePage::tr ("This is the starting color for the range of data that you are displaying.  The values are from the "
                 "standard HSV color wheel and range from 0.0 to 360.0.  0.0 is red, 120.0 is green, and 240.0 is blue.");

QString end_hueText = 
  imagePage::tr ("This is the ending color for the range of data that you are displaying.  The values are from the "
                 "standard HSV color wheel and range from 0.0 to 360.0.  0.0 is red, 120.0 is green, and 240.0 is blue.");

QString sampleText = 
  imagePage::tr ("These are some sample color settings.  Selecting one of the buttons will set the colors in the fields "
                 "to the left and redraw the sample to the right.");

QString sample0Text = 
  imagePage::tr ("Sets the color values to produce a light grayscale image.");

QString sample1Text = 
  imagePage::tr ("Sets the color values to produce a grayscale image.");

QString sample2Text = 
  imagePage::tr ("Sets the color values to produce a full color red to blue image.");

QString sample3Text = 
  imagePage::tr ("Sets the color values to produce a light full color red to blue image.");

QString sample4Text = 
  imagePage::tr ("Sets the color values to produce a full color red to magenta image.");

QString sample5Text = 
  imagePage::tr ("Sets the color values to produce a full color magenta to green image.");
