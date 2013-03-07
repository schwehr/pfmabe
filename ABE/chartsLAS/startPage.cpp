#include "startPage.hpp"
#include "startPageHelp.hpp"

startPage::startPage (QWidget *parent, NV_BOOL g03, NV_INT32 dtm, QString fpre):
  QWizardPage (parent)
{
  geoid03 = g03;
  datum = dtm;
  filePrefix = fpre;
  outputDir = "LAS_AREA_DIRECTORIES";


  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/chartsLASWatermark.png"));


  setTitle (tr ("Introduction"));

  setWhatsThis (tr ("See, it really works!"));

  QLabel *label = new QLabel (tr ("The chartsLAS program is used to subset CHARTS HOF and TOF files based on one or more "
                                  "input area files.  TOF files will be output in both ASCII text and LAS format.  HOF "
                                  "files will only be output in ASCII text file format.  One directory will be created "
                                  "for each input area file.  The output data will be stored in individual files in these "
                                  "directories.  Click the Next button to go to the area file input page.  Context "
                                  "sensitive help is available by clicking on the Help button and then clicking, with the "
                                  "Question Arrow cursor, on the field of interest."));

  label->setWordWrap(true);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->addWidget (label);
  vbox->addStretch (10);



  QGroupBox *fileBox = new QGroupBox (tr ("Output file"), this);
  QVBoxLayout *fileBoxLayout = new QVBoxLayout;
  fileBox->setLayout (fileBoxLayout);
  fileBoxLayout->setSpacing (10);

  QGroupBox *dBox = new QGroupBox (tr ("Directory"), this);
  QHBoxLayout *dBoxLayout = new QHBoxLayout;
  dBox->setLayout (dBoxLayout);
  dBoxLayout->setSpacing (10);

  out_edit = new QLineEdit (this);
  out_edit->setWhatsThis (outText);
  out_edit->setText (outputDir);
  dBoxLayout->addWidget (out_edit, 10);

  outBrowse = new QPushButton (tr ("Browse..."), this);
  outBrowse->setWhatsThis (outBrowseText);
  connect (outBrowse, SIGNAL (clicked ()), this, SLOT (slotOutBrowseClicked ()));
  dBoxLayout->addWidget (outBrowse, 1);


  fileBoxLayout->addWidget (dBox);


  QGroupBox *pBox = new QGroupBox (tr ("File prefix"), this);
  QHBoxLayout *pBoxLayout = new QHBoxLayout;
  pBox->setLayout (pBoxLayout);
  pBoxLayout->setSpacing (10);

  pre_edit = new QLineEdit (this);
  pre_edit->setText (filePrefix);
  pre_edit->setWhatsThis (preText);
  pBoxLayout->addWidget (pre_edit, 10);


  fileBoxLayout->addWidget (pBox);


  vbox->addWidget (fileBox);


  QGroupBox *oBox = new QGroupBox (tr ("Datum"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  oBoxLayout->setSpacing (10);


  QGroupBox *gBox = new QGroupBox (tr ("Orthometric height"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  gBoxLayout->setSpacing (10);


  oBoxLayout->addWidget (gBox);


  geoid = new QCheckBox (this);
  geoid->setToolTip (tr ("If checked, correct from ellipsoidal to orthometric height"));
  geoid->setWhatsThis (geoidText);
  geoid->setChecked (geoid03);
  gBoxLayout->addWidget (geoid);


  QGroupBox *hBox = new QGroupBox (tr ("Horizontal datum"), this);
  QVBoxLayout *hBoxLayout = new QVBoxLayout;
  hBox->setLayout (hBoxLayout);
  hBoxLayout->setSpacing (10);

  hDatum = new QComboBox (this);
  hDatum->setToolTip (tr ("Select the horizontal datum"));
  hDatum->setWhatsThis (hDatumText);
  hDatum->setEditable (FALSE);
  hDatum->addItem ("WGS 84");
  hDatum->addItem ("NAD 83");
  hDatum->setCurrentIndex (datum);
  hBoxLayout->addWidget (hDatum);


  oBoxLayout->addWidget (hBox);


  vbox->addWidget (oBox);


  registerField ("out_edit", out_edit);
  registerField ("pre_edit", pre_edit);
  registerField ("geoid", geoid);
  registerField ("hDatum", hDatum);
}



void 
startPage::slotOutBrowseClicked ()
{
  QStringList         files, filters;
  static QDir         dir = QDir (".");


  QFileDialog *fd = new QFileDialog (this, tr ("chartsLAS Output directory"));
  fd->setViewMode (QFileDialog::List);
  fd->setDirectory (dir);

  fd->setFileMode (QFileDialog::DirectoryOnly);


  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString out_dir = files.at (0);

      if (!out_dir.isEmpty())
        { 
          outputDir = out_dir;
          out_edit->setText (outputDir);
        }
    }
}
