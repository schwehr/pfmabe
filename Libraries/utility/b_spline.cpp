
/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the US Government.

    Neither the United States Government nor any employees of the United States Government,
    makes any warranty, express or implied, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdio.h>
#include <string.h>
#include "nvtypes.h"


/*!*************************************************************************\
*                                                                           *
*  - Function Name:      matrix_mult                                        *
*                                                                           *
*  - Programmer(s):      Scott Ferguson, SAIC                               *
*                        Translated to C by David B. Johanson               *
*                                                                           *
*  - Date Written:       August 17, 1988                                    *
*                        C version: April 20, 1998                          *
*                                                                           *
*  - Purpose:            Multiplies matrices matrix_1 and matrix_2 to       *
*                        give matrix matrix_out.  Now hard-wired to         *
*                        multiply a 4x4 matrix by a 4x3 matrix.             *
*                                                                           *
*  - Arguments:                                                             *
*                       - matrix_1   = first matrix to be multiplied        *
*                       - matrix_2   = second matrix to be multiplied       *
*                       - matrix_out = output matrix                        *
*                                                                           *
*  - Return Value:      void                                                *
*                                                                           *
\***************************************************************************/

void matrix_mult(NV_FLOAT64 matrix_1[4][4], NV_FLOAT64 matrix_2[4][3], NV_FLOAT64 matrix_out[4][3])
{
  NV_INT32        i, j, k;             /* loop counters */
  NV_FLOAT64      temp;


  for (i = 0 ; i < 4 ; i++)
    {
      for (j = 0 ; j < 3 ; j++)
        {
          temp = 0.0L;
          for (k = 0 ; k < 4 ; k++) temp += matrix_1[i][k] * matrix_2[k][j];
          matrix_out[i][j] = temp;
        }
    }
}



/*!*************************************************************************\
*                                                                           *
*  - Function Name:      b_spline                                           *
*                                                                           *
*  - Programmer(s):      Scott Ferguson, SAIC                               *
*                        Translated to C by David B. Johanson               *
*                                                                           *
*  - Date Written:       July 18, 1988                                      *
*                        C version: April 20, 1998                          *
*                                                                           *
*  - Purpose:            Written using description by Foley and Van Dam in  *
*                        "Fundamentals of Interactive Computer Graphics",   *
*                        pp. 514-523.                                       *
*                        Inputs are four points which define the unsmoothed *
*                        curve.  Output is the parametric equation for      *
*                        X and Y in matrix form.                            *
*                                                                           *
*                        - [x] = [ a b c d ]                                *
*                            - where x = a*T**3 + b*T**2 + c*T + d          *
*                                                                           *
*                        - [y] = [ e f g h ]                                *
*                            - where y = e*T**3 + f*T**2 + g*T + h          *
*                                                                           *
*                        and T lies between 0 and 1, inclusively.           *
*                                                                           *
*                        The line can then be plotted to any degree of      *
*                        smoothness by using varying values of T.           *
*                                                                           *
*                        Note that the smoothed line is guaranteed NOT to   *
*                        fall on the points which describe the unsmoothed   *
*                        line.  The curve IS continuous as is its first     *
*                        derivative.                                        *
*                                                                           *
*  - Arguments:                                                             *
*                        - x       =  x array of four control points        *
*                        - y       =  y array of four control points        *
*                        - x_coef  =  x array of coefficients               *
*                        - y_coef  =  y array of coefficients               *
*                                                                           *
*  - Return Value:       voi                                                *
*                                                                           *
\***************************************************************************/

void b_spline(NV_FLOAT64 *x_raw, NV_FLOAT64 *y_raw, NV_FLOAT64 *x_params, NV_FLOAT64 *y_params)
{
  NV_INT32      i;              /* loop counter */
  NV_FLOAT64    gs[4][3],       /* input matrix */
                coeff[4][3];    /* output matrix */
  NV_FLOAT64    ms[4][4] =      /* constant matrix */
                { {-1.0F/6.0F,  3.0F/6.0F, -3.0F/6.0F,  1.0F/6.0F} ,
                  { 3.0F/6.0F, -6.0F/6.0F,  3.0F/6.0F,  0.0F/6.0F} ,
                  {-3.0F/6.0F,  0.0F/6.0F,  3.0F/6.0F,  0.0F/6.0F} ,
                  { 1.0F/6.0F,  4.0F/6.0F,  1.0F/6.0F,  0.0F/6.0F} };


  memset (gs, 0, sizeof(gs));
  memset (coeff, 0, sizeof(coeff));

  for (i = 0 ; i < 4 ; i++)
    {
      gs[i][0] = x_raw[i];
      gs[i][1] = y_raw[i];
    }

  matrix_mult(ms, gs, coeff);

  for (i = 0 ; i < 4 ; i++)
    {
      x_params[i] = coeff[i][0];
      y_params[i] = coeff[i][1];
    }
}
