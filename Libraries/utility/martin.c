
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

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "martin.h"

/**************************************************************************/
/*!

  - Module Name:        martin

  - Programmer(s):      Unknown

  - Date Written:       Unknown

  - Purpose:            Compute filter weights for the convolve function.

  - Arguments:
                        - cutoff      =   cutoff frequency
                        - slope       =   filter slope
                        - length      =   filter length
                        - weights     =   array of weights

  - Return Value:       None

****************************************************************************/

void martin (NV_FLOAT64 cutoff, NV_FLOAT64 slope, NV_INT32 length, NV_FLOAT64 *weights, NV_INT32 itype)
{
  NV_FLOAT64           cuth, delta, dx, p, pi2, picut2, q, sum, x;
  NV_INT32             i, n1;


  /*  Make sure we haven't exceeded our maximum number of filter weights.  */

  if (length >= MAX_MARTIN_FILTER_WEIGHTS)
    {
      fprintf (stderr, "\n\n%d filter weights exceeds maximum allowable number of filter weights (%d)\n\n", length, MAX_MARTIN_FILTER_WEIGHTS - 1);
      exit (-1);
    }


  /*  Martin computes one side of the filter coefficients.    */

  pi2 = 6.28318531;
  cuth = cutoff + slope;
  picut2 = pi2 * cuth;
  n1 = length + 1;
  weights[0] = 2.0 * cuth;
  sum = 0.0;


  for (i = 1 ; i < n1 ; i++)
    {
      p = i;
      q = 1.0 - 16.0 * (slope * slope) * (p * p);
      if (fabs (q) <= 0.00001)
        {
          weights[i] = sin (p * picut2) * 0.25 / p;
        }
      else
        {
          weights[i] = 2.0 * cos (pi2 * p * slope) / q * sin (picut2 * p) / (pi2 * p);
        }
      sum += weights[i];
    }


  /*  Correct the weights for unity gain. */

  delta = 1.0 - (weights[0] + 2.0 * sum);
  x = 2 * length + 1;
  dx = delta / x;


  /*  High pass.  */

  if (!itype)
    {
      for (i = 0 ; i < n1 ; i++) weights[i] = -(weights[i] + dx);

      weights[0] += 1.0;
    }


    /*  Low pass.   */

  else
    {
      for (i = 0 ; i < n1 ; i++) weights[i] += dx;
    }
}
