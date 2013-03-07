
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



/*  Rproj class definitions.  */

#ifndef RPROJ_H
#define RPROJ_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cmath>


#include "nvtypes.h"
#include "nvdef.h"
#include "cnstnts.h"


#define     DEGRAD          180.0 / PI
#define     HALFPI          PI / 2.0
#define     QUATPI          PI / 4.0
#define     RADMIN          PI / 10800.0
#define     MINRAD          10800.0 / PI
#define     RE              1.0


#define     CORRECTED_MERCATOR                                       0
#define     UNDEFINED                                                1
#define     CYLINDRICAL_EQUAL_AREA                                   2
#define     CYLINDRICAL_EQUIDISTANT                                  3
#define     BRAUN_PERSPECTIVE_CYLINDRICAL                            4
#define     GALL_STEREOGRAPHIC                                       5
#define     MILLER_CYLINDRICAL                                       6
#define     PSEUDOCYLINDRICAL_EQUAL_AREA                             7
#define     SINUSOIDAL                                               8
#define     PSEUDOCYLINDRICAL                                        9
#define     VAN_DER_GRINTEN                                          10
#define     MERCATOR_FROM_MAILING                                    11
#define     NARROW                                                   12
#define     WIDE                                                     13
#define     STEREOGRAPHIC                                            14
#define     GNOMONIC                                                 15
#define     POSTEL                                                   16
#define     LAMBERT                                                  17
#define     ORTHOGRAPHIC                                             18
#define     TRANSVERSE_MERCATOR                                      19

#define     RPROJ_DEG                                                0
#define     RPROJ_MIN                                                1
#define     RPROJ_D_M                                                2



class rproj
{
  public:

    rproj (NV_INT32 projection, NV_FLOAT64 dum1, NV_FLOAT64 dum2, NV_FLOAT64 min_x_pix,
           NV_FLOAT64 max_x_pix, NV_FLOAT64 min_y_pix, NV_FLOAT64 max_y_pix,
           NV_FLOAT64 rlats, NV_FLOAT64 rlatn, NV_FLOAT64 rlonw, NV_FLOAT64 rlone, 
           NV_FLOAT64 *sf, NV_FLOAT64 *xsize, NV_FLOAT64 *ysize);
    ~rproj ();

    NV_FLOAT64 dm (NV_FLOAT64 rdm);
    NV_FLOAT64 udm (NV_FLOAT64 rmin);
    NV_FLOAT64 latmp (NV_FLOAT64 lat);
    NV_FLOAT64 mplat (NV_FLOAT64 mp);
    void preset ();
    void xtran (NV_FLOAT64 t);
    void ytran (NV_FLOAT64 t);
    void ztran (NV_FLOAT64 t);
    void xscale (NV_FLOAT64 t);
    void yscale (NV_FLOAT64 t);
    void zscale (NV_FLOAT64 t);
    void project (NV_INT32 n, NV_INT32 unit, NV_FLOAT64 rlonar[], NV_FLOAT64 rlatar[], 
                  NV_FLOAT64 rdatar[], NV_FLOAT64 rx[], NV_FLOAT64 ry[], NV_FLOAT64 rz[]);
    void unproject (NV_INT32 n, NV_INT32 unit, NV_FLOAT64 rlonar[], NV_FLOAT64 rlatar[], 
                    NV_FLOAT64 rdatar[], NV_FLOAT64 rx[], NV_FLOAT64 ry[], NV_FLOAT64 rz[]);
    void pscale (NV_FLOAT64 dummy);


  protected:

    void pmaplm (NV_FLOAT64 rlats, NV_FLOAT64 rlatn, NV_FLOAT64 rlonw, NV_FLOAT64 rlone, 
                 NV_FLOAT64 *sf, NV_FLOAT64 *xsize, NV_FLOAT64 *ysize);
    void pdrnge (NV_FLOAT64 dum1, NV_FLOAT64 dum2, NV_FLOAT64 dum3, NV_FLOAT64 dum4);


    NV_FLOAT64      rlnorg, rltorg, sx, sy, sz, tx, ty, tz, scale, dxleft, dxrigh,
                    dybot, dytop, dxsize, dysize;
    NV_INT32        iproj, invert_y, invert_x;
};

#endif
