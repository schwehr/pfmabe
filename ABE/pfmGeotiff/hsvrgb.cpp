
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

#define FACTOR 255.0

void hsvrgb (float hue, float saturation, float value, QColor *rgb)
{

    float           r = 0.0, g = 0.0, b = 0.0, f, p, q, t;
    int             i;

    if (!saturation)
      {
        rgb->setRed ((unsigned short) (value * FACTOR));
        rgb->setGreen ((unsigned short) (value * FACTOR));
        rgb->setBlue ((unsigned short) (value * FACTOR));
    }
    else
    {
        if (hue == 360.) hue = 0.;
        hue = hue / 60.;
        i = (int) hue;
        f = hue - (float) i;
        p = value * (1 - saturation);
        q = value * (1 - (saturation * f));
        t = value * (1 - (saturation * (1 - f)));

        switch (i) 
        {
            case 0:
                r = value;
                g = t;
                b = p;
                break;

            case 1:
                r = q;
                g = value;
                b = p;
                break;

            case 2:
                r = p;
                g = value;
                b = t;
                break;

            case 3:
                r = p;
                g = q;
                b = value;
                break;

            case 4:
                r = t;
                g = p;
                b = value;
                break;

            case 5:
                r = value;
                g = p;
                b = q;
                break;

            default:
                break;
        }

        rgb->setRed ((unsigned short) (r * FACTOR));
        rgb->setGreen ((unsigned short) (g * FACTOR));
        rgb->setBlue ((unsigned short) (b *FACTOR));
    }
}
