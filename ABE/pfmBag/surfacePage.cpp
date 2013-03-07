#include "surfacePage.hpp"
#include "surfacePageHelp.hpp"

surfacePage::surfacePage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;
  cube = NVFalse;


  setTitle (tr ("BAG surface and metadata"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmBagWatermark.png"));

  surface = new QComboBox (this);
  surface->setWhatsThis (surfaceText);
  surface->setEditable (FALSE);
  surface->addItem (tr ("Minimum Surface"));
  surface->addItem (tr ("Maximum Surface"));
  surface->addItem (tr ("Average Surface"));
  surface->addItem (tr ("CUBE Surface"));
  surface->setCurrentIndex (options->surface);
  connect (surface, SIGNAL (currentIndexChanged (int)), this, SLOT (slotSurfaceChanged (int)));


  uncertainty = new QComboBox (this);
  uncertainty->setWhatsThis (uncertaintyText);
  uncertainty->setEditable (FALSE);
  uncertainty->addItem (tr ("None"));
  uncertainty->addItem (tr ("Standard Deviation"));
  uncertainty->addItem (tr ("Average TPE"));
  uncertainty->addItem (tr ("Final Uncertainty (Max of CUBE/Avg Standard Deviation and Average TPE)"));


  feature = new QCheckBox (this);
  feature->setToolTip (tr ("Use features to make an enhanced navigation surface"));
  feature->setWhatsThis (featureText);
  feature->setChecked (options->enhanced);
  connect (feature, SIGNAL (clicked ()), this, SLOT (slotFeatureClicked (void)));


  mBinSize = new QDoubleSpinBox (this);
  mBinSize->setDecimals (2);
  mBinSize->setRange (0.0, 1000.0);
  mBinSize->setSingleStep (1.0);
  mBinSize->setValue (options->mbin_size);
  mBinSize->setWrapping (TRUE);
  mBinSize->setToolTip (tr ("Set the BAG bin size in meters"));
  mBinSize->setWhatsThis (mBinSizeText);
  connect (mBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotMBinSizeChanged (double)));


  gBinSize = new QDoubleSpinBox (this);
  gBinSize->setDecimals (3);
  gBinSize->setRange (0.0, 200.0);
  gBinSize->setSingleStep (0.05);
  gBinSize->setValue (options->gbin_size);
  gBinSize->setWrapping (TRUE);
  gBinSize->setToolTip (tr ("Set the BAG bin size in minutes"));
  gBinSize->setWhatsThis (gBinSizeText);
  connect (gBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotGBinSizeChanged (double)));


  title = new QLineEdit (this);
  title->setToolTip (tr ("BAG title"));
  title->setWhatsThis (titleText);

  individualName = new QLineEdit (this);
  individualName->setToolTip (tr ("Certifying official's name"));
  individualName->setWhatsThis (individualNameText);
  individualName->setText (options->pi_name);

  positionName = new QLineEdit (this);
  positionName->setToolTip (tr ("Certifying official's position title"));
  positionName->setWhatsThis (positionNameText);
  positionName->setText (options->pi_title);

  individualName2 = new QLineEdit (this);
  individualName2->setToolTip (tr ("Point of contact's name"));
  individualName2->setWhatsThis (individualName2Text);
  individualName2->setText (options->poc_name);

  abstract = new QTextEdit (this);
  abstract->setWhatsThis (abstractText);


  formLayout = new QFormLayout;

  formLayout->addRow (tr ("&Surface:"), surface);
  formLayout->addRow (tr ("&Uncertainty source:"), uncertainty);
  formLayout->addRow (tr ("Use &features for enhanced surface:"), feature);
  formLayout->addRow (tr ("Bin size (meters):"), mBinSize);
  formLayout->addRow (tr ("Bin size (minutes):"), gBinSize);
  formLayout->addRow (tr ("&Title:"), title);
  formLayout->addRow (tr ("&Certifying official:"), individualName);
  formLayout->addRow (tr ("Certifying official &position:"), positionName);
  formLayout->addRow (tr ("&POC name:"), individualName2);
  formLayout->addRow (tr ("BAG Comments:"), abstract);
  setLayout (formLayout);


  registerField ("uncertainty", uncertainty);
  registerField ("title", title);
  registerField ("individualName", individualName);
  registerField ("positionName", positionName);
  registerField ("individualName2", individualName2);
  registerField ("abstract", abstract, "plainText");
}



void surfacePage::slotFeatureClicked ()
{
  if (feature->checkState ())
    {
      options->enhanced = NVTrue;
    }
  else
    {
      options->enhanced = NVFalse;
    }
}



void 
surfacePage::slotSurfaceChanged (int index)
{
  options->surface = index;

  if (options->surface != CUBE_SURFACE && options->surface != AVG_SURFACE) feature->setChecked (NVFalse);

  if (options->surface != CUBE_SURFACE && options->uncertainty == FIN_UNCERT) 
    {
      options->uncertainty = TPE_UNCERT;
      uncertainty->setCurrentIndex (options->uncertainty);
    }

  if (options->surface == CUBE_SURFACE)
    {
      options->projection = 0;
      options->uncertainty = FIN_UNCERT;
      uncertainty->setCurrentIndex (options->uncertainty);
      options->mbin_size = save_mbin;
      mBinSize->setValue (save_mbin);
      mBinSize->setEnabled (FALSE);
      gBinSize->setEnabled (FALSE);
    }
  else
    {
      mBinSize->setEnabled (TRUE);
      gBinSize->setEnabled (TRUE);
      if (options->uncertainty == FIN_UNCERT) options->uncertainty = TPE_UNCERT;
      uncertainty->setCurrentIndex (options->uncertainty);
    }
}



void 
surfacePage::setFields (OPTIONS *options)
{
  NV_INT32 pfm_handle;
  PFM_OPEN_ARGS open_args;

  strcpy (open_args.list_path, options->pfm_file_name.toAscii ());

  open_args.checkpoint = 0;
  pfm_handle = open_existing_pfm_file (&open_args);


  if (pfm_handle < 0)
    {
      QMessageBox::warning (this, tr ("Open PFM Structure"),
			    tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
			    tr (" is not a PFM structure or there was an error reading the file.") +
			    tr ("  The error message returned was:\n\n") +
			    QString (pfm_error_str (pfm_error)));
    }
  else
    {
      options->mbin_size = 2.0;
      options->gbin_size = 0.0;


      //  Set the bin size based on the bin size of the PFM

      prev_mbin = save_mbin = open_args.head.bin_size_xy;
      prev_gbin = open_args.head.y_bin_size_degrees;

      if (fabs (open_args.head.x_bin_size_degrees - open_args.head.y_bin_size_degrees) < 0.0000001)
        {
          gBinSize->setValue (open_args.head.y_bin_size_degrees);
        }
      else
        {
          mBinSize->setValue (open_args.head.bin_size_xy);
        }


      prev_mbin = options->mbin_size;
      prev_gbin = options->gbin_size;


      if (strstr (open_args.head.average_filt_name, "CUBE"))
        {
          cube = NVTrue;
        }
      else
        {
          //  Disable (but don't remove) the Final Uncertainty method if cube surface isn't available.
          //  Get the index of the value to disable

          QModelIndex index = uncertainty->model ()->index (CUBE_SURFACE, 0);
          QVariant v (0);
          uncertainty->model ()->setData (index, v, Qt::UserRole -1);

          if (options->uncertainty == FIN_UNCERT) options->uncertainty = STD_UNCERT;
        }


      //  Check for a feature file.

      if (!strcmp (open_args.target_path, "NONE"))
        {
          formLayout->itemAt (2, QFormLayout::LabelRole)->widget ()->setEnabled (FALSE);
          feature->setEnabled (FALSE);
          options->enhanced = NVFalse;
	  feature->setChecked (FALSE);
        }
      else
        {
          formLayout->itemAt (2, QFormLayout::LabelRole)->widget ()->setEnabled (TRUE);
          feature->setEnabled (TRUE);
        }


      slotSurfaceChanged (options->surface);


      close_pfm_file (pfm_handle);
    }
}



void 
surfacePage::slotMBinSizeChanged (double value)
{
  options->mbin_size = value;


  //  Disconnect the geographic bin size value changed signal so we don't bounce back and forth.  Reconnect after
  //  setting the value.

  disconnect (gBinSize, SIGNAL (valueChanged (double)), 0, 0);


  //  We don't ever want both gbin and mbin to be 0.0.

  if (fabs (value < 0.0000001))
    {
      options->gbin_size = prev_gbin;
    }
  else
    {
      prev_mbin = value;
      options->gbin_size = 0.0;
    }
  gBinSize->setValue (options->gbin_size);

  connect (gBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotGBinSizeChanged (double)));
}



void 
surfacePage::slotGBinSizeChanged (double value)
{
  options->gbin_size = value;


  //  Disconnect the meter bin size value changed signal so we don't bounce back and forth.  Reconnect after setting
  //  the value.

  disconnect (mBinSize, SIGNAL (valueChanged (double)), 0, 0);


  //  We don't ever want both gbin and mbin to be 0.0.

  if (fabs (value < 0.0000001))
    {
      options->mbin_size = prev_mbin;
    }
  else
    {
      prev_gbin = value;
      options->mbin_size = 0.0;
    }
  mBinSize->setValue (options->mbin_size);

  connect (mBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotMBinSizeChanged (double)));
}
