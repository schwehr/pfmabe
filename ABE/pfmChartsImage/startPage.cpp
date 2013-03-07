#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, PFM_OPEN_ARGS *oa, OPTIONS *op):
  QWizardPage (parent)
{
  open_args = oa;
  options = op;


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmChartsImageWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("pfmChartsImage is a tool for extracting downlooking images from .img files "
                                  "associated with CHARTS LIDAR .hof or .tof files.  It uses the associated "
                                  ".pos or SBET files to get the aircraft attitude for each image.  A Digital "
                                  "Elevation Model (DEM) is created at a user defined grid spacing for the "
                                  "specified area using the Minimum Curvature Spline Interpolation (MISP) "
                                  "library.  An ASCII position and attitude file is also generated.  Camera "
                                  "biases and other offsets must be supplied in order to correctly position "
                                  "the photos.  This program creates a directory in the current working "
                                  "directory of the form <b>PFM_FILENAME__AREA_FILENAME_image_files</b>.  "
                                  "The images, the DEM file, and the attitude file will be stored in that "
                                  "directory.  Once the directory and files are created Ray Seyfarth's mosaic "
                                  "program is run to generate a GeoTIFF mosaic file from the images."));

  /*
                                  "<b>Due to problems with the <i>mosaic</i> program this application may abort "
                                  "after generating the DEM and image directory.  It is sometimes possible "
                                  "to manually run <i>mosaic</i> from within <i>valgrind</i> if this happens. "
                                  "If your output file was to be called fred.tif you would issue the command "
                                  "<i>valgrind mosaic fred.pars</i>.  If that is successful you can then run "
                                  "the command <i>gdalwarp -t_srs 'EPSG:4326' -co 'TILED=NO' -co 'COMPRESS=LZW' "
                                  "fred_utm.tif fred.tif</i> to create a geographic GeoTIFF from the UTM GeoTIFF.</b>"));
  */

  label->setWordWrap (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);


  QHBoxLayout *pfm_file_box = new QHBoxLayout (0);
  pfm_file_box->setSpacing (8);

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


  vbox->addLayout (pfm_file_box);


  QHBoxLayout *area_box = new QHBoxLayout (0);
  area_box->setSpacing (8);

  QLabel *area_file_label = new QLabel (tr ("Area File"), this);
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


  vbox->addLayout (area_box);


  QHBoxLayout *image_box = new QHBoxLayout (0);
  image_box->setSpacing (8);

  QLabel *image_dir_label = new QLabel (tr ("Image directory"), this);
  image_box->addWidget (image_dir_label, 1);

  image_dir_edit = new QLineEdit (this);
  image_box->addWidget (image_dir_edit, 10);

  image_dir_label->setWhatsThis (image_dirText);
  image_dir_edit->setWhatsThis (image_dirText);


  remove_check = new QCheckBox (tr ("Remove image directory"), this);
  remove_check->setToolTip (tr ("Remove the image file directory after creating the mosaic GeoTIFFs"));
  remove_check->setWhatsThis (removeText);
  remove_check->setChecked (options->remove);
  connect (remove_check, SIGNAL (clicked ()), this, SLOT (slotRemoveClicked (void)));
  image_box->addWidget (remove_check);


  vbox->addLayout (image_box);


  QGroupBox *levelBox = new QGroupBox (tr ("Optional single level area"), this);
  QHBoxLayout *levelBoxLayout = new QHBoxLayout;
  levelBox->setLayout (levelBoxLayout);

  QLabel *level_file_label = new QLabel (tr ("Area file"), this);
  levelBoxLayout->addWidget (level_file_label, 1);

  level_file_edit = new QLineEdit (this);
  level_file_edit->setReadOnly (TRUE);
  levelBoxLayout->addWidget (level_file_edit, 10);

  QPushButton *level_file_browse = new QPushButton (tr ("Browse..."), this);
  levelBoxLayout->addWidget (level_file_browse, 1);


  level_file_label->setWhatsThis (level_fileText);
  level_file_edit->setWhatsThis (level_fileText);
  level_file_browse->setWhatsThis (level_fileBrowseText);

  connect (level_file_browse, SIGNAL (clicked ()), this, SLOT (slotLevelFileBrowse ()));


  levelSpin = new QDoubleSpinBox (this);
  levelSpin->setDecimals (2);
  levelSpin->setRange (-1000.0, 1000.0);
  levelSpin->setSingleStep (10.0);
  levelSpin->setValue (options->level);
  levelSpin->setWrapping (TRUE);
  levelSpin->setToolTip (tr ("Set the level (m) for the area file"));
  levelSpin->setWhatsThis (levelText);
  levelBoxLayout->addWidget (levelSpin);


  vbox->addWidget (levelBox);


  //  Handle file name on command line so we can drag and drop to the Desktop icon.

  if (strlen (open_args->list_path) > 3)
    {
      QString pfm_file_name = QString (open_args->list_path);


      strcpy (open_args->list_path, pfm_file_name.toAscii ());
      open_args->checkpoint = 0;

      options->pfm_handle = open_existing_pfm_file (open_args);

      if (options->pfm_handle < 0)
        {
          if (QMessageBox::warning (this, tr ("Open PFM Structure"),
                                    tr ("The file ") + QDir::convertSeparators (QString (open_args->list_path)) + 
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

      if (open_args->head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("Sorry, pfmChartsBoresight only handles geographic PFM structures."));
          close_pfm_file (options->pfm_handle);
        }
      else
        {
          pfm_file_edit->setText (pfm_file_name);


          if (!area_file_edit->text ().isEmpty ())
            {
              QFileInfo *fi = new QFileInfo (area_file_edit->text ());

              QString dirString = pfm_file_name + "__" + fi->baseName () + "__bse_image_files";

              image_dir_edit->setText (dirString);
            }
        }
      registerField ("pfm_file_edit", pfm_file_edit);
    }
  else
    {
      registerField ("pfm_file_edit*", pfm_file_edit);
    }


  registerField ("area_file_edit*", area_file_edit);
  registerField ("image_dir_edit*", image_dir_edit);
  registerField ("level_file_edit", level_file_edit);
  registerField ("levelSpin", levelSpin);
}



void startPage::slotPFMFileBrowse ()
{
  QStringList         files, filters;
  QString             file;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmChartsImage Open PFM Structure"));
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
          strcpy (open_args->list_path, pfm_file_name.toAscii ());
          open_args->checkpoint = 0;

          options->pfm_handle = open_existing_pfm_file (open_args);

          if (options->pfm_handle < 0)
            {
              if (QMessageBox::warning (this, tr ("Open PFM Structure"),
                                        tr ("The file ") + QDir::toNativeSeparators (QString (open_args->list_path)) + 
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

      if (open_args->head.proj_data.projection)
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("Sorry, pfmChartsImage only handles geographic PFM structures."));
          close_pfm_file (options->pfm_handle);
        }
      else
        {
          pfm_file_edit->setText (pfm_file_name);


          if (!area_file_edit->text ().isEmpty ())
            {
              QFileInfo *fi = new QFileInfo (area_file_edit->text ());

              QString dirString = pfm_file_name + "__" + fi->baseName () + "__image_files";

              image_dir_edit->setText (dirString);
            }
        }

      options->input_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotAreaFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmChartsImage Area File"));
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


          if (!pfm_file_edit->text ().isEmpty ())
            {
              QFileInfo *fi = new QFileInfo (area_file_name);

              QString dirString = pfm_file_edit->text () + "__" + fi->baseName () + "__image_files";

              image_dir_edit->setText (dirString);
            }
        }

      options->area_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotLevelFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmChartsImage Level Area File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->level_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Area file (*.ARE *.are *.afs)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString level_file_name = files.at (0);


      if (!level_file_name.isEmpty())
        {
          level_file_edit->setText (level_file_name);
        }

      options->level_dir = fd->directory ().absolutePath ();
    }
}



void startPage::slotRemoveClicked ()
{
  if (remove_check->isChecked ())
    {
      options->remove = NVTrue;
    }
  else
    {
      options->remove = NVFalse;
    }
}
