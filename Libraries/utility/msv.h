
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



/*! <pre>
        msv.h
	purpose:  prototypes for functions
		to produce the mean sounding velocity (msv) - the
		quotient of the distance (z) a sound wave travels in the
		vertical and the length of the associated travel time
		interval (t), i.e.,
			msv = z/t
		which is the harmonic mean sound speed

	ref:  geoge a. maul, mean sounding velocity, a brief review,
		the international hydrographic review, vol 47(2),
		pp. 85-92, Jul 1970.
		and
		matlab code and conversations with christian
		deMoustier 29mar2002.

	author:  david h fabre, u.s. naval oceanographic office, 1apr2002
</pre>*/

#ifndef __MSV_H__
#define __MSV_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <math.h>
#include "nvtypes.h"

typedef struct
{
  NV_INT32       n;
  NV_FLOAT64     *z;
  NV_FLOAT64     *c;
  NV_FLOAT64     *t;
  NV_FLOAT64     *h;
} MSV;

NV_INT32 prephmc (NV_INT32 nin, NV_FLOAT64 *zin, NV_FLOAT64 *cin, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h);
void makhmc (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h);
NV_FLOAT64 msv (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h, NV_FLOAT64 z1);

#if defined (ARITHMETIC_VS_HARMONIC)

void makamc (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *a);
NV_FLOAT64 amsv (NV_INT32 n, NV_FLOAT64 *z, NV_FLOAT64 *c, NV_FLOAT64 *t, NV_FLOAT64 *h, NV_FLOAT64 z1);

#endif

#include "sspfilt.h"
#include "linterp.h"

#define DEEP 12000 /* dhf - deepest depth in ocean */


#ifdef  __cplusplus
}
#endif

#endif
