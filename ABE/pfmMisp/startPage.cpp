
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



#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (NV_INT32 *argc, NV_CHAR **argv, QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmMispWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("pfmMisp is a tool for replacing the <b>Average Filtered/Edited Surface</b> in "
                                  "a PFM structure with a Minimum Curvature Spline Interpolated (MISP) "
                                  "surface generated from the minimum, maximum, or all depths in the PFM bins."));

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
}



void startPage::slotPFMFileBrowse ()
{
  PFM_OPEN_ARGS       open_args;
  QStringList         files, filters;
  QString             file;
  NV_INT32            pfm_handle = -1;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmMisp Open PFM Structure"));
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
        }

      if (open_args.head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"), tr ("Sorry, pfmMisp only handles geographic PFM structures."));
          close_pfm_file (pfm_handle);
        }


      pfm_file_edit->setText (pfm_file_name);

      options->input_dir = fd->directory ().absolutePath ();
    }
}
