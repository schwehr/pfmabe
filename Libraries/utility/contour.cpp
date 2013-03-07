
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

  - Programmer(s):     Dwight A. Johnson
  - Date Written:      May 1994

  - Module Name :      contour.c

  - Purpose:           This file contains all of the user callable functions
                       for the contouring software, and any functions that
                       access the CONTOUR structure.

****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

/* INCLUDE DEFINITIONS AND TYPEDEFS */

#include "globals.hpp"

/* PROTOTYPES FROM strtcon.c */

void initStartContour(int rows, int columns);
int startContour(float level, CONTOUR_POINT *point);

/* PROTOTYPE FROM follow.c */

int follow(float level, CONTOUR_POINT *point);

/* PROTOTYPES FROM bitmap.c */

int buildMap(CONTOUR_PARAMETERS *con, float *grid, float *min, float *max);
void freeMap(void);
int valid(CONTOUR_POINT *point, int emphasis);
float getDataPoint(CONTOUR_POINT *point, int corner);

static CONTOUR_PARAMETERS CON;
static int CONTOURMINMAX = CONTOUR_FALSE;
static int CONTOUREMPHASIS = CONTOUR_FALSE;
static int CONTOURMAXDENSITY = CONTOUR_FALSE;
static int CONTOURMAXPOINTS = CONTOUR_FALSE;
static int CONTOURLEVELS = CONTOUR_FALSE;

static int INITIALIZED = CONTOUR_FALSE;
static int PRESENT_STEP, LAST_STEP, EMPHASIS;

/* STRUCTURE FOR STORING THE CONTOUR POINTS */

static struct
{
  int points;
  float *x;
  float *y;
} CONTOUR;

/* THE FOLLOWING FUNCTIONS SET OPTIONAL CONTOUR PARAMETERS */

/**************************************************************************/
/*!

  - Module Name:      contourMinMax

  - Purpose:          Used to set the maximum and minimum valid data values
                     Valiue are used later to mask out contours in regions
                     containing invalid data.

  - Inputs:           minValue, maxValue

  - Outputs:          none

  - Calling Routines: unknown

  - Routines Called:  none

  - Glossary:
        - Static Globals
              - CON      = CONTOUR_PARAMETERS structure
              - INITIALIZED = state of contouring software
        - Local
              - minValue = minimum valid grid value
              - maxValue = maximum valid grid value

***************************************************************************/

void contourMinMax (float minValue, float maxValue)
{
    if (!(INITIALIZED))
    {
        CON.minValue = minValue;
        CON.maxValue = maxValue;
        CONTOURMINMAX = CONTOUR_TRUE;
    }
    else
    {
/*        printf("WARNING-call to contourMinMax ignored\n");
        printf("        must call contourMinMax before calling initContour\n");*/
    }
}

/***************************************************************************/
/*!

  - Function Name:    contourEmphasis

  - Purpose:          This function sets the emphasis contour interval
                      which is used in conjuction with the maxDensity
                      parameter in determining which contours to return
                      in high density areas.  Also used by getContour
                      inform the used that a contour is an emphasis contour.

  - Inputs:           emphasis

  - Outputs:          none

  - Calling Routines: unknown
  - Routines Called:  none

  - Glossary:
        - Static Globals
              - INITIALIZED = static global, TRUE if the contouring
                              has already been initialized.
              - CON  = CONTOUR_PARAMETERS structure
        - Local
              - emphasis - emphasis contour interval specified in the
                           number of contours between emphasis contours

****************************************************************************/

void contourEmphasis (int emphasis)
{
    if (!(INITIALIZED))
    {
        CON.emphasis = emphasis;
        CONTOUREMPHASIS = CONTOUR_TRUE;
    }
    else
    {
/*        printf("WARNING-call to contourEmphasis ignored\n");
        printf("        must call contourEmphasis before calling initContour\n");*/
    }  
}

/***************************************************************************/
/*!

  - Function Name:    contourMaxDensity

  - Purpose:          Sets the maximum density of contour per grid cell.
                     Non-emphasis contours that pass through grid cells
                     which exceed this value will not be returned
                     by getContour.

  - Inputs:           maxDensity

  - Outputs:          none

  - Calling Routines: unknown

  - Routines Called:  none

  - Glossary:
        - Static Globals
              - INITIALIZED = static global, TRUE if the contouring
                              has already been initialized.
              - CON  = CONTOUR_PARAMETERS structure
        - Local
              - maxDensity - maximum number of contours per grid cell.

****************************************************************************/

void contourMaxDensity (int maxDensity)
{
    if (!(INITIALIZED))
    {
        CON.maxDensity = maxDensity;
        CONTOURMAXDENSITY = CONTOUR_TRUE;
    }
    else
    {
/*        printf("WARNING-call to contourDensity ignored\n");
        printf("        must call contourDensity before calling initContour\n");*/
    }  
}

/***************************************************************************/
/*!

  - Function Name:    contourMaxPoints

  - Purpose:          Sets the maximum number of contour points to be
                      returned to the users routine by getContour.

  - Inputs:           maxPoints

  - Outputs:          none

  - Calling Routines: unknown

  - Routines Called:  none

  - Glossary: maxPoints = maximum number of contour points to be
                          returned at one time by getContour.

****************************************************************************/

void contourMaxPoints (int maxPoints)
{
    CON.maxPoints = maxPoints;
    CONTOURMAXPOINTS = CONTOUR_TRUE;
}

/***************************************************************************/
/*!

  - Function Name:    contourLevels

  - Purpose:          Used to manually set the number of contours to be
                      computed and the value of each contour level.

  - Inputs:           numLevels, levels

  - Outputs:          none

  - Calling Routines: unknown

  - Routines Called:  none

  - Glossary:
        - numLevels = number of contour levels to be computed
                      maximum number of levels is 1000
        - levels = array of the values for each contour level

****************************************************************************/

void contourLevels (int numLevels, float levels[])
{
    int       i;
  
    if (!(INITIALIZED))
    {
        CON.numLevels = numLevels;
        for (i = 0 ; i < numLevels ; i++)
        {
            CON.levels[i] = levels[i];
        }
        CONTOURLEVELS = CONTOUR_TRUE;
    }
    else
    {
/*        printf("WARNING-call to contourLevels ignored\n");
        printf("        must call contourLevels before calling initContour\n");*/
    }  
}


/***************************************************************************/
/*!

  - Function Name:    initContour

  - Purpose:          Sets the contour interval, and saves the grid size and
                     location.  Set the contour state to INITAILIZED and
                     call buildMap the allocate and initialize the bitmap.
                     Also computes the minimum and maximum contour level.

  - Inputs:           interval, rows, columns, grid

  - Outputs:          INITIALIZED

  - Calling Routines: unknown

  - Routines Called:  initStartContour, buildMap

  - Glossary:
        - Static Globals
              - CON = CONTOUR_PARAMETERS structure
              - INITAILIZED = contouring software state
              - PRESENT_STEP = number of contour intervals from 0.0 to the
                               present contour level.  Initial value will be
                               minimum contour level.
              - LAST_STEP = number of contour intervals to the last level
        - Local
              - interval = contour interval
              - rows = number of rows in the users grid
              - columns = number of columns in the users grid
              - grid = pointer to the users grid
              - min = minimum grid value
              - max = maximum grid value

****************************************************************************/

int initContour (float interval, int rows, int columns, float *grid)
{
    float min, max;



/* SET INITIAL CONTOUR PARAMETER VALUES --- if they haven't already been set.  */
/* minValue         -10000000000000000.
   maxValue          10000000000000000.
   emphasis                          5
   maxDensity                      999
   maxPoints                      1000
   numLevels                         0
*/

    if (!CONTOURMINMAX)
      {
        CON.minValue = -1.0e16F;
        CON.maxValue = 1.0e16F;
      }
    if (!CONTOUREMPHASIS) CON.emphasis = 5;
    if (!CONTOURMAXDENSITY) CON.maxDensity = 999;
    if (!CONTOURMAXPOINTS) CON.maxPoints = 1000;
    if (!CONTOURLEVELS) CON.numLevels = 0;


    CON.interval = interval;
    CON.rows = rows;
    CON.columns = columns;
    initStartContour(rows,columns);
    if (buildMap(&CON,grid,&min,&max))
    {
        if (CON.interval == 0.0)
        {
            PRESENT_STEP = 0;
            LAST_STEP = CON.numLevels - 1;
        }
        else
        {
            PRESENT_STEP = (int) floor((double) (min/interval));
            if (fmod((double) min,(double) interval) != 0) PRESENT_STEP++;
            LAST_STEP = (int) floor((double) (max/interval));
        }
        INITIALIZED = CONTOUR_TRUE;
    }
    else
    {
        INITIALIZED = CONTOUR_FALSE;
    }
    return INITIALIZED;
}

/***************************************************************************/
/*!

  - Function Name:    getContour

  - Purpose:          Returns the computed contours.  The contour level,
                      the number of contour points, and the x, y
                      coordinates are returned along with a TRUE/FALSE flag
                      specifying if the contour is an emphasis contour.
                      getContour returns TRUE if a contour is being returned
                      and FALSE if all contours have been returned.

  - Inputs:           level, emph, points, x, y

  - Outputs:          !(no_contour), level, emph, points, x, y

  - Calling Routines: unknown

  - Routines Called:  startContour, follow, clearMapFaces, freeMap

  - Glossary:
        - Static Globals
              - CON - CONTOUR_PARAMETERS structure
              - INITIALIZED = contour software state
              - PRESENT_STEP = number of contour intervals to the present level
              - LAST_STEP = number of contour intervals to the last level
              - EMPHASIS = TRUE/FALSE if emphasis contour
              - CONTOUR = structure containing the number of contour points and
                          pointers to the user supplied x, y arrays
        - Static Local
              - segment = TRUE if previous contour returned was segmented
              - last_x  = x coordinate of the last point in the previous segment
              - last_y  = y coordinate of the last point in the previous segment
              - point   = CONTOUR_POINT structure defining the present x, y,
                          heading
        - Local
              - no_contour = flag specifying if a contour has been computed
              - level  = pointer to contour level, returned
              - emph   = pointer to TRUE if emphasis contour being returned
              - points = pointer to number of contour points being returned
              - x  = pointer to x coordinates of contour points being returned
              - y  = pointer to y coordinates of contour points being returned

****************************************************************************/

int getContour (float *level, int *emph, int *points, float *x, float *y)
{
    static int segment = CONTOUR_FALSE;
    static float last_x, last_y;
    static CONTOUR_POINT point;
    int no_contour;

    void clearMapFaces ();


    no_contour = CONTOUR_TRUE;
    if (INITIALIZED)
    {
        if (CON.interval == 0.0)
        {
            *level = CON.levels[PRESENT_STEP];
            *emph = EMPHASIS = CONTOUR_FALSE;
        }
        else
        {
            *level = PRESENT_STEP * CON.interval;
            EMPHASIS = ! (PRESENT_STEP % CON.emphasis);
            *emph = EMPHASIS;
        }
        CONTOUR.x = x;
        CONTOUR.y = y;
        CONTOUR.points = 0;
        while (no_contour && PRESENT_STEP <= LAST_STEP)
        {

            /* CHECK IF PREVIOUS CONTOUR WAS SEGMENTED */

            if (segment)
            {

                /* LOAD LAST POINT OF PREVIOUS SEGMENT AS FIRST POINT IN THE NEXT */

                CONTOUR.points = 1;
                CONTOUR.x[0] = last_x;
                CONTOUR.y[0] = last_y;

                /* CONTINUE FOLLOWING THE CONTOUR THROUGH THE GRID */

                segment = follow(*level,&point);
            }
            else
            {

                /* GET THE FIRST POINT OF THE NEXT CONTOUR */

                if (startContour(*level,&point))
                {

                /* FOLLOW THE CONTOUR THROUGH THE GRID */

                    segment = follow(*level,&point);
                }
                else
                {

                    /* NO MORE CONTOURS EXIST FOR THIS LEVEL */
                    /* RESET THE BITMAP AND SET NEXT CONTOUR LEVEL */

                    clearMapFaces();
                    PRESENT_STEP++;
                    if (CON.interval == 0.0)
                    {
                        *level = CON.levels[PRESENT_STEP];
                        *emph = EMPHASIS = CONTOUR_FALSE;
                    }
                    else
                    {
                        *level = PRESENT_STEP * CON.interval;
                        EMPHASIS = !(PRESENT_STEP % CON.emphasis);
                        *emph = EMPHASIS;
                    }
                }
            }
            if (CONTOUR.points < 2)
            {

                /* IGNORE SINGLE POINT CONTOURS */
                CONTOUR.points = 0;
            }
            else
            {
                *points = CONTOUR.points;
                no_contour = CONTOUR_FALSE;
            }
        }
        if (no_contour)
        {

            /* ALL CONTOURS HAVE BEEN RETURNED, RELEASE BITMAP */

            INITIALIZED = CONTOUR_FALSE;
            freeMap();
        }
        if (segment)
        {

            /* SAVE LAST POINT OF SEGMENTED CONTOURS FOR FIRST POINT OF NEXT */

            last_x = CONTOUR.x[CONTOUR.points-1];
            last_y = CONTOUR.y[CONTOUR.points-1];
        }
    }
    else
    {
        printf("ERROR: MUST CALL initContour BEFORE getContour\n");
    }
    return ! no_contour;
}

/***************************************************************************/
/*!

  - Function Name:    savePoint

  - Purpose:          Compute the point at which the contour cross the
                      defined point and save it in the CONTOUR structure

  - Inputs:           level, point

  - Outputs:          status

  - Calling Routines: follow

  - Routines Called:  valid, getDataPoint

  - Glossary:
        - Static Globals
              - CON = CONTOUR_PARAMETERS structure
              - CONTOUR = structure containing the number of points in the
                          contour and pointers to the x, y arrays.
              - EMPHASIS = TRUE if present contour is an emphasis contour
        - Local
              - ds = distance to contour point from grid point
              - last = lesser of two grid points that the contour passes
              - level = present contour level
              - next = greater of two grid point of the face
              - point = pointer to x, y, and heading
              - status = TRUE (1) if point save in CONTOUR
              - x = present contour point x coordinate
              - y = present contour point y coordinate

****************************************************************************/

int savePoint(float level, CONTOUR_POINT *point)
{
    int status;
    float x, y, ds, last, next;

    if (valid(point,EMPHASIS) && CONTOUR.points < CON.maxPoints)
    {
        last = getDataPoint(point,CONTOUR_SAME);
        next = getDataPoint(point,CONTOUR_NEXT);

        if (last == next)
        {

            /* THIS SHOULD NEVER OCCUR */

            printf("last = next: %d %d %d %f \n", point->x, point->y,              
                point->heading,level);
            exit(-1);
        }
        x = (float) point->x;
        y = (float) point->y;
        ds = (float) ((level-last)/(next-last));
        switch (point->heading)
        {
            case CONTOUR_EAST:
                y += ds;
                break;

            case CONTOUR_SOUTH:
                x += ds;
                y += 1.0;
                break;

            case CONTOUR_WEST:
                x += 1.0;
                y += 1.0F - ds;
                break;

            case CONTOUR_NORTH:
                x += 1.0F -ds;
                break;
        }
        CONTOUR.x[CONTOUR.points] = x;
        CONTOUR.y[CONTOUR.points] = y;
        CONTOUR.points++;
        status = CONTOUR_TRUE;
    }
    else if (CONTOUR.points < 2)
    {
        CONTOUR.points = 0;
        status = CONTOUR_TRUE;
    }
    else
    {
        status = CONTOUR_FALSE;
    }
    return status;
}
