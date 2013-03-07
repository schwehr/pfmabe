
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


/***************************************************************************/
/*!

  - Module Name:        put_buffer

  - Programmer(s):      Jan C. Depner

  - Date Written:       May 1999

  - Purpose:            Write the modified poinits from the point cloud
                        back to the file and recompute the bin values.

  - Inputs:

  - Return Value:
                        - 0 if nothing has been modified
                        - 1 if something has been modified

****************************************************************************/

NV_INT32 put_buffer (MISC *misc)
{
  NV_I32_COORD2   coord;
  NV_INT32        mod_flag;
  DEPTH_RECORD    depth;
  BIN_RECORD      bin;
  NV_U_BYTE       *bin_map;



  mod_flag = 0;


  //  If this was started in read only mode, don't save anything!

  if (!misc->abe_share->read_only)
    {
      misc->statusProg->setRange (0, misc->abe_share->point_cloud_count);
      misc->statusProgLabel->setText (pfmEdit::tr ("Saving edits"));
      misc->statusProgLabel->setVisible (TRUE);
      misc->statusProg->setTextVisible (TRUE);
      qApp->processEvents();


      for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
        {
          QString progText = pfmEdit::tr ("Saving edits for ") +
            QFileInfo (QString (misc->abe_share->open_args[pfm].list_path)).fileName ().remove (".pfm");

          misc->statusProgLabel->setText (progText);
          misc->statusProg->reset ();


          //  Compute the width and the height.

          NV_INT32 width = (misc->ur[pfm].x - misc->ll[pfm].x) + 1;
          NV_INT32 height = (misc->ur[pfm].y - misc->ll[pfm].y) + 1;


          if (width > 0  && height > 0)
            {
              //  Allocate and clear the bin map.

              bin_map = (NV_U_BYTE *) malloc (width * height);
              memset (bin_map, 0, width * height);


              //  This seems a bit silly but it takes a long time to spin through the data
              //  with a QProgressDialog running so we're only updating it at 10% intervals.

              NV_INT32 inc = misc->abe_share->point_cloud_count / 10;
              if (!inc) inc = 1;


              for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
                {
                  if (!(i % inc))
                    {
                      misc->statusProg->setValue (i);
                      qApp->processEvents();
                    }


                  //  Only deal with points that are in the current PFM (i.e. misc->abe_share->open_args[pfm])

                  if (misc->data[i].pfm == pfm)
                    {
                      //  If the validity has changed since this point was read in we need to save it.

                      if (misc->data[i].oval != misc->data[i].val)
                        {
                          //  Set the coordinate x and y value (trying to compute it from position is not 
                          //  accurate).

                          depth.coord.x = misc->data[i].xcoord;
                          depth.coord.y = misc->data[i].ycoord;


                          //  Get the address, file number, record number, and subrecord number to uniquely identify 
                          //  the sounding.

                          depth.address.block = misc->data[i].addr;
                          depth.address.record = misc->data[i].pos;
                          depth.file_number = misc->data[i].file;
                          depth.ping_number = misc->data[i].rec;
                          depth.beam_number = misc->data[i].sub;


                          //  Set the validity bits.

                          depth.validity = misc->data[i].val | PFM_MODIFIED;


                          //  Update the validity.

                          update_depth_record_index (misc->pfm_handle[pfm], &depth);


                          //  Set the bin map modified flag.

                          bin_map[(depth.coord.y - misc->ll[pfm].y) * width + (depth.coord.x - misc->ll[pfm].x)] = 1;

                          mod_flag = 1;
                        }
                    }
                }


              misc->statusProg->reset ();
              misc->statusProgLabel->setVisible (FALSE);
              misc->statusProg->setTextVisible (FALSE);
              qApp->processEvents();


              //  Recompute the bin values for those bins that were modified.

              for (coord.y = misc->ll[pfm].y ; coord.y <= misc->ur[pfm].y ; coord.y++)
                {
                  for (coord.x = misc->ll[pfm].x ; coord.x <= misc->ur[pfm].x ; coord.x++)
                    {
                      if (bin_map[(coord.y - misc->ll[pfm].y) * width + (coord.x - misc->ll[pfm].x)])
                        {
                          bin.validity |= PFM_DATA;
                          recompute_bin_values_index (misc->pfm_handle[pfm], coord, &bin, PFM_DATA);
                        }
                    }
                }

              free (bin_map);
            }
        }
    }


  return (mod_flag);
}
