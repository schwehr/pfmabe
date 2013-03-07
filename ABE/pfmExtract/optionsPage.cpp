#include "optionsPage.hpp"
#include "optionsPageHelp.hpp"

optionsPage::optionsPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Extract Options"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmExtractWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);



  QGroupBox *sBox = new QGroupBox (tr ("Data source"), this);
  QVBoxLayout *sBoxLayout = new QVBoxLayout;
  sBox->setLayout (sBoxLayout);
  sBox->setWhatsThis (sourceText);

  QRadioButton *minFilt = new QRadioButton (tr ("Minimum Filtered Surface"));
  QRadioButton *maxFilt = new QRadioButton (tr ("Maximum Filtered Surface"));
  QRadioButton *avgFilt = new QRadioButton (tr ("Average Filtered or MISP Surface"));
  QRadioButton *allData = new QRadioButton (tr ("All data points"));

  QButtonGroup *source = new QButtonGroup (this);
  source->setExclusive (TRUE);
  connect (source, SIGNAL (buttonClicked (int)), this, SLOT (slotSourceClicked (int)));

  source->addButton (minFilt, 0);
  source->addButton (maxFilt, 1);
  source->addButton (avgFilt, 2);
  source->addButton (allData, 3);

  sBoxLayout->addWidget (minFilt);
  sBoxLayout->addWidget (maxFilt);
  sBoxLayout->addWidget (avgFilt);
  sBoxLayout->addWidget (allData);
  
  minFilt->setWhatsThis (min_surfaceText);
  maxFilt->setWhatsThis (max_surfaceText);
  avgFilt->setWhatsThis (avg_surfaceText);
  allData->setWhatsThis (all_surfaceText);

  source->button (options->source)->setChecked (TRUE);


  vbox->addWidget (sBox);


  QGroupBox *tBox = new QGroupBox (tr ("Options"), this);
  QVBoxLayout *tBoxLayout = new QVBoxLayout;
  QHBoxLayout *tBoxTopLayout = new QHBoxLayout;
  QHBoxLayout *tBoxBotLayout = new QHBoxLayout;
  tBoxLayout->addLayout (tBoxTopLayout);
  tBoxLayout->addLayout (tBoxBotLayout);
  tBox->setLayout (tBoxLayout);
  tBoxLayout->setSpacing (10);

  QGroupBox *oBox = new QGroupBox (tr ("Output format"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  oBoxLayout->setSpacing (10);

  format = new QComboBox (this);
  format->setToolTip (tr ("Set the output file format"));
  format->setWhatsThis (formatText);
  format->setEditable (FALSE);
  format->addItem (tr ("ASCII"));
  format->addItem (tr ("LLZ"));
  format->addItem (tr ("RDP"));
  format->addItem (tr ("SHP"));
  format->setCurrentIndex (options->format);
  connect (format, SIGNAL (currentIndexChanged (int)), this, SLOT (slotFormatChanged (int)));
  oBoxLayout->addWidget (format);
  tBoxTopLayout->addWidget (oBox);


  QGroupBox *fbox = new QGroupBox (tr ("File size"), this);
  QHBoxLayout *fboxLayout = new QHBoxLayout;
  fbox->setLayout (fboxLayout);
  fboxLayout->setSpacing (10);

  size = new QComboBox (this);
  size->setToolTip (tr ("Set the output file size limit"));
  size->setWhatsThis (sizeText);
  size->setEditable (FALSE);
  size->addItem (tr ("No limit"));
  size->addItem (tr ("1GB"));
  size->addItem (tr ("2GB"));
  size->addItem (tr ("4GB"));
  size->setCurrentIndex (options->size);
  fboxLayout->addWidget (size);
  tBoxTopLayout->addWidget (fbox);



  QGroupBox *cutoffBox = new QGroupBox (tr ("Depth cutoff"), this);
  QHBoxLayout *cutoffBoxLayout = new QHBoxLayout;
  cutoffBox->setLayout (cutoffBoxLayout);

  cut = new QCheckBox (tr ("Apply cutoff"), this);
  cut->setToolTip (tr ("Set this to apply the depth cutoff"));
  cut->setWhatsThis (cutText);
  cut->setChecked (options->cut);
  connect (cut, SIGNAL (stateChanged (int)), this, SLOT (slotCutChanged (int)));
  cutoffBoxLayout->addWidget (cut);

  cutoff = new QDoubleSpinBox (cutoffBox);
  cutoff->setDecimals (2);
  cutoff->setRange (-2000.0, 2000.0);
  cutoff->setSingleStep (10.0);
  cutoff->setValue (options->cutoff);
  cutoff->setWrapping (FALSE);
  cutoff->setToolTip (tr ("Set a depth cutoff value"));
  cutoff->setWhatsThis (cutoffText);
  if (!options->cut) cutoff->setEnabled (FALSE);
  cutoffBoxLayout->addWidget (cutoff);
  tBoxTopLayout->addWidget (cutoffBox);


  QGroupBox *shiftBox = new QGroupBox (tr ("Datum shift"), this);
  QHBoxLayout *shiftBoxLayout = new QHBoxLayout;
  shiftBox->setLayout (shiftBoxLayout);
  datumShift = new QDoubleSpinBox (shiftBox);
  datumShift->setDecimals (2);
  datumShift->setRange (-2000.0, 2000.0);
  datumShift->setSingleStep (10.0);
  datumShift->setValue (options->datum_shift);
  datumShift->setWrapping (FALSE);
  datumShift->setToolTip (tr ("Set a value with which to shift the output data"));
  datumShift->setWhatsThis (datumShiftText);
  shiftBoxLayout->addWidget (datumShift);
  tBoxTopLayout->addWidget (shiftBox);


  QGroupBox *orthoBox = new QGroupBox (tr ("Ortho correction"), this);
  QHBoxLayout *orthoBoxLayout = new QHBoxLayout;
  orthoBox->setLayout (orthoBoxLayout);
  geoid = new QCheckBox (orthoBox);
  geoid->setToolTip (tr ("If checked, correct from ellipsoidal to orthometric height"));
  geoid->setWhatsThis (geoidText);
  geoid->setChecked (options->geoid03);
  orthoBoxLayout->addWidget (geoid);
  tBoxTopLayout->addWidget (orthoBox);


  QGroupBox *refBox = new QGroupBox (tr ("Reference data"), this);
  QHBoxLayout *refBoxLayout = new QHBoxLayout;
  refBox->setLayout (refBoxLayout);
  reference = new QCheckBox (this);
  reference->setToolTip (tr ("Include reference data in the output files"));
  reference->setWhatsThis (referenceText);
  reference->setChecked (options->ref);
  refBoxLayout->addWidget (reference);
  tBoxBotLayout->addWidget (refBox);


  QGroupBox *chkBox = new QGroupBox (tr ("Checked/Verified data"), this);
  QHBoxLayout *chkBoxLayout = new QHBoxLayout;
  chkBox->setLayout (chkBoxLayout);
  checked = new QCheckBox (this);
  checked->setToolTip (tr ("Only include data from checked/verified bins"));
  checked->setWhatsThis (checkedText);
  checked->setChecked (options->chk);
  chkBoxLayout->addWidget (checked);
  tBoxBotLayout->addWidget (chkBox);


  QGroupBox *flpBox = new QGroupBox (tr ("Invert"), this);
  QHBoxLayout *flpBoxLayout = new QHBoxLayout;
  flpBox->setLayout (flpBoxLayout);
  flip = new QCheckBox (this);
  flip->setToolTip (tr ("Invert the sign on Z values"));
  flip->setWhatsThis (flipText);
  flip->setChecked (options->flp);
  flpBoxLayout->addWidget (flip);
  tBoxBotLayout->addWidget (flpBox);


  utmBox = new QGroupBox (tr ("UTM"), this);
  QHBoxLayout *utmBoxLayout = new QHBoxLayout;
  utmBox->setLayout (utmBoxLayout);
  utm = new QCheckBox (this);
  utm->setToolTip (tr ("Convert position data to UTM"));
  utm->setWhatsThis (utmText);
  utm->setChecked (options->utm);
  utmBoxLayout->addWidget (utm);
  tBoxBotLayout->addWidget (utmBox);
  if (options->format)
    {
      utm->setEnabled (FALSE);
      utmBox->setEnabled (FALSE);
    }


  landBox = new QGroupBox (tr ("Exclude land"), this);
  QHBoxLayout *landBoxLayout = new QHBoxLayout;
  landBox->setLayout (landBoxLayout);
  land = new QCheckBox (this);
  land->setToolTip (tr ("Exclude land data from the output"));
  land->setWhatsThis (landText);
  land->setChecked (options->lnd);
  landBoxLayout->addWidget (land);
  tBoxBotLayout->addWidget (landBox);


  uncBox = new QGroupBox (tr ("Include uncertainty data"), this);
  QHBoxLayout *uncBoxLayout = new QHBoxLayout;
  uncBox->setLayout (uncBoxLayout);
  uncert = new QCheckBox (this);
  uncert->setToolTip (tr ("Include uncertainty data in output"));
  uncert->setWhatsThis (uncertText);
  uncert->setChecked (options->unc);
  uncBoxLayout->addWidget (uncert);
  tBoxBotLayout->addWidget (uncBox);
  if (options->format)
    {
      uncert->setEnabled (FALSE);
      uncBox->setEnabled (FALSE);
    }


  vbox->addWidget (tBox);


  registerField ("format", format);
  registerField ("reference", reference);
  registerField ("checked", checked);
  registerField ("flip", flip);
  registerField ("utm", utm);
  registerField ("land", land);
  registerField ("uncert", uncert);
  registerField ("cut", cut);
  registerField ("cutoff", cutoff, "value", "valueChanged");
  registerField ("datumShift", datumShift, "value", "valueChanged");
  registerField ("geoid", geoid);
  registerField ("size", size);
}



void 
optionsPage::slotSourceClicked (int id)
{
  options->source = id;
}



void 
optionsPage::slotCutChanged (int state)
{
  if (state)
    {
      cutoff->setEnabled (TRUE);
    }
  else
    {
      cutoff->setEnabled (FALSE);
    }
}



void 
optionsPage::slotFormatChanged (int id)
{
  if (id)
    {
      options->utm = NVFalse;
      utm->setChecked (FALSE);
      utm->setEnabled (FALSE);
      utmBox->setEnabled (FALSE);

      options->unc = NVFalse;
      uncert->setChecked (FALSE);
      uncert->setEnabled (FALSE);
      uncBox->setEnabled (FALSE);
    }
  else
    {
      utm->setEnabled (TRUE);
      utmBox->setEnabled (TRUE);

      uncert->setEnabled (TRUE);
      uncBox->setEnabled (TRUE);
    }
}



void
optionsPage::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}
