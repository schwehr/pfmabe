#include "geoSwath3D.hpp"

NV_INT32 pseudo_dist_from_viewer (MISC *misc, POINT_DATA *data, NV_FLOAT64 x, NV_FLOAT64 y);

/***************************************************************************\
*                                                                           *
*   Module Name:        check_bounds                                        *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       July 2000 (On vacation in Disney World, no less)    *
*                                                                           *
*   Purpose:            Check all of the conditions for displaying a beam.  *
*                                                                           *
*   Arguments:          ck       -   NVFalse - no validity check            *
*                       slice    -   NVFalse - no slice check               *
*                                                                           *
*   Return Value:       2 - don't display, 1 - display translucent,         *
*                       0 - display in full color                           *
*                                                                           *
\***************************************************************************/

NV_INT32 check_bounds (OPTIONS *options, MISC *misc, POINT_DATA *data, NV_FLOAT64 x, NV_FLOAT64 y, NV_U_INT32 v, NV_BOOL mask, NV_BOOL ck, NV_BOOL slice)
{
  NV_INT32         iz;


  //  Easy checks first, that way, if we can eliminate this point we don't have to do 
  //  the edge or inside check.  Check for displaying invalid, masked, then displaying class data.

  //  If this point is masked...

  if (mask) return (2);


  //  If the point is invalid (only if we're not displaying invalid).

  if (ck && !options->display_invalid && ((v & PFM_INVAL))) return (2);


  //  Now check the bounds (this will always be an MBR).

  if (x < data->bounds.min_x || x > data->bounds.max_x || y < data->bounds.min_y || y > data->bounds.max_y) return (2);


  //  Check for slicing if it is enabled.

  if (slice)
    {
      iz = pseudo_dist_from_viewer (misc, data, x, y);
      if (iz < misc->slice_min || iz > misc->slice_max) return (1);
    }


  return (0);
}



NV_INT32 check_bounds (OPTIONS *options, MISC *misc, POINT_DATA *data, NV_INT32 index, NV_BOOL ck, NV_BOOL slice)
{
  NV_FLOAT64       x = data->x[index];
  NV_FLOAT64       y = data->y[index];
  NV_U_INT32       v = data->val[index];
  NV_BOOL          m = data->mask[index];

  return (check_bounds (options, misc, data, x, y, v, m, ck, slice));
}
