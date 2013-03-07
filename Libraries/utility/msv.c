
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

  - msv.c

  - purpose:  to produce the mean sounding velocity (msv) - the
              quotient of the distance (z) a sound wave travels in the
              vertical and the length of the associated travel time
              interval (t), i.e.,
                  - msv = z/t
              which is the harmonic mean sound speed

  - ref:
              - george a. maul, mean sounding velocity, a brief review,
                the international hydrographic review, vol 47(2),
                pp. 85-92, Jul 1970.
              - discussions with kevin gustafson of simrad, wa (mar2002)
              - matlab code and conversations with
                christian deMoustier 29mar2002.


  - author:  david h fabre, u.s. naval oceanographic office, mar-apr2002

*/

#include "msv.h"
#include "linterp.h" /* get proto. for ptslope */

/******************************************************************************/

NV_INT32 prephmc (NV_INT32 nin, NV_FLOAT64 *zin, NV_FLOAT64 *cin, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h)
{
  NV_INT32 n;

  if ((n = sspfilt (nin, zin, cin, z, c)) < 2)
    {
      //fprintf (stderr, "prephmc:  problem with second filter\n");
      return (-1);
    }


  /* from surface */

  if (z[0] > 0)
    {
      z[0] = 0;
    }


  /* go full ocean depth */

  if (z[n - 1] < DEEP)
    {
      c[n - 1] = ptslope (z[n - 2], c[n - 2], z[n - 1], c[n - 1], DEEP);
      z[n - 1] = DEEP;
    }

  makhmc (n, z, c, t, h);

  return (n);
} /* prephmc */



/******************************************************************************/

void makhmc (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h)
{
  NV_INT32 i;
  NV_FLOAT64 dz, gradient;

  t[0] = 0;
  h[0] = c[0];
  for (i = 0 ; i < n - 1 ; i++)
    {
      dz = z[i + 1] - z[i];
      gradient = (c[i + 1] - c[i]) / dz;
      if (gradient == 0) /* layer of constant sound speed */
        {
          t[i + 1] = t[i] + dz / c[i]; 
        }
      else /* consider sound speed linearly changing */
        {
          t[i + 1] = t[i] + log (c[i + 1] / c[i]) / gradient;
        }
      h[i + 1] = (z[i + 1] - z[0]) / t[i + 1];
    }
} /* makhmc */



/******************************************************************************/

NV_FLOAT64 msv (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h, NV_FLOAT64 z1)
{
  NV_INT32 i, j;
  NV_FLOAT64 dz, gradient, t1, c1;

  if (z1 < z[0] || z1 > z[n - 1])
    {
      //fprintf(stderr, "msv:  depth out of range\n");
      return (-1000000.0);
    }

  if (z1 == z[0]) return (c[0]);

  j = -1;
  for (i = 0 ; i < n - 1 ; i++)
    {
      if (z1 == z[i]) return (h[i]);

      if (z1 == z[i + 1]) return h[i + 1];

      if (z1 > z[i] && z1 < z[i + 1])
        {
          j = i;
          break;
        }
    }

  if (j < 0)
    { 
      //fprintf(stderr, "msv:  layer not found\n"); 
      return (-1000000.0);
    }

  dz = z1 - z[j];
  c1 = ptslope (z[j], c[j], z[j + 1], c[j + 1], z1);
  if (c1 == LINTERP_NODATA) 
    { 
      //fprintf(stderr, "msv:  c1=NODATA\n"); 
      return (-1000000.0); 
    }

  gradient = (c1 - c[j]) / dz;
  if (gradient == 0) /* layer of constant sound speed */
    {
      t1 = t[j] + dz / c[j];
    }
  else /* for a linearly changing sound speed b/w layers */
    {
      t1 = t[j] + log (c1 / c[j]) / gradient;
    }

  return (z1/t1);
} /* msv */



#if defined( ARITHMETIC_VS_HARMONIC )

/******************************************************************************/

void makamc (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *a)
{
  NV_INT32 i, j;
  NV_FLOAT64 dz, gradient;

  t[0] = 0;
  a[0] = c[0];
  for (i = 0 ; i < n - 1 ; i++)
    {
      dz = z[i + 1] - z[i];
      t[i + 1] = t[i] + dz / (c[i] + 0.5 * (c[i + 1] - c[i]));
      a[i + 1] = (z[i + 1] - z[0] ) / t[i + 1];
    }
} /* makamc */




/******************************************************************************/

NV_FLOAT64 amsv (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *a, NV_FLOAT64 z1)
{
  NV_INT32 i, j;
  NV_FLOAT64 dz, gradient, t1, c1;

  if (z1 < z[0] || z1 > z[n - 1])
    {
      return (-1000000.0);
    }

  if (z1 == z[0]) return (c[0]);

  j = -1;
  for (i = 0 ; i < n - 1 ; i++)
    {
      if (z1 == z[i]) return (a[i]);

      if (z1 == z[i + 1]) return (a[i + 1]);

      if (z1 > z[i] && z1 < z[i + 1])
        {
          j = i;
          break;
        }
    }

  if (j < 0) return (-1000000.0);
  dz = z1 - z[j];
  c1 = ptslope (z[j], c[j], z[j + 1], c[j + 1], z1);
  if (c1 == LINTERP_NODATA)
    {
      //printf("c1=NODATA\n");
      return (-1000000.0);
    }
  t1 = t[j] + dz / (c[j] + 0.5 * (c1 - c[j]));

  return (z1/t1);
} /* amsv */
#endif
