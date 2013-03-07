
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



#include "screenshot.hpp"
#include "screenshotHelp.hpp"


//!  Dialog to capture, display, and optionally save a screenshot.

screenshot::screenshot (QWidget *parent, OPTIONS *op, MISC *mi, NV_INT32 ct):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  current_feature = ct;


  QString feature_id;
  feature_id.sprintf ("%05d", misc->feature[current_feature].record_number);
  setWindowTitle (tr ("pfmEdit3D Screenshot ") + feature_id);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  screenshotLabel = new QLabel;
  screenshotLabel->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
  screenshotLabel->setAlignment (Qt::AlignCenter);
  screenshotLabel->setMinimumSize (480, 240);


  vbox->addWidget (screenshotLabel, 1);


  QGroupBox *tbox = new QGroupBox (tr ("Time delay"), this);
  QHBoxLayout *tboxLayout = new QHBoxLayout;
  tbox->setLayout (tboxLayout);

  timeDelay = new QSpinBox (this);
  timeDelay->setRange (0, 60);
  timeDelay->setSingleStep (5);
  timeDelay->setToolTip (tr ("Screenshot time delay in seconds"));
  timeDelay->setWhatsThis (timeDelayText);
  timeDelay->setValue (options->screenshot_delay);
  tboxLayout->addWidget (timeDelay);


  vbox->addWidget (tbox);


  QHBoxLayout *actions = new QHBoxLayout ();
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *bNewScreenshot = new QPushButton (tr ("New Screenshot"), this);
  bNewScreenshot->setToolTip (tr ("Grab a new copy of the screen"));
  bNewScreenshot->setWhatsThis (newScreenshotText);
  connect (bNewScreenshot, SIGNAL (clicked ()), this, SLOT (slotNewScreenshot ()));
  actions->addWidget (bNewScreenshot);

  QPushButton *bSaveScreenshot = new QPushButton (tr ("Save Screenshot"), this);
  bSaveScreenshot->setToolTip (tr ("Save the screenshot to the BFD file"));
  bSaveScreenshot->setWhatsThis (saveScreenshotText);
  connect (bSaveScreenshot, SIGNAL (clicked ()), this, SLOT (slotSaveScreenshot ()));
  actions->addWidget (bSaveScreenshot);

  QPushButton *bQuit = new QPushButton (tr ("Quit"), this);
  bQuit->setToolTip (tr ("Discard screenshot and close this dialog"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  actions->addWidget (bQuit);


  //  Wait the requested time and then grab the screen

  QTimer::singleShot (options->screenshot_delay * 1000, this, SLOT (grab ()));
}



screenshot::~screenshot ()
{
}



void 
screenshot::resizeEvent (QResizeEvent *e __attribute__ ((unused)))
{
  QSize scaledSize = screenPixmap.size ();
  scaledSize.scale (screenshotLabel->size (), Qt::KeepAspectRatio);
  if (!screenshotLabel->pixmap () || scaledSize != screenshotLabel->pixmap ()->size ())
    screenshotLabel->setPixmap (screenPixmap.scaled (screenshotLabel->size (), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


void 
screenshot::slotNewScreenshot()
{
  hide ();

  QTimer::singleShot (options->screenshot_delay * 1000, this, SLOT (grab ()));
}



void 
screenshot::slotSaveScreenshot()
{
  QString screenshot_file_name;


  screenshot_file_name.sprintf ("%s_%05d.jpg", misc->abe_share->open_args[0].list_path, 
    misc->feature[current_feature].record_number);


  screenPixmap.save (screenshot_file_name, "jpg");


  emit screenshotSavedSignal (screenshot_file_name);


  close ();
}



void 
screenshot::grab ()
{
  screenPixmap = QPixmap ();

  screenPixmap = QPixmap::grabWindow (QApplication::desktop ()->winId());

  screenshotLabel->setPixmap (screenPixmap.scaled (screenshotLabel->size (), Qt::KeepAspectRatio, Qt::SmoothTransformation));

  show();
}



void
screenshot::slotQuit ()
{
  emit screenshotQuit ();

  close ();
}



void
screenshot::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}
