#include "imagePage.hpp"
#include "imagePageHelp.hpp"

imagePage::imagePage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Image/DEM parameters"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmChartsImageWatermark.png"));


  QHBoxLayout *hbox = new QHBoxLayout (this);
  hbox->setMargin (5);
  hbox->setSpacing (5);


  QGroupBox *pBox = new QGroupBox (tr ("Parameter Settings"), this);
  QVBoxLayout *pBoxLayout = new QVBoxLayout;
  pBox->setLayout (pBoxLayout);
  pBoxLayout->setSpacing (10);


  QGroupBox *demBox = new QGroupBox (tr ("DEM Options"), this);
  QHBoxLayout *demBoxLayout = new QHBoxLayout;
  demBox->setLayout (demBoxLayout);

  QLabel *datumLabel = new QLabel (tr ("Datum Offset"), this);
  datumLabel->setToolTip (tr ("Change the datum offset (-1000.0 - 1000.0)"));
  datumLabel->setWhatsThis (datumText);
  demBoxLayout->addWidget (datumLabel);

  datumSpin = new QDoubleSpinBox (this);
  datumSpin->setDecimals (1);
  datumSpin->setRange (-10000.0, 10000.0);
  datumSpin->setSingleStep (10.0);
  datumSpin->setValue (options->datum_offset);
  datumSpin->setWrapping (TRUE);
  datumSpin->setToolTip (tr ("Change the datum offset (-10000.0 - 10000.0)"));
  datumSpin->setWhatsThis (datumText);
  demBoxLayout->addWidget (datumSpin, 10);

  QPushButton *egmButton = new QPushButton (tr ("Get EGM08 offset"));
  egmButton->setToolTip (tr ("Get the ellipsoid to geoid datum offset from EGM08 model"));
  egmButton->setWhatsThis (egmText);
  connect (egmButton, SIGNAL (clicked ()), this, SLOT (slotEgmClicked (void)));
  demBoxLayout->addWidget (egmButton);


  QLabel *demLabel = new QLabel (tr ("DEM Grid Size (m)"), this);
  demLabel->setToolTip (tr ("Set the Digital Elevation Model grid size (m) (0.5 - 100.0)"));
  demLabel->setWhatsThis (demText);
  demBoxLayout->addWidget (demLabel);

  demSpin = new QDoubleSpinBox (this);
  demSpin->setDecimals (1);
  demSpin->setRange (0.5, 100.0);
  demSpin->setSingleStep (5.0);
  demSpin->setValue (options->grid_size);
  demSpin->setWrapping (TRUE);
  demSpin->setToolTip (tr ("Set the Digital Elevation Model grid size (m) (0.5 - 100.0)"));
  demSpin->setWhatsThis (demText);
  demBoxLayout->addWidget (demSpin);


  casi_check = new QCheckBox (tr ("Output DEM for CASI"), this);
  casi_check->setToolTip (tr ("Output a DEM file in ASCII xyz for input to CASI"));
  casi_check->setWhatsThis (casiText);
  casi_check->setChecked (options->casi);
  demBoxLayout->addWidget (casi_check);

  pBoxLayout->addWidget (demBox);


  QGroupBox *limitBox = new QGroupBox (tr ("Line Direction Limits"), this);
  QHBoxLayout *limitBoxLayout = new QHBoxLayout;
  limitBox->setLayout (limitBoxLayout);


  QHBoxLayout *lowBoxLayout = new QHBoxLayout;

  QLabel *lowLabel = new QLabel (tr ("Lower"), this);
  lowLabel->setToolTip (tr ("Set the lower direction limit"));
  lowLabel->setWhatsThis (lowText);
  lowBoxLayout->addWidget (lowLabel);

  lowSpin = new QSpinBox (this);
  lowSpin->setRange (0, 360);
  lowSpin->setSingleStep (1);
  lowSpin->setValue (options->low_limit);
  lowSpin->setWrapping (TRUE);
  lowSpin->setToolTip (tr ("Set the lower direction limit"));
  lowSpin->setWhatsThis (lowText);
  connect (lowSpin, SIGNAL (valueChanged (int)), this, SLOT (slotLowSpinValueChanged (int)));
  lowBoxLayout->addWidget (lowSpin, 1);

  limitBoxLayout->addLayout (lowBoxLayout);


  QHBoxLayout *highBoxLayout = new QHBoxLayout;

  QLabel *highLabel = new QLabel (tr ("Upper"), this);
  highLabel->setToolTip (tr ("Set the upper direction limit"));
  highLabel->setWhatsThis (highText);
  highBoxLayout->addWidget (highLabel);

  highSpin = new QSpinBox (this);
  highSpin->setRange (0, 360);
  highSpin->setSingleStep (1);
  highSpin->setValue (options->high_limit);
  highSpin->setWrapping (TRUE);
  highSpin->setToolTip (tr ("Set the upper direction limit"));
  highSpin->setWhatsThis (highText);
  connect (highSpin, SIGNAL (valueChanged (int)), this, SLOT (slotHighSpinValueChanged (int)));
  highBoxLayout->addWidget (highSpin, 1);

  limitBoxLayout->addLayout (highBoxLayout);


  opposite_check = new QCheckBox (tr ("Include lines in the opposite direction"), this);
  opposite_check->setToolTip (tr ("Include lines that are run in the opposite direction to the limits"));
  opposite_check->setWhatsThis (oppositeText);
  opposite_check->setChecked (options->opposite);
  limitBoxLayout->addWidget (opposite_check);


  if (options->low_limit == 0 && options->high_limit == 360) opposite_check->setEnabled (FALSE);

  connect (opposite_check, SIGNAL (clicked ()), this, SLOT (slotOppositeClicked (void)));


  pBoxLayout->addWidget (limitBox);


  QGroupBox *imageBox = new QGroupBox (tr ("Image quality"), this);
  QHBoxLayout *imageBoxLayout = new QHBoxLayout;
  imageBox->setLayout (imageBoxLayout);

  QHBoxLayout *intervalBoxLayout = new QHBoxLayout;

  QLabel *intervalLabel = new QLabel (tr ("Image Interval"), this);
  intervalLabel->setToolTip (tr ("Set the interval between images"));
  intervalLabel->setWhatsThis (intervalText);
  intervalBoxLayout->addWidget (intervalLabel);

  intervalSpin = new QSpinBox (this);
  intervalSpin->setRange (0, 10);
  intervalSpin->setSingleStep (1);
  intervalSpin->setValue (options->interval);
  intervalSpin->setWrapping (TRUE);
  intervalSpin->setToolTip (tr ("Set the interval between images"));
  intervalSpin->setWhatsThis (intervalText);
  intervalBoxLayout->addWidget (intervalSpin, 1);

  imageBoxLayout->addLayout (intervalBoxLayout);


  exclude_check = new QCheckBox (tr ("Exclude bright/dark images"), this);
  exclude_check->setToolTip (tr ("Exclude images that are too bright or too dark"));
  exclude_check->setWhatsThis (excludeText);
  exclude_check->setChecked (options->exclude);
  imageBoxLayout->addWidget (exclude_check);

  connect (exclude_check, SIGNAL (clicked ()), this, SLOT (slotExcludeClicked (void)));


  normalize_check = new QCheckBox (tr ("Normalize images"), this);
  normalize_check->setToolTip (tr ("Transform image to span the full range of colors"));
  normalize_check->setWhatsThis (normalizeText);
  normalize_check->setChecked (options->normalize);
  imageBoxLayout->addWidget (normalize_check);

#ifndef NVLinux
  normalize_check->setEnabled (FALSE);

  options->normalize = NVFalse;
#endif

  connect (normalize_check, SIGNAL (clicked ()), this, SLOT (slotNormalizeClicked (void)));

  pBoxLayout->addWidget (imageBox);


  QGroupBox *opBox = new QGroupBox (tr ("Nav options"), this);
  QHBoxLayout *opBoxLayout = new QHBoxLayout;
  opBox->setLayout (opBoxLayout);

  flip_check = new QCheckBox (tr ("Invert Z Value"), this);
  flip_check->setToolTip (tr ("Change sign on Z value"));
  flip_check->setWhatsThis (flipText);
  flip_check->setChecked (options->flip);
  opBoxLayout->addWidget (flip_check);

  connect (flip_check, SIGNAL (clicked ()), this, SLOT (slotFlipClicked (void)));


  pos_check = new QCheckBox (tr ("Exclude .pos navigation"), this);
  pos_check->setToolTip (tr ("Exclude any images that will use .pos files for navigation data"));
  pos_check->setWhatsThis (posText);
  pos_check->setChecked (options->pos);
  opBoxLayout->addWidget (pos_check);

  connect (pos_check, SIGNAL (clicked ()), this, SLOT (slotPosClicked (void)));

  pBoxLayout->addWidget (opBox);


  typeGrp = new QButtonGroup (this);
  typeGrp->setExclusive (TRUE);
  connect (typeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotType (int)));



  QGroupBox *typeBox = new QGroupBox (tr ("Data types"), this);
  QHBoxLayout *typeBoxLayout = new QHBoxLayout;
  typeBox->setLayout (typeBoxLayout);


  hof_check = new QCheckBox (tr ("Use HOF only"), this);
  hof_check->setToolTip (tr ("Use only images associated with HOF files"));
  hof_check->setWhatsThis (hofText);
  typeGrp->addButton (hof_check, PFM_CHARTS_HOF_DATA);
  typeBoxLayout->addWidget (hof_check);


  tof_check = new QCheckBox (tr ("Use TOF only"), this);
  tof_check->setToolTip (tr ("Use only images associated with TOF files"));
  tof_check->setWhatsThis (tofText);
  typeGrp->addButton (tof_check, PFM_SHOALS_TOF_DATA);
  typeBoxLayout->addWidget (tof_check);


  both_check = new QCheckBox (tr ("Use both HOF and TOF"), this);
  both_check->setToolTip (tr ("Use images associated with both HOF and TOF files"));
  both_check->setWhatsThis (bothText);
  typeGrp->addButton (both_check, 0);
  typeBoxLayout->addWidget (both_check);


  srtm_check = new QCheckBox (tr ("Use SRTM data"), this);
  srtm_check->setToolTip (tr ("Use SRTM data in empty land areas"));
  srtm_check->setWhatsThis (srtmText);
  srtm_check->setChecked (options->srtm);
  connect (srtm_check, SIGNAL (clicked ()), this, SLOT (slotSRTMClicked (void)));
  typeBoxLayout->addWidget (srtm_check);

  if (!options->srtm_available) srtm_check->setEnabled (FALSE);


  switch (options->type)
    {
    case 0:
      both_check->setChecked (TRUE);
      break;


      //  Just in case someone has the old PFM_SHOALS_1K_DATA stored for the type we need to switch it to the
      //  new PFM_CHARTS_HOF_DATA.

    case PFM_SHOALS_1K_DATA:
    case PFM_CHARTS_HOF_DATA:
      options->type = PFM_CHARTS_HOF_DATA;
      hof_check->setChecked (TRUE);
      break;

    case PFM_SHOALS_TOF_DATA:
      tof_check->setChecked (TRUE);
      break;
    }


  pBoxLayout->addWidget (typeBox);


  hbox->addWidget (pBox);


  registerField ("casi_check", casi_check);
  registerField ("datumSpin", datumSpin, "value");
  registerField ("demSpin", demSpin, "value");
  registerField ("intervalSpin", intervalSpin);
}



void imagePage::slotEgmClicked ()
{
  NV_FLOAT64 lat, lon;

  lat = options->mbr.min_y + (options->mbr.max_y - options->mbr.min_y) / 2.0;
  lon = options->mbr.min_x + (options->mbr.max_x - options->mbr.min_x) / 2.0;

  NV_FLOAT32 datum_offset = get_egm08 (lat, lon);
  cleanup_egm08 ();


  if (datum_offset < 999999.0) 
    {
      datumSpin->setValue (datum_offset);
    }
  else
    {
      QMessageBox::warning (this, tr ("Get EGM08 datum offset"), tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
    }
}



void imagePage::slotType (int id)
{
  options->type = id;
}



void imagePage::slotSRTMClicked ()
{
  if (srtm_check->isChecked ())
    {
      options->srtm = NVTrue;
    }
  else
    {
      options->srtm = NVFalse;
    }
}



void imagePage::slotExcludeClicked ()
{
  if (exclude_check->isChecked ())
    {
      options->exclude = NVTrue;
    }
  else
    {
      options->exclude = NVFalse;
    }
}


void imagePage::slotOppositeClicked ()
{
  if (opposite_check->isChecked ())
    {
      options->opposite = NVTrue;
    }
  else
    {
      options->opposite = NVFalse;
    }
}


void imagePage::slotNormalizeClicked ()
{
  if (normalize_check->isChecked ())
    {
      options->normalize = NVTrue;
    }
  else
    {
      options->normalize = NVFalse;
    }
}


void imagePage::slotFlipClicked ()
{
  if (flip_check->isChecked ())
    {
      options->flip = NVTrue;
    }
  else
    {
      options->flip = NVFalse;
    }
}


void imagePage::slotPosClicked ()
{
  if (pos_check->isChecked ())
    {
      options->pos = NVTrue;
    }
  else
    {
      options->pos = NVFalse;
    }
}


void imagePage::slotLowSpinValueChanged (int value)
{
  options->low_limit = value;

  if (options->low_limit == 0 && options->high_limit == 360)
    {
      opposite_check->setChecked (FALSE);
      opposite_check->setEnabled (FALSE);
      options->opposite = NVFalse;
    }
  else
    {
      opposite_check->setEnabled (TRUE);
    }
}


void imagePage::slotHighSpinValueChanged (int value)
{
  options->high_limit = value;

  if (options->low_limit == 0 && options->high_limit == 360)
    {
      opposite_check->setChecked (FALSE);
      opposite_check->setEnabled (FALSE);
      options->opposite = NVFalse;
    }
  else
    {
      opposite_check->setEnabled (TRUE);
    }
}
