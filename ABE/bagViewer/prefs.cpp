
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



#include "prefs.hpp"
#include "prefsHelp.hpp"

Prefs::Prefs (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;

  mod_options = *op;
  mod_misc = *mi;


  dataChanged = NVFalse;


  setWindowTitle (tr ("bagViewer Preferences"));


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


  QGroupBox *miscBox = new QGroupBox (tr ("Miscellaneous"), this);
  QHBoxLayout *miscBoxLayout = new QHBoxLayout;
  miscBox->setLayout (miscBoxLayout);


  QGroupBox *exagBox = new QGroupBox (tr ("Z exaggeration"), this);
  QHBoxLayout *exagBoxLayout = new QHBoxLayout;
  exagBox->setLayout (exagBoxLayout);
  exag = new QDoubleSpinBox (this);
  exag->setDecimals (1);
  exag->setRange (1.0, 20.0);
  exag->setSingleStep (1.0);
  exag->setValue (options->exaggeration);
  exag->setToolTip (tr ("Change the Z exaggeration (1.0-20.0)"));
  exag->setWhatsThis (exagText);
  exagBoxLayout->addWidget (exag);
  miscBoxLayout->addWidget (exagBox);


  QGroupBox *fSizeBox = new QGroupBox (tr ("Tracking list point size"), this);
  QHBoxLayout *fSizeBoxLayout = new QHBoxLayout;
  fSizeBox->setLayout (fSizeBoxLayout);
  fSize = new QDoubleSpinBox (this);
  fSize->setDecimals (3);
  fSize->setRange (0.001, 0.050);
  fSize->setSingleStep (0.001);
  fSize->setValue (options->feature_size);
  fSize->setToolTip (tr ("Change the tracking list marker size (0.001-0.050)"));
  fSize->setWhatsThis (fSizeText);
  fSizeBoxLayout->addWidget (fSize);
  miscBoxLayout->addWidget (fSizeBox);


  QGroupBox *scaleBox = new QGroupBox (tr ("Draw scale"), this);
  QHBoxLayout *scaleBoxLayout = new QHBoxLayout;
  scaleBox->setLayout (scaleBoxLayout);
  scale = new QCheckBox (this);
  scale->setToolTip (tr ("If checked, the scale will be drawn on the screen"));
  scale->setWhatsThis (scaleText);
  scaleBoxLayout->addWidget (scale);
  miscBoxLayout->addWidget (scaleBox);


  vbox->addWidget (miscBox);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxLayout->addWidget (bBackgroundColor);


  bTrackerColor = new QPushButton (tr ("Tracking cursor"), this);
  bTrackerColor->setToolTip (tr ("Change tracking (box) cursor color"));
  bTrackerColor->setWhatsThis (trackerColorText);
  bTrackerPalette = bTrackerColor->palette ();
  connect (bTrackerColor, SIGNAL (clicked ()), this, SLOT (slotTrackerColor ()));
  cboxLayout->addWidget (bTrackerColor);


  bFeatureColor = new QPushButton (tr ("Tracking list"), this);
  bFeatureColor->setToolTip (tr ("Change tracking list marker color"));
  bFeatureColor->setWhatsThis (featureColorText);
  bFeaturePalette = bFeatureColor->palette ();
  connect (bFeatureColor, SIGNAL (clicked ()), this, SLOT (slotFeatureColor ()));
  cboxLayout->addWidget (bFeatureColor);


  bFeatureInfoColor = new QPushButton (tr ("Tracking list information"), this);
  bFeatureInfoColor->setToolTip (tr ("Change tracking list information text color"));
  bFeatureInfoColor->setWhatsThis (featureInfoColorText);
  bFeatureInfoPalette = bFeatureInfoColor->palette ();
  connect (bFeatureInfoColor, SIGNAL (clicked ()), this, SLOT (slotFeatureInfoColor ()));
  cboxLayout->addWidget (bFeatureInfoColor);


  bScaleColor = new QPushButton (tr ("Scale"), this);
  bScaleColor->setToolTip (tr ("Change scale color"));
  bScaleColor->setWhatsThis (scaleColorText);
  bScalePalette = bScaleColor->palette ();
  connect (bScaleColor, SIGNAL (clicked ()), this, SLOT (slotScaleColor ()));
  cboxLayout->addWidget (bScaleColor);
  vbox->addWidget (cbox, 1);


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
Prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("bagViewer Background Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.background_color = clr;

  setFields ();
}



void
Prefs::slotTrackerColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.tracker_color, this, tr ("bagViewer Marker Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.tracker_color = clr;

  setFields ();
}



void
Prefs::slotFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_color, this, tr ("bagViewer Tracking List Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_color = clr;

  setFields ();
}



void
Prefs::slotFeatureInfoColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_info_color, this, tr ("bagViewer Tracking List Information Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_info_color = clr;

  setFields ();
}



void
Prefs::slotScaleColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.scale_color, this, tr ("bagViewer Scale Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.scale_color = clr;

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
  mod_misc = *misc;


  setFields ();


  *options = mod_options;
  *misc = mod_misc;

  emit dataChangedSignal ();

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

  exag->setValue (mod_options.exaggeration);

  fSize->setValue (mod_options.feature_size);

  bGrp->button (mod_options.position_form)->setChecked (TRUE);

  scale->setChecked (mod_options.draw_scale);

  NV_INT32 hue, sat, val;

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


  mod_options.tracker_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bTrackerPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bTrackerPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bTrackerPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bTrackerPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bTrackerPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.tracker_color);
  bTrackerPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.tracker_color);
  bTrackerColor->setPalette (bTrackerPalette);


  mod_options.feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_color);
  bFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_color);
  bFeatureColor->setPalette (bFeaturePalette);


  mod_options.feature_info_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeatureInfoPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_info_color);
  bFeatureInfoPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_info_color);
  bFeatureInfoColor->setPalette (bFeatureInfoPalette);


  mod_options.scale_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bScalePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bScalePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bScalePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bScalePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bScalePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.scale_color);
  bScalePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.scale_color);
  bScaleColor->setPalette (bScalePalette);
}



void
Prefs::slotApplyPrefs ()
{
  mod_options.exaggeration = exag->value ();
  mod_options.feature_size = fSize->value ();
  mod_options.draw_scale = scale->isChecked ();


  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.tracker_color != options->tracker_color) dataChanged = NVTrue;
  if (mod_options.feature_color != options->feature_color) dataChanged = NVTrue;
  if (mod_options.feature_info_color != options->feature_info_color) dataChanged = NVTrue;
  if (mod_options.scale_color != options->scale_color) dataChanged = NVTrue;
  if (mod_options.exaggeration != options->exaggeration) dataChanged = NVTrue;
  if (mod_options.feature_size != options->feature_size) dataChanged = NVTrue;
  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;
  if (mod_options.draw_scale != options->draw_scale) dataChanged = NVTrue;


  setFields ();


  if (dataChanged)
    {
      *options = mod_options;
      *misc = mod_misc;

      emit dataChangedSignal ();
    }

  close ();
}



void
Prefs::slotClosePrefs ()
{
  close ();
}
