#include "inputPage.hpp"
#include "inputPageHelp.hpp"

inputPage::inputPage (QWidget *parent, OPTIONS *op, QTextEdit **infiles):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Input Data Files"));


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/trackLineWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QHBoxLayout *browseBox = new QHBoxLayout;
  browseBox->setSpacing (5);


  QGroupBox *fileBox = new QGroupBox (tr ("Files"), this);
  QHBoxLayout *fileBoxLayout = new QHBoxLayout;
  fileBox->setLayout (fileBoxLayout);
  fileBoxLayout->setSpacing (10);


  inputBrowse = new QPushButton (tr ("Browse"), this);
  inputBrowse->setToolTip (tr ("Add input files to the list"));
  inputBrowse->setWhatsThis (inputBrowseText);
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
  fileMask->setWhatsThis (fileMaskText);
  fileMask->setEditable (TRUE);
  fileMask->addItem ("GSF (*.d\?\? *.gsf)");
  fileMask->addItem ("POS (*.pos *.POS)");
  fileMask->addItem ("SBET (*.out *.OUT)");
  fileMask->addItem ("WLF (*.wlf *.wtf *.whf)");
  fileMask->addItem ("Hawkeye (*.bin)");
  maskBoxLayout->addWidget (fileMask);

  if (options->inputFilter == "POS (*.pos *.POS)")
    {
      fileMask->setCurrentIndex (1);
    }
  if (options->inputFilter == "SBET (*.out *.OUT)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (options->inputFilter == "WLF (*.wlf *.wtf *.whf)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (options->inputFilter == "Hawkeye (*.bin)")
    {
      fileMask->setCurrentIndex (4);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
  connect (fileMask, SIGNAL (currentIndexChanged (const QString &)), this, SLOT (slotFileMaskTextChanged (const QString &)));
  dirBoxLayout->addWidget (maskBox);


  dirBrowse = new QPushButton (tr ("Browse"), this);
  dirBrowse->setToolTip (tr ("Add input directory contents to the list"));
  dirBrowse->setWhatsThis (dirBrowseText);
  connect (dirBrowse, SIGNAL (clicked ()), this, SLOT (slotDirBrowseClicked ()));
  dirBoxLayout->addWidget (dirBrowse);


  browseBox->addWidget (dirBox, 1);


  vbox->addLayout (browseBox);


  QGroupBox *inputBox = new QGroupBox (tr ("Input file list"), this);
  QHBoxLayout *inputBoxLayout = new QHBoxLayout;
  inputBox->setLayout (inputBoxLayout);
  inputBoxLayout->setSpacing (10);


  inputFiles = new QTextEdit (this);
  inputFiles->setWhatsThis (inputFilesText);
  inputFiles->setLineWrapMode (QTextEdit::NoWrap);
  inputBoxLayout->addWidget (inputFiles);


  vbox->addWidget (inputBox, 10);


  *infiles = inputFiles;
}



void 
inputPage::slotInputBrowseClicked ()
{
  QFileDialog fd (this, tr ("trackLine Input files"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, options->input_dir);


  QStringList filters;
  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("POS (*.pos *.POS)")
          << tr ("SBET (*.out *.OUT)")
          << tr ("WLF (*.wlf *.wtf *.whf)")
          << tr ("Hawkeye (*.bin)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFiles);
  fd.selectFilter (options->inputFilter);


  QStringList files;


  if (fd.exec () == QDialog::Accepted) 
    {
      files = fd.selectedFiles ();

      for (NV_INT32 i = 0 ; i < files.size () ; i++)
        {
          inputFiles->append (files.at (i));
        }
    }

  options->input_dir = fd.directory ().absolutePath ();
  options->inputFilter = fd.selectedFilter ();

  if (options->inputFilter == "POS (*.pos *.POS)")
    {
      fileMask->setCurrentIndex (1);
    }
  if (options->inputFilter == "SBET (*.out *.OUT)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (options->inputFilter == "WLF (*.wlf *.wtf *.whf)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (options->inputFilter == "Hawkeye (*.bin)")
    {
      fileMask->setCurrentIndex (4);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
}



void 
inputPage::slotDirBrowseClicked ()
{
  QString title = tr ("trackLine Input directories, mask = ") + options->inputFilter;
  QFileDialog *fd = new QFileDialog (this, title);
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  QStringList filters;
  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("POS (*.pos *.POS)")
          << tr ("SBET (*.out *.OUT)")
          << tr ("WLF (*.wlf *.wtf *.whf)")
          << tr ("Hawkeye (*.bin)");

  fd->setFilters (filters);

  fd->setFileMode (QFileDialog::Directory);

  fd->setFilter (options->inputFilter);


  //  Make the directory file dialog support multiple directories.

  QListView *fdListView = fd->findChild<QListView*> ("listView");

  if (fdListView)
    {
      fdListView->setSelectionMode (QAbstractItemView::ExtendedSelection);
    }

  QTreeView *fdTreeView = fd->findChild<QTreeView*> ();

  if (fdTreeView)
    {
      fdTreeView->setSelectionMode (QAbstractItemView::ExtendedSelection);
    }


  QString file;
  QStringList files;

  if (fd->exec () == QDialog::Accepted) 
    {
      //  Save the directory that we were in when we selected a directory.

      options->input_dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();

      for (NV_INT32 i = 0 ; i < files.size () ; i++)
        {
          file = files.at (i);

          if (file.isEmpty ()) file = fd->directory ().absolutePath ();

          QDir dirs;
          dirs.cd (file);

          dirs.setFilter (QDir::Dirs | QDir::Readable);


          //  Get all matching files in this directory.

          QDir files;
          files.setFilter (QDir::Files | QDir::Readable);

          QStringList nameFilter;
          if (options->inputFilter == "GSF (*.d\?\? *.gsf)")
            {
              nameFilter << "*.d\?\?" << "*.gsf";
            }
          else if (options->inputFilter == "POS (*.pos *.POS)")
            {
              nameFilter << "*.pos" << "*.POS";
            }
          else if (options->inputFilter == "SBET (*.out *.OUT)")
            {
              nameFilter << "*.out" << "*.OUT";
            }
          else if (options->inputFilter == "WLF (*.wlf *.wtf *.whf)")
            {
              nameFilter << "*.wlf" << "*.wtf" << "*.whf";
            }
          else if (options->inputFilter == "Hawkeye (*.bin)")
            {
              nameFilter << "*.bin";
            }
          else
            {
              nameFilter << options->inputFilter;
            }
          files.setNameFilters (nameFilter);

          if (files.cd (file))
            {
              QFileInfoList flist = files.entryInfoList ();
              for (NV_INT32 i = 0 ; i < flist.size () ; i++)
                {
                  QString tst = flist.at (i).absoluteFilePath ();

                  inputFiles->append (tst);
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
                      QString tst = flist.at (i).absoluteFilePath ();

                      inputFiles->append (tst);
                    }
                }
            }
        }
    }
}


void 
inputPage::slotFileMaskTextChanged (const QString &text)
{
  options->inputFilter = text;
}
