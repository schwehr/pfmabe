
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


//!  This is the preferences dialog.

Prefs::Prefs (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;

  mod_share = *misc->abe_share;
  mod_options = *op;

  setModal (TRUE);

  ancillaryProgramD = contoursD = geotiffD = hotKeyD = NULL;

  dataChanged = NVFalse;


  setWindowTitle (tr ("pfmEdit Preferences"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  QGroupBox *formBox = new QGroupBox (this);
  QVBoxLayout *formBoxLayout = new QVBoxLayout;
  formBox->setLayout (formBoxLayout);

  bGrp = new QButtonGroup (this);
  bGrp->setExclusive (TRUE);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPositionClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  formBoxLayout->addWidget (hdms);
  formBoxLayout->addWidget (hdm_);
  formBoxLayout->addWidget (hd__);
  formBoxLayout->addWidget (sdms);
  formBoxLayout->addWidget (sdm_);
  formBoxLayout->addWidget (sd__);

  formBoxLayout->addStretch (1);


  prefTab = new QTabWidget ();
  prefTab->setTabPosition (QTabWidget::North);

  prefTab->addTab (formBox, tr ("Position Format"));
  prefTab->setTabToolTip (0, tr ("Set the position display format"));
  prefTab->setTabWhatsThis (0, bGrpText);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);
  QVBoxLayout *cboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cboxRightLayout = new QVBoxLayout;
  cboxLayout->addLayout (cboxLeftLayout);
  cboxLayout->addLayout (cboxRightLayout);


  bTrackerColor = new QPushButton (tr ("Tracker"), this);
  bTrackerColor->setToolTip (tr ("Change tracker color"));
  bTrackerColor->setWhatsThis (trackerColorText);
  bTrackerPalette = bTrackerColor->palette ();
  connect (bTrackerColor, SIGNAL (clicked ()), this, SLOT (slotTrackerColor ()));
  cboxLeftLayout->addWidget (bTrackerColor);


  bFeatureColor = new QPushButton (tr ("Feature"), this);
  bFeatureColor->setToolTip (tr ("Change feature color"));
  bFeatureColor->setWhatsThis (featureColorText);
  bFeaturePalette = bFeatureColor->palette ();
  connect (bFeatureColor, SIGNAL (clicked ()), this, SLOT (slotFeatureColor ()));
  cboxLeftLayout->addWidget (bFeatureColor);


  bFeatureInfoColor = new QPushButton (tr ("Feature information"), this);
  bFeatureInfoColor->setToolTip (tr ("Change feature information text color"));
  bFeatureInfoColor->setWhatsThis (featureInfoColorText);
  bFeatureInfoPalette = bFeatureInfoColor->palette ();
  connect (bFeatureInfoColor, SIGNAL (clicked ()), this, SLOT (slotFeatureInfoColor ()));
  cboxLeftLayout->addWidget (bFeatureInfoColor);


  bFeaturePolyColor = new QPushButton (tr ("Feature polygon"), this);
  bFeaturePolyColor->setToolTip (tr ("Change feature polygonal area color"));
  bFeaturePolyColor->setWhatsThis (featurePolyColorText);
  bFeaturePolyPalette = bFeaturePolyColor->palette ();
  connect (bFeaturePolyColor, SIGNAL (clicked ()), this, SLOT (slotFeaturePolyColor ()));
  cboxLeftLayout->addWidget (bFeaturePolyColor);


  bHighFeatureColor = new QPushButton (tr ("Highlighted Feature"), this);
  bHighFeatureColor->setToolTip (tr ("Change highlighted feature color"));
  bHighFeatureColor->setWhatsThis (highFeatureColorText);
  bHighFeaturePalette = bHighFeatureColor->palette ();
  connect (bHighFeatureColor, SIGNAL (clicked ()), this, SLOT (slotHighFeatureColor ()));
  cboxLeftLayout->addWidget (bHighFeatureColor);


  bVerFeatureColor = new QPushButton (tr ("Verified Feature"), this);
  bVerFeatureColor->setToolTip (tr ("Change verified feature color"));
  bVerFeatureColor->setWhatsThis (verFeatureColorText);
  bVerFeaturePalette = bVerFeatureColor->palette ();
  connect (bVerFeatureColor, SIGNAL (clicked ()), this, SLOT (slotVerFeatureColor ()));
  cboxLeftLayout->addWidget (bVerFeatureColor);


  bContourColor = new QPushButton (tr ("Contour"), this);
  bContourColor->setToolTip (tr ("Change contour color"));
  bContourColor->setWhatsThis (contourColorText);
  bContourPalette = bContourColor->palette ();
  connect (bContourColor, SIGNAL (clicked ()), this, SLOT (slotContourColor ()));
  cboxLeftLayout->addWidget (bContourColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxLeftLayout->addWidget (bBackgroundColor);


  bRefColor = new QPushButton (tr ("Reference/NULL"), this);
  bRefColor->setToolTip (tr ("Change display reference and NULL data color"));
  bRefColor->setWhatsThis (refColorText);
  bRefPalette = bRefColor->palette ();
  connect (bRefColor, SIGNAL (clicked ()), this, SLOT (slotRefColor ()));
  cboxLeftLayout->addWidget (bRefColor);


  cGrp = new QButtonGroup (this);
  connect (cGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotColorClicked (int)));


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      QString string;
      string.sprintf (tr ("Cursor %02d").toAscii (), i);
      bWaveColor[i] = new QPushButton (string, this);
      string.sprintf (tr ("Change color for cursor %02d").toAscii (), i);
      bWaveColor[i]->setToolTip (string);
      bWaveColor[i]->setWhatsThis (waveColorText);
      bWavePalette[i] = bWaveColor[i]->palette ();

      cGrp->addButton (bWaveColor[i], i);

      cboxRightLayout->addWidget (bWaveColor[i]);
    }


  cboxLeftLayout->addStretch (1);
  cboxRightLayout->addStretch (1);


  prefTab->addTab (cbox, tr ("Colors"));
  prefTab->setTabToolTip (1, tr ("Set the display colors"));
  prefTab->setTabWhatsThis (1, colorText);


  QGroupBox *conBox = new QGroupBox (this);
  QHBoxLayout *conBoxLayout = new QHBoxLayout;
  conBox->setLayout (conBoxLayout);
  QVBoxLayout *conBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *conBoxRightLayout = new QVBoxLayout;
  conBoxLayout->addLayout (conBoxLeftLayout);
  conBoxLayout->addLayout (conBoxRightLayout);


  QGroupBox *intBox = new QGroupBox (tr ("Interval"), this);
  QHBoxLayout *intBoxLayout = new QHBoxLayout;
  intBox->setLayout (intBoxLayout);

  contourInt = new QLineEdit (intBox);
  contourInt->setToolTip (tr ("Change the contour interval"));
  contourInt->setWhatsThis (contourIntText);
  intBoxLayout->addWidget (contourInt);

  bContourLevels = new QPushButton (tr ("Set Contour Levels"), intBox);
  bContourLevels->setToolTip (tr ("Set non-uniform individual contour intervals"));
  bContourLevels->setWhatsThis (contourLevelsText);
  connect (bContourLevels, SIGNAL (clicked ()), this, SLOT (slotContourLevelsClicked ()));
  intBoxLayout->addWidget (bContourLevels);

  conBoxLeftLayout->addWidget (intBox);


  QGroupBox *smBox = new QGroupBox (tr ("Smoothing"), this);
  QHBoxLayout *smBoxLayout = new QHBoxLayout;
  smBox->setLayout (smBoxLayout);
  contourSm = new QSpinBox (smBox);
  contourSm->setRange (0, 10);
  contourSm->setSingleStep (1);
  contourSm->setToolTip (tr ("Change the contour smoothing factor (0 - 10)"));
  contourSm->setWhatsThis (contourSmText);
  smBoxLayout->addWidget (contourSm);
  conBoxLeftLayout->addWidget (smBox);


  QGroupBox *zBox = new QGroupBox (tr ("Scale"), this);
  QHBoxLayout *zBoxLayout = new QHBoxLayout;
  zBox->setLayout (zBoxLayout);
  ZFactor = new QComboBox (zBox);
  ZFactor->setToolTip (tr ("Change the Z scale factor"));
  ZFactor->setWhatsThis (ZFactorText);
  ZFactor->setEditable (TRUE);
  ZFactor->addItem ("1.00000 " + tr ("(no scaling)"));
  ZFactor->addItem ("3.28084 " + tr ("(meters to feet)"));
  ZFactor->addItem ("0.34080 " + tr ("(feet to meters)"));
  ZFactor->addItem ("0.54681 " + tr ("(meters to fathoms)"));
  ZFactor->addItem ("1.82880 " + tr ("(fathoms to meters)"));
  ZFactor->addItem ("0.53333 " + tr ("(meters to fathoms (at 4800 ft/sec)"));
  zBoxLayout->addWidget (ZFactor);
  conBoxLeftLayout->addWidget (zBox);


  QGroupBox *offBox = new QGroupBox (tr ("Offset"), this);
  QHBoxLayout *offBoxLayout = new QHBoxLayout;
  offBox->setLayout (offBoxLayout);
  offset = new QDoubleSpinBox (offBox);
  offset->setDecimals (2);
  offset->setRange (-1000.0, 1000.0);
  offset->setSingleStep (10.0);
  offset->setToolTip (tr ("Change the contour/Z offset value"));
  offset->setWhatsThis (offsetText);
  offBoxLayout->addWidget (offset);

  QPushButton *egmButton = new QPushButton (tr ("EGM08"));
  egmButton->setToolTip (tr ("Get the ellipsoid to geoid datum offset from EGM08 model"));
  egmButton->setWhatsThis (egmText);
  connect (egmButton, SIGNAL (clicked ()), this, SLOT (slotEgmClicked (void)));
  offBoxLayout->addWidget (egmButton);
  conBoxRightLayout->addWidget (offBox);

  QGroupBox *widBox = new QGroupBox (tr ("Contour width"), this);
  QHBoxLayout *widBoxLayout = new QHBoxLayout;
  widBox->setLayout (widBoxLayout);
  Width = new QComboBox (widBox);
  Width->setToolTip (tr ("Change the contour line width/thickness (pixels)"));
  Width->setWhatsThis (WidthText);
  Width->setEditable (TRUE);
  Width->addItem ("1");
  Width->addItem ("2");
  Width->addItem ("3");
  widBoxLayout->addWidget (Width);
  conBoxRightLayout->addWidget (widBox);

  QGroupBox *pixBox = new QGroupBox (tr ("Point size"), this);
  QHBoxLayout *pixBoxLayout = new QHBoxLayout;
  pixBox->setLayout (pixBoxLayout);
  pointSize = new QSpinBox (pixBox);
  pointSize->setRange (2, 10);
  pointSize->setSingleStep (1);
  pointSize->setToolTip (tr ("Change the point size (pixels) (2 - 10)"));
  pointSize->setWhatsThis (pointSizeText);
  pixBoxLayout->addWidget (pointSize);
  conBoxRightLayout->addWidget (pixBox);


  conBoxLeftLayout->addStretch (1);
  conBoxRightLayout->addStretch (1);


  prefTab->addTab (conBox, tr ("Contour/Depth"));
  prefTab->setTabToolTip (2, tr ("Set the contour/depth options"));
  prefTab->setTabWhatsThis (2, conText);


  QGroupBox *filterBox = new QGroupBox (this);
  QVBoxLayout *filterBoxLayout = new QVBoxLayout;
  filterBox->setLayout (filterBoxLayout);


  QGroupBox *sBox = new QGroupBox (tr ("Standard deviation"), this);
  QHBoxLayout *sBoxLayout = new QHBoxLayout;
  sBox->setLayout (sBoxLayout);
  filterSTD = new QDoubleSpinBox (sBox);
  filterSTD->setDecimals (1);
  filterSTD->setRange (0.3, 4.0);
  filterSTD->setSingleStep (0.1);
  filterSTD->setToolTip (tr ("Change the filter standard deviation"));
  filterSTD->setWhatsThis (filterSTDText);
  sBoxLayout->addWidget (filterSTD);
  filterBoxLayout->addWidget (sBox);

  QGroupBox *dBox = new QGroupBox (tr ("Deep filter only"), this);
  QHBoxLayout *dBoxLayout = new QHBoxLayout;
  dBox->setLayout (dBoxLayout);
  dFilter = new QCheckBox (dBox);
  dFilter->setToolTip (tr ("Toggle filtering of deep values only"));
  dFilter->setWhatsThis (dFilterText);
  dBoxLayout->addWidget (dFilter);
  filterBoxLayout->addWidget (dBox);

  QGroupBox *rBox = new QGroupBox (tr ("Feature exclusion radius"), this);
  QHBoxLayout *rBoxLayout = new QHBoxLayout;
  rBox->setLayout (rBoxLayout);
  featureRadius = new QDoubleSpinBox (rBox);
  featureRadius->setDecimals (1);
  featureRadius->setRange (0.0, 100.0);
  featureRadius->setSingleStep (1.0);
  featureRadius->setToolTip (tr ("Change the feature exclusion radius (m)"));
  featureRadius->setWhatsThis (featureRadiusText);
  rBoxLayout->addWidget (featureRadius);
  filterBoxLayout->addWidget (rBox);

  filterBoxLayout->addStretch (1);


  prefTab->addTab (filterBox, tr ("Filter"));
  prefTab->setTabToolTip (3, tr ("Set the statistical filter options"));
  prefTab->setTabWhatsThis (3, filtText);


  QGroupBox *miscBox = new QGroupBox (this);
  QHBoxLayout *miscBoxLayout = new QHBoxLayout;
  miscBox->setLayout (miscBoxLayout);
  QVBoxLayout *miscBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *miscBoxRightLayout = new QVBoxLayout;
  miscBoxLayout->addLayout (miscBoxLeftLayout);
  miscBoxLayout->addLayout (miscBoxRightLayout);


  QGroupBox *minBox = new QGroupBox (tr ("Minimum Z window size"), this);
  QHBoxLayout *minBoxLayout = new QHBoxLayout;
  minBox->setLayout (minBoxLayout);

  ihoOrder = new QComboBox (minBox);
  ihoOrder->setToolTip (tr ("Use IHO order to define minimum Z window size"));
  ihoOrder->setWhatsThis (ihoOrderText);
  ihoOrder->setEditable (TRUE);
  ihoOrder->addItem ("Set Manually ->");
  ihoOrder->addItem ("IHO Special Order");
  ihoOrder->addItem ("IHO Order 1a/1b");
  ihoOrder->addItem ("IHO Order 2");
  minBoxLayout->addWidget (ihoOrder);


  minZ = new QDoubleSpinBox (minBox);
  minZ->setDecimals (1);
  minZ->setRange (0.1, 100.0);
  minZ->setSingleStep (1.0);
  minZ->setToolTip (tr ("Manually set the minimum Z window size"));
  minZ->setWhatsThis (minZText);
  minBoxLayout->addWidget (minZ);

  miscBoxLeftLayout->addWidget (minBox);


  QGroupBox *sliceBox = new QGroupBox (tr ("Slice %"), this);
  QHBoxLayout *sliceBoxLayout = new QHBoxLayout;
  sliceBox->setLayout (sliceBoxLayout);
  slice = new QSpinBox (sliceBox);
  slice->setRange (1, 50);
  slice->setSingleStep (1);
  slice->setToolTip (tr ("Change the slice percentage (1 - 50)"));
  slice->setWhatsThis (sliceText);
  sliceBoxLayout->addWidget (slice);
  miscBoxLeftLayout->addWidget (sliceBox);


  QGroupBox *overlapBox = new QGroupBox (tr ("Overlap %"), this);
  QHBoxLayout *overlapBoxLayout = new QHBoxLayout;
  overlapBox->setLayout (overlapBoxLayout);
  overlap = new QSpinBox (overlapBox);
  overlap->setRange (1, 50);
  overlap->setSingleStep (1);
  overlap->setToolTip (tr ("Change the window overlap percentage (1 - 50)"));
  overlap->setWhatsThis (overlapText);
  overlapBoxLayout->addWidget (overlap);
  miscBoxLeftLayout->addWidget (overlapBox);


  QGroupBox *rotBox = new QGroupBox (tr ("Rotation"), this);
  QHBoxLayout *rotBoxLayout = new QHBoxLayout;
  rotBox->setLayout (rotBoxLayout);
  rot = new QDoubleSpinBox (rotBox);
  rot->setDecimals (1);
  rot->setRange (1.0, 30.0);
  rot->setSingleStep (1.0);
  rot->setToolTip (tr ("Change the view rotation increment (1.0 - 30.0)"));
  rot->setWhatsThis (rotText);
  rotBoxLayout->addWidget (rot);
  miscBoxRightLayout->addWidget (rotBox);


  QGroupBox *undoBox = new QGroupBox (tr ("Undo levels"), this);
  QHBoxLayout *undoBoxLayout = new QHBoxLayout;
  undoBox->setLayout (undoBoxLayout);
  undo = new QSpinBox (undoBox);
  undo->setRange (100, 1000000);
  undo->setSingleStep (100);
  undo->setToolTip (tr ("Change the maximum number of undo levels (100 - 1,000,000)"));
  undo->setWhatsThis (prefsUndoText);
  undoBoxLayout->addWidget (undo);
  miscBoxRightLayout->addWidget (undoBox);


  QGroupBox *killBox = new QGroupBox (tr ("Kill/Respawn"), this);
  QHBoxLayout *killBoxLayout = new QHBoxLayout;
  killBox->setLayout (killBoxLayout);
  kill = new QCheckBox (this);
  kill->setToolTip (tr ("If checked, ancillary programs will exit/respawn on close/open"));
  kill->setWhatsThis (killText);
  killBoxLayout->addWidget (kill);
  miscBoxRightLayout->addWidget (killBox);


  miscBoxLeftLayout->addStretch (1);
  miscBoxRightLayout->addStretch (1);


  prefTab->addTab (miscBox, tr ("Miscellaneous"));
  prefTab->setTabToolTip (4, tr ("Set miscellaneous options"));
  prefTab->setTabWhatsThis (4, miscText);



  QGroupBox *kbox = new QGroupBox (this);
  QVBoxLayout *kboxLayout = new QVBoxLayout;
  kbox->setLayout (kboxLayout);


  bHotKeys = new QPushButton (tr ("Main Buttons"), this);
  bHotKeys->setToolTip (tr ("Change hot key sequences for buttons"));
  bHotKeys->setWhatsThis (hotKeysText);
  connect (bHotKeys, SIGNAL (clicked ()), this, SLOT (slotHotKeys ()));
  kboxLayout->addWidget (bHotKeys);


  QGroupBox *iconBox = new QGroupBox (tr ("Main Button Icon Size"), this);
  QHBoxLayout *iconBoxLayout = new QHBoxLayout;
  iconBox->setLayout (iconBoxLayout);

  iconSize = new QComboBox (iconBox);
  iconSize->setToolTip (tr ("Set the size (in pixels) of the main button icons"));
  iconSize->setWhatsThis (iconSizeText);
  iconSize->setEditable (FALSE);
  iconSize->addItem ("16");
  iconSize->addItem ("20");
  iconSize->addItem ("24");
  iconSize->addItem ("28");
  iconSize->addItem ("32");
  iconBoxLayout->addWidget (iconSize);


  kboxLayout->addWidget (iconBox);


  bAncillaryPrograms = new QPushButton (tr ("Ancillary Programs"), this);
  bAncillaryPrograms->setToolTip (tr ("Change ancillary program hot keys and action keys"));
  bAncillaryPrograms->setWhatsThis (ancillaryProgramsText);
  connect (bAncillaryPrograms, SIGNAL (clicked ()), this, SLOT (slotAncillaryPrograms ()));
  kboxLayout->addWidget (bAncillaryPrograms);


  kboxLayout->addStretch (1);


  prefTab->addTab (kbox, tr ("Hot Keys"));
  prefTab->setTabToolTip (5, tr ("Set hot keys"));
  prefTab->setTabWhatsThis (5, kboxText);


  vbox->addWidget (prefTab);


  setFields ();


  //  Connect this here so that it doesn't cause the IHO combo to go to Manual every time.

  connect (minZ, SIGNAL (valueChanged (double)), this, SLOT (slotMinZValueChanged (double)));


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
Prefs::slotContourColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.contour_color, this, tr ("pfmEdit Contour Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.contour_color = clr;

  setFields ();
}



void
Prefs::slotTrackerColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.tracker_color, this, tr ("pfmEdit Tracker Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.tracker_color = clr;

  setFields ();
}



void
Prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("pfmEdit Background Color"));

  if (clr.isValid ()) mod_options.background_color = clr;

  setFields ();
}



void
Prefs::slotRefColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.ref_color[0], this, tr ("pfmEdit Reference Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.ref_color[0] = clr;

  setFields ();
}



void
Prefs::slotFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_color, this, tr ("pfmEdit Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_color = clr;

  setFields ();
}



void
Prefs::slotFeatureInfoColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_info_color, this, tr ("pfmEdit Feature Information Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_info_color = clr;

  setFields ();
}



void
Prefs::slotFeaturePolyColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_poly_color, this, tr ("pfmEdit Feature Polygon Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_poly_color = clr;

  setFields ();
}



void
Prefs::slotHighFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_highlight_color, this, tr ("pfmEdit Highlighted Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_highlight_color = clr;

  setFields ();
}



void
Prefs::slotVerFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.verified_feature_color, this, tr ("pfmEdit Verified Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.verified_feature_color = clr;

  setFields ();
}



void
Prefs::slotColorClicked (int id)
{
  QColor clr;

  QString name;

  name.sprintf (tr ("Waveform/Marker %d").toAscii (), id);

  clr = QColorDialog::getColor (mod_options.waveColor[id], this, tr ("pfmEdit ") + name + tr (" Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.waveColor[id] = clr;

  setFields ();
}



void
Prefs::slotAncillaryPrograms ()
{
  if (ancillaryProgramD) ancillaryProgramD->close ();


  ancillaryProgramD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  ancillaryProgramD->setWindowTitle (tr ("pfmEdit Ancillary Programs"));

  QVBoxLayout *vbox = new QVBoxLayout (ancillaryProgramD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *progBox = new QGroupBox (ancillaryProgramD);
  QHBoxLayout *progBoxLayout = new QHBoxLayout;
  progBox->setLayout (progBoxLayout);
  QVBoxLayout *progBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *progBoxRightLayout = new QVBoxLayout;
  progBoxLayout->addLayout (progBoxLeftLayout);
  progBoxLayout->addLayout (progBoxRightLayout);

  QGroupBox *programs[NUMPROGS];
  QHBoxLayout *programsLayout[NUMPROGS];

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      programs[i] = new QGroupBox (ancillaryProgramD);
      programsLayout[i] = new QHBoxLayout;
      programs[i]->setLayout (programsLayout[i]);
      
      program[i] = new QLineEdit (ancillaryProgramD);
      program[i]->setReadOnly (NVTrue);
      program[i]->setToolTip (options->description[i]);
      program[i]->setWhatsThis (programText);
      program[i]->setText (mod_options.name[i]);
      program[i]->setCursorPosition (0);
      programsLayout[i]->addWidget (program[i], 10);

      hotkey[i] = new QLineEdit (ancillaryProgramD);
      hotkey[i]->setToolTip (tr ("Hot key"));
      hotkey[i]->setWhatsThis (hotkeyText);
      hotkey[i]->setText (mod_options.hotkey[i]);
      hotkey[i]->setMinimumWidth (50);
      connect (hotkey[i], SIGNAL (textEdited (const QString &)), this, SLOT (slotApplyAncillaryChanges (const QString &)));
      programsLayout[i]->addWidget (hotkey[i], 1);

      actionkey[i] = new QLineEdit (ancillaryProgramD);
      actionkey[i]->setToolTip (tr ("Action key(s)"));
      actionkey[i]->setWhatsThis (actionkeyText);
      actionkey[i]->setText (mod_options.action[i]);
      actionkey[i]->setMinimumWidth (70);
      connect (actionkey[i], SIGNAL (textEdited (const QString &)), this, SLOT (slotApplyAncillaryChanges (const QString &)));
      programsLayout[i]->addWidget (actionkey[i], 1);


      if (!(i % 2))
        {
          progBoxLeftLayout->addWidget (programs[i]);
        }
      else
        {
          progBoxRightLayout->addWidget (programs[i]);
        }
    }


  vbox->addWidget (progBox);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (ancillaryProgramD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), ancillaryProgramD);
  closeButton->setToolTip (tr ("Close the ancillary program dialog"));
  closeButton->setWhatsThis (closeAncillaryProgramText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseAncillaryProgram ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  ancillaryProgramD->move (x () + width () / 2 - ancillaryProgramD->width () / 2, y () + height () / 2 - ancillaryProgramD->height () / 2);

  ancillaryProgramD->show ();
}



//!  Cheating again - any change will cause everything to update.  It only takes a second anyway.

void 
Prefs::slotApplyAncillaryChanges (const QString &text __attribute__ ((unused)))
{
  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      mod_options.hotkey[i] = hotkey[i]->text ();
      mod_options.action[i] = actionkey[i]->text ();
    }
}



void 
Prefs::slotCloseAncillaryProgram ()
{
  ancillaryProgramD->close ();
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


  misc->abe_share->cint = 5.0;
  misc->abe_share->num_levels = 0;


  //  Default contour levels

  misc->abe_share->num_levels = 52;
  misc->abe_share->contour_levels[0] = -30;
  misc->abe_share->contour_levels[1] = -20;
  misc->abe_share->contour_levels[2] = -10;
  misc->abe_share->contour_levels[3] = -5;
  misc->abe_share->contour_levels[4] = -2;
  misc->abe_share->contour_levels[5] = 0;
  misc->abe_share->contour_levels[6] = 2;
  misc->abe_share->contour_levels[7] = 5;
  misc->abe_share->contour_levels[8] = 10;
  misc->abe_share->contour_levels[9] = 20;
  misc->abe_share->contour_levels[10] = 30;
  misc->abe_share->contour_levels[11] = 40;
  misc->abe_share->contour_levels[12] = 50;
  misc->abe_share->contour_levels[13] = 60;
  misc->abe_share->contour_levels[14] = 70;
  misc->abe_share->contour_levels[15] = 80;
  misc->abe_share->contour_levels[16] = 90;
  misc->abe_share->contour_levels[17] = 100;
  misc->abe_share->contour_levels[18] = 120;
  misc->abe_share->contour_levels[19] = 130;
  misc->abe_share->contour_levels[20] = 140;
  misc->abe_share->contour_levels[21] = 160;
  misc->abe_share->contour_levels[22] = 180;
  misc->abe_share->contour_levels[23] = 200;
  misc->abe_share->contour_levels[24] = 250;
  misc->abe_share->contour_levels[25] = 300;
  misc->abe_share->contour_levels[26] = 350;
  misc->abe_share->contour_levels[27] = 400;
  misc->abe_share->contour_levels[28] = 450;
  misc->abe_share->contour_levels[29] = 500;
  misc->abe_share->contour_levels[30] = 600;
  misc->abe_share->contour_levels[31] = 700;
  misc->abe_share->contour_levels[32] = 800;
  misc->abe_share->contour_levels[33] = 900;
  misc->abe_share->contour_levels[34] = 1000;
  misc->abe_share->contour_levels[35] = 1500;
  misc->abe_share->contour_levels[36] = 2000;
  misc->abe_share->contour_levels[37] = 2500;
  misc->abe_share->contour_levels[38] = 3000;
  misc->abe_share->contour_levels[39] = 3500;
  misc->abe_share->contour_levels[40] = 4000;
  misc->abe_share->contour_levels[41] = 4500;
  misc->abe_share->contour_levels[42] = 5000;
  misc->abe_share->contour_levels[43] = 5500;
  misc->abe_share->contour_levels[44] = 6000;
  misc->abe_share->contour_levels[45] = 6500;
  misc->abe_share->contour_levels[46] = 7000;
  misc->abe_share->contour_levels[47] = 7500;
  misc->abe_share->contour_levels[48] = 8000;
  misc->abe_share->contour_levels[49] = 8500;
  misc->abe_share->contour_levels[50] = 9000;
  misc->abe_share->contour_levels[51] = 10000;


  mod_options = *options;
  mod_share = *misc->abe_share;

  setFields ();


  misc->GeoTIFF_init = NVTrue;


  *misc->abe_share = mod_share;
  *options = mod_options;


  //  Let the calling program (pfmView) know that we've changed some things that it uses.

  if (misc->abe_share != NULL)
    {
      misc->abe_share->settings_changed = NVTrue;
      misc->abe_share->position_form = options->position_form;
      misc->abe_share->smoothing_factor = options->smoothing_factor;
      misc->abe_share->z_factor = options->z_factor;
      misc->abe_share->z_offset = options->z_offset;


      //  Save the mosaic viewer program name and options.

      for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
        {
          if (options->prog[i].contains ("mosaicView"))
            {
              strcpy (misc->abe_share->mosaic_prog, options->prog[i].toAscii ());
              strcpy (misc->abe_share->mosaic_actkey, options->action[i].toAscii ());
              strcpy (misc->abe_share->mosaic_hotkey, options->hotkey[i].toAscii ());

              break;
            }
        }


      misc->abe_share->settings_changed = NVTrue;
    }


  emit dataChangedSignal ();

  close ();
}



void
Prefs::slotPositionClicked (int id)
{
  mod_options.position_form = id;
}



void
Prefs::slotCloseContours ()
{
  //  Get the values.

  QString string;
  NV_INT32 status, j;
  NV_FLOAT32 tmp;

  j = 0;
  for (NV_INT32 i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      string = contoursTable->item (i, 0)->text ();
      status = sscanf (string.toAscii (), "%f", &tmp);
      if (status == 1)
        {
          mod_share.contour_levels[j] = tmp;
          j++;
        }
    }

  mod_share.num_levels = j + 1;

  contoursD->close ();
}



void
Prefs::slotContourLevelsClicked ()
{
  if (contoursD) slotCloseContours ();


  QString string;

  mod_share.cint = 0.0;
  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  
  contoursD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  contoursD->setWindowTitle (tr ("pfmEdit Contour Levels"));

  QVBoxLayout *vbox = new QVBoxLayout (contoursD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  contoursTable = new QTableWidget (MAX_CONTOUR_LEVELS, 1, this);
  contoursTable->setWhatsThis (contoursTableText);
  contoursTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *chItem = new QTableWidgetItem (tr ("Contour level"));
  contoursTable->setHorizontalHeaderItem (0, chItem);

  QTableWidgetItem *cntItem[MAX_CONTOUR_LEVELS];

  for (NV_INT32 i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      if (i < mod_share.num_levels)
        {
          string.sprintf ("%f", mod_share.contour_levels[i]);
        }
      else
        {
          string.sprintf (" ");
        }
      cntItem[i] = new QTableWidgetItem (string); 
      contoursTable->setItem (i, 0, cntItem[i]);
    }
  contoursTable->resizeColumnsToContents ();


  vbox->addWidget (contoursTable, 1);

  NV_INT32 w = qMin (800, contoursTable->columnWidth (0) + 60);
  contoursD->resize (w, 600);

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (contoursD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), contoursD);
  closeButton->setToolTip (tr ("Close the contour levels dialog"));
  closeButton->setWhatsThis (closeContoursText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseContours ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  contoursD->move (x () + width () / 2 - contoursD->width () / 2, y () + height () / 2 - contoursD->height () / 2);

  contoursD->show ();
}



void 
Prefs::slotGeotiff ()
{
  if (geotiffD) geotiffD->close ();


  geotiffD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  geotiffD->setWindowTitle (tr ("pfmEdit GeoTIFF transparency"));


  QImage tmp_layer1 = QImage (":/icons/average.jpg");
  layer1 = tmp_layer1.convertToFormat (QImage::Format_ARGB32);

  geotiff_w = layer1.width ();
  geotiff_h = layer1.height ();


  QImage tmp_layer2 = QImage (":/icons/geotiff.jpg");
  layer2 = tmp_layer2.convertToFormat (QImage::Format_ARGB32);


  geotiffD->resize (geotiff_w + 20, geotiff_h + 120);



  QVBoxLayout *vbox = new QVBoxLayout (geotiffD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QFrame *frame = new QFrame (geotiffD);
  frame->setFrameStyle (QFrame::Panel | QFrame::Sunken);


  //  Seriously cheating here - I'm just using the map class so I don't have to make a special widget just
  //  to paint the demo part of the transparency dialog.

  NVMAP_DEF mapdef;
  mapdef.projection = 3;
  mapdef.draw_width = geotiff_w;
  mapdef.draw_height = geotiff_h;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;
 
  mapdef.border = 0;
  mapdef.background_color = Qt::white;
  mapdef.landmask = NVFalse;
  mapdef.coasts = NVFalse;


  geotiff = new nvMap (geotiffD, &mapdef);
  connect (geotiff, SIGNAL (preRedrawSignal (NVMAP_DEF)), this, SLOT (slotPreRedraw (NVMAP_DEF)));

  geotiff->setWhatsThis (transparencyText);


  vbox->addWidget (frame);


  QVBoxLayout *fBox = new QVBoxLayout (frame);


  QHBoxLayout *hBox = new QHBoxLayout ();
  hBox->addWidget (geotiff);
  fBox->addLayout (hBox);



  QGroupBox *sbox = new QGroupBox (tr ("GeoTIFF transparency"), geotiffD);
  QHBoxLayout *sboxLayout = new QHBoxLayout;
  sbox->setLayout (sboxLayout);

  QString tmp;
  tmp.sprintf ("%03d", mod_options.GeoTIFF_alpha);
  geotiff_left = new QLabel (tmp, geotiffD);
  geotiff_left->setWhatsThis (transparencyText);
  sboxLayout->addWidget (geotiff_left);

  geotiffScroll = new QScrollBar (Qt::Horizontal, geotiffD);
  geotiffScroll->setRange (25, 255);
  geotiffScroll->setSliderPosition (mod_options.GeoTIFF_alpha);
  geotiffScroll->setTracking (NVFalse);
  connect (geotiffScroll, SIGNAL (valueChanged (int)), this, SLOT (slotGeotiffScrollValueChanged (int)));
  connect (geotiffScroll, SIGNAL (sliderMoved (int)), this, SLOT (slotGeotiffScrollSliderMoved (int)));
  geotiffScroll->setToolTip (tr ("Move to change the amount of transparency in the GeoTIFF overlay"));
  geotiffScroll->setWhatsThis (transparencyText);
  sboxLayout->addWidget (geotiffScroll, 10);

  QLabel *right = new QLabel (tr ("Opaque"), geotiffD);
  right->setWhatsThis (transparencyText);
  sboxLayout->addWidget (right);


  vbox->addWidget (sbox);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (geotiffD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), geotiffD);
  closeButton->setToolTip (tr ("Close the GeoTIFF transparancy dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseGeotiff ()));
  actions->addWidget (closeButton);


  geotiffD->show ();

  geotiff->enableSignals ();

  geotiff->redrawMap (NVTrue);
}



void 
Prefs::slotPreRedraw (NVMAP_DEF mapdef __attribute__ ((unused)))
{
  QColor pixel;


  for (NV_INT32 i = 0 ; i < geotiff_w ; i++)
    {
      for (NV_INT32 j = 0 ; j < geotiff_h ; j++)
        {
          pixel = QColor (layer2.pixel (i, j));
          pixel.setAlpha (mod_options.GeoTIFF_alpha);
          layer2.setPixel (i, j, pixel.rgba ());
        }
    }

  QPixmap lay1 = QPixmap::fromImage (layer1);
  QPixmap lay2 = QPixmap::fromImage (layer2);

  geotiff->drawPixmap (0, 0, &lay1, 0, 0, geotiff_w, geotiff_h, NVTrue);
  geotiff->drawPixmap (0, 0, &lay2, 0, 0, geotiff_w, geotiff_h, NVTrue);

  geotiff->setCursor (Qt::ArrowCursor);
}



void 
Prefs::slotGeotiffScrollValueChanged (int value)
{
  mod_options.GeoTIFF_alpha = value;

  geotiff->redrawMap (NVTrue);
}



void 
Prefs::slotGeotiffScrollSliderMoved (int value)
{
  QString tmp;
  tmp.sprintf ("%03d", value);
  geotiff_left->setText (tmp);
}



void 
Prefs::slotCloseGeotiff ()
{
  geotiffD->close ();
}



void 
Prefs::slotHotKeys ()
{
  if (hotKeyD) slotCloseHotKeys ();


  hotKeyD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  hotKeyD->setWindowTitle (tr ("pfmEdit Hot Keys"));

  QVBoxLayout *vbox = new QVBoxLayout (hotKeyD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  hotKeyTable = new QTableWidget (HOTKEYS, 2, this);
  hotKeyTable->setWhatsThis (hotKeyTableText);
  hotKeyTable->setAlternatingRowColors (TRUE);
  QTableWidgetItem *bItemH = new QTableWidgetItem (tr ("Button"));
  hotKeyTable->setHorizontalHeaderItem (0, bItemH);
  QTableWidgetItem *kItemH = new QTableWidgetItem (tr ("Key"));
  hotKeyTable->setHorizontalHeaderItem (1, kItemH);

  QTableWidgetItem *bItem[HOTKEYS], *kItem[HOTKEYS];


  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      bItem[i] = new QTableWidgetItem (misc->buttonIcon[i], misc->buttonText[i]);
      bItem[i]->setFlags (Qt::ItemIsEnabled);

      hotKeyTable->setItem (i, 0, bItem[i]);
      kItem[i] = new QTableWidgetItem (mod_options.buttonAccel[i]);
      hotKeyTable->setItem (i, 1, kItem[i]);
    }
  hotKeyTable->resizeColumnsToContents ();
  hotKeyTable->resizeRowsToContents ();

  vbox->addWidget (hotKeyTable, 1);

  NV_INT32 w = qMin (800, hotKeyTable->columnWidth (0) + hotKeyTable->columnWidth (1) + 40);
  NV_INT32 h = qMin (600, hotKeyTable->rowHeight (0) * HOTKEYS + 75);
  hotKeyD->resize (w, h);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (hotKeyD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), hotKeyD);
  closeButton->setToolTip (tr ("Close the hot key dialog"));
  closeButton->setWhatsThis (closeHotKeyText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseHotKeys ()));
  actions->addWidget (closeButton);


  //  Put the dialog in the middle of the screen.

  hotKeyD->move (x () + width () / 2 - hotKeyD->width () / 2, y () + height () / 2 - hotKeyD->height () / 2);

  hotKeyD->show ();
}



void 
Prefs::slotCloseHotKeys ()
{
  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++) mod_options.buttonAccel[i] = hotKeyTable->item (i, 1)->text ();

  hotKeyD->close ();
}



void 
Prefs::slotEgmClicked ()
{
  NV_FLOAT64 lat, lon;

  lat = misc->abe_share->edit_area.min_y + (misc->abe_share->edit_area.max_y - misc->abe_share->edit_area.min_y) / 2.0;
  lon = misc->abe_share->edit_area.min_x + (misc->abe_share->edit_area.max_x - misc->abe_share->edit_area.min_x) / 2.0;

  qApp->setOverrideCursor (Qt::WaitCursor);
  qApp->processEvents ();

  NV_FLOAT32 datum_offset = get_egm08 (lat, lon);
  cleanup_egm08 ();

  qApp->restoreOverrideCursor ();

  if (datum_offset < 999999.0) 
    {
      offset->setValue ((NV_FLOAT64) datum_offset);
    }
  else
    {
      QMessageBox::warning (this, tr ("Get EGM08 datum offset"), 
                            tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
    }
}



void 
Prefs::slotMinZValueChanged (double value __attribute__ ((unused)))
{
  //  We manually modified the Z value so we want to set the IHO combo box to "Manual".

  ihoOrder->setCurrentIndex (0);
}



void 
Prefs::setFields ()
{
  QString string;

  bGrp->button (mod_options.position_form)->setChecked (TRUE);


  NV_INT32 hue, sat, val;

  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      mod_options.waveColor[i].getHsv (&hue, &sat, &val);
      if (val < 128)
        {
          bWavePalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
          bWavePalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
        }
      else
        {
          bWavePalette[i].setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          bWavePalette[i].setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
        }
      bWavePalette[i].setColor (QPalette::Normal, QPalette::Button, mod_options.waveColor[i]);
      bWavePalette[i].setColor (QPalette::Inactive, QPalette::Button, mod_options.waveColor[i]);
      bWaveColor[i]->setPalette (bWavePalette[i]);
    }


  mod_options.contour_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bContourPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bContourPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bContourPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bContourPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bContourPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.contour_color);
  bContourPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.contour_color);
  bContourColor->setPalette (bContourPalette);


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


  mod_options.ref_color[0].getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bRefPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bRefPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bRefPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bRefPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bRefPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.ref_color[0]);
  bRefPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.ref_color[0]);
  bRefColor->setPalette (bRefPalette);


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


  mod_options.feature_poly_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bFeaturePolyPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_poly_color);
  bFeaturePolyPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_poly_color);
  bFeaturePolyColor->setPalette (bFeaturePolyPalette);


  mod_options.feature_highlight_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bHighFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bHighFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bHighFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bHighFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bHighFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.feature_highlight_color);
  bHighFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.feature_highlight_color);
  bHighFeatureColor->setPalette (bHighFeaturePalette);


  mod_options.verified_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bVerFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.verified_feature_color);
  bVerFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.verified_feature_color);
  bVerFeatureColor->setPalette (bVerFeaturePalette);


  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  contourSm->setValue (mod_options.smoothing_factor);

  string.sprintf ("%.5f", mod_options.z_factor);
  ZFactor->lineEdit ()->setText (string);

  offset->setValue (mod_options.z_offset);

  string.sprintf ("%d", mod_options.contour_width);
  Width->lineEdit ()->setText (string);

  pointSize->setValue (mod_options.point_size);

  minZ->setValue (mod_options.min_window_size);

  ihoOrder->setCurrentIndex (mod_options.iho_min_window);

  filterSTD->setValue (mod_options.filterSTD);

  dFilter->setChecked (mod_options.deep_filter_only);

  featureRadius->setValue (mod_options.feature_radius);


  switch (mod_options.main_button_icon_size)
    {
    case 16:
      iconSize->setCurrentIndex (0);
      break;

    case 20:
      iconSize->setCurrentIndex (1);
      break;

    case 24:
    default:
      iconSize->setCurrentIndex (2);
      break;

    case 28:
      iconSize->setCurrentIndex (3);
      break;

    case 32:
      iconSize->setCurrentIndex (4);
      break;
    }

      
  slice->setValue (mod_options.slice_percent);
  rot->setValue (mod_options.rotation_increment);
  overlap->setValue (mod_options.overlap_percent);
  undo->setValue (mod_options.undo_levels);
  kill->setChecked (mod_options.kill_and_respawn);
}



void
Prefs::slotApplyPrefs ()
{
  NV_INT32 tmp_i, status;
  NV_FLOAT32 tmp_f;


  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;


  QString tmp = contourInt->text ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_share.cint = tmp_f;
  if (mod_share.cint != misc->abe_share->cint) dataChanged = NVTrue;

  tmp = contourSm->text ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.smoothing_factor = tmp_i;
  if (mod_options.smoothing_factor != options->smoothing_factor) dataChanged = NVTrue;


  tmp = ZFactor->currentText ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_options.z_factor = tmp_f;
  if (mod_options.z_factor != options->z_factor) dataChanged = NVTrue;


  tmp = offset->text ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_options.z_offset = tmp_f;
  if (mod_options.z_offset != options->z_offset) dataChanged = NVTrue;


  tmp = Width->currentText ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.contour_width = tmp_i;
  if (mod_options.contour_width != options->contour_width) dataChanged = NVTrue;


  tmp = pointSize->text ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.point_size = tmp_i;
  if (mod_options.point_size != options->point_size) dataChanged = NVTrue;


  mod_options.min_window_size = minZ->value ();
  if (mod_options.min_window_size != options->min_window_size) dataChanged = NVTrue;

  mod_options.iho_min_window = ihoOrder->currentIndex ();
  if (mod_options.iho_min_window != options->iho_min_window) dataChanged = NVTrue;

  mod_options.filterSTD = filterSTD->value ();
  if (mod_options.filterSTD != options->filterSTD) dataChanged = NVTrue;

  mod_options.deep_filter_only = dFilter->isChecked ();
  if (mod_options.deep_filter_only != options->deep_filter_only) dataChanged = NVTrue;

  mod_options.feature_radius = featureRadius->value ();
  if (mod_options.feature_radius != options->feature_radius) dataChanged = NVTrue;

  switch (iconSize->currentIndex ())
    {
    case 0:
      mod_options.main_button_icon_size = 16;
      break;

    case 1:
      mod_options.main_button_icon_size = 20;
      break;

    case 2:
      mod_options.main_button_icon_size = 24;
      break;

    case 3:
      mod_options.main_button_icon_size = 28;
      break;

    case 4:
      mod_options.main_button_icon_size = 32;
      break;
    }
  if (mod_options.main_button_icon_size != options->main_button_icon_size) dataChanged = NVTrue;

  mod_options.slice_percent = slice->value ();
  if (mod_options.slice_percent != options->slice_percent) dataChanged = NVTrue;

  mod_options.rotation_increment = rot->value ();
  if (mod_options.rotation_increment != options->rotation_increment) dataChanged = NVTrue;

  mod_options.overlap_percent = overlap->value ();
  if (mod_options.overlap_percent != options->overlap_percent) dataChanged = NVTrue;

  mod_options.undo_levels = undo->value ();
  if (mod_options.undo_levels != options->undo_levels && resize_undo (misc, options, mod_options.undo_levels)) dataChanged = NVTrue;

  mod_options.kill_and_respawn = kill->isChecked ();
  if (mod_options.kill_and_respawn != options->kill_and_respawn) dataChanged = NVTrue;

  if (mod_options.GeoTIFF_alpha != options->GeoTIFF_alpha)
    {
      dataChanged = NVTrue;
      misc->GeoTIFF_init = NVTrue;
    }

  if (mod_options.contour_color != options->contour_color) dataChanged = NVTrue;
  if (mod_options.tracker_color != options->tracker_color) dataChanged = NVTrue;
  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.ref_color[0] != options->ref_color[0]) dataChanged = NVTrue;
  if (mod_options.feature_color != options->feature_color) dataChanged = NVTrue;
  if (mod_options.feature_info_color != options->feature_info_color) dataChanged = NVTrue;
  if (mod_options.feature_poly_color != options->feature_poly_color) dataChanged = NVTrue;
  if (mod_options.verified_feature_color != options->verified_feature_color) dataChanged = NVTrue;


  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      if (mod_options.waveColor[i] != options->waveColor[i])
        {
          mod_share.mwShare.multiColors[i].r = mod_options.waveColor[i].red ();
          mod_share.mwShare.multiColors[i].g = mod_options.waveColor[i].green ();
          mod_share.mwShare.multiColors[i].b = mod_options.waveColor[i].blue ();
          mod_share.mwShare.multiColors[i].a = mod_options.waveColor[i].alpha ();
          dataChanged = NVTrue;
        }
    }


  //  Make sure we have no hotkey duplications.  First, the buttons and programs against hard-wired keys (just ESC at the moment).

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      if (mod_options.buttonAccel[i].toUpper () == "ESC")
        {
          QMessageBox::warning (0, tr ("pfmEdit3D preferences"), tr ("Esc is not allowed as a hot key.  It is reserved for disabling slice mode."));

          mod_options.buttonAccel[i] = options->buttonAccel[i];
          dataChanged = NVFalse;

          hotKeyD->close ();
          hotKeyD = NULL;

          return;
        }
    }

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      if (mod_options.hotkey[i].toUpper () == "ESC")
        {
          QMessageBox::warning (0, tr ("pfmEdit3D preferences"), tr ("Esc is not allowed as a hot key.  It is reserved for disabling slice mode."));

          mod_options.hotkey[i] = options->hotkey[i];
          dataChanged = NVFalse;

          ancillaryProgramD->close ();
          ancillaryProgramD = NULL;

          return;
        }
    }


  //  Next, the buttons against the buttons.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (mod_options.buttonAccel[i].toUpper () == mod_options.buttonAccel[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("pfmEdit preferences"),
                                    tr ("Hot key for <b>%1</b> button conflicts with hotkey for <b>%2</b> button!<br><br>").arg 
                                    (misc->buttonText[i]).arg (misc->buttonText[j]) +
                                    tr ("Resetting button hotkeys for these two buttons."));

              mod_options.buttonAccel[i] = options->buttonAccel[i];
              mod_options.buttonAccel[j] = options->buttonAccel[j];
              dataChanged = NVFalse;

              hotKeyD->close ();
              hotKeyD = NULL;

              return;
            }
        }
    }


  //  Next, the programs against the programs.

  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < NUMPROGS ; j++)
        {
          if (mod_options.hotkey[i].toUpper () == mod_options.hotkey[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("pfmEdit preferences"),
                                    tr ("Hot key for <b>%1</b> program conflicts with hotkey for <b>%2</b> program!<br><br>").arg
                                    (options->name[i]).arg (options->name[j]) +
                                    tr ("Resetting program hotkeys for these two programs."));

              mod_options.hotkey[i] = options->hotkey[i];
              mod_options.hotkey[j] = options->hotkey[j];
              dataChanged = NVFalse;

              ancillaryProgramD->close ();
              ancillaryProgramD = NULL;

              return;
            }
        }
    }


  //  Finally, the buttons against the programs.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = 0 ; j < NUMPROGS ; j++)
        {
          if (mod_options.buttonAccel[i].toUpper () == mod_options.hotkey[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("pfmEdit preferences"),
                                    tr ("Hot key for <b>%1</b> button conflicts with hotkey for <b>%2</b> program!<br><br>").arg
                                    (misc->buttonText[i]).arg (options->name[j]) +
                                    tr ("Resetting button and program hotkeys."));

              mod_options.buttonAccel[i] = options->buttonAccel[i];
              mod_options.hotkey[j] = options->hotkey[j];
              dataChanged = NVFalse;

              hotKeyD->close ();
              hotKeyD = NULL;
              ancillaryProgramD->close ();
              ancillaryProgramD = NULL;

              return;
            }
        }
    }


  //  Now check for changes to the hotkeys.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      if (mod_options.buttonAccel[i] != options->buttonAccel[i])
        {
          dataChanged = NVTrue;
          emit hotKeyChangedSignal (i);
          break;
        }
    }


  for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
    {
      if (mod_options.hotkey[i] != options->hotkey[i] ||
          mod_options.action[i] != options->action[i])
        {
          dataChanged = NVTrue;
          break;
        }

      for (NV_INT32 j = 0 ; j < PFM_DATA_TYPES ; j++)
        {
          if (mod_options.data_type[i][j] != options->data_type[i][j])
            {
              dataChanged = NVTrue;
              break;
            }
        }

      if (dataChanged) break;
    }


  if (mod_share.num_levels != misc->abe_share->num_levels) dataChanged = NVTrue;


  for (NV_INT32 i = 0 ; i < MAX_CONTOUR_LEVELS ; i++)
    {
      if (mod_share.contour_levels[i] != misc->abe_share->contour_levels[i])
        {
          dataChanged = NVTrue;
          break;
        }
    }


  setFields ();


  hide ();


  if (dataChanged)
    {
      *misc->abe_share = mod_share;
      *options = mod_options;


      //  Let the calling program (pfmView) know that we've changed some things that it uses.

      if (misc->abe_share != NULL)
        {
          misc->abe_share->settings_changed = NVTrue;
          misc->abe_share->position_form = options->position_form;
          misc->abe_share->smoothing_factor = options->smoothing_factor;
          misc->abe_share->z_factor = options->z_factor;
          misc->abe_share->z_offset = options->z_offset;


          //  Save the mosaic viewer program name and options.

          for (NV_INT32 i = 0 ; i < NUMPROGS ; i++)
            {
              if (options->prog[i].contains ("[MOSAIC FILE]"))
                {
                  strcpy (misc->abe_share->mosaic_prog, options->prog[i].toAscii ());
                  strcpy (misc->abe_share->mosaic_actkey, options->action[i].toAscii ());
                  strcpy (misc->abe_share->mosaic_hotkey, options->hotkey[i].toAscii ());

                  break;
                }
            }

          misc->abe_share->settings_changed = NVTrue;
        }


      emit dataChangedSignal ();
    }

  close ();
}



void
Prefs::slotClosePrefs ()
{
  close ();
}
