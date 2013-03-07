#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmFeatureWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (ridiculousText);

  QLabel *label = new QLabel (tr ("pfmFeature is a tool for selecting features in a PFM structure.  "
                                  "The selected features will be written to a new or pre-existing "
                                  "feature file.  The feature file name will be placed in the input PFM.  "
                                  "Feature selection criteria may be either IHO special order or order 1.  "
                                  "Help is available by clicking on the Help button and then clicking "
                                  "on the item for which you want help.  For a ridiculously involved "
                                  "explanation of the selection algorithm click on the help button below "
                                  "and then click on this text.<br><br>"
                                  "<b>IMPORTANT NOTE: This program assumes that the PFM has been very well "
                                  "cleaned.  That is, there should be no outliers in either the up or down "
                                  "direction.</b><br><br>"
                                  "Select a PFM file below.  You may then change the default output file "
                                  "name and, optionally, select an area file to limit the feature selection "
                                  "area and/or an area file to exclude from feature selection.  Click "
                                  "<b>Next</b> to continue or <b>Cancel</b> to exit."));
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


  QLabel *output_file_label = new QLabel (tr ("Output Feature File"), this);
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


  QHBoxLayout *ex_box = new QHBoxLayout (0);
  ex_box->setSpacing (8);

  vbox->addLayout (ex_box);


  QLabel *ex_file_label = new QLabel (tr ("Optional Exclusion Area File"), this);
  ex_box->addWidget (ex_file_label, 1);

  ex_file_edit = new QLineEdit (this);
  ex_file_edit->setReadOnly (TRUE);
  ex_box->addWidget (ex_file_edit, 10);

  QPushButton *ex_file_browse = new QPushButton (tr ("Browse..."), this);
  ex_box->addWidget (ex_file_browse, 1);


  ex_file_label->setWhatsThis (ex_fileText);
  ex_file_edit->setWhatsThis (ex_fileText);
  ex_file_browse->setWhatsThis (ex_fileBrowseText);

  connect (ex_file_browse, SIGNAL (clicked ()), this, SLOT (slotExFileBrowse ()));


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
          if (!open_args.head.proj_data.projection)
            {
              pfm_file_edit->setText (pfm_file_name);


              //  Set the output feature filename.

              QString output_file_name = pfm_file_name;
              output_file_name.replace (".pfm", ".bfd");


              //  If this file already exists we need to see if we are going to replace it or append to it.

              FILE *fp;
              NV_CHAR name[512];
              strcpy (name, output_file_name.toAscii ());

              if ((fp = fopen (name, "r")) != NULL)
                {
                  QMessageBox msgBox (this);
                  msgBox.setIcon (QMessageBox::Question);
                  msgBox.setText (tr ("The output file ") + output_file_name + tr (" already exists."));
                  msgBox.setInformativeText (tr ("Do you wish to replace it or append to it?"));
                  msgBox.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
                  msgBox.setDefaultButton (QMessageBox::Yes);
                  msgBox.setButtonText (QMessageBox::Yes, tr ("Replace"));
                  msgBox.setButtonText (QMessageBox::No, tr ("Append"));
                  NV_INT32 ret = msgBox.exec ();

                  if (ret == QMessageBox::Yes)
                    {
                      options->replace = NVTrue;
                    }
                  else
                    {
                      options->replace = NVFalse;
                    }

                  fclose (fp);
                }

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
  registerField ("ex_file_edit", ex_file_edit);
}



void startPage::slotPFMFileBrowse ()
{
  PFM_OPEN_ARGS       open_args;
  QStringList         files, filters;
  QString             file;
  NV_INT32            pfm_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmFeature Open PFM Structure"));
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
                                tr ("Sorry, pfmFeature only handles geographic PFM structures."));
          close_pfm_file (pfm_handle);
        }


      pfm_file_edit->setText (pfm_file_name);


      //  If one hasn't been set, set the output feature filename.

      if (output_file_edit->text ().isEmpty ())
        {
          QString output_file_name = pfm_file_name + ".bfd";


          //  If this file already exists we need to see if we are going to replace it, append to it, or make a new file.

          FILE *fp;
          NV_CHAR name[512];
          strcpy (name, output_file_name.toAscii ());

          if ((fp = fopen (name, "r")) != NULL)
            {
              QMessageBox msgBox (this);
              msgBox.setIcon (QMessageBox::Question);
              msgBox.setText (tr ("The output file ") + output_file_name + tr (" already exists."));
              msgBox.setInformativeText (tr ("Do you wish to replace it, append to it, or create a new file?"));
              msgBox.setStandardButtons (QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort);
              msgBox.setDefaultButton (QMessageBox::Yes);
              msgBox.setButtonText (QMessageBox::Yes, tr ("Replace"));
              msgBox.setButtonText (QMessageBox::No, tr ("Append"));
              msgBox.setButtonText (QMessageBox::Abort, tr ("New file"));
              NV_INT32 ret = msgBox.exec ();

              switch (ret)
                {
                default:
                case QMessageBox::Yes:
                  options->replace = NVTrue;
                  output_file_edit->setText (output_file_name);
                  break;

                case QMessageBox::No:
                  options->replace = NVFalse;
                  output_file_edit->setText (output_file_name);
                  break;

                case QMessageBox::Abort:
                  options->replace = NVTrue;
                  break;
                }

              fclose (fp);
            }
          else
            {
              output_file_edit->setText (output_file_name);
            }
        }

      options->input_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotOutputFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmFeature Output File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->output_dir);


  QStringList filters;
  filters << tr ("Feature file (*.bfd)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectFilter (tr ("Feature file (*.bfd)"));

  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString output_file_name = files.at (0);


      if (!output_file_name.isEmpty())
        {
          //  Add .bfd to filename if not there.
            
          if (!output_file_name.endsWith (".bfd")) output_file_name.append (".bfd");


          //  If this file already exists we need to see if we are going to replace it or append to it.

          FILE *fp;
          NV_CHAR name[512];
          strcpy (name, output_file_name.toAscii ());

          if ((fp = fopen (name, "r")) != NULL)
            {
              QMessageBox msgBox (this);
              msgBox.setIcon (QMessageBox::Question);
              msgBox.setText (tr ("The output file ") + output_file_name + tr (" already exists."));
              msgBox.setInformativeText (tr ("Do you wish to replace it or append to it?"));
              msgBox.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
              msgBox.setDefaultButton (QMessageBox::Yes);
              msgBox.setButtonText (QMessageBox::Yes, tr ("Replace"));
              msgBox.setButtonText (QMessageBox::No, tr ("Append"));
              NV_INT32 ret = msgBox.exec ();

              if (ret == QMessageBox::Yes)
                {
                  options->replace = NVTrue;
                }
              else
                {
                  options->replace = NVFalse;
                }

              fclose (fp);
            }

          output_file_edit->setText (output_file_name);
        }

      options->output_dir = fd->directory ().absolutePath ();
    }
}


void startPage::slotAreaFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmFeature Area File"));
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



void startPage::slotExFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmFeature Exclusion Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->exclude_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Area file (*.ARE *.are *.afs)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString ex_file_name = files.at (0);


      if (!ex_file_name.isEmpty())
        {
          ex_file_edit->setText (ex_file_name);
        }

      options->exclude_dir = fd->directory ().absolutePath ();
    }
}
