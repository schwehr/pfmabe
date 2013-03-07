
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "deleteFile.hpp"
#include "deleteFileHelp.hpp"


/*!
  This is the "Delete/Restore File" dialog that is started from the "Edit" pulldown menu.  When
  you delete a file the function runs through every data point in the PFM, checks it against the
  file number, then, if there is a match, it marks the point to PFM_DELETED.  These points are
  ignored by all PFM programs (as if they didn't exist).  If you restore a file that was previosly
  deleted, the opposite process is run.  This only works on the top layer PFM.
*/

deleteFile::deleteFile (QWidget *parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  running = NVFalse;


  setWindowTitle (tr ("pfmView Delete/Restore Files"));


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  count = get_next_list_file_number (misc->pfm_handle[0]);


  deleteTable = new QTableWidget (count, 2, this);
  deleteTable->setWhatsThis (deleteTableText);
  deleteTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *fnItem = new QTableWidgetItem (tr ("File name"));
  deleteTable->setHorizontalHeaderItem (0, fnItem);
  fnItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *dItem = new QTableWidgetItem (tr ("Delete"));
  deleteTable->setHorizontalHeaderItem (1, dItem);
  dItem->setTextAlignment (Qt::AlignHCenter);


  deleted = (QCheckBox **) calloc (count, sizeof (QCheckBox *));
  vItem = (QTableWidgetItem **) calloc (count, sizeof (QTableWidgetItem *));
  origStatus = (NV_BOOL *) calloc (count, sizeof (NV_BOOL));
  newStatus = (NV_BOOL *) calloc (count, sizeof (NV_BOOL));
  change = (NV_INT16 *) calloc (count, sizeof (NV_INT16));


  deleteGrp = new QButtonGroup (this);
  deleteGrp->setExclusive (FALSE);


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      NV_CHAR filename[512];
      NV_INT16 type;

      read_list_file (misc->pfm_handle[0], i, filename, &type);

      QString vh;
      vh.sprintf ("%d", i);
      vItem[i] = new QTableWidgetItem (vh);
      deleteTable->setVerticalHeaderItem (i, vItem[i]);

      QTableWidgetItem *fn = new QTableWidgetItem (QString (filename));
      deleteTable->setItem (i, 0, fn);
      fn->setFlags (Qt::ItemIsEnabled);

      deleted[i] = new QCheckBox (this);
      deleteTable->setCellWidget (i, 1, deleted[i]);
      deleteGrp->addButton (deleted[i], i);

      if (filename[0] == '*')
        {
          deleted[i]->setCheckState (Qt::Checked);
          origStatus[i] = newStatus[i] = NVTrue;
        }
      else
        {
          deleted[i]->setCheckState (Qt::Unchecked);
          origStatus[i] = newStatus[i] = NVFalse;
        }
      change[i] = -1;
    }
  deleteTable->resizeColumnsToContents ();
  deleteTable->resizeRowsToContents ();


  vbox->addWidget (deleteTable, 0, 0);


  //  Connect after populating table so the slot doesn't get called.

  connect (deleteGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));


  NV_INT32 width = qMin (800, deleteTable->columnWidth (0) + deleteTable->columnWidth (1) + 60);
  NV_INT32 height = qMin (600, deleteTable->rowHeight (0) * count + 75);
  resize (width, height);


  deleteTable->show ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Delete/Restore files"));
  applyButton->setWhatsThis (applyDFText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApply ()));
  actions->addWidget (applyButton);

  closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the Delete/Restore dialog"));
  closeButton->setWhatsThis (closeDFText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  //  Move the dialog to the center of the parent if possible.

  QRect tmp = parent->frameGeometry ();
  NV_INT32 window_x = tmp.x ();
  NV_INT32 window_y = tmp.y ();


  tmp = parent->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSize dlg = sizeHint ();
  NV_INT32 dlg_width = dlg.width ();

  move (window_x + width / 2 - dlg_width / 2, window_y + height / 2);


  show ();
}



deleteFile::~deleteFile ()
{
}



void
deleteFile::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
deleteFile::slotValueChanged (int row)
{
  if (deleted[row]->checkState () == Qt::Checked)
    {
      newStatus[row] = NVTrue;
    }
  else
    {
      newStatus[row] = NVFalse;
    }
}



void
deleteFile::slotApply ()
{
  NV_I32_COORD2       coord;
  DEPTH_RECORD        *depth;
  BIN_RECORD          bin_record;
  NV_INT32            recnum;


  //  Keeps people from multi-clicking the button.  If you try to disable it, you segfault.

  if (running) return;
  running = NVTrue;


  //  Disable the close button since we'll close when we finish.

  closeButton->setEnabled (FALSE);
  qApp->processEvents ();


  //  Check for a change in status.

  NV_INT32 num_files = 0;
  for (NV_INT16 i = 0 ; i < count ; i++)
    {
      if (origStatus[i] != newStatus[i]) 
        {
          change[num_files] = i;
          if (newStatus[i])
            {
              NV_CHAR filename[512];
              NV_INT16 type;

              read_list_file (misc->pfm_handle[0], i, filename, &type);

              delete_list_file (misc->pfm_handle[0], i);

              QString newName = QString (filename);
              newName.append (".pfmView_deleted");

              QFile *file = new QFile (QString (filename));
              file->rename (newName);
            }
          else
            {
              restore_list_file (misc->pfm_handle[0], i);

              NV_CHAR filename[512];
              NV_INT16 type;
              read_list_file (misc->pfm_handle[0], i, filename, &type);

              QString newName = QString (filename);
              QString oldName = newName;
              oldName.append (".pfmView_deleted");

              QFile *file = new QFile (oldName);
              file->rename (newName);
            }
          num_files++;
        }
    }


  //  If we got any file status changes do the dirty deed

  if (num_files)
    {
      NV_BOOL found = NVFalse;


      misc->statusProg->setRange (0, misc->abe_share->open_args[0].head.bin_height);
      misc->statusProgLabel->setText (tr (" Deleting/restoring file(s) "));
      misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
      misc->statusProgLabel->setPalette (misc->statusProgPalette);
      misc->statusProg->setTextVisible (TRUE);
      qApp->processEvents();



      for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[0].head.bin_height ; i++)
        {
          misc->statusProg->setValue (i);
          qApp->processEvents();


          coord.y = i;

          for (NV_INT32 j = 0; j < misc->abe_share->open_args[0].head.bin_width; j++)
            {
              coord.x = j;

              if (!read_depth_array_index (misc->pfm_handle[0], coord, &depth, &recnum))
                {
                  found = NVFalse;
                  for (NV_INT32 m = 0 ; m < recnum ; m++)
                    {
                      for (NV_INT16 k = 0 ; k < num_files ; k++)
                        {
                          if (change[k] == depth[m].file_number)
                            {
                              found = NVTrue;

                              if (newStatus[change[k]])
                                {
                                  depth[m].validity |= PFM_DELETED;
                                }
                              else
                                {
                                  depth[m].validity &= ~PFM_DELETED;
                                }

                              update_depth_record_index (misc->pfm_handle[0], &depth[m]);

                              break;
                            }
                        }
                    }

                  free (depth);
                }


              /*  Recompute the bin values.  */

              if (found) recompute_bin_values_index (misc->pfm_handle[0], coord, &bin_record, 0);
            }
        }


      misc->statusProg->reset ();
      misc->statusProg->setTextVisible (FALSE);
      qApp->processEvents();


      //  Reset the original status to match the new status so if we want to reverse our 
      //  application it will work.  Also reset the filename (leading *).

      NV_CHAR filename[512];
      NV_INT16 type;
      for (NV_INT16 i = 0 ; i < num_files ; i++) 
        {
          origStatus[change[i]] = newStatus[change[i]];

          read_list_file (misc->pfm_handle[0], change[i], filename, &type);

          deleted[change[i]]->setText (QString (filename));
        }


      emit dataChangedSignal ();
    }

  slotClose ();
}



void
deleteFile::slotClose ()
{
  free (deleted);
  free (vItem);
  free (origStatus);
  free (newStatus);
  free (change);

  running = NVFalse;


  close ();
}
