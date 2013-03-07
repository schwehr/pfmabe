#include "runPage.hpp"
#include "runPageHelp.hpp"

runPage::runPage (QWidget *parent, OPTIONS *op, RUN_PROGRESS *prog, QListWidget **cList):
  QWizardPage (parent)
{
  options = op;
  progress = prog;


  setTitle (tr ("Mosaic Page"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmChartsImageWatermark.png"));

  setFinalPage (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QHBoxLayout *out_file_box = new QHBoxLayout (0);
  out_file_box->setSpacing (8);

  QLabel *out_file_label = new QLabel (tr ("Output TIFF File"), this);
  out_file_box->addWidget (out_file_label, 1);

  out_file_edit = new QLineEdit (this);
  out_file_box->addWidget (out_file_edit, 10);

  QPushButton *out_file_browse = new QPushButton (tr ("Browse..."), this);
  out_file_box->addWidget (out_file_browse, 1);

  out_file_label->setWhatsThis (out_fileText);
  out_file_edit->setWhatsThis (out_fileText);
  out_file_browse->setWhatsThis (out_fileBrowseText);

  connect (out_file_browse, SIGNAL (clicked ()), this, SLOT (slotOutFileBrowse ()));


  vbox->addLayout (out_file_box);


  outGrp = new QButtonGroup (this);
  outGrp->setExclusive (TRUE);
  connect (outGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotOut (int)));


  QGroupBox *outBox = new QGroupBox (tr ("Output options"), this);
  QHBoxLayout *outBoxLayout = new QHBoxLayout;
  outBox->setLayout (outBoxLayout);


  utm_check = new QCheckBox (tr ("UTM output file"), this);
  utm_check->setToolTip (tr ("Output GeoTIFF file in UTM projection"));
  utm_check->setWhatsThis (utmText);
  outGrp->addButton (utm_check, 0);
  outBoxLayout->addWidget (utm_check);


  geo_check = new QCheckBox (tr ("Geographic output file"), this);
  geo_check->setToolTip (tr ("Output GeoTIFF file in unprojected lat/lon form"));
  geo_check->setWhatsThis (geoText);
  outGrp->addButton (geo_check, 1);
  outBoxLayout->addWidget (geo_check);

  if (options->utm)
    {
      utm_check->setChecked (TRUE);
    }
  else
    {
      geo_check->setChecked (TRUE);
    }


  caris_check = new QCheckBox (tr ("Caris format"), this);
  caris_check->setToolTip (tr ("Output in brain-dead Caris acceptable GeoTIFF format"));
  caris_check->setWhatsThis (carisText);
  caris_check->setChecked (options->caris);
  outBoxLayout->addWidget (caris_check);

  connect (caris_check, SIGNAL (clicked ()), this, SLOT (slotCarisClicked (void)));


  vbox->addWidget (outBox);


  QGroupBox *lbox = new QGroupBox (tr ("Mosaic status"), this);
  QVBoxLayout *lboxLayout = new QVBoxLayout;
  lbox->setLayout (lboxLayout);
  lboxLayout->setSpacing (10);


  progress->mbox = new QGroupBox (tr ("Generating mosaic"), this);
  QVBoxLayout *mboxLayout = new QVBoxLayout;
  progress->mbox->setLayout (mboxLayout);
  mboxLayout->setSpacing (10);


  progress->mbar = new QProgressBar (this);
  progress->mbar->setRange (0, 100);
  progress->mbar->setWhatsThis (tr ("Progress of the mosaic process."));
  mboxLayout->addWidget (progress->mbar);


  vbox->addWidget (progress->mbox);


  checkList = new QListWidget (this);
  checkList->setAlternatingRowColors (TRUE);
  lboxLayout->addWidget (checkList);


  vbox->addWidget (lbox);


  *cList = checkList;


  registerField ("out_file_edit*", out_file_edit);


  //  Serious cheating here ;-)  I want the finish button to be disabled when you first get to this page
  //  so I set the last progress bar to be a "mandatory" field.  I couldn't disable the damn button in
  //  initializePage in the parent for some unknown reason.

  registerField ("progress_mbar*", progress->mbar, "value");
}



void runPage::setOutFileName (QString name)
{
  out_file_edit->setText (name);
}



void runPage::slotOutFileBrowse ()
{
  QStringList         files, filters;
  QString             file;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmChartsImage Select Output File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->output_dir);


  filters << tr ("TIFF (*.tif)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("TIFF (*.tif)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString out_file_name = files.at (0);


      if (!out_file_name.isEmpty())
        {
          out_file_edit->setText (out_file_name);
        }

      options->output_dir = fd->directory ().absolutePath ();
    }
}



void runPage::slotOut (int id)
{
  if (id)
    {
      options->utm = NVFalse;
    }
  else
    {
      options->utm = NVTrue;
    }
}



void runPage::slotCarisClicked ()
{
  if (caris_check->isChecked ())
    {
      options->caris = NVTrue;
    }
  else
    {
      options->caris = NVFalse;
    }
}
