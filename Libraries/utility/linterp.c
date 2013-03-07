
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



/*!
  - linterp.c

  - purpose:  given a set of (x,y)s and an x produce the corresponding
              estimated y.
  - author:  dave fabre, neptune sciences, inc.

  - comp/link: gcc -g -DTEST linterp.c -o linterp -lm

  - using y11 below because math.h has y1 defined as the bessel function 
    of the second kind of order 1

*/

#include "linterp.h"

#define EPS	1.0e-9

/******************************************************************************/

NV_FLOAT64 ptslope (NV_FLOAT64 x1, NV_FLOAT64 y11, NV_FLOAT64 x2, NV_FLOAT64 y2, NV_FLOAT64 x)
{
  NV_FLOAT64 slope, y = LINTERP_NODATA;

  if (fabs(x1 - x2) > EPS)
    {
      slope = (y11 - y2) / (x1 - x2);
      y = y11 + slope * (x - x1);
    }

  return y;
} /* ptslope */


/******************************************************************************/
/*!
 xa should be increasing
 this function will not extrapolate
 LINTERP_NODATA is returned on failure
*/

NV_FLOAT64 linterp (NV_FLOAT64 *xa, NV_FLOAT64 *ya, NV_INT32 n, NV_FLOAT64 x)
{
  NV_INT32 i;


  if (n < 2 || x < xa[0] || x > xa[n - 1]) return (LINTERP_NODATA);


  /* gotta have at least 2 & must be within range of x data */

  for (i = 1 ; i < n ; i++)
    {
      if (x >= xa[i - 1] && x <= xa[i]) return (ptslope (xa[i - 1], ya[i - 1], xa[i], ya[i], x));
    }

  return (LINTERP_NODATA);
} /* linterp */
