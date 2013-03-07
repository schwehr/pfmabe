/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        min_curve                                           *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Minimum curvature routine.                          *
*                                                                           *
*   Inputs:             weighted_mean       -   pointer to weighted_mean    *
*                                               memory area                 *
*                       reg_quadrant        -   pointer to reg_quadrant     *
*                                               memory area                 *
*                       x                   -   pointer to x memory area    *
*                       y                   -   pointer to y memory area    *
*                       z                   -   pointer to z memory area    *
*                       sorted_pos          -   pointer to sorted_pos       *
*                                               memory area                 *
*                       reg_output          -   pointer to reg_output       *
*                                               memory area                 *
*                       reg_width           -   width of regional grid      *
*                       reg_height          -   height of regional grid     *
*                       x_max_num           -   loop upper x bound for      *
*                                               sortdata                    *
*                       y_max_num           -   loop upper y bound for      *
*                                               sortdata                    *
*                       x_increment         -   grid spacing in x           *
*                       y_increment         -   grid spacing in y           *
*                       delta               -   The value of the maximum    *
*                                               change in the surface       *
*                                               between iterations at which *
*                                               convergence is accepted in  *
*                                               regional computation (see   *
*                                               misp_iterate function).     *
*                                               (default = 0.05).           *
*                       error               -   error flag                  *
*                       reg_x_min           -   min x value for regional    *
*                       reg_y_min           -   min y value for regional    *
*                       x_max_dist          -   max x distance for          *
*                                               weight_mean                 *
*                       y_max_dist          -   max y distance for          *
*                                               weight_mean                 *
*                       num_points          -   number of points used for   *
*                                               the regional                *
*                       reg_multfact        -   Multiplicative factor for   *
*                                               initial coarse grid in      *
*                                               regional computation        *
*                                               (default = 4).              *
*                       squares             -                               *
*                       search_radius       -   Maximum distance from a grid*
*                                               point that data points are  *
*                                               to be used for regional     *
*                                               computation.  This is used  *
*                                               in the weight_mean function.*
*                                               (default = 20.0)            *
*                       search_radius2      -   search_radius squared       *
*                       convergence         -   convergence value for       *
*                                               weight_mean                 *
*                                                                           *
*   Outputs:            none                                                *
*                                                                           *
*   Calling Routines:   misps                                               *
*                                                                           * 
*   Glossary:           index1              -   utility integer             *
*                       index2              -   utility integer             *
*                       mode                -   0 normal grid spacing,      *
*                                               1 after halving grid spacing*
*                       loop2               -   end of loop flag            *
*                       position1           -   utility integer             *
*                       x_pos               -   x position                  *
*                       y_pos               -   y position                  *
*                       reg_width_mult      -   regional width -            *
*                                               lreg_multfact               *
*                       reg_height_mult     -   regional height -           *
*                                               lreg_multfact               *
*                       lreg_multfact       -   local reg_multfact          *
*                       value1              -   utility integer             *
*                       value2              -   utility integer             *
*                       value3              -   utility integer             *
*                       weight_value1       -                               *
*                       weight_value2       -                               *
*                       small               -                               *
*                       xvalue              -                               *
*                       yvalue              -                               *
*                       xi                  -                               *
*                       eta                 -                               *
*                       absxi               -   absolute value of xi        *
*                       abseta              -   absolute value of eta       *
*                       dist1               -                               *
*                       dist2               -                               *
*                       temp_weight         -   temp variable               *
*                                                                           *
*   Method:             This routine is based on the subroutine 'minc' in   *
*                       the article "A FORTRAN IV PROGRAM FOR INTERPOLATING *
*                       IRREGULARLY SPACED DATA USING THE DIFFERENCE        *
*                       EQUATIONS FOR MINIMUM CURVATURE" by C. J. Swain in  *
*                       Computers and Geosciences, Vol. 1 pp. 231-240.      *
*                       Mr. Swain's paper was, in turn, based on the        *
*                       article "MACHINE CONTOURING USING MINIMUM           *
*                       CURVATURE" by Ian C. Briggs in Geophysics, Vol 39,  *
*                       No. 1 (February 1974), pp. 39-48.                   *
*                                                                           *
*   Restrictions:       Throughout the CHRTR program the arrays have been   *
*                       converted to be zero based.  The reg_quadrant array *
*                       contains a quadrant number (times OFFSET) plus the  *
*                       point number of a data point returned from          *
*                       readsparse in misps.  The way that the original     *
*                       FORTRAN program dealt with these numbers was to set *
*                       the value to zero to indicate a null and set it to  *
*                       positive or negative point number to indicate       *
*                       different conditions as it processed the data.      *
*                       Since zero is a valid array index (memory offset)   *
*                       in this version of the program it was not possible  *
*                       to use the point numbers as returned from the       *
*                       readsparse function in the reg_quadrant array.  In  *
*                       order to make the algorithm perform correctly each  *
*                       of the point numbers is biased by 1 prior to being  *
*                       placed in the reg_quadrant array.  Care must be     *
*                       taken to unbias these values when they are to be    *
*                       used to retrieve data from the data array.  The     *
*                       biased values are used for testing in the           *
*                       misp_iterate function.                              *
\***************************************************************************/

#include <stdio.h>
#include <math.h>
#include "misp.h"

void min_curve (NV_FLOAT32 *weighted_mean, NV_INT32 *reg_quadrant, 
    NV_FLOAT32 *x, NV_FLOAT32 *y, NV_FLOAT32 *z, NV_FLOAT32 *sorted_pos, 
    NV_INT32 *reg_output, NV_INT32 reg_width, NV_INT32 reg_height, 
    NV_INT32 x_max_num, NV_INT32 y_max_num, NV_FLOAT32 x_increment,
    NV_FLOAT32 y_increment, NV_FLOAT32 delta, NV_INT32 *error, 
    NV_FLOAT64 reg_x_min, NV_FLOAT64 reg_y_min, NV_FLOAT32 x_max_dist, 
    NV_FLOAT32 y_max_dist, NV_INT32 num_points, NV_INT32 reg_multfact, 
    NV_FLOAT32 squares, NV_FLOAT32 search_radius, NV_FLOAT32 search_radius2, 
    NV_FLOAT32 convergence)
{
    NV_INT32     index1, index2, mode, loop2, position1, x_pos, y_pos,
                 reg_width_mult, reg_height_mult, lreg_multfact, value1, 
                 value2, value3;
            
    NV_FLOAT64   weight_value1, weight_value2, small, xvalue, yvalue, xi, eta,
                 absxi, abseta, dist1, dist2, temp_weight;
    NV_BOOL      prog_reg = NVFalse;


    void misp_iterate (NV_FLOAT32 *, NV_INT32 *, NV_INT32, NV_INT32, NV_FLOAT32, 
                       NV_FLOAT32 *, NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_FLOAT64, 
                       NV_FLOAT64, NV_FLOAT64, NV_FLOAT64);
    NV_FLOAT32 weight_mean (NV_FLOAT64, NV_FLOAT64, NV_FLOAT32, NV_FLOAT32, 
                            NV_INT32, NV_FLOAT32, NV_FLOAT32, NV_FLOAT64, NV_FLOAT64, NV_INT32, 
                            NV_INT32, NV_INT32, NV_INT32 *, NV_FLOAT32, NV_FLOAT32 *, NV_FLOAT32 *,
                            NV_FLOAT32 *, NV_FLOAT32 *);
           

    prog_reg = misp_progress_callback_registered ();


    if (prog_reg) misp_progress ("");

    small = 0.05;
    
    /*  Set initial weighted_mean values.                               */

    lreg_multfact = reg_multfact;

    weight_value2 = reg_y_min;
    for (index1 = 0; index1 < reg_height; index1 += lreg_multfact)
    {
        weight_value1 = reg_x_min;
        for (index2 = 0; index2 < reg_width; index2 += lreg_multfact)
        {
            if (prog_reg) misp_progress ("");


            *(weighted_mean + (index2 * reg_height) + index1) =
                weight_mean (weight_value1, weight_value2, search_radius,
                search_radius2, squares, x_max_dist, y_max_dist, reg_x_min,
                reg_y_min, x_max_num, y_max_num, num_points, error,
                convergence, x, y, z, sorted_pos);
            if (*error == 1) return;
            weight_value1 += x_increment * lreg_multfact;
        }
        weight_value2 += y_increment * lreg_multfact;
    }


    /*  mode = 0, while fitting surface to data points.                 */
    /*  mode = 1, while fitting surface to alternate points after       */
    /*  halving grid spacing.                                           */

    while (1)
    {
        mode = 0;
        loop2 = NVTrue;

        /*  Zero the quadrants at lreg_multfact spacing.                */
        
        for (index1 = 0; index1 < reg_height; index1 += lreg_multfact)
        {
            for (index2 = 0; index2 < reg_width; index2 += lreg_multfact)
            {
                *(reg_quadrant + (index2 * reg_height) + index1) = 0;
            }
        }


        /*  The right hand side of the following two equations must be  */
        /*  performed in single precision and the left hand side must   */
        /*  be double precision.  The original code depended on a word  */
        /*  size of 36 bits or more.  If the right hand side of the     */
        /*  following is too precise or the left hand side is not       */
        /*  precise enough then the section of code that assigns the    */
        /*  quadrant will fail because xi and eta will be exactly 0.0.  */
        /*                                                              */
        /*                                      Jan Depner, 06/24/93    */
        

        xvalue = (NV_FLOAT32) (1.0 / ((NV_FLOAT32) x_increment * (NV_FLOAT32) lreg_multfact));
        yvalue = (NV_FLOAT32) (1.0 / ((NV_FLOAT32) y_increment * (NV_FLOAT32) lreg_multfact));

        
        if (num_points > OFFSET)
        {
            printf ("Increase the size of OFFSET in misp.h\n");
            exit (-1);
        }

        /*  Select nearest grid square.                                 */

        for (index1 = 0; index1 < num_points; index1++)
        {
            xi = xvalue * (*(x + index1) - reg_x_min);
            eta = yvalue * (*(y + index1) - reg_y_min);
            x_pos = (NV_INT32) (xi + 0.5);
            y_pos = (NV_INT32) (eta + 0.5);
            xi -= x_pos;
            eta -= y_pos;
            x_pos = lreg_multfact * x_pos;
            y_pos = lreg_multfact * y_pos;
            if ((x_pos >= 0) && (x_pos < reg_width) && (y_pos >= 0) &&
                (y_pos < reg_height))
            {
                if ((x_pos <= lreg_multfact) || (x_pos >= ((reg_width - 
                    lreg_multfact) - 1)) || (y_pos <= lreg_multfact) || 
                    (y_pos >= ((reg_height - lreg_multfact) - 1)))
                {
                    position1 =
                        - *(reg_quadrant + (x_pos * reg_height) + y_pos);
                    if (position1 == 0)
                    {
                        *(reg_quadrant + (x_pos * reg_height) + y_pos) = 
                            -(index1 + 1);
                        *(weighted_mean + (x_pos * reg_height) + y_pos) = 
                            *(z + index1);
                    }
                    else if (position1 > 0)
                    {
                        dist2 =
                            (xvalue * (*(x + position1 - 1) - reg_x_min) - x_pos) *
                            (xvalue * (*(x + position1 - 1) - reg_x_min) - x_pos) +
                            (yvalue * (*(y + position1 - 1) - reg_y_min) - y_pos) *
                            (yvalue * (*(y + position1 - 1) - reg_y_min) - y_pos);
                        dist1 = xi * xi + eta * eta;
                        if ((dist1 - dist2) < 0)
                        {
                            *(reg_quadrant + (x_pos * reg_height) + y_pos) = 
                                -(index1 + 1);
                            *(weighted_mean + (x_pos * reg_height) + y_pos) = 
                                *(z + index1);
                        }
                    }
                }
                else
                {
                    absxi = fabs (xi);
                    abseta = fabs (eta);

                    if ((absxi < small) &&
                        (abseta < small))
                    {
                        *(reg_quadrant + (x_pos * reg_height) + y_pos) =
                            -(index1 + 1);
                        *(weighted_mean + (x_pos * reg_height) + y_pos) = 
                               *(z + index1);
                    }

                    /*  reg_quadrant receives integer 1 to 4 (times     */
                    /*  OFFSET to allow for number of points) to        */
                    /*  indicate which quadrant a point is in.          */
                    
                    else
                    {
                        if ((*(reg_quadrant + (x_pos * reg_height) + y_pos)
                             % OFFSET) == 0)
                        {
                            *(reg_quadrant + (x_pos * reg_height) + y_pos) =
                                index1 + 1;
                            if ((xi > 0.0) && (eta > 0.0)) *(reg_quadrant + 
                                (x_pos * reg_height) + y_pos) += 1 * OFFSET;
                            if ((xi < 0.0) && (eta > 0.0)) *(reg_quadrant + 
                                (x_pos * reg_height) + y_pos) += 2 * OFFSET;
                            if ((xi < 0.0) && (eta < 0.0)) *(reg_quadrant + 
                                (x_pos * reg_height) + y_pos) += 3 * OFFSET;
                            if ((xi > 0.0) && (eta < 0.0)) *(reg_quadrant + 
                                (x_pos * reg_height) + y_pos) += 4 * OFFSET;
                        }

                        /*  If there is more than one data point within */
                        /*  the grid square, select the closest one.    */
                        
                        else if ((*(reg_quadrant + (x_pos * reg_height) + y_pos)
                            % OFFSET) > 0)
                        {                                            
                            position1 =
                                *(reg_quadrant + (x_pos * reg_height) + y_pos)
                                - 1;
                            position1 %= OFFSET;
                            dist2 =
                                (xvalue * (*(x + position1) - reg_x_min)
                                - x_pos) * 
                                (xvalue * (*(x + position1) - reg_x_min)
                                - x_pos) + 
                                (yvalue * (*(y + position1) - reg_y_min)
                                - y_pos) *
                                (yvalue * (*(y + position1) - reg_y_min)
                                - y_pos);
                            dist1 = xi * xi + eta * eta;
                            if ((dist1 - dist2) < 0)                    
                            {
                                *(reg_quadrant + (x_pos * reg_height) + y_pos) =
                                    index1 + 1;
                                if ((xi > 0.0) && (eta > 0.0)) *(reg_quadrant + 
                                    (x_pos * reg_height) + y_pos) += 1 * OFFSET;
                                if ((xi < 0.0) && (eta > 0.0)) *(reg_quadrant + 
                                    (x_pos * reg_height) + y_pos) += 2 * OFFSET;
                                if ((xi < 0.0) && (eta < 0.0)) *(reg_quadrant + 
                                    (x_pos * reg_height) + y_pos) += 3 * OFFSET;
                                if ((xi > 0.0) && (eta < 0.0)) *(reg_quadrant + 
                                    (x_pos * reg_height) + y_pos) += 4 * OFFSET;
                            }
                        }
                    }
                }
            }
        }

        while (loop2)
        {
            if (prog_reg) misp_progress ("");

            misp_iterate (weighted_mean, reg_quadrant, reg_width, reg_height, delta, 
                          x, y, z, lreg_multfact, reg_x_min, reg_y_min, xvalue, yvalue);

            if (mode)
            {
                loop2 = NVFalse;
            }
            else
            {
                if (lreg_multfact == 1) return;

                reg_width_mult = reg_width - lreg_multfact;
                reg_height_mult = reg_height - lreg_multfact;
                value1 = lreg_multfact;
                
                /*  Halve grid spacing and assign new weighted_mean     */
                /*  values.                                             */
                
                lreg_multfact /= 2;

                for (index1 = 0; index1 < reg_height_mult; index1 += value1)
                {
                    value2 = index1 + lreg_multfact;
                    for (index2 = 0; index2 < reg_width_mult; index2 += value1)
                    {
                        value3 = index2 + lreg_multfact;
                        temp_weight =
                            *(weighted_mean + (index2 * reg_height) + index1);
                        *(weighted_mean + (value3 * reg_height) + index1) =
                            temp_weight;
                        *(weighted_mean + (index2 * reg_height) + value2) =
                            temp_weight;
                        *(weighted_mean + (value3 * reg_height) + value2) =
                            temp_weight;

                        *(reg_quadrant + (index2 * reg_height) + index1) = -1;
                        *(reg_quadrant + (value3 * reg_height) + index1) = 0;
                        *(reg_quadrant + (index2 * reg_height) + value2) = 0;
                        *(reg_quadrant + (value3 * reg_height) + value2) = 0;
                    }
                    *(weighted_mean + ((reg_width - 1) * reg_height) + value2) = 
                        *(weighted_mean + ((reg_width - 1) * reg_height) + index1);

                    *(reg_quadrant + ((reg_width - 1) * reg_height) + index1) = -1;
                    *(reg_quadrant + ((reg_width - 1) * reg_height) + value2) = 0;
                }
                for (index1 = 0; index1 < reg_width_mult; index1 += value1)
                {
                    value3 = lreg_multfact + index1;
                    *(weighted_mean + (value3 * reg_height) + reg_height - 1) = 
                        *(weighted_mean + (index1 * reg_height) + reg_height - 1);

                    *(reg_quadrant + (index1 * reg_height) + reg_height - 1) = -1;
                    *(reg_quadrant + (value3 * reg_height) + reg_height - 1) = 0;
                }
                *(reg_quadrant + ((reg_width - 1) * reg_height) + reg_height - 1) = -1;
                mode = 1;
            }
        }
    }
}
