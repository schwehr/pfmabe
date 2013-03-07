
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



#include "pfmView.hpp"

//!  Used to define the color scale boxes on the left of the main window.

void setScale (NV_FLOAT32 min_z, NV_FLOAT32 max_z, NV_FLOAT32 range, NV_INT32 attribute, MISC *misc, OPTIONS *options, NV_BOOL min_lock, NV_BOOL max_lock)
{
  //  Set the scale colors for the current range.

  NV_INT32 numhues = NUMHUES - 1;
  NV_FLOAT32 inc = 1.0;

  inc = range / (NV_FLOAT32) (NUM_SCALE_LEVELS - 1);


  for (NV_INT32 i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      NV_FLOAT32 value = min_z + (NV_FLOAT32) i * inc;
      NV_INT32 h_index = 0;


      //  Set the HSV fill color.

      if (attribute)
        {
          h_index = (NV_INT32) (numhues - fabs ((value - max_z) / range * numhues));

          if (options->stoplight)
            {
              if (value > options->stoplight_max_mid)
                {
                  h_index = options->stoplight_max_index;
                }
              else if (value < options->stoplight_min_mid)
                {
                  h_index = options->stoplight_min_index;
                }
              else
                {
                  h_index = options->stoplight_mid_index;
                }
            }
          else
            {
              h_index = (NV_INT32) (numhues - fabs ((value - misc->color_max) / misc->color_range * numhues));
            }


          //  Check for out of range data.

          if (value < misc->color_min)
            {
              h_index = 0;
            }
          else if (value > misc->color_max)
            {
              h_index = numhues - 1;
            }
        }
      else
        {
          if (options->stoplight)
            {
              if (value * options->z_factor + options->z_offset < options->stoplight_min_mid)
                {
                  h_index = options->stoplight_max_index;
                }
              else if (value * options->z_factor + options->z_offset > options->stoplight_max_mid)
                {
                  h_index = options->stoplight_min_index;
                }
              else
                {
                  h_index = options->stoplight_mid_index;
                }
            }
          else
            {
              if (options->zero_turnover && min_z < 0.0 && max_z >= 0.0)
                {
                  if (value <= 0.0)
                    {
                      h_index = (NV_INT32) (numhues - fabs ((value - misc->color_min) / (-misc->color_min) * numhues));
                    }
                  else
                    {
                      h_index = (NV_INT32) (numhues - fabs (value / misc->color_max * numhues));
                    }
                }
              else
                {
                  h_index = (NV_INT32) (numhues - fabs ((value - misc->color_min) / misc->color_range * numhues));
                }


              //  Check for out of range data.

              if (value < misc->color_min)
                {
                  h_index = numhues - 1;
                }
              else if (value > misc->color_max)
                {
                  h_index = 0;
                }
            }
        }


      //  If we've locked the first or last scaleBox, set the flag to draw the lock image.

      NV_INT32 lock = 0;

      if (!i)
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

      if (i == NUM_SCALE_LEVELS - 1)
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


      if (attribute)
        {
          misc->scale[(NUM_SCALE_LEVELS - 1) - i]->setContents (options->color_array[0][h_index][NUMSHADES - 1], value, lock);
        }
      else
        {
          misc->scale[i]->setContents (options->color_array[0][h_index][NUMSHADES - 1], value, lock);
        }
    }
}
