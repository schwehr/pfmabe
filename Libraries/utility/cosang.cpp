
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



/*!
    - cosang.c

    - purpose:  to produce the cosine of the angle made with sun given 3
                points on the earth and an azimuth and elevation for the sun.
    - author: david h. fabre
    - date:  sep 1997

    - Dropped the use of the POINT structure and replaced it with NV_F64_COORD3 to avoid
      a conflict with the Qt 4 libraries.  Jan Depner, 10/19/07
*/

#include <stdio.h>
#include <math.h>
#include "cosang.hpp"	/* prototype for cosang() */

#define DEG2RAD		0.0174532925199432957692

/******************************************************************************/
static NV_F64_COORD3 cross(NV_F64_COORD3 a, NV_F64_COORD3 b)
/* purpose:  compute the cross product of a and b */
{
	NV_F64_COORD3 ans;

	ans.x = a.y*b.z - b.y*a.z;
	ans.y = a.z*b.x - b.z*a.x;
	ans.z = a.x*b.y - b.x*a.y;

	return ans;

} /* cross */

/******************************************************************************/
static NV_FLOAT64 dot(NV_F64_COORD3 a, NV_F64_COORD3 b)
/* purpose:  compute the dot product of a and b */
{
#ifdef DEBUG_COSANG
	printf("dot = %f\n", a.x*b.x + a.y*b.y + a.z*b.z); 
#endif
	return a.x*b.x + a.y*b.y + a.z*b.z; 
} /* dot */

/******************************************************************************/
static NV_FLOAT64 mag(NV_F64_COORD3 a)
/* purpose:  compute the magnitude of a */
{
#ifdef DEBUG_COSANG
	printf("mag = %f\n", sqrt( a.x*a.x + a.y*a.y + a.z*a.z )); 
#endif
	return sqrt( a.x*a.x + a.y*a.y + a.z*a.z ); 
} /* mag */

/******************************************************************************/
NV_F64_COORD3 sun_unv(NV_FLOAT64 azi, NV_FLOAT64 elev)
/*!

   - purpose:  define the unit normal vector towards the sun.
	       Note that the azimuth is decimal degrees clockwise from North.
               it should be between 0 and 360.
               also note that elev is the sun's elevation angle (dec.deg.) and is 
               measured up from the horizon.  it should be between 0 and 90.
   - reference:
	- Land Analysis System 
	- U.S. Geological Survey National Mapping Division
	- EROS Data Center (EDC)
	- PICSHADE function as seen on Penn State Univ.'s web server
	  http://eoswww.essc.psu.edu/lasdoc/usr/ug/picshade.html
*/
{
	NV_F64_COORD3 sun;

	azi *= DEG2RAD;
	elev *= DEG2RAD;

	/* define the unit vector normal to the sun */
	sun.x = 1.0*sin(azi)*cos(elev);
	sun.y = 1.0*cos(azi)*cos(elev);
	sun.z = 1.0*sin(elev);

	return sun;

} /* sun_unv */

/******************************************************************************/
NV_FLOAT64 cosang(NV_F64_COORD3 *pt, NV_F64_COORD3 sun)
{
/*!<pre>
	pt = an array of triplets (x, y, z) that correspond to
	3 points of a plane surface (in the same units, meters perhaps).

	The reason we wish to have the cosine of the angle:
		(reference:  Foley, van Dam, et.al., Computer Graphics,
			2nd ed. in C, p. 724)
	a topic in visualization techniques often employed
	involves computing a diffuse illumination value which is
	shaded using the diffuse-reflection model

		I = I_p * k_d * cosang

	where
		I is illumination value,
		I_p is the point light source's intensity (the Sun perhaps),
		k_d is a constant ( [0, 1] in interval notation ) that
			varies according the properties of the material,

	This model is based on diffuse or Lambertian reflection which is
	associated with dull, matte surfaces.

	note that the "NV_F64_COORD3 sun" is defined by the
	sun_unv() function below.  It determines the Sun's unit normal vector.
	Since this need not be done repetitively, its been extracted from
	cosang().

	azi = the azimuth to the sun in decimal degrees and
		0 <= azi < 360.0

	elev = the elevation angle to the sun in decimal degrees and
		0 <= elev <= 90.0

	notes:
	(0) pt[0] is the principal point on our surface.
	(1) this routine returns -1 on failure due to zero divide
	(2) for sun shading a grid point which is oriented north to the top
		and assuming the z-axis is negative below the sea
		we'd have the pt[] array filled up as follows:

       (x=0,y=0,z=-100) pt[0] *-----------* pt[2] (x=pixlen,y=0,z=-99)
                              |
                              |
                              |
                              |
                              |
 (x=0,y=-pixlen,z=-101) pt[1] *

		this would produce the answer

		cosang = 0.747634 (ang = 41.614171)

	(3) also, for sun shading, a negative cosang() means its
		not directly illuminated by the sun (its in the 
		shadows).
</pre>*/
	NV_F64_COORD3 surf[2];
	NV_F64_COORD3 surfperp;
	NV_FLOAT64 magperp;

	/* define the surface vectors w.r.t. pt[0] */
	surf[0].x = pt[1].x - pt[0].x;
	surf[0].y = pt[1].y - pt[0].y;
	surf[0].z = pt[1].z - pt[0].z;
	
	surf[1].x = pt[2].x - pt[0].x;
	surf[1].y = pt[2].y - pt[0].y;
	surf[1].z = pt[2].z - pt[0].z;

	/* find the perpendicular vector to the surface vectors */
	surfperp = cross(surf[0], surf[1]);
	/* and it length */
	magperp = mag(surfperp);

#ifdef DEBUG_COSANG
printf("sun  : %f %f %f\n", sun.x, sun.y, sun.z);
printf("surf0: %f %f %f\n", surf[0].x, surf[0].y, surf[0].z);
printf("surf1: %f %f %f\n", surf[1].x, surf[1].y, surf[1].z);
printf("surfperp: %f %f %f\n", surfperp.x, surfperp.y, surfperp.z);
#endif
	
	if (magperp == 0.0)
		/* return -1 if we're going to bomb out */
		return -1;
	else
	/* define the cosine of the angle between the surface perpendicular
		and the perpendicular to the sun as the dot product 
		of the unit normals
	*/
		return dot(surfperp, sun) / magperp;

} /* cosang */

#ifdef TEST_COSANG
/******************************************************************************/
NV_INT32 main(NV_INT32 argc, NV_CHAR **argv)
{
/* c89 -o cosang cosang.c -DDEBUG_COSANG -DTEST_COSANG -lM */

	NV_FLOAT64 alpha;
	NV_FLOAT64 gridinc=50;
	NV_FLOAT64 vert_exag=1;
	NV_FLOAT64 pixlen=gridinc/vert_exag;
	NV_FLOAT64 elev=50;
	NV_FLOAT64 azi=50;
	NV_FLOAT64 pixel=(-100);
	NV_FLOAT64 peast=(-99);
	NV_FLOAT64 psouth=(-101);
	/* pt[3] below is a functionally equivalent surface to
		the "pixel, psouth, peast, pixlen" set
		that would be defined below that.
		this shows that the method will work for an arbitrary
		cartesian surface (set of 3 3-coordinate points) oriented
		w.r.t. the initial point in the array.
	*/
	NV_F64_COORD3 pt[3]={
			{100, 100, -100}
			,
			{100, 50, -101}
			,
			{150, 100, -99}
			};
/*
	pt[0].x = 0;
	pt[0].y = 0;
	pt[0].z = pixel;

	pt[1].x = 0;
	pt[1].y = -pixlen;
	pt[1].z = psouth;

	pt[2].x = pixlen;
	pt[2].y = 0;
	pt[2].z = peast;
*/
	/* these are jhc's vectangle() inputs ...
		test main() used to compare answers
	*/
	printf("pixel, psouth, peast, pixlen, azi, elev: \n");
	printf("%.0f %.0f %.0f %.0f %.0f %.0f\n",
		pixel, psouth, peast, pixlen, azi, elev);
	printf("cosang = %f (ang = %f)\n",
			(alpha=cosang( pt, sun_unv(azi, elev) )),
				acos(alpha)/DEG2RAD);

	while (1)
	{
		printf("enter pixel, psouth, peast, pixlen, azi, elev: ");
		scanf("%lf %lf %lf %lf %lf %lf",
			&pixel, &psouth, &peast, &pixlen, &azi, &elev);

		pt[0].x = 0;
		pt[0].y = 0;
		pt[0].z = pixel;

		pt[1].x = 0;
		pt[1].y = -pixlen;
		pt[1].z = psouth;

		pt[2].x = pixlen;
		pt[2].y = 0;
		pt[2].z = peast;

		if ( (alpha = cosang( pt, sun_unv(azi, elev) )) < 0)
			printf("0.0 > cosang = %f (not shadeable)\n", alpha);
		else
			printf("cosang = %f (ang = %f)\n",
				alpha, acos(alpha)/DEG2RAD);
	} 

} /* main */
#endif /* TEST_COSANG */

#ifdef TEST_COSANG2
/******************************************************************************/
NV_INT32 main(NV_INT32 argc, NV_CHAR **argv)
{
/* c89 -o cosang2 cosang.c -DDEBUG_COSANG -DTEST_COSANG2 -lM */

	NV_FLOAT64 alpha;
	NV_FLOAT64 elev=25;
	NV_FLOAT64 azi=75;
	NV_INT32 i;
	/* example taken from
		purcell's 3rd ed. of calculus with analytical geometry,
		prentice-hall, 1978, section 16.4, ex.5, p661.
		to check the cross product calculation
	*/
	NV_F64_COORD3 pt[3]={
			{2, -1, 4}
			,
			{1, 3, -2}
			,
			{-3, 1, 2}
			};

	printf("sun elev,azi:  %f %f\n", elev, azi);
	for(i = 0; i < 3; i++)
		printf("pt[%d]: %f %f %f\n",
			i, pt[i].x, pt[i].y, pt[i].z);
	printf("cosang = %f (ang = %f)\n",
			(alpha=cosang( pt, sun_unv(azi, elev) )),
				acos(alpha)/DEG2RAD);

	while (1)
	{
		/* ... a more general means of inputting ... */
		printf("\n\nenter sunazi, sunelev: ");
		scanf("%lf %lf", &azi, &elev);
		printf("enter principal point x,y,z: ");
		scanf("%lf %lf %lf", &pt[0].x, &pt[0].y, &pt[0].z);
		printf("enter secondary point 1 x,y,z: ");
		scanf("%lf %lf %lf", &pt[1].x, &pt[1].y, &pt[1].z);
		printf("enter secondary point 2 x,y,z: ");
		scanf("%lf %lf %lf", &pt[2].x, &pt[2].y, &pt[2].z);

		if ( (alpha = cosang( pt, sun_unv(azi, elev) )) < 0)
			printf("0.0 > cosang = %f (not shadeable)\n", alpha);
		else
			printf("cosang = %f (ang = %f)\n",
				alpha, acos(alpha)/DEG2RAD);
	} 

} /* main */
#endif /* TEST_COSANG2 */
