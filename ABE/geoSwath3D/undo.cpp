#include "geoSwath3D.hpp"


static NV_BOOL start_flag = NVTrue;


/*

    Function :     store_undo

    Purpose:       Stores a point to an undo block.

    Arguments:     misc        -  pointer to the MISC structure (which contains the undo info)
                   undo_levels -  the maximum number of allowable undo blocks
                   val         -  the single POINT_DATA record's validity
                   num         -  the point number of the POINT_DATA record in the POINT_DATA array

*/

void store_undo (MISC *misc, NV_INT32 undo_levels, NV_U_INT32 val, NV_U_INT32 num)
{
  //  Check to see if we're starting a new undo block.

  if (start_flag)
    {
      //  If we maxed out on the undo blocks we need to roll off the oldest one.  Yes, I know, I'm using a brute force
      //  method of rolling the memory around.  If you really go into a single edit session and do more than
      //  one hundred block edits you probably need to be shot anyway.

      if (misc->undo_count == undo_levels)
        {
          for (NV_INT32 i = 0 ; i < undo_levels - 1 ; i++)
            {
              if (misc->undo[i].count)
                {
                  free (misc->undo[i].val);
                  free (misc->undo[i].num);
                }

              misc->undo[i].count = misc->undo[i + 1].count;

              misc->undo[i].val = (NV_U_INT32 *) calloc (misc->undo[i].count, sizeof (NV_U_INT32));
              if (misc->undo[i].val == NULL)
                {
                  QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D store undo"), geoSwath3D::tr ("Unable to allocate UNDO validity memory!  Reason : %1").arg (strerror (errno)));
                  exit (-1);
                }

              misc->undo[i].num = (NV_U_INT32 *) calloc (misc->undo[i].count, sizeof (NV_U_INT32));
              if (misc->undo[i].num == NULL)
                {
                  QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D store undo"), geoSwath3D::tr ("Unable to allocate UNDO num memory!  Reason : %1").arg (strerror (errno)));
                  exit (-1);
                }

              memcpy (misc->undo[i].val, misc->undo[i + 1].val, misc->undo[i].count * sizeof (NV_U_INT32));
              memcpy (misc->undo[i].num, misc->undo[i + 1].num, misc->undo[i].count * sizeof (NV_U_INT32));
            }

          if (misc->undo[undo_levels - 1].count)
            {
              free (misc->undo[undo_levels - 1].val);
              free (misc->undo[undo_levels - 1].num);

              misc->undo[undo_levels - 1].val = NULL;
              misc->undo[undo_levels - 1].num = NULL;

              misc->undo[undo_levels - 1].count = 0;
            }


          //  Set the undo count to the last available undo block since we're going to re-use it.

          misc->undo_count = undo_levels - 1;
        }


      //  Reset the start flag.

      start_flag = NVFalse;
    }


  //  Allocate the needed memory and store the points validity and number.

  NV_INT32 ucnt = misc->undo_count;
  NV_INT32 cnt = misc->undo[ucnt].count;

  misc->undo[ucnt].val = (NV_U_INT32 *) realloc (misc->undo[ucnt].val, (cnt + 1) * sizeof (NV_U_INT32));
  if (misc->undo[ucnt].val == NULL)
    {
      QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D store undo"), geoSwath3D::tr ("Unable to allocate UNDO validity memory!  Reason : %1").arg (strerror (errno)));
      exit (-1);
    }

  misc->undo[ucnt].num = (NV_U_INT32 *) realloc (misc->undo[ucnt].num, (cnt + 1) * sizeof (NV_U_INT32));
  if (misc->undo[ucnt].num == NULL)
    {
      QMessageBox::critical (0, geoSwath3D::tr ("geoSwath3D store undo"), geoSwath3D::tr ("Unable to allocate UNDO num memory!  Reason : %1").arg (strerror (errno)));
      exit (-1);
    }

  misc->undo[ucnt].val[cnt] = val;
  misc->undo[ucnt].num[cnt] = num;


  //  Increment the undo count for this undo block.

  misc->undo[ucnt].count++;
}



/*

    Function :     end_undo_block

    Purpose:       Resets the undo start flag so a new block will be started on the next call to store_undo.

    Arguments:     misc       -  pointer to the MISC structure (which contains the undo info)

*/

void end_undo_block (MISC *misc)
{
  //  Check the count of the current block to make sure points were added.  If not, do nothing.

  if (misc->undo[misc->undo_count].count)
    {
      //  Make the pointer point to the next available undo block.

      misc->undo_count++;
      start_flag = NVTrue;
    }
}



/*

    Function :     undo

    Purpose:       Undo a block of edit operations.

    Arguments:     misc       -  pointer to the MISC structure (which contains the undo info)
                   data       -  pointer to the POINT_DATA array

*/

void undo (MISC *misc, POINT_DATA *data)
{
  //  Just making sure we actually have at least 1 undo block.

  if (misc->undo_count)
    {
      NV_INT32 ucnt = misc->undo_count - 1;


      //  Make sure we have undo data saved for this block.

      if (misc->undo[ucnt].count)
        {
          //  Reset each point's validity to whatever we saved.

          for (NV_INT32 i = 0 ; i < misc->undo[ucnt].count ; i++) data->val[misc->undo[ucnt].num[i]] = misc->undo[ucnt].val[i];


          //  Free the undo memory and reset the count.

          free (misc->undo[ucnt].val);
          free (misc->undo[ucnt].num);

          misc->undo[ucnt].val = NULL;
          misc->undo[ucnt].num = NULL;

          misc->undo[ucnt].count = 0;
        }


      //  Decrement the undo block counter.

      misc->undo_count--;
    }
}



/*

    Function :     resize_undo

    Purpose:       Resizes the undo array (if it is changed in prefs).

    Arguments:     misc        -  pointer to the MISC structure (which contains the undo info)
                   undo_levels - new count

    Returns:       NVTrue if successful, otherwise NVFalse

*/

NV_BOOL resize_undo (MISC *misc, OPTIONS *options, NV_INT32 undo_levels)
{
  //  If we haven't really changed the size, just leave.

  if (undo_levels == options->undo_levels) return (NVFalse);


  //  Don't allow anyone to shrink smaller than the already allocated number of undo blocks.

  if (undo_levels <= misc->undo_count)
    {
      QMessageBox::warning (0, geoSwath3D::tr ("geoSwath3D resize undo"), geoSwath3D::tr ("Resizing to less than already existing undo blocks is not allowed!"));
      return (NVFalse);
    }


  //  If it is shrinking, get rid of the overage.

  if (undo_levels < options->undo_levels)
    {
      for (NV_INT32 i = undo_levels ; i < options->undo_levels ; i++)
        {
          if (misc->undo[i].count)
            {
              free (misc->undo[i].val);
              free (misc->undo[i].num);
            }
        }
    }

              
  misc->undo = (UNDO *) realloc (misc->undo, undo_levels * sizeof (UNDO));
  if (misc->undo == NULL)
    {
      perror ("Allocating undo block pointers in undo.cpp");
      exit (-1);
    }


  //  If it is growing, we need to clear the new memory (cause realloc won't).

  if (undo_levels > options->undo_levels)
    {
      for (NV_INT32 i = options->undo_levels ; i < undo_levels ; i++)
        {
          misc->undo[i].val = NULL;
          misc->undo[i].num = NULL;
          misc->undo[i].count = 0;
        }
    }


  //  Finally, set the number of undo levels.

  options->undo_levels = undo_levels;


  return (NVTrue);
}
