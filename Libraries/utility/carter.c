
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



#include "carter.h"
#include "carter_tables.h"       /*  Correction table data  */


/*! carter.c

    - purpose:  implementaion of carters' tables.  given a nominal
		uncorrected fathmometer type depth, produce the estimate of
		true depth.

    - started with meredith's m-file and data indicated below ...

    - function [cordepth,correction_area] = carter(lat,lon,uncdepth);

    - Function to apply Carter table correction to echosounder
      data that was collected assuming sound speed = 1500 m/s.

    - Inputs are (latitude, longitude, uncorrected depth)
    - Inputs should have South and East = -ve
    - Inputs can be row or column arrays, but not matrices (function is
      designed for single-beam echosounder data)

    - Uses lookup tables BOUNDARY2.DAT and CORRECTN2.DAT

    - Outputs are corrected depth, and Carter correction area
      (Note that function returns correction_area = 99999 if no correction
      is applied because input depth is <200m)

    - Author: Mike Meredith, British Antarctic Survey, Dec 2005

    - Stuck in ice onboard RRS James Clark Ross, off Adelaide Island, Antarctic Peninsula

    - function:  carter
    - purpose:  apply carter table lookup to an array of uncorrected
                depths.  requires BOUDARY2.DAT and CORRECTN2.DAT
                (in $SOFTWARE_DEPOT/src/conversions/carter).  started
                with matlab version by m. meredith found on scripps web
                site.  returns 1 on success, 0 otherwise.
    - author:  dave fabre, navoceano, 12/2006
    - history:
	- feb 2007 = changed data to static arrays to eliminate
                     use of external files.
		
    - example:  from page 10 of np 139, echo-sounding correction tables, 3rd edition,
                the hydrographic dept, ministry of defence, taunton, djt carter, 1980
                - for test program compiled as carter_yxz ...
                - input:  54.416667 -168.250000 1159.000000
                - output: 54.416667 -168.250000 1159.000000 1137.410000 48

*/

NV_BOOL carter (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT64 nominal, NV_FLOAT64 *depth, NV_INT32 *cor_area)
{
  NV_INT32 i, lat_indx, lon_use;
  NV_BOOL found;
  NV_FLOAT64 cordepth_high = 0, cordepth_low = 0, uncdepth_low = 0, offset;

  *depth = nominal;
  *cor_area = 99999;

  if (fabs(lat) > 90 || fabs(lon) > 180 || nominal <= 200 || nominal > 12000)
    {
      fprintf (stderr, "navigation or depth out of range\n");
      return (NVFalse);
    }


  /* get lat index and round down lon */

  lat_indx = 89 - (NV_INT32) floor (lat);   /* 89-+89=0, ..., 89--91=180 */
  lon_use = (NV_INT32) floor (lon);


  /* now scan through longitudes to find correction area */

  found = NVFalse;
  if (lon_use > lonbnd[lat_indx][nbnd[lat_indx] - 1])
    {
      *cor_area = area[lat_indx][ nbnd[lat_indx] - 1];
      found = NVTrue;
    }
  for (i = 0 ; !found && i < nbnd[lat_indx] - 1 ; i++)
    {
      if ((lon_use >= lonbnd[lat_indx][i]) && (lon_use < lonbnd[lat_indx][i + 1]))
        {
          *cor_area = area[lat_indx][i];
          found = NVTrue;
        }
    }
  if (!found)
    {
      fprintf (stderr, "problems finding the proper longitude/area\n" );
      return (NVFalse);
    }


  /* find bounding values ... */

  found = NVFalse;
  for (i = 0 ; !found && i < ncor[*cor_area - 1] - 1 ; i++) 
    {
      if ((nominal >= uncdepth[i]) && (nominal < uncdepth[i + 1]))
        {
          cordepth_high = (NV_FLOAT64) (cor[*cor_area - 1][i + 1]);
          cordepth_low = (NV_FLOAT64) (cor[*cor_area - 1][i]);
          uncdepth_low = (NV_FLOAT64) ( uncdepth[i] );
          found = NVTrue;
        }
    }
  if (!found)
    {
      fprintf (stderr, "problems finding proper uncorrected depth layer\n");
      fprintf (stderr, "nominal = %f\n", nominal);
      return (NVFalse);
    }


  /* now linearly interpolate to find corrected depth */

  offset = (NV_FLOAT64) nominal - uncdepth_low;
  *depth = cordepth_low + ((cordepth_high - cordepth_low) * offset) / 100;

  return (NVTrue);
}


NV_BOOL rev_carter (NV_FLOAT64 lat, NV_FLOAT64 lon, NV_FLOAT64 tru, NV_FLOAT64 *depth, NV_INT32 *cor_area)
{
  NV_INT32 i, lat_indx, lon_use;
  NV_BOOL found;
  NV_FLOAT64 uncdepth_high = 0, uncdepth_low = 0, cordepth_low = 0, offset;

  *depth = tru;
  *cor_area = 99999;

  if (fabs (lat) > 90 || fabs (lon) > 180 || tru <= 200 || tru > 12000)
    {	
      fprintf (stderr, "navigation or depth out of range\n");
      return (NVFalse);
    }


  /* get lat index and round down lon */

  lat_indx = 89 - (NV_INT32) floor (lat); /* 89-+89=0, ..., 89--91=180 */
  lon_use = (NV_INT32) floor (lon);


  /* now scan through longitudes to find correction area */

  found = NVFalse;
  if (lon_use > lonbnd[lat_indx][nbnd[lat_indx] - 1])
    {
      *cor_area = area[lat_indx][nbnd[lat_indx] - 1];
      found = NVFalse;
    }
  for (i = 0 ; !found && i < nbnd[lat_indx] - 1 ; i++)
    {
      if ((lon_use >= lonbnd[lat_indx][i]) && (lon_use < lonbnd[lat_indx][i + 1]))
        {
          *cor_area = area[lat_indx][i];
          found = NVTrue;
        }
    }
  if (!found)
    {
      fprintf (stderr, "problems finding the proper longitude/area\n" );
      return (NVFalse);
    }


  /* find bounding values ... */
	
  found = NVFalse;
  for (i = 0 ; !found && i < ncor[*cor_area - 1] - 1 ; i++) 
    {
      if ((tru >= cor[*cor_area - 1][i]) && (tru < cor[*cor_area - 1][i + 1]))
        {
          uncdepth_high = (NV_FLOAT64) (uncdepth[i + 1]);
          uncdepth_low = (NV_FLOAT64) (uncdepth[i]);
          cordepth_low = (NV_FLOAT64) (cor[*cor_area - 1][i]);
          found = NVTrue;
        }
    }
  if (!found)
    {
      fprintf (stderr, "problems finding proper uncorrected depth layer\n");
      fprintf (stderr, "true = %f\n", tru);
      return (NVFalse);
    }


  /* now linearly interpolate to find corrected depth */

  offset = (NV_FLOAT64) tru - cordepth_low;

  *depth = uncdepth_low + ((uncdepth_high - uncdepth_low) * offset) / 100;

  return (NVTrue);
}
