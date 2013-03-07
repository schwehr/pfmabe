/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        readsparse                                          *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Recall the sparse data for use in misps.            *
*                                                                           *
*   Inputs:             row             -   row to be recalled              *
*                       column          -   column to be recalled           *
*                       memblock_height -   height of the memory block      *
*                       memblock_width  -   width of the memory block       *
*                       memblock_total  -   total memory block size         *
*                       memblock1       -   storage area                    *
*                       memblock2       -   count data                      *
*                       memblock3       -   z data                          *
*                       memblock4       -   x data                          *
*                       memblock5       -   y data                          *
*                       final_height    -   height of subarea               *
*                       final_width     -   width of subarea                *
*                       row_points      -   number of points recalled       *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   misps                                               *
*                                                                           * 
*   Glossary:           rownum          -   row number in file or memory    *
*                       index           -   utility integer                 *
*                       index2          -   utility integer                 *
*                       index3          -   utility integer                 *
*                       memsize         -   size of one memory block        *
*                       mem_block_size  -   size of all 4 blocks            *
*                                                                           *
*   Method:             This routine computes the bin that the data is      *
*                       stored in and either gets the data from memory of   *
*                       retrieves it from mass storage.                     *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void readsparse (NV_INT32 row, NV_INT32 column, NV_INT32 memblock_height, 
    NV_INT32 memblock_width, NV_INT32 memblock_total, NV_FLOAT32 *memblock1, 
    NV_FLOAT32 *memblock3, NV_FLOAT32 *memblock4, NV_FLOAT32 *memblock5, 
    NV_INT32 final_height, NV_INT32 final_width, NV_INT32 *row_points)
{
    NV_INT32             rownum, index, index1, index2;
    static NV_INT32      memsize, mem_block_size;

    memsize = memblock_total * sizeof (NV_FLOAT32);
    mem_block_size = memsize * 4;

    for (index1 = 0; index1 < final_height; index1++)
    {
        rownum = ((row - 1) + index1) % memblock_height;

        /*  Compute the row number and move the data to memblock1.      */
        
        index = rownum * memblock_width + (column - 1);
        for (index2 = 0; index2 < final_width; index2++)
        {
            if (*(memblock3 + index + index2) < MISPNULL) (*row_points)++;
            *(memblock1 + (index2 * final_height) + index1) = 
                *(memblock3 + index + index2);
        }
    }       

    return;
}
