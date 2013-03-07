
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



#include "inputPage.hpp"
#include "inputPageHelp.hpp"

inputPage::inputPage (QWidget *parent, QString *inFilt, QTextEdit **infiles, PFM_GLOBAL *pfmg):
  QWizardPage (parent)
{
  pfm_global = pfmg;
  inputFilter = inFilt;


  setTitle (tr ("Input Data Files"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmLoadWatermark.png"));


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
  fileMask->addItem ("WLF (*.wlf *.wtf *.whf)");
  fileMask->addItem ("HAWKEYE (*.bin)");
  fileMask->addItem ("HOF (*.hof)");
  fileMask->addItem ("TOF (*.tof)");
  fileMask->addItem ("UNISIPS (*.u)");
  fileMask->addItem ("YXZ (*.yxz *.txt)");
  fileMask->addItem ("HYPACK (*.raw )");
  fileMask->addItem ("IVS XYZ (*.xyz)");
  fileMask->addItem ("LLZ (*.llz)");
  fileMask->addItem ("CZMIL (*.cxy)");
  fileMask->addItem ("DTED (*.dt1 *.dt2)");
  fileMask->addItem ("CHRTR (*.fin *.ch2)");
  fileMask->addItem ("BAG (*.bag)");
  fileMask->addItem ("LIST (*.lst)");
  maskBoxLayout->addWidget (fileMask);

  if (*inputFilter == "WLF (*.wlf *.wtf *.whf)")
    {
      fileMask->setCurrentIndex (1);
    }
  else if (*inputFilter == "HAWKEYE (*.bin)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (*inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (*inputFilter == "TOF (*.tof)")
    {
      fileMask->setCurrentIndex (4);
    }
  else if (*inputFilter == "UNISIPS (*.u)")
    {
      fileMask->setCurrentIndex (5);
    }
  else if (*inputFilter == "YXZ (*.yxz *.txt)")
    {
      fileMask->setCurrentIndex (6);
    }
  else if (*inputFilter == "HYPACK (*.raw )")
    {
      fileMask->setCurrentIndex (7);
    }
  else if (*inputFilter == "IVS XYZ (*.xyz)")
    {
      fileMask->setCurrentIndex (8);
    }
  else if (*inputFilter == "LLZ (*.llz)")
    {
      fileMask->setCurrentIndex (9);
    }
  else if (*inputFilter == "CZMIL (*.cxy)")
    {
      fileMask->setCurrentIndex (10);
    }
  else if (*inputFilter == "DTED (*.dt1 *.dt2)")
    {
      fileMask->setCurrentIndex (11);
    }
  else if (*inputFilter == "CHRTR (*.fin *.ch2)")
    {
      fileMask->setCurrentIndex (12);
    }
  else if (*inputFilter == "BAG (*.bag)")
    {
      fileMask->setCurrentIndex (13);
    }
  else if (*inputFilter == "LIST (*.lst)")
    {
      fileMask->setCurrentIndex (14);
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


  QGroupBox *pfmBox = new QGroupBox (tr ("PFM files"), this);
  QHBoxLayout *pfmBoxLayout = new QHBoxLayout;
  pfmBox->setLayout (pfmBoxLayout);
  pfmBoxLayout->setSpacing (10);


  pfmBrowse = new QPushButton (tr ("Browse"), this);
  pfmBrowse->setToolTip (tr ("Add input files from a preexisting PFM structure to the list"));
  pfmBrowse->setWhatsThis (pfmBrowseText);
  connect (pfmBrowse, SIGNAL (clicked ()), this, SLOT (slotPfmBrowseClicked ()));
  pfmBoxLayout->addWidget (pfmBrowse);
  browseBox->addWidget (pfmBox, 1);


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
  QFileDialog fd (this, tr ("pfmLoad Input files"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, pfm_global->input_dir);


  QStringList filters;
  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("WLF (*.wlf *.wtf *.whf)")
          << tr ("HAWKEYE (*.bin)")
          << tr ("HOF (*.hof)")
          << tr ("TOF (*.tof)")
          << tr ("UNISIPS (*.u)")
          << tr ("YXZ (*.yxz *.txt)")
          << tr ("HYPACK (*.raw )")
          << tr ("IVS XYZ (*.xyz)")
          << tr ("LLZ (*.llz)")
          << tr ("CZMIL (*.cxy)")
          << tr ("DTED (*.dt1 *.dt2)")
          << tr ("CHRTR (*.fin *.ch2)")
          << tr ("BAG (*.bag)")
          << tr ("LIST (*.lst)")
          << tr ("All files (*)");

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

  pfm_global->input_dir = fd.directory ().absolutePath ();
  *inputFilter = fd.selectedFilter ();

  if (*inputFilter == "WLF (*.wlf *.wtf *.whf)")
    {
      fileMask->setCurrentIndex (1);
    }
  else if (*inputFilter == "HAWKEYE (*.bin)")
    {
      fileMask->setCurrentIndex (2);
    }
  else if (*inputFilter == "HOF (*.hof)")
    {
      fileMask->setCurrentIndex (3);
    }
  else if (*inputFilter == "TOF (*.tof)")
    {
      fileMask->setCurrentIndex (4);
    }
  else if (*inputFilter == "UNISIPS (*.u)")
    {
      fileMask->setCurrentIndex (5);
    }
  else if (*inputFilter == "YXZ (*.yxz *.txt)")
    {
      fileMask->setCurrentIndex (6);
    }
  else if (*inputFilter == "HYPACK (*.raw )")
    {
      fileMask->setCurrentIndex (7);
    }
  else if (*inputFilter == "IVS XYZ (*.xyz)")
    {
      fileMask->setCurrentIndex (8);
    }
  else if (*inputFilter == "LLZ (*.llz)")
    {
      fileMask->setCurrentIndex (9);
    }
  else if (*inputFilter == "CZMIL (*.cxy)")
    {
      fileMask->setCurrentIndex (10);
    }
  else if (*inputFilter == "DTED (*.dt1 *.dt2)")
    {
      fileMask->setCurrentIndex (11);
    }
  else if (*inputFilter == "CHRTR (*.fin *.ch2)")
    {
      fileMask->setCurrentIndex (12);
    }
  else if (*inputFilter == "BAG (*.bag)")
    {
      fileMask->setCurrentIndex (13);
    }
  else if (*inputFilter == "LIST (*.lst)")
    {
      fileMask->setCurrentIndex (14);
    }
  else
    {
      fileMask->setCurrentIndex (0);
    }
}



void 
inputPage::slotDirBrowseClicked ()
{
  QString title = tr ("pfmLoad Input directories, mask = ") + *inputFilter;
  QFileDialog *fd = new QFileDialog (this, title);
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, pfm_global->input_dir);


  QStringList filters;
  filters << tr ("GSF (*.d\?\? *.gsf)")
          << tr ("WLF (*.wlf *.wtf *.whf)")
          << tr ("HAWKEYE (*.bin)")
          << tr ("HOF (*.hof)")
          << tr ("TOF (*.tof)")
          << tr ("UNISIPS (*.u)")
          << tr ("YXZ (*.yxz *.txt)")
          << tr ("HYPACK (*.raw )")
          << tr ("IVS XYZ (*.xyz)")
          << tr ("LLZ (*.llz)")
          << tr ("CZMIL (*.cxy)")
          << tr ("DTED (*.dt1 *.dt2)")
          << tr ("CHRTR (*.fin *.ch2)")
          << tr ("BAG (*.bag)")
          << tr ("LIST (*.lst)")
          << tr ("All files (*)");

  fd->setFilters (filters);

  fd->setFileMode (QFileDialog::Directory);

  fd->setFilter (*inputFilter);


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

      pfm_global->input_dir = fd->directory ().absolutePath ();

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
          QString type;
          if (*inputFilter == "GSF (*.d\?\? *.gsf)")
            {
              type = "GSF";
              nameFilter << "*.d\?\?" << "*.gsf";
            }
          else if (*inputFilter == "WLF (*.wlf *.wtf *.whf)")
            {
              type = "WLF";
              nameFilter << "*.wlf" << "*.wtf" << "*.whf";
            }
          else if (*inputFilter == "HAWKEYE (*.bin)")
            {
              type = "HAWKEYE";
              nameFilter << "*.bin";
            }
          else if (*inputFilter == "HOF (*.hof)")
            {
              type = "HOF";
              nameFilter << "*.hof";
            }
          else if (*inputFilter == "TOF (*.tof)")
            {
              type = "TOF";
              nameFilter << "*.tof";
            }
          else if (*inputFilter == "UNISIPS (*.u)")
            {
              type = "UNISIPS";
              nameFilter << "*.u";
            }
          else if (*inputFilter == "YXZ (*.yxz *.txt)")
            { 
              type = "YXZ";
              nameFilter << "*.yxz" << "*.txt";
            }
          else if (*inputFilter == "HYPACK (*.raw )")
            {
              type = "HYPACK";
              nameFilter << "*.raw";
            }
          else if (*inputFilter == "IVS XYZ (*.xyz)")
            {
              type = "IVS XYZ";
              nameFilter << "*.xyz";
            }
          else if (*inputFilter == "LLZ (*.llz)")
            {
              type = "LLZ";
              nameFilter << "*.llz";
            }
          else if (*inputFilter == "CZMIL (*.cxy)")
            {
              type = "CZMIL";
              nameFilter << "*.cxy";
            }
          else if (*inputFilter == "DTED (*.dt1 *.dt2)")
            {
              type = "DTED";
              nameFilter << "*.dt1" << "*.dt2";
            }
          else if (*inputFilter == "CHRTR (*.fin *.ch2)")
            {
              type = "CHRTR";
              nameFilter << "*.fin" << "*.ch2";
            }
          else if (*inputFilter == "BAG (*.bag)")
            {
              type = "BAG";
              nameFilter << "*.bag";
            }
          else if (*inputFilter == "LIST (*.lst)")
            {
              type = "LIST";
              nameFilter << "*.lst";
            }
          else
            {
              nameFilter << *inputFilter;
            }
          files.setNameFilters (nameFilter);


          //  Add the filter and directory to the directories list

          pfm_global->input_dirs += (type + ":" + file);


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
}



void 
inputPage::slotPfmBrowseClicked ()
{
  QStringList         files, filters;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmLoad Input PFM Structure"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, pfm_global->input_dir);


  filters << tr ("PFM (*.pfm)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("PFM (*.pfm)"));

  if (fd->exec () == QDialog::Accepted)
    {
      //  Save the directory that we were in when we selected a file.

      pfm_global->input_dir = fd->directory ().absolutePath ();


      files = fd->selectedFiles ();

      QString file = files.at (0);


      if (!file.isEmpty())
        {
          NV_INT32 temp_handle;
          PFM_OPEN_ARGS open_args;

          strcpy (open_args.list_path, file.toAscii ());

          open_args.checkpoint = 0;

          if ((temp_handle = open_existing_pfm_file (&open_args)) < 0)
            {
              QMessageBox::warning (this, tr ("pfmLoad Input PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" is not a PFM structure or there was an error reading the file.") +
                                    tr ("  The error message returned was:\n\n") +
                                    QString (pfm_error_str (pfm_error)));
            }
          else
            {
              NV_INT16 i = 0, type;
              NV_CHAR rfile[1024];
              while (!read_list_file (temp_handle, i, rfile, &type))
                {
                  inputFiles->append (QString (rfile));
                  i++;
                }

              close_pfm_file (temp_handle);
            }
        }
    }
}


void 
inputPage::slotFileMaskTextChanged (const QString &text)
{
  *inputFilter = text;
}
