
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
#include <string.h>
#include <memory.h>
#include <errno.h>
#include "area.h"
#include "select.h"

/***************************************************************************/
/*!

  - Module Name:        load_vertices

  - Programmer:         Jan C. Depner

  - Date Written:       July 1992

  - Purpose:            Loads the vertices of the area into the area_file
                        structure.

  - Inputs:
                        - area_file           =   area_file structure
                        - string              =   string containing the area
                                                  name
                        - sheet               =   pointer to sheet file

  - Outputs:
                        - void

****************************************************************************/

void load_vertices (AREA_FILE *area_file, const NV_CHAR *name, FILE *sheet, NV_CHAR *projection)
{
    NV_INT32         latd, lond, latm, lonm, i, j;
    NV_FLOAT32       lats, lons;
    NV_CHAR          lath, lonh, next_char, string[132]/* , id[20]*/;

    /*  Load the area name into the area structure.                     */
    
    strcpy (area_file->area.name, name);
    strcpy (area_file->area.projection, projection);
    
    i = 0;

    /*  Loop through the file and get the scale, and vertices  */
    /*  of the area and load them into the area structure.     */

    while (1)
    {
        if (fgets (string, 132, sheet) == NULL) break;

        if (!strncmp (string, "SCALE", 5))
            sscanf (&string[6], "%f", &area_file->area.scale);

        /*  Load the vertices.                                          */
        
        if (!strncmp (string, "POINT=P-", 8))
        {
            /*  Check for first semi-colon: marks start of position information.  */
            
          for (j = 0 ; j < (NV_INT32) strlen (string) ; j++)
            {
                next_char = string[j];
                if (next_char == ';') break;
            }
            j++;

            /*  If position of first semi-colon in string is greater than  */
            /*  the thirteenth position, then this is probably a bad record.  */
            /*  So ignore.  */
            
            if (j < 13)
            {
                sscanf (&string[j], "%d %d %f%c;%d %d %f%c", &latd, &latm, &lats,
                        &lath, &lond, &lonm, &lons, &lonh);
                area_file->area.lat[i] = latd + latm / 60.0 + lats /
                    3600.0;
                if (lath == 'S') area_file->area.lat[i] =
                    -area_file->area.lat[i];
                area_file->area.lon[i] = lond + lonm / 60.0 + lons /
                    3600.0;
                if (lonh == 'W') area_file->area.lon[i] =
                    -area_file->area.lon[i];
                i++;
            }
        }
    }

    /*  Load the number of vertices into the area structure.            */
    
    area_file->area.points = i;
    area_file->flag = 0;
}
