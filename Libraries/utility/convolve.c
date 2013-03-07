
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



#include "convolve.h"

/***************************************************************************/
/*!

  - Module Name:        convolve

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 1994

  - Purpose:            Convolve one row data with the filter.
                        The filter weights were created using the function
                        martin.

  - Arguments:
                        - indat       =   input row
                        - outdat      =   output row
                        - width       =   row size
                        - length      =   filter length

  - Return value:       None

****************************************************************************/

void convolve (NV_FLOAT64 *indat, NV_FLOAT64 *outdat, NV_INT32 width, NV_INT32 length, NV_FLOAT64 *weights)
{
  NV_INT32             i, ii, j, k, npts;
  NV_FLOAT64           filter[MAX_MARTIN_FILTER_WEIGHTS];

  npts = length + 1;
  for (i = 0 ; i < npts ; i++)
    {
      filter[i] = weights[length - i];
      filter[i + length] = weights[i];
    }


  /*  Fill borders with original data.    */

  for (i = 0 ; i < npts - 1 ; i++) outdat[i] = indat[i];


  for (i = width - npts ; i < width ; i++) outdat[i] = indat[i];


  for (j = npts - 1 ; j < width - npts ; j++)
    {
      outdat[j] = 0.0;
      ii = 0;

      for (k = j - length ; k <= j + length ; k++)
        {
          outdat[j] += (filter[ii] * indat[k]);
          ii++;
        }
    }
}
