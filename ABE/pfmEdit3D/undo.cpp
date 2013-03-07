
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



#include "pfmEdit3D.hpp"


static NV_BOOL start_flag = NVTrue;


/*!

   - Function :     store_undo

   - Purpose:       Stores a point to an undo block.

   - Arguments:
                    - misc        =  pointer to the MISC structure (which contains the undo info)
                    - undo_levels =  the maximum number of allowable undo blocks
                    - val         =  the single POINT_CLOUD record's validity
                    - num         =  the point number of the POINT_CLOUD record in the POINT_CLOUD array

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
                  QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D store undo"), pfmEdit3D::tr ("Unable to allocate UNDO validity memory!  Reason : %1").arg (strerror (errno)));
                  exit (-1);
                }

              misc->undo[i].num = (NV_U_INT32 *) calloc (misc->undo[i].count, sizeof (NV_U_INT32));
              if (misc->undo[i].num == NULL)
                {
                  QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D store undo"), pfmEdit3D::tr ("Unable to allocate UNDO num memory!  Reason : %1").arg (strerror (errno)));
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
      QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D store undo"), pfmEdit3D::tr ("Unable to allocate UNDO validity memory!  Reason : %1").arg (strerror (errno)));
      exit (-1);
    }

  misc->undo[ucnt].num = (NV_U_INT32 *) realloc (misc->undo[ucnt].num, (cnt + 1) * sizeof (NV_U_INT32));
  if (misc->undo[ucnt].num == NULL)
    {
      QMessageBox::critical (0, pfmEdit3D::tr ("pfmEdit3D store undo"), pfmEdit3D::tr ("Unable to allocate UNDO num memory!  Reason : %1").arg (strerror (errno)));
      exit (-1);
    }

  misc->undo[ucnt].val[cnt] = val;
  misc->undo[ucnt].num[cnt] = num;


  //  Increment the undo count for this undo block.

  misc->undo[ucnt].count++;
}



/*!

   - Function :     end_undo_block

   - Purpose:       Resets the undo start flag so a new block will be started on the next call to store_undo.

   - Arguments:     misc       -  pointer to the MISC structure (which contains the undo info)

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



/*!

   - Function :     undo

   - Purpose:       Undo a block of edit operations.

   - Arguments:     misc       -  pointer to the MISC structure (which contains the undo info)

*/

void undo (MISC *misc)
{
  //  Just making sure we actually have at least 1 undo block.

  if (misc->undo_count)
    {
      NV_INT32 ucnt = misc->undo_count - 1;


      //  Make sure we have undo data saved for this block.

      if (misc->undo[ucnt].count)
        {
          //  Reset each point's validity to whatever we saved.

          for (NV_INT32 i = 0 ; i < misc->undo[ucnt].count ; i++) misc->data[misc->undo[ucnt].num[i]].val = misc->undo[ucnt].val[i];


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



/*!

   - Function :     resize_undo

   - Purpose:       Resizes the undo array (if it is changed in prefs).

   - Arguments:
                    - misc        =  pointer to the MISC structure (which contains the undo info)
                    - undo_levels = new count

   - Returns:       NVTrue if successful, otherwise NVFalse

*/

NV_BOOL resize_undo (MISC *misc, OPTIONS *options, NV_INT32 undo_levels)
{
  //  If we haven't really changed the size, just leave.

  if (undo_levels == options->undo_levels) return (NVFalse);


  //  Don't allow anyone to shrink smaller than the already allocated number of undo blocks.

  if (undo_levels <= misc->undo_count)
    {
      QMessageBox::warning (0, pfmEdit3D::tr ("pfmEdit3D resize undo"), pfmEdit3D::tr ("Resizing to less than already existing undo blocks is not allowed!"));
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
