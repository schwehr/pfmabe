/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        shift                                               *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Shift averaged input values to grid intersections   *
*                       using the computed slope of the regional field at   *
*                       the averaged input point position.                  *
*                                                                           *
*   Inputs:             memblock1           -   memory block 1              *
*                       mergeblock          -   pointer to mergeblock       *
*                                               memory area                 *
*                       memblock4           -   memory block 4              *
*                       memblock5           -   memory block 5              *
*                       final_height        -   height of final subarea     *
*                       final_width         -   width of final subarea      *
*                       posvalue1           -   starting y                  *
*                       posvalue2           -   ending y                    *
*                       posvalue3           -   starting x                  *
*                       posvalue4           -   ending x                    *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   merge_grid                                          *
*                                                                           * 
*   Glossary:           position1           -   utility integer             *
*                       position2           -   utility integer             *
*                       position3           -   utility integer             *
*                       index               -   utility integer             *
*                       x_pos               -   x index into the data       *
*                       y_pos               -   y index into the data       *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void shift(NV_FLOAT32 *memblock1, NV_FLOAT32 *mergeblock, 
    NV_FLOAT32 *memblock4, NV_FLOAT32 *memblock5, NV_INT32 final_height, 
    NV_INT32 final_width, NV_INT32 posvalue1, NV_INT32 posvalue2, 
    NV_INT32 posvalue3, NV_INT32 posvalue4)
{
    NV_FLOAT32   position1, position2, position;
    NV_INT32     index, x_pos, y_pos;

    for (index = posvalue3; index < posvalue4; index++)
    {
        x_pos = index + *(memblock4 + index);
        y_pos = posvalue2 + *(memblock5 + index);

        position1 = *(memblock1 + ((x_pos + 1) * final_height) + y_pos) -
            *(memblock1 + (x_pos * final_height) + y_pos);
        position2 =
            *(memblock1 + ((x_pos + 1) * final_height) + (y_pos + 1))
            - *(memblock1 + (x_pos * final_height) + (y_pos + 1));

        position1 = position1 * *(memblock4 + index) + *(memblock1 + 
            (x_pos * final_height) + y_pos);
        position2 = position2 * *(memblock4 + index) + *(memblock1 + (x_pos * 
            final_height) + (y_pos + 1));

        position = (position2 - position1) * *(memblock5 + index) +
            position1 - *(memblock1 + (x_pos * final_height) + y_pos);

        *(mergeblock + (posvalue1 * final_width) + index) -= position;
    }
    return;
}
