
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



#include "pfmEdit.hpp"


//!  This function activates the action key for an external ancillary program.

void actionKey (pfmEdit *parent, nvMap *map, OPTIONS *options, MISC *misc, QString key, NV_INT32 lock_point)
{
  NV_INT32 set_point;


  misc->abeShare->lock ();


  //  We set the modcode to 0 (i.e. nothing has changed)

  misc->abe_share->modcode = 0;


  //  We then set the action key for the external program (it should be monitoring
  //  misc->abe_share->key for it to change).

  misc->abe_share->key = (NV_U_INT32) key.toAscii()[0];


  misc->abeShare->unlock ();


  //  Then we wait at least one second for something to be done in the external program

  NV_BOOL hit = NVFalse;
  QTime t;
  t.start();
  do
    {
      //  The external program should set modcode to it's data type to signify that it has 
      //  done something

      if (misc->abe_share->modcode)
        {
          if (misc->abe_share->modcode == NO_ACTION_REQUIRED) break;


          //  After the external program sets modcode we want to lock shared memory.

          misc->abeShare->lock ();


          NV_INT32 x, y, z;


          //  Unfortunately this is data type specific, which I really hate to put in here,
          //  but we have no other choice that I can see.  At the moment we're only doing
          //  this with CHARTS LIDAR data.

          switch (misc->abe_share->modcode)
            {
            case PFM_SHOALS_1K_DATA:
            case PFM_CHARTS_HOF_DATA:
              if ((NV_U_INT32) misc->data[lock_point].type == misc->abe_share->modcode && misc->data[lock_point].file == 
                  misc->abe_share->modified_file_num && misc->data[lock_point].rec == misc->abe_share->modified_record &&
		  misc->data[lock_point].sub == misc->abe_share->modified_subrecord)
                {
                  //  The external program sets the modified point number

                  set_point = misc->abe_share->modified_point;
                  misc->data[set_point].z = misc->abe_share->modified_value;
                  misc->data[set_point].exflag = NVTrue;

                  if (!misc->view)
                    {
                      NV_FLOAT64 dummy = (NV_FLOAT64) misc->data[set_point].z;
                      map->map_to_screen (1, &misc->data[set_point].x, &misc->data[set_point].y, &dummy, &x, &y, &z);
                    }
                  else
                    {
                      scale_view (map, misc, misc->data[set_point].x, misc->data[set_point].y, misc->data[set_point].z, &x, &y, &z);
                    }

                  drawx (map, x, y, options->contour_color, HIGHLIGHTED, NVTrue);

                  misc->highlight = (NV_INT32 *) realloc (misc->highlight, (misc->highlight_count + 1) * sizeof (NV_INT32));
                  if (misc->highlight == NULL)
                    {
                      perror ("Allocating misc->highlight in actionKey.cpp");
                      exit (-1);
                    }

                  misc->highlight[misc->highlight_count] = set_point;
                  misc->highlight_count++;

                  hit = NVTrue;
                }
              break;


              //  The infamous chartsPic "I saved the picture" flag  :-D

            case CHARTSPIC_SAVED_PICTURE:

              if (misc->bfd_open)
                {
                  BFDATA_RECORD bfd_record;

                  if ((binaryFeatureData_read_record (misc->bfd_handle, misc->feature[misc->nearest_feature].record_number, &bfd_record)) >= 0)
                    {
                      bfd_record.heading = misc->abe_share->heading;

                      binaryFeatureData_write_record_image_file (misc->bfd_handle, misc->feature[misc->nearest_feature].record_number,
								 &bfd_record, NULL, misc->abe_share->snippet_file_name);

                      remove (misc->abe_share->snippet_file_name);
                    }
                }

              break;
            }

          misc->abe_share->key = 0;


          //  Unlock it after we have processed the data.

          misc->abeShare->unlock ();


          break;
        }
    } while (t.elapsed () < 1000);


  if (hit) 
    {
      parent->enableClearHighlight (NVTrue);
      parent->redrawMap (NVTrue);
    }
}
