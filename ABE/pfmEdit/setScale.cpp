
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



#include "pfmEdit.hpp"


//!  Used to define the color scale boxes on the left of the main window.

void setScale (NV_FLOAT32 actual_min, NV_FLOAT32 actual_max, NV_FLOAT32 attr_min, NV_FLOAT32 attr_max, MISC *misc, OPTIONS *options, NV_BOOL min_lock,
               NV_BOOL max_lock)
{
  //  Set the scale colors for the current range.

  NV_FLOAT32 numshades = (NV_FLOAT32) (NUMSHADES - 1);
  NV_FLOAT32 inc = 1.0, min_z, max_z, range;
  NV_FLOAT32 value = 0.0;


  min_z = actual_min;
  max_z = actual_max;


  //  Color by uncertainty/attribute

  if (options->color_index == 2 || options->color_index == 3 || (options->color_index >= PRE_ATTR && options->color_index < PRE_ATTR + 10))
    {
      min_z = attr_min;
      max_z = attr_max;
    }

  range = max_z - min_z;

  if (fabs (range) < 0.0000001) range = 1.0;

  inc = range / (NV_FLOAT32) (NUM_SCALE_LEVELS - 1);


  for (NV_INT32 i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      value = min_z + (NV_FLOAT32) i * inc;

      NV_INT32 c_index = 0;


      //  Color by depth

      if (!options->color_index)
        {
          c_index = NINT ((value - misc->color_min_z) / misc->color_range_z * numshades);
        }


      //  Color by uncertainty or attribute

      else if (options->color_index == 2 || options->color_index == 3 || (options->color_index >= PRE_ATTR && options->color_index < PRE_ATTR + 10))
        {
          c_index = (NUMSHADES - 1) - NINT ((value - misc->attr_color_min) / misc->attr_color_range * numshades);
        }


      //  Check for out of range values (this is the easy way to work the locked color scale.

      if (c_index < 0) c_index = 0;
      if (c_index > NUMSHADES - 1) c_index = NUMSHADES - 1;


      //  Blank out the scale if we're coloring by line.

      if (options->color_index == 1)
        {
          misc->scale[i]->setContents (Qt::white, 99999.99);
        }
      else
        {
          //  If we've locked the first or last scaleBox, set the flag to draw the lock image.

          NV_INT32 lock = 0;

          if (!i)
            {
              if (!options->color_index)
                {
                  if (min_lock)
                    {
                      lock = 1;
                    }
                  else
                    {
                      lock = -1;
                    }
                }
              else
                {
                  if (max_lock)
                    {
                      lock = 1;
                    }
                  else
                    {
                      lock = -1;
                    }
                }
            }

          if (i == NUM_SCALE_LEVELS - 1)
            {
              if (!options->color_index)
                {
                  if (max_lock)
                    {
                      lock = 1;
                    }
                  else
                    {
                      lock = -1;
                    }
                }
              else
                {
                  if (min_lock)
                    {
                      lock = 1;
                    }
                  else
                    {
                      lock = -1;
                    }
                }
            }

          if (!options->color_index)
            {
              misc->scale[i]->setContents (misc->color_array[0][c_index], value, lock);
            }
          else
            {
              misc->scale[(NUM_SCALE_LEVELS - 1) - i]->setContents (misc->color_array[0][c_index], value, lock);
            }
        }
    }
}
