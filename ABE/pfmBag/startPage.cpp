#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("pfmBag is a tool for creating a Bathymetric Attributed Grid (BAG) file "
				  "that represents a best fit surface from a PFM structure.  Help is available "
                                  "by clicking on the Help button and then clicking on the item for which "
                                  "you want help.  Select a PFM file below.  You may then change the default "
                                  "output file name and, optionally, select an area file to limit the extent "
                                  "of the BAG file that is created.  You may also select an optional ellipsoid "
                                  "to datum separation file to be inserted in the BAG file.  Click <b>Next</b> "
                                  "to continue or <b>Cancel</b> to exit."));
  label->setWordWrap (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QHBoxLayout *pfm_file_box = new QHBoxLayout (0);
  pfm_file_box->setSpacing (8);

  vbox->addLayout (pfm_file_box);


  QLabel *pfm_file_label = new QLabel (tr ("PFM File"), this);
  pfm_file_box->addWidget (pfm_file_label, 1);

  pfm_file_edit = new QLineEdit (this);
  pfm_file_edit->setReadOnly (TRUE);
  pfm_file_box->addWidget (pfm_file_edit, 10);

  QPushButton *pfm_file_browse = new QPushButton (tr ("Browse..."), this);
  pfm_file_box->addWidget (pfm_file_browse, 1);

  pfm_file_label->setWhatsThis (pfm_fileText);
  pfm_file_edit->setWhatsThis (pfm_fileText);
  pfm_file_browse->setWhatsThis (pfm_fileBrowseText);

  connect (pfm_file_browse, SIGNAL (clicked ()), this, SLOT (slotPFMFileBrowse ()));



  QHBoxLayout *output_box = new QHBoxLayout (0);
  output_box->setSpacing (8);

  vbox->addLayout (output_box);


  QLabel *output_file_label = new QLabel (tr ("Output BAG File"), this);
  output_box->addWidget (output_file_label, 1);

  output_file_edit = new QLineEdit (this);
  output_box->addWidget (output_file_edit, 10);

  QPushButton *output_file_browse = new QPushButton (tr ("Browse..."), this);
  output_box->addWidget (output_file_browse, 1);

  output_file_label->setWhatsThis (output_fileText);
  output_file_edit->setWhatsThis (output_fileText);
  output_file_browse->setWhatsThis (output_fileBrowseText);

  connect (output_file_browse, SIGNAL (clicked ()), this, SLOT (slotOutputFileBrowse ()));


  QHBoxLayout *area_box = new QHBoxLayout (0);
  area_box->setSpacing (8);

  vbox->addLayout (area_box);


  QLabel *area_file_label = new QLabel (tr ("Optional Area File"), this);
  area_box->addWidget (area_file_label, 1);

  area_file_edit = new QLineEdit (this);
  area_file_edit->setReadOnly (TRUE);
  area_box->addWidget (area_file_edit, 10);

  QPushButton *area_file_browse = new QPushButton (tr ("Browse..."), this);
  area_box->addWidget (area_file_browse, 1);


  area_file_label->setWhatsThis (area_fileText);
  area_file_edit->setWhatsThis (area_fileText);
  area_file_browse->setWhatsThis (area_fileBrowseText);

  connect (area_file_browse, SIGNAL (clicked ()), this, SLOT (slotAreaFileBrowse ()));


  QHBoxLayout *sep_box = new QHBoxLayout (0);
  sep_box->setSpacing (8);

  vbox->addLayout (sep_box);


  QLabel *sep_file_label = new QLabel (tr ("Optional Separation File"), this);
  sep_box->addWidget (sep_file_label, 1);

  sep_file_edit = new QLineEdit (this);
  sep_file_edit->setReadOnly (TRUE);
  sep_box->addWidget (sep_file_edit, 10);

  QPushButton *sep_file_browse = new QPushButton (tr ("Browse..."), this);
  sep_box->addWidget (sep_file_browse, 1);


  sep_file_label->setWhatsThis (sep_fileText);
  sep_file_edit->setWhatsThis (sep_fileText);
  sep_file_browse->setWhatsThis (sep_fileBrowseText);

  connect (sep_file_browse, SIGNAL (clicked ()), this, SLOT (slotSepFileBrowse ()));


  //  If there was a file name on the command line put it in the file slot.

  if (*argc == 2)
    {
      PFM_OPEN_ARGS open_args;
      NV_INT32 pfm_handle = -1;

      QString pfm_file_name = QString (argv[1]);

      strcpy (open_args.list_path, argv[1]);

      open_args.checkpoint = 0;
      pfm_handle = open_existing_pfm_file (&open_args);

      if (pfm_handle >= 0)
        {
          pfm_file_edit->setText (pfm_file_name);


          //  If one hasn't been set, set the output BAG filename.

          if (output_file_edit->text ().isEmpty ())
            { 
              QString output_file_name = pfm_file_name + ".bag";
              output_file_edit->setText (output_file_name);
            }

          close_pfm_file (pfm_handle);
        }
    }


  if (!pfm_file_edit->text ().isEmpty ())
    {
      registerField ("pfm_file_edit", pfm_file_edit);
    }
  else
    {
      registerField ("pfm_file_edit*", pfm_file_edit);
    }


  registerField ("output_file_edit", output_file_edit);
  registerField ("area_file_edit", area_file_edit);
  registerField ("sep_file_edit", sep_file_edit);
}



void startPage::slotPFMFileBrowse ()
{
  PFM_OPEN_ARGS       open_args;
  QStringList         files, filters;
  QString             file;
  NV_INT32            pfm_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmBag Open PFM Structure"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->input_dir);


  filters << tr ("PFM (*.pfm)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("PFM (*.pfm)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString pfm_file_name = files.at (0);


      if (!pfm_file_name.isEmpty())
        {
          strcpy (open_args.list_path, pfm_file_name.toAscii ());

          open_args.checkpoint = 0;
          pfm_handle = open_existing_pfm_file (&open_args);

          if (pfm_handle < 0)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                    tr (" is not a PFM structure or there was an error reading the file.") +
                                    tr ("  The error message returned was:\n\n") +
                                    QString (pfm_error_str (pfm_error)));

              if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
                {
                  fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
                  exit (-1);
                }

              return;
            }

          if (open_args.head.proj_data.projection)
            {
              QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("Sorry, pfmBag only handles geographic PFM structures."));
              close_pfm_file (pfm_handle);
            }


          pfm_file_edit->setText (pfm_file_name);


          //  If one hasn't been set, set the output BAG filename.

          if (output_file_edit->text ().isEmpty ())
            { 
              QString output_file_name = pfm_file_name + ".bag";
              output_file_edit->setText (output_file_name);
            }

          close_pfm_file (pfm_handle);
        }

      options->input_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotOutputFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmBag Output File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->output_dir);


  QStringList filters;
  filters << tr ("BAG file (*.bag)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectFilter (tr ("BAG file (*.bag)"));

  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString output_file_name = files.at (0);


      if (!output_file_name.isEmpty())
        {
          //  Add .bag to filename if not there.
            
          if (!output_file_name.endsWith (".bag")) output_file_name.append (".bag");

          output_file_edit->setText (output_file_name);
        }

      options->output_dir = fd->directory ().absolutePath ();
    }
}


void startPage::slotAreaFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmBag Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->area_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Area file (*.ARE *.are)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString area_file_name = files.at (0);


      if (!area_file_name.isEmpty())
        {
          area_file_edit->setText (area_file_name);
        }

      options->area_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotSepFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmBag Separation Surface File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->sep_dir);


  QStringList filters;
  filters << tr ("CHRTR2 file (*.ch2)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("CHRTR2 file (*.ch2)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString sep_file_name = files.at (0);


      if (!sep_file_name.isEmpty())
        {
          CHRTR2_HEADER chrtr2_header;
          NV_CHAR in_file[512];
          NV_INT32 chrtr2_handle;

          strcpy (in_file, sep_file_name.toAscii ());

          if ((chrtr2_handle = chrtr2_open_file (in_file, &chrtr2_header, CHRTR2_READONLY)) < 0)
            {
              QMessageBox::warning (this, tr ("pfmBag error"), tr ("The file ") + QDir::toNativeSeparators (QString (in_file)) + 
                                    tr (" is not a CHRTR2 file or there was an error reading the file.") + tr ("  The error message returned was:\n\n") +
                                    QString (chrtr2_strerror ()));
            }
          else
            {
              chrtr2_close_file (chrtr2_handle);

              if (chrtr2_header.z1_scale == 0.0)
                {
                  QMessageBox::warning (this, tr ("pfmBag"), tr ("The file ") + QDir::toNativeSeparators (QString (in_file)) + 
                                        tr (" does not contain a Z1 layer.  Assuming Z1 and SEP are identical (MSL to ellipsoid)."));
                  options->has_z1 = NVFalse;
                }
              else
                {
                  options->has_z1 = NVTrue;
                }

              if (chrtr2_header.z0_scale == 0.0)
                {
                  options->has_z0 = NVFalse;
                }
              else
                {
                  options->has_z0 = NVTrue;
                }

              sep_file_edit->setText (sep_file_name);
            }
        }

      options->sep_dir = fd->directory ().absolutePath ();
    }
}
