#include "inputPage.hpp"
#include "inputPageHelp.hpp"

inputPage::inputPage (QWidget *parent):
  QWizardPage (parent)
{
  imageProc = NULL;
  killFlag = NVFalse;


  setTitle (tr ("Select images"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *inputBox = new QGroupBox (tr ("Image file list"), this);
  QHBoxLayout *inputBoxLayout = new QHBoxLayout;
  inputBox->setLayout (inputBoxLayout);
  inputBoxLayout->setSpacing (10);


  imageTable = new QTableWidget (1, 3, this);
  imageTable->setWhatsThis (imageTableText);
  imageTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *thItem = new QTableWidgetItem (tr ("Display"));
  imageTable->setHorizontalHeaderItem (0, thItem);
  thItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *fnItem = new QTableWidgetItem (tr ("File name"));
  imageTable->setHorizontalHeaderItem (1, fnItem);
  fnItem->setTextAlignment (Qt::AlignHCenter);
  QTableWidgetItem *dItem = new QTableWidgetItem (tr ("Do not use"));
  imageTable->setHorizontalHeaderItem (2, dItem);
  dItem->setTextAlignment (Qt::AlignHCenter);

  inputBoxLayout->addWidget (imageTable);


  vbox->addWidget (inputBox, 10);
}



NV_BOOL 
inputPage::getCheckBox (NV_INT32 row)
{
  if (imaged[row]->checkState () == Qt::Checked) return (NVTrue);

  return (NVFalse);
}



void 
inputPage::slotViewError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmChartsImage image viewer"), tr ("Unable to start the image viewer process!"));
      break;

    case QProcess::Crashed:
      if (!killFlag) QMessageBox::critical (this, tr ("pfmChartsImage image viewer"), tr ("The image viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmChartsImage image viewer"), tr ("The image viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmChartsImage image viewer"), 
                             tr ("There was a write error to the image viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmChartsImage image viewer"),
                             tr ("There was a read error from the image viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmChartsImage image viewer"),
                             tr ("The image viewer process died with an unknown error!"));
      break;
    }
}



void 
inputPage::slotDisplay (int row)
{
  if (imageProc && imageProc->state () == QProcess::Running)
    {
      killFlag = NVTrue;
      imageProc->kill ();
      delete (imageProc);
      killFlag = NVFalse;
    }
  imageProc = new QProcess (this);


  QString arg;
  QStringList arguments;


  arguments += filenames.at (row);

  arg.sprintf ("%f", 0.0);
  arguments += arg;


  connect (imageProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotViewError (QProcess::ProcessError)));

  imageProc->start ("targetPic", arguments);
}



void 
inputPage::initializeTable (NV_INT32 icount, QString dir, QString txt_file)
{
  strcpy (data_dir, dir.toAscii ());
  strcpy (text_file, txt_file.toAscii ());
  count = icount;


  if ((tfp = fopen (text_file, "r")) == NULL)
    {
      perror (text_file);
      exit (-1);
    }


  imaged = (QCheckBox **) calloc (count, sizeof (QCheckBox *));
  vItem = (QTableWidgetItem **) calloc (count, sizeof (QTableWidgetItem *));
  imageTable->setRowCount (count);


  imageGrp = new QButtonGroup (this);
  imageGrp->setExclusive (FALSE);

  displayGrp = new QButtonGroup (this);
  displayGrp->setExclusive (FALSE);
  connect (displayGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotDisplay (int)));


  for (NV_INT32 i = 0 ; i < count ; i++)
    {
      NV_CHAR string[1024], filename[512], dirfile[512];
      NV_INT32 dummy;

      fgets (string, sizeof (string), tfp);
      sscanf (string, "%d %s", &dummy, dirfile);

      filenames << QString (dirfile);

      strcpy (filename, pfm_basename (dirfile));

      QString vh;
      vh.sprintf ("%d", i);
      vItem[i] = new QTableWidgetItem (vh);
      imageTable->setVerticalHeaderItem (i, vItem[i]);


      //QIcon thumb;
      //thumb.addFile (dirfile, QSize (32, 24), QIcon::Normal);
      //QPushButton *dispButton = new QPushButton (thumb, "", this);
      //dispButton->setIconSize (QSize (32, 24));
      QPushButton *dispButton = new QPushButton (tr ("Display"), this);
      displayGrp->addButton (dispButton, i);
      QString tip;
      tip.sprintf (tr ("Press this button to display image %d").toAscii (), i);
      dispButton->setToolTip (tip);
      imageTable->setCellWidget (i, 0, dispButton);


      QTableWidgetItem *fn = new QTableWidgetItem (QString (filename));
      imageTable->setItem (i, 1, fn);
      fn->setFlags (Qt::ItemIsEnabled);


      imaged[i] = new QCheckBox (this);
      imageTable->setCellWidget (i, 2, imaged[i]);
      imageGrp->addButton (imaged[i], i);

      imaged[i]->setCheckState (Qt::Unchecked);
    }
  imageTable->resizeColumnsToContents ();
  imageTable->resizeRowsToContents ();


  NV_INT32 width = qMin (800, imageTable->columnWidth (0) + imageTable->columnWidth (1) + 60);
  NV_INT32 height = qMin (600, imageTable->rowHeight (0) * count + 75);
  resize (width, height);


  //  Connect after populating table so the slot doesn't get called.

  connect (imageGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));


  fclose (tfp);
}



void
inputPage::slotValueChanged (int row)
{
  //  We really only want to do something if we've clicked a checkbox inside a selected
  //  range.  In that case, we want to set/unset checkboxes for all of the selected range

  QList<QTableWidgetSelectionRange> ranges = imageTable->selectedRanges ();


  if (ranges.size () > 0)
    {
      rng.clear ();

      for (NV_INT32 i = 0 ; i < ranges.size () ; i++)
        {
          for (NV_INT32 j = 0 ; j < ranges.at (i).rowCount () ; j++)
            {
              rng << ranges.at (i).topRow () + j;
            }
        }


      NV_BOOL hit = NVFalse;

      if (imaged[row]->checkState () == Qt::Checked)
        {
          for (NV_INT32 i = 0 ; i < rng.size () ; i++)
            {
              if (row == rng.at (i))
                {
                  hit = NVTrue;
                  break;
                }
            }

          if (hit)
            {
              disconnect (imageGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));
              
              for (NV_INT32 i = 0 ; i < rng.size () ; i++)
                {
                  imaged[rng.at (i)]->setCheckState (Qt::Checked);
                }
            }
        }
      else
        {
          for (NV_INT32 i = 0 ; i < rng.size () ; i++)
            {
              if (row == rng.at (i))
                {
                  hit = NVTrue;
                  break;
                }
            }

          if (hit)
            {
              disconnect (imageGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));

              for (NV_INT32 i = 0 ; i < rng.size () ; i++)
                {
                  imaged[rng.at (i)]->setCheckState (Qt::Unchecked);
                }
            }
        }


      //  If we had a hit, reconnect the checkbox button group.

      if (hit) connect (imageGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotValueChanged (int)));
    }
}
