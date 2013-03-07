/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        misp_iterate                                        *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Applies finite difference equations for minimum     *
*                       curvature to weighted_mean controlled by station    *
*                       values x, y, z.  Station numbers are contained in   *
*                       reg_quadrant along with an integer 1 to 4 which     *
*                       determines how the off-grid equations are applied.  *
*                       The term station numbers refers to point numbers    *
*                       that are returned from readsparse in misps.  The    *
*                       values are stored as the quadrant * OFFSET + the    *
*                       point number.  Offset must be larger than the       *
*                       number of points returned from readsparse.          *
*                                                                           *
*   Inputs:             weighted_mean       -   regional data               *
*                       reg_quadrant        -   station numbers and         *
*                                               quadrants                   *
*                       reg_width           -   width of the regional grid  *
*                       reg_height          -   height of the regional grid *
*                       delta               -   The value of the maximum    *
*                                               change in the surface       *
*                                               between iterations at which *
*                                               convergence is accepted in  *
*                                               regional computation        *
*                       x                   -   x station values            *
*                       y                   -   y station values            *
*                       z                   -   z station values            *
*                       lreg_multfact       -   local regional mult factor  *
*                       reg_xmin            -   regional minimum x value    *
*                                               for the subarea             *
*                       reg_ymin            -   regional minimum y value    *
*                                               for the subarea             *
*                       xvalue              -                               *
*                       yvalue              -                               *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   minc                                                *
*                                                                           * 
*   Method:             This routine is based on the subroutine 'iterat' in *
*                       the article "A FORTRAN IV PROGRAM FOR INTERPOLATING *
*                       IRREGULARLY SPACED DATA USING THE DIFFERENCE        *
*                       EQUATIONS FOR MINIMUM CURVATURE" by C. J. Swain in  *
*                       Computers and Geosciences, Vol. 1 pp. 231-240.      *
*                       Mr. Swain's paper was, in turn, based on the        *
*                       article "MACHINE CONTOURING USING MINIMUM           *
*                       CURVATURE" by Ian C. Briggs in Geophysics, Vol 39,  *
*                       No. 1 (February 1974), pp. 39-48.                   *
*                                                                           *
*   Restrictions:       None                                                *
*                                                                           * 
\***************************************************************************/

#include <stdio.h>
#include <math.h>
#include "misp.h"

void misp_iterate (NV_FLOAT32 *weighted_mean, NV_INT32 *reg_quadrant, 
                   NV_INT32 reg_width, NV_INT32 reg_height, NV_FLOAT32 delta, NV_FLOAT32 *x, 
                   NV_FLOAT32 *y, NV_FLOAT32 *z, NV_INT32 lreg_multfact, NV_FLOAT64 reg_x_min,
                   NV_FLOAT64 reg_y_min, NV_FLOAT64 xvalue, NV_FLOAT64 yvalue)
{
    NV_INT32   mfact1, mfact2, mfact3, mfact4, xmfact1, xmfact2, xmfact3, 
               ymfact1, ymfact2, ymfact3, yoff2, yoff3, yoff4, xoff1, xoff2, 
               xoff3, iterate1, posval1, posval2, noff, rowpos1, rowpos2, 
               rowpos3, rowpos4, datapoint1, datapoint2, index1, index2;
            
    NV_FLOAT32 iterate2, dataval0, dataval1, dataval2, value, xi, eta, xy1,
               coeff1, coeff2, coeff3, coeff4, coeff5, sum;


    /*  mfact1 and mfact3 are kept for ease of understanding (HA!) when */
    /*  comparing this code to that in the referenced articles.         */
    
    mfact1 = lreg_multfact * 2;
    mfact2 = lreg_multfact;
    mfact3 = lreg_multfact * 2;
    mfact4 = lreg_multfact * 3;
    xmfact1 = (reg_width - lreg_multfact) - 1;
    xmfact2 = (reg_width - (lreg_multfact * 2)) - 1;                
    xmfact3 = (reg_width - (lreg_multfact * 3)) - 1;
    ymfact1 = (reg_height - lreg_multfact) - 1;
    ymfact2 = (reg_height - (lreg_multfact * 2)) - 1;
    ymfact3 = (reg_height - (lreg_multfact * 3)) - 1;

    /*  These are offsets into the data that are used throughout this   */
    /*  function.  They are computed here to save having to recompute   */
    /*  each time they are needed.                                      */
    
    yoff2 = mfact2 * reg_height;
    yoff3 = mfact3 * reg_height;
    yoff4 = mfact4 * reg_height;
    xoff1 = xmfact1 * reg_height;
    xoff2 = xmfact2 * reg_height;
    xoff3 = xmfact3 * reg_height;
    noff = (reg_width - 1) * reg_height;

    iterate1 = 0;
    iterate2 = 0.0;
    dataval0 = 0.0;

    while (1)
    {
        if (iterate1 != 0) dataval2 = dataval0;
        dataval0 = 0.0;
               
        /*  First Row.                                                  */

        if (!(*reg_quadrant))
        {
            *weighted_mean = (-0.5 * (*(weighted_mean + mfact3) +
            *(weighted_mean + yoff3)) +
            *(weighted_mean + mfact2) +
            *(weighted_mean + yoff2));
        }

        if (!(*(reg_quadrant + yoff2)))
        {
            *(weighted_mean + yoff2) =
            (-*(weighted_mean + yoff2 + mfact3) -
            *(weighted_mean + yoff3 + mfact2) -
            *(weighted_mean + mfact2) -
            *(weighted_mean + yoff4) + 2.0 *
            *(weighted_mean) + 4.0 *
            (*(weighted_mean + yoff3) +
            *(weighted_mean + yoff2 + mfact2))) / 6.0;
        }

        if (xmfact2 >= mfact3)
        {
            for (index1 = mfact3; index1 <= xmfact2; index1 += lreg_multfact)
            {
                posval1 = index1 + lreg_multfact;
                posval2 = index1 - lreg_multfact;
                if (!(*(reg_quadrant + (index1 * reg_height))))
                {
                    *(weighted_mean + (index1 * reg_height)) =
                    (-*(weighted_mean + ((index1 - mfact1) * reg_height)) -
                    *(weighted_mean + ((index1 + mfact1) * reg_height)) -
                    *(weighted_mean + (index1 * reg_height) + mfact3) -
                    *(weighted_mean + (posval1 * reg_height) + mfact2) -
                    *(weighted_mean + (posval2 * reg_height) + mfact2) + 4.0 * 
                    (*(weighted_mean + (posval2 * reg_height)) + 
                    *(weighted_mean + (index1 * reg_height) + mfact2) + 
                    *(weighted_mean + (posval1 * reg_height)))) / 7.0;
                }
            }
        }

        if (!(*(reg_quadrant + xoff1)))
        {
            *(weighted_mean + xoff1) =
            (-*(weighted_mean + xoff1 + mfact3) -
            *(weighted_mean + xoff2 + mfact2) -
            *(weighted_mean + noff + mfact2) -
            *(weighted_mean + xoff3) + 2.0 * 
            *(weighted_mean + noff) + 4.0 * 
            (*(weighted_mean + xoff2) +
            *(weighted_mean + xoff1 + mfact2))) / 6.0;
        }

        if (!(*(reg_quadrant + noff)))
        {
            *(weighted_mean + noff) = (-0.5 * 
            (*(weighted_mean + noff + mfact3) + 
            *(weighted_mean + xoff2)) +
            *(weighted_mean + noff + mfact2) +
            *(weighted_mean + xoff1));
        }
        
        /*  Second Row.                                                 */
        
        if (!(*(reg_quadrant + mfact2)))
        {
            *(weighted_mean + mfact2) =
            (-*(weighted_mean + yoff3 + mfact2) -
            *(weighted_mean + yoff2 + mfact3) -
            *(weighted_mean + yoff2) -
            *(weighted_mean + mfact4) + 2.0 *
            *weighted_mean + 4.0 *
            (*(weighted_mean + mfact3) + 
            *(weighted_mean + yoff2 + mfact2))) / 6.0;
        }

        if (!(*(reg_quadrant + yoff2 + mfact2)))
        {
            *(weighted_mean + yoff2 + mfact2) = 
            (-*(weighted_mean + yoff2 + mfact4) - 
            *(weighted_mean + yoff4 + mfact2) - 
            *(weighted_mean + mfact3) -
            *(weighted_mean + yoff3) - 2.0 *
            *(weighted_mean + yoff3 + mfact3) + 8.0 *
            (*(weighted_mean + yoff2 + mfact3) +
            *(weighted_mean + yoff3 + mfact2)) + 4.0 *
            (*(weighted_mean + yoff2) +
            *(weighted_mean + mfact2))) / 18.0;
        }

        if (xmfact2 >= mfact3)
        {
            for (index1 = mfact3; index1 <= xmfact2; index1 += lreg_multfact)
            {
                posval1 = index1 + lreg_multfact;
                posval2 = index1 - lreg_multfact;
                if (!(*(reg_quadrant + (index1 * reg_height) + mfact2)))
                {
                    *(weighted_mean + (index1 * reg_height) + mfact2) = 
                    (-*(weighted_mean + ((index1 - mfact1) * reg_height) + 
                    mfact2) - *(weighted_mean + ((index1 + mfact1) * 
                    reg_height) + mfact2) - *(weighted_mean + (index1 * 
                    reg_height) + mfact4) - 2.0 * (*(weighted_mean + 
                    (posval2 * reg_height) + mfact3) + *(weighted_mean + 
                    (posval1 * reg_height) + mfact3)) - *(weighted_mean + 
                    (posval2 * reg_height)) - *(weighted_mean + (posval1 * 
                    reg_height)) + 8.0 * (*(weighted_mean + (posval2 * 
                    reg_height) + mfact2) + *(weighted_mean + (index1 * 
                    reg_height) + mfact3) + *(weighted_mean + (posval1 * 
                    reg_height) + mfact2)) + 4.0 * *(weighted_mean + (index1 *
                    reg_height))) / 19.0;
                }
            }
        }

        if (!(*(reg_quadrant + xoff1 + mfact2)))
        {
            *(weighted_mean + xoff1 + mfact2) = 
            (-*(weighted_mean + xoff1 + mfact4) - 
            *(weighted_mean + xoff3 + mfact2) - 
            *(weighted_mean + noff + mfact3) -
            *(weighted_mean + xoff2) - 2.0 * 
            *(weighted_mean + xoff2 + mfact3) + 8.0 * 
            (*(weighted_mean + xoff1 + mfact3) +
            *(weighted_mean + xoff2 + mfact2)) + 4.0 *
            (*(weighted_mean + xoff1) +
            *(weighted_mean + noff + mfact2))) / 18.0;
        }

        if (!(*(reg_quadrant + noff + mfact2)))
        {
            *(weighted_mean + noff + mfact2) = 
            (-*(weighted_mean + xoff2 + mfact2) - 
            *(weighted_mean + xoff1 + mfact3) - 
            *(weighted_mean + xoff1) -
            *(weighted_mean + noff + mfact4) + 2.0 *
            *(weighted_mean + noff) + 4.0 *
            (*(weighted_mean + noff + mfact3) +
            *(weighted_mean + xoff1 + mfact2))) / 6.0;
        }

        /*  Third to (m - 2)th Row.                                     */
        
        if (ymfact2 >= mfact3)
        {
            for (index1 = mfact3; index1 <= ymfact2; index1 += lreg_multfact)
            {
                rowpos1 = index1 + lreg_multfact;
                rowpos2 = index1 - lreg_multfact;
                rowpos3 = index1 + mfact1;
                rowpos4 = index1 - mfact1;
                if (!(*(reg_quadrant + index1)))
                {
                    *(weighted_mean + index1) =
                    (-*(weighted_mean + rowpos4) -
                    *(weighted_mean + rowpos3) -
                    *(weighted_mean + yoff3 + index1) -
                    *(weighted_mean + yoff2 + rowpos2) -
                    *(weighted_mean + yoff2 + rowpos1) + 4.0 * 
                    (*(weighted_mean + rowpos2) +
                    *(weighted_mean + yoff2 + index1) +
                    *(weighted_mean + rowpos1))) / 7.0;
                }

                if (!(*(reg_quadrant + yoff2 + index1)))
                {
                    *(weighted_mean + yoff2 + index1) = 
                    (-*(weighted_mean + yoff2 + rowpos4) -
                    *(weighted_mean + yoff2 + rowpos3) - 
                    *(weighted_mean + yoff4 + index1) - 2.0 *
                    (*(weighted_mean + yoff3 + rowpos2) +
                    *(weighted_mean + yoff3 + rowpos1)) -
                    *(weighted_mean + rowpos2) - 
                    *(weighted_mean + rowpos1) + 8.0 *
                    (*(weighted_mean + yoff2 + rowpos2) +
                    *(weighted_mean + yoff3 + index1) +
                    *(weighted_mean + yoff2 + rowpos1)) + 4.0 * 
                    *(weighted_mean + index1)) / 19.0;
                }

                if (xmfact2 >= mfact3)
                {
                    for (index2 = mfact3; index2 <= xmfact2; index2 += 
                        lreg_multfact)
                    {
                        posval1 = index2 + lreg_multfact;
                        posval2 = index2 - lreg_multfact;
                        if (!(*(reg_quadrant + (index2 * reg_height) + index1)))
                        {
                            value = 0.05 *
                            (-*(weighted_mean + ((index2 + mfact1) * reg_height) + index1) - 
                            *(weighted_mean + (index2 * reg_height) + rowpos3) -
                            *(weighted_mean + ((index2 - mfact1) * reg_height) + index1) - 
                            *(weighted_mean + (index2 * reg_height) + rowpos4) - 2.0 *
                            (*(weighted_mean + (posval1 * reg_height) + rowpos1) +
                            *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                            *(weighted_mean + (posval1 * reg_height) + rowpos2) + 
                            *(weighted_mean + (posval2 * reg_height) + rowpos2)) + 8.0 *
                            (*(weighted_mean + (posval1 * reg_height) + index1) +
                            *(weighted_mean + (posval2 * reg_height) + index1) + 
                            *(weighted_mean + (index2 * reg_height) + rowpos1) +
                            *(weighted_mean + (index2 * reg_height) + rowpos2)));

                            dataval1 = fabs ((NV_FLOAT64) (value - 
                                *(weighted_mean + (index2 * reg_height) + index1)));
                            if (dataval1 > dataval0) dataval0 = dataval1;

                            *(weighted_mean + (index2 * reg_height) + index1) = 
                                value;
                        }
                        else if (*(reg_quadrant + (index2 * reg_height) + index1)
                            > 0)
                        {
                            datapoint1 = *(reg_quadrant + (index2 * reg_height)
                                + index1) - 1;
                            datapoint2 = datapoint1 / OFFSET;
                            datapoint1 -= (datapoint2 * OFFSET);
                            xi = xvalue * (*(x + datapoint1) - reg_x_min) + 1.0;
                            eta = yvalue * (*(y + datapoint1) - reg_y_min) +
                                1.0;

                            xi -= (NV_INT32) (xi + 0.5);
                            eta -= (NV_INT32) (eta + 0.5);
                            xi = fabs ((NV_FLOAT64) xi);
                            eta = fabs ((NV_FLOAT64) eta);

                            xy1 = 2.0 / (xi + eta + 1.0);
                            coeff5 = (2.0 / (xi + eta)) * xy1;
                            coeff1 = 1.0 - xi * (1.0 + xi) * coeff5 * 0.5;
                            coeff4 = 1.0 - eta * (1.0 + eta) * coeff5 * 0.5;
                            coeff2 = (1.0 + xi - eta) * xy1;
                            coeff3 = (1.0 + eta - xi) * xy1;
                            if ((datapoint2 - 2) < 0)
                            {
                                sum = coeff1 *
                                    *(weighted_mean + (posval1 * reg_height) + rowpos2) +
                                    coeff2 *
                                    *(weighted_mean + (index2 * reg_height) + rowpos2) +
                                    coeff3 *
                                    *(weighted_mean + (posval2 * reg_height) + index1) +
                                    coeff4 *
                                    *(weighted_mean + (posval2 * reg_height) + rowpos1);
                                value =
                                    (-*(weighted_mean + ((index2 + mfact1) * reg_height) + index1) -
                                    *(weighted_mean + (index2 * reg_height) + rowpos3) - 
                                    *(weighted_mean + ((index2 - mfact1) * reg_height) + index1) -
                                    *(weighted_mean + (index2 * reg_height) + rowpos4) - 2.0 * 
                                    (*(weighted_mean + (posval1 * reg_height) + rowpos1) +
                                    *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                                    *(weighted_mean + (posval1 * reg_height) + rowpos2) + 
                                    *(weighted_mean + (posval2 * reg_height) + rowpos2)) + 4.0 *
                                    (*(weighted_mean + (posval1 * reg_height) + index1) + 
                                    *(weighted_mean + (posval2 * reg_height) + index1) +
                                    *(weighted_mean + (index2 * reg_height) + rowpos1) +
                                    *(weighted_mean + (index2 * reg_height) + rowpos2)) + 4.0 * 
                                    (sum + coeff5 * *(z + datapoint1))) /
                                    (4.0 * (1.0 + coeff1 + coeff2 + coeff3 + 
                                    coeff4 + coeff5));

                                dataval1 = fabs ((NV_FLOAT64) (value - 
                                    *(weighted_mean + (index2 * reg_height) + index1)));
                                if (dataval1 > dataval0) dataval0 = dataval1;

                                *(weighted_mean + (index2 * reg_height) + index1) 
                                    = value;
                            }
                            else if ((datapoint2 - 2) == 0)
                            {
                                sum = coeff1 *
                                    *(weighted_mean + (posval2 * reg_height) + rowpos2) +
                                    coeff2 * 
                                    *(weighted_mean + (index2 * reg_height) + rowpos2) +
                                    coeff3 *
                                    *(weighted_mean + (posval1 * reg_height) + index1) +
                                    coeff4 * 
                                    *(weighted_mean + (posval1 * reg_height) + rowpos1);
                                        
                                value =
                                    (-*(weighted_mean + ((index2 + mfact1) * reg_height) + index1) - 
                                    *(weighted_mean + (index2 * reg_height) + rowpos3) -
                                    *(weighted_mean + ((index2 - mfact1) * reg_height) + index1) - 
                                    *(weighted_mean + (index2 * reg_height) + rowpos4) - 2.0 *
                                    (*(weighted_mean + (posval1 * reg_height) + rowpos1) + 
                                    *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                                    *(weighted_mean + (posval1 * reg_height) + rowpos2) +
                                    *(weighted_mean + (posval2 * reg_height) + rowpos2)) + 4.0 * 
                                    (*(weighted_mean + (posval1 * reg_height) + index1) +
                                    *(weighted_mean + (posval2 * reg_height) + index1) +
                                    *(weighted_mean + (index2 * reg_height) + rowpos1) + 
                                    *(weighted_mean + (index2 * reg_height) + rowpos2)) +
                                    4.0 * (sum + coeff5 *
                                    *(z + datapoint1))) / (4.0 * (1.0 + 
                                    coeff1 + coeff2 + coeff3 + coeff4 + 
                                    coeff5));

                                dataval1 = fabs ((NV_FLOAT64) (value - 
                                    *(weighted_mean + (index2 * reg_height) + index1)));
                                if (dataval1 > dataval0) dataval0 = dataval1;

                                *(weighted_mean + (index2 * reg_height) + index1) 
                                    = value;
                            }
                            else if ((datapoint2 - 2) > 0)
                            {
                                if ((datapoint2 - 4) < 0)
                                {
                                    sum = coeff1 *
                                        *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                                        coeff2 *
                                        *(weighted_mean + (index2 * reg_height) + rowpos1) +
                                        coeff3 *
                                        *(weighted_mean + (posval1 * reg_height) + index1) + 
                                        coeff4 *
                                        *(weighted_mean + (posval1 * reg_height) + rowpos2);
                                    value =
                                        (-*(weighted_mean + ((index2 + mfact1) * reg_height) + index1) - 
                                        *(weighted_mean + (index2 * reg_height) + rowpos3) -
                                        *(weighted_mean + ((index2 - mfact1) * reg_height) + index1) - 
                                        *(weighted_mean + (index2 * reg_height) + rowpos4) - 2.0 *
                                        (*(weighted_mean + (posval1 * reg_height) + rowpos1) + 
                                        *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                                        *(weighted_mean + (posval1 * reg_height) + rowpos2) +
                                        *(weighted_mean + (posval2 * reg_height) + rowpos2)) + 4.0 *
                                        (*(weighted_mean + (posval1 * reg_height) + index1) +
                                        *(weighted_mean + (posval2 * reg_height) + index1) +
                                        *(weighted_mean + (index2 * reg_height) + rowpos1) + 
                                        *(weighted_mean + (index2 * reg_height) + rowpos2)) +
                                        4.0 * (sum + coeff5 * 
                                        *(z + datapoint1))) / (4.0 * 
                                        (1.0 + coeff1 + coeff2 + coeff3 + coeff4
                                        + coeff5));

                                    dataval1 = fabs ((NV_FLOAT64) (value - 
                                        *(weighted_mean + (index2 * reg_height) + index1)));
                                    if (dataval1 > dataval0) dataval0 =
                                        dataval1;

                                    *(weighted_mean + (index2 * reg_height) + 
                                        index1) = value;
                                }
                                else if ((datapoint2 - 4) == 0)
                                {
                                    sum = coeff1 *
                                        *(weighted_mean + (posval1 * reg_height) + rowpos1) +
                                        coeff2 * 
                                        *(weighted_mean + (index2 * reg_height) + rowpos1) +
                                        coeff3 *
                                        *(weighted_mean + (posval2 * reg_height) + index1) +
                                        coeff4 *
                                        *(weighted_mean + (posval2 * reg_height) + rowpos2);
                                    value =
                                        (-*(weighted_mean + ((index2 + mfact1) * reg_height) + index1) - 
                                        *(weighted_mean + (index2 * reg_height) + rowpos3) -
                                        *(weighted_mean + ((index2 - mfact1) * reg_height)+ index1) - 
                                        *(weighted_mean + (index2 * reg_height) + rowpos4) - 2.0 *
                                        (*(weighted_mean + (posval1 * reg_height) + rowpos1) + 
                                        *(weighted_mean + (posval2 * reg_height) + rowpos1) +
                                        *(weighted_mean + (posval1 * reg_height ) + rowpos2) +
                                        *(weighted_mean + (posval2 * reg_height) + rowpos2)) + 4.0 *
                                        (*(weighted_mean + (posval1 * reg_height) + index1) +
                                        *(weighted_mean + (posval2 * reg_height) + index1) + 
                                        *(weighted_mean + (index2 * reg_height) + rowpos1) +
                                        *(weighted_mean + (index2 * reg_height) + rowpos2)) +
                                        4.0 * (sum + coeff5 *
                                        *(z + datapoint1))) / 
                                        (4.0 * (1.0 + coeff1 + coeff2 + coeff3 +
                                        coeff4 + coeff5));

                                    dataval1 = fabs ((NV_FLOAT64) (value - 
                                        *(weighted_mean + (index2 * reg_height) + index1)));
                                    if (dataval1 > dataval0) dataval0 =
                                        dataval1;

                                    *(weighted_mean + (index2 * reg_height) + 
                                        index1) = value;
                                }
                                else if ((datapoint2 - 4) > 0) return;
                            }
                        }
                    }
                }
                        
                if (!(*(reg_quadrant + xoff1 + index1)))
                {
                    *(weighted_mean + xoff1 + index1) = 
                    (-*(weighted_mean + xoff1 + rowpos4) - 
                    *(weighted_mean + xoff1 + rowpos3) - 
                    *(weighted_mean + xoff3 + index1) - 2.0 *
                    (*(weighted_mean + xoff2 + rowpos2) + 
                    *(weighted_mean + xoff2 + rowpos1)) -
                    *(weighted_mean + noff + rowpos2) -
                    *(weighted_mean + noff + rowpos1) + 8.0 *
                    (*(weighted_mean + xoff1 + rowpos2) +
                    *(weighted_mean + xoff2 + index1) +
                    *(weighted_mean + xoff1 + rowpos1)) + 4.0 *
                    *(weighted_mean + noff + index1)) / 19.0;
                }

                if (!(*(reg_quadrant + noff + index1)))
                {
                    *(weighted_mean + noff + index1) = 
                    (-*(weighted_mean + noff + rowpos4) -
                    *(weighted_mean + noff + rowpos3) -
                    *(weighted_mean + xoff2 + index1) -
                    *(weighted_mean + xoff1 + rowpos2) -
                    *(weighted_mean + xoff1 + rowpos1) + 4.0 *
                    (*(weighted_mean + noff + rowpos2) +
                    *(weighted_mean + xoff1 + index1) +
                    *(weighted_mean + noff + rowpos1))) / 7.0;
                }
            }
        }
                        
               
        /*  (m - 1)th Row.                                              */

        if (!(*(reg_quadrant + ymfact1)))
        {
            *(weighted_mean + ymfact1) =
            (-*(weighted_mean + yoff3 + ymfact1) -
            *(weighted_mean + yoff2 + ymfact2) -
            *(weighted_mean + yoff2 + reg_height - 1) -
            *(weighted_mean + ymfact3) + 2.0 *
            *(weighted_mean + reg_height - 1) + 4.0 *
            (*(weighted_mean + ymfact2) +
            *(weighted_mean + yoff2 + ymfact1))) / 6.0;
        }

        if (!(*(reg_quadrant + yoff2 + ymfact1)))
        {
            *(weighted_mean + yoff2 + ymfact1) = 
            (-*(weighted_mean + yoff2 + ymfact3) - 
            *(weighted_mean + yoff4 + ymfact1) - 
            *(weighted_mean + ymfact2) -
            *(weighted_mean + yoff3 + reg_height - 1) - 2.0 *
            *(weighted_mean + yoff3 + ymfact2) + 8.0 *
            (*(weighted_mean + yoff2 + ymfact2) +
            *(weighted_mean + yoff3 + ymfact1)) + 4.0 *
            (*(weighted_mean + yoff2 + reg_height - 1) +
            *(weighted_mean + ymfact1))) / 18.0;
        }

        if (xmfact2 >= mfact3)
        {
            for (index1 = mfact3; index1 <= xmfact2; index1 += lreg_multfact)
            {
                posval1 = index1 + lreg_multfact;
                posval2 = index1 - lreg_multfact;
                if (!(*(reg_quadrant + (index1 * reg_height) + ymfact1)))
                {
                    *(weighted_mean + (index1 * reg_height) + ymfact1) =
                    (-*(weighted_mean + ((index1 - mfact1) * reg_height) + ymfact1) -
                    *(weighted_mean + ((index1 + mfact1) * reg_height) + ymfact1) -
                    *(weighted_mean + (index1 * reg_height) + ymfact3) - 2.0 *
                    (*(weighted_mean + (posval2 * reg_height) + ymfact2) +
                    *(weighted_mean + (posval1 * reg_height) + ymfact2)) -
                    *(weighted_mean + (posval2 * reg_height) + reg_height - 1) -
                    *(weighted_mean + (posval1 * reg_height) + reg_height - 1) + 8.0 *
                    (*(weighted_mean + (posval2 * reg_height) + ymfact1) +
                    *(weighted_mean + (index1 * reg_height) + ymfact2) +
                    *(weighted_mean + (posval1 * reg_height) + ymfact1)) + 4.0 * 
                    *(weighted_mean + (index1 * reg_height) + reg_height - 1)) / 
                    19.0;
                }
            }
        }
                
        if (!(*(reg_quadrant + xoff1 + ymfact1)))
        {
            *(weighted_mean + xoff1 + ymfact1) =
            (-*(weighted_mean + xoff1 + ymfact3) -
            *(weighted_mean + xoff3 + ymfact1) -
            *(weighted_mean + noff + ymfact2) -
            *(weighted_mean + xoff2 + reg_height - 1) - 2.0 *
            *(weighted_mean + xoff2 + ymfact2) + 8.0 *
            (*(weighted_mean + xoff1 + ymfact2) + 
            *(weighted_mean + xoff2 + ymfact1)) + 4.0 * 
            (*(weighted_mean + xoff1 + reg_height - 1) + 
            *(weighted_mean + noff + ymfact1))) / 18.0;
        }

        if (!(*(reg_quadrant + noff + ymfact1)))
        {
            *(weighted_mean + noff + ymfact1) = 
            (-*(weighted_mean + xoff2 + ymfact1) - 
            *(weighted_mean + xoff1 + ymfact2) - 
            *(weighted_mean + xoff1 + reg_height - 1) - 
            *(weighted_mean + noff + ymfact3) + 2.0 * 
            *(weighted_mean + noff + reg_height - 1) + 4.0 * 
            (*(weighted_mean + noff + ymfact2) + 
            *(weighted_mean + xoff1 + ymfact1))) / 6.0;
        }
               
        /*  mth Row.                                                    */

        if (!(*(reg_quadrant + reg_height - 1)))
        {
            *(weighted_mean + reg_height - 1) =
            (*(weighted_mean + ymfact1) +
            *(weighted_mean + yoff2 + reg_height - 1) - 0.5 *
            (*(weighted_mean + ymfact2) +
            *(weighted_mean + yoff3 + reg_height - 1)));
        }

        if (!(*(reg_quadrant + yoff2 + reg_height - 1)))
        {
            *(weighted_mean + yoff2 + reg_height - 1) = 
            (-*(weighted_mean + yoff2 + ymfact2) -
            *(weighted_mean + yoff3 + ymfact1) - 
            *(weighted_mean + ymfact1) -
            *(weighted_mean + yoff4 + reg_height - 1) + 2.0 *
            *(weighted_mean + reg_height - 1) + 4.0 *
            (*(weighted_mean + yoff3 + reg_height - 1) + 
            *(weighted_mean + yoff2 + ymfact1))) / 6.0;
        }
                
        if (xmfact2 >= mfact3)
        {
            for (index1 = mfact3; index1 <= xmfact2; index1 += lreg_multfact)  
            {
                posval1 = index1 + lreg_multfact;
                posval2 = index1 - lreg_multfact;
                if (!(*(reg_quadrant + (index1 * reg_height) + reg_height - 1)))
                {
                    *(weighted_mean + (index1 * reg_height) + reg_height - 1) = 
                    (-*(weighted_mean + ((index1 - mfact1) * reg_height) + reg_height - 1) -
                    *(weighted_mean + ((index1 + mfact1) * reg_height) + reg_height - 1) -
                    *(weighted_mean + (index1 * reg_height) + ymfact2) -
                    *(weighted_mean + (posval2 * reg_height) + ymfact1) -
                    *(weighted_mean + (posval1 * reg_height) + ymfact1) + 4.0 *
                    (*(weighted_mean + (posval2 * reg_height) + reg_height - 1) +
                    *(weighted_mean + (index1 * reg_height) + ymfact1) +
                    *(weighted_mean + (posval1 * reg_height) + reg_height - 1))) / 7.0;
                }
            }
        }
                
        if (!(*(reg_quadrant + xoff1 + reg_height - 1)))
        {
            *(weighted_mean + xoff1 + reg_height - 1) = 
            (-*(weighted_mean + xoff1 + ymfact2) - 
            *(weighted_mean + xoff2 + ymfact1) - 
            *(weighted_mean + noff + ymfact1) -
            *(weighted_mean + xoff3 + reg_height - 1) + 2.0 *
            *(weighted_mean + noff + reg_height - 1) + 4.0 *
            (*(weighted_mean + xoff2 + reg_height - 1) + 
            *(weighted_mean + xoff1 + ymfact1))) / 6.0;
        }

        if (!(*(reg_quadrant + noff + reg_height - 1)))
        {
            *(weighted_mean + noff + reg_height - 1) = 
            (*(weighted_mean + noff + ymfact1) + 
            *(weighted_mean + xoff1 + reg_height - 1) - 0.5 *
            (*(weighted_mean + noff + ymfact2) +
            *(weighted_mean + xoff2 + reg_height - 1)));
        }

        iterate1++;
                
               
        /*  One complete iteration.                                     */
                
        iterate2 += 1.0;
        if (iterate1 != 1 && (dataval0 < delta || iterate1 >= 400)) return;
    }
}
