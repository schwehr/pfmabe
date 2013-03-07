
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



#include "examGSF.hpp"
#include "examGSFHelp.hpp"


NV_FLOAT64 settings_version = 1.0;


static QProgressDialog *prog;


static void indexProgress (int state, int percent)
{
  static NV_BOOL second = NVFalse;

  if (state == 1) second = NVFalse;

  if (!second && state == 2) 
    {
      prog->setLabelText ("Writing index file...");
      second = NVTrue;
    }
      
  if (state == 2 || percent < 100) prog->setValue (percent);
  qApp->processEvents ();
}



examGSF::examGSF (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/exam_gsf.xpm"));


  filearg = NVFalse;
  rec_arg_num = -1;
  display_other = NVFalse;
  record_num = 1;
  beam_recs = NULL;
  other_recs = NULL;
  prof_size = 200;


  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"file", required_argument, 0, 0},
                                             {"record", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:
          NV_CHAR tmp[512];
          sscanf (optarg, "%s", tmp);

          switch (option_index)
            {
            case 0:
              filename = QString (optarg);
              filearg = NVTrue;
              break;

            case 1:
              sscanf (tmp, "%d", &record_num);
              rec_arg_num = record_num;
              break;
            }
          break;
        }
    }


  //  This is so we can drag and drop files on the Desktop icon.

  if (*argc == 2 && !filearg && rec_arg_num == -1)
    {
      filename = QString (argv[1]);
      filearg = NVTrue;
    }


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("examGSF main toolbar"));


  envin ();


  //  Set the window size and location from the defaults

  this->resize (options.width, options.height);
  this->move (options.window_x, options.window_y);


  a0 = 6378137.0;
  b0 = 6356752.314245;


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  gsfTable = new QTableWidget (PAGE_SIZE, GSF_COLUMNS, frame);
  gsfTable->setWhatsThis (gsfTableText);
  vheader = gsfTable->verticalHeader ();
  vheader->setToolTip (tr ("Click on the record number to display the beam data"));
  connect (vheader, SIGNAL (sectionClicked (int)), this, SLOT (slotBeamDataClicked (int)));


  setHeaders ();


  //  This is not intuitively obvious to the most casual observer.  All events for gsfTable will
  //  first be sent to examGSF's event filter.  We're doing this to catch key events so we can 
  //  move the whole thing up and down (our scroll bar) instead of just the QTable's scroll bar.

  gsfTable->installEventFilter (this);


  vhItem = new QTableWidgetItem*[PAGE_SIZE];


  for (NV_INT32 i = 0 ; i < PAGE_SIZE ; i++) 
    {
      QString hh;
      hh.setNum (i + 1);
      vhItem[i] = new QTableWidgetItem (hh);
      gsfTable->setVerticalHeaderItem (i, vhItem[i]);
    }


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  scrollBar = new QScrollBar (Qt::Vertical, frame);
  scrollBar->setTracking (FALSE);
  scrollBar->setToolTip (tr ("Move through the file (Page Down or Page Up)"));
  scrollBar->setWhatsThis (scrollBarText);
  connect (scrollBar, SIGNAL (valueChanged (int)), this, SLOT (slotScrollBarValueChanged (int)));
  connect (scrollBar, SIGNAL (sliderMoved (int)), this, SLOT (slotScrollBarSliderMoved (int)));


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (scrollBar);
  hBox->addWidget (gsfTable);
  hBox->setStretchFactor (gsfTable, 1);
  vBox->addLayout (hBox);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  bOpen = new QToolButton (this);
  bOpen->setIcon (QIcon (":/icons/fileopen.xpm"));
  bOpen->setToolTip (tr ("Open GSF file"));
  bOpen->setWhatsThis (openText);
  connect (bOpen, SIGNAL (clicked ()), this, SLOT (slotOpen ()));
  tools->addWidget (bOpen);


  bOther = new QToolButton (this);
  bOther->setIcon (QIcon (":/icons/other.xpm"));
  bOther->setToolTip (tr ("Toggle display of non-ping GSF records"));
  bOther->setWhatsThis (otherText);
  bOther->setCheckable (TRUE);
  bOther->setChecked (FALSE);
  bOther->setEnabled (FALSE);
  connect (bOther, SIGNAL (clicked ()), this, SLOT (slotOther ()));
  tools->addWidget (bOther);


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.xpm"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  tools->addAction (bHelp);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QAction *fileOpenAction = new QAction (tr ("&Open"), this);
  fileOpenAction->setShortcut (tr ("Ctrl+O"));
  fileOpenAction->setStatusTip (tr ("Open GSF file"));
  connect (fileOpenAction, SIGNAL (triggered ()), this, SLOT (slotOpen ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);
  fileMenu->addAction (fileOpenAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about examGSF"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("A&cknowledgements"), this);
  acknowledgements->setShortcut (tr ("Ctrl+c"));
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


  filLabel = new QLabel ("                                ", this);
  filLabel->setAlignment (Qt::AlignLeft);
  filLabel->setMinimumSize (filLabel->sizeHint ());
  filLabel->setToolTip (tr ("File name"));


  verLabel = new QLabel ("                ", this);
  verLabel->setAlignment (Qt::AlignCenter);
  verLabel->setMinimumSize (verLabel->sizeHint ());
  verLabel->setToolTip (tr ("GSF version"));

  sizLabel = new QLabel ("                ", this);
  sizLabel->setAlignment (Qt::AlignCenter);
  sizLabel->setMinimumSize (sizLabel->sizeHint ());
  sizLabel->setToolTip (tr ("File size"));

  senLabel = new QLabel ("                       ", this);
  senLabel->setAlignment (Qt::AlignCenter);
  senLabel->setMinimumSize (senLabel->sizeHint ());
  senLabel->setToolTip (tr ("Sensor"));

  statusBar ()->addWidget (filLabel, 1);
  statusBar ()->addWidget (verLabel);
  statusBar ()->addWidget (sizLabel);
  statusBar ()->addWidget (senLabel);
  statusBar ()->setSizeGripEnabled (FALSE);


  gsf_handle = -1;


  if (filearg) openFile ();
}



examGSF::~examGSF ()
{
  slotQuit ();
}



//  We're catching the key press events so that page up, down, etc don't move the gsfTable
//  scroll bar (if present) but move our main scroll bar.  Same with the wheel events.

bool 
examGSF::eventFilter (QObject *o, QEvent *e)
{
  if (e->type () == QEvent::Wheel)
    {
      QWheelEvent *wheelEvent = (QWheelEvent *) e;

      if (wheelEvent->delta () < 0)
        {
          scrollBar->triggerAction (QAbstractSlider::SliderPageStepAdd);
        }
      else
        {
          scrollBar->triggerAction (QAbstractSlider::SliderPageStepSub);
        }
      return (TRUE);
    }

  if (e->type () == QEvent::KeyPress)
    {
      QKeyEvent *keyEvent = (QKeyEvent *) e;

      if (keyEvent->key () == Qt::Key_Up || keyEvent->key () == Qt::Key_Down || keyEvent->key () == Qt::Key_PageUp || 
          keyEvent->key () == Qt::Key_PageDown || keyEvent->key () == Qt::Key_Home || keyEvent->key () == Qt::Key_End)
        {
          keyPressEvent (keyEvent);
          return (TRUE);
        }
    }

  return QMainWindow::eventFilter (o, e);
}



void 
examGSF::keyPressEvent (QKeyEvent *e)
{
  switch (e->key ())
    {
    case Qt::Key_Up:
      scrollBar->triggerAction (QAbstractSlider::SliderSingleStepAdd);
      break;

    case Qt::Key_PageUp:
      scrollBar->triggerAction (QAbstractSlider::SliderPageStepSub);
      break;

    case Qt::Key_Down:
      scrollBar->triggerAction (QAbstractSlider::SliderSingleStepSub);
      break;

    case Qt::Key_PageDown:
      scrollBar->triggerAction (QAbstractSlider::SliderPageStepAdd);
      break;

    case Qt::Key_Home:
      scrollBar->setValue (record_num = 1);
      break;

    case Qt::Key_End:
      scrollBar->setValue (record_num = numrecs);
      break;
    }
}



//  A bunch of slots.

void 
examGSF::slotQuit ()
{
  envout ();

  exit (0);
}



void 
examGSF::slotOpen ()
{
  QStringList files, filters;


  QFileDialog *fd = new QFileDialog (this, tr ("examGSF Open GSF File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options.input_dir);


  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("All files (*)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("GSF (*.d\?\? *.gsf)"));


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      filename = files.at (0);


      if (filename.isEmpty())
        {
          QMessageBox::warning (this, tr ("Open GSF file"), tr ("A file must be selected!"));
          return;
        }


      options.input_dir = fd->directory ().absolutePath ();


      record_num = 1;

      if (gsf_handle != -1) gsfClose (gsf_handle);

      openFile ();
    }
}



void 
examGSF::openFile ()
{
  QString ndx_file;

  ndx_file = filename;
  ndx_file.replace (ndx_file.length () - 3, 1, "n");

  FILE *fp;

  if ((fp = fopen (ndx_file.toAscii (), "r")) == NULL)
    {
      gsf_register_progress_callback (indexProgress);


      prog = new QProgressDialog (tr ("Reading GSF file..."), 0, 0, 100, this);

      prog->setWindowTitle (tr ("Index GSF File"));
      prog->setWindowModality (Qt::WindowModal);
      prog->show ();

      qApp->processEvents();

      if (gsfOpen (filename.toAscii (), GSF_READONLY_INDEX, &gsf_handle)) 
        {
          QMessageBox::warning (this, tr ("Open GSF File"), tr ("Unable to index GSF file"));
          return;
        }
      gsfClose (gsf_handle);

      setCursor (Qt::PointingHandCursor);
    }
  else
    {
      fclose (fp);
    }


  if (gsfOpen (filename.toAscii (), GSF_READONLY_INDEX, &gsf_handle)) 
    {
      QMessageBox::warning (this, tr ("Open GSF File"), tr ("Unable to open GSF file"));
      return;
    }


  //  Get the GSF version and set it in the status bar

  gsfSeek (gsf_handle, GSF_REWIND);
  gsfRead (gsf_handle, GSF_NEXT_RECORD, &gsf_data_id, &gsf_record, NULL, 0);
  verLabel->setText (QString (gsf_record.header.version));
  gsfSeek (gsf_handle, GSF_REWIND);


  //  Set the filename in the status bar

  filLabel->setText (filename);


  //  Get the file size and set it in the status bar

  QFileInfo fi (filename);
  QString tmp;
  tmp = QString ("%1").arg (fi.size ());
  sizLabel->setText (tmp);


  //  Get the number of swath bathy ping records in the file.

  numrecs = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING);
  digits = (NV_INT32) (log10 ((NV_FLOAT64) numrecs)) + 1;


  //  We have to save the record number because setMinValue causes a valueChanged callback which then resets
  //  record_num.

  NV_INT32 save_rec = record_num;
  scrollBar->setMinimum (1);
  scrollBar->setMaximum (numrecs);
  scrollBar->setSingleStep (1);
  scrollBar->setPageStep (PAGE_SIZE - 5);
  record_num = save_rec;
  scrollBar->setValue (record_num);


  //  Make the otherRecs dialog

  other_recs = new otherRecs (this, gsf_handle, &options, &misc);
  connect (other_recs, SIGNAL (findPingSignal (NV_INT32)), this, SLOT (slotFindPing (NV_INT32)));
  connect (other_recs, SIGNAL (closedSignal ()), this, SLOT (slotOtherRecsClosed ()));


  //  Get the sensor type and set it in the status bar.

  memset (&gsf_record, 0, sizeof (gsfRecords));

  gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
  gsf_data_id.record_number = 1;
  if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0) < 0) 
    {
      QString errmsg;

      errmsg.sprintf (tr ("Error reading record 1 : %s").toAscii (), gsfStringError ());

      QMessageBox::warning (this, tr ("Read GSF File"), errmsg);

      return;
    }


  //  Add sensor specific columns to the table and set the data type;

  switch (gsf_record.mb_ping.sensor_id)
    {
    case GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 1);
      tmp = "SeaBeam";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC:
      tmp = "Simrad EM12";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 8);
      tmp = "Simrad EM100";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 6);
      tmp = "Simrad EM950";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 6);
      tmp = "Simrad EM1000";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 8);
      tmp = "Simrad EM121A";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 9);
      tmp = "Simrad EM121";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC:
      tmp = "SeaMap";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 6);
      if (gsf_record.mb_ping.sensor_data.gsfSeaBatSpecific.mode & GSF_SEABAT_9002)
        {
          tmp = "Reson SeaBat 9002";
        }
      else if (gsf_record.mb_ping.sensor_data.gsfSeaBatSpecific.mode & GSF_SEABAT_9003)
        {
          tmp = "Reson SeaBat 9003";
        }
      else
        {
          tmp = "Reson SeaBat 9001";
        }
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 3);
      tmp = "SeaBeam (w/amp)";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 8);
      if (gsf_record.mb_ping.sensor_data.gsfSeaBatIISpecific.mode & GSF_SEABAT_9002)
        {
          tmp = "Reson SeaBat 9002";
        }
      else if (gsf_record.mb_ping.sensor_data.gsfSeaBatSpecific.mode & GSF_SEABAT_9003)
        {
          tmp = "Reson SeaBat 9003";
        }
      else
        {
          tmp = "Reson SeaBat 9001";
        }
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 11);
      tmp = "Reson SeaBat 8101";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 27);
      tmp = "Reson 8111";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 27);
      tmp = "Reson 8124";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 27);
      tmp = "Reson 8125";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 27);
      tmp = "Reson 8150";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 27);
      tmp = "Reson 8160";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC:
      tmp = "SeaBeam 2112/36";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 6);
      tmp = "ELAC MK II";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 300";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 1002";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 3000";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 120";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 3002";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 3000D";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 35);
      tmp = "SIMRAD EM 3002D";
      break;

    case GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC:
    case GSF_SWATH_BATHY_SB_SUBRECORD_ECHOTRAC_SPECIFIC:
      tmp = "Echotrac";
      break;

    case GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC:
    case GSF_SWATH_BATHY_SB_SUBRECORD_BATHY2000_SPECIFIC:
      tmp = "Bathy 2000";
      break;

    case GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC:
    case GSF_SWATH_BATHY_SB_SUBRECORD_MGD77_SPECIFIC:
      tmp = "MGD77";
      break;

    case GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC:
    case GSF_SWATH_BATHY_SB_SUBRECORD_BDB_SPECIFIC:
      tmp = "BDB";
      break;

    case GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC:
    case GSF_SWATH_BATHY_SB_SUBRECORD_NOSHDB_SPECIFIC:
      tmp = "NOSHDB";
      break;

    case GSF_SWATH_BATHY_SB_SUBRECORD_PDD_SPECIFIC:
      tmp = "PDD";
      break;

    case GSF_SWATH_BATHY_SB_SUBRECORD_NAVISOUND_SPECIFIC:
      tmp = "NAVISOUND";
      break;

    case GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 2);
      tmp = "Compressed SASS";
      break;

    default:
      tmp = "unknown";
    }
  senLabel->setText (tmp);

  readData (record_num);

  bOther->setEnabled (TRUE);
}



void 
examGSF::readData (NV_INT32 record)
{
  extern int gsfError;

  start = record - HALF_PAGE;

  end = record + HALF_PAGE;

  if (start < 1) 
    {
      start = 1;
      end = (start + PAGE_SIZE);
      if (end > numrecs) end = numrecs;
    }

  if (end > numrecs)
    {
      end = numrecs + 1;
      start = (end - PAGE_SIZE);
      if (start < 1) start = 1;
    }


  NV_INT32 i = start;
  while (i < end)
    {
      memset (&gsf_record, 0, sizeof (gsfRecords));

      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = i;

      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0) < 0) 
        {
          if (gsfError == GSF_READ_TO_END_OF_FILE) break;

          QString errmsg;

          errmsg.sprintf (tr ("Error reading record %d : %s").toAscii (), i, gsfStringError ());

          QMessageBox::warning (this, tr ("Read GSF File"), errmsg);

          break;
        }

      populateTable (i - start, gsf_data_id, gsf_record);

      i++;
    }

  gsfTable->resizeColumnsToContents ();
  gsfTable->resizeRowsToContents ();

  NV_INT32 width = 0;
  for (NV_INT32 i = 0 ; i < GSF_COLUMNS ; i++) width += gsfTable->columnWidth (i);
  width = qMin (1200, width + 120);

  NV_INT32 height = gsfTable->rowHeight (0) * PAGE_SIZE;
  height = qMin (800, height + 200);

  resize (width, height);


  //  If we had a record on the command line and it's in the window, highlight it.

  if (rec_arg_num <= end && rec_arg_num >= start) gsfTable->selectRow (rec_arg_num - start);
}



void 
examGSF::populateTable (NV_INT32 row, gsfDataID dataID, gsfRecords record)
{
  NV_CHAR hem, format[5];
  NV_FLOAT64 deg, min, sec;
  NV_INT32 cnt = 0;
  QString tmp, tmp2, timeString, latString = " ", lonString = " ", dataString = " ";
  NV_INT32 year, day, hour, minute, month, mday;
  NV_FLOAT32 second;


  sprintf (format, "%%0%dd", digits);
  tmp.sprintf (format, dataID.record_number);
  vhItem[row]->setText (tmp);


  cvtime (record.mb_ping.ping_time.tv_sec, record.mb_ping.ping_time.tv_nsec, &year, &day, &hour, &minute, &second);
  jday2mday (year, day, &month, &mday);
  month++;

  tmp.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f", year + 1900, month, mday, day, hour, minute, second);
  gsfTable->setItem (row, 0, new QTableWidgetItem (tmp));


  latString.sprintf ("%s", fixpos (record.mb_ping.latitude, &deg, &min, &sec, &hem, POS_LAT, options.position_form));
  lonString.sprintf ("%s", fixpos (record.mb_ping.longitude, &deg, &min, &sec, &hem, POS_LON, options.position_form
));

  tmp = latString + " " + lonString;
  gsfTable->setItem (row, 1, new QTableWidgetItem (tmp));


  cnt = 0;
  for (NV_INT32 i = 0 ; i < record.mb_ping.number_beams ; i++)
    {
      if (record.mb_ping.beam_flags)
        {
          if (!(record.mb_ping.beam_flags[i] & GSF_IGNORE_BEAM)) cnt++;
        }
      else if (record.mb_ping.depth)
        {
          if (record.mb_ping.depth[i] != GSF_NULL_DEPTH) cnt++;
        }
    }

  tmp.sprintf ("%d", cnt);
  gsfTable->setItem (row, 2, new QTableWidgetItem (tmp));
  tmp.sprintf ("%d", record.mb_ping.number_beams);
  gsfTable->setItem (row, 3, new QTableWidgetItem (tmp));
  tmp.sprintf ("%d", record.mb_ping.center_beam + 1);
  gsfTable->setItem (row, 4, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.heading);
  gsfTable->setItem (row, 5, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.pitch);
  gsfTable->setItem (row, 6, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.roll);
  gsfTable->setItem (row, 7, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.heave);
  gsfTable->setItem (row, 8, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.course);
  gsfTable->setItem (row, 9, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.speed);
  gsfTable->setItem (row, 10, new QTableWidgetItem (tmp));
  tmp.sprintf ("%.2f", record.mb_ping.tide_corrector);
  gsfTable->setItem (row, 11, new QTableWidgetItem (tmp));

  NV_INT32 tide_type = (record.mb_ping.ping_flags & (GSF_PING_USER_FLAG_15 | GSF_PING_USER_FLAG_14)) >> 14;

  switch (tide_type)
    {
    case 0:
      tmp = "None";
      break;

    case 1:
      tmp = "Predicted";
      break;

    case 2:
      tmp = "Observed";
      break;

    case 3:
      tmp = "Verified";
      break;
    }
  gsfTable->setItem (row, 12, new QTableWidgetItem (tmp));

  tmp.sprintf ("%.2f", record.mb_ping.depth_corrector * misc.depth_factor[options.depth_units]);
  gsfTable->setItem (row, 13, new QTableWidgetItem (tmp));
  tmp.sprintf ("%x", record.mb_ping.ping_flags);
  gsfTable->setItem (row, 14, new QTableWidgetItem (tmp));


  //  Populate the sensor specific records.

  switch (record.mb_ping.sensor_id)
    {
    case GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC:
      gsfTable->setColumnCount (GSF_COLUMNS + 1);
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Eclipse\nTime")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBeamSpecific.EclipseTime);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      break;
      
    case GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ship\nPitch")));
      tmp.sprintf ("%4.2f", record.mb_ping.sensor_data.gsfEM100Specific.ship_pitch);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Transducer\nPitch")));
      tmp.sprintf ("%4.2f", record.mb_ping.sensor_data.gsfEM100Specific.transducer_pitch);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Power")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.power);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Attenuation")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.attenuation);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Time Varying\nGain")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.tvg);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Pulse\nLength")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.pulse_length);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Counter")));
      tmp.sprintf ("%3d", record.mb_ping.sensor_data.gsfEM100Specific.counter);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Valid\nBeams")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.valid_beams);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Pulse\nLength")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.pulse_length);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Beam\nWidth")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.beam_width);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Transmit\nPower")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.tx_power);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Transmit\nStatus")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.tx_status);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Receive\nStatus")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121ASpecific.rx_status);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 8, new QTableWidgetItem (tr ("Surface\nVelocity")));
      tmp.sprintf ("%7.3f", record.mb_ping.sensor_data.gsfEM121ASpecific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 8, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Valid\nBeams")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.valid_beams);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Pulse\nLength")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.pulse_length);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Beam\nWidth")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.beam_width);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Transmit\nPower")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.tx_power);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Transmit\nStatus")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.tx_status);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Receive\nStatus")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM121Specific.rx_status);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 8, new QTableWidgetItem (tr ("Surface\nVelocity")));
      tmp.sprintf ("%7.3f", record.mb_ping.sensor_data.gsfEM121Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 8, new QTableWidgetItem (tmp));
                   
      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatSpecific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatSpecific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Sonar\nRange")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatSpecific.sonar_range);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Transmit\nPower")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatSpecific.transmit_power);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Receive\nGain")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatSpecific.receive_gain);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Surface\nVelocity")));
      tmp.sprintf ("%7.3f", record.mb_ping.sensor_data.gsfSeaBatSpecific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM950Specific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM950Specific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Ping\nQuality")));
      tmp.sprintf ("%d", abs (record.mb_ping.sensor_data.gsfEM950Specific.ping_quality));
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Ship\nPitch")));
      tmp.sprintf ("%5.2f", record.mb_ping.sensor_data.gsfEM950Specific.ship_pitch);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Transducer\nPitch")));
      tmp.sprintf ("%5.2f", record.mb_ping.sensor_data.gsfEM950Specific.transducer_pitch);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Surface\nVelocity")));
      tmp.sprintf ("%7.3f", record.mb_ping.sensor_data.gsfEM950Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Eclipse Time")));
      tmp.sprintf ("%2.2d:%2.2d:%2.2d.%2.2d", record.mb_ping.sensor_data.gsfSBAmpSpecific.hour,
                   record.mb_ping.sensor_data.gsfSBAmpSpecific.minute,
                   record.mb_ping.sensor_data.gsfSBAmpSpecific.second,
                   record.mb_ping.sensor_data.gsfSBAmpSpecific.hundredths);

      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Block\nNumber")));
      tmp.sprintf ("%5d", record.mb_ping.sensor_data.gsfSBAmpSpecific.block_number);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Average Gate\nDepth\n") +
                                                                                misc.depth_string[options.depth_units]));
      tmp.sprintf ("%.2f", record.mb_ping.sensor_data.gsfSBAmpSpecific.avg_gate_depth * 
        misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatIISpecific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Surface Velocity (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBatIISpecific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%04x", record.mb_ping.sensor_data.gsfSeaBatIISpecific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Sonar Range\n(meters)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatIISpecific.sonar_range);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Transmit\nPower")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatIISpecific.transmit_power);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr (">Receive\nGain")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBatIISpecific.receive_gain);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Fore/Aft\nBeam Width")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBatIISpecific.fore_aft_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Athwartships\nBeam Width")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBatIISpecific.athwart_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Surface\nVelocity (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBat8101Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%04x", record.mb_ping.sensor_data.gsfSeaBat8101Specific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Range\n(meters)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.range);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));


      if (record.mb_ping.sensor_data.gsfSeaBat8101Specific.power & 0x80)
        {
          gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Power\n(auto)")));
          tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.power & ~0x80);
        }
      else
        {
          gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Power\n(manual)")));
          tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.power & ~0x80);
        }
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));


      if (record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & 0x8000)
        {
          if (record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & 0x4000)
            {
              gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5,
                                                 new QTableWidgetItem (tr ("Gain (auto,\ntime varying)")));
              tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & ~0xC000);
            }
          else
            {
              gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Gain\n(auto, fixed)")));
              tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & ~0xC000);
            }
        }
      else
        {
          if (record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & 0x4000)
            {
              gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5,
                                                 new QTableWidgetItem (tr ("Gain (manual,\ntime varying)")));
              tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & ~0xC000);
            }
          else
            {
              gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Gain (manual,\nfixed)")));
              tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.gain & ~0xC000);
            }
        }
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Pulse Width\n(us)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.pulse_width);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Time Varying\nGain Spread\n(db)")));
      tmp.sprintf ("%5.2f", (NV_FLOAT64) record.mb_ping.sensor_data.gsfSeaBat8101Specific.tvg_spreading * 0.25);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 8, new QTableWidgetItem (tr ("Time Varying\nGain Absorption")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBat8101Specific.tvg_absorption);
      gsfTable->setItem (row, GSF_COLUMNS + 8, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 9, new QTableWidgetItem (tr ("Fore/Aft\nBeam Width")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBat8101Specific.fore_aft_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 9, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 10, new QTableWidgetItem (tr ("Atwartships\nBeam Width")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBat8101Specific.athwart_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 10, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%04x", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Surface\nVelocity (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("SSV\nSource")));
      tmp.sprintf ("%c", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.ssv_source);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Ping\nGain (db)")));
      tmp.sprintf ("%02d", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.ping_gain);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Pulse\nWidth (ms)")));
      tmp.sprintf ("%02d", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.pulse_width);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Transmitter\nAttenuation (db)")));
      tmp.sprintf ("%02d", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.transmitter_attenuation);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Number of\nAlgorithms")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.number_algorithms);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Algorithm\nOrder")));
      tmp.sprintf ("%s", record.mb_ping.sensor_data.gsfSeaBeam2112Specific.algorithm_order);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Mode")));
      tmp.sprintf ("%04x", record.mb_ping.sensor_data.gsfElacMkIISpecific.mode);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfElacMkIISpecific.ping_num);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Sound\nVelocity (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfElacMkIISpecific.sound_vel * 0.1);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Pulse\nLength (ms)")));
      tmp.sprintf ("%0.2f", record.mb_ping.sensor_data.gsfElacMkIISpecific.pulse_length * 0.01);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Starboard Receiver\nGain (db)")));
      tmp.sprintf ("%02d", record.mb_ping.sensor_data.gsfElacMkIISpecific.receiver_gain_stbd);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Port Receiver\nGain (db)")));
      tmp.sprintf ("%02d", record.mb_ping.sensor_data.gsfElacMkIISpecific.receiver_gain_port);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Model")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.model_number);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Serial\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.serial_number);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Surface\nVelocity (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfEM3Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Transducer\nDepth\n") + 
                                                                                misc.depth_string[options.depth_units]));
      tmp.sprintf ("%0.2f", record.mb_ping.sensor_data.gsfEM3Specific.transducer_depth *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Valid\nBeams")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.valid_beams);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Sample\nRate (Hz)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.sample_rate);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Depth\nDifference\n") +
                                                                                misc.depth_string[options.depth_units]));
      tmp.sprintf ("%0.2f", record.mb_ping.sensor_data.gsfEM3Specific.depth_difference *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 8, new QTableWidgetItem (tr ("Offset\nMultiplier")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.offset_multiplier);
      gsfTable->setItem (row, GSF_COLUMNS + 8, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 9, new QTableWidgetItem (tr ("RUN\nTIME\nPARAMETERS")));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 10, new QTableWidgetItem (tr ("Model")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].model_number);
      gsfTable->setItem (row, GSF_COLUMNS + 10, new QTableWidgetItem (tmp));


      cvtime (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].dg_time.tv_sec, 
              record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].dg_time.tv_nsec, &year, &day, &hour, 
              &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 11, new QTableWidgetItem (tr ("Time")));
      tmp.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f", year + 1900, month, mday, day, hour, minute, second);
      gsfTable->setItem (row, GSF_COLUMNS + 11, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 12, new QTableWidgetItem (tr ("Ping\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 12, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 13, new QTableWidgetItem (tr ("Serial\nNumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].serial_number);
      gsfTable->setItem (row, GSF_COLUMNS + 13, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 14, new QTableWidgetItem (tr ("System\nStatus")));
      tmp.sprintf ("%08x", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].system_status);
      gsfTable->setItem (row, GSF_COLUMNS + 14, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 15, new QTableWidgetItem (tr ("Mode")));
      switch (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].mode)
        {
        case 0:
          if ((record.mb_ping.sensor_data.gsfEM3Specific.model_number >= 3000) && 
              (record.mb_ping.sensor_data.gsfEM3Specific.model_number <= 3008))
            {
              tmp = tr ("Nearfield");
            }
          else
            {
              tmp = tr ("Very Shallow");
            }
          break;

        case 1:
          if ((record.mb_ping.sensor_data.gsfEM3Specific.model_number >= 3000) && 
              (record.mb_ping.sensor_data.gsfEM3Specific.model_number <= 3008))
            {
              tmp = tr ("Normal");
            }
          else
            {
              tmp = tr ("Shallow");
            }
          break;

        case 2:
          if ((record.mb_ping.sensor_data.gsfEM3Specific.model_number >= 3000) && 
              (record.mb_ping.sensor_data.gsfEM3Specific.model_number <= 3008))
            {
              tmp = tr ("Target");
            }
          else
            {
              tmp = tr ("Medium");
            }
          break;

        case 3:
          tmp = tr ("Deep");
          break;

        case 4:
          tmp = tr ("Very Deep");
          break;

        default:
          tmp = tr ("Unknown");
          break;
        }
      gsfTable->setItem (row, GSF_COLUMNS + 15, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 16, new QTableWidgetItem (tr ("Filter")));
      tmp.sprintf ("%02x", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].filter_id);
      gsfTable->setItem (row, GSF_COLUMNS + 16, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 17, new QTableWidgetItem (tr ("Min\nDepth\n") +
                                                                                 misc.depth_string[options.depth_units]));
      tmp.sprintf ("%5.0f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].min_depth *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 17, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 18, new QTableWidgetItem (tr ("Max\nDepth\n") +
                                                                                 misc.depth_string[options.depth_units]));
      tmp.sprintf ("%5.0f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].max_depth *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 18, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 19, new QTableWidgetItem (tr ("Absorption\n(dB/km)")));
      tmp.sprintf ("%.2f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].absorption);
      gsfTable->setItem (row, GSF_COLUMNS + 19, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 20, new QTableWidgetItem (tr ("Pulse\nLength (us)")));
      tmp.sprintf ("%.0f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].pulse_length);
      gsfTable->setItem (row, GSF_COLUMNS + 20, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 21, new QTableWidgetItem (tr ("Transmit Beam\nWidth (deg)")));
      tmp.sprintf ("%.1f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].transmit_beam_width);
      gsfTable->setItem (row, GSF_COLUMNS + 21, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 22, new QTableWidgetItem (tr ("Transmit Power\nReduction (dB)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].power_reduction);
      gsfTable->setItem (row, GSF_COLUMNS + 22, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 23, new QTableWidgetItem (tr ("Receive\nBeamwidth (deg)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].receive_beam_width);
      gsfTable->setItem (row, GSF_COLUMNS + 23, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 24, new QTableWidgetItem (tr ("Receive\nBandwidth (Hz)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].receive_bandwidth);
      gsfTable->setItem (row, GSF_COLUMNS + 24, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 25, new QTableWidgetItem (tr ("Receive\nGain (dB)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].receive_gain);
      gsfTable->setItem (row, GSF_COLUMNS + 25, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 26, 
                                         new QTableWidgetItem (tr ("Time Varying\nGain Cross-Over\nAngle (deg)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].cross_over_angle);
      gsfTable->setItem (row, GSF_COLUMNS + 26, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 27, new QTableWidgetItem (tr ("SSV\nSource")));
      switch (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].ssv_source)
        {
        case 0:
          tmp = tr ("From Probe");
          break;
        case 1:
          tmp = tr ("Manual");
          break;
        case 2:
          tmp = tr ("From Profile");
          break;
        default:
          tmp = tr ("Unknown");
          break;
        }
      gsfTable->setItem (row, GSF_COLUMNS + 27, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 28, new QTableWidgetItem (tr ("Max Port\nSwath Width (m)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].port_swath_width);
      gsfTable->setItem (row, GSF_COLUMNS + 28, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 29, new QTableWidgetItem (tr ("Beam\nSpacing")));
      switch (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].beam_spacing)
        {
        case 0:
          tmp = tr ("Beamwidth");
          break;

        case 1:
          if (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].hilo_freq_absorp_ratio == 0)
            {
              tmp = tr ("Equiangle");
            }
          else
            {
              tmp = tr ("Equidistant");
            }
          break;

        case 2:
          if (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].hilo_freq_absorp_ratio == 0)
            {
              tmp = tr ("Equidistant");
            }
          else
            {
              tmp = tr ("Equiangle");
            }
          break;

        case 3:
          tmp = tr ("Intermediate");
          break;

        default:
          tmp = tr ("Unknown");
        }
      gsfTable->setItem (row, GSF_COLUMNS + 29, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 30, new QTableWidgetItem (tr ("Port Coverage\nSector (deg)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].port_coverage_sector);
      gsfTable->setItem (row, GSF_COLUMNS + 30, new QTableWidgetItem (tmp));


      tmp2 = tr ("None");
      if (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].stabilization & 0x80) tmp2 = tr ("Pitch");

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 31, new QTableWidgetItem (tr ("Stabilization")));
      switch (record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].stabilization & (~0x80))
        {
        case 0:
        default:
          tmp = tmp2;
          break;

        case 1:
          tmp = tmp2 + tr (" yaw to line azimuth");
          break;

        case 2:
          tmp = tmp2 + tr (" yaw to mean heading");
          break;

        case 3:
          tmp = tmp2 + tr ("yaw to manual heading");
          break;
        }
      gsfTable->setItem (row, GSF_COLUMNS + 31, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 32,
                                         new QTableWidgetItem (tr ("Starboard\nCoverage\nSector (deg)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].stbd_coverage_sector);
      gsfTable->setItem (row, GSF_COLUMNS + 32, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 33,
                                         new QTableWidgetItem (tr ("Max Starboard\nSwath Width\n(meters)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].stbd_swath_width);
      gsfTable->setItem (row, GSF_COLUMNS + 33, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 34, new QTableWidgetItem (tr ("HiLo Freq\nAbsorption Ratio")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfEM3Specific.run_time[0].hilo_freq_absorp_ratio);
      gsfTable->setItem (row, GSF_COLUMNS + 34, new QTableWidgetItem (tmp));

      break;

    case GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, 
                                         new QTableWidgetItem (tr ("Sea Surface\nSound Velocity\n(lfreq) (m/s)")));
      tmp.sprintf ("%0.1f", record.mb_ping.sensor_data.gsfCmpSassSpecific.lfreq);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("lntens")));
      tmp.sprintf ("%0.2f", record.mb_ping.sensor_data.gsfCmpSassSpecific.lntens);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      break;


      //  I am assuming that this record is used for all of these types.  There is nothing in the
      //  documentation covering it so if it doesn't work - too bad.

    case GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC:
    case GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC:
      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 0, new QTableWidgetItem (tr ("Latency\n(ms)")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfReson8100Specific.latency);
      gsfTable->setItem (row, GSF_COLUMNS + 0, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 1, new QTableWidgetItem (tr ("Ping\nnumber")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfReson8100Specific.ping_number);
      gsfTable->setItem (row, GSF_COLUMNS + 1, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 2, new QTableWidgetItem (tr ("Sonar\nID")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfReson8100Specific.sonar_id);
      gsfTable->setItem (row, GSF_COLUMNS + 2, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 3, new QTableWidgetItem (tr ("Sonar\nModel")));
      tmp.sprintf ("%d", record.mb_ping.sensor_data.gsfReson8100Specific.sonar_model);
      gsfTable->setItem (row, GSF_COLUMNS + 3, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 4, new QTableWidgetItem (tr ("Frequency\n(KHz)")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.frequency);
      gsfTable->setItem (row, GSF_COLUMNS + 4, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 5, new QTableWidgetItem (tr ("Surface\nVelocity")));
      tmp.sprintf ("%.2f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.surface_velocity);
      gsfTable->setItem (row, GSF_COLUMNS + 5, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 6, new QTableWidgetItem (tr ("Sample\nRate (Hz)")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.sample_rate);
      gsfTable->setItem (row, GSF_COLUMNS + 6, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 7, new QTableWidgetItem (tr ("Ping Rate\n(pings/sec * 1000)")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.ping_rate);
      gsfTable->setItem (row, GSF_COLUMNS + 7, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 8, new QTableWidgetItem (tr ("Mode")));
      if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_WIDE_MODE)
        {
          tmp = tr ("Wide");
        }
      else if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_TWO_HEADS)
        {
          tmp = tr ("Two Heads");
        }
      else if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_STBD_HEAD)
        {
          tmp = tr ("Starboard Head (2)");
        }
      else if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_AMPLITUDE)
        {
          tmp = tr ("Beam amplitude available");
        }
      gsfTable->setItem (row, GSF_COLUMNS + 8, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 9, new QTableWidgetItem (tr ("Mode")));
      if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_PITCH_STAB)
        {
          tmp = tr ("Pitch Stabilized");
        }
      else if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.mode && GSF_8100_ROLL_STAB)
        {
          tmp = tr ("Roll Stabilized");
        }
      gsfTable->setItem (row, GSF_COLUMNS + 9, new QTableWidgetItem (tmp));


      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 10, new QTableWidgetItem (tr ("Range\n(meters)")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.range);
      gsfTable->setItem (row, GSF_COLUMNS + 10, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 11, new QTableWidgetItem (tr ("Power")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.power);
      gsfTable->setItem (row, GSF_COLUMNS + 11, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 12, new QTableWidgetItem (tr ("Gain")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.gain);
      gsfTable->setItem (row, GSF_COLUMNS + 12, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 13, new QTableWidgetItem (tr ("Pulse\nWidth (us)")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.pulse_width);
      gsfTable->setItem (row, GSF_COLUMNS + 13, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 14,
                                         new QTableWidgetItem (tr ("Time Varying\nGain Spreading\nCoefficient")));
      tmp.sprintf ("%.2f", (NV_FLOAT32) gsf_record.mb_ping.sensor_data.gsfReson8100Specific.tvg_spreading / 4.0);
      gsfTable->setItem (row, GSF_COLUMNS + 14, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 15,
                                         new QTableWidgetItem (tr ("Time Varying\nGain Absorption\nCoefficient")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.tvg_absorption);
      gsfTable->setItem (row, GSF_COLUMNS + 15, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 16, new QTableWidgetItem (tr ("Fore/Aft\nBeam width\n(degrees)")));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.fore_aft_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 16, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 17,
                                         new QTableWidgetItem (tr ("Athwartships\nBeam Width\n(degrees)")));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.athwart_bw);
      gsfTable->setItem (row, GSF_COLUMNS + 17, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 18, new QTableWidgetItem (tr ("Projector\nType")));
      tmp.sprintf ("%d", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.projector_type);
      gsfTable->setItem (row, GSF_COLUMNS + 18, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 19, new QTableWidgetItem (tr ("Projector\nAngle\n(degrees)")));
      tmp.sprintf ("%f", (NV_FLOAT32) gsf_record.mb_ping.sensor_data.gsfReson8100Specific.projector_angle / 10.0);
      gsfTable->setItem (row, GSF_COLUMNS + 19, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 20,
                                         new QTableWidgetItem (tr ("Minimum Range\nFilter Value\n(meters)")));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.range_filt_min);
      gsfTable->setItem (row, GSF_COLUMNS + 20, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 21,
                                         new QTableWidgetItem (tr ("Maximum Range\nFilter Value\n(meters)")));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.range_filt_max);
      gsfTable->setItem (row, GSF_COLUMNS + 21, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 22, new QTableWidgetItem (tr ("Minimum Depth\nFilter Value\n") +
                                                                                 misc.depth_string[options.depth_units]));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.depth_filt_min *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 22, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 23, new QTableWidgetItem (tr ("Maximum Depth\nFilter Value\n") +
                                                                                 misc.depth_string[options.depth_units]));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.depth_filt_max *
                   misc.depth_factor[options.depth_units]);
      gsfTable->setItem (row, GSF_COLUMNS + 23, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 24, new QTableWidgetItem (tr ("Active\nFilter")));
      if (gsf_record.mb_ping.sensor_data.gsfReson8100Specific.filters_active & 0x01)
        {
          tmp = tr ("Range");
        }
      else
        {
          tmp = tr ("Depth");
        }
      gsfTable->setItem (row, GSF_COLUMNS + 24, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 25, new QTableWidgetItem (tr ("Temperature\n(degrees C)")));
      tmp.sprintf ("%f", (NV_FLOAT32) gsf_record.mb_ping.sensor_data.gsfReson8100Specific.temperature / 10.0);
      gsfTable->setItem (row, GSF_COLUMNS + 25, new QTableWidgetItem (tmp));

      gsfTable->setHorizontalHeaderItem (GSF_COLUMNS + 26, new QTableWidgetItem (tr ("Beam\nSpacing")));
      tmp.sprintf ("%f", gsf_record.mb_ping.sensor_data.gsfReson8100Specific.beam_spacing);
      gsfTable->setItem (row, GSF_COLUMNS + 26, new QTableWidgetItem (tmp));

      break;


    default:
      break;
    }
}



void 
examGSF::slotOther ()
{
  if (display_other)
    {
      other_recs->hide ();
      display_other = NVFalse;
    }
  else
    {
      other_recs->show ();
      display_other = NVTrue;
    }
}



void 
examGSF::slotFindPing (NV_INT32 record)
{
  record_num = record;
  scrollBar->setValue (record_num);
  gsfTable->selectRow (record_num - start);
}



void 
examGSF::slotOtherRecsClosed ()
{
  other_recs->hide ();
  display_other = NVFalse;

  bOther->setChecked (FALSE);
}



void 
examGSF::slotBeamDataClicked (int row)
{
  last_beam_rec = vhItem[row]->text ().toInt ();

  beam_recs = new beamRecs (this, gsf_handle, last_beam_rec, &options, &misc);

  beam_recs->show ();
}



void 
examGSF::slotScrollBarValueChanged (int value)
{
  statusBar ()->clearMessage ();
  record_num = value;
  readData (record_num);
}



void 
examGSF::slotScrollBarSliderMoved (int value)
{
  time_t sec;
  long nsec;
  NV_INT32 year, day, hour, minute, month, mday;
  NV_FLOAT32 second;

  gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, value, &sec, &nsec);

  cvtime (sec, nsec, &year, &day, &hour, &minute, &second);
  jday2mday (year, day, &month, &mday);
  month++;

  QString tmp;
  tmp.sprintf (tr ("Record: %d    Date/Time: %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), value, year + 1900,
               month, mday, day, hour, minute, second);

  statusBar ()->showMessage (tmp);
}



void
examGSF::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("chartsPic Preferences"));
  prefsD->setModal (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (prefsD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *fbox = new QGroupBox (tr ("Position Format"), prefsD);
  fbox->setWhatsThis (bGrpText);

  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  bGrp = new QButtonGroup (prefsD);
  bGrp->setExclusive (TRUE);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPosClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  QHBoxLayout *fboxSplit = new QHBoxLayout;
  QVBoxLayout *fboxLeft = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxLeft->addWidget (hd__);
  fboxRight->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);

  bGrp->button (options.position_form)->setChecked (TRUE);


  QGroupBox *dbox = new QGroupBox (tr ("Depth units"), prefsD);
  dbox->setWhatsThis (dGrpText);

  QRadioButton *mt = new QRadioButton (tr ("Meters"));
  QRadioButton *ft = new QRadioButton (tr ("Feet"));
  QRadioButton *fm = new QRadioButton (tr ("Fathoms"));
  QRadioButton *cu = new QRadioButton (tr ("Cubits"));
  QRadioButton *wi = new QRadioButton (tr ("Willetts"));

  dGrp = new QButtonGroup (prefsD);
  dGrp->setExclusive (TRUE);
  connect (dGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDepthClicked (int)));

  dGrp->addButton (mt, 0);
  dGrp->addButton (ft, 1);
  dGrp->addButton (fm, 2);
  dGrp->addButton (cu, 3);
  dGrp->addButton (wi, 4);

  QHBoxLayout *dboxSplit = new QHBoxLayout;
  QVBoxLayout *dboxLeft = new QVBoxLayout;
  QVBoxLayout *dboxRight = new QVBoxLayout;
  dboxSplit->addLayout (dboxLeft);
  dboxSplit->addLayout (dboxRight);
  dboxLeft->addWidget (mt);
  dboxLeft->addWidget (ft);
  dboxLeft->addWidget (fm);
  dboxRight->addWidget (cu);
  dboxRight->addWidget (wi);
  dbox->setLayout (dboxSplit);

  vbox->addWidget (dbox, 1);

  dGrp->button (options.depth_units)->setChecked (TRUE);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (prefsD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), prefsD);
  closeButton->setToolTip (tr ("Close the preferences dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);


  prefsD->show ();
}



void
examGSF::slotPosClicked (int id)
{
  options.position_form = id;


  readData (record_num);
}



void
examGSF::slotDepthClicked (int id)
{
  options.depth_units = id;

  setUnits ();

  setHeaders ();
  readData (record_num);
}



void 
examGSF::setUnits ()
{
  if (beam_recs != NULL && beam_recs->isVisible ()) 
    {
      beam_recs->close ();

      beam_recs = new beamRecs (this, gsf_handle, last_beam_rec, &options, &misc);

      beam_recs->show ();
    }
  if (other_recs != NULL && other_recs->isVisible ())
    {
      other_recs->close ();

      other_recs = new otherRecs (this, gsf_handle, &options, &misc);
      connect (other_recs, SIGNAL (findPingSignal (NV_INT32)), this, SLOT (slotFindPing (NV_INT32)));
      connect (other_recs, SIGNAL (closedSignal ()), this, SLOT (slotOtherRecsClosed ()));

      other_recs->show ();
    }
}



void 
examGSF::setHeaders ()
{
  QString tmp;

  gsfTable->setHorizontalHeaderItem (0, new QTableWidgetItem (tr ("Date (Julian Day) Time")));
  gsfTable->setHorizontalHeaderItem (1, new QTableWidgetItem (tr ("Position")));
  gsfTable->setHorizontalHeaderItem (2, new QTableWidgetItem (tr ("Valid\nBeams")));
  gsfTable->setHorizontalHeaderItem (3, new QTableWidgetItem (tr ("Total\nBeams")));
  gsfTable->setHorizontalHeaderItem (4, new QTableWidgetItem (tr ("Center\nBeam")));
  gsfTable->setHorizontalHeaderItem (5, new QTableWidgetItem (tr ("Heading")));
  gsfTable->setHorizontalHeaderItem (6, new QTableWidgetItem (tr ("Pitch")));
  gsfTable->setHorizontalHeaderItem (7, new QTableWidgetItem (tr ("Roll")));
  gsfTable->setHorizontalHeaderItem (8, new QTableWidgetItem (tr ("Heave")));
  gsfTable->setHorizontalHeaderItem (9, new QTableWidgetItem (tr ("Course")));
  gsfTable->setHorizontalHeaderItem (10, new QTableWidgetItem (tr ("Speed")));
  tmp = tr ("Tide\nCorrection\n") + misc.depth_string[options.depth_units];
  gsfTable->setHorizontalHeaderItem (11, new QTableWidgetItem (tmp));
  gsfTable->setHorizontalHeaderItem (12, new QTableWidgetItem (tr ("Tide\nType")));
  tmp = tr ("Depth\nCorrection\n") + misc.depth_string[options.depth_units];
  gsfTable->setHorizontalHeaderItem (13, new QTableWidgetItem (tmp));
  gsfTable->setHorizontalHeaderItem (14, new QTableWidgetItem (tr ("Validity\nFlags")));
}



void 
examGSF::slotDataProfileMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 x, NV_FLOAT64 y)
{
  QString tmp;

  tmp.sprintf ("%d", NINT (y) + start_rec);
  recordLabel->setText (tmp);

  tmp.sprintf ("%f", x);
  dataLabel->setText (tmp);
}



void
examGSF::slotClosePrefs ()
{
  prefsD->close ();
}



void
examGSF::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
examGSF::about ()
{
  QMessageBox::about (this, VERSION,
                      tr ("examGSF - GSF data file viewer.") + 
                      tr ("\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)"));
}


void
examGSF::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}


void
examGSF::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}


//  Get the users defaults.

void
examGSF::envin ()
{
  NV_FLOAT64 saved_version = 0.0;


  // Set Defaults so the if keys don't exist the parms are defined

  misc.depth_string[0] = tr ("(meters)");
  misc.depth_factor[0] = 1.0;
  misc.depth_string[1] = tr ("(feet)");
  misc.depth_factor[1] = 3.28084;
  misc.depth_string[2] = tr ("(fathoms)");
  misc.depth_factor[2] = 0.54681;
  misc.depth_string[3] = tr ("(cubits)");
  misc.depth_factor[3] = 0.44196;
  misc.depth_string[4] = tr ("(willetts)");
  misc.depth_factor[4] = 16.4042;

  options.position_form = 0;
  options.window_x = 0;
  options.window_y = 0;
  options.width = EXAM_X_SIZE;
  options.height = EXAM_Y_SIZE;
  options.beam_window_x = 0;
  options.beam_window_y = 0;
  options.beam_width = BEAM_X_SIZE;
  options.beam_height = BEAM_Y_SIZE;
  options.other_window_x = 0;
  options.other_window_y = 0;
  options.other_width = OTHER_X_SIZE;
  options.other_height = OTHER_Y_SIZE;
  options.extended_window_x = 0;
  options.extended_window_y = 0;
  options.extended_width = EXTENDED_X_SIZE;
  options.extended_height = EXTENDED_Y_SIZE;
  options.profile_window_x = 0;
  options.profile_window_y = 0;
  options.profile_width = PROFILE_X_SIZE;
  options.profile_height = PROFILE_Y_SIZE;
  options.svp_window_x = 0;
  options.svp_window_y = 0;
  options.svp_width = SVP_X_SIZE;
  options.svp_height = SVP_Y_SIZE;
  options.depth_units = 0;
  options.input_dir = ".";
  setUnits ();


  QSettings settings (tr ("navo.navy.mil"), tr ("examGSF"));
  settings.beginGroup (tr ("examGSF"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  options.position_form = settings.value (tr ("position form"), options.position_form).toInt ();

  options.width = settings.value (tr ("width"), options.width).toInt ();

  options.height = settings.value (tr ("height"), options.height).toInt ();

  options.window_x = settings.value (tr ("window x"), options.window_x).toInt ();

  options.window_y = settings.value (tr ("window y"), options.window_y).toInt ();


  options.beam_width = settings.value (tr ("beam width"), options.beam_width).toInt ();

  options.beam_height = settings.value (tr ("beam height"), options.beam_height).toInt ();

  options.beam_window_x = settings.value (tr ("beam window x"), options.beam_window_x).toInt ();

  options.beam_window_y = settings.value (tr ("beam window y"), options.beam_window_y).toInt ();


  options.other_width = settings.value (tr ("other width"), options.other_width).toInt ();

  options.other_height = settings.value (tr ("other height"), options.other_height).toInt ();

  options.other_window_x = settings.value (tr ("other window x"), options.other_window_x).toInt ();

  options.other_window_y = settings.value (tr ("other window y"), options.other_window_y).toInt ();


  options.extended_width = settings.value (tr ("extended width"), options.extended_width).toInt ();

  options.extended_height = settings.value (tr ("extended height"), options.extended_height).toInt ();

  options.extended_window_x = settings.value (tr ("extended window x"), options.extended_window_x).toInt ();

  options.extended_window_y = settings.value (tr ("extended window y"), options.extended_window_y).toInt ();


  options.profile_width = settings.value (tr ("profile width"), options.profile_width).toInt ();

  options.profile_height = settings.value (tr ("profile height"), options.profile_height).toInt ();

  options.profile_window_x = settings.value (tr ("profile window x"), options.profile_window_x).toInt ();

  options.profile_window_y = settings.value (tr ("profile window y"), options.profile_window_y).toInt ();


  options.svp_width = settings.value (tr ("svp width"), options.svp_width).toInt ();

  options.svp_height = settings.value (tr ("svp height"), options.svp_height).toInt ();

  options.svp_window_x = settings.value (tr ("svp window x"), options.svp_window_x).toInt ();

  options.svp_window_y = settings.value (tr ("svp window y"), options.svp_window_y).toInt ();


  options.depth_units = settings.value (tr ("depth units"), options.depth_units).toInt ();

  options.input_dir = settings.value (tr ("input directory"), options.input_dir).toString ();

  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();


  switch (options.depth_units)
    {
    case 1:
      QMessageBox::warning (this, tr ("Units of Measure"), feetText);
      break;

    case 2:
      QMessageBox::warning (this, tr ("Units of Measure"), fathomText);
      break;

    case 3:
      QMessageBox::warning (this, tr ("Units of Measure"), cubitText);
      break;

    case 4:
      QMessageBox::warning (this, tr ("Units of Measure"), willettText);
      break;
    }


  setUnits ();
}




//  Save the users defaults.

void
examGSF::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  options.window_x = tmp.x ();
  options.window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  options.width = tmp.width ();
  options.height = tmp.height ();


  //  For the beam information window

  if (beam_recs != NULL && beam_recs->isVisible ()) 
    {
      //  Use frame geometry to get the absolute x and y.

      tmp = beam_recs->frameGeometry ();

      options.beam_window_x = tmp.x ();
      options.beam_window_y = tmp.y ();


      options.beam_width = tmp.width ();
      options.beam_height = tmp.height ();
    }


  //  For the other information window

  if (other_recs != NULL && other_recs->isVisible ()) 
    {
      //  Use frame geometry to get the absolute x and y.

      tmp = other_recs->frameGeometry ();

      options.other_window_x = tmp.x ();
      options.other_window_y = tmp.y ();


      options.other_width = tmp.width ();
      options.other_height = tmp.height ();
    }


  QSettings settings (tr ("navo.navy.mil"), tr ("examGSF"));
  settings.beginGroup (tr ("examGSF"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), options.position_form);


  settings.setValue (tr ("width"), options.width);

  settings.setValue (tr ("height"), options.height);

  settings.setValue (tr ("window x"), options.window_x);

  settings.setValue (tr ("window y"), options.window_y);


  settings.setValue (tr ("beam width"), options.beam_width);

  settings.setValue (tr ("beam height"), options.beam_height);

  settings.setValue (tr ("beam window x"), options.beam_window_x);

  settings.setValue (tr ("beam window y"), options.beam_window_y);


  settings.setValue (tr ("other width"), options.other_width);

  settings.setValue (tr ("other height"), options.other_height);

  settings.setValue (tr ("other window x"), options.other_window_x);

  settings.setValue (tr ("other window y"), options.other_window_y);


  settings.setValue (tr ("extended width"), options.extended_width);

  settings.setValue (tr ("extended height"), options.extended_height);

  settings.setValue (tr ("extended window x"), options.extended_window_x);

  settings.setValue (tr ("extended window y"), options.extended_window_y);


  settings.setValue (tr ("profile width"), options.profile_width);

  settings.setValue (tr ("profile height"), options.profile_height);

  settings.setValue (tr ("profile window x"), options.profile_window_x);

  settings.setValue (tr ("profile window y"), options.profile_window_y);


  settings.setValue (tr ("svp width"), options.svp_width);

  settings.setValue (tr ("svp height"), options.svp_height);

  settings.setValue (tr ("svp window x"), options.svp_window_x);

  settings.setValue (tr ("svp window y"), options.svp_window_y);


  settings.setValue (tr ("depth units"), options.depth_units);

  settings.setValue (tr ("input directory"), options.input_dir);

  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}

