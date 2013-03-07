
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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "points.h"
#include "ngets.h"

/***************************************************************************/
/*!

  - Module Name:        get_points

  - Programmer:         Jan C. Depner

  - Date Written:       September 1992

  - Purpose:            Prompt the user for pairs of geographic coordinates
                        and allow editing of the input data.

  - Inputs:
                        - points              =   array of POINT structures
                        - num_points          =   number of POINT structures

  - Outputs:            none

  - Restrictions:       Geographic positions are entered as a lat, lon pair
                        separated by a comma.  A lat or lon may be in any
                        of the following formats (degrees, minutes, and
                        seconds must be separated by a space or tab) :
                            - Degrees decimal                 : S 28.4532
                            - Degrees minutes decimal         : S 28 27.192
                            - Degrees minutes seconds decimal : S 28 27 11.52
                        Hemisphere may be indicated by letter or by sign.
                        West longitude and south latitude are negative :
                            - Ex. : -28 27 11.52 = S28 27 11.52 = s 28 27.192

****************************************************************************/

void get_points (NAV_PAIR points[], NV_INT32 *num_points)
{
    NV_INT32         i;
    NV_CHAR          string[10];

    NV_INT32 get_coords (NV_CHAR *, NV_INT32 *, NV_INT32 *, NV_FLOAT32 *, NV_CHAR *, NV_INT32 *, NV_INT32 *,
                         NV_FLOAT32 *);

    *num_points = 0;

    /*  Get the coordinates.                                            */
    
    for (i = 0 ; i < 20 ; i++)
    {
        printf
            (" \n Enter the lat and lon for point %d (enter done to quit) :\n",
            i + 1);

        if (get_coords (&points[i].lat_hemi, &points[i].lat_deg,
            &points[i].lat_min, &points[i].lat_sec, &points[i].lon_hemi,
            &points[i].lon_deg, &points[i].lon_min, &points[i].lon_sec))
            break;

        (*num_points)++;
    }

    /*  Allow the user to edit the coordinate pairs.                    */
    
    while (1)
    {
        printf ("\n\n\n");

        /*  Print out all of the points.                                */
        
        for (i = 0 ; i < *num_points ; i++)
        {
            printf ("  (%02d)  -  %1c %02d %02d %05.2f , %1c %03d %02d %05.2f\n",
                i + 1, points[i].lat_hemi, points[i].lat_deg,
                points[i].lat_min, points[i].lat_sec, points[i].lon_hemi,
                points[i].lon_deg, points[i].lon_min, points[i].lon_sec);
        }

        /*  Ask the user if he wants to edit any.                       */
        
        printf ("\n\n If you wish to change a point, enter the point number.\n");
        printf (" Enter done to quit : ");

        ngets (string, sizeof (string), stdin);

        if (strstr (string, "done") != NULL) break;

        sscanf (string, "%d", &i);

        i--;

        /*  Get the replacement points.                                 */
        
        if (i >= 0 && i < *num_points)
        {
            printf ("%1c %02d %02d %05.2f   %1c %03d %02d %05.2f\n",
                points[i].lat_hemi, points[i].lat_deg, points[i].lat_min,
                points[i].lat_sec, points[i].lon_hemi, points[i].lon_deg,
                points[i].lon_min, points[i].lon_sec);

            get_coords (&points[i].lat_hemi, &points[i].lat_deg,
                &points[i].lat_min, &points[i].lat_sec, &points[i].lon_hemi,
                &points[i].lon_deg, &points[i].lon_min, &points[i].lon_sec);
        }
    }
}
