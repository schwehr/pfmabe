
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



/* newgp.c */

#include "newgp.h"

/******************************************************************************/
/*!

  - notes:
      - 1. This function was written to replace newgp of "when dinosaurs
           roamed".  It is simply a toplevel runner of the direct routine
           that was converted to C from NOAA's FORTRAN code.
      - 2. latobs, lonobs are the coordinates of the observed position (degrees)
      - 3. az is the azimuth from North and must be in [0, 360)
      - 4. dist is in meters
      - 5. *lat, *lon will contain the coordinate values of the directed
           position
      - 6. function direct works in arc seconds, azimuth from south, and meters
      - 7. function azNtoS takes the azimuth from North and gives you azimuth
           from South
      - 8. the ellipsoid constants for newgp are hard-coded to be those of
           wgs 84
*/

void newgp(NV_FLOAT64 latobs, NV_FLOAT64 lonobs, NV_FLOAT64 az, NV_FLOAT64 dist, NV_FLOAT64 *lat, NV_FLOAT64 *lon)
{

	NV_FLOAT64 phipri, alampr;
	void direct(NV_FLOAT64 phi, NV_FLOAT64 alam, NV_FLOAT64 fazi, NV_FLOAT64 s,
		NV_FLOAT64 *phipri, NV_FLOAT64 *alampr);
	NV_FLOAT64 azNtoS(NV_FLOAT64 azFromNorth);

	/* the conditional below was put in because direct bombed out when
		on the equator going east or west
	*/
	if ( latobs == 0.0 && (az == 90.0 || az == 270.0) )
		latobs += 1.0e-37;

	direct(3600.0*latobs, 3600.0*lonobs, 3600.0*azNtoS(az), dist,
		&phipri, &alampr);

	*lat = phipri/3600.0;
	*lon = alampr/3600.0;

} /* newgp */

/******************************************************************************/
NV_FLOAT64 azNtoS(NV_FLOAT64 azFromNorth)
/* azFromNorth must be in [0, 360)
*/
{
	NV_FLOAT64 azFromSouth;

	azFromSouth = azFromNorth + 180.0;

	if (azFromSouth > 360.0) azFromSouth -= 360.0;
	
	return azFromSouth;

} /* azNtoS */
