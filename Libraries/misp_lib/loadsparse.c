/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        loadsparse                                          *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Load all the data from the sparse grid into an      *
*                       array.                                              *
*                                                                           *
*   Inputs:             memblock2       -   count data                      *
*                       memblock3       -   z data                          *
*                       memblock4       -   x data                          *
*                       memblock5       -   y data                          *
*                       memblock_width  -   width of the memory block       *
*                       memblock_height -   height of the memory block      *
*                       memblock_total  -   total memory block size         *
*                       xvalue          -   x value passed in               *
*                       yvalue          -   y value passed in               *
*                       zvalue          -   z value passed in               *
*                       numblocks       -   number of memory blocks         *
*                       init            -   1 for loading data, 0 to        *
*                                           compute averages                *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   main                                                *
*                                                                           * 
*   Glossary:           first           -   signifies first time through    *
*                                           loadsparse.                     *
*                       memsize         -   size of one memory block        *
*                       mem_block_size  -   size of all 4 blocks            *
*                       int_xvalue      -   integer x value in minutes      *
*                       int_yvalue      -   integer y value in minutes      *
*                       position        -   position in bytes of the        *
*                                           current record                  *
*                       pre_position    -   position in bytes of the        *
*                                           previous record                 *
*                                                                           *
*   Method:             If neccessary this routine bins the input data to a *
*                       temporary file in records that represent bands from *
*                       west to east.  If all of the data is to be stored   *
*                       in memory it is placed in the 4 memory blocks.      *
*                       These blocks contain the count of points, the sum   *
*                       of the x, y, and z values.  After all of the data   *
*                       has been loaded this routine is called one more     *
*                       time to compute the average x, y, and z.            *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void loadsparse (NV_FLOAT32 *memblock2, NV_FLOAT32 *memblock3, NV_FLOAT32 *memblock4, NV_FLOAT32 *memblock5, NV_INT32 memblock_width, 
                 NV_INT32 memblock_height, NV_INT32 memblock_total, NV_FLOAT32 xvalue, NV_FLOAT32 yvalue, NV_FLOAT32 zvalue, NV_INT32 init)
{
  static NV_INT32      first = NVTrue, *count_array;
  NV_INT32             index1, index2, int_xvalue, int_yvalue;


  if (init == 1)
    {
      if (first)
        {
          count_array = (NV_INT32 *) memblock2;

          first = NVFalse;
        }


      /*  Sum multiple occurrences.                                   */

      int_xvalue = (NV_INT32) (xvalue + 0.5);
      int_yvalue = (NV_INT32) (yvalue + 0.5);

      index1 = (int_yvalue % memblock_height) * memblock_width + int_xvalue;
      *(memblock3 + index1) += zvalue;
      *(memblock4 + index1) += (xvalue - int_xvalue);
      *(memblock5 + index1) += (yvalue - int_yvalue);        
      *(count_array + index1) += 1;
    }
  else
    {
      for (index2 = 0; index2 < memblock_total; index2++)
        {
          if (*(count_array + index2) >= 1)
            {
              *(memblock3 + index2) /= ((NV_FLOAT32) (*(count_array + index2)));
              *(memblock4 + index2) /= ((NV_FLOAT32) (*(count_array + index2)));
              *(memblock5 + index2) /= ((NV_FLOAT32) (*(count_array + index2)));
            }
          else
            {
              *(memblock3 + index2) = MISPNULL;
            }
        }


      /*  Reset for subsequent passes.  */

      first = NVTrue;
    }

  return;
}
