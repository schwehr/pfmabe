
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
/*!

 - Programer(s):     Dwight A. Johnson

 - Date Written:     May 1994

 - Module Name:      follow

 - Purpose:          Based on an article by Bruce Giles in Dr. Dobbs Journal,
                     June 1992.  Given the present contour level and an
                     initial POINT, follow will trace a contour through a grid
                     until 1 of 4 conditions occurs:
                         - 1. The contour exits the grid.
                         - 2. The contour closes on itself.
                         - 3. The contour is traced in an invalid grid cell.
                         - 4. The maximum number of saved contour points is
                              reached.

 - Inputs:           level, point
 - Outputs:          segment

 - Files Included:   globals.h

 - Calling Routines: getContour

 - Routines Called:
                     - savePoint
                     - faceInUse
                     - markInUse
                     - cellExists
                     - getDataPoint
                     - getExit

 - Glossary:
                     - adj = 2 if contour may exit adjacent face from present
                     - exitFace = relative offset of the exit face from the present
                     - fadj = grid value of point adjacent to present point
                     - fopp = grid value of point opposite to present point
                     - level = present contour level
                     - point = POINT pointer to x,y indexes and heading of the last
                               point in the contour that has been followed
                     - segment = TRUE if partial contour has been computed
                     - start_heading = saves heading of the initial contour POINT

*****************************************************************************/
#include "globals.hpp"                                                        

/* PROTOTYPES FROM contour.c */

int savePoint(float level, CONTOUR_POINT *point);

/* PROTOTYPES FROM bitmap.c */

int faceInUse(CONTOUR_POINT *point, int corner);
void markInUse(CONTOUR_POINT *point);
int cellExists(CONTOUR_POINT *point);
float getDataPoint(CONTOUR_POINT *point, int corner);

/* PROTOTYPE FROM getexit.c */

int getExit (float level, CONTOUR_POINT *point);

int follow(float level, CONTOUR_POINT *point)
{
    int segment, start_heading, exitFace, adj, opp;
    float fadj,fopp;

    exitFace = CONTOUR_UNKNOWN;

    segment = -1;
    start_heading = point->heading;

    while (segment == -1)
    {
        /* SAVE PRESENT POINT IF VALID */

        if (savePoint(level,point))
        {

            /* HAS THE CONTOUR CLOSED */

            if (!(faceInUse(point,CONTOUR_SAME)))
            {
                markInUse(point);

                /* HAS THE CONTOUR REACHED AN EDGE */

                if (cellExists(point))
                {

                    /* COMPUTE THE EXIT FACE RELATIVE TO THE PRESENT FACE */

                    fadj = getDataPoint(point,CONTOUR_ADJACENT);
                    fopp = getDataPoint(point,CONTOUR_OPPOSITE);
                    adj = (fadj > level) ? 2 : 0;
                    opp = (fopp <= level) ? 1 : 0;
                    switch (adj+opp)
                    {
                        case 0:
                            exitFace = CONTOUR_OPPOSITE;
                            break;

                        case 1:
                            exitFace = CONTOUR_NEXT;
                            break;

                        case 2:
                            exitFace = CONTOUR_ADJACENT;
                            break;

                        case 3:
                            exitFace = getExit(level,point);
                            break;
                    }
                    if (exitFace == CONTOUR_UNKNOWN)
                    {
                        point->heading = start_heading;
                    }
                    else
                    {
                        point->heading = (point->heading + exitFace + 2) % 4;
                    }
          
                    /* UPDATE THE INDEXES BASE ON THE NEW HEADING */

                    switch (point->heading)
                    {
                        case CONTOUR_EAST:
                            point->x++;
                            break;

                        case CONTOUR_NORTH:
                            point->y++;
                            break;

                        case CONTOUR_WEST:
                            point->x--;
                            break;

                        case CONTOUR_SOUTH:
                            point->y--;
                            break;
                    }
                }
                else
                {
                    /* CONTOUR HAS REACHED AN EDGE, EXIT */

                    segment = CONTOUR_FALSE;
                }
            }
            else
            {
                /* CONTOUR HAS CLOSED, EXIT */
                segment = CONTOUR_FALSE;
            }
        }
        else
        {
            /* CONTOUR IS SEGMENTED BY INVALID DATA OR MAX CONTOUR POINTS */
            segment = CONTOUR_TRUE;
        }
    }
    return segment;
}
