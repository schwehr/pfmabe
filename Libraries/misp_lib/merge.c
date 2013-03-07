/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        merge                                               *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Compute weighted values based on nine adjacent      *
*                       points for all grid points of the working grid      *
*                       (excluding corner points and the 1st and last       *
*                       column).                                            *
*                                                                           *
*   Inputs:             *memblock1      -   pointer to regional data        *
*                       *mergeblock     -   pointer to original sparse data *
*                       *x              -   pointer to output data          *
*                       final_width     -   width of subarea                *
*                       final_height    -   height of subarea               *
*                       rownum          -   row position in mergeblock      *
*                       rowpos          -   row position in memblock1k      *
*                       weight_factor   -   weighting factor, final to      *
*                                           regional                        *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   merge_grid                                          *
*                                                                           * 
*   Glossary:           average     -   average value                       *
*                       position1   -   upper end of x loop                 *
*                       good_count  -   number of non_null points of the 9  *
*                       position3   -   row index into memblock1            *
*                       valpos1     -   index of previous x                 *
*                       valpos2     -   index of next x                     *
*                       index1      -   utility integer                     *
*                       index2      -   utility integer                     *
*                       index3      -   utility integer                     *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void merge (NV_FLOAT32 *memblock1, NV_FLOAT32 *mergeblock, NV_FLOAT32 *x, 
    NV_INT32 final_width, NV_INT32 final_height, NV_INT32 rownum, 
    NV_INT32 rowpos, NV_INT32 weight_factor)
{
    NV_FLOAT32   average;
    NV_INT32     position1, good_count, position3, valpos1, valpos2, index1, 
                 index2, index3;
   
    position1   = final_width - 2;


    /*  Loop through the data from left to right.                       */

    for (index1 = 1; index1 <= position1; index1++)
    {
        valpos1     = index1 - 1;
        valpos2     = index1 + 1;
        good_count   = 0;

        /*  Go throught the nine points looking for non-null data.      */
    
        for (index2 = 0; index2 < 3; index2++)
        {
            for (index3 = valpos1; index3 <= valpos2; index3++)
            {
                if (*(mergeblock + (index2 * final_width) + index3) < MISPNULL)
                    good_count++;
            }
        }

        /*  If there are no non-null points pass back the unweighted    */
        /*  value.                                                      */
    
        if (good_count <= 0)
        {
            *(x + index1) = *(memblock1 + (index1 * final_height) + rowpos);
        }

        /*  Otherwise, calculate the weighted value.                    */
    
        else
        {
            average = 0.0;
            for (index2 = 0; index2 < 3; index2++)
            {
                position3 = rowpos - 1 + index2;

                if (rowpos == 0) position3++;
                if (rowpos == final_height - 1) position3--;


                /*  Loop through the points and get the sum of the good */
                /*  points.                                             */
            
                for (index3 = valpos1; index3 <= valpos2; index3++)
                {
                    if (*(mergeblock + (index2 * final_width) + index3) <
                        MISPNULL)
                    {
                        average +=
                            *(mergeblock + (index2 * final_width) + index3) -
                            *(memblock1 + (index3 * final_height) + position3);
                    }
                }
            }

            /*  Compute the average.                                    */
        
            average = average / (NV_FLOAT32) good_count;
            if ((*(mergeblock + (rownum * final_width) + index1) <= MISPNULL) || 
                (good_count != 1))
            {
                good_count *= abs (weight_factor);
                if (good_count > 9) good_count = 9;

                if (*(mergeblock + (rownum * final_width) + index1) < MISPNULL)
                {
                    average = *(mergeblock + (rownum * final_width) + index1) - 
                        *(memblock1 + (index1 * final_height) + rowpos);
                }
            }

            /*  Compute the weighted value.                             */

            if ((*(mergeblock + (rownum * final_width) + index1) < MISPNULL) && 
                (weight_factor < 0))
            {
                *(x + index1) = *(mergeblock + (rownum * final_width) + index1);
            }
            else
            {
                *(x + index1) = *(memblock1 + (index1 * final_height) + rowpos) +
                ((NV_FLOAT32) good_count / 10.0) * average;
            }
        }
    }
    return;
}
