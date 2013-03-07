#include "featurePic.hpp"
#include "featurePicHelp.hpp"


NV_FLOAT64 settings_version = 2.0;


featurePic::featurePic (NV_INT32 *argc __attribute__ ((unused)), NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;
  BFDATA_HEADER   bfd_header;
  BFDATA_RECORD   bfd_record;
  NV_U_BYTE       *image = NULL;
  NV_INT32        bfd_handle, record;
  NV_FLOAT64      heading;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/feature_pic.xpm"));


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("chartsPic main toolbar"));


  envin ();


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  statusBar ()->setSizeGripEnabled (FALSE);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the pic.

  picdef.transMode = Qt::SmoothTransformation;
  pic = new nvPic (this, &picdef);


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  vBox->addWidget (pic);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.xpm"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  tools->addSeparator ();
  tools->addSeparator ();


  QToolButton *bBrighten = new QToolButton (this);
  bBrighten->setIcon (QIcon (":/icons/brighten.xpm"));
  bBrighten->setToolTip (tr ("Brighten the picture"));
  connect (bBrighten, SIGNAL (clicked ()), this, SLOT (slotBrighten ()));
  tools->addWidget (bBrighten);


  QToolButton *bDarken = new QToolButton (this);
  bDarken->setIcon (QIcon (":/icons/darken.xpm"));
  bDarken->setToolTip (tr ("Darken the picture"));
  connect (bDarken, SIGNAL (clicked ()), this, SLOT (slotDarken ()));
  tools->addWidget (bDarken);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), qApp, SLOT (closeAllWindows ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about chartsPic"));
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


  NV_INT32 option_index = 0;
  heading = 0.0;
  record = -1;
  NV_CHAR filename[512];


  while (NVTrue) 
    {
      static struct option long_options[] = {{"file", required_argument, 0, 0},
                                             {"record", required_argument, 0, 0},
                                             {"heading", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;


      QString skey;


      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              strcpy (filename, optarg);
              break;

            case 1:
              sscanf (optarg, "%d", &record);
              break;

            case 2:
              sscanf (optarg, "%lf", &heading);
              break;

            default:
              break;
            }
          break;
        }
    }


  show ();


  //  If we specified a record on the command line, this must be a BFD file so we'll read the image from the file.

  if (record >= 0)
    {
      if ((bfd_handle = binaryFeatureData_open_file (filename, &bfd_header, BFDATA_READONLY)) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, tr ("featurePic"), tr ("Unable to open feature file.\nReason: ") + msg);
          exit (-1);
        }

      if (binaryFeatureData_read_record (bfd_handle, record, &bfd_record) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, tr ("featurePic"), tr ("Unable to read the feature record.\nReason: ") + msg);
          exit (-1);
        }

      image = (NV_U_BYTE *) malloc (bfd_record.image_size);

      if (image == NULL)
        {
          QString msg = QString (strerror (errno));
          QMessageBox::warning (this, tr ("featurePic"), tr ("Error allocating image memory.\nReason: ") + msg);
          exit (-1);
        }

      if (binaryFeatureData_read_image (bfd_handle, record, image) < 0)
        {
          QString msg = QString (binaryFeatureData_strerror ());
          QMessageBox::warning (this, tr ("featurePic"), tr ("Unable to read the image data.\nReason: ") + msg);
          exit (-1);
        }

      //  Load the image

      if (!pic->OpenData (image, bfd_record.image_size))
        {
          QMessageBox::warning (this, tr ("featurePic"),
                                tr ("There was an error opening the image from file ") + 
                                QDir::toNativeSeparators (QString (filename)));
          exit (-1);
        }

      free (image);
    }
  else
    {
      //  Load the picture.

      if (!pic->OpenFile (filename))
        {
          QMessageBox::warning (this, tr ("featurePic"),
                                tr ("There was an error opening or reading the file ") + 
                                QDir::toNativeSeparators (QString (filename)));
          exit (-1);
        }
    }


  //  Rotate to "north up" if available

  pic->RotatePixmap (heading);

  pic->setCursor (Qt::ArrowCursor);

  pic->enableSignals ();
}



featurePic::~featurePic ()
{
}


void
featurePic::redrawPic ()
{
  pic->redrawPic ();
}



//  A bunch of slots.

void 
featurePic::slotQuit ()
{
  envout ();

  exit (0);
}



void 
featurePic::slotBrighten ()
{
  pic->brighten (10);
}



void 
featurePic::slotDarken ()
{
  pic->brighten (-10);
}



void
featurePic::about ()
{
  QMessageBox::about (this, VERSION,
                      tr ("featurePic - Feature snippet file viewer.") + 
                      tr ("\n\nAuthor : Jan C. Depner (jan.depner@navy.mil)"));
}


void
featurePic::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}


void
featurePic::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}


//  Get the users defaults.

void
featurePic::envin ()
{
  NV_FLOAT64 saved_version = 0.0;


  // Set defaults so the if keys don't exist the parameters are defined

  window_x = 0;
  window_y = 0;
  width = PIC_X_SIZE;
  height = PIC_Y_SIZE;


  QSettings settings (tr ("navo.navy.mil"), tr ("featurePic"));
  settings.beginGroup (tr ("featurePic"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();


  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
featurePic::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("featurePic"));
  settings.beginGroup (tr ("featurePic"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
