
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



#include "direct.h"

/* direct.c */
/******************************************************************************/
void direct(double phi, double alam, double fazi, double s,
		double *phipri, double *alampr)
/*! <pre>
c
c     ..................................................................
c
c	computes the geodetic position (latitude,longitude) and   
c	azimuth of an observed station from a station of known      
c	geodetic position, with azimuth and distance to the observed
c	station given. 
c
c	the spheroid of reference is the clarke spheroid of 1866.
c
c	evaluation is based on equations for the forward position   
c	computation developed by the u.s. coast and geodetic survey.
c	this method is valid for distances up to 600 miles.         
c
c	reference:
c	simmons,lansing g., natural tables for the calculation of   
c	geodetic positions, u.s. coast and geodetic survey special  
c	publication no.241, 1949.  
c
c	description of parameters
c	phi    - geodetic latitude of known station, 
c                in seconds of arc (input)
c	alam   - geodetic longitude of known station,
c                in seconds of arc (input)
c	fazi   - geodetic forward azimuth of known station,
c                in seconds of arc (input)
c	s      - geodetic distance, in meters (input)
c	phipri - geodetic latitude of observed station,  
c                in seconds of arc (output)
c	alampr - geodetic longitude of observed station,  
c                in seconds of arc (output)
c
c	authors:
c	original author   - rosemary e. riordan /noaa/nos/c54x2.
c	subroutine author - milton stein /noaa/nos/c542. 
c
c	modified by yann king ,12/30/86 to include datum option.
c
c	modified 10/21/87 by tom stepka.
c
c	moved to vax/vms and fortran 77.
c	there are now integers or do loops to cause f77 conversion problems.
c
c	put all horizontal datum constants into common block hdatum.
c	call routine hdinit to load these constants with the proper values.
c
c	the original verison of this routine contained a discrepency in
c	its definitions of the constants z2 and z4.  in gpxy and marc these
c	are called w1 and w3, respectively, and are defined having the
c	same magnitude but have the opposite sign.  therefore, in order to
c	keep all common block definitions consistent for this version, z2 and
c	z4 will change signs in this verison of direct.  fortunately they
c	are only used once, as arguments to the inline funcion angle, so
c	to keep direct running the same, i changed the signs of z2 and z4
c	in this function call.
c
c	modified by tim rulon 22-mar-89
c
c	corrected underflow problem when faz very close to multiple
c	of 90 degrees and s is small. did not cause a problem under
c	ordinary circumstances. caused floating point trap.
c
c     ..................................................................
c
	Converted to C by David Fabre, Planning Systems Inc., 15 Mar 94
	for NRL for NAVO

        Had to change the macro "angle" to "direct_angle" due to conflict with 
        Qt 4 libraries.  Jan Depner, 10/19/07

</pre>*/
{
	/* wgs 84 / nad 83 parameters taken from hdinit routine */
	static double axis = 6378137.0,
		esq = 6.69438002292318e-3,
		z2 = 0.1104825480468667,
		z3 = 21.25880271589918,
		z4 = 5048.250737106752,
		z1 = 6367449.145771138,
		w2 = 0.23832988623869,
		w3 = 29.36926254762117,
		w4 = 5022.894084128594,
		w1 = 0.1570487611454482e-6;
	/* local constant */
	static double arc1 = 0.484813681110e-5;
	/* local variables */
	double fazj, x, y, b, phj, sina, ef, xcor, xpri, a, ycor, ypri, cosa,
		cssq, y0, y1, omega, sin1, cos1, css1, phi1, faca, v, va, y2,
		facb, facc, cay, y3, omegb, sin2, cos2, css2, h, applam,
		asinco, dellam;

/****	determined these unnecessary DHF
	double delphi, phimid, applap;
****/

	/*computation of geodetic position of observed station */

	fazj = fazi*arc1;
	x = s * sin(fazj);
	if (fabs(x) < 1.0e-3) x = 0.001;
	y = -s * cos(fazj);
	if (fabs(y) < 1.0e-3) y = 0.001;
	b = pow(y/10000.0, 2.0);
	phj = phi*arc1;
	sina = sin(phj);
	ef = (pow(1.0 - esq*sina*sina, 2.0)*1.0e15)/(3.0*axis*axis*(1.0 - esq));
	xcor = b*ef/2.0;
	xpri = x - xcor*x*1.0e-7;  
	a = pow(xpri/10000.0, 2.0);  
	ycor = ef*a;
	ypri = y + ycor*y*1.0e-7;  
	cosa = cos(phj);
	cssq = cosa*cosa;
	y0 = z1*direct_angle(phj, sina, cosa, -z4, cssq, z3, -z2);
	y1 = y0 + ypri;
	omega = w1*y1;
	sin1 = sin(omega);
	cos1 = cos(omega);
	css1 = cos1*cos1;
	phi1 = direct_angle(omega, sin1, cos1, w4, css1, w3, w2);
	sin1 = sin(phi1);
	cos1 = cos(phi1);
	faca = (sqrt(1.0 - esq*sin1*sin1)) / (2.0*axis);
	v = sin1/cos1*faca*1.0e8;
	va = v*a;
	y2 = y1 - va;
	facb = (1.0 + 3.0*( (sin1*sin1)/(cos1*cos1) )) / (3.0*(sin1/cos1));
	facc = (3.0*esq*sin1*cos1) / (1.0 - esq);
	cay = (faca*(facb - facc))*1.0e6;
	y3 = y2 + cay*pow(va/1000.0, 2.0);
	omegb = w1*y3;
	sin2 = sin(omegb);
	cos2 = cos(omegb);
	css2 = cos2*cos2;
	*phipri = direct_angle(omegb, sin2, cos2, w4, css2, w3, w2);

/****	delphi is related only to the appalp and phimid assignments below DHF
	delphi = phj - *phipri;
****/
	h = sqrt(1.0 - esq*pow(sin(*phipri), 2.0)) / (axis*cos(*phipri)*arc1);
	applam = h*xpri;
	asinco = (v*va)/15.0;
	dellam = applam + applam*asinco*1.0e-7;
	*alampr = alam - dellam;

/****	the 2 assignments below are made for no apparent reason so
	I took them out, perhaps they were output at one time but
	not necessary here DHF
	appalp = dellam * (sin(phj) + sin(*phipri)) / (1.0 + cos(delphi));
	phimid = (phj + *phipri)/2.0e0;
****/

	*phipri /= arc1;

	if (fabs(*alampr) > 648000.0)
	{
		if (*alampr > 0.0)

			*alampr -= 1296000.0;

		else if (*alampr < 0.0)

			*alampr += 1296000.0;
	}

  	return;

} /* direct */


/******************************************************************************/
