#include "ancillaryProg.hpp"

ancillaryProg::ancillaryProg (QWidget *parent, nvMapGL *ma, POINT_DATA *da, OPTIONS *op, MISC *mi, NV_INT32 prog, 
                              NV_INT32 lock_point):
  QProcess (parent)
{
  NV_INT32 buildCommand (QString progString, QString actionString, POINT_DATA *data, MISC *misc, 
                         NV_INT32 nearest_point, QString *cmd, QStringList *args, NV_INT32 kill_switch);


  map = ma;
  options = op;
  misc = mi;
  data = da;
  pa = parent;
  appTimerCount = 0;


  command_line = NVFalse;
  command_line_up = NVFalse;


  //  Prepare to get information back from the external program.

  misc->abeShare->lock ();


  misc->abe_share->modcode = 0;
  misc->abe_share->modified_point = -1;


  //  Build the command line.

  QString progString = options->prog[prog], actionString = options->action[prog];


  //  Add [KILL_SWITCH] options for "kill and respawn" programs.

  if (options->state[prog]) progString += " [KILL_SWITCH]";


  NV_INT32 ret = buildCommand (progString, actionString, data, misc, lock_point, &cmd, &args, options->kill_switch[prog]);


  if (ret == 2)
    {
      command_line = NVTrue;

      commandLineD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
      commandLineD->setWindowTitle (cmd);
      commandLineD->resize (600, 600);

      commandLineD->setSizeGripEnabled (TRUE);

      QVBoxLayout *vbox = new QVBoxLayout (commandLineD);
      vbox->setMargin (5);
      vbox->setSpacing (5);


      commandText = new QTextEdit (commandLineD);
      commandText->setReadOnly (TRUE);

      vbox->addWidget (commandText);


      QHBoxLayout *actions = new QHBoxLayout (0);
      vbox->addLayout (actions);

      QPushButton *bHelp = new QPushButton (commandLineD);
      bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
      bHelp->setToolTip (tr ("Enter What's This mode for help"));
      connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
      actions->addWidget (bHelp);

      actions->addStretch (10);

      QPushButton *closeButton = new QPushButton (tr ("Close"), commandLineD);
      closeButton->setToolTip (tr ("Close the command line program dialog"));
      connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseCommand ()));
      actions->addWidget (closeButton);
    }


  connect (this, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotDone (int, QProcess::ExitStatus)));
  connect (this, SIGNAL (readyReadStandardError ()), this, SLOT (slotReadyReadStandardError ()));
  connect (this, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotReadyReadStandardOutput ()));
  connect (this, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotError (QProcess::ProcessError)));


  misc->abeShare->unlock ();


  if (options->prog[prog].contains ("mosaicView") || options->prog[prog].contains ("chartsPic"))
    {
      qApp->setOverrideCursor (Qt::WaitCursor);
      qApp->processEvents ();
      misc->abe_share->key = WAIT_FOR_START;
    }


  this->start (cmd, args);


  //  Starting a half second timer so that we can set the wait cursor.  If we use the started signal from QProcess
  //  the return is instant even though the application isn't visible.  This is just to give the user a warm and
  //  fuzzy.  If the WAIT_FOR_START flag isn't reset in 5 seconds we release anyway.
  //  NOTE:  I'm only doing this for mosaicView and chartsPic at this time.  Maybe more later.

  if (options->prog[prog].contains ("mosaicView") || options->prog[prog].contains ("chartsPic"))
    {
      appTimerCount = 0;
      appTimer = new QTimer (this);
      connect (appTimer, SIGNAL (timeout()), this, SLOT (slotAppTimer ()));
      appTimer->start (500);
    }
}



ancillaryProg::~ancillaryProg ()
{
}



void 
ancillaryProg::slotAppTimer ()
{
  if (appTimerCount > 10 || misc->abe_share->key != WAIT_FOR_START)
    {
      appTimer->stop ();

      if (appTimerCount > 10)
        {
          QString program;
          program = cmd.section (' ', 0, 0);
          QMessageBox::information (pa, tr ("geoSwath3D ancillary program"), tr ("The program ") + 
                                    program + tr (" has been started but may take some time to appear.\n") +
                                    tr ("Please be patient, it will appear eventually."));
          qApp->processEvents ();
        }

      qApp->restoreOverrideCursor ();

      appTimerCount = 0;
    }
  else
    {
      appTimerCount++;
    }
}



void 
ancillaryProg::slotError (QProcess::ProcessError error)
{
  QString commandLine = cmd;
  for (NV_INT32 i = 0 ; i < args.size () ; i++) commandLine += (" " + args.at (i));

  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"), tr ("Unable to start the ancillary program!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Crashed:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"), tr ("The ancillary program crashed!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Timedout:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"), tr ("The ancillary program timed out!") + 
                             "\n" + commandLine);
      break;

    case QProcess::WriteError:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"), 
                             tr ("There was a write error to the ancillary program!") + "\n" + commandLine);
      break;

    case QProcess::ReadError:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"),
                             tr ("There was a read error from the ancillary program!") + "\n" + commandLine);
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (pa, tr ("geoSwath3D ancillary program"), 
                             tr ("The ancillary program died with an unknown error!") + "\n" + commandLine);
      break;

      if (appTimerCount)
        {
          appTimer->stop ();
          appTimerCount = 0;
        }
    }
}



void 
ancillaryProg::slotDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  if (command_line && command_line_up)
    {
      QScrollBar *vbar = commandText->verticalScrollBar ();

      vbar->setSliderPosition (vbar->minimum ());
    }


  //  The external program should set modcode to it's data type to signify that it has 
  //  done something

  if (misc->abe_share->modcode)
    {
      misc->abeShare->lock ();


      //  Unfortunately this is data type specific, which I really hate to put in here,
      //  but we have no other choice that I can see.

      switch (misc->abe_share->modcode)
        {
        default:
          break;
        }
      misc->abeShare->unlock ();
    }
}



void 
ancillaryProg::slotReadyReadStandardOutput ()
{
  if (command_line && !command_line_up)
    {
      commandLineD->show ();
      command_line_up = NVTrue;
    }

  QByteArray response = this->readAllStandardOutput ();
  NV_CHAR *res = response.data ();

  if (command_line)
    {
      commandText->append (QString (res) + "\n");
    }
  else
    {
      fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
      fflush (stderr);
    }
}



void 
ancillaryProg::slotReadyReadStandardError ()
{
  if (command_line && !command_line_up)
    {
      commandLineD->show ();
      command_line_up = NVTrue;
    }

  QByteArray response = this->readAllStandardError ();
  NV_CHAR *res = response.data ();

  if (command_line)
    {
      commandText->append (QString (res) + "\n");
    }
  else
    {
      //  Let's not print out the "QProcess: Destroyed while process is still running" messages that come from
      //  killing ancillary programs in the editor.

      if (!strstr (res, "Destroyed while"))
        {
          fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
          fflush (stderr);
        }
    }
}



void 
ancillaryProg::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
ancillaryProg::slotCloseCommand ()
{
  commandLineD->close ();
}
