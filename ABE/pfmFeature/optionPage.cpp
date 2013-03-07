#include "optionPage.hpp"
#include "optionPageHelp.hpp"
#include "featureTypes.hpp"

optionPage::optionPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Feature selection options"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmFeatureWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout (hbox);


  QGroupBox *optionbox = new QGroupBox (tr ("Options"), this);
  QHBoxLayout *optionboxLayout = new QHBoxLayout;
  optionbox->setLayout (optionboxLayout);


  QGroupBox *obox = new QGroupBox (tr ("IHO order"), this);
  QHBoxLayout *oboxLayout = new QHBoxLayout;
  obox->setLayout (oboxLayout);
  order = new QComboBox (obox);
  order->setToolTip (tr ("Select the IHO order for feature selection"));
  order->setWhatsThis (orderText);
  order->setEditable (FALSE);
  order->addItem (tr ("IHO special order"));
  order->addItem (tr ("IHO order 1"));
  order->setCurrentIndex (options->order);
  oboxLayout->addWidget (order);
  optionboxLayout->addWidget (obox);


  QGroupBox *zeroBox = new QGroupBox (tr ("Select features above zero"), this);
  QHBoxLayout *zeroBoxLayout = new QHBoxLayout;
  zeroBox->setLayout (zeroBoxLayout);
  zero = new QCheckBox (this);
  zero->setToolTip (tr ("If checked, features will be selected above zero"));
  zero->setWhatsThis (zeroText);
  zero->setChecked (options->zero);
  zeroBoxLayout->addWidget (zero);
  optionboxLayout->addWidget (zeroBox);


  QGroupBox *offBox = new QGroupBox (tr ("Offset"), this);
  QHBoxLayout *offBoxLayout = new QHBoxLayout;
  offBox->setLayout (offBoxLayout);
  offset = new QDoubleSpinBox (offBox);
  offset->setDecimals (2);
  offset->setRange (-1000.0, 1000.0);
  offset->setSingleStep (10.0);
  offset->setToolTip (tr ("Set a Z offset value"));
  offset->setWhatsThis (offsetText);
  offBoxLayout->addWidget (offset);

  QPushButton *egmButton = new QPushButton (tr ("EGM08"));
  egmButton->setToolTip (tr ("Get the ellipsoid to geoid datum offset from EGM08 model"));
  egmButton->setWhatsThis (egmText);
  connect (egmButton, SIGNAL (clicked ()), this, SLOT (slotEgmClicked (void)));
  offBoxLayout->addWidget (egmButton);
  optionboxLayout->addWidget (offBox);


  QGroupBox *hpcBox = new QGroupBox (tr ("Compute confidence"), this);
  QHBoxLayout *hpcBoxLayout = new QHBoxLayout;
  hpcBox->setLayout (hpcBoxLayout);
  hpc = new QCheckBox (this);
  hpc->setToolTip (tr ("Compute confidence values using HPC"));
  hpc->setWhatsThis (hpcText);
  hpc->setChecked (options->hpc);
  hpcBoxLayout->addWidget (hpc);
  optionboxLayout->addWidget (hpcBox);


  vbox->addWidget (optionbox);


  QGroupBox *descBox = new QGroupBox (tr ("Description"), this);
  QVBoxLayout *descBoxLayout = new QVBoxLayout;
  descBox->setLayout (descBoxLayout);
  QHBoxLayout *descBoxTopLayout = new QHBoxLayout;
  QHBoxLayout *descBoxBottomLayout = new QHBoxLayout;
  descBoxLayout->addLayout (descBoxTopLayout);
  descBoxLayout->addLayout (descBoxBottomLayout);

  description = new QLineEdit (descBox);
  description->setToolTip (tr ("Default feature description"));
  description->setWhatsThis (descriptionText);
  descBoxTopLayout->addWidget (description);

  QPushButton *rocks = new QPushButton (tr ("Rocks, Wrecks, and Obstructions"), this);
  rocks->setToolTip (tr ("Select standard description from list of rock, wreck, and obstruction types"));
  connect (rocks, SIGNAL (clicked ()), this, SLOT (slotRocks ()));
  descBoxBottomLayout->addWidget (rocks);

  QPushButton *offshore = new QPushButton (tr ("Offshore Installations"), this);
  offshore->setToolTip (tr ("Select standard description from list of offshore installation types"));
  connect (offshore, SIGNAL (clicked ()), this, SLOT (slotOffshore ()));
  descBoxBottomLayout->addWidget (offshore);

  QPushButton *lights = new QPushButton (tr ("Lights, Buoys, and Beacons"), this);
  lights->setToolTip (tr ("Select standard description from list of light, buoy, and beacon types"));
  connect (lights, SIGNAL (clicked ()), this, SLOT (slotLights ()));
  descBoxBottomLayout->addWidget (lights);
  

  vbox->addWidget (descBox, 1);


  registerField ("order", order);
  registerField ("zero", zero);
  registerField ("offset", offset);
  registerField ("hpc", hpc);
  registerField ("description", description);
}



void 
optionPage::slotRocks ()
{
  rockD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  rockD->setWindowTitle (tr ("pfmEdit Rock, Wreck, and Obstruction Types"));
  rockD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (rockD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  rockBox = new QListWidget (rockD);
  rockBox->setSelectionMode (QAbstractItemView::SingleSelection);
  rockBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < ROCK_TYPES ; i++) rockBox->addItem (rockType[i]);


  vbox->addWidget (rockBox, 10);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (rockD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *OKButton = new QPushButton (tr ("OK"), rockD);
  OKButton->setToolTip (tr ("Use the selected item as the feature description"));
  connect (OKButton, SIGNAL (clicked ()), this, SLOT (slotRockOKClicked ()));
  actions->addWidget (OKButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), rockD);
  cancelButton->setToolTip (tr ("Discard the selection and close the dialog"));
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotRockCancelClicked ()));
  actions->addWidget (cancelButton);


  rockD->show ();
}


void 
optionPage::slotRockOKClicked ()
{
  description->setText (rockBox->currentItem ()->text ());

  rockD->close ();
}


void 
optionPage::slotRockCancelClicked ()
{
  rockD->close ();
}


void 
optionPage::slotOffshore ()
{
  offshoreD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose); 
  offshoreD->setWindowTitle (tr ("pfmEdit Offshore Installation Types"));
  offshoreD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (offshoreD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  offshoreBox = new QListWidget (offshoreD);
  offshoreBox->setSelectionMode (QAbstractItemView::SingleSelection);
  offshoreBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < OFFSHORE_TYPES ; i++) offshoreBox->addItem (offshoreType[i]);


  vbox->addWidget (offshoreBox, 10);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (offshoreD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *OKButton = new QPushButton (tr ("OK"), offshoreD);
  OKButton->setToolTip (tr ("Use the selected item as the feature description"));
  connect (OKButton, SIGNAL (clicked ()), this, SLOT (slotOffshoreOKClicked ()));
  actions->addWidget (OKButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), offshoreD);
  cancelButton->setToolTip (tr ("Discard the selection and close the dialog"));
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotOffshoreCancelClicked ()));
  actions->addWidget (cancelButton);


  offshoreD->show ();
}


void 
optionPage::slotOffshoreOKClicked ()
{
  description->setText (offshoreBox->currentItem ()->text ());

  offshoreD->close ();
}


void 
optionPage::slotOffshoreCancelClicked ()
{
  offshoreD->close ();
}


void 
optionPage::slotLights ()
{
  lightD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  lightD->setWindowTitle (tr ("pfmEdit Light, Buoy, and Beacon Types"));
  lightD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (lightD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  lightBox = new QListWidget (lightD);
  lightBox->setSelectionMode (QAbstractItemView::SingleSelection);
  lightBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < LIGHT_TYPES ; i++) lightBox->addItem (lightType[i]);


  vbox->addWidget (lightBox, 10);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (lightD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *OKButton = new QPushButton (tr ("OK"), lightD);
  OKButton->setToolTip (tr ("Use the selected item as the feature description"));
  connect (OKButton, SIGNAL (clicked ()), this, SLOT (slotLightOKClicked ()));
  actions->addWidget (OKButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), lightD);
  cancelButton->setToolTip (tr ("Discard the selection and close the dialog"));
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotLightCancelClicked ()));
  actions->addWidget (cancelButton);


  lightD->show ();
}


void 
optionPage::slotLightOKClicked ()
{
  description->setText (lightBox->currentItem ()->text ());

  lightD->close ();
}


void 
optionPage::slotLightCancelClicked ()
{
  lightD->close ();
}



void 
optionPage::setFiles (QString pfm_file, QString area_file)
{
  pfmFile = pfm_file;
  areaFile = area_file;
}



//  Get the EGM value nearest to the center of the displayed area.

void 
optionPage::slotEgmClicked ()
{
  PFM_OPEN_ARGS       open_args;
  NV_INT32            pfm_handle;
  NV_FLOAT64          lat, lon;

  
  if (!areaFile.isEmpty ())
    {
      NV_CHAR af[512];
      strcpy (af, areaFile.toAscii ());

      get_area_mbr (af, &options->polygon_count, options->polygon_x, options->polygon_y, &options->mbr);

      lat = options->mbr.min_y + (options->mbr.max_y - options->mbr.min_y) / 2.0;
      lon = options->mbr.min_x + (options->mbr.max_x - options->mbr.min_x) / 2.0;
    }
  else
    {
      strcpy (open_args.list_path, pfmFile.toAscii ());


      //  Open the PFM file.

      open_args.checkpoint = 0;
      if ((pfm_handle = open_existing_pfm_file (&open_args)) < 0) pfm_error_exit (pfm_error);

      lat = open_args.head.mbr.min_y + (open_args.head.mbr.max_y - open_args.head.mbr.min_y) / 2.0;
      lon = open_args.head.mbr.min_x + (open_args.head.mbr.max_x - open_args.head.mbr.min_x) / 2.0;

      close_pfm_file (pfm_handle);
    }


  NV_FLOAT32 datum_offset = get_egm08 (lat, lon);
  cleanup_egm08 ();

  if (datum_offset < 999999.0) 
    {
      offset->setValue ((NV_FLOAT64) datum_offset);
    }
  else
    {
      QMessageBox::warning (this, tr ("pfmView Get EGM08 datum offset"), tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
    }
}



void
optionPage::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}
