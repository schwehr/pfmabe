
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



#include "globalPage.hpp"
#include "globalPageHelp.hpp"


NV_INT32 gsfAttributesSet (PFM_GLOBAL global);
NV_INT32 hofAttributesSet (PFM_GLOBAL global);
NV_INT32 tofAttributesSet (PFM_GLOBAL global);
NV_INT32 wlfAttributesSet (PFM_GLOBAL global);
NV_INT32 czmilAttributesSet (PFM_GLOBAL global);
NV_INT32 bagAttributesSet (PFM_GLOBAL global);
NV_INT32 hawkeyeAttributesSet (PFM_GLOBAL global);


globalPage::globalPage (QWidget *prnt, PFM_GLOBAL *pfm_glb, FLAGS *fl, GLOBAL_DIALOG *gd, NV_BOOL cube_available):
  QWizardPage (prnt)
{
  pfm_global = pfm_glb;
  global_dialog = gd;
  parent = prnt;
  flags = fl;
  global_dialog->gsfD = NULL;
  global_dialog->hofD = NULL;
  global_dialog->tofD = NULL;
  global_dialog->wlfD = NULL;
  global_dialog->czmilD = NULL;
  global_dialog->bagD = NULL;
  global_dialog->hawkeyeD = NULL;
  global_dialog->cubeD = NULL;


  setTitle (tr ("Global Options"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmLoadMWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *obox = new QGroupBox (tr ("Optional flag settings"), this);
  QHBoxLayout *oboxLayout = new QHBoxLayout;
  obox->setLayout (oboxLayout);
  QVBoxLayout *oboxLLayout = new QVBoxLayout;
  QVBoxLayout *oboxMLayout = new QVBoxLayout;
  QVBoxLayout *oboxRLayout = new QVBoxLayout;
  oboxLayout->addLayout (oboxLLayout);
  oboxLayout->addLayout (oboxMLayout);
  oboxLayout->addLayout (oboxRLayout);


  optGrp = new QButtonGroup (this);
  optGrp->setExclusive (FALSE);
  connect (optGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotOptClicked (int)));


  fileCheck = new QCheckBox (tr ("Preliminary file check"), this);
  fileCheck->setChecked (flags->chk);
  fileCheck->setToolTip (tr ("Check input files for read access"));
  fileCheck->setWhatsThis (fileCheckText);
  optGrp->addButton (fileCheck);
  oboxLLayout->addWidget (fileCheck);

  gsfNominal = new QCheckBox (tr ("Load nominal depths from GSF files"), this);
  gsfNominal->setChecked (flags->nom);
  gsfNominal->setToolTip (tr ("Load the nominal depths from GSF files if available"));
  gsfNominal->setWhatsThis (gsfNominalText);
  optGrp->addButton (gsfNominal);
  oboxLLayout->addWidget (gsfNominal);

  cubeAttributes = new QCheckBox (tr ("Insert CUBE bin attributes"), this);
  cubeAttributes->setChecked (flags->attr);
  cubeAttributes->setToolTip (tr ("Save space for the CUBE bin attributes"));
  cubeAttributes->setWhatsThis (cubeAttributesText);
  optGrp->addButton (cubeAttributes, 1);
  oboxLLayout->addWidget (cubeAttributes);


  cubeLoad = new QCheckBox (tr ("Run CUBE"), this);
  cubeLoad->setChecked (flags->cube);
  cubeLoad->setToolTip (tr ("Run the CUBE process when the PFM has completed loading"));
  cubeLoad->setWhatsThis (cubeLoadText);
  optGrp->addButton (cubeLoad, 2);
  oboxLLayout->addWidget (cubeLoad);

  if (!cube_available)
    {
      cubeLoad->setChecked (FALSE);
      cubeLoad->setEnabled (FALSE);
    }


  refData = new QCheckBox (tr ("Mark data as reference"), this);
  refData->setChecked (flags->ref);
  refData->setToolTip (tr ("Mark input data as reference only"));
  refData->setWhatsThis (refDataText);
  optGrp->addButton (refData);
  oboxLLayout->addWidget (refData);


  subPath = new QCheckBox (tr ("Invert paths"), this);
  subPath->setChecked (flags->sub);
  subPath->setToolTip (tr ("Invert paths from UNIX to Windows syntax"));
  subPath->setWhatsThis (subPathText);
  optGrp->addButton (subPath);
  oboxLLayout->addWidget (subPath);

  if ((find_startup_name (".pfm_cfg")) == NULL) subPath->setEnabled (FALSE);

#ifndef NVLinux
  subPath->setEnabled (FALSE);
#endif


  hofOld = new QCheckBox (tr ("Load HOF in GCS compatible form"), this);
  hofOld->setChecked (flags->old);
  hofOld->setToolTip (tr ("Load HOF data in the old PFM_SHOALS_1K_DATA (GCS compatible) form"));
  hofOld->setWhatsThis (hofOldText);
  connect (hofOld, SIGNAL (stateChanged (int)), this, SLOT (slotHofOldClicked (int)));
  optGrp->addButton (hofOld);
  oboxMLayout->addWidget (hofOld);

  hofNull = new QCheckBox (tr ("Load HOF/CZMIL null values"), this);
  hofNull->setChecked (flags->hof);
  hofNull->setToolTip (tr ("Load HOF or CZMIL null values into the PFM structure"));
  hofNull->setWhatsThis (hofNullText);
  optGrp->addButton (hofNull);
  oboxMLayout->addWidget (hofNull);

  invLand = new QCheckBox (tr ("Invalidate HOF land values"), this);
  invLand->setChecked (flags->lnd);
  invLand->setToolTip (tr ("Invalidate all HOF values that are marked as Land"));
  invLand->setWhatsThis (invLandText);
  optGrp->addButton (invLand);
  oboxMLayout->addWidget (invLand);

  invSec = new QCheckBox (tr ("Invalidate HOF secondary values"), this);
  invSec->setChecked (flags->sec);
  invSec->setToolTip (tr ("Invalidate all HOF secondary return values"));
  invSec->setWhatsThis (invSecText);
  optGrp->addButton (invSec);
  oboxMLayout->addWidget (invSec);


  if (flags->old) invSec->setEnabled (FALSE);


  tofRef = new QCheckBox (tr ("Load TOF first returns as reference"), this);
  tofRef->setChecked (flags->tof);
  tofRef->setToolTip (tr ("Load TOF first return values as reference points"));
  tofRef->setWhatsThis (tofRefText);
  optGrp->addButton (tofRef);
  oboxMLayout->addWidget (tofRef);


  hofFlag = new QCheckBox (tr ("Load HOF standard user flags"), this);
  hofFlag->setChecked (flags->lid);
  hofFlag->setToolTip (tr ("Load HOF (IVS) standard user flags"));
  hofFlag->setWhatsThis (hofFlagText);
  optGrp->addButton (hofFlag);
  oboxMLayout->addWidget (hofFlag);


  //  The SRTM stuff is set up as a pseudo radio box via the slot.

  srtm1 = check_srtm1_topo ();
  srtm2 = check_srtm2_topo ();
  srtm3 = check_srtm3_topo ();
  srtm30 = check_srtm30_topo ();


  srtmBest = new QCheckBox (tr ("Load best resolution SRTM data"), this);
  srtmBest->setChecked (flags->srtmb);
  srtmBest->setToolTip (tr ("Load the best available resolution SRTM data if available"));
  srtmBest->setWhatsThis (srtmBestText);
  optGrp->addButton (srtmBest, 10);
  oboxRLayout->addWidget (srtmBest);

  if (!srtm1 && !srtm2 && !srtm3 &&!srtm30) srtmBest->setEnabled (FALSE);


  srtmOne = new QCheckBox (tr ("Load 1 second resolution SRTM data"), this);
  srtmOne->setChecked (flags->srtm1);
  srtmOne->setToolTip (tr ("Load the 1 second resolution SRTM data if available"));
  srtmOne->setWhatsThis (srtmOneText);
  optGrp->addButton (srtmOne, 11);
  oboxRLayout->addWidget (srtmOne);

  srtmOne->setEnabled (srtm1);


  srtmThree = new QCheckBox (tr ("Load 3 second resolution SRTM data"), this);
  srtmThree->setChecked (flags->srtm3);
  srtmThree->setToolTip (tr ("Load the 3 second resolution SRTM data if available"));
  srtmThree->setWhatsThis (srtmThreeText);
  optGrp->addButton (srtmThree, 12);
  oboxRLayout->addWidget (srtmThree);

  srtmThree->setEnabled (srtm3);


  srtmThirty = new QCheckBox (tr ("Load 30 second resolution SRTM data"), this);
  srtmThirty->setChecked (flags->srtm30);
  srtmThirty->setToolTip (tr ("Load the 30 second resolution SRTM data if available"));
  srtmThirty->setWhatsThis (srtmThirtyText);
  optGrp->addButton (srtmThirty, 13);
  oboxRLayout->addWidget (srtmThirty);

  srtmThirty->setEnabled (srtm30);


  srtmRef = new QCheckBox (tr ("Load SRTM data as reference"), this);
  srtmRef->setChecked (flags->srtmr);
  srtmRef->setToolTip (tr ("Mark any loaded SRTM data as reference"));
  srtmRef->setWhatsThis (srtmRefText);
  optGrp->addButton (srtmRef, 14);
  oboxRLayout->addWidget (srtmRef);

  if (!srtm1 && !srtm2 && !srtm3 &&!srtm30) srtmRef->setEnabled (FALSE);


  srtm2Ex = new QCheckBox (tr ("Exclude SRTM2 data"), this);
  srtm2Ex->setChecked (flags->srtme);
  srtm2Ex->setToolTip (tr ("Do not load any limited distribution SRTM2 data"));
  srtm2Ex->setWhatsThis (srtm2ExText);
  optGrp->addButton (srtm2Ex, 15);
  oboxRLayout->addWidget (srtm2Ex);

  srtm2Ex->setEnabled (srtm2);



  if (getenv ("SRTM_DATA") == NULL)
    {
      srtmBest->setEnabled (FALSE);
      srtmOne->setEnabled (FALSE);
      srtmThree->setEnabled (FALSE);
      srtmThirty->setEnabled (FALSE);
      srtmRef->setEnabled (FALSE);
    }
  else
    {
      if (flags->srtmb)
        {
          srtmOne->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }
      if (flags->srtm1)
        {
          srtmBest->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }

      if (flags->srtm3)
        {
          srtmBest->setChecked (FALSE);
          srtmOne->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }

      if (flags->srtm30)
        {
          srtmBest->setChecked (FALSE);
          srtmOne->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
        }
    }


  vbox->addWidget (obox, 1);


  //  Index file attribute selection.

  QGroupBox *attrBox = new QGroupBox (tr ("Depth attributes"), this);
  QHBoxLayout *attrBoxLayout = new QHBoxLayout;
  attrBox->setLayout (attrBoxLayout);
  attrBoxLayout->setSpacing (10);


  QGroupBox *tbox = new QGroupBox (tr ("Time (POSIX minutes)"), this);
  tbox->setWhatsThis (timeCheckText);
  QHBoxLayout *tboxLayout = new QHBoxLayout;
  tbox->setLayout (tboxLayout);

  timeCheck = new QCheckBox (tbox);
  timeCheck->setToolTip (tr ("Insert time (in POSIX minutes) as an attribute for all data types"));
  timeCheck->setWhatsThis (timeCheckText);
  if (pfm_global->time_attribute_num)
    {
      timeCheck->setChecked (TRUE);
    }
  else
    {
      timeCheck->setChecked (FALSE);
    }
  tboxLayout->addWidget (timeCheck);
  connect (timeCheck, SIGNAL (clicked (bool)), this, SLOT (slotTimeCheckClicked (bool)));
  attrBoxLayout->addWidget (tbox);


  NV_INT32 cnt = 0;
  QString attrString;

  if ((cnt = gsfAttributesSet (*pfm_global)))
    {
      attrString = tr ("GSF (%1)").arg (cnt);
      gsf = new QPushButton (attrString, this);
    }
  else
    {
      gsf = new QPushButton (tr ("GSF"), this);
    }
  gsf->setToolTip (tr ("Select depth attributes for GSF files (* indicates some attributes set)"));
  gsf->setWhatsThis (gsfText);
  connect (gsf, SIGNAL (clicked ()), this, SLOT (slotGSFClicked ()));
  attrBoxLayout->addWidget (gsf);

  gsfPalette = gsf->palette ();
  widgetBackgroundColor = gsfPalette.color (QPalette::Normal, QPalette::Button);
  if (gsfAttributesSet (*pfm_global))
    {
      gsfPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      gsfPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      gsf->setPalette (gsfPalette);
    }


  if ((cnt = hofAttributesSet (*pfm_global)))
    {
      attrString = tr ("HOF (%1)").arg (cnt);
      hof = new QPushButton (attrString, this);
    }
  else
    {
      hof = new QPushButton (tr ("HOF"), this);
    }
  hof->setToolTip (tr ("Select depth attributes for HOF files (red background indicates some attributes set)"));
  hof->setWhatsThis (hofText);
  connect (hof, SIGNAL (clicked ()), this, SLOT (slotHOFClicked ()));
  attrBoxLayout->addWidget (hof);

  hofPalette = hof->palette ();
  if (hofAttributesSet (*pfm_global))
    {
      hofPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      hofPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      hof->setPalette (hofPalette);
    }


  if ((cnt = tofAttributesSet (*pfm_global)))
    {
      attrString = tr ("TOF (%1)").arg (cnt);
      tof = new QPushButton (attrString, this);
    }
  else
    {
      tof = new QPushButton (tr ("TOF"), this);
    }
  tof->setToolTip (tr ("Select depth attributes for TOF files (red background indicates some attributes set)"));
  tof->setWhatsThis (tofText);
  connect (tof, SIGNAL (clicked ()), this, SLOT (slotTOFClicked ()));
  attrBoxLayout->addWidget (tof);

  tofPalette = tof->palette ();
  if (tofAttributesSet (*pfm_global))
    {
      tofPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      tofPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      tof->setPalette (tofPalette);
    }


  if ((cnt = wlfAttributesSet (*pfm_global)))
    {
      attrString = tr ("WLF (%1)").arg (cnt);
      wlf = new QPushButton (attrString, this);
    }
  else
    {
      wlf = new QPushButton (tr ("WLF"), this);
    }
  wlf->setToolTip (tr ("Select depth attributes for WLF files (red background indicates some attributes set)"));
  wlf->setWhatsThis (wlfText);
  connect (wlf, SIGNAL (clicked ()), this, SLOT (slotWLFClicked ()));
  attrBoxLayout->addWidget (wlf);

  wlfPalette = wlf->palette ();
  if (wlfAttributesSet (*pfm_global))
    {
      wlfPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      wlfPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      wlf->setPalette (wlfPalette);
    }


  if ((cnt = czmilAttributesSet (*pfm_global)))
    {
      attrString = tr ("CZMIL (%1)").arg (cnt);
      czmil = new QPushButton (attrString, this);
    }
  else
    {
      czmil = new QPushButton (tr ("CZMIL"), this);
    }
  czmil->setToolTip (tr ("Select depth attributes for CZMIL files (red background indicates some attributes set)"));
  czmil->setWhatsThis (czmilText);
  connect (czmil, SIGNAL (clicked ()), this, SLOT (slotCZMILClicked ()));
  attrBoxLayout->addWidget (czmil);

  czmilPalette = czmil->palette ();
  if (czmilAttributesSet (*pfm_global))
    {
      czmilPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      czmilPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      czmil->setPalette (czmilPalette);
    }


  if ((cnt = bagAttributesSet (*pfm_global)))
    {
      attrString = tr ("BAG (%1)").arg (cnt);
      bag = new QPushButton (attrString, this);
    }
  else
    {
      bag = new QPushButton (tr ("BAG"), this);
    }
  bag->setToolTip (tr ("Select depth attributes for BAG files (red background indicates some attributes set)"));
  bag->setWhatsThis (bagText);
  connect (bag, SIGNAL (clicked ()), this, SLOT (slotBAGClicked ()));
  attrBoxLayout->addWidget (bag);

  bagPalette = bag->palette ();
  if (bagAttributesSet (*pfm_global))
    {
      bagPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      bagPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      bag->setPalette (bagPalette);
    }


  if ((cnt = hawkeyeAttributesSet (*pfm_global)))
    {
      attrString = tr ("HAWKEYE (%1)").arg (cnt);
      hawkeye = new QPushButton (attrString, this);
    }
  else
    {
      hawkeye = new QPushButton (tr ("HAWKEYE"), this);
    }
  hawkeye->setToolTip (tr ("Select depth attributes for HAWKEYE files (red background indicates some attributes set)"));
  hawkeye->setWhatsThis (hawkeyeText);
  connect (hawkeye, SIGNAL (clicked ()), this, SLOT (slotHAWKEYEClicked ()));
  attrBoxLayout->addWidget (hawkeye);

  hawkeyePalette = hawkeye->palette ();
  if (hawkeyeAttributesSet (*pfm_global))
    {
      hawkeyePalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      hawkeyePalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      hawkeye->setPalette (hawkeyePalette);
    }


  vbox->addWidget (attrBox, 1);


  QGroupBox *hvBox = new QGroupBox (tr ("Horizontal and vertical errors"), this);
  QHBoxLayout *hvBoxLayout = new QHBoxLayout;
  hvBox->setLayout (hvBoxLayout);
  hvBoxLayout->setSpacing (10);


  QGroupBox *horizontalBox = new QGroupBox (tr ("Default horizontal error"), this);
  QHBoxLayout *horizontalBoxLayout = new QHBoxLayout;
  horizontalBox->setLayout (horizontalBoxLayout);
  horizontalBoxLayout->setSpacing (10);

  horizontal = new QDoubleSpinBox (this);
  horizontal->setDecimals (2);
  horizontal->setRange (0.0, 100.0);
  horizontal->setSingleStep (5.0);
  horizontal->setValue (pfm_global->horizontal_error);
  horizontal->setWrapping (TRUE);
  horizontal->setToolTip (tr ("Set the default horizontal error value"));
  horizontal->setWhatsThis (horizontalText);
  horizontalBoxLayout->addWidget (horizontal);


  hvBoxLayout->addWidget (horizontalBox);


  QGroupBox *verticalBox = new QGroupBox (tr ("Default vertical error"), this);
  QHBoxLayout *verticalBoxLayout = new QHBoxLayout;
  verticalBox->setLayout (verticalBoxLayout);
  verticalBoxLayout->setSpacing (10);

  vertical = new QDoubleSpinBox (this);
  vertical->setDecimals (2);
  vertical->setRange (0.0, 100.0);
  vertical->setSingleStep (5.0);
  vertical->setValue (pfm_global->vertical_error);
  vertical->setWrapping (TRUE);
  vertical->setToolTip (tr ("Set the default vertical error value"));
  vertical->setWhatsThis (verticalText);
  verticalBoxLayout->addWidget (vertical);


  hvBoxLayout->addWidget (verticalBox);

  vbox->addWidget (hvBox, 1);


  registerField ("hofOld", hofOld);
  registerField ("hofNull", hofNull);
  registerField ("subPath", subPath);
  registerField ("invLand", invLand);
  registerField ("invSec", invSec);
  registerField ("refData", refData);
  registerField ("fileCheck", fileCheck);
  registerField ("tofRef", tofRef);
  registerField ("hofFlag", hofFlag);
  registerField ("gsfNominal", gsfNominal);
  registerField ("srtmBest", srtmBest);
  registerField ("srtmOne", srtmOne);
  registerField ("srtmThree", srtmThree);
  registerField ("srtmThirty", srtmThirty);
  registerField ("srtmRef", srtmRef);
  registerField ("srtm2Ex", srtm2Ex);
  registerField ("cubeLoad", cubeLoad);
  registerField ("cubeAttributes", cubeAttributes);

  registerField ("horizontal", horizontal, "value", "valueChanged");
  registerField ("vertical", vertical, "value", "valueChanged");
}



void
globalPage::slotHofOldClicked (int state)
{
  if (state)
    {
      invSec->setEnabled (FALSE);

      QMessageBox::warning (this, tr ("pfmLoadM GCS Compatible Mode"),
                            tr ("Do not use GCS compatible mode unless you are going to reprocess the data in the PFM using Optech's GCS program!"));
    }
  else
    {
      invSec->setEnabled (TRUE);
    }
}



void
globalPage::slotTimeCheckClicked (bool state)
{
  //  If we unset the time attribute check box we need to adjust al of the already selected attributes.

  if (!state)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->time_attribute_num;
      pfm_global->time_attribute_num = 0;


      //  Move all lower attributes up one and decrement the counter.

      move_and_decrement (selected);
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          timeCheck->setChecked (FALSE);

          QMessageBox::warning (this, tr ("pfmLoadM Time Attribute"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->time_attribute_num = pfm_global->attribute_count;
        }
    }
}



void
globalPage::slotGSFClicked ()
{
  if (global_dialog->gsfD) global_dialog->gsfD->close ();

  global_dialog->gsfD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->gsfD->setWindowTitle (tr ("pfmLoadM GSF Attributes"));

  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->gsfD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  gsfGrp = new QButtonGroup (global_dialog->gsfD);
  gsfGrp->setExclusive (FALSE);
  connect (gsfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotGSFAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      gsf_attr[i] = new QCheckBox (pfm_global->gsf_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->gsf_attribute_name[i] + tr (" attribute");
      gsf_attr[i]->setToolTip (string);
      gsfGrp->addButton (gsf_attr[i], i);

      if (pfm_global->gsf_attribute_num[i]) gsf_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) gsf_attr[i]->setEnabled (FALSE);

      vbox->addWidget (gsf_attr[i]);
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->gsfD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the GSF attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearGSF ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->gsfD);
  closeButton->setToolTip (tr ("Close the GSF attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseGSF ()));
  actions->addWidget (closeButton);

  global_dialog->gsfD->show ();
}



void 
globalPage::slotGSFAttrClicked (int index)
{
  //  If we unset a GSF attribute check box we need to adjust all of the already selected attributes.

  if (gsf_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->gsf_attribute_num[index];

      for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
        {
          if (pfm_global->gsf_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->gsf_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (gsfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotGSFAttrClicked (int)));
          gsf_attr[index]->setChecked (FALSE);
          connect (gsfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotGSFAttrClicked (int)));

          QMessageBox::warning (global_dialog->gsfD, tr ("pfmLoadM GSF Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->gsf_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = gsfAttributesSet (*pfm_global)))
    {
      gsfPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      gsfPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      gsf->setPalette (gsfPalette);
      QString attrString = tr ("GSF (%1)").arg (cnt);
      gsf->setText (attrString);
    }
  else
    {
      gsfPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      gsfPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      gsf->setPalette (gsfPalette);
      gsf->setText (tr ("GSF"));
    }
}



void 
globalPage::slotClearGSF ()
{
  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      if (pfm_global->gsf_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->gsf_attribute_num[i];

          pfm_global->gsf_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (gsfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotGSFAttrClicked (int)));
          gsf_attr[i]->setChecked (FALSE);
          connect (gsfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotGSFAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  gsfPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  gsfPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  gsf->setPalette (gsfPalette);
  gsf->setText (tr ("GSF"));
}



void 
globalPage::slotCloseGSF ()
{
  global_dialog->gsfD->close ();
}



void
globalPage::slotHOFClicked ()
{
  if (global_dialog->hofD) global_dialog->hofD->close ();

  global_dialog->hofD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->hofD->setWindowTitle (tr ("pfmLoadM HOF Attributes"));


  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->hofD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setMargin (5);
  hbox->setSpacing (5);
  QVBoxLayout *vboxLLayout = new QVBoxLayout;
  QVBoxLayout *vboxRLayout = new QVBoxLayout;
  hbox->addLayout (vboxLLayout);
  hbox->addLayout (vboxRLayout);


  vbox->addLayout (hbox);


  hofGrp = new QButtonGroup (global_dialog->hofD);
  hofGrp->setExclusive (FALSE);
  connect (hofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHOFAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      hof_attr[i] = new QCheckBox (pfm_global->hof_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->hof_attribute_name[i] + tr (" attribute");
      hof_attr[i]->setToolTip (string);
      hofGrp->addButton (hof_attr[i], i);

      if (pfm_global->hof_attribute_num[i]) hof_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) hof_attr[i]->setEnabled (FALSE);

      if (i < HOF_ATTRIBUTES / 2)
        {
          vboxLLayout->addWidget (hof_attr[i]);
        }
      else
        {
          vboxRLayout->addWidget (hof_attr[i]);
        }
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->hofD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the HOF attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearHOF ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->hofD);
  closeButton->setToolTip (tr ("Close the HOF attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseHOF ()));
  actions->addWidget (closeButton);


  global_dialog->hofD->show ();
}



void 
globalPage::slotHOFAttrClicked (int index)
{
  //  If we unset a HOF attribute check box we need to adjust all of the already selected attributes.

  if (hof_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->hof_attribute_num[index];

      for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
        {
          if (pfm_global->hof_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->hof_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (hofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHOFAttrClicked (int)));
          hof_attr[index]->setChecked (FALSE);
          connect (hofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHOFAttrClicked (int)));

          QMessageBox::warning (global_dialog->hofD, tr ("pfmLoadM HOF Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->hof_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = hofAttributesSet (*pfm_global)))
    {
      hofPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      hofPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      hof->setPalette (hofPalette);
      QString attrString = tr ("HOF (%1)").arg (cnt);
      hof->setText (attrString);
    }
  else
    {
      hofPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      hofPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      hof->setPalette (hofPalette);
      hof->setText (tr ("HOF"));
    }
}



void 
globalPage::slotClearHOF ()
{
  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      if (pfm_global->hof_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->hof_attribute_num[i];

          pfm_global->hof_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (hofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHOFAttrClicked (int)));
          hof_attr[i]->setChecked (FALSE);
          connect (hofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHOFAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  hofPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  hofPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  hof->setPalette (hofPalette);
  hof->setText (tr ("HOF"));
}



void 
globalPage::slotCloseHOF ()
{
  global_dialog->hofD->close ();
}



void
globalPage::slotTOFClicked ()
{
  if (global_dialog->tofD) global_dialog->tofD->close ();

  global_dialog->tofD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->tofD->setWindowTitle (tr ("pfmLoadM TOF Attributes"));

  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->tofD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  tofGrp = new QButtonGroup (global_dialog->tofD);
  tofGrp->setExclusive (FALSE);
  connect (tofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTOFAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      tof_attr[i] = new QCheckBox (pfm_global->tof_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->tof_attribute_name[i] + tr (" attribute");
      tof_attr[i]->setToolTip (string);
      tofGrp->addButton (tof_attr[i], i);

      if (pfm_global->tof_attribute_num[i]) tof_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) tof_attr[i]->setEnabled (FALSE);

      vbox->addWidget (tof_attr[i]);
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->tofD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the TOF attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearTOF ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->tofD);
  closeButton->setToolTip (tr ("Close the TOF attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseTOF ()));
  actions->addWidget (closeButton);


  global_dialog->tofD->show ();
}



void 
globalPage::slotTOFAttrClicked (int index)
{
  //  If we unset a TOF attribute check box we need to adjust all of the already selected attributes.

  if (tof_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->tof_attribute_num[index];

      for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
        {
          if (pfm_global->tof_attribute_num[i])
            {
              if (index == i)
                {
                  pfm_global->tof_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (tofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTOFAttrClicked (int)));
          tof_attr[index]->setChecked (FALSE);
          connect (tofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTOFAttrClicked (int)));

          QMessageBox::warning (global_dialog->tofD, tr ("pfmLoadM TOF Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->tof_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = tofAttributesSet (*pfm_global)))
    {
      tofPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      tofPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      tof->setPalette (tofPalette);
      QString attrString = tr ("TOF (%1)").arg (cnt);
      tof->setText (attrString);
    }
  else
    {
      tofPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      tofPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      tof->setPalette (tofPalette);
      tof->setText (tr ("TOF"));
    }
}



void 
globalPage::slotClearTOF ()
{
  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      if (pfm_global->tof_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->tof_attribute_num[i];

          pfm_global->tof_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (tofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTOFAttrClicked (int)));
          tof_attr[i]->setChecked (FALSE);
          connect (tofGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotTOFAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  tofPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  tofPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  tof->setPalette (tofPalette);
  tof->setText (tr ("TOF"));
}



void 
globalPage::slotCloseTOF ()
{
  global_dialog->tofD->close ();
}



void
globalPage::slotWLFClicked ()
{
  if (global_dialog->wlfD) global_dialog->wlfD->close ();

  global_dialog->wlfD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->wlfD->setWindowTitle (tr ("pfmLoadM WLF Attributes"));

  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->wlfD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  wlfGrp = new QButtonGroup (global_dialog->wlfD);
  wlfGrp->setExclusive (FALSE);
  connect (wlfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWLFAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      wlf_attr[i] = new QCheckBox (pfm_global->wlf_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->wlf_attribute_name[i] + tr (" attribute");
      wlf_attr[i]->setToolTip (string);
      wlfGrp->addButton (wlf_attr[i], i);

      if (pfm_global->wlf_attribute_num[i]) wlf_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) wlf_attr[i]->setEnabled (FALSE);

      vbox->addWidget (wlf_attr[i]);
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->wlfD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the WLF attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearWLF ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->wlfD);
  closeButton->setToolTip (tr ("Close the WLF attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseWLF ()));
  actions->addWidget (closeButton);


  global_dialog->wlfD->show ();
}



void 
globalPage::slotWLFAttrClicked (int index)
{
  //  If we unset a WLF attribute check box we need to adjust all of the already selected attributes.

  if (wlf_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->wlf_attribute_num[index];

      for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
        {
          if (pfm_global->wlf_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->wlf_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (wlfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWLFAttrClicked (int)));
          wlf_attr[index]->setChecked (FALSE);
          connect (wlfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWLFAttrClicked (int)));

          QMessageBox::warning (global_dialog->wlfD, tr ("pfmLoadM WLF Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->wlf_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = wlfAttributesSet (*pfm_global)))
    {
      wlfPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      wlfPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      wlf->setPalette (wlfPalette);
      QString attrString = tr ("WLF (%1)").arg (cnt);
      wlf->setText (attrString);
    }
  else
    {
      wlfPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      wlfPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      wlf->setPalette (wlfPalette);
      wlf->setText (tr ("WLF"));
    }
}



void 
globalPage::slotClearWLF ()
{
  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      if (pfm_global->wlf_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->wlf_attribute_num[i];

          pfm_global->wlf_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (wlfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWLFAttrClicked (int)));
          wlf_attr[i]->setChecked (FALSE);
          connect (wlfGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotWLFAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  wlfPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  wlfPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  wlf->setPalette (wlfPalette);
  wlf->setText (tr ("WLF"));
}



void 
globalPage::slotCloseWLF ()
{
  global_dialog->wlfD->close ();
}



void
globalPage::slotCZMILClicked ()
{
  if (global_dialog->czmilD) global_dialog->czmilD->close ();

  global_dialog->czmilD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->czmilD->setWindowTitle (tr ("pfmLoadM CZMIL Attributes"));

  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->czmilD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  czmilGrp = new QButtonGroup (global_dialog->czmilD);
  czmilGrp->setExclusive (FALSE);
  connect (czmilGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotCZMILAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      czmil_attr[i] = new QCheckBox (pfm_global->czmil_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->czmil_attribute_name[i] + tr (" attribute");
      czmil_attr[i]->setToolTip (string);
      czmilGrp->addButton (czmil_attr[i], i);

      if (pfm_global->czmil_attribute_num[i]) czmil_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) czmil_attr[i]->setEnabled (FALSE);

      vbox->addWidget (czmil_attr[i]);
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->czmilD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the CZMIL attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearCZMIL ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->czmilD);
  closeButton->setToolTip (tr ("Close the CZMIL attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseCZMIL ()));
  actions->addWidget (closeButton);


  global_dialog->czmilD->show ();
}



void 
globalPage::slotCZMILAttrClicked (int index)
{
  //  If we unset a CZMIL attribute check box we need to adjust all of the already selected attributes.

  if (czmil_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->czmil_attribute_num[index];

      for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
        {
          if (pfm_global->czmil_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->czmil_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (czmilGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotCZMILAttrClicked (int)));
          czmil_attr[index]->setChecked (FALSE);
          connect (czmilGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotCZMILAttrClicked (int)));

          QMessageBox::warning (global_dialog->czmilD, tr ("pfmLoadM CZMIL Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->czmil_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = czmilAttributesSet (*pfm_global)))
    {
      czmilPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      czmilPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      czmil->setPalette (czmilPalette);
      QString attrString = tr ("CZMIL (%1)").arg (cnt);
      czmil->setText (attrString);
    }
  else
    {
      czmilPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      czmilPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      czmil->setPalette (czmilPalette);
      czmil->setText (tr ("CZMIL"));
    }
}



void 
globalPage::slotClearCZMIL ()
{
  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      if (pfm_global->czmil_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->czmil_attribute_num[i];

          pfm_global->czmil_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (czmilGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotCZMILAttrClicked (int)));
          czmil_attr[i]->setChecked (FALSE);
          connect (czmilGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotCZMILAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  czmilPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  czmilPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  czmil->setPalette (czmilPalette);
  czmil->setText (tr ("CZMIL"));
}



void 
globalPage::slotCloseCZMIL ()
{
  global_dialog->czmilD->close ();
}



void
globalPage::slotBAGClicked ()
{
  if (global_dialog->bagD) global_dialog->bagD->close ();

  global_dialog->bagD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->bagD->setWindowTitle (tr ("pfmLoadM BAG Attributes"));

  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->bagD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  bagGrp = new QButtonGroup (global_dialog->bagD);
  bagGrp->setExclusive (FALSE);
  connect (bagGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBAGAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      bag_attr[i] = new QCheckBox (pfm_global->bag_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->bag_attribute_name[i] + tr (" attribute");
      bag_attr[i]->setToolTip (string);
      bagGrp->addButton (bag_attr[i], i);

      if (pfm_global->bag_attribute_num[i]) bag_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) bag_attr[i]->setEnabled (FALSE);

      vbox->addWidget (bag_attr[i]);
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->bagD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the BAG attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearBAG ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->bagD);
  closeButton->setToolTip (tr ("Close the BAG attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseBAG ()));
  actions->addWidget (closeButton);


  global_dialog->bagD->show ();
}



void 
globalPage::slotBAGAttrClicked (int index)
{
  //  If we unset a BAG attribute check box we need to adjust all of the already selected attributes.

  if (bag_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->bag_attribute_num[index];

      for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
        {
          if (pfm_global->bag_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->bag_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (bagGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBAGAttrClicked (int)));
          bag_attr[index]->setChecked (FALSE);
          connect (bagGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBAGAttrClicked (int)));

          QMessageBox::warning (global_dialog->bagD, tr ("pfmLoadM BAG Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->bag_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = bagAttributesSet (*pfm_global)))
    {
      bagPalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      bagPalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      bag->setPalette (bagPalette);
      QString attrString = tr ("BAG (%1)").arg (cnt);
      bag->setText (attrString);
    }
  else
    {
      bagPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      bagPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      bag->setPalette (bagPalette);
      bag->setText (tr ("BAG"));
    }
}



void 
globalPage::slotClearBAG ()
{
  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      if (pfm_global->bag_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->bag_attribute_num[i];

          pfm_global->bag_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (bagGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBAGAttrClicked (int)));
          bag_attr[i]->setChecked (FALSE);
          connect (bagGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotBAGAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  bagPalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  bagPalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  bag->setPalette (bagPalette);
  bag->setText (tr ("BAG"));
}



void 
globalPage::slotCloseBAG ()
{
  global_dialog->bagD->close ();
}



void
globalPage::slotHAWKEYEClicked ()
{
  if (global_dialog->hawkeyeD) global_dialog->hawkeyeD->close ();

  global_dialog->hawkeyeD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->hawkeyeD->setWindowTitle (tr ("pfmLoadM HAWKEYE Attributes"));


  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->hawkeyeD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setMargin (5);
  hbox->setSpacing (5);
  QVBoxLayout *vboxLLayout = new QVBoxLayout;
  QVBoxLayout *vboxRLayout = new QVBoxLayout;
  hbox->addLayout (vboxLLayout);
  hbox->addLayout (vboxRLayout);


  vbox->addLayout (hbox);


  hawkeyeGrp = new QButtonGroup (global_dialog->hawkeyeD);
  hawkeyeGrp->setExclusive (FALSE);
  connect (hawkeyeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHAWKEYEAttrClicked (int)));

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      hawkeye_attr[i] = new QCheckBox (pfm_global->hawkeye_attribute_name[i]);
      QString string = tr("Toggle ") + pfm_global->hawkeye_attribute_name[i] + tr (" attribute");
      hawkeye_attr[i]->setToolTip (string);
      hawkeyeGrp->addButton (hawkeye_attr[i], i);

      if (pfm_global->hawkeye_attribute_num[i]) hawkeye_attr[i]->setChecked (TRUE);

      if (pfm_global->appending) hawkeye_attr[i]->setEnabled (FALSE);

      if (i < HAWKEYE_ATTRIBUTES / 2)
        {
          vboxLLayout->addWidget (hawkeye_attr[i]);
        }
      else
        {
          vboxRLayout->addWidget (hawkeye_attr[i]);
        }
    }
  

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *clearButton = new QPushButton (tr ("Clear"), global_dialog->hawkeyeD);
  clearButton->setToolTip (tr ("Clear all selected attributes in the HAWKEYE attribute dialog"));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (slotClearHAWKEYE ()));
  actions->addWidget (clearButton);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->hawkeyeD);
  closeButton->setToolTip (tr ("Close the HAWKEYE attribute dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseHAWKEYE ()));
  actions->addWidget (closeButton);


  global_dialog->hawkeyeD->show ();
}



void 
globalPage::slotHAWKEYEAttrClicked (int index)
{
  //  If we unset a HAWKEYE attribute check box we need to adjust all of the already selected attributes.

  if (hawkeye_attr[index]->checkState () != Qt::Checked)
    {
      //  Find out which attribute number it was set to and zero it out.

      NV_INT16 selected = pfm_global->hawkeye_attribute_num[index];

      for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
        {
          if (pfm_global->hawkeye_attribute_num[i])
            {
              if (i == index)
                {
                  pfm_global->hawkeye_attribute_num[i] = 0;


                  //  Move all lower attributes up one and decrement the counter.

                  move_and_decrement (selected);
                  break;
                }
            }
        }
    }
  else
    {
      //  Make sure we don't exceed our allowable number of depth attributes.

      if (pfm_global->attribute_count >= NUM_ATTR)
        {
          disconnect (hawkeyeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHAWKEYEAttrClicked (int)));
          hawkeye_attr[index]->setChecked (FALSE);
          connect (hawkeyeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHAWKEYEAttrClicked (int)));

          QMessageBox::warning (global_dialog->hawkeyeD, tr ("pfmLoadM HAWKEYE Attributes"), tr ("Only ten index attributes are allowed"));
        }
      else
        {
          //  Increment the attribute count and set the attribute number for this attribute.

          pfm_global->attribute_count++;
          pfm_global->hawkeye_attribute_num[index] = pfm_global->attribute_count;
        }
    }


  //  Set the number of attributes set (if any) and set the proper color for the button.

  NV_INT32 cnt = 0;
  if ((cnt = hawkeyeAttributesSet (*pfm_global)))
    {
      hawkeyePalette.setColor (QPalette::Normal, QPalette::Button, Qt::red);
      hawkeyePalette.setColor (QPalette::Inactive, QPalette::Button, Qt::red);
      hawkeye->setPalette (hawkeyePalette);
      QString attrString = tr ("HAWKEYE (%1)").arg (cnt);
      hawkeye->setText (attrString);
    }
  else
    {
      hawkeyePalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
      hawkeyePalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
      hawkeye->setPalette (hawkeyePalette);
      hawkeye->setText (tr ("HAWKEYE"));
    }
}



void 
globalPage::slotClearHAWKEYE ()
{
  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      if (pfm_global->hawkeye_attribute_num[i])
        {
          NV_INT32 selected = pfm_global->hawkeye_attribute_num[i];

          pfm_global->hawkeye_attribute_num[i] = 0;

          move_and_decrement (selected);


          //  Now clear the check box.

          disconnect (hawkeyeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHAWKEYEAttrClicked (int)));
          hawkeye_attr[i]->setChecked (FALSE);
          connect (hawkeyeGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotHAWKEYEAttrClicked (int)));
        }
    }


  //  Clear the number of attributes set and set the proper color for the button.

  hawkeyePalette.setColor (QPalette::Normal, QPalette::Button, widgetBackgroundColor);
  hawkeyePalette.setColor (QPalette::Inactive, QPalette::Button, widgetBackgroundColor);
  hawkeye->setPalette (hawkeyePalette);
  hawkeye->setText (tr ("HAWKEYE"));
}



void 
globalPage::slotCloseHAWKEYE ()
{
  global_dialog->hawkeyeD->close ();
}



void
globalPage::slotOptClicked (int id)
{
  switch (id)
    {
    case 1:

      //  If we unchecked the Cube attributes box we have to unforce Run CUBE.

      if (!cubeAttributes->isChecked ())
        {
          cubeLoad->setChecked (FALSE);
	  if (global_dialog->cubeD != NULL) global_dialog->cubeD->close ();
        }
      break;

    case 2:

      //  If we checked the Run CUBE box we have to force the CUBE attributes.

      if (cubeLoad->isChecked ())
        {
          cubeAttributes->setChecked (TRUE);
	  cubeDialog ();
        }
      else
        {
	  if (global_dialog->cubeD != NULL) global_dialog->cubeD->close ();
        }
      break;

    case 10:
      if (srtmBest->isChecked ())
        {
          if (srtm2 && !flags->srtme) 
            QMessageBox::information (this, tr ("pfmLoadM SRTM2 data"),
                                      tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n\n") +
                                      tr ("If your PFM file includes any areas outside of the United States it will be limited ") +
                                      tr ("distribution due to the inclusion of NGA SRTM2 topography data.  If you do not want to ") +
                                      tr ("include the SRTM2 data please select 'Exclude SRTM2 data' from the 'Optional flag settings'.\n\n") +
                                      tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING"));

          flags->srtmb = NVTrue;

          flags->srtm1 = NVFalse;
          flags->srtm3 = NVFalse;
          flags->srtm30 = NVFalse;
          srtmOne->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }
      else
        {
          flags->srtmb = NVFalse;
        }
      break;

    case 11:
      if (srtmOne->isChecked ())
        {
          if (srtm2 && !flags->srtme) 
            QMessageBox::information (this, tr ("pfmLoadM SRTM2 data"),
                                      tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n\n") +
                                      tr ("If your PFM file includes any areas outside of the United States it will be limited ") +
                                      tr ("distribution due to the inclusion of NGA SRTM2 topography data.  If you do not want to ") +
                                      tr ("include the SRTM2 data please select 'Exclude SRTM2 data' from the 'Optional flag settings'.\n\n") +
                                      tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING"));

          flags->srtm1 = NVTrue;

          flags->srtmb = NVFalse;
          flags->srtm3 = NVFalse;
          flags->srtm30 = NVFalse;
          srtmBest->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }
      else
        {
          flags->srtm1 = NVFalse;
        }
      break;

    case 12:
      if (srtmThree->isChecked ())
        {
          flags->srtm3 = NVTrue;

          flags->srtmb = NVFalse;
          flags->srtm1 = NVFalse;
          flags->srtm30 = NVFalse;
          srtmBest->setChecked (FALSE);
          srtmOne->setChecked (FALSE);
          srtmThirty->setChecked (FALSE);
        }
      else
        {
          flags->srtm3 = NVFalse;
        }
      break;

    case 13:
      if (srtmThirty->isChecked ())
        {
          flags->srtm30 = NVTrue;

          flags->srtmb = NVFalse;
          flags->srtm1 = NVFalse;
          flags->srtm3 = NVFalse;
          srtmBest->setChecked (FALSE);
          srtmOne->setChecked (FALSE);
          srtmThree->setChecked (FALSE);
        }
      else
        {
          flags->srtm30 = NVFalse;
        }
      break;

    case 15:
      if (srtm2Ex->isChecked ())
        {
          flags->srtme = NVTrue;
        }
      else
        {
          flags->srtme = NVFalse;
          if (srtmBest->isChecked () || srtmOne->isChecked ())
            {
              QMessageBox::information (this, tr ("pfmLoadM SRTM2 data"),
                                        tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING\n\n") +
                                        tr ("If your PFM file includes any areas outside of the United States it will be limited ") +
                                        tr ("distribution due to the inclusion of NGA SRTM2 topography data.  If you do not want to ") +
                                        tr ("include the SRTM2 data please select 'Exclude SRTM2 data' from the 'Optional flag settings'.\n\n") +
                                        tr ("WARNING WARNING WARNING WARNING WARNING WARNING WARNING"));
            }
        }
      break;
    }
}



void 
globalPage::setFields (PFM_GLOBAL *pfm_glb, FLAGS *fl)
{
  setField ("hofOld", fl->old);
  setField ("hofNull", fl->hof);
  setField ("subPath", fl->sub);
  setField ("invLand", fl->lnd);
  setField ("invSec", fl->sec);
  setField ("refData", fl->ref);
  setField ("fileCheck", fl->chk);
  setField ("tofRef", fl->tof);
  setField ("hofFlag", fl->lid);
  setField ("gsfNominal", fl->nom);
  setField ("srtmBest", fl->srtmb);
  setField ("srtmOne", fl->srtm1);
  setField ("srtmThree", fl->srtm3);
  setField ("srtmThirty", fl->srtm30);
  setField ("srtmRef", fl->srtmr);
  setField ("cubeLoad", fl->cube);
  setField ("cubeAttributes", fl->attr);
  setField ("horizontal", pfm_glb->horizontal_error);
  setField ("vertical", pfm_glb->vertical_error);

  if (fl->old)
    {
      invSec->setEnabled (FALSE);
    }
  else
    {
      invSec->setEnabled (TRUE);
    }


  if (fl->cube) cubeDialog ();
}



void 
globalPage::cubeDialog ()
{
  if (global_dialog->cubeD) global_dialog->cubeD->close ();

  global_dialog->cubeD = new QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  global_dialog->cubeD->setWindowTitle (tr ("pfmLoadM CUBE Options"));


  QVBoxLayout *vbox = new QVBoxLayout (global_dialog->cubeD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *cubeBox = new QGroupBox (tr ("CUBE parameters"), this);
  QVBoxLayout *cubeBoxLayout = new QVBoxLayout;
  cubeBox->setLayout (cubeBoxLayout);
  cubeBoxLayout->setSpacing (10);
  QHBoxLayout *cubeTopLayout = new QHBoxLayout;
  QHBoxLayout *cubeBotLayout = new QHBoxLayout;
  cubeBoxLayout->addLayout (cubeTopLayout);
  cubeBoxLayout->addLayout (cubeBotLayout);
  cubeBox->setWhatsThis (cubeBoxText);


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
  ihoOrder->setCurrentIndex (pfm_global->iho);
  connect (ihoOrder, SIGNAL (currentIndexChanged (int)), this, SLOT (slotIhoOrderChanged (int)));
  ihoBoxLayout->addWidget (ihoOrder);


  cubeTopLayout->addWidget (ihoBox);


  QGroupBox *captureBox = new QGroupBox (tr ("Capture percentage"), this);
  QHBoxLayout *captureBoxLayout = new QHBoxLayout;
  captureBox->setLayout (captureBoxLayout);
  captureBoxLayout->setSpacing (10);

  capture = new QDoubleSpinBox (captureBox);
  capture->setDecimals (1);
  capture->setRange (1.0, 10.0);
  capture->setSingleStep (1.0);
  capture->setValue (pfm_global->capture);
  capture->setWrapping (TRUE);
  capture->setToolTip (tr ("Set the capture percentage for the CUBE process"));
  capture->setValue (pfm_global->capture);
  connect (capture, SIGNAL (valueChanged (double)), this, SLOT (slotCaptureChanged (double)));
  captureBoxLayout->addWidget (capture);


  cubeTopLayout->addWidget (captureBox);


  QGroupBox *queueBox = new QGroupBox (tr ("Queue length"), this);
  QHBoxLayout *queueBoxLayout = new QHBoxLayout;
  queueBox->setLayout (queueBoxLayout);
  queueBoxLayout->setSpacing (10);

  queue = new QSpinBox (queueBox);
  queue->setRange (1, 20);
  queue->setSingleStep (1);
  queue->setValue (pfm_global->queue);
  queue->setWrapping (TRUE);
  queue->setToolTip (tr ("Set the queue length for the CUBE process"));
  queue->setValue (pfm_global->queue);
  connect (queue, SIGNAL (valueChanged (int)), this, SLOT (slotQueueChanged (int)));
  queueBoxLayout->addWidget (queue);


  cubeTopLayout->addWidget (queueBox);


  QGroupBox *horizBox = new QGroupBox (tr ("Horizontal position uncertainty"), this);
  QHBoxLayout *horizBoxLayout = new QHBoxLayout;
  horizBox->setLayout (horizBoxLayout);
  horizBoxLayout->setSpacing (10);

  horiz = new QDoubleSpinBox (horizBox);
  horiz->setDecimals (1);
  horiz->setRange (0.0, 100.0);
  horiz->setSingleStep (1.0);
  horiz->setValue (pfm_global->horiz);
  horiz->setWrapping (TRUE);
  horiz->setToolTip (tr ("Set the horizontal position uncertainty for the CUBE process"));
  horiz->setValue (pfm_global->horiz);
  connect (horiz, SIGNAL (valueChanged (double)), this, SLOT (slotHorizChanged (double)));
  horizBoxLayout->addWidget (horiz);


  cubeTopLayout->addWidget (horizBox);


  QGroupBox *distanceBox = new QGroupBox (tr ("Distance exponent"), this);
  QHBoxLayout *distanceBoxLayout = new QHBoxLayout;
  distanceBox->setLayout (distanceBoxLayout);
  distanceBoxLayout->setSpacing (10);

  distance = new QDoubleSpinBox (distanceBox);
  distance->setDecimals (1);
  distance->setRange (0.0, 4.0);
  distance->setSingleStep (1.0);
  distance->setValue (pfm_global->distance);
  distance->setWrapping (TRUE);
  distance->setToolTip (tr ("Set the distance exponent for the CUBE process"));
  distance->setValue (pfm_global->distance);
  distanceBoxLayout->addWidget (distance);
  connect (distance, SIGNAL (valueChanged (double)), this, SLOT (slotDistanceChanged (double)));


  cubeBotLayout->addWidget (distanceBox);


  QGroupBox *minContextBox = new QGroupBox (tr ("Minimum context"), this);
  QHBoxLayout *minContextBoxLayout = new QHBoxLayout;
  minContextBox->setLayout (minContextBoxLayout);
  minContextBoxLayout->setSpacing (10);

  minContext = new QDoubleSpinBox (minContextBox);
  minContext->setDecimals (1);
  minContext->setRange (0.0, 10.0);
  minContext->setSingleStep (1.0);
  minContext->setValue (pfm_global->min_context);
  minContext->setWrapping (TRUE);
  minContext->setToolTip (tr ("Set the minimum context for the CUBE process"));
  minContext->setValue (pfm_global->min_context);
  connect (minContext, SIGNAL (valueChanged (double)), this, SLOT (slotMinContextChanged (double)));
  minContextBoxLayout->addWidget (minContext);


  cubeBotLayout->addWidget (minContextBox);


  QGroupBox *maxContextBox = new QGroupBox (tr ("Maximum context"), this);
  QHBoxLayout *maxContextBoxLayout = new QHBoxLayout;
  maxContextBox->setLayout (maxContextBoxLayout);
  maxContextBoxLayout->setSpacing (10);

  maxContext = new QDoubleSpinBox (maxContextBox);
  maxContext->setDecimals (1);
  maxContext->setRange (0.0, 10.0);
  maxContext->setSingleStep (1.0);
  maxContext->setValue (pfm_global->max_context);
  maxContext->setWrapping (TRUE);
  maxContext->setToolTip (tr ("Set the maximum context for the CUBE process"));
  maxContext->setValue (pfm_global->max_context);
  connect (maxContext, SIGNAL (valueChanged (double)), this, SLOT (slotMaxContextChanged (double)));
  maxContextBoxLayout->addWidget (maxContext);


  cubeBotLayout->addWidget (maxContextBox);


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
  std2conf->setCurrentIndex (pfm_global->std2conf);
  connect (std2conf, SIGNAL (currentIndexChanged (int)), this, SLOT (slotStd2confChanged (int)));
  std2confBoxLayout->addWidget (std2conf);


  cubeBotLayout->addWidget (std2confBox);


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
  disambiguation->setCurrentIndex (pfm_global->disambiguation);
  connect (disambiguation, SIGNAL (currentIndexChanged (int)), this, SLOT (slotDisambiguationChanged (int)));
  disBoxLayout->addWidget (disambiguation);


  cubeBotLayout->addWidget (disBox);


  vbox->addWidget (cubeBox, 1);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *closeButton = new QPushButton (tr ("Close"), global_dialog->cubeD);
  closeButton->setToolTip (tr ("Close the CUBE options dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseCube ()));
  actions->addWidget (closeButton);


  global_dialog->cubeD->show ();
}



void 
globalPage::slotIhoOrderChanged (int index)
{
  pfm_global->iho = index;
}



void 
globalPage::slotCaptureChanged (double value)
{
  pfm_global->capture = value;
}



void 
globalPage::slotQueueChanged (int value)
{
  pfm_global->queue = value;
}



void 
globalPage::slotHorizChanged (double value)
{
  pfm_global->horiz = value;
}



void 
globalPage::slotDistanceChanged (double value)
{
  pfm_global->distance = value;
}



void 
globalPage::slotMinContextChanged (double value)
{
  pfm_global->min_context = value;
}



void 
globalPage::slotMaxContextChanged (double value)
{
  pfm_global->max_context = value;
}



void 
globalPage::slotStd2confChanged (int index)
{
  pfm_global->std2conf = index;
}



void 
globalPage::slotDisambiguationChanged (int index)
{
  pfm_global->disambiguation = index;
}



void 
globalPage::slotCloseCube ()
{
  global_dialog->cubeD->close ();
}



void 
globalPage::move_and_decrement (NV_INT32 selected)
{
  //  Move all selected attributes that were lower than the selected slot up by one slot.

  if (pfm_global->time_attribute_num && pfm_global->time_attribute_num > selected) pfm_global->time_attribute_num--;

  for (NV_INT32 i = 0 ; i < GSF_ATTRIBUTES ; i++)
    {
      if (pfm_global->gsf_attribute_num[i] && pfm_global->gsf_attribute_num[i] > selected) pfm_global->gsf_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < HOF_ATTRIBUTES ; i++)
    {
      if (pfm_global->hof_attribute_num[i] && pfm_global->hof_attribute_num[i] > selected) pfm_global->hof_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < TOF_ATTRIBUTES ; i++)
    {
      if (pfm_global->tof_attribute_num[i] && pfm_global->tof_attribute_num[i] > selected) pfm_global->tof_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < WLF_ATTRIBUTES ; i++)
    {
      if (pfm_global->wlf_attribute_num[i] && pfm_global->wlf_attribute_num[i] > selected) pfm_global->wlf_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < CZMIL_ATTRIBUTES ; i++)
    {
      if (pfm_global->czmil_attribute_num[i] && pfm_global->czmil_attribute_num[i] > selected) pfm_global->czmil_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < BAG_ATTRIBUTES ; i++)
    {
      if (pfm_global->bag_attribute_num[i] && pfm_global->bag_attribute_num[i] > selected) pfm_global->bag_attribute_num[i]--;
    }

  for (NV_INT32 i = 0 ; i < HAWKEYE_ATTRIBUTES ; i++)
    {
      if (pfm_global->hawkeye_attribute_num[i] && pfm_global->hawkeye_attribute_num[i] > selected) pfm_global->hawkeye_attribute_num[i]--;
    }


  //  Decrement the attribute count.

  pfm_global->attribute_count--;
}
