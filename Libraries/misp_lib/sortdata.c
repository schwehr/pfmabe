/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        sortdata                                            *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Sorts regional data into rectangles and stores in   *
*                       x, y, and z to allow faster access to data by       *
*                       function weight_mean.                               *
*                                                                           *
*   Inputs:             sorted_pos          -   pointer to sorted_pos       *
*                                               memory area                 *
*                       x                   -   pointer to x memory area    *
*                       y                   -   pointer to y memory area    *
*                       z                   -   pointer to z memory area    *
*                       reg_data            -   pointer to reg_data         *
*                                               memory area                 *
*                       reg_x_max           -   max x value for regional    *
*                       reg_x_min           -   min x value for regional    *
*                       reg_y_max           -   max y value for regional    *
*                       reg_y_min           -   min y value for regional    *
*                       x_max_num           -   loop upper x bound          *
*                       y_max_num           -   loop upper y bound          *
*                       x_max_dist          -   max x distance for          *
*                                               weight_mean                 *
*                       y_max_dist          -   max y distance for          *
*                                               weight_mean                 *
*                       num_points          -   number of points used for   *
*                                               the regional                *
*                       reg_grid            -   total number of grid points *
*                                               in regional                 *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   min_curve                                           *
*                                                                           * 
*   Glossary:           position1           -   utility integer             *
*                       position2           -   utility integer             *
*                       position3           -   utility integer             *
*                       index1              -   utility integer             *
*                       index2              -   utility integer             *
*                       index3              -   utility integer             *
*                       tempval             -   utility float               *
*                                                                           *
*   Method:             This routine is based on the subroutine 'sort' in   *
*                       the article "A FORTRAN IV PROGRAM FOR INTERPOLATING *
*                       IRREGULARLY SPACED DATA USING THE DIFFERENCE        *
*                       EQUATIONS FOR MINIMUM CURVATURE" by C. J. Swain in  *
*                       Computers and Geosciences, Vol. 1 pp. 231-240.      *
*                       Mr. Swain's paper was, in turn, based on the        *
*                       article "MACHINE CONTOURING USING MINIMUM           *
*                       CURVATURE" by Ian C. Briggs in Geophysics, Vol 39,  *
*                       No. 1 (February 1974), pp. 39-48.                   *
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void sortdata (NV_FLOAT32 *sorted_pos, NV_FLOAT32 *x, NV_FLOAT32 *y, 
    NV_FLOAT32 *z, NV_FLOAT32 *reg_data, NV_FLOAT32 reg_x_max, 
    NV_FLOAT64 reg_x_min, NV_FLOAT32 reg_y_max, NV_FLOAT64 reg_y_min, 
    NV_INT32 x_max_num, NV_INT32 y_max_num, NV_FLOAT32 x_max_dist, 
    NV_FLOAT32 y_max_dist, NV_INT32 num_points, NV_INT32 reg_grid)
{
    NV_INT32     position1, position2, position3, index1, index2, index3;
    NV_FLOAT32   tempval;      
      
    position1 = 0;
    position2 = 0;

    for (index1 = 0; index1 < x_max_num; index1++)
    {
        for (index2 = 0; index2 < num_points; index2++)
        {
            if ((((NV_INT32) ((*(reg_data + index2) - reg_x_min) / 
                x_max_dist) == index1)) || ((index1 == 0) && (*(reg_data + 
                index2) <= reg_x_min)) || ((index1 == (x_max_num - 1) &&
                (*(reg_data + index2) >= reg_x_max))))
            {
                *(x + position2) = *(reg_data + (0 * reg_grid) + index2);
                *(y + position2) = *(reg_data + (1 * reg_grid) + index2);
                *(z + position2) = *(reg_data + (2 * reg_grid) + index2);
                position2++;
            }
        }
      
        for (index2 = 0; index2 < y_max_num; index2++)
        {
            *(sorted_pos + (index1 * y_max_num) + index2) = position1;
            position3 = position2 - 1;
            if (position1 <= position3)
            {
                for (index3 = position1; index3 <= position3; index3++)
                {
                    if (((NV_INT32)((*(y + index3) - reg_y_min) / y_max_dist)
                        == index2) || ((index2 == 0) && (*(y + index3) 
                        <= reg_y_min)) || ((index2 == y_max_num) && 
                        (*(y + index3) >= reg_y_max)))
                    {
                        tempval = *(x + index3);
                        *(x + index3) = *(x + position1);
                        *(x + position1) = tempval;
                        tempval = *(y + index3);
                        *(y + index3) = *(y + position1);
                        *(y + position1) = tempval;
                        tempval = *(z + index3);
                        *(z + index3) = *(z + position1);
                        *(z + position1) = tempval;
                        position1++;
                    }
                }
            }
        }
    }
    return;
}
