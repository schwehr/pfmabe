
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



#include "ancillaryProg.hpp"


//!  This widget runs and external ancillary program in response to a button or hot key press.

ancillaryProg::ancillaryProg (QWidget *parent, nvMap *ma, OPTIONS *op, MISC *mi, NV_INT32 prog, NV_INT32 lock_point):
  QProcess (parent)
{
  NV_INT32 buildCommand (QString progString, QString actionString, MISC *misc, NV_INT32 nearest_point, QString *cmd, QStringList *args, NV_INT32 kill_switch);


  map = ma;
  options = op;
  misc = mi;
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


  NV_INT32 ret = buildCommand (progString, actionString, misc, lock_point, &cmd, &args, options->kill_switch[prog]);


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
          QMessageBox::information (pa, tr ("pfmEdit ancillary program"), tr ("The program ") + 
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
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"), tr ("Unable to start the ancillary program!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Crashed:
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"), tr ("The ancillary program crashed!") + 
                             "\n" + commandLine);
      break;

    case QProcess::Timedout:
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"), tr ("The ancillary program timed out!") + 
                             "\n" + commandLine);
      break;

    case QProcess::WriteError:
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"), 
                             tr ("There was a write error to the ancillary program!") + "\n" + commandLine);
      break;

    case QProcess::ReadError:
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"),
                             tr ("There was a read error from the ancillary program!") + "\n" + commandLine);
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (pa, tr ("pfmEdit ancillary program"), 
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


      NV_INT32 x, y, z;


      //  Unfortunately this is data type specific, which I really hate to put in here,
      //  but we have no other choice that I can see.  At the moment we're only doing
      //  this with CHARTS LIDAR data.

      switch (misc->abe_share->modcode)
        {
        case PFM_SHOALS_1K_DATA:
        case PFM_CHARTS_HOF_DATA:
          if (misc->abe_share->modified_point >= 0)
            {
              NV_INT32 set_point = misc->abe_share->modified_point;
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
                  perror ("Allocating misc->highlight in ancillaryProg.cpp");
                  exit (-1);
                }

              misc->highlight[misc->highlight_count] = set_point;
              misc->highlight_count++;
            }


          //  This is a bit obscure - the waveform monitor program(s) will look for this modcode to force a redraw.

          misc->abe_share->modcode = WAVEMONITOR_FORCE_REDRAW;


          emit redrawSignal (NVTrue);

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
      fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
      fflush (stderr);
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
