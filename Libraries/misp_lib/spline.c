/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        spline                                              *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Computes 53 point bicubic spline interpolated data  *
*                       from input data.                                    *
*                                                                           *
*   Inputs:             x               -   input x data                    *
*                       y               -   input y data                    *
*                       pos             -   position in array               *
*                       x_pos           -   x value                         *
*                       y_pos           -   y value computed                *
*                       ater            -                                   *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   interpolate                                         *
*                                                                           * 
*   Glossary:           endloop1        -   end of loop flag                *
*                       endloop2        -   end of loop flag                *
*                       coeffs          -   coefficient array               *
*                       valpos          -                                   *
*                                                                           *
*   Method:             bicubic spline                                      *
*                                                                           * 
\***************************************************************************/

#include <stdio.h>
#include <math.h>
#include "misp.h"

void spline (NV_FLOAT32 *x, NV_FLOAT32 *y, NV_INT32 pos, NV_FLOAT32 x_pos, 
    NV_FLOAT32 *y_pos, NV_FLOAT32 *ater)
{
    NV_INT32         endloop1, endloop2;

    NV_FLOAT32       coeffs[SPLINE_ROW + 1][SPLINE_COL];

    static NV_INT32  valpos;
                
    void    spline_cof (NV_FLOAT32 *, NV_FLOAT32 *, NV_INT32, NV_FLOAT32 *);


    if ((*x + *(y + pos) + *(y + (pos - 1)) + *(x + (pos - 1)) +
        *(y + (pos - 2)) - *ater) != 0.0)
    {
        spline_cof (x, y, pos, &coeffs[0][0]);

        *ater = *x + *(y + pos) + *(y + (pos - 1)) + *(x + (pos - 1)) + 
                *(y + (pos - 2));
        valpos = 0;
    }

    endloop1 = NVFalse;
    endloop2 = NVFalse;

    while (!endloop1)
    {
        endloop1 = NVTrue;
        if (fabs ((NV_FLOAT64) (x_pos - *x)) >= 0.00001)
        {
            if ((x_pos - *x) < 0) 
            {
                valpos = 0;
               
                /*  Extrapolate with average slope of closest two       */
                /*  intervals.                                          */
                
                *y_pos = *y + (x_pos - *x) * ((*(y + 1) - *y) /
                    (*(x + 1) - *x) + (*(y + 2) - *(y + 1)) / (*(x + 2) -
                    *(x + 1))) * 0.5;

                return;
            }
            else if ((x_pos - *x) == 0) 
            {
                *y_pos = *y;
                return;
            }
            else
            {
                if ((fabs ((NV_FLOAT64) (x_pos - *(x + (valpos + 1))))) >= 
                    0.00001)
                {
                    if ((x_pos - *(x + (valpos + 1))) < 0)
                    {
                        while (!endloop2)
                        {
                            endloop2 = NVTrue;
                            if ((fabs ((NV_FLOAT64) (x_pos - 
                                *(x + (valpos + 1))))) >= 0.00001)
                            {
                                if (fabs ((NV_FLOAT64) (x_pos - 
                                    *(x + valpos))) < 0)
                                {
                                    valpos--;
                                    endloop2 = NVFalse;
                                }
                                else if (fabs ((NV_FLOAT64) (x_pos - 
                                    *(x + valpos))) == 0)
                                {
                                    *y_pos = *(y + valpos);
                                    return;
                                }
                                else
                                {
                                    *y_pos = (*(x + (valpos + 1)) - x_pos) *
                                        (coeffs[0][valpos] *
                                        (*(x + (valpos + 1)) - x_pos) *
                                        (*(x + (valpos + 1)) - x_pos) + 
                                        coeffs[2][valpos]);

                                    *y_pos += (x_pos - *(x + valpos)) *
                                        (coeffs[1][valpos] * 
                                        (x_pos - *(x + valpos)) *
                                        (x_pos - *(x + valpos)) +
                                        coeffs[3][valpos]);
                                    return;
                                }
                            }
                            else
                            {
                                *y_pos = *(y + valpos);
                                return;
                            }
                        }
                    }
                    else if ((x_pos - *(x + (valpos + 1))) == 0)
                    {
                        *y_pos = *(y + (valpos + 1));
                        return;
                    }
                    
                    else
                    {
                        valpos++;
                        if ((pos - valpos) > 0)
                        {
                            endloop1 = NVFalse;
                        }
                        else
                        {
                            valpos = pos - 1;


                            /*  Extrapolate with average slope of       */
                            /*  closest two intervals.                  */
    
                            if (valpos == 1)
                            {
                                *y_pos = *y + (x_pos - *x) * ((*(y + 1) - *y) /
                                    (*(x + 1) - *x) + (*(y + 2) - *(y + 1)) /
                                    (*(x + 2) - *(x + 1))) * 0.5;
                            }
                            else
                            {
                                *y_pos = *(y + pos) + (x_pos - *(x + pos)) *
                                    ((*(y + valpos) - *(y + valpos - 1)) / 
                                    (*(x + valpos) - *(x + (valpos - 1))) +
                                    (*(y + pos) - *(y + valpos)) /
                                    (*(x + pos) - *(x + valpos))) * 0.5;
                            }
                            return;
                        }
                    }
                }
                else
                {
                    *y_pos = *(y + (valpos + 1));
                    return;
                }
            }
        }
        else
        {
            *y_pos = *y;
            return;
        }
    }
}
