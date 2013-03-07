
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



#include "pfmPage.hpp"
#include "pfmPageHelp.hpp"


pfmPage::pfmPage (QWidget *parent, PFM_DEFINITION *pfmDef, PFM_GLOBAL *pfmg, NV_INT32 page_num):
  QWizardPage (parent)
{
  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmLoadWatermark.png"));

  pfm_def = pfmDef;
  pfm_global = pfmg;
  pfm_def->existing = NVFalse;
  l_page_num = page_num;
  pfmIndex.sprintf ("%02d", page_num - 1);
  prev_mbin = pfm_def->mbin_size;
  prev_gbin = pfm_def->gbin_size;

  QString title;
  title = tr ("PFM ") + pfmIndex + tr (" Options");
  setTitle (title);


  QVBoxLayout *pageLayout = new QVBoxLayout (this);
  pageLayout->setMargin (5);
  pageLayout->setSpacing (5);


  QHBoxLayout *pfm_file_box = new QHBoxLayout;
  pfm_file_box->setSpacing (5);

  pageLayout->addLayout (pfm_file_box);


  QString pfl = tr ("PFM file ") + pfmIndex;
  pfm_file_label = new QLabel (pfl, this);

  pfm_file_box->addWidget (pfm_file_label, 1);

  pfm_file_edit = new QLineEdit (this);
  pfm_file_edit->setToolTip (tr ("Set the PFM file name manually"));
  connect (pfm_file_edit, SIGNAL (textChanged (const QString &)), this, SLOT (slotPFMFileEdit (const QString &)));


  pfm_file_box->addWidget (pfm_file_edit, 10);

  pfm_file_browse = new QPushButton (tr ("Browse..."), this);
  pfm_file_browse->setToolTip (tr ("Select a preexisting PFM file to append to or create file in new directory"));

  pfm_file_label->setWhatsThis (pfm_fileText);
  pfm_file_edit->setWhatsThis (pfm_fileText);
  pfm_file_browse->setWhatsThis (pfm_fileBrowseText);

  connect (pfm_file_browse, SIGNAL (clicked ()), this, SLOT (slotPFMFileBrowse ()));

  pfm_file_box->addWidget (pfm_file_browse, 1);


  QGroupBox *limBox = new QGroupBox (tr ("Limits"), this);
  QHBoxLayout *limBoxLayout = new QHBoxLayout;
  limBox->setLayout (limBoxLayout);
  limBoxLayout->setSpacing (10);


  QGroupBox *mBinsBox = new QGroupBox (tr ("Bin size (meters)"), this);
  QHBoxLayout *mBinsBoxLayout = new QHBoxLayout;
  mBinsBox->setLayout (mBinsBoxLayout);
  mBinsBoxLayout->setSpacing (10);

  mBinSize = new QDoubleSpinBox (this);
  mBinSize->setDecimals (2);
  mBinSize->setRange (0.0, 1000.0);
  mBinSize->setSingleStep (1.0);
  mBinSize->setValue (pfm_def->mbin_size);
  mBinSize->setWrapping (TRUE);
  mBinSize->setToolTip (tr ("Set the PFM bin size in meters"));
  mBinSize->setWhatsThis (mBinSizeText);
  connect (mBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotMBinSizeChanged (double)));
  mBinsBoxLayout->addWidget (mBinSize);


  limBoxLayout->addWidget (mBinsBox);


  QGroupBox *gBinsBox = new QGroupBox (tr ("Bin size (minutes)"), this);
  QHBoxLayout *gBinsBoxLayout = new QHBoxLayout;
  gBinsBox->setLayout (gBinsBoxLayout);
  gBinsBoxLayout->setSpacing (10);

  gBinSize = new QDoubleSpinBox (this);
  gBinSize->setDecimals (3);
  gBinSize->setRange (0.0, 200.0);
  gBinSize->setSingleStep (0.05);
  gBinSize->setValue (pfm_def->gbin_size);
  gBinSize->setWrapping (TRUE);
  gBinSize->setToolTip (tr ("Set the PFM bin size in minutes"));
  gBinSize->setWhatsThis (gBinSizeText);
  connect (gBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotGBinSizeChanged (double)));
  gBinsBoxLayout->addWidget (gBinSize);


  limBoxLayout->addWidget (gBinsBox);


  QGroupBox *minDBox = new QGroupBox (tr ("Minimum depth"), this);
  QHBoxLayout *minDBoxLayout = new QHBoxLayout;
  minDBox->setLayout (minDBoxLayout);
  minDBoxLayout->setSpacing (10);

  minDepth = new QDoubleSpinBox (this);
  minDepth->setDecimals (1);
  minDepth->setRange (-10000.0, 12000.0);
  minDepth->setSingleStep (1000.0);
  minDepth->setValue (pfm_def->min_depth);
  minDepth->setWrapping (TRUE);
  minDepth->setToolTip (tr ("Set the minimum allowable depth for the PFM structure"));
  minDepth->setWhatsThis (minDepthText);
  minDBoxLayout->addWidget (minDepth);


  limBoxLayout->addWidget (minDBox);


  QGroupBox *maxDBox = new QGroupBox (tr ("Maximum depth"), this);
  QHBoxLayout *maxDBoxLayout = new QHBoxLayout;
  maxDBox->setLayout (maxDBoxLayout);
  maxDBoxLayout->setSpacing (10);

  maxDepth = new QDoubleSpinBox (this);
  maxDepth->setDecimals (1);
  maxDepth->setRange (-10000.0, 12000.0);
  maxDepth->setSingleStep (1000.0);
  maxDepth->setValue (pfm_def->max_depth);
  maxDepth->setWrapping (TRUE);
  maxDepth->setToolTip (tr ("Set the maximum allowable depth for the PFM structure"));
  maxDepth->setWhatsThis (maxDepthText);
  maxDBoxLayout->addWidget (maxDepth);


  limBoxLayout->addWidget (maxDBox);


  QGroupBox *precBox = new QGroupBox (tr ("Precision"), this);
  QHBoxLayout *precBoxLayout = new QHBoxLayout;
  precBox->setLayout (precBoxLayout);
  precBoxLayout->setSpacing (10);

  precision = new QComboBox (this);
  precision->setToolTip (tr ("Set the PFM structure depth precision"));
  precision->setWhatsThis (precisionText);
  precision->setEditable (FALSE);
  precision->addItem ("0.01 " + tr ("(one centimeter)"));
  precision->addItem ("0.10 " + tr ("(one decimeter)"));
  precision->addItem ("1.00 " + tr ("(one meter)"));
  precBoxLayout->addWidget (precision);


  limBoxLayout->addWidget (precBox);


  pageLayout->addWidget (limBox);


  QGroupBox *areaBox = new QGroupBox (tr ("Area file"), this);
  QHBoxLayout *areaBoxLayout = new QHBoxLayout;
  areaBox->setLayout (areaBoxLayout);
  areaBoxLayout->setSpacing (10);

  area_edit = new QLineEdit (this);
  area_edit->setReadOnly (TRUE);
  area_edit->setToolTip (tr ("Area file name for this PFM")); 
  area_edit->setWhatsThis (areaText);
  areaBoxLayout->addWidget (area_edit);

  area_browse = new QPushButton (tr ("Browse..."), this);
  area_browse->setToolTip (tr ("Select an area file to define the PFM area"));
  area_browse->setWhatsThis (areaBrowseText);
  connect (area_browse, SIGNAL (clicked ()), this, SLOT (slotAreaFileBrowse ()));
  areaBoxLayout->addWidget (area_browse);

  area_map = new QPushButton (tr ("Map..."), this);
  area_map->setToolTip (tr ("Create an area file using areaCheck")); 
  area_map->setWhatsThis (area_mapText);
  connect (area_map, SIGNAL (clicked ()), this, SLOT (slotAreaMap ()));
  areaBoxLayout->addWidget (area_map);

  area_pfm = new QPushButton (tr ("PFM..."), this);
  area_pfm->setToolTip (tr ("Use the area in an already existing PFM structure")); 
  area_pfm->setWhatsThis (area_PFMText);
  connect (area_pfm, SIGNAL (clicked ()), this, SLOT (slotAreaPFM ()));
  areaBoxLayout->addWidget (area_pfm);

  area_nsew = new QPushButton (tr ("NSEW..."), this);
  area_nsew->setToolTip (tr ("Create an area file by defining North, South, East, and West bounds")); 
  area_nsew->setWhatsThis (area_nsewText);
  connect (area_nsew, SIGNAL (clicked ()), this, SLOT (slotAreaNSEW ()));
  areaBoxLayout->addWidget (area_nsew);


  pageLayout->addWidget (areaBox, 1);



  QGroupBox *optBox = new QGroupBox (tr ("Optional files"), this);
  QHBoxLayout *optBoxLayout = new QHBoxLayout;
  optBox->setLayout (optBoxLayout);
  optBoxLayout->setSpacing (10);


  QGroupBox *mosaicBox = new QGroupBox (tr ("Mosaic file"), this);
  QHBoxLayout *mosaicBoxLayout = new QHBoxLayout;
  mosaicBox->setLayout (mosaicBoxLayout);
  mosaicBoxLayout->setSpacing (10);

  mosaic_edit = new QLineEdit (this);
  mosaic_edit->setReadOnly (TRUE);
  mosaic_edit->setToolTip (tr ("Mosaic file name for this PFM"));
  mosaic_edit->setWhatsThis (mosaicText);
  mosaicBoxLayout->addWidget (mosaic_edit);

  mosaic_browse = new QPushButton (tr ("Browse..."), this);
  mosaic_browse->setToolTip (tr ("Select a mosaic file for this PFM"));
  mosaic_browse->setWhatsThis (mosaicBrowseText);
  mosaic_edit->setText (pfm_def->mosaic);
  connect (mosaic_browse, SIGNAL (clicked ()), this, SLOT (slotMosaicFileBrowse ()));
  mosaicBoxLayout->addWidget (mosaic_browse);


  optBoxLayout->addWidget (mosaicBox);


  QGroupBox *featureBox = new QGroupBox (tr ("Feature file"), this);
  QHBoxLayout *featureBoxLayout = new QHBoxLayout;
  featureBox->setLayout (featureBoxLayout);
  featureBoxLayout->setSpacing (10);

  feature_edit = new QLineEdit (this);
  feature_edit->setReadOnly (TRUE);
  feature_edit->setToolTip (tr ("Feature file name for this PFM"));
  feature_edit->setWhatsThis (featureText);
  featureBoxLayout->addWidget (feature_edit);

  feature_browse = new QPushButton (tr ("Browse..."), this);
  feature_browse->setToolTip (tr ("Select a feature file for this PFM"));
  feature_browse->setWhatsThis (featureBrowseText);
  feature_edit->setText (pfm_def->feature);
  connect (feature_browse, SIGNAL (clicked ()), this, SLOT (slotFeatureFileBrowse ()));
  featureBoxLayout->addWidget (feature_browse);


  optBoxLayout->addWidget (featureBox);


  pageLayout->addWidget (optBox, 1);


  QGroupBox *filtBox = new QGroupBox (tr ("Area filter settings"), this);
  QHBoxLayout *filtBoxLayout = new QHBoxLayout;
  filtBox->setLayout (filtBoxLayout);
  filtBoxLayout->setSpacing (10);


  QGroupBox *aBox = new QGroupBox (tr ("Apply area filter"), this);
  QHBoxLayout *aBoxLayout = new QHBoxLayout;
  aBox->setLayout (aBoxLayout);
  aBoxLayout->setSpacing (10);

  applyFilter = new QCheckBox (this);
  applyFilter->setToolTip (tr ("Apply the area filter for this PFM"));
  applyFilter->setWhatsThis (applyFilterText);
  applyFilter->setChecked (pfm_def->apply_area_filter);
  connect (applyFilter, SIGNAL (stateChanged (int)), this, SLOT (slotApplyFilterStateChanged (int)));
  aBoxLayout->addWidget (applyFilter);


  filtBoxLayout->addWidget (aBox);


  QGroupBox *dBox = new QGroupBox (tr ("Deep filter only"), this);
  QHBoxLayout *dBoxLayout = new QHBoxLayout;
  dBox->setLayout (dBoxLayout);
  dBoxLayout->setSpacing (10);

  deepFilter = new QCheckBox (this);
  deepFilter->setToolTip (tr ("Only filter values deeper than the average surface"));
  deepFilter->setWhatsThis (deepFilterText);
  deepFilter->setChecked (pfm_def->deep_filter_only);
  if (!pfm_def->apply_area_filter) deepFilter->setEnabled (FALSE);
  dBoxLayout->addWidget (deepFilter);


  filtBoxLayout->addWidget (dBox);


  QGroupBox *bBox = new QGroupBox (tr ("Bin standard deviation"), this);
  QHBoxLayout *bBoxLayout = new QHBoxLayout;
  bBox->setLayout (bBoxLayout);
  bBoxLayout->setSpacing (10);

  stdSpin = new QDoubleSpinBox (this);
  stdSpin->setDecimals (2);
  stdSpin->setRange (0.3, 3.0);
  stdSpin->setSingleStep (0.1);
  stdSpin->setValue (pfm_def->cellstd);
  stdSpin->setWrapping (TRUE);
  stdSpin->setToolTip (tr ("Set the area filter standard deviation"));
  stdSpin->setWhatsThis (stdText);
  if (!pfm_def->apply_area_filter) stdSpin->setEnabled (FALSE);
  bBoxLayout->addWidget (stdSpin);


  filtBoxLayout->addWidget (bBox);


  QGroupBox *tBox = new QGroupBox (tr ("Feature Radius"), this);
  QHBoxLayout *tBoxLayout = new QHBoxLayout;
  tBox->setLayout (tBoxLayout);
  tBoxLayout->setSpacing (10);

  featureRadius = new QDoubleSpinBox (this);
  featureRadius->setDecimals (2);
  featureRadius->setRange (0.0, 200.0);
  featureRadius->setSingleStep (10.0);
  featureRadius->setValue (pfm_def->radius);
  featureRadius->setWrapping (TRUE);
  featureRadius->setToolTip (tr ("Set the radius of the area around features to exclude from filtering"));
  featureRadius->setWhatsThis (featureRadiusText);
  if (!pfm_def->apply_area_filter) featureRadius->setEnabled (FALSE);
  tBoxLayout->addWidget (featureRadius);


  filtBoxLayout->addWidget (tBox);


  pageLayout->addWidget (filtBox, 1);


  //  Register fields.

  pfm_file_edit_field = "pfm_file_edit" + pfmIndex;
  registerField (pfm_file_edit_field, pfm_file_edit);

  area_edit_field = "area_edit" + pfmIndex;
  registerField (area_edit_field, area_edit);

  mBinSizeField = "mBinSize" + pfmIndex;
  registerField (mBinSizeField, mBinSize, "value", "valueChanged");

  gBinSizeField = "gBinSize" + pfmIndex;
  registerField (gBinSizeField, gBinSize, "value", "valueChanged");

  minDepthField = "minDepth" + pfmIndex;
  registerField (minDepthField, minDepth, "value", "valueChanged");

  maxDepthField = "maxDepth" + pfmIndex;
  registerField (maxDepthField, maxDepth, "value", "valueChanged");

  precisionField = "precision" + pfmIndex;
  registerField (precisionField, precision);

  mosaic_edit_field = "mosaic_edit" + pfmIndex;
  registerField (mosaic_edit_field, mosaic_edit);

  feature_edit_field = "feature_edit" + pfmIndex;
  registerField (feature_edit_field, feature_edit);

  applyFilterField = "applyFilter" + pfmIndex;
  registerField (applyFilterField, applyFilter);

  deepFilterField = "deepFilter" + pfmIndex;
  registerField (deepFilterField, deepFilter);

  stdSpinField = "stdSpin" + pfmIndex;
  registerField (stdSpinField, stdSpin, "value", "valueChanged");

  featureRadiusField = "featureRadius" + pfmIndex;
  registerField (featureRadiusField, featureRadius, "value", "valueChanged");


  setFields (pfmDef);
}



NV_INT32 
pfmPage::nextId () const
{
  if (pfm_file_edit->text ().isEmpty () || area_edit->text ().isEmpty ())
    return (MAX_LOAD_FILES + 2);

  return (l_page_num + 1);
}



bool 
pfmPage::validatePage ()
{
  if (l_page_num == 1 && (pfm_file_edit->text ().isEmpty () || area_edit->text ().isEmpty ()))
    {
      QMessageBox::critical (this, tr ("pfmLoad first page"),
                             tr ("You must enter a PFM name and an area file name to continue!"));

      return (FALSE);
    }

  return (TRUE);
}



void 
pfmPage::slotMBinSizeChanged (double value)
{
  pfm_def->mbin_size = value;


  //  Disconnect the geographic bin size value changed signal so we don't bounce back and forth.  Reconnect after
  //  setting the value.

  disconnect (gBinSize, SIGNAL (valueChanged (double)), 0, 0);


  //  We don't ever want both gbin and mbin to be 0.0.

  if (fabs (value < 0.0000001))
    {
      pfm_def->gbin_size = prev_gbin;
    }
  else
    {
      prev_mbin = value;
      pfm_def->gbin_size = 0.0;
    }
  gBinSize->setValue (pfm_def->gbin_size);

  connect (gBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotGBinSizeChanged (double)));
}



void 
pfmPage::slotGBinSizeChanged (double value)
{
  pfm_def->gbin_size = value;


  //  Disconnect the meter bin size value changed signal so we don't bounce back and forth.  Reconnect after setting
  //  the value.

  disconnect (mBinSize, SIGNAL (valueChanged (double)), 0, 0);


  //  We don't ever want both gbin and mbin to be 0.0.

  if (fabs (value < 0.0000001))
    {
      pfm_def->mbin_size = prev_mbin;
    }
  else
    {
      prev_gbin = value;
      pfm_def->mbin_size = 0.0;
    }
  mBinSize->setValue (pfm_def->mbin_size);

  connect (mBinSize, SIGNAL (valueChanged (double)), this, SLOT (slotMBinSizeChanged (double)));
}



void 
pfmPage::slotApplyFilterStateChanged (int state)
{
  if (state == Qt::Unchecked)
    {
      deepFilter->setEnabled (FALSE);
      stdSpin->setEnabled (FALSE);
      featureRadius->setEnabled (FALSE);
    }
  else
    {
      deepFilter->setEnabled (TRUE);
      stdSpin->setEnabled (TRUE);
      featureRadius->setEnabled (TRUE);
    }
}



void 
pfmPage::slotPFMFileEdit (const QString &string)
{
  pfm_def->name = string;


  if (!pfm_def->name.endsWith (".pfm")) pfm_def->name += ".pfm";


  mBinSize->setEnabled (TRUE);
  gBinSize->setEnabled (TRUE);
  minDepth->setEnabled (TRUE);
  maxDepth->setEnabled (TRUE);
  precision->setEnabled (TRUE);
  area_browse->setEnabled (TRUE);
  area_map->setEnabled (TRUE);
  area_pfm->setEnabled (TRUE);
  area_nsew->setEnabled (TRUE);


  //  Watch out for directory names that might be typed in.

  if (QFileInfo (string).isDir ()) return;


  FILE *fp;
  NV_INT32 hnd;
  PFM_OPEN_ARGS open_args;


  strcpy (open_args.list_path, pfm_def->name.toAscii ());


  //  Try to open the file.  If it exists we will not allow the user to change the bin size, 
  //  depth precision, etc.

  pfm_def->existing = NVFalse;
  if ((fp = fopen (open_args.list_path, "r")) != NULL)
    {
      fclose (fp);

      open_args.checkpoint = 0;

      if ((hnd = open_existing_pfm_file (&open_args)) >= 0)
        {
          NV_CHAR file[512];
          get_target_file (hnd, open_args.list_path, file);
          feature_edit->setText (QString (file));
          get_mosaic_file (hnd, open_args.list_path, file);
          mosaic_edit->setText (QString (file));

          close_pfm_file (hnd);


          area_edit->setText (tr ("Defined in PFM structure"));


          mBinSize->setEnabled (FALSE);
          gBinSize->setEnabled (FALSE);
          minDepth->setEnabled (FALSE);
          maxDepth->setEnabled (FALSE);
          precision->setEnabled (FALSE);
          area_browse->setEnabled (FALSE);
          area_map->setEnabled (FALSE);
          area_pfm->setEnabled (FALSE);
          area_nsew->setEnabled (FALSE);

          pfm_def->existing = NVTrue;
        }
      else
        {
          QMessageBox::warning (this, tr ("Open PFM Structure"),
                                tr ("The file ") + QDir::toNativeSeparators (QString (open_args.list_path)) + 
                                tr (" is not a PFM structure or there was an error reading the file.") +
                                tr ("  The error message returned was:\n\n") +
                                QString (pfm_error_str (pfm_error)));

          if (pfm_error == CHECKPOINT_FILE_EXISTS_ERROR)
            {
              fprintf (stderr, "\n\n%s\n", pfm_error_str (pfm_error));
              exit (-1);
            }
          return;
        }
    }
  else
    {
      if (area_edit->text () == tr ("Defined in PFM structure")) area_edit->clear ();

      feature_edit->setText ("NONE");
      mosaic_edit->setText ("NONE");

      QFileInfo fi = QFileInfo (pfm_def->name);
      if (fi.isRelative ())
        {
          //  Get the absolute path name

          QDir dir;

          pfm_def->name.prepend ("/");
          pfm_def->name.prepend (dir.canonicalPath());
        }
    }

  if (!pfm_file_edit->text ().isEmpty () && !area_edit->text ().isEmpty ())
    {
      setButtonText (QWizard::NextButton, tr ("Next"));
    }
  else
    {
      if (l_page_num > 1) setButtonText (QWizard::NextButton, tr ("Finish"));
    }
}



void 
pfmPage::slotPFMFileBrowse ()
{
  QStringList         files, filters;
  QString             file;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmLoad Open PFM Structure"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, pfm_global->output_dir);


  filters << tr ("PFM (*.pfm)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectFilter (tr ("PFM (*.pfm)"));

  if (fd->exec () == QDialog::Accepted)
    {
      //  Save the directory that we were in when we selected a file.

      pfm_global->output_dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();

      pfm_def->name = files.at (0);


      if (!pfm_def->name.isEmpty())
        {
          if (!pfm_def->name.endsWith (".pfm")) pfm_def->name.append (".pfm");


          //  This will trigger slotPFMFileEdit.

          pfm_file_edit->setText (pfm_def->name);
        }
    }
}



void 
pfmPage::slotAreaFileBrowse ()
{
  QFileDialog fd (this, tr ("pfmLoad Area File"));
  fd.setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (&fd, pfm_global->area_dir);


  QStringList filters;
  filters << tr ("Area file (*.ARE *.are *.afs)");

  fd.setFilters (filters);
  fd.setFileMode (QFileDialog::ExistingFile);
  fd.selectFilter (tr ("Area file (*.ARE *.are *.afs)"));


  QStringList files;
  if (fd.exec () == QDialog::Accepted)
    {
      //  Save the directory that we were in when we selected a file.

      pfm_global->area_dir = fd.directory ().absolutePath ();

      files = fd.selectedFiles ();

      pfm_def->area = files.at (0);


      if (!pfm_def->area.isEmpty())
        {
          area_edit->setText (pfm_def->area);

          if (!pfm_file_edit->text ().isEmpty () && !area_edit->text ().isEmpty ()) setButtonText (QWizard::NextButton, tr ("Next"));
        }
    }
}



void 
pfmPage::slotMosaicFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmLoad Mosaic File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, pfm_global->mosaic_dir);


  QStringList filters;
  filters << tr ("Mosaic file (*.mos, *.tif)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Mosaic file (*.mos, *.tif)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      //  Save the directory that we were in when we selected a file.

      pfm_global->mosaic_dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();

      pfm_def->mosaic = files.at (0);


      if (!pfm_def->mosaic.isEmpty())
        {
          mosaic_edit->setText (pfm_def->mosaic);
        }
    }
}



void 
pfmPage::slotFeatureFileBrowse ()
{
  QFileDialog *fd = new QFileDialog (this, tr ("pfmLoad Feature File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, pfm_global->feature_dir);


  QStringList filters;
  filters << tr ("Binary Feature Data (*.bfd)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Binary Feature Data (*.bfd)"));


  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      //  Save the directory that we were in when we selected a file.

      pfm_global->feature_dir = fd->directory ().absolutePath ();

      files = fd->selectedFiles ();

      pfm_def->feature = files.at (0);


      if (!pfm_def->feature.isEmpty())
        {
          feature_edit->setText (pfm_def->feature);
        }
    }
}



void 
pfmPage::slotMapReadyReadStandardError ()
{
  QByteArray response = mapProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



void 
pfmPage::slotMapReadyReadStandardOutput ()
{
  QByteArray response = mapProc->readAllStandardOutput ();
  NV_CHAR *res = response.data ();

  fprintf (stderr,"%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



void 
pfmPage::slotMapError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("pfmLoad map editor"), tr ("Unable to start the map editor process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("pfmLoad map editor"), tr ("The map editor process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("pfmLoad map editor"), tr ("The map editor process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("pfmLoad map editor"), 
                             tr ("There was a write error from the map editor process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("pfmLoad map editor"),
                             tr ("There was a read error from the map editor process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("pfmLoad map editor"),
                             tr ("The map editor process died with an unknown error!"));
      break;
    }
}



void 
pfmPage::slotMapDone (int exitCode __attribute__ ((unused)), QProcess::ExitStatus exitStatus __attribute__ ((unused)))
{
  pfm_def->area = pfm_def->name + ".are";

  rename (temp_area_name.toAscii (), pfm_def->area.toAscii ());

  area_edit->setText (pfm_def->area);

  if (!pfm_def->name.isEmpty () && !pfm_def->area.isEmpty ()) setButtonText (QWizard::NextButton, tr ("Next"));
}



void 
pfmPage::slotAreaMap ()
{
  if (pfm_def->name.isEmpty ())
    {
      QMessageBox::warning (this, tr ("pfmLoad"),
                            tr ("You must first set a PFM file name before trying to create an area file."));
    }
  else
    {
      temp_area_name.sprintf ("TEMPORARY_AREA_FILE_%d_%d.are", l_page_num - 1, getpid ());


      mapProc = new QProcess (this);
      QStringList arguments;


      if (pfm_def->area.isEmpty ())
        {
          arguments += "--empty_file";
          arguments += temp_area_name;

 
          connect (mapProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                   SLOT (slotMapDone (int, QProcess::ExitStatus)));
          connect (mapProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMapReadyReadStandardError ()));
          connect (mapProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotMapReadyReadStandardOutput ()));
          connect (mapProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMapError (QProcess::ProcessError)));

          mapProc->start ("areaCheck", arguments);
        }
      else
        {
          arguments += "--file";
          arguments += pfm_def->area;

 
          connect (mapProc, SIGNAL (finished (int, QProcess::ExitStatus)), this, 
                   SLOT (slotMapDone (int, QProcess::ExitStatus)));
          connect (mapProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotMapReadyReadStandardError ()));
          connect (mapProc, SIGNAL (readyReadStandardOutput ()), this, SLOT (slotMapReadyReadStandardOutput ()));
          connect (mapProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotMapError (QProcess::ProcessError)));

          mapProc->start ("areaCheck", arguments);
        }
    }
}



void 
pfmPage::slotAreaPFM ()
{
  QStringList         files, filters;
  QString             file;
  NV_INT32            hnd;
  PFM_OPEN_ARGS       open_args;


  if (pfm_def->name.isEmpty ())
    {
      QMessageBox::warning (this, tr ("pfmLoad"),
                            tr ("You must first set a PFM file name before trying to use a pre-existing PFM area definition."));
    }
  else
    {
      QFileDialog *fd = new QFileDialog (this, tr ("pfmLoad Open PFM Structure (area)"));
      fd->setViewMode (QFileDialog::List);


      //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
      //  This function is in the nvutility library.

      setSidebarUrls (fd, pfm_global->area_dir);


      filters << tr ("PFM (*.pfm)");

      fd->setFilters (filters);
      fd->setFileMode (QFileDialog::ExistingFile);
      fd->selectFilter (tr ("PFM (*.pfm)"));

      if (fd->exec () == QDialog::Accepted)
	{
          //  Save the directory that we were in when we selected a file.

          pfm_global->area_dir = fd->directory ().absolutePath ();

	  files = fd->selectedFiles ();

	  QString pfm_name = files.at (0);


	  if (!pfm_name.isEmpty())
	    {
	      strcpy (open_args.list_path, pfm_name.toAscii ());
	      open_args.checkpoint = 0;

	      if ((hnd = open_existing_pfm_file (&open_args)) >= 0)
		{
		  temp_area_name.sprintf ("TEMPORARY_AREA_FILE_%d_%d.are", l_page_num - 1, getpid ());

		  FILE *fp;

		  if ((fp = fopen (temp_area_name.toAscii (), "w")) != NULL)
		    {
		      NV_CHAR ltstring[25], lnstring[25], hem;
		      NV_FLOAT64 deg, min, sec;


		      for (NV_INT32 i = 0 ; i < open_args.head.polygon_count ; i++)
			{
			  strcpy (ltstring, fixpos (open_args.head.polygon[i].y, &deg, &min, &sec, &hem, POS_LAT, POS_HDMS));
			  strcpy (lnstring, fixpos (open_args.head.polygon[i].x, &deg, &min, &sec, &hem, POS_LON, POS_HDMS));

			  fprintf (fp, "%s, %s\n", ltstring, lnstring);
			}

		      fclose (fp);


		      pfm_def->area = pfm_def->name + ".are";

		      rename (temp_area_name.toAscii (), pfm_def->area.toAscii ());

		      area_edit->setText (pfm_def->area);

		      if (!pfm_def->name.isEmpty () && !pfm_def->area.isEmpty ()) setButtonText (QWizard::NextButton, tr ("Next"));
		    }

		  close_pfm_file (hnd);
		}
	    }
	}
    }
}



void 
pfmPage::slotAreaNSEW ()
{
  if (pfm_def->name.isEmpty ())
    {
      QMessageBox::warning (this, tr ("pfmLoad"),
                            tr ("You must first set a PFM file name before trying to use a pre-existing PFM area definition."));
    }
  else
    {
      nsewD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
      nsewD->setWindowTitle (tr ("pfmLoad NSEW bounds"));


      QVBoxLayout *nsewBox = new QVBoxLayout (nsewD);
      nsewBox->setMargin (5);
      nsewBox->setSpacing (5);


      QGroupBox *northBox = new QGroupBox (tr ("North Latitude"), nsewD);
      QHBoxLayout *northBoxLayout = new QHBoxLayout;
      northBox->setLayout (northBoxLayout);
      northBoxLayout->setSpacing (10);

      north = new QLineEdit (nsewD);
      north->setToolTip (tr ("Set the northern boundary of the area"));
      north->setWhatsThis (northText);

      northBoxLayout->addWidget (north, 1);

      nsewBox->addWidget (northBox);


      QGroupBox *southBox = new QGroupBox (tr ("South Latitude"), nsewD);
      QHBoxLayout *southBoxLayout = new QHBoxLayout;
      southBox->setLayout (southBoxLayout);
      southBoxLayout->setSpacing (10);

      south = new QLineEdit (nsewD);
      south->setToolTip (tr ("Set the southern boundary of the area"));
      south->setWhatsThis (southText);

      southBoxLayout->addWidget (south, 1);

      nsewBox->addWidget (southBox);


      QGroupBox *westBox = new QGroupBox (tr ("West Longitude"), nsewD);
      QHBoxLayout *westBoxLayout = new QHBoxLayout;
      westBox->setLayout (westBoxLayout);
      westBoxLayout->setSpacing (10);

      west = new QLineEdit (nsewD);
      west->setToolTip (tr ("Set the western boundary of the area"));
      west->setWhatsThis (westText);

      westBoxLayout->addWidget (west, 1);

      nsewBox->addWidget (westBox);


      QGroupBox *eastBox = new QGroupBox (tr ("East Longitude"), nsewD);
      QHBoxLayout *eastBoxLayout = new QHBoxLayout;
      eastBox->setLayout (eastBoxLayout);
      eastBoxLayout->setSpacing (10);

      east = new QLineEdit (nsewD);
      east->setToolTip (tr ("Set the eastern boundary of the area"));
      east->setWhatsThis (eastText);

      eastBoxLayout->addWidget (east, 1);

      nsewBox->addWidget (eastBox);


      QHBoxLayout *actions = new QHBoxLayout (0);
      nsewBox->addLayout (actions);

      QPushButton *bHelp = new QPushButton (nsewD);
      bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
      bHelp->setToolTip (tr ("Enter What's This mode for help"));
      connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
      actions->addWidget (bHelp);

      actions->addStretch (10);


      QPushButton *applyButton = new QPushButton (tr ("OK"), nsewD);
      applyButton->setToolTip (tr ("Accept values and close dialog"));
      applyButton->setWhatsThis (applyNSEWText);
      connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyNSEW ()));
      actions->addWidget (applyButton);

      QPushButton *closeButton = new QPushButton (tr ("Cancel"), nsewD);
      closeButton->setToolTip (tr ("Discard values and close dialog"));
      closeButton->setWhatsThis (closeNSEWText);
      connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseNSEW ()));
      actions->addWidget (closeButton);


      nsewD->show ();
    }
}



void
pfmPage::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
pfmPage::slotApplyNSEW ()
{
  if (north->text ().isEmpty () || south->text ().isEmpty () || west->text ().isEmpty () || east->text ().isEmpty ())
    {
      QMessageBox::warning (this, tr ("pfmLoad NSEW bounds"), tr ("You must set all four fields or use the Cancel button."));
    }
  else
    {
      NV_CHAR nlat[50], slat[50], elon[50], wlon[50], file[512], hem;
      NV_FLOAT64 ndegs, sdegs, wdegs, edegs, deg, min, sec, tmp;

      strcpy (nlat, north->text ().toAscii ());
      strcpy (slat, south->text ().toAscii ());
      strcpy (elon, east->text ().toAscii ());
      strcpy (wlon, west->text ().toAscii ());

      posfix (nlat, &ndegs, POS_LAT);
      posfix (slat, &sdegs, POS_LAT);
      posfix (elon, &edegs, POS_LON);
      posfix (wlon, &wdegs, POS_LON);

      if (ndegs < sdegs)
	{
	  tmp = sdegs;
	  sdegs = ndegs;
	  ndegs = tmp;
	}

      if (edegs < wdegs)
	{
	  if ((edegs < 0.0 && wdegs < 0.0) || (edegs >= 0.0 && wdegs >= 0.0))
	    {
	      tmp = wdegs;
	      wdegs = edegs;
	      edegs = tmp;
	    }
	}


      pfm_def->area = pfm_def->name + ".are";

      strcpy (file, pfm_def->area.toAscii ());

      FILE *fp;
      
      if ((fp = fopen (file, "w")) == NULL)
	{
	  QMessageBox::warning (this, tr ("pfmLoad NSEW bounds"), tr ("Unable to open area file ") + pfm_def->area);
	}
      else
	{
	  strcpy (nlat, fixpos (ndegs, &deg, &min, &sec, &hem, POS_LAT, POS_HDMS));
	  strcpy (slat, fixpos (sdegs, &deg, &min, &sec, &hem, POS_LAT, POS_HDMS));
	  strcpy (wlon, fixpos (wdegs, &deg, &min, &sec, &hem, POS_LON, POS_HDMS));
	  strcpy (elon, fixpos (edegs, &deg, &min, &sec, &hem, POS_LON, POS_HDMS));

	  fprintf (fp, "%s, %s\n", slat, wlon);
	  fprintf (fp, "%s, %s\n", nlat, wlon);
	  fprintf (fp, "%s, %s\n", nlat, elon);
	  fprintf (fp, "%s, %s\n", slat, elon);

	  fclose (fp);

	  area_edit->setText (pfm_def->area);

	  if (!pfm_def->name.isEmpty () && !pfm_def->area.isEmpty ()) setButtonText (QWizard::NextButton, tr ("Next"));
	}

      nsewD->close ();
    }
}



void 
pfmPage::slotCloseNSEW ()
{
  nsewD->close ();
}



void 
pfmPage::setFields (PFM_DEFINITION *pfmDef)
{
  pfm_def = pfmDef;

  if (pfm_def->name != "") setField (pfm_file_edit_field, pfm_def->name);

  setField (area_edit_field, pfm_def->area);
                                     
  setField (mBinSizeField, pfm_def->mbin_size);
  setField (gBinSizeField, pfm_def->gbin_size);
  setField (minDepthField, pfm_def->min_depth);
  setField (maxDepthField, pfm_def->max_depth);

  if (pfm_def->precision < 0.1)
    {
      setField (precisionField, 0);
    }
  else if (pfm_def->precision < 1.0)
    {
      setField (precisionField, 1);
    }
  else
    {
      setField (precisionField, 2);
    }

  setField (mosaic_edit_field, pfm_def->mosaic);
  setField (feature_edit_field, pfm_def->feature);

  setField (applyFilterField, pfm_def->apply_area_filter);
  setField (deepFilterField, pfm_def->deep_filter_only);

  setField (stdSpinField, pfm_def->cellstd);

  setField (featureRadiusField, pfm_def->radius);
}
