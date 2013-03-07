
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

  - Module Name:        bitmap.c

  - Programmer(s):      Dwight A. Johnson

  - Date Written:       May 1994

  - Purpose:            bitmap.c is a collection of functions that build and
                        maintain a bitmap of information about each grid value
                        and grid face used by the contouring software.  Each
                        byte in the bitmap contains validity, density and face
                        in use information about the grid cell faces to the
                        right and above a grid point.

****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>

/* INCLUDE GLOBAL DEFINITIONS AND TYPEDEFS */

#include "globals.hpp"

/* DEFINE BIT MASK FOR SETTING/CHECKING FACE IN USE, VALIDITY, AND DENSITY */

#define EW_FACE    0x01
#define NS_FACE    0x02
#define EW_INVALID 0x04
#define NS_INVALID 0x08
#define EW_DENSITY 0x10
#define NS_DENSITY 0x20
#define CLEAR_FACE 0xFC

/* DECLARE GLOBAL VARIABLES FOR THE BIT MAP */

static float *GRID;
static unsigned char *MAP;
static int MAX_X, MAX_Y;

/***************************************************************************/
/*!

  - Function Name:    buildMap

  - Purpose:          This function allocates the bitmap memory, and scans
                      the grid, setting the approriate bit in the bitmap.
                      The minimum and maximum grid values are returned in
                      the min and max parameters.  If the bit map memory is
                      allocated successfully the function returns TRUE.

  - Inputs:           con, grid, min, max

  - Outputs:          status, min, max

  - Calling Routines: initContour

  - Routines Called:  none

  - Glossary:
      - Static Globals
              - GRID = pointer to users grid
              - MAP  = pointer to the bit map
              - MAX_X = number of columns in the grid
              - MAX_Y = number of rows in the grid
      - Local
              - con      = pointer to CONTOUR_PARAMETERS structure
                           see globals.h
              - grid     = pointer to input data grid values
              - min      = minimum grid data value
              - max      = maximum grid data value
              - x        = columns index
              - y        = rows index
              - g_ptr    = temporary pointer into grid
              - maxDelta = maximum data value increment used to mark grid
                           faces which exceed maxDensity of contours
              - m_ptr    = temporary pointer into bitmap

****************************************************************************/

int buildMap(CONTOUR_PARAMETERS *con, float *grid, float *min, float *max)
{
    /* DECLARE LOOP COUNTERS AND TEMPORARY POINTERS */

    int status, x, y;
    float *g_ptr, maxDelta;
    unsigned char *m_ptr;

    /* SAVE GRID DIMENSIONS AND POINTER */

    MAX_X = con->columns;
    MAX_Y = con->rows;
    GRID = grid;

    /* ALLOCATE MAP MEMORY */

    MAP = (unsigned char *) malloc (sizeof (unsigned char) * MAX_X * MAX_Y);
    if (MAP)
    {
        /* INITIALIZE BIT MAP */

        memset(MAP, 0, MAX_X * MAX_Y);
  
        /* INITIALIZE MIN/MAX AND DELTA GRID VALUES */

        *min = con->maxValue;
        *max = con->minValue;
        maxDelta = con->interval * con->maxDensity;
        if (con->interval == 0.0) maxDelta = 999999999.0F;
  
        /* LOOP THROUGH EACH GRID CELL, AND SET PROPER BIT MASKS */

        g_ptr = GRID;
        m_ptr = MAP;
        for (y=0; y < MAX_Y; y++)
        {  
            for (x=0; x < MAX_X; x++)
            {
                if (*g_ptr < con->minValue || *g_ptr > con->maxValue)
                {
                    /* SET INVALID BITS, GRID CELL EXCEEDS USER DEFINED BOUNDS */

                    *m_ptr = EW_INVALID | NS_INVALID;
                    if (y > 0) *(m_ptr-MAX_X) = *(m_ptr-MAX_X) | EW_INVALID;
                    if (x > 0) *(m_ptr-1) = *(m_ptr-1) | NS_INVALID;
                }
                else
                {

                    /* SAVE MAXIMUM AND MINIMUM VALID DATA VALUES */

                    if (*g_ptr < *min) *min = *g_ptr;
                    if (*g_ptr > *max) *max = *g_ptr;
                }

                /* SET DENSITY BIT IF DELTA BETWEEN CELLS EXCEEDS USER DEFINED MAX */

                if (x < MAX_X-1)
                if (fabs((double) (*g_ptr - *(g_ptr+1))) > maxDelta)
                    *m_ptr = *m_ptr | NS_DENSITY;
                if (y < MAX_Y-1)
                {
                    if (fabs((double) (*g_ptr - *(g_ptr+MAX_X))) > maxDelta)
                        *m_ptr = *m_ptr | EW_DENSITY;
                }
                g_ptr++;
                m_ptr++;
            }
        }
        status = CONTOUR_TRUE;
    }
    else
    {

        /* UNABLE TO ALLOCATE MAP MEMORY */

        status = CONTOUR_FALSE;
    }
    return status;
}

/***************************************************************************/
/*!

  - Module Name:      markInUse

  - Purpose:          markInUse is called to set the face in use bit for a
                      specific grid face.  This is called for each face that
                      a contour passes through.

  - Inputs:           point
  - Outputs:          none

  - Calling Routines: follow

  - Routines Called:  none

  - Glossary:
      - Static Globals
               - MAP = pointer to the bit map
      - Local
               - point  = pointer to CONTOUR_POINT structure, contains the x,
                          y, and heading defining the face to be marked in
                          use.
               - m_ptr  = pointer to the bit map location defined by point.

****************************************************************************/

void markInUse (CONTOUR_POINT *point)
{
    unsigned char *m_ptr;

    m_ptr = MAP + (point->y * MAX_X) + point->x;

    /* SET BIT BASED ON HEADING, HEADING DEFINITIONS ARE IN globals.h */

    switch (point->heading)
    {
        case CONTOUR_WEST:
            m_ptr++;
        case CONTOUR_EAST:

            /* SET BIT FOR East or West FACE IN USE */

            *m_ptr = *m_ptr | EW_FACE;
            break;

        case CONTOUR_SOUTH:
            m_ptr += MAX_X;

        case CONTOUR_NORTH: 

            /* SET BIT FOR North or South FACE IN USE */

            *m_ptr = *m_ptr | NS_FACE;
            break;
    }
}

/***************************************************************************/
/*!

  - Module Name:      faceInUse

  - Purpose:          Used to check if a contour has passed through the
                      grid face defined by the input parameters.

  - Inputs:           point, face

  - Outputs:          inUse

  - Calling Routines: startContour, follow

  - Routines Called:  none

  - Glossary:
        - MAP     = static global, pointer to bitmap
        - point   = pointer to CONTOUR_POINT structure defining the x,
                    y and heading of the face to check.
        - face    = relative offset from the present point of the face
                    to check.
        - inUse   = TRUE or FALSE value returned
        - heading = true heading after add relative face offset
        - m_ptr   = temporary pointer to bit map location requested

****************************************************************************/

int faceInUse (CONTOUR_POINT *point, int face)
{
    int inUse, heading;
    unsigned char *m_ptr;

    inUse = 0;
  
    m_ptr = MAP + (point->y * MAX_X) + point->x;

    /* CHECK BIT BASED ON HEADING, HEADING DEFINITIONS ARE IN globals.h */

    heading = (point->heading + face) % 4;
    switch (heading)
    {
        case CONTOUR_WEST:
            m_ptr++;

        case CONTOUR_EAST:

            /* CHECK BIT FOR East or West FACE IN USE */

            inUse = *m_ptr & EW_FACE;
            break;

        case CONTOUR_SOUTH:
            m_ptr += MAX_X;

        case CONTOUR_NORTH: 

            /* CHECK BIT FOR North or South FACE IN USE */

            inUse = *m_ptr & NS_FACE;
            break;
    }
    return inUse;
}

/***************************************************************************/
/*!

  - Function Name:      valid

  - Purpose:            valid checks the validity bit in the bit map for the
                        face defined by point.  If the contour is not an
                        emphasis contour the density bit is also checked.

  - Inputs:             point, emph

  - Outputs:            valid

  - Calling Routines:   savePoint

  - Routines Called:    none

  - Glossary:
        - MAP          = static global, poiner to the bitmap
        - point        = pointer to CONTOUR_POINT structure defining
                         the x, y, and heading of the grid face to
                         check.
        - emph         = TRUE if present contour is an emphasis contour
        - valid        = value returned
        - m_ptr        = pointer to bit map location defined by point
        - density_mask = value of the density bit in the bit map

****************************************************************************/

int valid (CONTOUR_POINT *point, int emph)
{
    int valid;
    unsigned char *m_ptr, density_mask;

    valid = 0;
  
    m_ptr = MAP + (point->y * MAX_X) + point->x;
    switch (point->heading)
    {
        case CONTOUR_WEST:
            m_ptr++;

        case CONTOUR_EAST:
            density_mask = (emph) ? 0 : EW_DENSITY;
            valid = !(*m_ptr & (EW_INVALID | density_mask));
            break;

        case CONTOUR_SOUTH:
            m_ptr += MAX_X;

        case CONTOUR_NORTH: 
            density_mask = (emph) ? 0 : NS_DENSITY;
            valid = !(*m_ptr & (NS_INVALID | density_mask));
    }
    return valid;
}

/**************************************************************************/
/*!

  - Function Name:    clearMapFaces

  - Purpose:          This function is call after each contour interval has
                      completely processed.  The function clears the face
                      in use bits for every face in the map.

  - Inputs:           none

  - Outputs:          none

  - Calling Routines: getContour
  - Routines Called:  none

  - Glossary:
        - end_ptr = pointer to the address 1 greater than the
                    end of the bit map
        - m_ptr   = pointer to the present location in the
                    bit map

***************************************************************************/

void clearMapFaces (void)
{
    unsigned char *m_ptr, *end_ptr;

    end_ptr = MAP + (MAX_Y * MAX_X);
    for (m_ptr = MAP; m_ptr < end_ptr; m_ptr++)
        *m_ptr &= CLEAR_FACE;
}


/**************************************************************************/
/*!

  - Function Name:    freeMap

  - Purpose:          Free the memory allocated for the bitmap

  - Inputs:           none

  - Outputs:          none

  - Calling Routines: getContour
  - Routines Called:  none

  - Glossary:
        - MAP = static global, pointer to the bitmap

***************************************************************************/

void freeMap(void)
{
    if (MAP) free(MAP);
}


/**************************************************************************/
/*!

  - Function Name:    getDataPoint

  - Purpose:          This function retrieves the data value from the grid
                      for the point defined by the input parameters. Based on
                      an article by Bruce Giles in Dr. Dobbs Journal, June 1994

  - Inputs:           point, corner

  - Outputs:          grid point value

  - Calling Routines: startContour, follow, savePoint
  - Routines Called:  none

  - Glossary:
        - GRID    = static global pointer to the grid
        - point   = CONTOUR_POINT pointer defining x, y, and
                    heading
        - corner  = relative offset to required grid point from
                    the present point.
        - dx      = column index adjustment based on heading
        - dy      = row index adjustment based on heading
        - heading = final heading after adding the offset

***************************************************************************/

float getDataPoint (CONTOUR_POINT *point, int corner)
{
    int dx,dy;
    int heading;

    dx = 0;
    dy = 0;
  
    heading = (point->heading + corner) % 4;
    switch (heading)
    {
        case CONTOUR_SAME:
            dx = 0;
            dy = 0;
            break;

        case CONTOUR_NEXT:
            dx = 0;
            dy = 1;
            break;

        case CONTOUR_OPPOSITE:
            dx = 1;
            dy = 1;
            break;

        case CONTOUR_ADJACENT:
            dx = 1;
            dy = 0;
            break;
    }
    return *(GRID + ((point->y + dy) * MAX_X) + (point->x + dx));
}


/**************************************************************************/
/*!

  - Function Name:    cellExists

  - Purpose:          Check to ensure that the present point points to a
                      grid cell that exists.

  - Inputs:           point

  - Outputs:          TRUE/FALSE

  - Calling Routines: follow
  - Routines Called:  none

  - Glossary:
        - point   = CONTOUR_POINT pointer define the present grid cell

***************************************************************************/

int cellExists (CONTOUR_POINT *point)
{
    if (point->x < 0 || point->y < 0 || point->x > MAX_X - 2 || point->y >
        MAX_Y - 2)
    {
        return CONTOUR_FALSE;
    }
    else
    {
        return CONTOUR_TRUE;
    }
}

/**************************************************************************/
/*!

  - Function Name:    dumpMap

  - Purpose:          Debugging aid to print bitmap
  - Inputs:           none

  - Outputs:          stdout, Formatted output of the bit map

  - Calling Routines: unknown
  - Routines Called:  none

***************************************************************************/

void dumpMap()
{
    int x,y;
    unsigned char *m_ptr,nf,ef,nv,ev,nd,ed;

    m_ptr = MAP;
    printf("\n MAP DUMP - NS_D EW_D NS_V EW_V NS_F EW_F\n");
    for (y=0; y < MAX_Y; y++)
    {
        printf("ROW %2d  ",y);
        for (x=0; x < MAX_X; x++)
        {
            nf = (*m_ptr & NS_FACE)  ? 'T' : 'F';
            ef = (*m_ptr & EW_FACE)  ? 'T' : 'F';
            nv = (*m_ptr & NS_INVALID) ? 'T' : 'F';
            ev = (*m_ptr & EW_INVALID) ? 'T' : 'F';
            nd = (*m_ptr & NS_DENSITY) ? 'T' : 'F';
            ed = (*m_ptr & EW_DENSITY) ? 'T' : 'F';
            printf("%c%c%c%c%c%c ",nd,ed,nv,ev,nf,ef);
            m_ptr++;
        }
        printf("\n");
    }
}
