
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



#include "pfmGeotiffDef.hpp"

void palshd (int num_shades, int num_hues, float start_hue, float end_hue, 
             float min_saturation, float max_saturation, float min_value, float max_value,
             int start_color __attribute__ ((unused)), QColor color_array[])
{
  int                 half_length, color, i;
  float               saturation_increment, value_increment, hue_increment;
  float               hue, saturation, value, s;


  void hsvrgb (float, float, float, QColor *);



  half_length = num_shades / 2;
  saturation_increment = (max_saturation - min_saturation) / half_length;
  value_increment = (max_value - min_value) / num_shades;

  if (num_hues != 1)
    {
      hue_increment = (end_hue - start_hue) / ((float) num_hues - 1.0);
    }
  else
    {
      hue_increment = 0.0;
    }

  color = 0;

  for (i = 0 ; i <= num_hues ; i++)
    {
      hue = start_hue + (i * hue_increment);
      if (hue > 360.) hue -= 360.;
      if (hue < 0.) hue += 360.;
      saturation = min_saturation;
      value = max_value;

      if (i == num_hues)
        {
          hue = 0.0;
          saturation = 0.0;
        }

      for (s = 0.0 ; s < (float) num_shades ; s++)
        {
          hsvrgb (hue, saturation, value, &color_array[color]);


          color++;


          if (s < half_length) saturation += saturation_increment;

          value -= value_increment;
        }
    }
}
