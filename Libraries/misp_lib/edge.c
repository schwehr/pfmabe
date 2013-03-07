/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        edge                                                *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Compute a weighted value based on nine adjacent     *
*                       points for the first and last (edge) columns of the *
*                       working grid except for corner points.              *
*                                                                           *
*   Inputs:             *memblock1      -   pointer to regional data        *
*                       *mergeblock     -   pointer to original sparse data *
*                       *x              -   pointer to output data          *
*                       y_start         -   starting row                    *
*                       y_end           -   ending row                      *
*                       rownum1         -   row position in memblock1       *
*                       weight_factor   -   weighting factor, final to      *
*                                           regional                        *
*                       final_height    -   height of subarea               *
*                       final_width     -   width of subarea                *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   merge_grid                                          *
*                                                                           * 
*   Glossary:           average     -   average value                       *
*                       good_count  -   number of non_null points of the 9  *
*                                       adjacent points                     *
*                       increment   -   directional increment for the x     *
*                                       loop                                *
*                       rownum2     -   row index into memblock1            *
*                       index1      -   utility integer                     *
*                       index2      -   utility integer                     *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void edge (NV_FLOAT32 *memblock1, NV_FLOAT32 *mergeblock, NV_FLOAT32 *x, 
    NV_INT32 y_start, NV_INT32 y_end, NV_INT32 rownum1, NV_INT32 weight_factor,
    NV_INT32 final_height, NV_INT32 final_width)
{
    NV_FLOAT32   average;
    NV_INT32     good_count, increment, rownum2, index1, index2;
    
    good_count = 0;
    increment = 1;
    if (y_start > y_end) increment = -1;

    /*  Go throught the nine points looking for non-null data.          */
    
    for (index1 = 0; index1 < 3; index1++)
    {
        for (index2 = y_start; index2 != (y_end + increment);
            index2 += increment)
        {
            if (*(mergeblock + (index1 * final_width) + index2) < MISPNULL)
                good_count++;
        }
    }

    /*  If there are no non-null points pass back the unweighted        */
    /*  value.                                                          */
    
    if (good_count <= 0)
    {
        *(x + y_start) = *(memblock1 + (y_start * final_height) + rownum1);
    }

    /*  Otherwise, calculate the weighted value.                        */
    
    else
    {
        average = 0.0;
        for (index1 = 0; index1 < 3; index1++)
        {
            rownum2 = rownum1 - 1 + index1;

            /*  Loop through the points and get the sum of the good     */
            /*  points.                                                 */
            
            for (index2 = y_start; index2 != (y_end + increment);
                index2 += increment)
            {
                if (*(mergeblock + (index1 * final_width) + index2) < MISPNULL)
                {
                    average += (*(mergeblock + (index1 * final_width) + index2) -
                        *(memblock1 + (index2 * final_height) + rownum2));
                }
            }
        }

        /*  Compute the average.                                        */
        
        average = average / (NV_FLOAT32) good_count;
        if ((*(mergeblock + final_width + y_start) <= MISPNULL) || 
            (good_count != 1))
        {
            good_count *= abs (weight_factor);
            if (good_count > 9) good_count = 9;

            if (*(mergeblock + final_width + y_start) < MISPNULL)
            {
                average = *(mergeblock + final_width + y_start) -
                    *(memblock1 + (y_start * final_height) + rownum1);
            }
        }

        /*  Compute the weighted value.                                 */

        if ((*(mergeblock + final_width + y_start) < MISPNULL) &&
            (weight_factor < 0))
        {
            *(x + y_start) = *(mergeblock + final_width + y_start);
        }
        else
        {
            *(x + y_start) = *(memblock1 + (y_start * final_height) + rownum1) +
                ((NV_FLOAT32) good_count / 10.0) * average;
        }
    }
    return;
}
