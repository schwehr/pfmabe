
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



#include "pfmEditDef.hpp"


/*!
  This function compares the entered point (current point) to the stack of geographically nearest
  points and then puts it into the proper place in the stack if it is nearer to the cursor than
  one, some, or all of the points already in the stack of nearest points.
*/

NV_BOOL compare_to_stack (NV_INT32 current_point, NV_FLOAT64 dist, MISC *misc)
{
  NV_INT32 hit_previous = 0;
  for (NV_INT32 j = MAX_STACK_POINTS - 1 ; j >= 0 ; j--)
    {
      if (dist < misc->nearest_stack.dist[j])
        {
          if (!j)
            {
              for (NV_INT32 k = MAX_STACK_POINTS - 1 ; k > 0 ; k--)
                {
                  misc->nearest_stack.dist[k] = misc->nearest_stack.dist[k - 1];
                  misc->nearest_stack.point[k] = misc->nearest_stack.point[k - 1];
                }

              misc->nearest_stack.point[0] = current_point;
              misc->nearest_stack.dist[0] = dist;


              //  Return NVTrue since this is the  minimum distance so far.

              return (NVTrue);
            }

          hit_previous = j;
        }
      else
        {
          if (hit_previous)
            {
              for (NV_INT32 k = MAX_STACK_POINTS - 1 ; k > hit_previous ; k--)
                {
                  misc->nearest_stack.dist[k] = misc->nearest_stack.dist[k - 1];
                  misc->nearest_stack.point[k] = misc->nearest_stack.point[k - 1];
                }

              misc->nearest_stack.point[hit_previous] = current_point;
              misc->nearest_stack.dist[hit_previous] = dist;

              break;
            }
        }
    }


  //  If we got here we didn't replace the minimum distance point.

  return (NVFalse);
}
