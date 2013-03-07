
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



/***************************************************************************/
/*!

  - Programmer(s):      Dominic Avery, Jan C. Depner

  - Date Written:       April 1992

  - Module Name:        spline_cof

  - Purpose:            Computes coefficients for use in the spline
                        function.

  - Inputs:
                        - x               =   pointer to x data
                        - y               =   pointer to y data
                        - pos             =   position in array
                        - coeffs          =   coefficient array

  - Outputs:
                        - void

  - Calling Routines:   spline

  - Glossary:
                        - index           =   utility integer
                        - posval          =   pos - 1
                        - inv_index       =   inverse index
                        - array_a         =
                        - array_d         =
                        - array_p         =
                        - array_e         =
                        - array_b         =
                        - array_z         =
                        - value           =   utility float

****************************************************************************/

#include <stdio.h>
#include "interp.hpp"
#include "nvtypes.h"


void spline_cof (NV_FLOAT32 *x, NV_FLOAT32 *y, NV_INT32 pos, NV_FLOAT32 *coeffs)
{
  NV_INT32        i;
  NV_FLOAT32      array_a[SPLINE_COL][SPLINE_ROW], array_d[SPLINE_COL], array_p[SPLINE_COL], array_e[SPLINE_COL], 
                  array_b[SPLINE_COL], array_z[SPLINE_COL], value;

  for (i = 0; i < pos; i++)
    {
      array_d[i] = *(x + (i + 1)) - *(x + i);
      array_p[i] = array_d[i] / 6.0;
      array_e[i] = (*(y + (i + 1)) - *(y + i)) / array_d[i];
    }

  for (i = 1; i < pos; i++) array_b[i] = array_e[i] - array_e[i - 1];

  array_a[0][1] = -1.0 - array_d[0] / array_d[1];
  array_a[0][2] = array_d[0] / array_d[1];
  array_a[1][2] = array_p[1] - array_p[0] * array_a[0][2];
  array_a[1][1] = 2.0 * (array_p[0] + array_p[1]) - array_p[0] * array_a[0][1];
  array_a[1][2] = array_a[1][2] / array_a[1][1];
  array_b[1] = array_b[1] / array_a[1][1];

  for (i = 2; i < pos; i++)
    {
      array_a[i][1] = 2.0 * (array_p[i - 1] + array_p[i]) - array_p[i - 1] * array_a[i - 1][2];
      array_b[i] = array_b[i] - array_p[i - 1] * array_b[i - 1];
      array_a[i][2] = array_p[i] / array_a[i][1];
      array_b[i] = array_b[i] / array_a[i][1];
    }

  value =  array_d[pos - 2] / array_d[pos - 2];
  array_a[pos][0] = 1.0 + value + array_a[pos - 2][2];
  array_a[pos][1] = -value - array_a[pos][0] * array_a[pos - 1][2];
  array_b[pos] = array_b[pos - 2] - array_a[pos][0] * array_b[pos - 1];
  array_z[pos] = array_b[pos] / array_a[pos][1];


  NV_INT32 posval = pos - 1;

  for (i = 0; i < posval; i++)
    {
      NV_INT32 inv_i = pos - i - 1;
      array_z[inv_i] = array_b[inv_i] - array_a[inv_i][2] * array_z[inv_i + 1];
    }

  array_z[0] = -array_a[0][1] * array_z[1] - array_a[0][2] * array_z[2];

  for (i = 0; i < pos; i++)
    {
      value = 1.0 / (6.0 * array_d[i]);
      *(coeffs + i) = array_z[i] * value;
      *(coeffs + SPLINE_COL + i) = array_z[i + 1] * value;
      *(coeffs + (2 * SPLINE_COL) + i) = *(y + i) / array_d[i] - array_z[i] * array_p[i];
      *(coeffs + (3 * SPLINE_COL) + i) = *(y + i + 1) / array_d[i] - array_z[i + 1] * array_p[i];
    }

  return;
}
