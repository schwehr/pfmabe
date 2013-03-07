
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
/*! <pre>
*                                                                           *
*       this is a general purpose 3 dimensional map projection              *
*       subroutine. the 3d allow the computation of coordinates             *
*       for realistic globe-type earth models which look so good            *
*       on modern graphics terminals. though the spherical model            *
*       is strictly not a map projection, but rather a conversion           *
*       from spherical to cartesian coordinates, it is presented            *
*       here as an alternative to map projection for those who              *
*       possess a device capable of 3d display.                             *
*              the z coordinate is the data value associated with           *
*       the x(longitude) and y(latitude). the inverse projection            *
*       function is included (entry unmap) to support digitizing            *
*       capability.                                                         *
*       the lats and lons may be in the following forms:                    *
*              'deg' real degrees                                           *
*              'min' real minutes                                           *
*              'd-m' real combined degrees and minutes like if you          *
*                     read the number 12040 (meaning 120 deg, 40 min)       *
*                     with a f6.0 format.                                   *
*       written by jerry landrum and bill breed, navoceano code8322         *
*       1985                                                                *
*       disclaimer: many of these projections have been used very little    *
*       van der grinton seems to have a little problem near 0 deg longitude.*
*                                                                           *
*              the following must be called in order to setup               *
*       call proj to set the projection number                              *
*       call pdrnge to specify the graphics device coordinate               *
*              range(projected coordinates will be scaled into this range)  *
*       call pmaplm to define the geographic area to be mapped              *
*              into the coordinate space of the graphics device.            *
*       call zscale to enter a z axis scale factor                          *
*              now you can project and unproject data                       *
*       call project to project an arrays of geographic data                *
*       call unproject to perform the inverse of the projection             *
*                                                                           *
*   Modified :      Jan C. Depner, 05/18/93, ported to C                    *
*                                                                           *
</pre>
****************************************************************************/

#include "rproj.hpp"

rproj::rproj (NV_INT32 projection, NV_FLOAT64 dum1, NV_FLOAT64 dum2, NV_FLOAT64 min_x_pix,
              NV_FLOAT64 max_x_pix, NV_FLOAT64 min_y_pix, NV_FLOAT64 max_y_pix,
              NV_FLOAT64 rlats, NV_FLOAT64 rlatn, NV_FLOAT64 rlonw, NV_FLOAT64 rlone, 
              NV_FLOAT64 *sf, NV_FLOAT64 *xsize, NV_FLOAT64 *ysize)
{
    /*  Call to initialize the proj common block with the type of   */
    /*  map projection and the translation and scaling factors to   */
    /*  be applied to the data during projection.                   */

    iproj = projection;

    rlnorg = dum1 * RADDEG;
    rltorg = dum2 * RADDEG;

    pdrnge (min_x_pix, max_x_pix, min_y_pix, max_y_pix);
    pmaplm (rlats, rlatn, rlonw, rlone, sf, xsize, ysize);
}



rproj::~rproj ()
{
}




NV_FLOAT64 rproj::dm (NV_FLOAT64 rdm)
{
    NV_INT32    deg;
    
    /*  Converts a lat or lon in deg and minutes into total minutes for */
    /*  example: 1030(10deg,30 min) becomes 630.0 minutes.              */

    deg = (NV_INT32) (rdm / 100.0);

    return ((NV_FLOAT64) (rdm - deg * 40.0));
}




NV_FLOAT64 rproj::udm (NV_FLOAT64 rmin)
{
    NV_INT32    deg, min;

    /*  Inverse of dm.                                                  */

    min = (NV_INT32) (fmod ((NV_FLOAT64) rmin, 60.0) + 0.5);
    deg = (NV_INT32) rmin / 60;
    return ((NV_FLOAT64) (deg * 100 + min));
}





/***************************************************************************/
/*! <pre>
*                                                                           *
*   Module Name:        latmp                                               *
*                                                                           *
*   Programmer:         Unknown                                             *
*                                                                           *
*   Date Written:       Unknown                                             *
*                                                                           *
*   Modified:           Jan C. Depner, ported to C.                         *
*                                                                           *
*   Date Modified:      August 1992                                         *
*                                                                           *
*   Purpose:            Converts latitude to meridional parts in the        *
*                       Mercator projection.                                *
*                                                                           *
*   Inputs:             lat                 -   latitude                    *
*                                                                           *
*   Outputs:            NV_FLOAT64          -   meridional parts            *
*                                                                           *
*   Method:             Bowditch                                            *
*
</pre>                                                                           *
****************************************************************************/

NV_FLOAT64 rproj::latmp (NV_FLOAT64 lat)
{
    NV_FLOAT64  y, t, ym, answer;
    
    if (lat >= 90.0 * 60.0)
    {
        fprintf (stderr, "Latitude out of range in latmp, returning 0.0\n");
        fflush (stderr);
        return (0.0);
    }
    
    y = fabs (lat) * .000290888209;
    t = tan (0.7853981634 + y * 0.5);
    ym = 7915.7045 * log10 (t) - 23.268932 * sin (y);
    answer = ym * SIGN (lat);

    return (answer);
}

/***************************************************************************/
/*! <pre>
*                                                                           *
*   Module Name:        mplat                                               *
*                                                                           *
*   Programmer:         Unknown                                             *
*                                                                           *
*   Date Written:       Unknown                                             *
*                                                                           *
*   Modified:           Jan C. Depner, ported to C.                         *
*                                                                           *
*   Date Modified:      August 1992                                         *
*                                                                           *
*   Purpose:            Converts meridional parts to latitude in the        *
*                       Mercator projection.                                *
*                                                                           *
*   Inputs:             mp                  -   meridional parts            *
*                                                                           *
*   Outputs:            NV_FLOAT64          -   latitude                    *
*                                                                           *
*   Method:             Bowditch                                            *
*                                                                           *
</pre>
****************************************************************************/

NV_FLOAT64 rproj::mplat (NV_FLOAT64 mp)
{
    NV_FLOAT64  amp, blat, bl, bmp, answer;
    
    amp = fabs (mp);
    blat = 0;
    do
    {
        bl = blat;
        bmp = amp + 23.268932 * sin (bl) + 0.0525 * pow (sin(bl), 3.0);
        blat = 2.0 * (atan (pow (10.0, (bmp / 7915.704468))) - atan (1.));
    } while (fabs (blat - bl) > 0.00029);

    answer = SIGN (mp) * blat * 3437.74677;

    return (answer);
}


void rproj::preset ()
{
    /*  Resets translations and scale performed during projection.  */

    tx = 0.0;
    ty = 0.0;
    tz = 0.0;
    sx = 1.0;
    sy = 1.0;

    /*  Spherical.                                                  */

    if (iproj == 1) tz = 20000.0;
}




void rproj::xtran (NV_FLOAT64 t)
{
    /*  Modify the x parameter.                                     */

    tx = t;
}



void rproj::ytran (NV_FLOAT64 t)
{
    /*  Modify the y parameter.                                     */

    ty = t;
}



void rproj::ztran (NV_FLOAT64 t)
{
    /*  Modify the z parameter.                                     */

    tz = t;
}




void rproj::xscale (NV_FLOAT64 t)
{
    sx = t;
}




void rproj::yscale (NV_FLOAT64 t)
{ 
    sy = t;
}




void rproj::zscale (NV_FLOAT64 t)
{
    sz = t;
}





void rproj::project (NV_INT32 n, NV_INT32 unit, NV_FLOAT64 rlonar[], NV_FLOAT64 rlatar[], 
                     NV_FLOAT64 rdatar[], NV_FLOAT64 rx[], NV_FLOAT64 ry[], NV_FLOAT64 rz[])
{
    NV_FLOAT64      a, asq, b, g, q, r, rcolat, rlat, rlon, theta, x, y,
                    p, psq;
    NV_INT32        i;

    /*  n       -   number of points                                */
    /*  unit    -   0 = deg, 1 = min, 2 = d-m units of lats and lons  */
    /*  rlonar  -   array of longitudes                             */
    /*  rlatar  -   array of latitudes                              */
    /*  rdatar  -   array of data values                            */
    /*  pltary  -   array of plotting coordinates returned (3 X 1)  */

    if (unit < 0 || unit > 2)
      {
        fprintf (stderr, 
                 "\n\nInvalid units for lat and lon in function project : %d\n\n",
                 unit);
        exit (0);
      }

    for (i = 0 ; i < n ; i++)
      {
        switch (unit)
          {
          case RPROJ_MIN:
            rlat = rlatar[i] * RADMIN;
            rlon = rlonar[i] * RADMIN;
            break;

          case RPROJ_D_M:
            rlat = dm ((NV_FLOAT64) rlatar[i]) * RADMIN;
            rlon = dm ((NV_FLOAT64) rlonar[i]) * RADMIN;
            break;

          case RPROJ_DEG:
          default:
            rlat = rlatar[i] * RADDEG;
            rlon = rlonar[i] * RADDEG;
            break;
          }

        if (iproj == 1)
        {
            /*  Compute xyz for sphere.                                 */

            rcolat = HALFPI - rlat;
            r = rdatar[i] * sz + tz;
            rx[i] = ( - r * sin (rcolat) * sin (rlon));
            ry[i] = (r * cos (rcolat));
            rz[i] = (r * sin (rcolat) * cos (rlon));
        }
        else
        {
            switch (iproj)
            {
                case 2 :    /*  Cylindrical equal-area.                     */

                    x = RE * rlon;
                    y = RE * sin (rlat);
                    break;

                case 3 :    /*  Cylindrical equidistant (plate carree).     */

                    x = RE * rlon;
                    y = RE * rlat;
                    break;

                case 4 :    /*  Braun perspective cylindrical.              */

                    x = RE * rlon;
                    y = RE * (2 * tan (rlat / 2.0));
                    break;

                case 5 :    /*  Gall stereographic (lon(0)=45).             */

                    x = RE * (sqrt (2.0) / 2.0) * rlon;
                    y = RE * (sqrt (2.0) + 2.0) / 2.0 * tan (rlat / 2.0);
                    break;

                case 6 :    /*  Miller cylindrical, version a.              */

                    x = RE * rlon;
                    y = RE * 3.0 / 2.0 * log (tan (QUATPI + rlat / 3.0));
                    break;

                case 7 :    /*  Pseudocylindrical equal-area (fournier 2).  */

                    x = RE * rlon * cos (rlat) / sqrt (PI);
                    y = RE * sqrt (PI) / 2.0 * sin (rlat);
                    break;

                case 8 :    /*  Sinusoidal proj'n (sanson ouble).       */

                    x = RE * rlon * cos (rlat);
                    y = RE * rlat;
                    break;

                case 9 :   /*  Pseudocylindrical (winkel 1).               */

                    x = RE * rlon / 2.0 * (1.0 + cos (rlat));
                    y = RE * rlat;
                    break;

                case 10 :   /*  Van der grinten.                            */

                    rlat = rlat - rltorg;
                    rlon = rlon - rlnorg;
                    theta = asin (fabs (2.0 * rlat / PI));

                    if (fabs (rlat) < 0.01)
                    {
                        y = rlat * RE;
                        x = RE * rlon;
                    }
                    else if (fabs (rlon) < 0.01)
                    {
                        x = rlon * RE;
                        y = PI * RE * tan (theta / 2.0);
                        y *= SIGN (rlat);
                    }
                    else
                    {
                        a = 0.5 * fabs (PI / rlon - rlon / PI);
                        g = cos (theta) / (sin (theta) + cos (theta) - 1.0);
                        p = g * (2.0 / sin (theta) - 1.0);
                        q = a * a + g;

                        x = PI * RE * pow ((a * (g - p * p) +
                            (a * a * (g - p * p) * (g - p * p) - (p * p + a * a) *
                            (g * g - p * p))), 0.5) / (p * p + a * a);
                        x *= SIGN (rlon);
                        psq = p * p;
                        asq = a * a;
 
                        y = PI * RE * (p * q - a *
                            pow (((asq + 1.0) * (psq + asq) - q * q), 0.5)) /
                            (psq + asq);
                        y *= SIGN (rlat);
                    }
                    break;

                case 11 :   /*  Mercator from mailing.                      */

                    x = RE * rlon;
                    y = RE * log (tan (QUATPI + rlat / 2.0));
                    break;

                case 12 :   /*  A narrow projection.                        */

                    x = RE * rlon / 4.0;
                    y = RE * rlat;
                    break;

                case 13 :   /*  A wide projection.                          */

                    x = RE * rlon;
                    y = RE * rlat / 4.0;
                    break;

                case 14 :   /*  Stereographic.                              */

                    x = RE * cos (rlat) * sin (rlon - rlnorg);
                    y = RE * (cos (rltorg) * sin (rlat) - sin (rltorg) *
                        cos (rlat) * cos (rlon - rlnorg));
                    break;

                case 15 :   /*  Gnomonic.                                   */

                    r = 0.1 * tan (HALFPI - rlat);
                    theta = rlon;
                    x = RE * r * sin (theta);
                    y = RE * r * cos (theta);
                    break;

                case 16 :   /*  Postel.                                     */

                    r = (HALFPI - rlat);
                    theta = rlon;
                    x = RE * r * sin (theta);
                    y = RE * r * cos (theta);
                    break;

                case 17 :   /*  Lambert.                                    */

                    r = 2.0 * sin ((HALFPI - rlat) / 2.0);
                    theta = rlon;
                    x = RE * r * sin (theta);
                    y = RE * r * cos (theta);
                    break;

                case 18 :   /*  Orthographic.                               */

                    r = sin (HALFPI - rlat);
                    theta = rlon;
                    x = RE * r * sin (theta);
                    y = RE * r * cos (theta);
                    break;

                case 19 :   /*  Transverse mercator.                        */

                    b = cos (rlat) * sin (rlon - rlnorg);
                    x = RE * (0.5 * log ((1.0 + b) / (1.0 - b)));
                    y = RE * (atan (tan (rlat) / cos (rlon - rlnorg)) - rltorg);
                    break;

                case 0 :
                default :   /*  Corrected mercator.                         */

                    x = RE * rlon;
                    y = RE * latmp (rlat * MINRAD) * RADMIN;
                    break;
            }
 
            /*  Apply offset and scale.                                     */

            rx[i] = x * sx + tx;
            ry[i] = y * sy + ty;
            rz[i] = rdatar[i] * (- sz) + tz;

            if (invert_x && sx != 1.0) rx[i] = dxleft + (dxrigh - rx[i]);
            if (invert_y && sy != 1.0) ry[i] = dybot + (dytop - ry[i]);
        }
    }
}




void rproj::unproject (NV_INT32 n, NV_INT32 unit, NV_FLOAT64 rlonar[], NV_FLOAT64 rlatar[], 
                       NV_FLOAT64 rdatar[], NV_FLOAT64 rx[], NV_FLOAT64 ry[], NV_FLOAT64 rz[])
{
    NV_FLOAT64      a1, c, c1, c2, c3, d, r, rcolat, rlat, rlon, theta,
                    theta1, ttt, x, y, z, xsq, ysq;
    NV_INT32        i, m1;
    
    /*  n       -   number of points                                */
    /*  unit    -   0 = deg, 1 = min, 2 = d-m units of lats and lons  */
    /*  rlonar  -   array of longitudes                             */
    /*  rlatar  -   array of latitudes                              */
    /*  rdatar  -   array of data values                            */
    /*  pltary  -   array of plotting coordinates returned (3 X 1)  */


    if (unit < 0 || unit > 2)
    {
        fprintf (stderr, 
            "\n\nInvalid units for lat and lon in function unproject : %d\n\n",
            unit);
        exit (0);
    }

    for (i = 0 ; i < n ; i++)
    {
        x = rx[i];
        y = ry[i];
        z = rz[i];

        if (invert_x) x = dxleft + (dxrigh - x);
        if (invert_y) y = dybot + (dytop - y);

        if (iproj == 1)
        {
            /*  Spherical.                                                  */

            r = sqrt (x * x + y * y + z * z);
            rcolat = acos (y / r);
            rlat = HALFPI - rcolat;
            if (z == 0.0)
            {
                /*  Special case because atan is undefined.                 */

                if (y < 0.0) rlon = -PI;
                if (y > 0.0) rlon = PI;
            }
            else
            {
                rlon = - atan (x / z);

                /*  But since period of tan is 180 must check quadrant.     */

                if (z < 0.0)
                {
                    if (x < 0.0) rlon = rlon + PI;
                    if (x > 0.0) rlon = rlon - PI;
                }
            }
            rdatar[i] = (r - tz) / sz;
        }
        else
        {
            x = ((x - tx) / sx) / RE;
            y = ((y - ty) / sy) / RE;

            switch (iproj)
            {
                case 2 :    /*  Inverse of cylindrical equal-area.          */

                    rlon = x;
                    rlat = asin (y);
                    break;

                case 3 :    /*  Inverse of cylindrical equidistant.         */

                    rlon = x;
                    rlat = y;
                    break;

                case 4 :    /*  Inverse of braun perspective cylindrical.   */

                    rlon = x;
                    rlat = 2.0 * atan (y / 2.0);
                    break;

                case 5 :    /*  Inverse of gall stereographic.              */

                    rlon = 2.0 * x / sqrt (2.0);
                    rlat = 2.0 * atan (2.0 * y / ((2.0 + sqrt (2.0))));
                    break;

                case 6 :    /*  Inverse of miller cylindrical.              */

                    rlon = x;
                    rlat = (atan (exp (y * 2.0 / 3.0)) - QUATPI) * 3.0;

                    if (x == 0.0 && y == 0.0)
                    {
                        rlon = rlnorg;
                        rlat = 0.0;
                    }
                    break;

                case 7 :    /*  Inverse of pseudocylindrical equal area.    */

                    rlat = asin (2.0 * y / sqrt (PI));
                    rlon = x * sqrt (PI) / cos (rlat);
                    break;

                case 8 :    /*  Inverse of sinusoidal proj'n.               */

                    rlat = y;
                    rlon = x / cos (rlat);
                    break;

                case 9 :   /*  Inverse of winkel 1.                        */

                    rlat = y;
                    rlon = 2.0 * x / (1.0 + cos (rlat));
                    break;

                case 10 :   /*  Inverse of van der grinten.                 */

                    if (x == 0.0 && y == 0.0)
                    {
                        rlon = rlnorg;
                        rlat = 0.0;
                    }
                    else
                    {
                        x = x / PI;
                        y = y / PI;
                        xsq = x * x;
                        ysq = y * y;
                        c1 = - fabs (y) * (1.0 + xsq + ysq);
                        c2 = c1 - 2.0 * ysq + xsq;
                        c3 = -2.0 * c1 + 1.0 + 2.0 * ysq + (xsq + ysq) *
                            (xsq + ysq);
                        d = ysq / c3 + (2.0 * (c2 * c2 * c2) / (c3 * c3 * c3) -
                            9.0 * c1 * c2 / (c3 * c3)) / 27.0;
                        a1 = (c1 - c2 * c2 / (3.0 * c3)) / c3;
                        m1 = (NV_INT32) (2.0 * pow (- a1 / 3.0, 0.5));

                        /*  Since the vax doesn't like to do acos(-1).      */

                        ttt = 3.0 * d / (a1 * m1);
                        if (ttt < - 0.99999)
                        {
                            theta1 = 0.333333 * (- PI);
                        }
                        else if (ttt > 0.99999)
                        {
                            theta1 = 0.333333 * (PI);
                        }
                        else
                        {
                            theta1 = 0.3333333 * acos (ttt);
                        }

                        rlat = PI * (- m1 * cos (theta1 + PI / 3.0) -
                            c2 / (3.0 * c3));
                        rlat *= SIGN (y);
                        if (x == 0.0)
                        {
                            rlon = rlnorg;
                        }
                        else
                        {
                            rlon = PI * (xsq + ysq - 1.0 + 
                                pow (1.0 + 2.0 * (xsq - ysq) + (xsq + ysq) *
                                (xsq + ysq), 0.5)) / (2.0 * x) + rlnorg;
                        }
                    }
                    break;

                case 11 :   /*  Inverse of mercator from mailing.           */

                    rlon = x;
                    rlat = 2.0 * (atan (exp (y)) - QUATPI);
                    break;

                case 12 :   /*  Inverse of narrow.                          */

                    rlon = x * 4.0;
                    rlat = y;
                    break;

                case 13 :   /*  Inverse of wide.                            */

                    rlon = x;
                    rlat = y * 4.0;
                    break;

                case 14 :   /*  Inverse of stereographic.                   */

                    r = sqrt (x * x + y * y);
                    c = 2.0 * atan2 (r , 2.0);
                    if (r == 0.0)
                    {
                        rlat = rltorg;
                    }
                    else
                    {
                        rlat = asin (cos (c) * sin (rltorg) + y * sin (c) *
                            cos (rltorg / r));
                    }

                    if (rltorg == HALFPI)
                    {
                        rlon = rlnorg + atan2 (x, -y);
                    }
                    else if (rltorg == - HALFPI)
                    {
                        rlon = rlnorg + atan2 (x, y);
                    }
                    else
                    {
                        rlon = rlnorg + atan2 (x * sin (c), r * cos (rltorg) *
                            cos (c) - y * sin (rltorg) * sin (c));
                    }
                    break;

                case 15 :   /*  Inverse of gnomonic.                        */

                    theta = atan (x / y);
                    if (fabs (theta) > QUATPI)
                    {
                        r = 10.0 * x / sin (theta);
                    }
                    else
                    {
                        r = 10.0 * y / cos (theta);
                    }
                    
                    rlon = theta;
                    rlat = HALFPI - atan (r);
                    break;

                case 16 :   /*  Inverse of postel.                          */

                    theta = atan (x / y);
                    if (fabs (theta) > QUATPI)
                    {
                        r = x / sin (theta);
                    }
                    else
                    {
                        r = y / cos (theta);
                    }

                    rlon = theta;
                    rlat = HALFPI - r;
                    break;

                case 17 :   /*  Inverse of lambert.                         */

                    theta = atan (x / y);
                    if (fabs (theta) > QUATPI)
                    {
                        r = x / sin (theta);
                    }
                    else
                    {
                        r = y / cos (theta);
                    }

                    rlon = theta;
                    rlat = HALFPI - asin (r / 2.0) * 2.0;
                    break;

                case 18 :   /*  Inverse of orthographic.                    */

                    theta = atan (x / y);
                    if (fabs (theta) > QUATPI)
                    {
                        r = x / sin (theta);
                    }
                    else
                    {
                        r = y / cos (theta);
                    }

                    rlon = theta;
                    rlat = HALFPI - asin (r);
                    break;

                case 19 :   /*  Inverse of transverse mercator.             */

                    d = y + rltorg;
                    rlat = asin (sin (d) / cosh (x));
                    rlon = rlnorg + atan (sinh (x) / cos (d));
                    break;

                case 0 :
                default :   /*  Corrected merc.                             */

                    rlon = x;
                    rlat = mplat (y * MINRAD) * RADMIN;
                    break;
            }
 
            switch (unit)
              {
              case RPROJ_MIN:
                rlatar[i] = rlat * MINRAD;
                rlonar[i] = rlon * MINRAD;
                break;

              case RPROJ_D_M:
                rlatar[i] = udm ((NV_FLOAT64) rlat * MINRAD);
                rlonar[i] = udm ((NV_FLOAT64) rlon * MINRAD);
                break;

              case RPROJ_DEG:
              default:
                rlatar[i] = rlat * DEGRAD;
                rlonar[i] = rlon * DEGRAD;
                break;
              }
        }
    }
}




void rproj::pmaplm (NV_FLOAT64 rlats, NV_FLOAT64 rlatn, NV_FLOAT64 rlonw, NV_FLOAT64 rlone, 
                    NV_FLOAT64 *sf, NV_FLOAT64 *xsize, NV_FLOAT64 *ysize)
{
    NV_FLOAT64      dlat, dlon, xmax, xmin, ymax, ymin;
    NV_FLOAT64      rlat, rlon, rdat, rx, ry, rz;
    NV_INT32        i, j, lck;

    /*  Call to define the geographic area to be mapped into the device     */
    /*  coordinate range.                                                   */
    /*  Parameters are real degrees south,north,west,east.                  */

    /*  Center and scale data to the number space.                          */

    preset ();

    /*  Resets trans and scale factors in proj.                             */

    xmax = -32000.0;
    xmin = 32000.0;
    ymax = -32000.0;
    ymin = 32000.0;

    /*  Sample 100 points to determine map extent.                          */

    lck = 10;
    dlat = (rlatn - rlats) / lck;
    dlon = (rlone - rlonw) / lck;
    for (i = 0 ; i <= lck ; i++)
    {
        rlat = rlats + i * dlat;
        for (j = 0 ; j <= lck ; j++)
        {
            rlon = rlonw + j * dlon;
            project (1, RPROJ_DEG, &rlon, &rlat, &rdat, &rx, &ry, &rz);
            xmax = MAX (rx, xmax);
            xmin = MIN (rx, xmin);
            ymax = MAX (ry, ymax);
            ymin = MIN (ry, ymin);
        }
    }

    if (scale == 0.0)
    {
        /*  Compute scale and offsets to spread the chart out over the      */
        /*  device coordinate range.                                        */

        sx = dxsize / (xmax - xmin);
        sy = dysize / (ymax - ymin);

        /*  Scale factor needed to scale to y dev.                          */

        *sf = MIN (sx, sy);

        /*  Choose the smaller scale for both to maintain the aspect ratio. */
    }
    else
    {
        *sf = scale;
    }

    tx = dxleft - (xmin * *sf);
    ty = dybot - (ymin * *sf);
 
    *xsize = (xmax - xmin) * *sf;
    *ysize = (ymax - ymin) * *sf;

    xtran (tx);
    ytran (ty);
    xscale (*sf);
    yscale (*sf);
}






void rproj::pdrnge (NV_FLOAT64 dum1, NV_FLOAT64 dum2, NV_FLOAT64 dum3, NV_FLOAT64 dum4)
{
    /*  Call to provide the device coordinate range.                    */
    /*  This allow this projection package to be device independent.    */
    /*  The parameters represent the number range into which the        */
    /*  geographic map limits will be mapped.                           */

    /*  Save input device range.                                        */

    if (dum2 < dum1)
    {
        invert_x = 1;
        dxleft = dum2;
        dxrigh = dum1;
    }
    else
    {
        invert_x = 0;
        dxleft = dum1;
        dxrigh = dum2;
    }

    if (dum4 < dum3)
    {
        invert_y = 1;
        dybot = dum4;
        dytop = dum3;
    }
    else
    {
        invert_y = 0;
        dybot = dum3;
        dytop = dum4;
    }
    
    dxsize = dxrigh - dxleft;
    dysize = dytop - dybot;
    scale = 0;
}




void rproj::pscale (NV_FLOAT64 dummy)
{
    /*  Dummy is the scale factor in plotting device units per degree.  */

    scale = dummy * DEGRAD;

    /*  Scale is the desired scale factor in plotting device units per  */
    /*  radian.                                                         */
}
