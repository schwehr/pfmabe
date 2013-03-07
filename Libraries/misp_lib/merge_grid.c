/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        merge_grid                                          *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            This function merges the regional grid at the final *
*                       grid spacing with the original sparse input data.   *
*                                                                           *
*   Inputs:             memblock_width      -   Number of grid columns in   *
*                                               memory for loadsparse and   *
*                                               readsparse.                 *
*                       memblock_height     -   Number of grid rows in      *
*                                               memory to be used in        *
*                                               loadsparse and readsparse.  *
*                       weight_factor       -   Weight factor for merging   *
*                                               the regional field with the *
*                                               original sparse input data. *
*                                               The weight factor may be    *
*                                               set to 1,2, or 3.  The      *
*                                               larger the value, the more  *
*                                               weight that is put on the   *
*                                               input data. (default = 2).  *
*                       memblock1           -   Memory block 1.             *
*                       memblock2           -   Memory block 2.             *
*                       memblock3           -   Memory block 3.             *
*                       memblock4           -   Memory block 4.             *
*                       memblock5           -   Memory block 5.             *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   main                                                *
*                                                                           * 
*   Glossary:           temp            -   pointer to temp memory area     *
*                       y               -   pointer to y memory area        *
*                       x               -   pointer to x memory area        *
*                       mergeblock      -   pointer to mergeblock memory    *
*                                           area                            *
*                       reg_columns     -   regional columns                *
*                       reg_rows        -   regional rows                   *
*                       reg_grid        -   total grid cells in regional    *
*                       max_length      -   larger of the two axes          *
*                       temp_size       -   size of temp memory area        *
*                       x_size          -   size of x memory area           *
*                       y_size          -   size of y memory area           *
*                       index1          -   utility integer                 *
*                       index2          -   utility integer                 *
*                       index3          -   utility integer                 *
*                       width_1         -   width - 1                       *
*                       width_2         -   width - 2                       *
*                       width_3         -   width - 3                       *
*                       height_1        -   height - 1                      *
*                       height_2        -   height - 2                      *
*                       height_3        -   height - 3                      *
*                       endrow          -   last row for main merge loop    *
*                       mergeblock_size -   size of the mergeblock memory   *
*                                           area                            *
*                                                                           *
*   Method:             Shifts the original value to the grid crossing      *
*                       based on the average position of the original point *
*                       (passed from readsparse in memory or a temp file)   *
*                       and the slope from the regional grid.  The weighted *
*                       value is computed from nine adjacent points to each *
*                       data point.                                         *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void merge_grid (NV_INT32 memblock_width, NV_INT32 memblock_height, 
    NV_INT32 weight_factor, NV_FLOAT32 *memblock1, NV_FLOAT32 *memblock2, 
    NV_FLOAT32 *memblock3, NV_FLOAT32 *memblock4, NV_FLOAT32 *memblock5, 
    MISP_HEADER *final)
{
    NV_FLOAT32   *temp, *y, *x, *mergeblock;
                   
    NV_INT32     reg_columns, reg_rows, reg_grid, max_length,
                 temp_size, x_size, y_size, index1,
                 index2, index3, width_1, width_2, width_3, height_1, height_2,
                 height_3, endrow, mergeblock_size;

    NV_BOOL      prog_reg = NVFalse;
    NV_CHAR      string[128];


    void corner (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_INT32,
        NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32);
    void edge (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_INT32, 
        NV_INT32, NV_INT32, NV_INT32, NV_INT32);
    void merge (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_INT32, 
        NV_INT32, NV_INT32, NV_INT32);
    void shift (NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, 
        NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32);
    void bit_set (NV_FLOAT32 *, NV_INT32, NV_INT32);



    prog_reg = misp_progress_callback_registered ();


    if (prog_reg)
      {
        misp_progress ("Merging regional and final grids");
      }
    else
      {
        printf ("\n\nMerging regional and final grids\n\n");
      }


    reg_columns = memblock_width / MULT + 1;
    reg_rows = memblock_height / MULT + 1;
    reg_grid = (reg_columns + 1) * (reg_rows + 1);
    
    if (memblock_width > memblock_height)
    {
        max_length = memblock_width;
    }
    else
    {
        max_length = memblock_height;
    }

    /*  Compute the offset for all the pointers.                        */

    temp_size = max_length;
    x_size   = reg_grid;
    y_size   = reg_grid;
    mergeblock_size = max_length * (MULT + 1);

    /*  Calculate the address for all the pointers.                     */

    temp  = memblock2;
    x = memblock2 + temp_size;
    y = memblock2 + temp_size + x_size;
    mergeblock = memblock2 + temp_size + x_size + y_size;


    /*  Move the first three rows of the original sparse grid from the  */
    /*  memblock3, memblock4, memblock5 array.                          */
   
    for (index1 = 0; index1 < 3; index1++)
    {
        for (index2 = 0; index2 < final->width; index2++)
        {
            index3 = index1 * memblock_width/*final->width*/ + index2;
            *(mergeblock + (index1 * final->width) + index2) = *(memblock3 + 
                index3);
            *(memblock4 + index2) = *(memblock4 + index3);
            *(memblock5 + index2) = *(memblock5 + index3);
        }

        /*  Shift the values based on the average position.             */
        
        shift (memblock1, mergeblock, memblock4, memblock5, final->height, 
            final->width, index1, index1, 1, final->width - 1);

    }
    if (prog_reg) misp_progress ("");


    /*  Merge the corners and all data across the first row.            */

    corner (memblock1, mergeblock, x, 0, 0, 2, 0, final->height, final->width,
        weight_factor);

    width_1 = final->width - 1;
    width_2 = final->width - 2;
    width_3 = final->width - 3;
    
    corner (memblock1, mergeblock, x, 0, width_1, width_3, 0, final->height,
        final->width, weight_factor);
    if (prog_reg) misp_progress ("");
    merge (memblock1, mergeblock, x, final->width, final->height, 0, 0, 
        weight_factor);
    if (prog_reg) misp_progress ("");

    for (index1 = 0; index1 < final->width; index1++)
    {
        
        /*  Set the least significant bit to 1 for real data.           */
  
        if (*(mergeblock + index1) < MISPNULL)
        {
            bit_set (x + index1, 0, 1);
        }
        else
        {
            bit_set (x + index1, 0, 0);
        }
        *(y + index1) = *(x + index1);
    }
    if (prog_reg) misp_progress ("");

    /*  Merge row 1 through final->height - 2.                           */
    
    endrow = final->height - 2;
    for (index1 = 1; index1 <= endrow; index1++)    
    {
        if (prog_reg) misp_progress ("");
        edge (memblock1, mergeblock, x, 0, 2, index1, weight_factor,
            final->height, final->width);
        if (prog_reg) misp_progress ("");
        edge (memblock1, mergeblock, x, width_3, width_1, index1,
            weight_factor, final->height, final->width);
        if (prog_reg) misp_progress ("");
        merge (memblock1, mergeblock, x, final->width, final->height, 1, index1,
            weight_factor);
        if (prog_reg) misp_progress ("");


        /*  x-array contains ith row results, stick y-array into        */
        /*  (i-1)st row of memblock1, stick x into y.                   */

        if (index1 <= (final->height - 3))
        {
            for (index2 = 0; index2 < final->width; index2++)
            {
                *(memblock1 + (index2 * final->height) + index1 - 1) = *(y
                    + index2);
               
                /*  Set the least significant bit to 1 for real data.   */
                
                if (*(mergeblock + (1 * final->width) + index2) < MISPNULL)
                {
                    bit_set (x + index2, 0, 1);
                }
                else
                {
                    bit_set (x + index2, 0, 0);
                }
                *(y + index2) = *(x + index2);
            }
            
            /*  Move the data forward in the triple buffer.             */
            
            for (index2 = 0; index2 < final->width; index2++)
            {
                *(mergeblock + index2) = *(mergeblock + (1 * final->width) + 
                    index2);
                *(mergeblock + (1 * final->width) + index2) = *(mergeblock +
                    (2 * final->width) + index2);
            }
            
            /*  Move a new row of original data into the 3rd row of the */
            /*  triple buffer from the memblock3, memblock4, and        */
            /*  memblock5 buffers.                                      */
        
            for (index2 = 0; index2 < final->width; index2++)
            {
                index3 = (index1 + 2) * memblock_width + index2;
                *(mergeblock + (2 * final->width) + index2) = *(memblock3 +
                    index3);
                *(memblock4 + index2) = *(memblock4 + index3);
                *(memblock5 + index2) = *(memblock5 + index3);
            }
          
            /*  Shift the values based on the average position of the   */
            /*  data.                                                   */
            
            shift (memblock1, mergeblock, memblock4, memblock5, final->height,
                final->width, 2, index1 + 2, 1, final->width - 1);
            if (prog_reg) misp_progress ("");
        }
    }
        
    for (index1 = 0; index1 < final->width; index1++) *(temp + index1) = *(x + index1);
   
    /*  Shift the values based on the average position, then merge the  */
    /*  corners and all data across the last row.                       */
    
    if (prog_reg) misp_progress ("");
    corner (memblock1, mergeblock, x, 2, 0, 2, final->height - 1, final->height, final->width, weight_factor);
    if (prog_reg) misp_progress ("");
    corner (memblock1, mergeblock, x, 2, width_1, width_3, final->height - 1, final->height, final->width, weight_factor);
    if (prog_reg) misp_progress ("");
    merge (memblock1, mergeblock, x, final->width, final->height, 3, final->height - 1, weight_factor);
    if (prog_reg) misp_progress ("");
    height_3 = final->height - 3;
    height_2 = final->height - 2;
    height_1 = final->height - 1;
    
    for (index1 = 0; index1 < final->width; index1++)
    {
        *(memblock1 + (index1 * final->height) + height_3) = *(y + index1);

        /*  Set the least signigicant bit to 1 for real data.           */
     
        if (*(mergeblock + (1 * final->width) + index1) < MISPNULL)
        {
            bit_set (temp + index1, 0, 1);
        }
        else
        {
            bit_set (temp + index1, 0, 0);
        }

        *(memblock1 + (index1 * final->height) + height_2) = *(temp + index1);

        /*  Set the least signigicant bit to 1 for real data.           */
     
        if (*(mergeblock + (2 * final->width) + index1) < MISPNULL)
        {
            bit_set (x + index1, 0, 1);
        }
        else
        {
            bit_set (x + index1, 0, 0);
        }

        *(memblock1 + (index1 * final->height) + height_1) = *(x + index1);
    }
    if (prog_reg) misp_progress ("");


    /*  Output interpolated grid.                                       */

    if (prog_reg)
      {
        misp_progress ("\nFinal grid values covering the final grid area\n");
        sprintf (string, "%d Rows\t%d Columns\n\n\n", final->grid_rows, final->grid_cols);
        misp_progress (string);
        misp_progress ("\nGridding Complete\n");
      }
    else
      {
        printf ("\nFinal grid values covering the final grid area\n");
        printf ("%d Rows\t%d Columns\n\n\n", final->grid_rows, final->grid_cols);


        printf("\nGridding Complete\n");
      }

    return;
}
