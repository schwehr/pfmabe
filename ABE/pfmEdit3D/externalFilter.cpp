
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



#include "externalFilter.hpp"


//!  This is used to run an external filter program.  At present we only use it for the hofWaveFilter.

externalFilter::externalFilter (QWidget *parent, nvMapGL *ma, OPTIONS *op, MISC *mi, NV_INT32 prog, NV_BOOL *failed):
  QProcess (parent)
{
  NV_INT32 buildCommand (QString progString, QString actionString, MISC *misc, NV_INT32 nearest_point, QString *cmd, QStringList *args, NV_INT32 kill_switch);

  map = ma;
  options = op;
  misc = mi;
  pa = parent;
  *failed = NVFalse;



  misc->statusProgLabel->setText (tr ("Running ") + options->name[prog]);
  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, 0);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  QString progString = options->prog[prog];

  QString actionString = options->action[prog];
  buildCommand (progString, actionString, misc, -999, &cmd, &args, -1);


  connect (this, SIGNAL (finished (int, QProcess::ExitStatus)), this, SLOT (slotDone (int, QProcess::ExitStatus)));
  connect (this, SIGNAL (readyReadStandardError ()), this, SLOT (slotReadyReadStandardError ()));
  connect (this, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotReadyReadStandardOutput ()));
  connect (this, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotError (QProcess::ProcessError)));


  misc->dataShare->unlock ();


  this->start (cmd, args);
}



externalFilter::~externalFilter ()
{
}



void 
externalFilter::slotError (QProcess::ProcessError error)
{
  QString commandLine = cmd;
  for (NV_INT32 i = 0 ; i < args.size () ; i++) commandLine += (" " + args.at (i));

  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("Unable to start the area program!") + "\n" + commandLine);
      break;

    case QProcess::Crashed:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("The area program crashed!") + "\n" + commandLine);
      break;

    case QProcess::Timedout:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("The area program timed out!") + "\n" + commandLine);
      break;

    case QProcess::WriteError:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("There was a write error to the area program!") + "\n" + commandLine);
      break;

    case QProcess::ReadError:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("There was a read error from the area program!") + "\n" + commandLine);
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (pa, tr ("pfmEdit3D area program"), tr ("The area program died with an unknown error!") + "\n" + commandLine);
      break;
    }


  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  misc->dataShare->lock ();
  emit externalFilterDone ();
}



void 
externalFilter::slotDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  misc->dataShare->lock ();


  switch (misc->abe_share->modcode)
    {
    default:
    case NO_ACTION_REQUIRED:
      misc->abe_share->modcode = 0;
      break;


      //  From hofWaveFilter

    case PFM_CHARTS_HOF_DATA:

      for (NV_INT32 i = 0 ; i < misc->abe_share->point_cloud_count ; i++)
        {
          if (misc->data[i].exflag && !misc->data[i].mask)
            {
              misc->data[i].exflag = NVFalse;
              misc->filter_kill_list = (NV_INT32 *) realloc (misc->filter_kill_list, (misc->filter_kill_count + 1) * sizeof (NV_INT32));

              if (misc->filter_kill_list == NULL)
                {
                  perror ("Allocating misc->filter_kill_list memory in externalFilter.cpp");
                  exit (-1);
                }

              misc->filter_kill_list[misc->filter_kill_count] = i;
              misc->filter_kill_count++;
            }
        }
      misc->abe_share->modcode = 0;
      break;
    }


  //  Force a redraw of the waveMonitor just in case.

  misc->abe_share->modcode = WAVEMONITOR_FORCE_REDRAW;


  misc->statusProgLabel->setVisible (TRUE);
  misc->statusProg->setRange (0, 100);
  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  emit externalFilterDone ();
}



void 
externalFilter::slotReadyReadStandardError ()
{
  QByteArray response = this->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



void 
externalFilter::slotReadyReadStandardOutput ()
{
  QByteArray response = this->readAllStandardOutput ();
  NV_CHAR *res = response.data ();

  fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}
