#include "geoSwath3DDef.hpp"

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
