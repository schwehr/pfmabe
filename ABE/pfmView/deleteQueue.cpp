
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



#include "deleteQueue.hpp"
#include "deleteQueueHelp.hpp"


/*!
  This is the "Delete Queue File" dialog that is started from the "Edit" pulldown menu.  When
  you press the Accept button the function runs through every data point in the PFM(s), checks it against the
  PFM and file numbers in the queue, then, if there is a match, it marks the point as PFM_DELETED.  These points are
  ignored by all PFM programs (as if they didn't exist).
*/

deleteQueue::deleteQueue (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  running = NVFalse;


  setWindowTitle (tr ("pfmView Delete File Queue"));


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  queueTable = new QTableWidget (misc->abe_share->delete_file_queue_count, 2, this);
  queueTable->setWhatsThis (queueTableText);
  queueTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *pnItem = new QTableWidgetItem (tr ("PFM layer"));
  queueTable->setHorizontalHeaderItem (0, pnItem);
  pnItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *fnItem = new QTableWidgetItem (tr ("File name"));
  queueTable->setHorizontalHeaderItem (1, fnItem);
  fnItem->setTextAlignment (Qt::AlignHCenter);
  queueTable->verticalHeader ()->hide ();


  for (NV_INT32 i = 0 ; i < misc->abe_share->delete_file_queue_count ; i++)
    {
      NV_CHAR filename[512];
      NV_INT16 type;

      QString pfmname = QString (pfm_basename (misc->abe_share->open_args[misc->abe_share->delete_file_queue[i][0]].list_path));
      QTableWidgetItem *pn = new QTableWidgetItem (pfmname);
      queueTable->setItem (i, 0, pn);
      pn->setFlags (Qt::ItemIsEnabled);


      read_list_file (misc->pfm_handle[misc->abe_share->delete_file_queue[i][0]], misc->abe_share->delete_file_queue[i][1], filename, &type);


      QTableWidgetItem *fn = new QTableWidgetItem (QString (filename));
      queueTable->setItem (i, 1, fn);
      fn->setFlags (Qt::ItemIsEnabled);
    }
  queueTable->resizeColumnsToContents ();
  queueTable->resizeRowsToContents ();


  vbox->addWidget (queueTable, 0, 0);


  NV_INT32 width = qMin (800, queueTable->columnWidth (0) + queueTable->columnWidth (1) + 60);
  NV_INT32 height = qMin (600, queueTable->rowHeight (0) * misc->abe_share->delete_file_queue_count + 75);
  resize (width, height);


  queueTable->show ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Delete files in queue list"));
  acceptButton->setWhatsThis (acceptDQText);
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotAccept ()));
  actions->addWidget (acceptButton);

  rejectButton = new QPushButton (tr ("Reject"), this);
  rejectButton->setToolTip (tr ("Close this dialog without deleting files"));
  rejectButton->setWhatsThis (rejectDQText);
  connect (rejectButton, SIGNAL (clicked ()), this, SLOT (slotReject ()));
  actions->addWidget (rejectButton);

  clearButton = new QPushButton (tr ("Clear"), this);
  clearButton->setToolTip (tr ("Clear all files from the delete file queue"));
  clearButton->setWhatsThis (clearDQText);
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClear ()));
  actions->addWidget (clearButton);


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



deleteQueue::~deleteQueue ()
{
}



void
deleteQueue::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
deleteQueue::slotAccept ()
{
  NV_I32_COORD2       coord;
  DEPTH_RECORD        *depth;
  BIN_RECORD          bin_record;
  NV_INT32            recnum, **pfm_list, *file_count;


  //  Keeps people from multi-clicking the button.  If you try to disable it, you segfault.

  if (running) return;
  running = NVTrue;


  //  Disable the reject and clear buttons since we'll deal with those when we finish.

  rejectButton->setEnabled (FALSE);
  clearButton->setEnabled (FALSE);
  qApp->processEvents ();


  //  Make up the list of PFMs and associated file numbers and counts.

  pfm_list = (NV_INT32 **) calloc (misc->abe_share->pfm_count, sizeof (NV_INT32 *));
  if (pfm_list == NULL)
    {
      perror ("Allocating pfm_list in deleteQueue.cpp");
      exit (-1);
    }

  file_count = (NV_INT32 *) calloc (misc->abe_share->pfm_count, sizeof (NV_INT32));
  if (file_count == NULL)
    {
      perror ("Allocating file_count in deleteQueue.cpp");
      exit (-1);
    }

  for (NV_INT32 i = 0 ; i < misc->abe_share->delete_file_queue_count ; i++)
    {
      pfm_list[misc->abe_share->delete_file_queue[i][0]] =
        (NV_INT32 *) realloc (pfm_list[misc->abe_share->delete_file_queue[i][0]],
                              (file_count[misc->abe_share->delete_file_queue[i][0]] + 1) * sizeof (NV_INT32));

      if (pfm_list[misc->abe_share->delete_file_queue[i][0]] == NULL)
        {
          perror ("Allocating pfm_list[] in deleteQueue.cpp");
          exit (-1);
        }

      pfm_list[misc->abe_share->delete_file_queue[i][0]][file_count[misc->abe_share->delete_file_queue[i][0]]] = misc->abe_share->delete_file_queue[i][1];

      file_count[misc->abe_share->delete_file_queue[i][0]]++;
    }


  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (file_count[pfm])
        {
          NV_BOOL found = NVFalse;

          misc->statusProg->setRange (0, misc->abe_share->open_args[pfm].head.bin_height);
          misc->statusProgLabel->setText (tr (" Deleting file(s) "));
          misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
          misc->statusProgLabel->setPalette (misc->statusProgPalette);
          misc->statusProg->setTextVisible (TRUE);
          qApp->processEvents();

          for (NV_INT32 i = 0 ; i < misc->abe_share->open_args[pfm].head.bin_height ; i++)
            {
              misc->statusProg->setValue (i);
              qApp->processEvents();


              coord.y = i;

              for (NV_INT32 j = 0; j < misc->abe_share->open_args[pfm].head.bin_width; j++)
                {
                  coord.x = j;

                  if (!read_depth_array_index (misc->pfm_handle[pfm], coord, &depth, &recnum))
                    {
                      found = NVFalse;
                      for (NV_INT32 m = 0 ; m < recnum ; m++)
                        {
                          for (NV_INT16 k = 0 ; k < file_count[pfm] ; k++)
                            {
                              if (pfm_list[pfm][k] == depth[m].file_number)
                                {
                                  found = NVTrue;

                                  depth[m].validity |= PFM_DELETED;

                                  update_depth_record_index (misc->pfm_handle[pfm], &depth[m]);

                                  break;
                                }
                            }
                        }

                      free (depth);
                    }


                  /*  Recompute the bin values.  */

                  if (found) recompute_bin_values_index (misc->pfm_handle[pfm], coord, &bin_record, 0);
                }

              misc->statusProg->reset ();
              misc->statusProg->setTextVisible (FALSE);
              qApp->processEvents();
            }


          //  Mark the files as deleted and then rename the actual files so that they won't get used again.

          for (NV_INT16 k = 0 ; k < file_count[pfm] ; k++)
            {
              NV_CHAR filename[512];
              NV_INT16 type;

              read_list_file (misc->pfm_handle[pfm], pfm_list[pfm][k], filename, &type);

              delete_list_file (misc->pfm_handle[pfm], pfm_list[pfm][k]);

              QString newName = QString (filename);
              newName.append (".pfmView_deleted");

              QFile *file = new QFile (QString (filename));
              file->rename (newName);
            }
        }
    }


  //  Free the allocated memory.

  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (file_count[pfm]) free (pfm_list[pfm]);
    }
  if (file_count) free (file_count);
  if (pfm_list) free (pfm_list);


  slotClear ();
}



void
deleteQueue::slotReject ()
{
  running = NVFalse;

  close ();
}



void
deleteQueue::slotClear ()
{
  running = NVFalse;

  misc->abe_share->delete_file_queue_count = 0;

  emit dataChangedSignal ();

  close ();
}
