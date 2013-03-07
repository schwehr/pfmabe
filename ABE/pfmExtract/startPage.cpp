#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmExtractWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The pfmExtract program is a tool for reading a PFM file and writing out a series of "
                                  "files (in ASCII, LLZ, or RDP) for input to CARIS or other software packages.  Help is "
                                  "available by clicking on the Help button and then clicking on the item for which you "
                                  "want help.  Click <b>Next</b> to continue or <b>Cancel</b> to exit."));


  label->setWordWrap(true);


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


  QLabel *output_file_label = new QLabel (tr ("Output file base name"), this);
  output_box->addWidget (output_file_label, 1);

  output_file_edit = new QLineEdit (this);
  output_box->addWidget (output_file_edit, 10);

  output_file_label->setWhatsThis (output_fileText);
  output_file_edit->setWhatsThis (output_fileText);


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


  if (*argc == 2)
    {
      PFM_OPEN_ARGS open_args;
      NV_INT32 pfm_handle;

      QString pfm_file_name = QString (argv[1]);

      strcpy (open_args.list_path, pfm_file_name.toAscii ());

      open_args.checkpoint = 0;
      pfm_handle = open_existing_pfm_file (&open_args);

      if (pfm_handle >= 0)
        {
          if (!open_args.head.proj_data.projection)
            {
              pfm_file_edit->setText (pfm_file_name);

              output_file_edit->setText (pfm_file_name);

              close_pfm_file (pfm_handle);
            }
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
}



void startPage::slotPFMFileBrowse ()
{
  PFM_OPEN_ARGS       open_args;
  QStringList         files, filters;
  QString             file;
  NV_INT32            pfm_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmExtract Open PFM Structure"));
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
              if (QMessageBox::warning (this, tr ("Open PFM Structure"),
                                        tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                        tr (" is not a PFM structure or there was an error reading the file.") +
                                        tr ("  The error message returned was:\n\n") +
                                        QString (pfm_error_str (pfm_error))));

              if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
                {
                  fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
                  exit (-1);
                }

              return;
            }
        }

      if (open_args.head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("Sorry, pfmExtract only handles geographic PFM structures."));
          close_pfm_file (pfm_handle);
        }
      else
        {
          pfm_file_edit->setText (pfm_file_name);

          output_file_edit->setText (pfm_file_name);
        }

      options->input_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotAreaFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmExtract Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->area_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Area file (*.ARE *.are *.afs)"));


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
