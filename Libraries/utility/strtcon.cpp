
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



/*********************************************************************/
/*!

   - Programmer(s):      Dwight A. Johnson

   - Date Written:       May 1994

   - Module Name:        strtcon.c

   - Files Included:     globals.h

   - Glossary
        - Global:          none

        - Static Global:
              - MAX_X = int, x dimension of the input grid
              - MAX_Y = int, y dimension of the input grid
              - REF   = POINT structure, holds the current
                        x,y indexes and heading, starting
                        point when searching for contours

**********************************************************************/

/* INCLUDE GLOBAL DEFINITIONS AND TYPEDEFS */

#include "globals.hpp"

/* PROTOTYPES FROM bitmap.c */

float getDataPoint(CONTOUR_POINT *point, int corner);
int faceInUse(CONTOUR_POINT *point, int corner);

/* DECLARE STATIC GLOBALS FOR THIS MODULE */

static int MAX_X, MAX_Y;

/*********************************************************************************************

    This is public domain software that was developed by the U.S. Naval Oceanographic Office.

    This is a work of the US Government. In accordance with 17 USC 105,
    copyright protection is not available for any work of the US Government.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with slash*! are being used by Doxygen to document the
    API.  Dashes in these comment blocks are used to create bullet lists.  The lack of blank
    lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need to
    be left alone.  If you see a comment that starts with slash*! and there is something that
    looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very careful 
    modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/


static CONTOUR_POINT REF;

/*********************************************************************/
/*!
   - Function Name:     initStartContour

   - Purpose:           Save the dimensions of the input grid and
                       initialize the REF POINT used when searching
                       for the start of contours.

   - Parameters
         - Inputs:          rows, columns
         - Outputs:         none

   - Calling Routines:  initContour

   - Routines Called:   none

   - Glossary
         - Globals:         none
         - Static Globals:  MAX_X, MAX_Y, REF
         - Local:
               - rows = int, number of rows in the input grid
               - columns = int, number of columns in the input grid.

**********************************************************************/

void initStartContour(int rows, int columns)
{
    MAX_X = columns;
    MAX_Y = rows;
    REF.x = 0;
    REF.y = 0;
    REF.heading = CONTOUR_EAST;
}

/*********************************************************************/
/*!

   - Function Name:     startContour

   - Purpose:           This function returns the starting POINT for
                        a contour of the specified level in the POINT
                        parameter.  If no contour is found, the
                        function returns FALSE.

   - Parameters:
         - Input:           level, point
         - Output:          !(no_contour), point

   - Calling Routines:  initContour

   - Routines Called:   none

   - Glossary
         - Globals:         none
         - Static Globals:  MAX_X, MAX_Y, REF
         - Local:
               - rows = int, number of rows in the input grid
               - columns = int, number of columns in the input grid.
               - level = float, current contour level
               - point = pointer to POINT structure, returns x,y
                         indexes and heading of first contour face.
               - index = int pointer, pointer to REF.x or REF.y
                         depending on present heading
               - inc = int, index step, +/- depends on heading
               - end = int, maximum index value, depends on heading
               - no_contour = int, TRUE/FALSE, set FALSE when
                              a contour is found.
               - next = float, value of NEXT grid point from
                        present REF point.
               - last = float, value of previous grid point

**********************************************************************/

int startContour (float level, CONTOUR_POINT *point)
{
    int *index, inc, end, no_contour;
    float last, next;

    index = 0;
    end = 0;
    inc = 0;
  
    no_contour = CONTOUR_TRUE;
    while (no_contour && REF.heading <= CONTOUR_NORTH)
    {
        /* SET LOOP VARIABLES BASED ON PRESENT HEADING */

        switch (REF.heading)
        {
            case CONTOUR_EAST:
                index = &REF.y;
                end = MAX_Y - 1;
                inc = 1;
                break;
        
            case CONTOUR_SOUTH:
                index = &REF.x;
                end = MAX_X - 1;
                inc = 1;
                break;
        
            case CONTOUR_WEST:
                index = &REF.y;
                end = -1;
                inc = -1;
                break;
        
            case CONTOUR_NORTH:
                index = &REF.x;
                end = -1;
                inc = -1;
                break;
        }

        next = getDataPoint(&REF,CONTOUR_SAME);

        /* LOOP THROUGH PRESENT ROW/COLUMN FOR CONTOURS OF THE PRESENT LEVEL */

        while ((*index != end) && no_contour)
        {
            last = next;
            next = getDataPoint(&REF,CONTOUR_NEXT);

            /* CHECK IF LEVEL IS BETWEEN TWO GRID POINTS */

            if ((last <= level) && (level < next))
            {
                /* CHECK IF CONTOUR FOUND EARLIER */

                if (!faceInUse(&REF,CONTOUR_SAME))
                {
                    /* SET RETURN VALUES */

                    no_contour = CONTOUR_FALSE;
                    point->x = REF.x;
                    point->y = REF.y;
                    point->heading = REF.heading;
                }
            }
            *index += inc;
        }

        /* CHECK IF A CONTOUR WAS FOUND */

        if (no_contour)
        {
            /* SET REF VALUES FOR THEN NEXT ROW/COLUMN */

            if (REF.heading != CONTOUR_NORTH)
            {
                REF.heading++;
                if (REF.heading == CONTOUR_NORTH) REF.y = -1;
            }

            switch (REF.heading)
            {
                case CONTOUR_SOUTH:
                    REF.x = 0;
                    REF.y = MAX_Y - 2;
                    break;
                    
                case CONTOUR_WEST:
                    REF.x = MAX_X - 2;
                    REF.y = MAX_Y -2;
                    break;
                    
                case CONTOUR_NORTH:
                    if (REF.y < MAX_Y -2)
                    {
                        REF.y++;
                        REF.x = MAX_X - 2;
                    }
                    else
                    {
                        REF.heading++;
                    }
            }
        }
    }

    /* IF NO_CONTOUR, ENTIRE GRID SEARCHED, RESET REF TO START NEXT LEVEL */

    if (no_contour)
    {
        REF.x = 0;
        REF.y = 0;
        REF.heading = CONTOUR_EAST;
    }
    return ! no_contour;
}
