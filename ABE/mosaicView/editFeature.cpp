#include "editFeature.hpp"
#include "editFeatureHelp.hpp"
#include "featureTypes.hpp"

#include "version.hpp"

editFeature::editFeature (QWidget *pt, OPTIONS *op, MISC *mi, NV_INT32 num):
  QDialog (pt, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  current_feature = num;
  polyTimer = NULL;
  screen_dialog = NULL;
  parent = pt;
  new_feature = NVFalse;
  assigned_image = NVFalse;
  strcpy (image_name, "");


  if (num < 0) new_feature = NVTrue;


  if (new_feature)
    {
      bfd_record = misc->new_record;
    }
  else
    {
      binaryFeatureData_read_record (misc->bfd_handle, current_feature, &bfd_record);

      if (bfd_record.poly_count) binaryFeatureData_read_polygon (misc->bfd_handle, current_feature, &bfd_polygon);
    }


  setWindowTitle (tr ("mosaicView Edit Feature"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *idBox = new QGroupBox (tr ("Record number and depth"), this);
  QHBoxLayout *idBoxLayout = new QHBoxLayout;
  idBox->setLayout (idBoxLayout);

  QGroupBox *recordBox = new QGroupBox (tr ("Feature record number"), this);
  QHBoxLayout *recordBoxLayout = new QHBoxLayout;
  recordBox->setLayout (recordBoxLayout);
  QString recordID;
  recordID.sprintf ("%05d", bfd_record.record_number);
  QLineEdit *recordEdit = new QLineEdit (recordBox);
  recordEdit->setText (recordID);
  recordEdit->setReadOnly (NVTrue);
  recordEdit->setAlignment (Qt::AlignHCenter);
  recordEdit->setToolTip (tr ("Feature record number"));
  recordBoxLayout->addWidget (recordEdit);
  idBoxLayout->addWidget (recordBox);

  QGroupBox *depthBox = new QGroupBox (tr ("Depth"), this);
  QHBoxLayout *depthBoxLayout = new QHBoxLayout;
  depthBox->setLayout (depthBoxLayout);
  QString depth;
  depth.sprintf ("%0.2f", bfd_record.depth);
  QLineEdit *depthEdit = new QLineEdit (depthBox);
  depthEdit->setText (depth);
  depthEdit->setReadOnly (NVTrue);
  depthEdit->setAlignment (Qt::AlignHCenter);
  depthEdit->setToolTip (tr ("Feature Depth"));
  depthBoxLayout->addWidget (depthEdit);
  idBoxLayout->addWidget (depthBox);

  vbox->addWidget (idBox, 1);


  QGroupBox *dimBox = new QGroupBox (tr ("Dimensions and confidence"), this);
  QHBoxLayout *dimBoxLayout = new QHBoxLayout;
  dimBox->setLayout (dimBoxLayout);

  QGroupBox *lengthBox = new QGroupBox (tr ("Length"), this);
  QHBoxLayout *lengthBoxLayout = new QHBoxLayout;
  lengthBox->setLayout (lengthBoxLayout);
  length = new QDoubleSpinBox (lengthBox);
  length->setDecimals (1);
  length->setRange (0.0, 1000.0);
  length->setSingleStep (1.0);
  length->setValue (bfd_record.length);
  length->setWrapping (TRUE);
  length->setToolTip (tr ("Feature length in meters"));
  length->setWhatsThis (lengthText);
  length->setValue (bfd_record.length);
  lengthBoxLayout->addWidget (length);
  dimBoxLayout->addWidget (lengthBox);


  QGroupBox *widthBox = new QGroupBox (tr ("Width"), this);
  QHBoxLayout *widthBoxLayout = new QHBoxLayout;
  widthBox->setLayout (widthBoxLayout);
  width = new QDoubleSpinBox (widthBox);
  width->setDecimals (1);
  width->setRange (0.0, 1000.0);
  width->setSingleStep (1.0);
  width->setValue (bfd_record.width);
  width->setWrapping (TRUE);
  width->setToolTip (tr ("Feature width in meters"));
  width->setWhatsThis (widthText);
  widthBoxLayout->addWidget (width);
  dimBoxLayout->addWidget (widthBox);


  QGroupBox *orientationBox = new QGroupBox (tr ("Orientation"), this);
  QHBoxLayout *orientationBoxLayout = new QHBoxLayout;
  orientationBox->setLayout (orientationBoxLayout);
  orientation = new QDoubleSpinBox (orientationBox);
  orientation->setDecimals (1);
  orientation->setRange (0.0, 359.9);
  orientation->setSingleStep (1.0);
  orientation->setValue (bfd_record.horizontal_orientation);
  orientation->setWrapping (TRUE);
  orientation->setToolTip (tr ("Feature orientation in degrees"));
  orientation->setWhatsThis (orientationText);
  orientationBoxLayout->addWidget (orientation);
  dimBoxLayout->addWidget (orientationBox);


  QGroupBox *confidenceBox = new QGroupBox (tr ("Confidence"), this);
  QHBoxLayout *confidenceBoxLayout = new QHBoxLayout;
  confidenceBox->setLayout (confidenceBoxLayout);
  confidence = new QSpinBox (confidenceBox);
  confidence->setRange (0, 5);
  confidence->setSingleStep (1);
  confidence->setValue (bfd_record.confidence_level);
  confidence->setWrapping (TRUE);
  confidence->setToolTip (tr ("Feature confidence (0-5)"));
  confidence->setWhatsThis (confidenceText);
  confidenceBoxLayout->addWidget (confidence);
  dimBoxLayout->addWidget (confidenceBox);


  vbox->addWidget (dimBox, 1);


  QGroupBox *aBox = new QGroupBox (tr ("Area definition"), this);
  QHBoxLayout *aBoxLayout = new QHBoxLayout;
  aBox->setLayout (aBoxLayout);


  bPolygon = new QPushButton (tr ("Define polygon"), this);
  bPolygon->setToolTip (tr ("Define a polygonal area for this feature."));
  connect (bPolygon, SIGNAL (clicked ()), this, SLOT (slotPolygon ()));
  bPolygon->setWhatsThis (polygonText);
  if (bfd_record.poly_count) bPolygon->setEnabled (FALSE); 
  aBoxLayout->addWidget (bPolygon);


  bDelete = new QPushButton (tr ("Delete polygon"), this);
  bDelete->setToolTip (tr ("Delete the polygonal area for this feature."));
  connect (bDelete, SIGNAL (clicked ()), this, SLOT (slotDelete ()));
  bDelete->setWhatsThis (deleteText);
  if (!bfd_record.poly_count) bDelete->setEnabled (FALSE); 
  aBoxLayout->addWidget (bDelete);


  bGroup = new QPushButton (tr ("Group sub-features"), this);
  bGroup->setToolTip (tr ("Group all features within the polygon as sub-features."));
  connect (bGroup, SIGNAL (clicked ()), this, SLOT (slotGroup ()));
  bGroup->setWhatsThis (groupText);
  if (!bfd_record.poly_count) bGroup->setEnabled (FALSE); 
  aBoxLayout->addWidget (bGroup);


  bUngroup = new QPushButton (tr ("Un-group sub-features"), this);
  bUngroup->setToolTip (tr ("Un-group all sub-features within the polygon."));
  connect (bUngroup, SIGNAL (clicked ()), this, SLOT (slotUngroup ()));
  bUngroup->setWhatsThis (ungroupText);
  if (!bfd_record.poly_count) bUngroup->setEnabled (FALSE); 
  aBoxLayout->addWidget (bUngroup);


  vbox->addWidget (aBox, 1);


  QGroupBox *descBox = new QGroupBox (tr ("Description"), this);
  QVBoxLayout *descBoxLayout = new QVBoxLayout;
  descBox->setLayout (descBoxLayout);
  QHBoxLayout *descBoxTopLayout = new QHBoxLayout;
  QHBoxLayout *descBoxBottomLayout = new QHBoxLayout;
  descBoxLayout->addLayout (descBoxTopLayout);
  descBoxLayout->addLayout (descBoxBottomLayout);

  description = new QLineEdit (descBox);
  description->setToolTip (tr ("Feature description"));
  description->setWhatsThis (descriptionText);
  if (new_feature)
    {
      description->setText (options->last_feature_description);
    }
  else
    {
      description->setText (bfd_record.description);
    }
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
  
  QPushButton *lidar = new QPushButton (tr ("LIDAR/Other"), this);
  lidar->setToolTip (tr ("Select LIDAR specific or informational description from list"));
  connect (lidar, SIGNAL (clicked ()), this, SLOT (slotLidar ()));
  descBoxBottomLayout->addWidget (lidar);
  

  vbox->addWidget (descBox, 1);


  QGroupBox *remarksBox = new QGroupBox (tr ("Remarks"), this);
  QHBoxLayout *remarksBoxLayout = new QHBoxLayout;
  remarksBox->setLayout (remarksBoxLayout);
  remarks = new QLineEdit (remarksBox);
  remarks->setToolTip (tr ("Feature remarks"));
  remarks->setWhatsThis (remarksText);
  if (new_feature)
    {
      remarks->setText (options->last_feature_remarks);
    }
  else
    {
      remarks->setText (bfd_record.remarks);
    }
  remarksBoxLayout->addWidget (remarks);


  vbox->addWidget (remarksBox, 1);


  QGroupBox *snipBox = new QGroupBox (tr ("Snippet"), this);
  QHBoxLayout *snipBoxLayout = new QHBoxLayout;
  snipBox->setLayout (snipBoxLayout);

  bView = new QPushButton (tr ("View"), this);
  bView->setToolTip (tr ("View the attached sidescan or photo image"));
  bView->setWhatsThis (viewText);
  connect (bView, SIGNAL (clicked ()), this, SLOT (slotViewSnippet ()));
  if (!bfd_record.image_size) bView->setEnabled (FALSE);
  snipBoxLayout->addWidget (bView);

  bAssign = new QPushButton (tr ("Assign"), this);
  bAssign->setToolTip (tr ("Assign an image to this feature"));
  bAssign->setWhatsThis (assignText);
  connect (bAssign, SIGNAL (clicked ()), this, SLOT (slotAssignSnippet ()));
  snipBoxLayout->addWidget (bAssign);

  bScreen = new QPushButton (tr ("Screenshot"), this);
  bScreen->setToolTip (tr ("Grab a screenshot to be saved for this feature"));
  bScreen->setWhatsThis (screenText);
  connect (bScreen, SIGNAL (clicked ()), this, SLOT (slotScreen ()));
  snipBoxLayout->addWidget (bScreen);


  vbox->addWidget (snipBox, 1);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Apply feature changes"));
  applyButton->setWhatsThis (applyEditFeatureText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyEditFeature ()));
  actions->addWidget (applyButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), this);
  cancelButton->setToolTip (tr ("Cancel feature changes"));
  cancelButton->setWhatsThis (cancelEditFeatureText);
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotCancelEditFeature ()));
  actions->addWidget (cancelButton);
}



editFeature::~editFeature ()
{
}



void
editFeature::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
editFeature::slotViewError (QProcess::ProcessError error)
{
  switch (error)
    {
    case QProcess::FailedToStart:
      QMessageBox::critical (this, tr ("mosaicView image viewer"), tr ("Unable to start the image viewer process!"));
      break;

    case QProcess::Crashed:
      QMessageBox::critical (this, tr ("mosaicView image viewer"), tr ("The image viewer process crashed!"));
      break;

    case QProcess::Timedout:
      QMessageBox::critical (this, tr ("mosaicView image viewer"), tr ("The image viewer process timed out!"));
      break;

    case QProcess::WriteError:
      QMessageBox::critical (this, tr ("mosaicView image viewer"), 
                             tr ("There was a write error to the image viewer process!"));
      break;

    case QProcess::ReadError:
      QMessageBox::critical (this, tr ("mosaicView image viewer"),
                             tr ("There was a read error from the image viewer process!"));
      break;

    case QProcess::UnknownError:
      QMessageBox::critical (this, tr ("mosaicView image viewer"),
                             tr ("The image viewer process died with an unknown error!"));
      break;
    }
}



//  This is the stderr read return from the feature viewer QProcess
//  Hopefully you won't see anything here

void 
editFeature::slotSnippetViewerReadyReadStandardError ()
{
  QByteArray response = imageProc->readAllStandardError ();
  NV_CHAR *res = response.data ();

  fprintf (stderr, "%s %d %s\n", __FILE__, __LINE__, res);
  fflush (stderr);
}



void 
editFeature::slotViewSnippet ()
{
  imageProc = new QProcess (this);


  QString arg;
  QStringList arguments;


  arg = "--file=" + QString (misc->abe_share->open_args[0].target_path);
  arguments += arg;

  arg.sprintf ("--heading=%f", bfd_record.heading);
  arguments += arg;

  arg.sprintf ("--record=%d", bfd_record.record_number);
  arguments += arg;

  connect (imageProc, SIGNAL (error (QProcess::ProcessError)), this, SLOT (slotViewError (QProcess::ProcessError)));
  connect (imageProc, SIGNAL (readyReadStandardError ()), this, SLOT (slotSnippetViewerReadyReadStandardError ()));

  imageProc->start ("featurePic", arguments);
}



void 
editFeature::slotAssignSnippet ()
{
  QString file;
  QStringList filters;
  QStringList files;

  QFileDialog *fd = new QFileDialog (this, tr ("mosaicView Assign Image File"));
  fd->setViewMode (QFileDialog::List);
  fd->setDirectory (QFileInfo (misc->abe_share->open_args[0].list_path).dir ());

  filters << tr ("Image (*.bmp *.jpg *.png *.gif *.xpm)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Image (*.bmp *.jpg *.png *.gif *.xpm)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      file = files.at (0);


      if (file.isEmpty())
        { 
          return;
        }
      else
        {
          strcpy (image_name, file.toAscii ());

          assigned_image = NVTrue;

          bView->setEnabled (TRUE);
        }
    }
}



void 
editFeature::slotScreenshotSaved (QString filename)
{
  strcpy (image_name, filename.toAscii ());

  assigned_image = NVFalse;

  bView->setEnabled (TRUE);

  screen_dialog = NULL;

  show ();
}



void 
editFeature::slotScreenshotQuit ()
{
  screen_dialog = NULL;

  show ();
}



void 
editFeature::slotScreen ()
{
  hide ();


  if (screen_dialog) screen_dialog->close ();


  if (new_feature)
    {
      screen_dialog = new screenshot (this, options, misc, misc->bfd_header.number_of_records);
    }
  else
    {
      screen_dialog = new screenshot (this, options, misc, current_feature);
    }

  connect (screen_dialog, SIGNAL (screenshotSavedSignal (QString)), this, SLOT (slotScreenshotSaved (QString)));
  connect (screen_dialog, SIGNAL (screenshotQuit ()), this, SLOT (slotScreenshotQuit ()));
}



void 
editFeature::slotHelpEditFeature ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
editFeature::slotApplyEditFeature ()
{
  bfd_record.length = length->value ();
  bfd_record.width = width->value ();
  bfd_record.horizontal_orientation = orientation->value ();
  bfd_record.confidence_level = confidence->value ();

  options->last_feature_remarks = remarks->text ();
  strcpy (bfd_record.remarks, options->last_feature_remarks.toAscii ());

  options->last_feature_description = description->text ();

  strcpy (bfd_record.description, options->last_feature_description.toAscii ());


  if (new_feature)
    {
      /*  If there was no feature file, create one.  */

      if (!misc->bfd_open)
        {
          QString name = misc->abe_share->open_args[0].list_path;
          name.replace (".pfm", ".bfd");


          QFileDialog fd (parent, tr ("mosaicView Assign Feature File"));
          fd.setViewMode (QFileDialog::List);
          fd.setDirectory (QFileInfo (misc->abe_share->open_args[0].list_path).dir ());

          fd.setFilter (tr ("Binary Feature Data file (*.bfd)"));
          fd.setFileMode (QFileDialog::AnyFile);


          QStringList files;
          QString file;
          if (fd.exec () == QDialog::Accepted)
            {
              files = fd.selectedFiles ();

              file = files.at (0);

              if (file.isEmpty()) return;


              if (!file.endsWith (".bfd")) file.append (".bfd");


              //  Check for a pre-existing file

              NV_CHAR filename[512];
              strcpy (filename, file.toAscii ());

              if ((misc->bfd_handle = binaryFeatureData_open_file (filename, &misc->bfd_header, BFDATA_UPDATE)) < 0)
                {
                  QMessageBox::warning (parent, tr ("mosaicView"), tr ("Creating new feature file not allowed in mosaicView, use pfmView."));
                  return;
                }
              else
                {
                  if (misc->bfd_open) binaryFeatureData_close_file (misc->bfd_handle);
                  misc->bfd_open = NVFalse;

                  if ((misc->bfd_handle = binaryFeatureData_open_file (misc->abe_share->open_args[0].target_path, &misc->bfd_header, BFDATA_UPDATE)) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (parent, tr ("mosaicView"), tr ("Unable to open feature file\nReason: ") + msg);
                      return;
                    }

                  if (binaryFeatureData_read_all_short_features (misc->bfd_handle, &misc->feature) < 0)
                    {
                      QString msg = QString (binaryFeatureData_strerror ());
                      QMessageBox::warning (this, tr ("mosaicView"), tr ("Unable to read feature records\nReason: ") + msg);
                      binaryFeatureData_close_file (misc->bfd_handle);
                      return;
                    }
                  else
                    {
                      misc->bfd_open = NVTrue;
                    }
                }
            }
          else
            {
              return;
            }
        }


      //  Append a record to the file.  The dataChangedSignal handler will close and re-open the file to
      //  flush the header and read the records again.

      binaryFeatureData_write_record_image_file (misc->bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, image_name);
    }
  else
    {
      binaryFeatureData_write_record_image_file (misc->bfd_handle, bfd_record.record_number, &bfd_record, &bfd_polygon, image_name);
    }


  //  If we didn't assign an existing file to be the feature image (something we don't do much) we need to delete 
  //  the image file after we have stored it in the BFD file.

  if (!assigned_image) remove (image_name);


  emit dataChangedSignal ();

  close ();
}



void
editFeature::slotCancelEditFeature ()
{
  //  Just in case we made a screenshot, we need to remove the temporary file.

  remove (image_name);


  close ();
}



void 
editFeature::slotPolygon ()
{
  misc->feature_polygon_flag = 0;


  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  window_width = tmp.width ();
  window_height = tmp.height ();


  hide ();


  emit definePolygonSignal ();


  if (polyTimer == NULL)
    {
      polyTimer = new QTimer (this);
      connect (polyTimer, SIGNAL (timeout ()), SLOT (slotPolyTimer ()));
    }

  if (!polyTimer->isActive ()) polyTimer->start (1000);
}



void 
editFeature::slotDelete ()
{
  BFDATA_RECORD rec;


  //  If this is a new feature it can't have children yet (since that change is applied automatically) so all we
  //  have to do is zero the poly_count, poly_type, and poly_address then GTHOOD (Get The Hell Out Of Dodge).

  bfd_record.poly_count = 0;
  bfd_record.poly_type = 0;
  bfd_record.poly_address = 0;


  if (new_feature) return;


  if (QMessageBox::information (this, tr ("pfmView"), 
                                tr ("Deleting the polygon ungroups sub-features and forces an apply of all changes to this record.  Do you really want to do this?"),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;


  if (!bfd_record.parent_record && bfd_record.child_record)
    {
      binaryFeatureData_read_record (misc->bfd_handle, bfd_record.child_record - 1, &rec);

      bfd_record.child_record = 0;
      binaryFeatureData_write_record (misc->bfd_handle, bfd_record.record_number, &bfd_record, NULL, NULL);

      NV_INT32 next_rec = -1;
      while (rec.child_record)
        {
          next_rec = rec.child_record - 1;

          rec.parent_record = rec.child_record = 0;

          binaryFeatureData_write_record (misc->bfd_handle, rec.record_number, &rec, NULL, NULL);

          binaryFeatureData_read_record (misc->bfd_handle, next_rec, &rec);
        }
      rec.parent_record = rec.child_record = 0;
      binaryFeatureData_write_record (misc->bfd_handle, next_rec, &rec, NULL, NULL);
    }

  slotApplyEditFeature ();
}



void 
editFeature::slotGroup ()
{
  //  First make sure that we have no pre-existing parents or children inside the polygon.


  for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      if (inside (bfd_polygon.longitude, bfd_polygon.latitude, bfd_record.poly_count, misc->feature[i].longitude,
                  misc->feature[i].latitude))
        {
          if (misc->feature[i].parent_record)
            {
              QMessageBox::warning (this, tr ("mosaicView"),
                                    tr ("A sub-feature already exists within the polygon.\nPlease un-group the features first."));
              return;
            }

          if (misc->feature[i].child_record)
            {
              QMessageBox::warning (this, tr ("mosaicView"),
                                    tr ("A master feature already exists within the polygon.\nPlease un-group the features first."));
              return;
            }
        }
    }


  if (QMessageBox::information (this, tr ("mosaicView"), 
                                tr ("Grouping sub-features forces an apply of all changes to this record.  Do you really want to do this?"),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

  if (new_feature)
    {
      //  This should never happen.

      if (!strcmp (misc->abe_share->open_args[0].target_path, "NONE")) return;

      binaryFeatureData_write_record_image_file (misc->bfd_handle, BFDATA_NEXT_RECORD, &bfd_record, &bfd_polygon, image_name);

      binaryFeatureData_close_file (misc->bfd_handle);

      misc->bfd_handle = binaryFeatureData_open_file (misc->abe_share->open_args[0].target_path, &misc->bfd_header, BFDATA_UPDATE);

      binaryFeatureData_read_all_short_features (misc->bfd_handle, &misc->feature);
    }


  //  Save the polygon if it is there (it might have just been created).

  if (binaryFeatureData_write_record (misc->bfd_handle, bfd_record.record_number, &bfd_record, &bfd_polygon, NULL) < 0)
    binaryFeatureData_perror ();


  //  Next, group the features.

  BFDATA_RECORD rec = bfd_record;
  NV_U_INT32 prev_rec = rec.record_number + 1;
  NV_BOOL first = NVTrue;

  for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      if (inside (bfd_polygon.longitude, bfd_polygon.latitude, bfd_record.poly_count, misc->feature[i].longitude,
                  misc->feature[i].latitude))
        {
          if (i != bfd_record.record_number)
            {
              //  Write the record number of this record to the child_record of the previous record.

              rec.child_record = i + 1;
              binaryFeatureData_write_record (misc->bfd_handle, prev_rec - 1, &rec, NULL, NULL);

              if (first)
                {
                  bfd_record.child_record = rec.child_record;
                  first = NVFalse;
                }


              //  Read this record.

              binaryFeatureData_read_record (misc->bfd_handle, i, &rec);


              //  Set the parent record of this record to be the previous record.

              rec.parent_record = prev_rec;

              binaryFeatureData_write_record (misc->bfd_handle, i, &rec, NULL, NULL);

              prev_rec = i + 1;
            }
        }
    }


  slotApplyEditFeature ();
}



void 
editFeature::slotUngroup ()
{
  BFDATA_RECORD rec;


  if (QMessageBox::information (this, tr ("mosaicView"), 
                                tr ("Un-grouping sub-features forces an apply of all changes to this record.  Do you really want to do this?"),
                                QMessageBox::Yes,  QMessageBox::No,  QMessageBox::NoButton) == QMessageBox::No) return;

  if (!bfd_record.child_record || bfd_record.parent_record)
    {
      QMessageBox::warning (this, tr ("mosaicView"), tr ("This is not a master feature."));
      return;
    }


  binaryFeatureData_read_record (misc->bfd_handle, bfd_record.child_record - 1, &rec);

  bfd_record.child_record = 0;
  binaryFeatureData_write_record (misc->bfd_handle, bfd_record.record_number, &bfd_record, NULL, NULL);

  NV_INT32 next_rec = -1;
  while (rec.child_record)
    {
      next_rec = rec.child_record - 1;

      rec.parent_record = rec.child_record = 0;

      binaryFeatureData_write_record (misc->bfd_handle, rec.record_number, &rec, NULL, NULL);

      binaryFeatureData_read_record (misc->bfd_handle, next_rec, &rec);
    }
  rec.parent_record = rec.child_record = 0;
  binaryFeatureData_write_record (misc->bfd_handle, next_rec, &rec, NULL, NULL);


  slotApplyEditFeature ();
}



//  Timer to check to see if the user has finished drawing a polygon in mosaicView (sets the
//  variable misc->feature_polygon_flag to -1 or 1 for discarded or accepted).

void 
editFeature::slotPolyTimer ()
{
  if (misc->feature_polygon_flag)
    {
      polyTimer->stop ();


      if (misc->feature_polygon_flag > 0)
        {
          //  Compute length, width, and orientation.

          NV_FLOAT64 max_dist = -1.0, max_az = 0.0;
          NV_FLOAT64 dist, az;
          NV_INT32 hit[2] = {0, 0};

          for (NV_INT32 i = 0 ; i < misc->poly_count ; i++)
            {
              for (NV_INT32 j = 0 ; j < misc->poly_count ; j++)
                {
                  if (j != i)
                    {
                      invgp (NV_A0, NV_B0, misc->polygon_y[i], misc->polygon_x[i], misc->polygon_y[j], misc->polygon_x[j], 
                             &dist, &az);
                      if (dist > max_dist)
                        {
                          hit[0] = i;
                          hit[1] = j;
                          max_dist = dist;
                          max_az = az;
                        }
                    }
                }
            }


          bfd_record.poly_count = misc->poly_count;
          bfd_record.poly_type = 1;
          for (NV_INT32 i = 0 ; i < misc->poly_count ; i++)
            {
              bfd_polygon.longitude[i] = misc->polygon_x[i];
              bfd_polygon.latitude[i] = misc->polygon_y[i];
            }

          length->setValue (max_dist);
          orientation->setValue (max_az);

          bGroup->setEnabled (TRUE); 
          bUngroup->setEnabled (TRUE); 
          bDelete->setEnabled (TRUE); 
          bPolygon->setEnabled (FALSE); 

          misc->poly_count = 0;
        }


      //  Set the window size and location from what we saved when we hid the dialog.

      this->resize (window_width, window_height);
      this->move (window_x, window_y);

      show ();
    }
}



void 
editFeature::slotRocks ()
{
  rockD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  rockD->setWindowTitle (tr ("mosaicView Rock, Wreck, and Obstruction Types"));
  rockD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (rockD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  rockBox = new QListWidget (rockD);
  rockBox->setSelectionMode (QAbstractItemView::SingleSelection);
  rockBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < ROCK_TYPES ; i++) rockBox->addItem (rockType[i]);

  rockBox->setCurrentRow (options->last_rock_feature_desc);


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
editFeature::slotRockOKClicked ()
{
  description->setText (rockBox->currentItem ()->text ());

  options->last_rock_feature_desc = rockBox->currentRow ();

  rockD->close ();
}


void 
editFeature::slotRockCancelClicked ()
{
  rockD->close ();
}


void 
editFeature::slotOffshore ()
{
  offshoreD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose); 
  offshoreD->setWindowTitle (tr ("mosaicView Offshore Installation Types"));
  offshoreD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (offshoreD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  offshoreBox = new QListWidget (offshoreD);
  offshoreBox->setSelectionMode (QAbstractItemView::SingleSelection);
  offshoreBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < OFFSHORE_TYPES ; i++) offshoreBox->addItem (offshoreType[i]);

  offshoreBox->setCurrentRow (options->last_offshore_feature_desc);


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
editFeature::slotOffshoreOKClicked ()
{
  description->setText (offshoreBox->currentItem ()->text ());

  options->last_offshore_feature_desc = offshoreBox->currentRow ();

  offshoreD->close ();
}


void 
editFeature::slotOffshoreCancelClicked ()
{
  offshoreD->close ();
}


void 
editFeature::slotLights ()
{
  lightD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  lightD->setWindowTitle (tr ("mosaicView Light, Buoy, and Beacon Types"));
  lightD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (lightD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  lightBox = new QListWidget (lightD);
  lightBox->setSelectionMode (QAbstractItemView::SingleSelection);
  lightBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < LIGHT_TYPES ; i++) lightBox->addItem (lightType[i]);

  lightBox->setCurrentRow (options->last_light_feature_desc);


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
editFeature::slotLightOKClicked ()
{
  description->setText (lightBox->currentItem ()->text ());

  options->last_light_feature_desc = lightBox->currentRow ();

  lightD->close ();
}


void 
editFeature::slotLightCancelClicked ()
{
  lightD->close ();
}


void 
editFeature::slotLidar ()
{
  lidarD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  lidarD->setWindowTitle (tr ("mosaicView LIDAR Types"));
  lidarD->resize (800, 500);


  QVBoxLayout *vbox = new QVBoxLayout (lidarD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  lidarBox = new QListWidget (lidarD);
  lidarBox->setSelectionMode (QAbstractItemView::SingleSelection);
  lidarBox->setWhatsThis (typeBoxText);

  for (NV_INT32 i = 0 ; i < LIDAR_TYPES ; i++) lidarBox->addItem (lidarType[i]);

  lidarBox->setCurrentRow (options->last_lidar_feature_desc);


  vbox->addWidget (lidarBox, 10);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (lidarD);
  bHelp->setIcon (QIcon (":/icons/contextHelp"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *OKButton = new QPushButton (tr ("OK"), lidarD);
  OKButton->setToolTip (tr ("Use the selected item as the feature description"));
  connect (OKButton, SIGNAL (clicked ()), this, SLOT (slotLidarOKClicked ()));
  actions->addWidget (OKButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), lidarD);
  cancelButton->setToolTip (tr ("Discard the selection and close the dialog"));
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotLidarCancelClicked ()));
  actions->addWidget (cancelButton);


  lidarD->show ();
}


void 
editFeature::slotLidarOKClicked ()
{
  description->setText (lidarBox->currentItem ()->text ());

  options->last_lidar_feature_desc = lidarBox->currentRow ();

  lidarD->close ();
}


void 
editFeature::slotLidarCancelClicked ()
{
  lidarD->close ();
}
