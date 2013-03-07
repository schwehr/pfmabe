#include "prefs.hpp"
#include "prefsHelp.hpp"
#include "io_data.hpp"

Prefs::Prefs (QWidget *parent, OPTIONS *op, MISC *mi, POINT_DATA *da):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  data = da;

  mod_share = *misc->abe_share;
  mod_options = *op;

  setModal (TRUE);

  ancillaryProgramD = hotKeyD = NULL;


  dataChanged = NVFalse;


  setWindowTitle (tr ("geoSwath3D Preferences"));

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
  QVBoxLayout *fboxMid = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxMid);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxMid->addWidget (hd__);
  fboxMid->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);


  QGroupBox *conBox = new QGroupBox (tr ("Depth options"), this);
  QHBoxLayout *conBoxLayout = new QHBoxLayout;
  conBox->setLayout (conBoxLayout);


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
  zBoxLayout->addWidget (ZFactor);
  conBoxLayout->addWidget (zBox);


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
  conBoxLayout->addWidget (offBox);


  QGroupBox *zoomBox = new QGroupBox (tr ("Zoom percentage"), this);
  QHBoxLayout *zoomBoxLayout = new QHBoxLayout;
  zoomBox->setLayout (zoomBoxLayout);
  zoomPercent = new QSpinBox (zoomBox);
  zoomPercent->setRange (5, 50);
  zoomPercent->setSingleStep (5);
  zoomPercent->setToolTip (tr ("Change the zoom in/out percentage (10 - 50)"));
  zoomPercent->setWhatsThis (zoomPercentText);
  zoomBoxLayout->addWidget (zoomPercent);
  conBoxLayout->addWidget (zoomBox);


  QGroupBox *exagBox = new QGroupBox (tr ("Z exaggeration"), this);
  QHBoxLayout *exagBoxLayout = new QHBoxLayout;
  exagBox->setLayout (exagBoxLayout);
  exag = new QDoubleSpinBox (offBox);
  exag->setDecimals (1);
  exag->setRange (1.0, 10.0);
  exag->setSingleStep (1.0);
  exag->setToolTip (tr ("Change the Z exaggeration value"));
  exag->setWhatsThis (exagText);
  exagBoxLayout->addWidget (exag);
  conBoxLayout->addWidget (exagBox);

  QGroupBox *pixBox = new QGroupBox (tr ("Point size"), this);
  QHBoxLayout *pixBoxLayout = new QHBoxLayout;
  pixBox->setLayout (pixBoxLayout);
  pointSize = new QSpinBox (pixBox);
  pointSize->setRange (2, 10);
  pointSize->setSingleStep (1);
  pointSize->setToolTip (tr ("Change the point size (pixels) (2 - 10)"));
  pointSize->setWhatsThis (pointSizeText);
  pixBoxLayout->addWidget (pointSize);
  conBoxLayout->addWidget (pixBox);


  vbox->addWidget (conBox, 1);


  QGroupBox *miscBox = new QGroupBox (tr ("Miscellaneous"), this);
  QHBoxLayout *miscBoxLayout = new QHBoxLayout;
  miscBox->setLayout (miscBoxLayout);


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

  miscBoxLayout->addWidget (minBox);


  QGroupBox *pointBox = new QGroupBox (tr ("Displayed point limit"), this);
  QHBoxLayout *pointBoxLayout = new QHBoxLayout;
  pointBox->setLayout (pointBoxLayout);

  pointLimit = new QSpinBox (this);
  pointLimit->setRange (10000, 500000);
  pointLimit->setSingleStep (10000);
  pointLimit->setToolTip (tr ("Number of points that you wish to have displayed"));
  pointLimit->setWhatsThis (pointLimitText);
  pointBoxLayout->addWidget (pointLimit);

  miscBoxLayout->addWidget (pointBox);


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
  miscBoxLayout->addWidget (rotBox);


  QGroupBox *scaleBox = new QGroupBox (tr ("Draw scale"), this);
  QHBoxLayout *scaleBoxLayout = new QHBoxLayout;
  scaleBox->setLayout (scaleBoxLayout);
  scale = new QCheckBox (this);
  scale->setToolTip (tr ("If checked, the scale will be drawn on the screen"));
  scale->setWhatsThis (scaleText);
  scaleBoxLayout->addWidget (scale);
  miscBoxLayout->addWidget (scaleBox);


  QGroupBox *undoBox = new QGroupBox (tr ("Undo levels"), this);
  QHBoxLayout *undoBoxLayout = new QHBoxLayout;
  undoBox->setLayout (undoBoxLayout);
  undo = new QSpinBox (undoBox);
  undo->setRange (100, 1000000);
  undo->setSingleStep (100);
  undo->setToolTip (tr ("Change the maximum number of undo levels (100 - 1,000,000)"));
  undo->setWhatsThis (prefsUndoText);
  undoBoxLayout->addWidget (undo);
  miscBoxLayout->addWidget (undoBox);


  vbox->addWidget (miscBox, 1);



  QGroupBox *dummyBox = new QGroupBox (this);
  QHBoxLayout *dummyBoxLayout = new QHBoxLayout;
  dummyBox->setLayout (dummyBoxLayout);

  QGroupBox *kbox = new QGroupBox (tr ("Hot keys"), this);
  QHBoxLayout *kboxLayout = new QHBoxLayout;
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


  dummyBoxLayout->addWidget (kbox, 1);


  QGroupBox *filterBox = new QGroupBox (tr ("Filter settings"), this);
  QHBoxLayout *filterBoxLayout = new QHBoxLayout;
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

  dummyBoxLayout->addWidget (filterBox, 1);


  vbox->addWidget (dummyBox, 1);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QHBoxLayout *cboxLayout = new QHBoxLayout;
  cbox->setLayout (cboxLayout);
  QVBoxLayout *cboxLeftLayout = new QVBoxLayout;
  QVBoxLayout *cboxMidLayout = new QVBoxLayout;
  QVBoxLayout *cboxRightLayout = new QVBoxLayout;
  cboxLayout->addLayout (cboxLeftLayout);
  cboxLayout->addLayout (cboxMidLayout);
  cboxLayout->addLayout (cboxRightLayout);


  bTrackerColor = new QPushButton (tr ("Tracker"), this);
  bTrackerColor->setToolTip (tr ("Change tracker color"));
  bTrackerColor->setWhatsThis (trackerColorText);
  bTrackerPalette = bTrackerColor->palette ();
  connect (bTrackerColor, SIGNAL (clicked ()), this, SLOT (slotTrackerColor ()));
  cboxLeftLayout->addWidget (bTrackerColor);


  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (backgroundColorText);
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));
  cboxLeftLayout->addWidget (bBackgroundColor);


  bScaleColor = new QPushButton (tr ("Scale"), this);
  bScaleColor->setToolTip (tr ("Change scale color"));
  bScaleColor->setWhatsThis (scaleColorText);
  bScalePalette = bScaleColor->palette ();
  connect (bScaleColor, SIGNAL (clicked ()), this, SLOT (slotScaleColor ()));
  cboxLeftLayout->addWidget (bScaleColor);


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

      if (!i)
        {
          cboxLeftLayout->addWidget (bWaveColor[i]);
        }
      else if (i < 5)
        {
          cboxMidLayout->addWidget (bWaveColor[i]);
        }
      else
        {
          cboxRightLayout->addWidget (bWaveColor[i]);
        }
    }


  vbox->addWidget (cbox, 1);


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
Prefs::slotTrackerColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.tracker_color, this, tr ("geoSwath3D Tracker Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.tracker_color = clr;

  setFields ();
}



void
Prefs::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.background_color, this, tr ("geoSwath3D Background Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.background_color = clr;


  setFields ();
}



void
Prefs::slotScaleColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (mod_options.scale_color, this, tr ("geoSwath3D Scale Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.scale_color = clr;

  setFields ();
}



void
Prefs::slotColorClicked (int id)
{
  QString name;

  name.sprintf (tr ("Waveform/Marker %d").toAscii (), id);

  QColor clr;

  clr = QColorDialog::getColor (mod_options.waveColor[id], this, tr ("geoSwath3D ") + name + tr (" Color"),
                                QColorDialog::ShowAlphaChannel);

  if (clr.isValid ()) mod_options.waveColor[id] = clr;

  setFields ();
}



void
Prefs::slotAncillaryPrograms ()
{
  if (ancillaryProgramD) ancillaryProgramD->close ();


  ancillaryProgramD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  ancillaryProgramD->setWindowTitle (tr ("geoSwath3D Ancillary Programs"));

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



//  Cheating again - any change will cause everything to update.  It only takes a second anyway.

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
  void set_defaults (MISC *misc, OPTIONS *options, POINT_DATA *data, NV_BOOL restore);


  set_defaults (misc, options, data, NVTrue);


  mod_options = *options;
  mod_share = *misc->abe_share;


  setFields ();


  *misc->abe_share = mod_share;
  *options = mod_options;


  //  Let the calling program (pfmView) know that we've changed some things that it uses.

  if (misc->abe_share != NULL)
    {
      misc->abe_share->settings_changed = NVTrue;
      misc->abe_share->position_form = options->position_form;
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
Prefs::slotHotKeys ()
{
  if (hotKeyD) slotCloseHotKeys ();


  hotKeyD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  hotKeyD->setWindowTitle (tr ("geoSwath3D Hot Keys"));

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

  lat = data->bounds.min_y + (data->bounds.max_y - data->bounds.min_y) / 2.0;
  lon = data->bounds.min_x + (data->bounds.max_x - data->bounds.min_x) / 2.0;

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
      QMessageBox::warning (this, tr ("Get EGM08 datum offset"), tr ("Error retrieving EGM08 ellipsoid to geoid offset value"));
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


  string.sprintf ("%.5f", mod_options.z_factor);
  ZFactor->lineEdit ()->setText (string);

  offset->setValue (mod_options.z_offset);

  pointSize->setValue (mod_options.point_size);
  pointLimit->setValue (mod_options.point_limit);

  minZ->setValue (mod_options.min_window_size);

  ihoOrder->setCurrentIndex (mod_options.iho_min_window);

  filterSTD->setValue (mod_options.filterSTD);

  dFilter->setChecked (mod_options.deep_filter_only);

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

      
  zoomPercent->setValue (mod_options.zoom_percent);
  rot->setValue (mod_options.rotation_increment);
  exag->setValue (mod_options.exaggeration);
  undo->setValue (mod_options.undo_levels);
  scale->setChecked (mod_options.draw_scale);
}



void
Prefs::slotApplyPrefs ()
{
  NV_INT32 status;
  NV_FLOAT32 tmp_f;

  void get_buffer (POINT_DATA *data, MISC *misc, NV_INT32 record);


  if (mod_options.position_form != options->position_form) dataChanged = NVTrue;


  QString tmp = ZFactor->currentText ();
  status = sscanf (tmp.toAscii (), "%f", &tmp_f);
  if (status == 1) mod_options.z_factor = tmp_f;
  if (mod_options.z_factor != options->z_factor) dataChanged = NVTrue;


  mod_options.z_offset = offset->value ();
  if (mod_options.z_offset != options->z_offset) dataChanged = NVTrue;


  mod_options.point_size = pointSize->value ();
  if (mod_options.point_size != options->point_size) dataChanged = NVTrue;

  mod_options.point_limit = pointLimit->value ();
  if (mod_options.point_limit != options->point_limit)
    {
      options->point_limit = mod_options.point_limit;

      NV_INT32 record = get_record () - misc->num_records;
      io_data_close ();
      io_data_open (misc, options);

      get_buffer (data, misc, record);

      dataChanged = NVTrue;
    }

  mod_options.min_window_size = minZ->value ();
  if (mod_options.min_window_size != options->min_window_size) dataChanged = NVTrue;

  mod_options.iho_min_window = ihoOrder->currentIndex ();
  if (mod_options.iho_min_window != options->iho_min_window) dataChanged = NVTrue;

  mod_options.filterSTD = filterSTD->value ();
  if (mod_options.filterSTD != options->filterSTD) dataChanged = NVTrue;

  mod_options.deep_filter_only = dFilter->isChecked ();
  if (mod_options.deep_filter_only != options->deep_filter_only) dataChanged = NVTrue;

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

  mod_options.rotation_increment = rot->value ();
  if (mod_options.rotation_increment != options->rotation_increment) dataChanged = NVTrue;

  mod_options.exaggeration = exag->value ();
  if (mod_options.exaggeration != options->exaggeration) dataChanged = NVTrue;

  mod_options.zoom_percent = zoomPercent->value ();
  if (mod_options.zoom_percent != options->zoom_percent) dataChanged = NVTrue;

  mod_options.undo_levels = undo->value ();
  if (mod_options.undo_levels != options->undo_levels && resize_undo (misc, options, mod_options.undo_levels)) dataChanged = NVTrue;

  mod_options.draw_scale = scale->isChecked ();
  if (mod_options.draw_scale != options->draw_scale) dataChanged = NVTrue;

  if (mod_options.tracker_color != options->tracker_color) dataChanged = NVTrue;
  if (mod_options.background_color != options->background_color) dataChanged = NVTrue;
  if (mod_options.scale_color != options->scale_color) dataChanged = NVTrue;


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


  //  Make sure we have no hotkey duplications.  First, the buttons against the buttons.

  for (NV_INT32 i = 0 ; i < HOTKEYS ; i++)
    {
      for (NV_INT32 j = i + 1 ; j < HOTKEYS ; j++)
        {
          if (mod_options.buttonAccel[i].toUpper () == mod_options.buttonAccel[j].toUpper ())
            {
              QMessageBox::warning (0, tr ("geoSwath3D preferences"),
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
              QMessageBox::warning (0, tr ("geoSwath3D preferences"),
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
              QMessageBox::warning (0, tr ("geoSwath3D preferences"),
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


  setFields ();


  hide ();


  if (dataChanged)
    {
      *misc->abe_share = mod_share;
      *options = mod_options;


      emit dataChangedSignal ();
    }

  close ();
}



void
Prefs::slotClosePrefs ()
{
  close ();
}
