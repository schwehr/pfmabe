#include "geoSwath3D.hpp"

void setScale (NV_FLOAT32 actual_min, NV_FLOAT32 actual_max, MISC *misc)
{
  //  Set the scale colors for the current range.

  NV_FLOAT32 numshades = (NV_FLOAT32) (NUMSHADES - 1);
  NV_FLOAT32 inc = 1.0, min_z, max_z, range;
  NV_FLOAT32 value = 0.0;


  min_z = actual_min;
  max_z = actual_max;


  range = max_z - min_z;

  if (fabs (range) < 0.0000001) range = 1.0;

  inc = range / (NV_FLOAT32) (NUM_SCALE_LEVELS - 1);


  for (NV_INT32 i = 0 ; i < NUM_SCALE_LEVELS ; i++)
    {
      value = min_z + (NV_FLOAT32) i * inc;

      NV_INT32 c_index = 0;


      //  Color by depth

      c_index = NINT ((max_z - value) / range * numshades);


      QColor clr;
      clr.setRed (misc->color_array[0][c_index][0]);
      clr.setGreen (misc->color_array[0][c_index][1]);
      clr.setBlue (misc->color_array[0][c_index][2]);
      clr.setAlpha (misc->color_array[0][c_index][3]);

      misc->scale[(NUM_SCALE_LEVELS - 1) - i]->setContents (clr, -value, 0);
    }
}
