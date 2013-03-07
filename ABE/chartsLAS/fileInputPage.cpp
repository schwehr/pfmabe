#include "fileInputPage.hpp"
#include "fileInputPageHelp.hpp"

fileInputPage::fileInputPage (QWidget *parent, QString *inFilt, QTextEdit **infiles):
  QWizardPage (parent)
{
  dir = ".";
  inputFilter = inFilt;


  setTitle (tr ("CHARTS File Input"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/chartsLASWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QHBoxLayout *browseBox = new QHBoxLayout;
  browseBox->setSpacing (5);


  QGroupBox *fileBox = new QGroupBox (tr ("Input files"), this);
  QHBoxLayout *fileBoxLayout = new QHBoxLayout;
  fileBox->setLayout (fileBoxLayout);
  fileBoxLayout->setSpacing (10);


  inputBrowse = new QPushButton (tr ("Browse"), this);
  inputBrowse->setToolTip (tr ("Add input files to the list"));
  inputBrowse->setWhatsThis (fileInputBrowseText);
  connect (inputBrowse, SIGNAL (clicked ()), this, SLOT (slotInputBrowseClicked ()));
  fileBoxLayout->addWidget (inputBrowse);
  browseBox->addWidget (fileBox, 1);


  QGroupBox *dirBox = new QGroupBox (tr ("Directories"), this);
  QHBoxLayout *dirBoxLayout = new QHBoxLayout;
  dirBox->setLayout (dirBoxLayout);
  dirBoxLayout->setSpacing (10);


  QGroupBox *maskBox = new QGroupBox (tr ("Directory file mask"), this);
  QHBoxLayout *maskBoxLayout = new QHBoxLayout;
  maskBox->setLayout (maskBoxLayout);
  maskBoxLayout->setSpacing (10);

  fileMask = new QComboBox (this);
  fileMask->setToolTip (tr ("Set the file mask for the directory Browse button"));
  fileMask->setWhatsThis (fileFileMaskText);
  fileMask->setEditable (FALSE);
  fileMask->addItem ("TOF (*.tof)");
  fileMask->addItem ("HOF (*.hof)");
  maskBoxLayout->addWidget (fileMask);

  if (*inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (1);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
  connect (fileMask, SIGNAL (currentIndexChanged (const QString &)), this, 
           SLOT (slotFileMaskTextChanged (const QString &)));
  dirBoxLayout->addWidget (maskBox);


  dirBrowse = new QPushButton (tr ("Browse"), this);
  dirBrowse->setToolTip (tr ("Add input directory contents to the list"));
  dirBrowse->setWhatsThis (fileDirBrowseText);
  connect (dirBrowse, SIGNAL (clicked ()), this, SLOT (slotDirBrowseClicked ()));
  dirBoxLayout->addWidget (dirBrowse);


  browseBox->addWidget (dirBox, 1);


  vbox->addLayout (browseBox);


  QGroupBox *inputBox = new QGroupBox (tr ("Input file list"), this);
  QHBoxLayout *inputBoxLayout = new QHBoxLayout;
  inputBox->setLayout (inputBoxLayout);
  inputBoxLayout->setSpacing (10);


  inputFiles = new QTextEdit (this);
  inputFiles->setWhatsThis (fileInputFilesText);
  inputFiles->setLineWrapMode (QTextEdit::NoWrap);
  inputBoxLayout->addWidget (inputFiles);


  vbox->addWidget (inputBox, 10);


  *infiles = inputFiles;
}



void 
fileInputPage::slotInputBrowseClicked ()
{
  QFileDialog fd (this, tr ("chartsLAS Input files"));
  fd.setViewMode (QFileDialog::List);
  fd.setDirectory (dir);

  QStringList filters;
  filters << tr ("TOF (*.tof)")
          << tr ("HOF (*.hof)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFiles);
  fd.selectFilter (*inputFilter);


  QStringList files;


  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();

      for (NV_INT32 i = 0 ; i < files.size () ; i++)
        {
          inputFiles->append (files.at (i));
        }
    }

  dir = fd.directory ().path ();
  *inputFilter = fd.selectedFilter ();

  if (*inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (1);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
}



void 
fileInputPage::slotDirBrowseClicked ()
{
  QString title = tr ("chartsLAS Input directories, mask = ") + *inputFilter;
  QFileDialog *fd = new QFileDialog (this, title);
  fd->setViewMode (QFileDialog::List);


  fd->setDirectory (dir);


  QStringList filters;
  filters << tr ("TOF (*.tof)")
          << tr ("HOF (*.hof)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::Directory);
  fd->setFilter (*inputFilter);


  QString file;
  QStringList files;

  if (fd->exec () == QDialog::Accepted) 
    {
      //  Save the directory that we were in when we selected a directory.

      dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();
      file = files.at (0);

      if (file.isEmpty ()) file = fd->directory ().absolutePath ();


      QDir dirs;
      dirs.cd (file);

      dirs.setFilter (QDir::Dirs | QDir::Readable);


      //  Get all matching files in this directory.

      QDir files;
      files.setFilter (QDir::Files | QDir::Readable);

      QStringList nameFilter;
      if (*inputFilter == "HOF (*.hof)")
        {
          nameFilter << "*.hof";
        }
      else
        {
          nameFilter << "*.tof";
        }
      files.setNameFilters (nameFilter);

      if (files.cd (file))
        {
          QFileInfoList flist = files.entryInfoList ();
          for (NV_INT32 i = 0 ; i < flist.size () ; i++)
            {
              //  Don't load HOF timing lines by default.  These can still be loaded using the file browser.

              QString tst = flist.at (i).absoluteFilePath ();

              if (!inputFilter->contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                {
                  inputFiles->append (tst);
                }
            }
        }


      //  Get all directories in this directory.

      QFileInfoList dlist = dirs.entryInfoList ();
      QStringList dirList;
      for (NV_INT32 i = 0 ; i < dlist.size () ; i++)
        {
          if (dlist.at (i).fileName () != "." && dlist.at (i).fileName () != "..") 
            dirList.append (dlist.at (i).absoluteFilePath ());
        }


      //  Get all subordinate directories.

      for (NV_INT32 i = 0 ; i < dirList.size () ; i++)
        {
          QString dirName = dirList.at (i);

          if (dirs.cd (dirName))
            {
              QFileInfoList nlist = dirs.entryInfoList ();
              for (NV_INT32 i = 0 ; i < nlist.size () ; i++)
                {
                  if (nlist.at (i).fileName () != "." && nlist.at (i).fileName () != "..") 
                    dirList.append (nlist.at (i).absoluteFilePath ());
                }
            }
        }


      //  Get all matching files in all subordinate directories

      for (NV_INT32 i = 0 ; i < dirList.size () ; i++)
        {
          files.setFilter (QDir::Files | QDir::Readable);
          files.setNameFilters (nameFilter);

          QString dirName = dirList.at (i);

          if (files.cd (dirName))
            {
              QFileInfoList flist = files.entryInfoList ();
              for (NV_INT32 i = 0 ; i < flist.size () ; i++)
                {
                  //  Don't load HOF timing lines by default.  These can still be loaded using the file browser.

                  QString tst = flist.at (i).absoluteFilePath ();

                  if (!inputFilter->contains ("*.hof") || tst.mid (tst.length () - 13, 4) != "_TA_")
                    {
                      inputFiles->append (tst);
                    }
                }
            }
        }
    }
}


void 
fileInputPage::slotFileMaskTextChanged (const QString &text)
{
  *inputFilter = text;
}
