
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

  - Programmer(s):    Dwight A. Johnson

  - Date Written:     May 1994

  - Module Name:      getExit

  - Purpose:  The present POINT defines the last face of the grid that the
             contour was traced through.  Given the present POINT and the
             present contour interval, this function returns the exit face
             of the contour relative to the present POINT.  This function is
             only called when the contour may exit more than one face. If
             two contours of the same level pass through the same grid cell,
             this routine selects the exit face so the two closest points
             will connect.

  - Inputs:           level, point

  - Outputs:          exit_heading

  - Files Included:   math.h globals.h

  - Calling Routines: follow

  - Routines Called:  faceInUse, getDataPoint

  - Glossary:
                      - exit_heading = relative offset from the present POINT to the
                                       exit face
                      - adjFace = 1 if the adjacent face has been previously used
                      - nextFace =  1 if the next face has been previously used
                      - same = grid value at SAME location defined by POINT
                      - next = grid value at NEXT location defined by POINT
                      - opp  = grid value at OPPOSITE location defined by POINT
                      - adj  = grid value at ADJACENT location defined by POINT
                      - samePt = offset to the present contour value between the same
                                 and next grid values.
                      - nextPt = offset to the present contour value between the next
                                 and opposite grid values.
                      - oppPt  = offset to the present contour value between the
                                 opposite and adjacent grid values.
                      - adjPt  = offset to the present contour value between the same
                                 and adjacent grid values.
                      - same2next = length of line between same and next points
                      - next2opp  = length of line between next and opposite points
                      - opp2adj   = length of line between opposite and adjacent
                      - adj2same  = length of line between adjacent and same points

****************************************************************************/

#include <math.h>
#include "globals.hpp"

/* PROTOTYPES FROM BITMAP.C */

float getDataPoint (CONTOUR_POINT *point, int corner);
int faceInUse (CONTOUR_POINT *point, int corner);


int getExit (float level, CONTOUR_POINT *point)
{
    int exit_heading, adjFace, nextFace;
    float same,next,opp,adj;
    float samePt,nextPt,oppPt,adjPt;
    float same2next,next2opp,opp2adj,adj2same;

    adjFace = faceInUse(point,CONTOUR_ADJACENT);
    nextFace = faceInUse(point,CONTOUR_NEXT);
    if (nextFace && adjFace)
    {
        /* BOTH FACES USED, CONTOUR WILL CLOSE, EXIT FACE UNKNOWN */

        exit_heading = CONTOUR_UNKNOWN;
    }
    else if (adjFace)
    {
        exit_heading = CONTOUR_NEXT;
    }
    else if (nextFace)
    {
        exit_heading = CONTOUR_ADJACENT;
    }
    else
    {
        /* GET FOUR CORNERS OF CELL */

        same = getDataPoint(point,CONTOUR_SAME);
        next = getDataPoint(point,CONTOUR_NEXT);
        opp  = getDataPoint(point,CONTOUR_OPPOSITE);
        adj  = getDataPoint(point,CONTOUR_ADJACENT);

        /* COMPUTE OFFSET TO CONTOUR LEVEL FOR EACH FACE */

        samePt = (level-same)/(next-same);
        nextPt = (level-opp)/(next-opp);
        oppPt  = (level-opp)/(adj-opp);
        adjPt  = (level-same)/(adj-same);

        /* COMPUTE DISTANCE OF EACH LINE */

        same2next = (float) sqrt((double)((1-samePt)*(1-samePt))
            +((1-nextPt)*(1-nextPt)));
        next2opp  = (float) sqrt((double)(oppPt * oppPt) + (nextPt * nextPt));
        opp2adj   = (float) sqrt((double)((1-oppPt)*(1-oppPt)) + ((1-adjPt)*(1-adjPt)));
        adj2same  = (float) sqrt((double)(samePt * samePt) + (adjPt * adjPt));

        /* SELECT EXIT HEADING BASED IN SHORTEST LINE */

        if (same2next < next2opp)
        {
            if (same2next < adj2same)
            {
                exit_heading = CONTOUR_NEXT;
            }
            else
            {
                exit_heading = CONTOUR_ADJACENT;
            }
        }
        else
        {
            if (next2opp < opp2adj)
            {
                exit_heading = CONTOUR_ADJACENT;
            }
            else
            {
                exit_heading = CONTOUR_NEXT;
            }
        }
    }
    return exit_heading;
}
