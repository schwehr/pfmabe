#include "prefs.hpp"
#include "prefsHelp.hpp"

Prefs::Prefs (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  mod_options = *options;
  misc = mi;
  dataChanged = NVFalse;


  setWindowTitle (tr ("areaCheck preferences"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *fbox = new QGroupBox (tr ("Position Format"), this);
  fbox->setWhatsThis (bGrpText);

  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  bGrp = new QButtonGroup (this);
  bGrp->setExclusive (TRUE);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPositionClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  QHBoxLayout *fboxSplit = new QHBoxLayout;
  QVBoxLayout *fboxLeft = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxLeft->addWidget (hd__);
  fboxRight->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QHBoxLayout *cboxSplit = new QHBoxLayout;
  cbox->setLayout (cboxSplit);
  QVBoxLayout *cboxLeft = new QVBoxLayout;
  QVBoxLayout *cboxRight = new QVBoxLayout;
  cboxSplit->addLayout (cboxLeft);
  cboxSplit->addLayout (cboxRight);


  bCoastColor = new QPushButton (tr ("Coast"), this);
  bCoastPalette = bCoastColor->palette ();
  bCoastColor->setToolTip (tr ("Change coastline color"));
  bCoastColor->setWhatsThis (coastColorText);
  connect (bCoastColor, SIGNAL (clicked ()), this, SLOT (slotCoastColor ()));
  cboxLeft->addWidget (bCoastColor);


  bMaskColor = new QPushButton (tr ("Landmask"), this);
  bMaskPalette = bMaskColor->palette ();
  bMaskColor->setToolTip (tr ("Change landmask color"));
  bMaskColor->setWhatsThis (maskColorText);
  connect (bMaskColor, SIGNAL (clicked ()), this, SLOT (slotMaskColor ()));
  cboxLeft->addWidget (bMaskColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundPalette = bBackgroundColor->palette ();
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxRight->addWidget (bBackgroundColor);


  bHighlightColor = new QPushButton (tr ("Highlight"), this);
  bHighlightPalette = bHighlightColor->palette ();
  bHighlightColor->setToolTip (tr ("Change highlight color"));
  bHighlightColor->setWhatsThis (highlightColorText);
  connect (bHighlightColor, SIGNAL (clicked ()), this, SLOT (slotHighlightColor ()));
  cboxRight->addWidget (bHighlightColor);


  vbox->addWidget (cbox, 1);


  QGroupBox *c2box = new QGroupBox (tr ("Data Type Colors"), this);
  c2box->setWhatsThis (colorGrpText);

  c2[0] = new QPushButton (misc->type_name[ISS60_AREA], this);
  c2Palette[0] = c2[0]->palette ();
  c2[1] = new QPushButton (misc->type_name[GENERIC_AREA], this);
  c2Palette[1] = c2[1]->palette ();
  c2[2] = new QPushButton (misc->type_name[ACE_AREA], this);
  c2Palette[2] = c2[2]->palette ();
  c2[3] = new QPushButton (misc->type_name[ISS60_ZONE], this);
  c2Palette[3] = c2[3]->palette ();
  c2[4] = new QPushButton (misc->type_name[GENERIC_ZONE], this);
  c2Palette[4] = c2[4]->palette ();
  c2[5] = new QPushButton (misc->type_name[SHAPE], this);
  c2Palette[5] = c2[5]->palette ();
  c2[6] = new QPushButton (misc->type_name[TRACK], this);
  c2Palette[6] = c2[6]->palette ();
  c2[7] = new QPushButton (misc->type_name[FEATURE], this);
  c2Palette[7] = c2[7]->palette ();
  c2[8] = new QPushButton (misc->type_name[YXZ], this);
  c2Palette[8] = c2[8]->palette ();
  c2[9] = new QPushButton (misc->type_name[LLZ_DATA], this);
  c2Palette[9] = c2[9]->palette ();
  c2[10] = new QPushButton (misc->type_name[GEOTIFF], this);
  c2Palette[10] = c2[10]->palette ();
  c2[11] = new QPushButton (misc->type_name[CHRTR], this);
  c2Palette[11] = c2[11]->palette ();
  c2[12] = new QPushButton (misc->type_name[BAG], this);
  c2Palette[12] = c2[12]->palette ();
  c2[13] = new QPushButton (misc->type_name[OPTECH_FLT], this);
  c2Palette[13] = c2[13]->palette ();


  colorGrp = new QButtonGroup (c2box);
  colorGrp->setExclusive (FALSE);
  connect (colorGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotColor (int)));

  colorGrp->addButton (c2[0], ISS60_AREA);
  colorGrp->addButton (c2[1], GENERIC_AREA);
  colorGrp->addButton (c2[2], ACE_AREA);
  colorGrp->addButton (c2[3], ISS60_ZONE);
  colorGrp->addButton (c2[4], GENERIC_ZONE);
  colorGrp->addButton (c2[5], SHAPE);
  colorGrp->addButton (c2[6], TRACK);
  colorGrp->addButton (c2[7], FEATURE);
  colorGrp->addButton (c2[8], YXZ);
  colorGrp->addButton (c2[9], LLZ_DATA);
  colorGrp->addButton (c2[10], GEOTIFF);
  colorGrp->addButton (c2[11], CHRTR);
  colorGrp->addButton (c2[12], BAG);
  colorGrp->addButton (c2[13], OPTECH_FLT);

  QHBoxLayout *c2boxSplit = new QHBoxLayout;
  QVBoxLayout *c2boxLeft = new QVBoxLayout;
  QVBoxLayout *c2boxRight = new QVBoxLayout;
  c2boxSplit->addLayout (c2boxLeft);
  c2boxSplit->addLayout (c2boxRight);
  c2boxLeft->addWidget (c2[0]);
  c2boxLeft->addWidget (c2[1]);
  c2boxLeft->addWidget (c2[2]);
  c2boxLeft->addWidget (c2[3]);
  c2boxLeft->addWidget (c2[4]);
  c2boxLeft->addWidget (c2[5]);
  c2boxLeft->addWidget (c2[6]);
  c2boxRight->addWidget (c2[7]);
  c2boxRight->addWidget (c2[8]);
  c2boxRight->addWidget (c2[9]);
  c2boxRight->addWidget (c2[10]);
  c2boxRight->addWidget (c2[11]);
  c2boxRight->addWidget (c2[12]);
  c2boxRight->addWidget (c2[13]);
  c2box->setLayout (c2boxSplit);

  vbox->addWidget (c2box, 1);


  QGroupBox *sunBox = new QGroupBox (tr ("CHRTR/BAG Sun Shading"), this);
  QHBoxLayout *sunBoxLayout = new QHBoxLayout;
  sunBox->setLayout (sunBoxLayout);

  QGroupBox *angBox = new QGroupBox (tr ("Angle"), this);
  QHBoxLayout *angBoxLayout = new QHBoxLayout;
  angBox->setLayout (angBoxLayout);
  sunAz = new QDoubleSpinBox (angBox);
  sunAz->setDecimals (1);
  sunAz->setRange (0.0, 359.9);
  sunAz->setSingleStep (1.0);
  sunAz->setValue (mod_options.sunopts.azimuth);
  sunAz->setWrapping (TRUE);
  sunAz->setToolTip (tr ("Change the sun angle (0.0-360.0)"));
  sunAz->setWhatsThis (sunAzText);
  angBoxLayout->addWidget (sunAz);
  sunBoxLayout->addWidget (angBox);


  QGroupBox *eleBox = new QGroupBox (tr ("Elevation"), this);
  QHBoxLayout *eleBoxLayout = new QHBoxLayout;
  eleBox->setLayout (eleBoxLayout);
  sunEl = new QDoubleSpinBox (eleBox);
  sunEl->setDecimals (1);
  sunEl->setRange (0.0, 90.0);
  sunEl->setSingleStep (1.0);
  sunEl->setValue (mod_options.sunopts.elevation);
  sunEl->setWrapping (TRUE);
  sunEl->setToolTip (tr ("Change the sun elevation (0.0-90.0)"));
  sunEl->setWhatsThis (sunElText);
  eleBoxLayout->addWidget (sunEl);
  sunBoxLayout->addWidget (eleBox);


  QGroupBox *exaBox = new QGroupBox (tr ("Exaggeration"), this);
  QHBoxLayout *exaBoxLayout = new QHBoxLayout;
  exaBox->setLayout (exaBoxLayout);
  sunEx = new QDoubleSpinBox (exaBox);
  sunEx->setDecimals (1);
  sunEx->setRange (0.0, 359.9);
  sunEx->setSingleStep (1.0);
  sunEx->setValue (mod_options.sunopts.exag);
  sunEx->setWrapping (TRUE);
  sunEx->setToolTip (tr ("Change the sun Z exaggeration (1-10)"));
  sunEx->setWhatsThis (sunExText);
  exaBoxLayout->addWidget (sunEx);
  sunBoxLayout->addWidget (exaBox);


  vbox->addWidget (sunBox, 1);


  QGroupBox *mbox = new QGroupBox (tr ("MINMAX parameters"), this);
  mbox->setWhatsThis (colorGrpText);
  QHBoxLayout *mboxLayout = new QHBoxLayout;
  mbox->setLayout (mboxLayout);

  QGroupBox *sBox = new QGroupBox (tr ("Use stoplight colors"), this);
  QHBoxLayout *sBoxLayout = new QHBoxLayout;
  sBox->setLayout (sBoxLayout);
  stopLight = new QCheckBox (this);
  connect (stopLight, SIGNAL (clicked ()), this, SLOT (slotStopLightClicked ()));
  stopLight->setToolTip (tr ("Use stoplight colors when displaying MINMAX .trk files"));
  stopLight->setWhatsThis (stopLightText);
  sBoxLayout->addWidget (stopLight);
  mboxLayout->addWidget (sBox);

  QGroupBox *gBox = new QGroupBox (tr ("Green to yellow value"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  minStop = new QLineEdit (this);
  mboxLayout->addWidget (minStop);
  minStop->setToolTip (tr ("Set value below which data is green"));
  minStop->setWhatsThis (minStopText);
  gBoxLayout->addWidget (minStop);
  mboxLayout->addWidget (gBox);

  QGroupBox *yBox = new QGroupBox (tr ("Yellow to red value"), this);
  QHBoxLayout *yBoxLayout = new QHBoxLayout;
  yBox->setLayout (yBoxLayout);
  maxStop = new QLineEdit (this);
  mboxLayout->addWidget (maxStop);
  maxStop->setToolTip (tr ("Set value above which data is red"));
  maxStop->setWhatsThis (maxStopText);
  yBoxLayout->addWidget (maxStop);
  mboxLayout->addWidget (yBox);


  vbox->addWidget (mbox, 1);


  setFields ();


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  bRestoreDefaults = new QPushButton (tr ("Restore Defaults"), this);
  bRestoreDefaults->setToolTip (tr ("Restore all preferences to the default state"));
  bRestoreDefaults->setWhatsThis (restoreDefaultsText);
  connect (bRestoreDefaults, SIGNAL (clicked ()), this, SLOT (slotRestoreDefaults ()));
  actions->addWidget (bRestoreDefaults);

  QPushButton *applyButton = new QPushButton (tr ("OK"), this);
  applyButton->setToolTip (tr ("Accept changes and close dialog"));
  applyButton->setWhatsThis (applyPrefsText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyPrefs ()));
  actions->addWidget (applyButton);

  QPushButton *closeButton = new QPushButton (tr ("Cancel"), this);
  closeButton->setToolTip (tr ("Discard changes and close dialog"));
  closeButton->setWhatsThis (closePrefsText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);


  show ();
}



Prefs::~Prefs ()
{
}



void
Prefs::slotCoastColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.coast_color, this, tr ("areaCheck Coastline Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.coast_color = clr;

  setFields ();
}



void
Prefs::slotMaskColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.mask_color, this, tr ("areaCheck Land Mask Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.mask_color = clr;

  setFields ();
}



void
Prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("areaCheck Background Color"));

  if (clr.isValid ()) mod_options.background_color = clr;

  setFields ();
}



void
Prefs::slotColor (int id)
{
  QString name;


  switch (id)
    {
    case 0:
      name = tr ("ISS-60 Area File");
      break;

    case 1:
      name = tr ("Generic Area File");
      break;

    case 2:
      name = tr ("ACE Area File");
      break;

    case 3:
      name = tr ("ISS-60 Zone File");
      break;

    case 4:
      name = tr ("Generic Zone File");
      break;

    case 5:
      name = tr ("Shape File");
      break;

    case 6:
      name = tr ("Track File");
      break;

    case 7:
      name = tr ("Feature File");
      break;

    case 8:
      name = tr ("YXZ File");
      break;

    case 9:
      name = tr ("LLZ File");
      break;

    case 10:
      name = tr ("GeoTIFF File");
      break;

    case 11:
      name = tr ("CHRTR File");
      break;

    case 12:
      name = tr ("BAG File");
      break;

    case 13:
      name = tr ("Optech Flightline File");
      break;
    }

  QColor clr;

  clr = QColorDialog::getColor (mod_options.color[id], this, tr ("areaCheck ") + name + tr (" Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.color[id] = clr;

  setFields ();
}



void
Prefs::slotHighlightColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.highlight_color, this, tr ("areaCheck Highlight Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.highlight_color = clr;

  setFields ();
}



void
Prefs::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
Prefs::slotRestoreDefaults ()
{
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


  set_defaults (misc, options, NVTrue);


  mod_options = *options;


  setFields ();


  emit dataChangedSignal (NVTrue);


  close ();
}



void
Prefs::slotPositionClicked (int id)
{
  mod_options.position_form = id;
}



void 
Prefs::setFields ()
{
  QString string;


  bGrp->button (mod_options.position_form)->setChecked (TRUE);


  NV_INT32 hue, sat, val;

  mod_options.coast_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCoastPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCoastPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCoastPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.coast_color);
  bCoastPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.coast_color);
  bCoastColor->setPalette (bCoastPalette);


  mod_options.mask_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bMaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bMaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bMaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bMaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bMaskPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.mask_color);
  bMaskPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.mask_color);
  bMaskColor->setPalette (bMaskPalette);


  mod_options.background_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bBackgroundPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.background_color);
  bBackgroundPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.background_color);
  bBackgroundColor->setPalette (bBackgroundPalette);


  for (NV_INT32 i = 0 ; i < NUM_TYPES ; i++)
    {
      mod_options.color[i].getHsv (&hue, &sat, &val);
      if (i != GEOTIFF && i != CHRTR && i != BAG && val < 128)
        {
          c2Palette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
          c2Palette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
        }
      else
        {
          c2Palette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          c2Palette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
        }
      c2Palette[i].setColor (QPalette::Normal, QPalette::Button, mod_options.color[i]);
      c2Palette[i].setColor (QPalette::Inactive, QPalette::Button, mod_options.color[i]);
      c2[i]->setPalette (c2Palette[i]);
    }


  mod_options.highlight_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bHighlightPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.highlight_color);
  bHighlightPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.highlight_color);
  bHighlightColor->setPalette (bHighlightPalette);


  sunAz->setValue (mod_options.sunopts.azimuth);
  sunEl->setValue (mod_options.sunopts.elevation);
  sunEx->setValue (mod_options.sunopts.exag);
  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);


  stopLight->setChecked (mod_options.stoplight);

  QString tmp;

  tmp.sprintf ("%.2f", mod_options.minstop);
  minStop->setText (tmp);
  tmp.sprintf ("%.2f", mod_options.maxstop);
  maxStop->setText (tmp);
}


void 
Prefs::slotStopLightClicked ()
{
  if (stopLight->isChecked ())
    {
      mod_options.stoplight = NVTrue;
    }
  else
    {
      mod_options.stoplight = NVFalse;
    }
}


void
Prefs::slotApplyPrefs ()
{
  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;

  if (mod_options.stoplight != options->stoplight) dataChanged = NVTrue;

  sscanf (minStop->text ().toAscii (), "%f", &mod_options.minstop);
  if (mod_options.minstop != options->minstop) dataChanged = NVTrue;

  sscanf (maxStop->text ().toAscii (), "%f", &mod_options.maxstop);
  if (mod_options.maxstop != options->maxstop) dataChanged = NVTrue;

  mod_options.sunopts.azimuth = sunAz->value ();
  if (mod_options.sunopts.azimuth != options->sunopts.azimuth) dataChanged = NVTrue;
  mod_options.sunopts.elevation = sunEl->value ();
  if (mod_options.sunopts.elevation != options->sunopts.elevation) dataChanged = NVTrue;
  mod_options.sunopts.exag = sunEx->value ();
  if (mod_options.sunopts.exag != options->sunopts.exag) dataChanged = NVTrue;


  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);


  if (mod_options.coast_color != options->coast_color) dataChanged = NVTrue;
  if (mod_options.mask_color != options->mask_color) dataChanged = NVTrue;
  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.highlight_color != options->highlight_color) dataChanged = NVTrue;

  for (NV_INT32 i = 0 ; i < NUM_TYPES ; i++)
    {
      if (mod_options.color[i] != options->color[i]) dataChanged = NVTrue;
    }


  if (mod_options.color[CHRTR] != options->color[CHRTR] || mod_options.color[BAG] != options->color[BAG])
    {
      //  Redefine the surface colors because the CHRTR/BAG alpha value has changed..

      NV_FLOAT32 hue_inc = 315.0 / (NV_FLOAT32) (NUMHUES + 1);

      for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
        {
          for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
            {
              misc->color_array[i][j].setHsv ((NV_INT32) (((NUMHUES + 1) - i) * hue_inc), 255, j, mod_options.color[CHRTR].alpha ());
            }
        }
      dataChanged = NVTrue;
    }


  setFields ();


  if (dataChanged)
    {
      *options = mod_options;

      emit dataChangedSignal (NVTrue);
    }

  close ();
}


void
Prefs::slotClosePrefs ()
{
  close ();
}
