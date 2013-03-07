#include "datumPage.hpp"
#include "datumPageHelp.hpp"

datumPage::datumPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Datum/Projection parameters"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));


  units = new QComboBox (this);
  units->setWhatsThis (unitsText);
  units->setEditable (FALSE);
  units->addItem (tr ("Meters"));
  units->addItem (tr ("Feet"));
  units->addItem (tr ("Fathoms"));
  units->setCurrentIndex (options->units);


  depthCor = new QComboBox (this);
  depthCor->setWhatsThis (depthCorText);
  depthCor->setEditable (FALSE);
  depthCor->addItem (tr ("Corrected depth"));
  depthCor->addItem (tr ("Uncorrected depth @ 1500 m/s"));
  depthCor->addItem (tr ("Uncorrected depth @ 4800 ft/s"));
  depthCor->addItem (tr ("Uncorrected depth @ 800 fm/s"));
  depthCor->addItem (tr ("Mixed corrections"));
  depthCor->setCurrentIndex (options->depth_cor);


  projection = new QComboBox (this);
  projection->setWhatsThis (projectionText);
  projection->setEditable (FALSE);
  projection->addItem (tr ("Geodetic"));
  projection->addItem (tr ("UTM"));
  projection->setCurrentIndex (options->projection);


  hDatum = new QComboBox (this);
  hDatum->setWhatsThis (hDatumText);
  hDatum->setEditable (FALSE);
  for (NV_INT32 i = 0 ; i < options->h_datum_count ; i++)
    {
      if (options->h_datums[i].active) hDatum->addItem (options->h_datums[i].abbrev, i);
    }
  hDatum->setCurrentIndex (hDatum->findData (options->h_datum));

  connect (hDatum, SIGNAL (activated (int)), this, SLOT (slotHDatumActivated (int)));


  vDatum = new QComboBox (this);
  vDatum->setWhatsThis (vDatumText);
  vDatum->setEditable (FALSE);
  for (NV_INT32 i = 0 ; i < options->v_datum_count ; i++)
    {
      if (options->v_datums[i].active) vDatum->addItem (options->v_datums[i].abbrev, i);
    }
  vDatum->setCurrentIndex (vDatum->findData (options->v_datum));

  connect (vDatum, SIGNAL (activated (int)), this, SLOT (slotVDatumActivated (int)));


  group = new QLineEdit (this);
  group->setWhatsThis (groupText);
  group->setText (options->group);


  source = new QLineEdit (this);
  source->setWhatsThis (sourceText);
  source->setText (options->source);


  version = new QLineEdit (this);
  version->setWhatsThis (versionText);
  version->setText (options->version);


  QFormLayout *formLayout = new QFormLayout;


  formLayout->addRow (tr ("&Units:"), units);
  formLayout->addRow (tr ("&Depth correction:"), depthCor);
  formLayout->addRow (tr ("&Projection:"), projection);


  //  We're not supporting these at the moment.  Maybe in the future.

  hDatum->hide ();
  vDatum->hide ();
  //formLayout->addRow (tr ("&Horizontal datum:"), hDatum);
  //formLayout->addRow (tr ("&Vertical datum:"), vDatum);


  formLayout->addRow (tr ("&Group name:"), group);
  formLayout->addRow (tr ("Data &source:"), source);
  formLayout->addRow (tr ("Data &version:"), version);
  setLayout (formLayout);


  registerField ("units", units);
  registerField ("depthCor", depthCor);
  registerField ("projection", projection);
  registerField ("group", group);
  registerField ("source", source);
  registerField ("version", version);
}



void 
datumPage::slotHDatumActivated (int index)
{
  options->h_datum = hDatum->itemData (index).toInt ();
}



void 
datumPage::slotVDatumActivated (int index)
{
  options->v_datum = vDatum->itemData (index).toInt ();
}
