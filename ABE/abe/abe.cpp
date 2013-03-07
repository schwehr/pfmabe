
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



//  abe class.

/***************************************************************************\

    Module :        abe

    Prorammer :     Jan C. Depner

    Date :          01/15/10

    Purpose :       Silly little toolbar with buttons for all of the ABE
                    graphical programs.

\***************************************************************************/

#include "abe.hpp"
#include "abeHelp.hpp"


abe::abe (int *argc __attribute__ ((unused)), char **argv __attribute__ ((unused)), QWidget *parent):
  QDialog (parent, 0)
{
  void envin (OPTIONS *options, MISC *misc);
  void set_defaults (MISC *misc, OPTIONS *options);


  half_progs = NUM_PROGS / 2;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  setSizePolicy (QSizePolicy::Maximum, QSizePolicy::Maximum);


  //  Set the main title and icon

  setWindowIcon (QIcon (":/icons/abe.xpm"));
  setWindowTitle (VERSION);


  //  Set all of the defaults

  set_defaults (&misc, &options);


  //  Get the user's defaults if available

  envin (&options, &misc);


  QSize mainButtonIconSize (options.main_button_icon_size, options.main_button_icon_size);


  vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);
  vbox->setSizeConstraint (QLayout::SetNoConstraint);


  QMenuBar *menuBar = new QMenuBar (this);


  QAction *verticalAction = new QAction (tr ("Vertical"), this);
  verticalAction->setShortcut (tr ("Ctrl+V"));
  connect (verticalAction, SIGNAL (triggered ()), this, SLOT (slotVertical ()));

  QAction *horizontalAction = new QAction (tr ("Horizontal"), this);
  horizontalAction->setShortcut (tr ("Ctrl+H"));
  connect (horizontalAction, SIGNAL (triggered ()), this, SLOT (slotHorizontal ()));

  QMenu *orientationMenu = menuBar->addMenu (tr ("&View"));
  orientationMenu->addAction (verticalAction);
  orientationMenu->addAction (horizontalAction);


  //  Setup the help menu.

  QAction *whatsThisAct = QWhatsThis::createAction (this);

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about abe"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgements = new QAction (tr ("A&cknowledgements"), this);
  acknowledgements->setShortcut (tr ("Ctrl+c"));
  acknowledgements->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgements, SIGNAL (triggered ()), this, SLOT (slotAcknowledgements ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar->addMenu (tr ("&Help"));
  helpMenu->addAction (whatsThisAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgements);
  helpMenu->addAction (aboutQtAct);


  vbox->addWidget (menuBar);


  grid = new QGridLayout ();
  grid->setSizeConstraint (QLayout::SetNoConstraint);


  QButtonGroup *toolGrp = new QButtonGroup (this);
  connect (toolGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotToolGrp (int)));
  toolGrp->setExclusive (FALSE);

  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      toolButton[i] = new dndButton (this, i);
      toolButton[i]->setIcon (misc.buttonIcon[i]);
      toolButton[i]->setIconSize (mainButtonIconSize);
#ifdef NVWIN3X
      QString prog = misc.progName[i];
      QString tip = prog.remove (".exe") + " - (" + options.buttonAccel[i] + ")";
#else
      QString tip = misc.progName[i] + " - (" + options.buttonAccel[i] + ")";
#endif
      toolButton[i]->setToolTip (tip);
      toolButton[i]->setWhatsThis (misc.buttonWhat[i]);
      connect (toolButton[i], SIGNAL (dropSignal (NV_CHAR *, NV_INT32)), this, SLOT (slotDrop (NV_CHAR *, NV_INT32)));
      toolGrp->addButton (toolButton[i], i);

      NV_INT32 row;
      NV_INT32 col;

      if (options.orientation == Qt::Vertical)
        {
          row = i % half_progs;
          col = i / half_progs;
        }
      else
        {
          row = i / half_progs;
          col = i % half_progs;
        }
      grid->setColumnStretch (col, 0);
      grid->setRowStretch (row, 0);

      grid->addWidget (toolButton[i], row, col);


      NV_CHAR pn[128];
      strcpy (pn, misc.progName[i].toAscii ());

      if (find_startup_name (pn) == NULL) toolButton[i]->setEnabled (FALSE);
    }


  vbox->addLayout (grid);


  //  Set the location from defaults.

  this->move (options.window_x, options.window_y);


  show ();
}



abe::~abe ()
{
}



//  Somebody pressed the little X in the window decoration.

void 
abe::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  void envout (OPTIONS *options, MISC *misc);

  envout (&options, &misc);
}



void
abe::keyPressEvent (QKeyEvent *event)
{
  QString qkey = event->text ();


  //  Check for <Ctrl>Q to quit.

  if (event->key () == Qt::Key_Q && event->modifiers () == Qt::ControlModifier) exit (0);


  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      if (qkey == options.buttonAccel[i])
        {
          QProcess *prog = new QProcess (this);

          prog->start (misc.progName[i]);

          break;
        }
    }
}



void
abe::slotToolGrp (int id)
{
  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      if (i == id)
        {
          QProcess *prog = new QProcess (this);

          prog->start (misc.progName[i]);

          break;
        }
    }
}



void
abe::slotDrop (NV_CHAR *text, NV_INT32 id)
{
  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      if (i == id)
        {
          if (misc.buttonFile[i])
            {
              QProcess *prog = new QProcess (this);

              QStringList arguments;
              QString arg;

              arg = QString (text);
              arguments += arg;

              prog->start (misc.progName[i], arguments);
            }
          else
            {
              QProcess *prog = new QProcess (this);

              prog->start (misc.progName[i]);
            }

          break;
        }
    }
}



void 
abe::slotVertical ()
{
  options.orientation = Qt::Vertical;


  //  We have to delete and recreate the grid layout so it will recalculate the number of rows and columns.

  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++) grid->removeWidget (toolButton[i]);


  vbox->removeItem (grid);
  delete (grid);

  grid = new QGridLayout ();
  grid->setSizeConstraint (QLayout::SetNoConstraint);

  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      NV_INT32 row = i % half_progs;
      NV_INT32 col = i / half_progs;

      grid->setColumnStretch (col, 0);
      grid->setRowStretch (row, 0);

      grid->addWidget (toolButton[i], row, col);


      NV_CHAR pn[128];
      strcpy (pn, misc.progName[i].toAscii ());

      if (find_startup_name (pn) == NULL) toolButton[i]->setEnabled (FALSE);
    }

  vbox->addLayout (grid);


  resize (1, 1);
  adjustSize ();
}



void 
abe::slotHorizontal ()
{
  options.orientation = Qt::Horizontal;


  //  We have to delete and recreate the grid layout so it will recalculate the number of rows and columns.

  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++) grid->removeWidget (toolButton[i]);


  vbox->removeItem (grid);
  delete (grid);

  grid = new QGridLayout ();
  grid->setSizeConstraint (QLayout::SetNoConstraint);

  for (NV_INT32 i = 0 ; i < NUM_PROGS ; i++)
    {
      NV_INT32 row = i / half_progs;
      NV_INT32 col = i % half_progs;

      grid->setColumnStretch (col, 0);
      grid->setRowStretch (row, 0);

      grid->addWidget (toolButton[i], row, col);


      NV_CHAR pn[128];
      strcpy (pn, misc.progName[i].toAscii ());

      if (find_startup_name (pn) == NULL) toolButton[i]->setEnabled (FALSE);
    }

  vbox->addLayout (grid);

  resize (1, 1);
  adjustSize ();
}



void
abe::about ()
{
  QMessageBox::about (this, VERSION, abeAboutText);
}



void
abe::slotAcknowledgements ()
{
  QMessageBox::about (this, VERSION, acknowledgementsText);
}



void
abe::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}
