
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



#include "sspfilt.h"

/******************************************************************************/

/*!
  - sspfilt.c

  - purpose:  given a sound speed profile filter out
              the duplicate depths.  This may be important
              to the raytracing (?) but it will also cut down the
              size of a LUT.

  - author:  dave fabre, neptune sciences, inc., 1998-1999
*/

int nsspfilt (int nin, double *zin, double *cin __attribute__ ((unused)))
{
  int i, j = 1;

  if (nin <= 0) return (0);

  for (i = 1 ; i < nin ; i++)
    {
      if (zin[i] != zin[i - 1])
        {
          ++j;
        }
    } /* for i */

  return j;
} /* nsspfilt */



/******************************************************************************/

int sspfilt (int nin, double *zin, double *cin, double *zout, double *cout)
{
  int i, j = 0;

  zout[j] = zin[0];
  cout[j++] = cin[0];		

  for (i = 1 ; i < nin ; i++)
    {
      if (zin[i] != zin[i - 1])
        {
          zout[j] = zin[i];
          cout[j++] = cin[i];
        }
    } /* for i */

  return j;
} /* sspfilt */
