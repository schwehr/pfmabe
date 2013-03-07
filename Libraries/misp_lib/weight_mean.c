/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        weight_mean                                         *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Computes value at x_pos, y_pos as weighted mean of  *
*                       data inside radius search_radius.                   *
*                                                                           *
*   Inputs:             x_pos               -   x position                  *
*                       y_pos               -   y position                  *
*                       search_radius       -   Maximum distance from a grid*
*                                               point that data points are  *
*                                               to be used for regional     *
*                                               computation.  This is used  *
*                                               in the weight_mean function.*
*                                               (default = 20.0)            *
*                       search_radius2      -   search_radius squared       *
*                       squares             -                               *
*                       x_max_dist          -   max x distance for          *
*                                               weight_mean                 *
*                       y_max_dist          -   max y distance for          *
*                                               weight_mean                 *
*                       reg_x_min           -   min x value for regional    *
*                       reg_y_min           -   min y value for regional    *
*                       x_max_num           -   loop upper x bound for      *
*                                               sortdata                    *
*                       y_max_num           -   loop upper y bound for      *
*                                               sortdata                    *
*                       num_points          -   number of points used for   *
*                                               the regional                *
*                       error               -   error flag                  *
*                       convergence         -   convergence value for       *
*                                               weight_mean                 *
*                       x                   -   pointer to x memory area    *
*                       y                   -   pointer to y memory area    *
*                       z                   -   pointer to z memory area    *
*                       sorted_pos          -   pointer to sorted_pos       *
*                                               memory area                 *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   min_curve                                           *
*                                                                           * 
*   Glossary:           sum_weight_z        -   sum of the z values times   *
*                                               the weight                  *
*                       sum_weight          -   sum of the weights          *
*                       x_pos2              -   x difference between the    *
*                                               current point and x_pos     *
*                       y_pos2              -   y difference between the    *
*                                               current point and y_pos     *
*                       sum_of_squares      -   sum of the squares of       *
*                                               x_pos2 and y_pos2           *
*                       distance            -   square root of              *
*                                               sum_of_squares              *
*                       weight              -   square of the difference    *
*                                               between distance and        *
*                                               search_radius divided by    *
*                                               sum_of_squares              *
*                       numpoints           -   number of points within the *
*                                               search radius               *
*                       xstart              -   start of x loop             *
*                       ystart              -   y offset for first          *
*                       xend                -   end of x loop               *
*                       yend                -   y offset for last           *
*                       last                -   end of y loop               *
*                       first               -   start of y loop             *
*                       index1              -   utility integer             *
*                       index2              -   utility integer             *
*                                                                           *
*   Method:             This routine is based on the subroutine 'hite' in   *
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
#include <math.h>
#include "misp.h"

NV_FLOAT32 weight_mean (NV_FLOAT64 x_pos, NV_FLOAT64 y_pos, 
    NV_FLOAT32 search_radius, NV_FLOAT32 search_radius2, NV_INT32 squares, 
    NV_FLOAT32 x_max_dist, NV_FLOAT32 y_max_dist, NV_FLOAT64 reg_x_min, 
    NV_FLOAT64 reg_y_min, NV_INT32 x_max_num, NV_INT32 y_max_num, 
    NV_INT32 num_points, NV_INT32 *error, NV_FLOAT32 convergence, 
    NV_FLOAT32 *x, NV_FLOAT32 *y, NV_FLOAT32 *z, NV_FLOAT32 *sorted_pos)
{
    NV_FLOAT32   sum_weight_z, sum_weight, x_pos2, y_pos2, sum_of_squares, 
                 distance, weight;
            
    NV_INT32     numpoints, xstart, ystart, xend, yend, last, first, index1, 
                 index2;

    sum_weight_z = 0.0;
    sum_weight = 0.0;
    numpoints = 0;


    /*  Compute start and end of loops.                                 */
    
    xstart = (NV_INT32) ((x_pos - search_radius - reg_x_min) / x_max_dist);
    ystart = (NV_INT32) ((y_pos - search_radius - reg_y_min) / y_max_dist);
    xend = xstart + squares;
    yend = ystart + squares;
    if (xstart < 0) xstart = 0;
    if (xstart >= x_max_num) xstart = x_max_num - 1;
    if (ystart < 0) ystart = 0;
    if (ystart >= y_max_num) ystart = y_max_num - 1;
    if (xend < 0) xend = 0;
    if (xend >= x_max_num) xend = x_max_num - 1;
    if (yend < 0) yend = 0;
    if (yend >= y_max_num) yend = y_max_num - 1;

    for (index1 = xstart; index1 <= xend; index1++)
    {
        last = *(sorted_pos + (index1 * y_max_num) + (yend + 1)) - 1;
        if (yend >= (y_max_num - 1)) last = *(sorted_pos + ((index1 + 1) *
            y_max_num) + 0) - 1;
        if ((yend >= (y_max_num - 1)) && (index1 >= (x_max_num - 1)))
            last = num_points - 1;
        first = *(sorted_pos + (index1 * y_max_num) + ystart);
        if (first <= last)
        {
            for (index2 = first; index2 <= last; index2++)
            {
                x_pos2 = *(x + index2) - x_pos;
                y_pos2 = *(y + index2) - y_pos;
                sum_of_squares = (x_pos2 * x_pos2) + (y_pos2 * y_pos2);
                if (sum_of_squares < search_radius2)
                {
                    if (sum_of_squares < convergence) return (*(z + index2));

                    distance = sqrt ((NV_FLOAT64) sum_of_squares);
                    weight = (search_radius - distance) * (search_radius - 
                        distance) / sum_of_squares;
                    sum_weight_z += weight * *(z + index2);
                    sum_weight  += weight;

                    numpoints++;
                }
            }
        }
    }
    if (numpoints > 0) return ((NV_FLOAT32) (sum_weight_z / sum_weight));

    *error = 1;
    return (0.0);
}
