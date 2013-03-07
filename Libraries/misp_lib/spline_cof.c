/***************************************************************************\
*                                                                           *
*   Programmer(s):      Dominic Avery, Jan C. Depner                        *
*                                                                           *
*   Date Written:       April 1992                                          *
*                                                                           *
*   Module Name:        spline_cof                                          *
*                                                                           *
*   Module Security                                                         *
*   Classification:     Unclassified                                        *
*                                                                           *
*   Data Security                                                           *
*   Classification:     Unknown                                             *
*                                                                           *
*   Purpose:            Computes coefficients for use in the spline         *
*                       function.                                           *
*                                                                           *
*   Inputs:             x               -   pointer to x data               *
*                       y               -   pointer to y data               *
*                       pos             -   position in array               *
*                       coeffs          -   coefficient array               *
*                                                                           *
*   Outputs:            None                                                *
*                                                                           *
*   Calling Routines:   spline                                              *
*                                                                           * 
*   Glossary:           index           -   utility integer                 *
*                       posval          -   pos - 1                         *
*                       inv_index       -   inverse index                   *
*                       array_a         -                                   *
*                       array_d         -                                   *
*                       array_p         -                                   *
*                       array_e         -                                   *
*                       array_b         -                                   *
*                       array_z         -                                   *
*                       value           -   utility float                   *
*                                                                           * 
\***************************************************************************/

#include <stdio.h>
#include "misp.h"

void spline_cof (NV_FLOAT32 *x, NV_FLOAT32 *y, NV_INT32 pos, 
    NV_FLOAT32 *coeffs)
{
    NV_INT32     index, posval, inv_index;
    NV_FLOAT32   array_a[SPLINE_COL][SPLINE_ROW], array_d[SPLINE_COL], 
                 array_p[SPLINE_COL], array_e[SPLINE_COL], array_b[SPLINE_COL],
                 array_z[SPLINE_COL], value;

    for (index = 0; index < pos; index++)
    {
        array_d[index] = *(x + (index + 1)) - *(x + index);
        array_p[index] = array_d[index] / 6.0;
        array_e[index] = (*(y + (index + 1)) - *(y + index)) / array_d[index];
    }

    for (index = 1; index < pos; index++)
        array_b[index] = array_e[index] - array_e[index - 1];

    array_a[0][1] = -1.0 - array_d[0] / array_d[1];
    array_a[0][2] = array_d[0] / array_d[1];
    array_a[1][2] = array_p[1] - array_p[0] * array_a[0][2];
    array_a[1][1] = 2.0 * (array_p[0] + array_p[1]) - array_p[0] *
        array_a[0][1];
    array_a[1][2] = array_a[1][2] / array_a[1][1];
    array_b[1] = array_b[1] / array_a[1][1];

    for (index = 2; index < pos; index++)
    {
        array_a[index][1] = 2.0 * (array_p[index - 1] + array_p[index]) - 
            array_p[index - 1] * array_a[index - 1][2];
        array_b[index] = array_b[index] - array_p[index - 1] *
            array_b[index - 1];
        array_a[index][2] = array_p[index] / array_a[index][1];
        array_b[index] = array_b[index] / array_a[index][1];
    }

    value =  array_d[pos - 2] / array_d[pos - 2];
    array_a[pos][0] = 1.0 + value + array_a[pos - 2][2];
    array_a[pos][1] = -value - array_a[pos][0] * array_a[pos - 1][2];
    array_b[pos] = array_b[pos - 2] - array_a[pos][0] * array_b[pos - 1];
    array_z[pos] = array_b[pos] / array_a[pos][1];
    posval = pos - 1;

    for (index = 0; index < posval; index++)
    {
        inv_index = pos - index - 1;
        array_z[inv_index] = array_b[inv_index] - array_a[inv_index][2] * 
            array_z[inv_index + 1];
    }

    array_z[0] = -array_a[0][1] * array_z[1] - array_a[0][2] * array_z[2];

    for (index = 0; index < pos; index++)
    {
        value = 1.0 / (6.0 * array_d[index]);
        *(coeffs + index) = array_z[index] * value;
        *(coeffs + SPLINE_COL + index) = array_z[index + 1] * value;
        *(coeffs + (2 * SPLINE_COL) + index) = *(y + index) / array_d[index] - 
            array_z[index] * array_p[index];
        *(coeffs + (3 * SPLINE_COL) + index) = *(y + index + 1) / 
            array_d[index] - array_z[index + 1] * array_p[index];
    }

    return;
}
