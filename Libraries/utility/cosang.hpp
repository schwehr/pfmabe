
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



#ifndef _COSANG_H
#define _COSANG_H
#include "nvtypes.h"

/*

   - cosang.h

   - purpose:  prototype for the functions that determine
               the cosine of the angle between a surface
               (3 points on earth) and the sun (azimuth, elevation).
               also define the struct point.
   - notes: 
         - (0) pt[0] is the principal point on our surface 
         - (1) the coordinates should be in the same units, meters perhaps 
         - (2) for sun shading a grid which is oriented north to the top 
               - x is a cartesian value which is negative west 
               - y is a cartesian value which is negative south 
               - z is a cartesian value which is negative below the water 
         - (3) note that the function cosang returns negative values 
               if the surface is not sun shadeable (-1 on bomb out). 
         - (4) since the sun unit normal vector is the same for a given
               azimuth and elevation angle, the sun_unv function is
               provided and can be called once before the calls to
               cosang.
   
   - author:  dave fabre (sep 1997)
*/

typedef struct
{
  NV_F64_COORD3 sun;
  NV_FLOAT64  azimuth;
  NV_FLOAT64  elevation;
  NV_FLOAT64  exag;
  NV_FLOAT64  power_cos;
  NV_INT32    num_shades;
  NV_FLOAT32  min_shade;
} SUN_OPT;


NV_F64_COORD3 sun_unv (NV_FLOAT64 azi, NV_FLOAT64 elev);
NV_FLOAT64 cosang (NV_F64_COORD3 *pt, NV_F64_COORD3 sun);

#endif /* _COSANG_H */
