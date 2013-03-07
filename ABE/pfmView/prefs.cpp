
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include "prefs.hpp"
#include "prefsHelp.hpp"


/*!
  This is the preferences dialog.  There are about a bazillion (that's a technical term) different
  options here.  Hopefully the code is easy to follow.
*/

Prefs::Prefs (QWidget * parent, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;

  mod_share = *misc->abe_share;
  mod_options = *op;
  mod_misc = *mi;


  contoursD = geotiffD = cubeD = hotKeyD = NULL;


  setModal (TRUE);


  //  See if any of the layers have cube attributes.

  NV_BOOL cube_attr_available = NVFalse;
  for (NV_INT32 pfm = 0 ; pfm < misc->abe_share->pfm_count ; pfm++)
    {
      if (misc->cube_attr_available[pfm])
	{
	  cube_attr_available = NVTrue;
	  break;
	}
    }


  geotiffD = NULL;


  dataChanged = NVFalse;


  setWindowTitle (tr ("pfmView Preferences"));

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


  QGroupBox *cbox = new QGroupBox (this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);
  QVBoxLayout *cboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cboxRightLayout = new QVBoxLayout;
  cboxLayout->addLayout (cboxLeftLayout);
  cboxLayout->addLayout (cboxRightLayout);


  bContourColor = new QPushButton (tr ("Contour"), this);
  bContourColor->setToolTip (tr ("Change contour color"));
  bContourColor->setWhatsThis (contourColorText);
  bContourPalette = bContourColor->palette ();
  connect (bContourColor, SIGNAL (clicked ()), this, SLOT (slotContourColor ()));
  cboxLeftLayout->addWidget (bContourColor);


  bCoastColor = new QPushButton (tr ("Coast"), this);
  bCoastColor->setToolTip (tr ("Change coastline color"));
  bCoastColor->setWhatsThis (coastColorText);
  bCoastPalette = bCoastColor->palette ();
  connect (bCoastColor, SIGNAL (clicked ()), this, SLOT (slotCoastColor ()));
  cboxRightLayout->addWidget (bCoastColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxLeftLayout->addWidget (bBackgroundColor);


  bLandmaskColor = new QPushButton (tr ("Landmask"), this);
  bLandmaskColor->setToolTip (tr ("Change landmask color"));
  bLandmaskColor->setWhatsThis (landmaskColorText);
  bLandmaskPalette = bLandmaskColor->palette ();
  connect (bLandmaskColor, SIGNAL (clicked ()), this, SLOT (slotLandmaskColor ()));
  cboxRightLayout->addWidget (bLandmaskColor);


  bCovFeatureColor = new QPushButton (tr ("Coverage Feature"), this);
  bCovFeatureColor->setToolTip (tr ("Change valid coverage feature color"));
  bCovFeatureColor->setWhatsThis (covFeatureColorText);
  bCovFeaturePalette = bCovFeatureColor->palette ();
  connect (bCovFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovFeatureColor ()));
  cboxLeftLayout->addWidget (bCovFeatureColor);


  bCovInvFeatureColor = new QPushButton (tr ("Invalid Coverage Feature"), this);
  bCovInvFeatureColor->setToolTip (tr ("Change invalidated coverage feature color"));
  bCovInvFeatureColor->setWhatsThis (covInvFeatureColorText);
  bCovInvFeaturePalette = bCovInvFeatureColor->palette ();
  connect (bCovInvFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovInvFeatureColor ()));
  cboxRightLayout->addWidget (bCovInvFeatureColor);


  bCovVerFeatureColor = new QPushButton (tr ("Verified Coverage Feature"), this);
  bCovVerFeatureColor->setToolTip (tr ("Change verified coverage feature color"));
  bCovVerFeatureColor->setWhatsThis (covVerFeatureColorText);
  bCovVerFeaturePalette = bCovVerFeatureColor->palette ();
  connect (bCovVerFeatureColor, SIGNAL (clicked ()), this, SLOT (slotCovVerFeatureColor ()));
  cboxLeftLayout->addWidget (bCovVerFeatureColor);


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
  cboxRightLayout->addWidget (bFeatureInfoColor);


  bFeaturePolyColor = new QPushButton (tr ("Feature polygon"), this);
  bFeaturePolyColor->setToolTip (tr ("Change feature polygonal area color"));
  bFeaturePolyColor->setWhatsThis (featurePolyColorText);
  bFeaturePolyPalette = bFeaturePolyColor->palette ();
  connect (bFeaturePolyColor, SIGNAL (clicked ()), this, SLOT (slotFeaturePolyColor ()));
  cboxLeftLayout->addWidget (bFeaturePolyColor);


  bVerFeatureColor = new QPushButton (tr ("Verified Feature"), this);
  bVerFeatureColor->setToolTip (tr ("Change verified feature color"));
  bVerFeatureColor->setWhatsThis (verFeatureColorText);
  bVerFeaturePalette = bVerFeatureColor->palette ();
  connect (bVerFeatureColor, SIGNAL (clicked ()), this, SLOT (slotVerFeatureColor ()));
  cboxLeftLayout->addWidget (bVerFeatureColor);


  bContourHighlightColor = new QPushButton (tr ("Highlight"), this);
  bContourHighlightColor->setToolTip (tr ("Change contour/feature highlight color"));
  bContourHighlightColor->setWhatsThis (contourHighlightColorText);
  bContourHighlightPalette = bContourHighlightColor->palette ();
  connect (bContourHighlightColor, SIGNAL (clicked ()), this, SLOT (slotContourHighlightColor ()));
  cboxRightLayout->addWidget (bContourHighlightColor);


  bMaskColor = new QPushButton (tr ("Filter Mask"), this);
  bMaskColor->setToolTip (tr ("Change filter mask color"));
  bMaskColor->setWhatsThis (maskColorText);
  bMaskPalette = bMaskColor->palette ();
  connect (bMaskColor, SIGNAL (clicked ()), this, SLOT (slotMaskColor ()));
  cboxRightLayout->addWidget (bMaskColor);


  bStoplightMinColor = new QPushButton (tr ("Stoplight Minimum"), this);
  bStoplightMinColor->setToolTip (tr ("Change stoplight minimum color"));
  bStoplightMinColor->setWhatsThis (stoplightMinColorText);
  bStoplightMinPalette = bStoplightMinColor->palette ();
  connect (bStoplightMinColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMinColor ()));
  cboxLeftLayout->addWidget (bStoplightMinColor);

  bStoplightMidColor = new QPushButton (tr ("Stoplight Mid"), this);
  bStoplightMidColor->setToolTip (tr ("Change stoplight mid color"));
  bStoplightMidColor->setWhatsThis (stoplightMidColorText);
  bStoplightMidPalette = bStoplightMidColor->palette ();
  connect (bStoplightMidColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMidColor ()));
  cboxRightLayout->addWidget (bStoplightMidColor);

  bStoplightMaxColor = new QPushButton (tr ("Stoplight Maximum"), this);
  bStoplightMaxColor->setToolTip (tr ("Change stoplight maximum color"));
  bStoplightMaxColor->setWhatsThis (stoplightMaxColorText);
  bStoplightMaxPalette = bStoplightMaxColor->palette ();
  connect (bStoplightMaxColor, SIGNAL (clicked ()), this, SLOT (slotStoplightMaxColor ()));
  cboxRightLayout->addWidget (bStoplightMaxColor);


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
  contourSm->setToolTip (tr ("Change the contour smoothing factor (0-10)"));
  contourSm->setWhatsThis (contourSmText);
  smBoxLayout->addWidget (contourSm);
  conBoxLeftLayout->addWidget (smBox);


  QGroupBox *cfiltBox = new QGroupBox (tr ("Contour filter envelope"), this);
  QHBoxLayout *cfiltBoxLayout = new QHBoxLayout;
  cfiltBox->setLayout (cfiltBoxLayout);
  cfilt = new QDoubleSpinBox (cfiltBox);
  cfilt->setDecimals (2);
  cfilt->setRange (0.0, 100.0);
  cfilt->setSingleStep (10.0);
  cfilt->setToolTip (tr ("Change the contour filter envelope value"));
  cfilt->setWhatsThis (cfiltText);
  cfiltBoxLayout->addWidget (cfilt);
  conBoxLeftLayout->addWidget (cfiltBox);


  QGroupBox *zBox = new QGroupBox (tr ("Scale"), this);
  QHBoxLayout *zBoxLayout = new QHBoxLayout;
  zBox->setLayout (zBoxLayout);
  ZFactor = new QComboBox (zBox);
  ZFactor->setToolTip (tr ("Change the contour/Z scale factor"));
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


  QGroupBox *widBox = new QGroupBox (tr ("Width"), this);
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


  QGroupBox *iBox = new QGroupBox (tr ("Index"), this);
  QHBoxLayout *iBoxLayout = new QHBoxLayout;
  iBox->setLayout (iBoxLayout);
  Index = new QComboBox (iBox);
  Index->setToolTip (tr ("Change the index contour (Nth contour emphasized)"));
  Index->setWhatsThis (IndexText);
  Index->setEditable (TRUE);
  Index->addItem ("0");
  Index->addItem ("2");
  Index->addItem ("4");
  Index->addItem ("5");
  Index->addItem ("10");
  iBoxLayout->addWidget (Index);
  conBoxRightLayout->addWidget (iBox);


  conBoxLeftLayout->addStretch (1);
  conBoxRightLayout->addStretch (1);


  prefTab->addTab (conBox, tr ("Contour/Depth"));
  prefTab->setTabToolTip (2, tr ("Set the contour and depth options"));
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
  prefTab->setTabWhatsThis (3, filterText);


  QGroupBox *miscBox = new QGroupBox (this);
  QHBoxLayout *miscBoxLayout = new QHBoxLayout;
  miscBox->setLayout (miscBoxLayout);
  QVBoxLayout *miscBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *miscBoxRightLayout = new QVBoxLayout;
  miscBoxLayout->addLayout (miscBoxLeftLayout);
  miscBoxLayout->addLayout (miscBoxRightLayout);

  QGroupBox *wBox = new QGroupBox (tr ("MISP weight factor"), this);
  QHBoxLayout *wBoxLayout = new QHBoxLayout;
  wBox->setLayout (wBoxLayout);
  WFactor = new QComboBox (wBox);
  WFactor->setToolTip (tr ("Change the MISP weighting factor"));
  WFactor->setWhatsThis (WFactorText);
  WFactor->setEditable (TRUE);
  WFactor->addItem (tr ("1"));
  WFactor->addItem (tr ("2"));
  WFactor->addItem (tr ("3"));
  if (mod_misc.average_type)
    {
      WFactor->setEnabled (TRUE);
    }
  else
    {
      WFactor->setEnabled (FALSE);
    }
  wBoxLayout->addWidget (WFactor);
  miscBoxLeftLayout->addWidget (wBox);


  QGroupBox *forceBox = new QGroupBox (tr ("MISP force original input value"), this);
  QHBoxLayout *forceBoxLayout = new QHBoxLayout;
  forceBox->setLayout (forceBoxLayout);
  force = new QCheckBox (forceBox);
  force->setToolTip (tr ("Force MISP to use the original input value when available"));
  force->setWhatsThis (forceText);
  if (mod_misc.average_type)
    {
      force->setEnabled (TRUE);
    }
  else
    {
      force->setEnabled (FALSE);
    }
  forceBoxLayout->addWidget (force);
  miscBoxLeftLayout->addWidget (forceBox);


  QGroupBox *replaceBox = new QGroupBox (tr ("MISP replace all values"), this);
  QHBoxLayout *replaceBoxLayout = new QHBoxLayout;
  replaceBox->setLayout (replaceBoxLayout);
  replace = new QCheckBox (replaceBox);
  replace->setToolTip (tr ("Replace average edited surface values in all bins"));
  replace->setWhatsThis (replaceText);
  if (mod_misc.average_type)
    {
      replace->setEnabled (TRUE);
    }
  else
    {
      replace->setEnabled (FALSE);
    }
  replaceBoxLayout->addWidget (replace);
  miscBoxLeftLayout->addWidget (replaceBox);


  QGroupBox *zeroBox = new QGroupBox (tr ("Restart colors at zero"), this);
  QHBoxLayout *zeroBoxLayout = new QHBoxLayout;
  zeroBox->setLayout (zeroBoxLayout);
  zero = new QCheckBox (zeroBox);
  zero->setToolTip (tr ("Restart the color range at zero depth"));
  zero->setWhatsThis (zeroText);
  zeroBoxLayout->addWidget (zero);
  miscBoxLeftLayout->addWidget (zeroBox);


  QGroupBox *pBox = new QGroupBox (tr ("Highlight depth percentage"), this);
  QHBoxLayout *pBoxLayout = new QHBoxLayout;
  pBox->setLayout (pBoxLayout);
  hPercent = new QDoubleSpinBox (pBox);
  hPercent->setDecimals (2);
  hPercent->setRange (0.01, 5.0);
  hPercent->setSingleStep (0.10);
  hPercent->setToolTip (tr ("Change the highlight depth percentage value"));
  hPercent->setWhatsThis (hPercentText);
  pBoxLayout->addWidget (hPercent);
  miscBoxLeftLayout->addWidget (pBox);


  QGroupBox *cntBox = new QGroupBox (tr ("Highlight valid bin count"), this);
  QHBoxLayout *cntBoxLayout = new QHBoxLayout;
  cntBox->setLayout (cntBoxLayout);
  hCount = new QSpinBox (cntBox);
  hCount->setRange (1, 1000);
  hCount->setSingleStep (1);
  hCount->setToolTip (tr ("Change the highlight valid bin count value"));
  hCount->setWhatsThis (hCountText);
  cntBoxLayout->addWidget (hCount);
  miscBoxLeftLayout->addWidget (cntBox);


  QGroupBox *csBox = new QGroupBox (tr ("Chart scale"), this);
  QHBoxLayout *csBoxLayout = new QHBoxLayout;
  csBox->setLayout (csBoxLayout);
  chartScale = new QSpinBox (pBox);
  chartScale->setRange (0, 3000000);
  chartScale->setSingleStep (5000);
  chartScale->setToolTip (tr ("Change the chart scale"));
  chartScale->setWhatsThis (chartScaleText);
  csBoxLayout->addWidget (chartScale);
  miscBoxRightLayout->addWidget (csBox);


  QGroupBox *oBox = new QGroupBox (tr ("Window overlap percentage"), this);
  QHBoxLayout *oBoxLayout = new QHBoxLayout;
  oBox->setLayout (oBoxLayout);
  overlap = new QSpinBox (oBox);
  overlap->setRange (0, 95);
  overlap->setSingleStep (1);
  overlap->setToolTip (tr ("Change the move window overlap percentage"));
  overlap->setWhatsThis (overlapText);
  oBoxLayout->addWidget (overlap);
  miscBoxRightLayout->addWidget (oBox);

  QGroupBox *otfBox = new QGroupBox (tr ("OTF bin size"), this);
  QHBoxLayout *otfBoxLayout = new QHBoxLayout;
  otfBox->setLayout (otfBoxLayout);
  otf = new QDoubleSpinBox (pBox);
  otf->setDecimals (2);
  otf->setRange (1.00, 1852.0);
  otf->setSingleStep (1.00);
  otf->setToolTip (tr ("Change the on-the-fly gridding bin size (meters)"));
  otf->setWhatsThis (otfText);
  otfBoxLayout->addWidget (otf);
  miscBoxRightLayout->addWidget (otfBox);


  QGroupBox *textSearchBox = new QGroupBox (tr ("Feature search string"), this);
  QHBoxLayout *textSearchBoxLayout = new QHBoxLayout;
  textSearchBox->setLayout (textSearchBoxLayout);
  textSearch = new QLineEdit (textSearchBox);
  textSearch->setToolTip (tr ("Highlight features based on text search"));
  textSearch->setWhatsThis (textSearchText);
  textSearchBoxLayout->addWidget (textSearch);
  miscBoxRightLayout->addWidget (textSearchBox);


  invert = new QCheckBox (tr ("Invert"), textSearchBox);
  invert->setToolTip (tr ("Search for features that <b><i>do not</i></b> contain the search string"));
  invert->setWhatsThis (invertText);
  textSearchBoxLayout->addWidget (invert);


  QRadioButton *highlight = new QRadioButton (tr ("Highlight"));
  highlight->setWhatsThis (featureHighlightText);
  QRadioButton *display = new QRadioButton (tr ("Display"));
  display->setWhatsThis (displayText);

  hGrp = new QButtonGroup (this);
  hGrp->setExclusive (TRUE);
  connect (hGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHighlightClicked (int)));

  hGrp->addButton (highlight, 0);
  hGrp->addButton (display, 1);

  textSearchBoxLayout->addWidget (highlight);
  textSearchBoxLayout->addWidget (display);


  bHotKeys = new QPushButton (tr ("Button Hot Keys"), this);
  bHotKeys->setToolTip (tr ("Change hot key sequences for buttons"));
  bHotKeys->setWhatsThis (hotKeysText);
  connect (bHotKeys, SIGNAL (clicked ()), this, SLOT (slotHotKeys ()));
  miscBoxRightLayout->addWidget (bHotKeys);


  QGroupBox *iconBox = new QGroupBox (tr ("Button Icon Size"), this);
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

  miscBoxRightLayout->addWidget (iconBox);


  bGeotiff = new QPushButton (tr ("GeoTIFF transparency"), this);
  bGeotiff->setToolTip (tr ("Change the amount of transparency for GeoTIFF overlay"));
  bGeotiff->setWhatsThis (prefsGeotiffText);
  connect (bGeotiff, SIGNAL (clicked ()), this, SLOT (slotGeotiff ()));
  miscBoxLeftLayout->addWidget (bGeotiff);


  miscBoxLeftLayout->addStretch (1);
  miscBoxRightLayout->addStretch (1);


  prefTab->addTab (miscBox, tr ("Miscellaneous"));
  prefTab->setTabToolTip (4, tr ("Set miscellaneous options"));
  prefTab->setTabWhatsThis (4, miscText);


  QGroupBox *sunBox = new QGroupBox (this);
  QVBoxLayout *sunBoxLayout = new QVBoxLayout;
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

  sunBoxLayout->addStretch (1);


  prefTab->addTab (sunBox, tr ("Sun Shading"));
  prefTab->setTabToolTip (5, tr ("Set sun shading options"));
  prefTab->setTabWhatsThis (5, sunText);


  QGroupBox *cubeBox = new QGroupBox (this);
  QHBoxLayout *cubeBoxLayout = new QHBoxLayout;
  cubeBox->setLayout (cubeBoxLayout);
  QVBoxLayout *cubeBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cubeBoxRightLayout = new QVBoxLayout;
  cubeBoxLayout->addLayout (cubeBoxLeftLayout);
  cubeBoxLayout->addLayout (cubeBoxRightLayout);


  QGroupBox *ihoBox = new QGroupBox (tr ("IHO order"), this);
  QHBoxLayout *ihoBoxLayout = new QHBoxLayout;
  ihoBox->setLayout (ihoBoxLayout);
  ihoBoxLayout->setSpacing (10);

  ihoOrder = new QComboBox (ihoBox);
  ihoOrder->setToolTip (tr ("Set the IHO order for the CUBE process"));
  ihoOrder->setEditable (FALSE);
  ihoOrder->addItem ("IHO Special Order");
  ihoOrder->addItem ("IHO Order 1a/1b");
  ihoOrder->addItem ("IHO Order 2");
  ihoBoxLayout->addWidget (ihoOrder);
  if (!misc->cube_available || !cube_attr_available) ihoOrder->setEnabled (FALSE);


  cubeBoxLeftLayout->addWidget (ihoBox);


  QGroupBox *captureBox = new QGroupBox (tr ("Capture percentage"), this);
  QHBoxLayout *captureBoxLayout = new QHBoxLayout;
  captureBox->setLayout (captureBoxLayout);
  captureBoxLayout->setSpacing (10);

  capture = new QDoubleSpinBox (captureBox);
  capture->setDecimals (1);
  capture->setRange (1.0, 10.0);
  capture->setSingleStep (1.0);
  capture->setWrapping (TRUE);
  capture->setToolTip (tr ("Set the capture percentage for the CUBE process"));
  captureBoxLayout->addWidget (capture);
  if (!misc->cube_available || !cube_attr_available) capture->setEnabled (FALSE);


  cubeBoxLeftLayout->addWidget (captureBox);


  QGroupBox *queueBox = new QGroupBox (tr ("Queue length"), this);
  QHBoxLayout *queueBoxLayout = new QHBoxLayout;
  queueBox->setLayout (queueBoxLayout);
  queueBoxLayout->setSpacing (10);

  queue = new QSpinBox (queueBox);
  queue->setRange (1, 20);
  queue->setSingleStep (1);
  queue->setWrapping (TRUE);
  queue->setToolTip (tr ("Set the queue length for the CUBE process"));
  queueBoxLayout->addWidget (queue);
  if (!misc->cube_available || !cube_attr_available) queue->setEnabled (FALSE);


  cubeBoxLeftLayout->addWidget (queueBox);


  QGroupBox *horizBox = new QGroupBox (tr ("Horizontal position uncertainty"), this);
  QHBoxLayout *horizBoxLayout = new QHBoxLayout;
  horizBox->setLayout (horizBoxLayout);
  horizBoxLayout->setSpacing (10);

  horiz = new QDoubleSpinBox (horizBox);
  horiz->setDecimals (1);
  horiz->setRange (0.0, 100.0);
  horiz->setSingleStep (1.0);
  horiz->setWrapping (TRUE);
  horiz->setToolTip (tr ("Set the horizontal position uncertainty for the CUBE process"));
  horizBoxLayout->addWidget (horiz);
  if (!misc->cube_available || !cube_attr_available) horiz->setEnabled (FALSE);


  cubeBoxLeftLayout->addWidget (horizBox);


  QGroupBox *distanceBox = new QGroupBox (tr ("Distance exponent"), this);
  QHBoxLayout *distanceBoxLayout = new QHBoxLayout;
  distanceBox->setLayout (distanceBoxLayout);
  distanceBoxLayout->setSpacing (10);

  distance = new QDoubleSpinBox (distanceBox);
  distance->setDecimals (1);
  distance->setRange (0.0, 4.0);
  distance->setSingleStep (1.0);
  distance->setWrapping (TRUE);
  distance->setToolTip (tr ("Set the distance exponent for the CUBE process"));
  distanceBoxLayout->addWidget (distance);
  if (!misc->cube_available || !cube_attr_available) distance->setEnabled (FALSE);


  cubeBoxLeftLayout->addWidget (distanceBox);


  QGroupBox *minContextBox = new QGroupBox (tr ("Minimum context"), this);
  QHBoxLayout *minContextBoxLayout = new QHBoxLayout;
  minContextBox->setLayout (minContextBoxLayout);
  minContextBoxLayout->setSpacing (10);

  minContext = new QDoubleSpinBox (minContextBox);
  minContext->setDecimals (1);
  minContext->setRange (0.0, 10.0);
  minContext->setSingleStep (1.0);
  minContext->setWrapping (TRUE);
  minContext->setToolTip (tr ("Set the minimum context for the CUBE process"));
  minContextBoxLayout->addWidget (minContext);
  if (!misc->cube_available || !cube_attr_available) minContext->setEnabled (FALSE);


  cubeBoxRightLayout->addWidget (minContextBox);


  QGroupBox *maxContextBox = new QGroupBox (tr ("Maximum context"), this);
  QHBoxLayout *maxContextBoxLayout = new QHBoxLayout;
  maxContextBox->setLayout (maxContextBoxLayout);
  maxContextBoxLayout->setSpacing (10);

  maxContext = new QDoubleSpinBox (maxContextBox);
  maxContext->setDecimals (1);
  maxContext->setRange (0.0, 10.0);
  maxContext->setSingleStep (1.0);
  maxContext->setWrapping (TRUE);
  maxContext->setToolTip (tr ("Set the maximum context for the CUBE process"));
  maxContextBoxLayout->addWidget (maxContext);
  if (!misc->cube_available || !cube_attr_available) maxContext->setEnabled (FALSE);


  cubeBoxRightLayout->addWidget (maxContextBox);


  QGroupBox *std2confBox = new QGroupBox (tr ("Std2Conf"), this);
  QHBoxLayout *std2confBoxLayout = new QHBoxLayout;
  std2confBox->setLayout (std2confBoxLayout);
  std2confBoxLayout->setSpacing (10);

  std2conf = new QComboBox (std2confBox);
  std2conf->setToolTip (tr ("Set the standard deviation to confidence scale for the CUBE process"));
  std2conf->setEditable (FALSE);
  std2conf->addItem ("2.56");
  std2conf->addItem ("1.96");
  std2conf->addItem ("1.69");
  std2conf->addItem ("1.00");
  std2confBoxLayout->addWidget (std2conf);
  if (!misc->cube_available || !cube_attr_available) std2conf->setEnabled (FALSE);


  cubeBoxRightLayout->addWidget (std2confBox);


  QGroupBox *disBox = new QGroupBox (tr ("Disambiguation method"), this);
  QHBoxLayout *disBoxLayout = new QHBoxLayout;
  disBox->setLayout (disBoxLayout);
  disBoxLayout->setSpacing (10);

  disambiguation = new QComboBox (disBox);
  disambiguation->setToolTip (tr ("Set the disambiguation method for the CUBE process"));
  disambiguation->setEditable (FALSE);
  disambiguation->addItem ("Prior");
  disambiguation->addItem ("Likelihood");
  disambiguation->addItem ("Posterior");
  disBoxLayout->addWidget (disambiguation);
  if (!misc->cube_available || !cube_attr_available) disambiguation->setEnabled (FALSE);

  cubeBoxRightLayout->addWidget (disBox);

  cubeBoxLeftLayout->addStretch (1);
  cubeBoxRightLayout->addStretch (1);


  prefTab->addTab (cubeBox, tr ("CUBE"));
  prefTab->setTabToolTip (6, tr ("Set CUBE options"));
  prefTab->setTabWhatsThis (6, cubeBoxText);


  vbox->addWidget (prefTab);


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



/*!
  The following functions (until further notice) are just servicing pushbuttons.  They're intuitively obvious to the
  most casual observer.
*/

void
Prefs::slotContourColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.contour_color, this, tr ("pfmView Contour Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.contour_color = clr;

  setFields ();
}



void
Prefs::slotFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_color, this, tr ("pfmView Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_color = clr;

  setFields ();
}



void
Prefs::slotVerFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.verified_feature_color, this, tr ("pfmView Verified Feature Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.verified_feature_color = clr;

  setFields ();
}



void
Prefs::slotCovVerFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_verified_feature_color, this, tr ("pfmView Coverage Verified Feature Color"),
				QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_verified_feature_color = clr;

  setFields ();
}



void
Prefs::slotFeatureInfoColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_info_color, this, tr ("pfmView Feature Information Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_info_color = clr;

  setFields ();
}



void
Prefs::slotFeaturePolyColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.feature_poly_color, this, tr ("pfmView Feature Polygon Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.feature_poly_color = clr;

  setFields ();
}



void
Prefs::slotCoastColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.coast_color, this, tr ("pfmView Coastline Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.coast_color = clr;

  setFields ();
}



void
Prefs::slotLandmaskColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.landmask_color, this, tr ("pfmView Land Mask Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.landmask_color = clr;

  setFields ();
}



void
Prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("pfmView Background Color"));

  if (clr.isValid ()) mod_options.background_color = clr;

  setFields ();
}



void
Prefs::slotContourHighlightColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.contour_highlight_color, this, tr ("pfmView Highlighted Contour Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.contour_highlight_color = clr;

  setFields ();
}



void
Prefs::slotMaskColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.poly_filter_mask_color, this, tr ("pfmView Filter Mask Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.poly_filter_mask_color = clr;

  setFields ();
}



void
Prefs::slotCovFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_feature_color, this, tr ("pfmView Coverage Map Feature Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_feature_color = clr;

  setFields ();
}



void
Prefs::slotCovInvFeatureColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.cov_inv_feature_color, this, tr ("pfmView Coverage Map Invalid Feature Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.cov_inv_feature_color = clr;

  setFields ();
}



void
Prefs::slotStoplightMinColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_min_color, this, tr ("pfmView Stoplight Min Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_min_color = clr;

  setFields ();
}



void
Prefs::slotStoplightMidColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_mid_color, this, tr ("pfmView Stoplight Mid Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_mid_color = clr;

  setFields ();
}



void
Prefs::slotStoplightMaxColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.stoplight_max_color, this, tr ("pfmView Stoplight Max Color"), QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.stoplight_max_color = clr;

  setFields ();
}



void
Prefs::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//!  Restore all of the application defaults (this calls set_defaults and gets the original defaults).

void
Prefs::slotRestoreDefaults ()
{
  void set_defaults (MISC *misc, OPTIONS *options, NV_BOOL restore);


  set_defaults (misc, options, NVTrue);

  mod_options = *options;
  mod_share = *misc->abe_share;
  mod_misc = *misc;

  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);

  setFields ();


  mod_misc.GeoTIFF_init = NVTrue;


  *misc->abe_share = mod_share;
  *options = mod_options;
  *misc = mod_misc;

  emit dataChangedSignal (NVTrue);

  close ();
}



void
Prefs::slotPositionClicked (int id)
{
  mod_options.position_form = id;
}



void
Prefs::slotHighlightClicked (int id)
{
  mod_options.feature_search_type = id;
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


  setFields ();

  contoursD->close ();
}



//!  This function is used to set user defined contour levels.

void
Prefs::slotContourLevelsClicked ()
{
  if (contoursD) slotCloseContours ();


  QString string;

  mod_share.cint = 0.0;
  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  
  contoursD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  contoursD->setWindowTitle (tr ("pfmView Contour Levels"));

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



//!  This function sets the GeoTIFF transparency level (it uses a dummy GeoTIFF).

void 
Prefs::slotGeotiff ()
{
  if (geotiffD) geotiffD->close ();


  geotiffD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  geotiffD->setWindowTitle (tr ("pfmView GeoTIFF transparency"));


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


  //  Put the dialog in the middle of the screen.

  geotiffD->move (x () + width () / 2 - geotiffD->width () / 2, y () + height () / 2 - geotiffD->height () / 2);

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



//!  This is the hotkey changing dialog.

void 
Prefs::slotHotKeys ()
{
  if (hotKeyD) slotCloseHotKeys ();

  hotKeyD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  hotKeyD->setWindowTitle (tr ("pfmView Hot Keys"));

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
      bItem[i] = new QTableWidgetItem (mod_misc.buttonIcon[i], mod_misc.buttonText[i]);
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



//!  Get the EGM value nearest to the center of the displayed area.

void 
Prefs::slotEgmClicked ()
{
  NV_FLOAT64 lat, lon;

  lat = misc->total_displayed_area.min_y + (misc->total_displayed_area.max_y - misc->total_displayed_area.min_y) / 2.0;
  lon = misc->total_displayed_area.min_x + (misc->total_displayed_area.max_x - misc->total_displayed_area.min_x) / 2.0;

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
      QMessageBox::warning (this, tr ("pfmView Get EGM08 datum offset"), tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
    }
}



//!  Preliminary field/button setting function.

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


  mod_options.landmask_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bLandmaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bLandmaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bLandmaskPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bLandmaskPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bLandmaskPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.landmask_color);
  bLandmaskPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.landmask_color);
  bLandmaskColor->setPalette (bLandmaskPalette);


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


  mod_options.cov_verified_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovVerFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_verified_feature_color);
  bCovVerFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_verified_feature_color);
  bCovVerFeatureColor->setPalette (bCovVerFeaturePalette);


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


  mod_options.contour_highlight_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bContourHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bContourHighlightPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bContourHighlightPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.contour_highlight_color);
  bContourHighlightPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.contour_highlight_color);
  bContourHighlightColor->setPalette (bContourHighlightPalette);


  mod_options.poly_filter_mask_color.getHsv (&hue, &sat, &val);
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
  bMaskPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.poly_filter_mask_color);
  bMaskPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.poly_filter_mask_color);
  bMaskColor->setPalette (bMaskPalette);


  mod_options.cov_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_feature_color);
  bCovFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_feature_color);
  bCovFeatureColor->setPalette (bCovFeaturePalette);


  mod_options.cov_inv_feature_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bCovInvFeaturePalette.setColor (QPalette::Normal, QPalette::Button, mod_options.cov_inv_feature_color);
  bCovInvFeaturePalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.cov_inv_feature_color);
  bCovInvFeatureColor->setPalette (bCovInvFeaturePalette);


  //  Before we do the stoplight colors we want to find the nearest one in the color array so we can use the already
  //  defined shades to do sunshading.

  for (NV_INT32 j = 0 ; j < 3 ; j++)
    {
      QColor tmp;

      switch (j)
        {
        case 0:
          tmp = mod_options.stoplight_min_color;
          break;

        case 1:
          tmp = mod_options.stoplight_mid_color;
          break;

        case 2:
          tmp = mod_options.stoplight_max_color;
          break;
        }

      NV_INT32 min_hue_diff = 99999;
      NV_INT32 closest = -1;

      for (NV_INT32 i = 0 ; i < NUMHUES ; i++)
        {
          NV_INT32 hue_diff = abs (tmp.hue () - mod_options.color_array[0][i][255].hue ());

          if (hue_diff < min_hue_diff)
            {
              min_hue_diff = hue_diff;
              closest = i;
            }
        }

      switch (j)
        {
        case 0:
          mod_options.stoplight_min_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_min_index = closest;
          break;

        case 1:
          tmp = mod_options.stoplight_mid_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_mid_index = closest;
          break;

        case 2:
          tmp = mod_options.stoplight_max_color = mod_options.color_array[0][closest][255];
          mod_options.stoplight_max_index = closest;
          break;
        }
    }



  mod_options.stoplight_min_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMinPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_min_color);
  bStoplightMinPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_min_color);
  bStoplightMinColor->setPalette (bStoplightMinPalette);

  mod_options.stoplight_mid_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMidPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMidPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_mid_color);
  bStoplightMidPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_mid_color);
  bStoplightMidColor->setPalette (bStoplightMidPalette);

  mod_options.stoplight_max_color.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bStoplightMaxPalette.setColor (QPalette::Normal, QPalette::Button, mod_options.stoplight_max_color);
  bStoplightMaxPalette.setColor (QPalette::Inactive, QPalette::Button, mod_options.stoplight_max_color);
  bStoplightMaxColor->setPalette (bStoplightMaxPalette);


  string.sprintf ("%.2f", mod_share.cint);
  contourInt->setText (string);

  contourSm->setValue (mod_options.smoothing_factor);

  cfilt->setValue (mod_options.contour_filter_envelope);

  string.sprintf ("%.5f", mod_options.z_factor);
  ZFactor->lineEdit ()->setText (string);

  offset->setValue (mod_options.z_offset);

  string.sprintf ("%d", mod_options.contour_width);
  Width->lineEdit ()->setText (string);

  string.sprintf ("%d", mod_options.contour_index);
  Index->lineEdit ()->setText (string);

  string.sprintf ("%d", mod_options.misp_weight);
  WFactor->lineEdit ()->setText (string);
  force->setChecked (mod_options.misp_force_original);
  replace->setChecked (mod_options.misp_replace_all);

  zero->setChecked (mod_options.zero_turnover);

  filterSTD->setValue (mod_options.filterSTD);

  dFilter->setChecked (mod_options.deep_filter_only);

  featureRadius->setValue (mod_options.feature_radius);

  hPercent->setValue (mod_options.highlight_percent);

  hCount->setValue (mod_options.h_count);

  chartScale->setValue (mod_options.chart_scale);

  otf->setValue (mod_options.otf_bin_size_meters);

  overlap->setValue (mod_options.overlap_percent);

  sunAz->setValue (mod_options.sunopts.azimuth);
  sunEl->setValue (mod_options.sunopts.elevation);
  sunEx->setValue (mod_options.sunopts.exag);

  textSearch->setText (mod_options.feature_search_string);


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


  ihoOrder->setCurrentIndex (mod_options.iho);
  queue->setValue (mod_options.queue);
  capture->setValue (mod_options.capture);
  horiz->setValue (mod_options.horiz);
  distance->setValue (mod_options.distance);
  minContext->setValue (mod_options.min_context);
  maxContext->setValue (mod_options.max_context);
  std2conf->setCurrentIndex (mod_options.std2conf);
  disambiguation->setCurrentIndex (mod_options.disambiguation);

  hGrp->button (mod_options.feature_search_type)->setChecked (TRUE);
  invert->setChecked (mod_options.feature_search_invert);
}



//!  Get the values from the fields/buttons and issue a change signal if needed.

void
Prefs::slotApplyPrefs ()
{
  NV_INT32 tmp_i, status;
  NV_FLOAT32 tmp_f;
  NV_BOOL feature_search_changed = NVFalse;


  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;


  QString tmp = contourInt->text ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_share.cint = tmp_f;
  if (mod_share.cint != misc->abe_share->cint) dataChanged = NVTrue;

  tmp = contourSm->text ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.smoothing_factor = tmp_i;
  if (mod_options.smoothing_factor != options->smoothing_factor) dataChanged = NVTrue;

  tmp = cfilt->text ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_options.contour_filter_envelope = tmp_f;
  if (mod_options.contour_filter_envelope != options->contour_filter_envelope) dataChanged = NVTrue;

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


  tmp = Index->currentText ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.contour_index = tmp_i;
  if (mod_options.contour_index != options->contour_index) dataChanged = NVTrue;


  tmp = WFactor->currentText ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.misp_weight = tmp_i;
  if (mod_options.misp_weight != options->misp_weight) dataChanged = NVTrue;

  mod_options.misp_force_original = force->isChecked ();
  if (mod_options.misp_force_original != options->misp_force_original) dataChanged = NVTrue;

  mod_options.misp_replace_all = replace->isChecked ();
  if (mod_options.misp_replace_all != options->misp_replace_all) dataChanged = NVTrue;

  mod_options.zero_turnover = zero->isChecked ();
  if (mod_options.zero_turnover != options->zero_turnover) dataChanged = NVTrue;

  mod_options.filterSTD = filterSTD->value ();
  if (mod_options.filterSTD != options->filterSTD) dataChanged = NVTrue;

  mod_options.deep_filter_only = dFilter->isChecked ();
  if (mod_options.deep_filter_only != options->deep_filter_only) dataChanged = NVTrue;

  mod_options.feature_radius = featureRadius->value ();
  if (mod_options.feature_radius != options->feature_radius) dataChanged = NVTrue;

  mod_options.highlight_percent = hPercent->value ();
  if (mod_options.highlight_percent != options->highlight_percent) dataChanged = NVTrue;

  mod_options.h_count = hCount->value ();
  if (mod_options.h_count != options->h_count) dataChanged = NVTrue;

  mod_options.chart_scale = chartScale->value ();
  if (mod_options.chart_scale != options->chart_scale) dataChanged = NVTrue;

  mod_options.otf_bin_size_meters = otf->value ();
  if (mod_options.otf_bin_size_meters != options->otf_bin_size_meters) dataChanged = NVTrue;


  tmp = overlap->text ();
  status = sscanf (tmp.toAscii (), "%d", &tmp_i);
  if (status == 1) mod_options.overlap_percent = tmp_i;
  if (mod_options.overlap_percent != options->overlap_percent) dataChanged = NVTrue;


  mod_options.sunopts.azimuth = sunAz->value ();
  if (mod_options.sunopts.azimuth != options->sunopts.azimuth) dataChanged = NVTrue;
  mod_options.sunopts.elevation = sunEl->value ();
  if (mod_options.sunopts.elevation != options->sunopts.elevation) dataChanged = NVTrue;
  mod_options.sunopts.exag = sunEx->value ();
  if (mod_options.sunopts.exag != options->sunopts.exag) dataChanged = NVTrue;


  mod_options.sunopts.sun = sun_unv (mod_options.sunopts.azimuth, mod_options.sunopts.elevation);


  if (mod_options.feature_search_type != options->feature_search_type) dataChanged = NVTrue;
  mod_options.feature_search_invert = invert->isChecked ();
  if (mod_options.feature_search_invert != options->feature_search_invert) dataChanged = NVTrue;


  mod_options.feature_search_string = textSearch->text ();
  if (mod_options.feature_search_string != options->feature_search_string || mod_options.feature_search_type != options->feature_search_type ||
      mod_options.feature_search_invert != options->feature_search_invert)
    {
      dataChanged = NVTrue;
      feature_search_changed = NVTrue;
    }


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


  mod_options.iho = ihoOrder->currentIndex ();
  if (mod_options.iho != options->iho) dataChanged = NVTrue;
  mod_options.capture = capture->value ();
  if (mod_options.capture != options->capture) dataChanged = NVTrue;
  mod_options.queue = queue->value ();
  if (mod_options.queue != options->queue) dataChanged = NVTrue;
  mod_options.horiz = horiz->value ();
  if (mod_options.horiz != options->horiz) dataChanged = NVTrue;
  mod_options.distance = distance->value ();
  if (mod_options.distance != options->distance) dataChanged = NVTrue;
  mod_options.min_context = minContext->value ();
  if (mod_options.min_context != options->min_context) dataChanged = NVTrue;
  mod_options.max_context = maxContext->value ();
  if (mod_options.max_context != options->max_context) dataChanged = NVTrue;
  mod_options.std2conf = std2conf->currentIndex ();
  if (mod_options.std2conf != options->std2conf) dataChanged = NVTrue;
  mod_options.disambiguation = disambiguation->currentIndex ();
  if (mod_options.disambiguation != options->disambiguation) dataChanged = NVTrue;


  if (mod_options.GeoTIFF_alpha != options->GeoTIFF_alpha)
    {
      dataChanged = NVTrue;
      mod_misc.GeoTIFF_init = NVTrue;
    }


  if (mod_options.contour_color != options->contour_color) dataChanged = NVTrue;
  if (mod_options.coast_color != options->coast_color) dataChanged = NVTrue;
  if (mod_options.landmask_color != options->landmask_color) dataChanged = NVTrue;
  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.feature_color != options->feature_color) dataChanged = NVTrue;
  if (mod_options.feature_info_color != options->feature_info_color) dataChanged = NVTrue;
  if (mod_options.feature_poly_color != options->feature_poly_color) dataChanged = NVTrue;
  if (mod_options.contour_highlight_color != options->contour_highlight_color) dataChanged = NVTrue;
  if (mod_options.poly_filter_mask_color != options->poly_filter_mask_color) dataChanged = NVTrue;
  if (mod_options.cov_feature_color != options->cov_feature_color) dataChanged = NVTrue;
  if (mod_options.cov_inv_feature_color != options->cov_inv_feature_color) dataChanged = NVTrue;
  if (mod_options.verified_feature_color != options->verified_feature_color) dataChanged = NVTrue;
  if (mod_options.cov_verified_feature_color != options->cov_verified_feature_color) dataChanged = NVTrue;
  if (mod_options.stoplight_min_color != options->stoplight_min_color) dataChanged = NVTrue;
  if (mod_options.stoplight_mid_color != options->stoplight_mid_color) dataChanged = NVTrue;
  if (mod_options.stoplight_max_color != options->stoplight_max_color) dataChanged = NVTrue;



  //  Make sure we have no hotkey duplications.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (mod_options.buttonAccel[i].toUpper () == mod_options.buttonAccel[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("pfmView preferences"),
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
      *misc = mod_misc;

      emit dataChangedSignal (feature_search_changed);
    }

  close ();
}



void
Prefs::slotClosePrefs ()
{
  close ();
}
