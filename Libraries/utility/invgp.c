
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



#include "invgp.h"

/***************************************************************************/
/*!

  - Module Name:        invgp

  - Programmer:         Unknown

  - Date Written:       When dinosaurs roamed the earth.

  - Modified:           Jan C. Depner, converted to C and changed the
                       arguments to degrees instead of radians.

  - Date:               September, 1992

  - Purpose:            Given the semi-major axis, semi-minor axis, and
                       two geographic positions this routine will compute
                       the distance between the two gp's and the azimuth
                       (clockwise from north) from the first gp to the
                       second gp.

  - Inputs:
                       - a0                  =   semi-major axis in meters
                       - b0                  =   semi-minor axis in meters
                       - rlat1               =   start latitude in degrees
                       - rlon1               =   start longitude in degrees
                       - rlat2               =   end latitude in degrees
                       - rlon2               =   end longitude in degrees
                       - dist                =   distance in meters
                       - az                  =   azimuth in degrees

  - Outputs:            none

****************************************************************************/

void invgp (NV_FLOAT64 a0, NV_FLOAT64 b0, NV_FLOAT64 rlat1, NV_FLOAT64 rlon1, NV_FLOAT64 rlat2,
NV_FLOAT64 rlon2, NV_FLOAT64 *dist, NV_FLOAT64 *az)
{
    NV_FLOAT64          drlat1, drlat2, drlon1, drlon2, dell, beta1, sbeta1,
                        cbeta1, beta2, sbeta2, cbeta2, adell, sidel, codel, a,
                        b, siphi, cophi, q1, q2, c, em, phi, phisq, csphi,
                        ctphi, psyco, term1, term2, term3, term4, term5, term6,
                        xlam1, tan;
    NV_INT32            n;
    static NV_INT32     first = 1;
    static NV_FLOAT64   pi = 3.141592653589793, twopi = 6.283185307179586,
                        rad_to_deg = 57.2957795147195,
                        tiny = .0000000000000000000000000000001, flat, flat2, f1,
                        f2, f3, f4, f5, f6, f7, f8;

    if (first)
    {
        flat = 1.0l - (b0 / a0);
        flat2 = flat * flat;
        f1 = flat2 * 1.25;
        f2 = flat2 * 0.5;
        f3 = flat2 * 0.25;
        f4 = flat2 * 0.125;
        f5 = flat2 * 0.0625;
        f6 = flat + flat2;
        f7 = f6 + 1.0;
        f8 = f6 * 0.5;

        first = 0;
    }

    drlat1 = rlat1 / rad_to_deg;
    drlat2 = rlat2 / rad_to_deg;
    drlon1 = rlon1 / rad_to_deg;
    drlon2 = rlon2 / rad_to_deg;

    beta1 = atan ((1.0 - flat) * sin (drlat1) / cos (drlat1));
    sbeta1 = sin (beta1);
    cbeta1 = cos (beta1);
    beta2 = atan ((1.0 - flat) * sin (drlat2) / cos (drlat2));
    sbeta2 = sin (beta2);
    cbeta2 = cos (beta2);

    dell = drlon1 - drlon2;
    adell = fabs (dell);

    if (drlon1 * drlon2 < 0.0)
    {
        adell = fabs (drlon1) + fabs (drlon2);
        dell = adell;
        if (drlon1 < 0.0) dell = - adell;

        if (adell > pi)
        {
            adell = twopi - adell;
            dell = adell;
            if (drlon1 > 0.0) dell = - adell;
        }
    }

    adell = twopi - adell;
    sidel = sin (adell);
    codel = cos (adell);
    a = sbeta1 * sbeta2;
    b = cbeta1 * cbeta2;
    cophi = a + b * codel;
    q1 = sidel * cbeta2;
    q1 *= q1;
    q2 = sbeta2 * cbeta1 - sbeta1 * cbeta2 * codel;
    q2 *= q2;
    siphi = sqrt (q1 + q2);
    c = b * sidel / siphi;
    em = 1.0 - c * c;

    phi = atan (siphi / (sqrt (1.0 - siphi * siphi) + tiny));

    if (cophi < 0.0) phi = pi - phi;
    phisq = phi * phi;
    csphi = 1.0 / siphi;
    ctphi = cophi / siphi;
    psyco = siphi / cophi;

    /*  Compute distance.                                               */

    term1 = f7 * phi;
    term2 = a * (f6 * siphi - f2 * phisq * csphi);
    term3 = em * (f2 * phisq * ctphi - f8 * (phi + psyco));
    term4 = a * a * f2 * psyco;
    term5 = em * em * (f5 * (phi + psyco) - f2 * phisq * ctphi - f4 * psyco *
        cophi * cophi);
    term6 = a * em * f2 * (phisq * csphi + psyco * cophi);
    *dist = b0 * (term1 + term2 + term3 - term4 + term5 + term6);

    /*  Compute azimuth.                                                */

    term1 = f6 * phi;
    term2 = a * (f2 * siphi + flat2 * phisq * csphi);
    term3 = em * (f3 * psyco + flat2 * phisq * ctphi - f1 * phi);
    xlam1 = c * (term1 - term2 + term3) + adell;
    q1 = sbeta2 * cbeta1 - cos (xlam1) * sbeta1 * cbeta2;
    q2 = sin (xlam1) * cbeta2;
    if (q1 == 0.0) q1 = tiny;
    tan = q2 / q1;
    *az = atan (tan);

    /*  Put azimuth in proper quadrant.                                 */

    n = 3;
    if (dell * tan < 0.0) n = 4;
    if (dell < 0.0) n = n - 2;
    if (q1 > 0.0 && dell == 0.0) n = 1;
    q2 = n;
    q1 = q2 * pi - pi - *az;
    if (n >= 3) q1 = (q2 - 2.0) * pi + *az;
    *az = q1 * rad_to_deg;
}
