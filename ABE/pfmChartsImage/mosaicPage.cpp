#include "mosaicPage.hpp"
#include "mosaicPageHelp.hpp"

mosaicPage::mosaicPage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;


  setTitle (tr ("Mosaic/Camera parameters"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmChartsImageWatermark.png"));


  QHBoxLayout *hbox = new QHBoxLayout (this);
  hbox->setMargin (5);
  hbox->setSpacing (5);


  QGroupBox *pBox = new QGroupBox (tr ("Parameter Settings"), this);
  QVBoxLayout *pBoxLayout = new QVBoxLayout;
  pBox->setLayout (pBoxLayout);
  pBoxLayout->setSpacing (10);


  QHBoxLayout *cellBoxLayout = new QHBoxLayout;

  QLabel *cellLabel = new QLabel (tr ("Output Cell Size (m)"), this);
  cellLabel->setToolTip (tr ("Change the output cell size (0.2 - 10.0)"));
  cellLabel->setWhatsThis (cellText);
  cellBoxLayout->addWidget (cellLabel);

  cellSpin = new QDoubleSpinBox (this);
  cellSpin->setDecimals (2);
  cellSpin->setRange (0.2, 10.0);
  cellSpin->setSingleStep (1.0);
  cellSpin->setValue (options->cell_size);
  cellSpin->setWrapping (TRUE);
  cellSpin->setToolTip (tr ("Change the output cell size (0.2 - 10.0)"));
  cellSpin->setWhatsThis (cellText);
  cellBoxLayout->addWidget (cellSpin);

  pBoxLayout->addLayout (cellBoxLayout);


  QHBoxLayout *txt_file_box = new QHBoxLayout (0);
  txt_file_box->setSpacing (8);

  QLabel *txt_file_label = new QLabel (tr ("Mission Parameters File"), this);
  txt_file_box->addWidget (txt_file_label, 1);

  txt_file_edit = new QLineEdit (this);
  txt_file_edit->setReadOnly (TRUE);
  txt_file_box->addWidget (txt_file_edit, 10);

  QPushButton *txt_file_browse = new QPushButton (tr ("Browse..."), this);
  txt_file_box->addWidget (txt_file_browse, 1);

  txt_file_label->setWhatsThis (txt_fileText);
  txt_file_edit->setWhatsThis (txt_fileText);
  txt_file_browse->setWhatsThis (txt_fileBrowseText);

  connect (txt_file_browse, SIGNAL (clicked ()), this, SLOT (slotTxtFileBrowse ()));


  pBoxLayout->addLayout (txt_file_box);


  QHBoxLayout *timeBoxLayout = new QHBoxLayout;

  QLabel *timeLabel = new QLabel (tr ("Time Offset (s)"), this);
  timeLabel->setToolTip (tr ("Set a time offset to be added to the image time"));
  timeLabel->setWhatsThis (timeText);
  timeBoxLayout->addWidget (timeLabel);

  timeSpin = new QDoubleSpinBox (this);
  timeSpin->setDecimals (2);
  timeSpin->setRange (0.0, 1.0);
  timeSpin->setSingleStep (0.05);
  timeSpin->setValue (options->time_offset);
  timeSpin->setWrapping (TRUE);
  timeSpin->setToolTip (tr ("Set a time offset to be added to the image time"));
  timeSpin->setWhatsThis (timeText);
  timeBoxLayout->addWidget (timeSpin);

  pBoxLayout->addLayout (timeBoxLayout);


  QHBoxLayout *rollBoxLayout = new QHBoxLayout;

  QLabel *rollLabel = new QLabel (tr ("Roll Bias"), this);
  rollLabel->setToolTip (tr ("Change the camera boresight roll bias (-30.0 - 30.0)"));
  rollLabel->setWhatsThis (rollText);
  rollBoxLayout->addWidget (rollLabel);

  rollSpin = new QDoubleSpinBox (this);
  rollSpin->setDecimals (4);
  rollSpin->setRange (-30.0, 30.0);
  rollSpin->setSingleStep (1.0);
  rollSpin->setValue (options->roll_bias);
  rollSpin->setWrapping (TRUE);
  rollSpin->setToolTip (tr ("Change the camera boresight roll bias (-30.0 - 30.0)"));
  rollSpin->setWhatsThis (rollText);
  rollBoxLayout->addWidget (rollSpin);

  pBoxLayout->addLayout (rollBoxLayout);


  QHBoxLayout *pitchBoxLayout = new QHBoxLayout;

  QLabel *pitchLabel = new QLabel (tr ("Pitch Bias"), this);
  pitchLabel->setToolTip (tr ("Change the camera boresight pitch bias (-30.0 - 30.0)"));
  pitchLabel->setWhatsThis (pitchText);
  pitchBoxLayout->addWidget (pitchLabel);

  pitchSpin = new QDoubleSpinBox (this);
  pitchSpin->setDecimals (4);
  pitchSpin->setRange (-30.0, 30.0);
  pitchSpin->setSingleStep (1.0);
  pitchSpin->setValue (options->pitch_bias);
  pitchSpin->setWrapping (TRUE);
  pitchSpin->setToolTip (tr ("Change the camera boresight pitch bias (-30.0 - 30.0)"));
  pitchSpin->setWhatsThis (pitchText);
  pitchBoxLayout->addWidget (pitchSpin);

  pBoxLayout->addLayout (pitchBoxLayout);



  QHBoxLayout *headingBoxLayout = new QHBoxLayout;

  QLabel *headingLabel = new QLabel (tr ("Heading Bias"), this);
  headingLabel->setToolTip (tr ("Change the camera boresight heading bias (-30.0 - 30.0)"));
  headingLabel->setWhatsThis (headingText);
  headingBoxLayout->addWidget (headingLabel);

  headingSpin = new QDoubleSpinBox (this);
  headingSpin->setDecimals (4);
  headingSpin->setRange (-30.0, 30.0);
  headingSpin->setSingleStep (1.0);
  headingSpin->setValue (options->heading_bias);
  headingSpin->setWrapping (TRUE);
  headingSpin->setToolTip (tr ("Change the camera boresight heading bias (-30.0 - 30.0)"));
  headingSpin->setWhatsThis (headingText);
  headingBoxLayout->addWidget (headingSpin);

  pBoxLayout->addLayout (headingBoxLayout);



  QHBoxLayout *focalBoxLayout = new QHBoxLayout;

  QLabel *focalLabel = new QLabel (tr ("Camera Focal Length"), this);
  focalLabel->setToolTip (tr ("Change the camera focal length (mm)"));
  focalLabel->setWhatsThis (focalText);
  focalBoxLayout->addWidget (focalLabel);

  focalSpin = new QDoubleSpinBox (this);
  focalSpin->setDecimals (3);
  focalSpin->setRange (0.0, 200.0);
  focalSpin->setSingleStep (10.0);
  focalSpin->setValue (options->focal_length);
  focalSpin->setWrapping (TRUE);
  focalSpin->setToolTip (tr ("Change the camera focal length (mm)"));
  focalSpin->setWhatsThis (focalText);
  focalBoxLayout->addWidget (focalSpin);

  pBoxLayout->addLayout (focalBoxLayout);



  QHBoxLayout *pixelBoxLayout = new QHBoxLayout;

  QLabel *pixelLabel = new QLabel (tr ("Camera Pixel Size (u)"), this);
  pixelLabel->setToolTip (tr ("Change the camera pixel size"));
  pixelLabel->setWhatsThis (pixelText);
  pixelBoxLayout->addWidget (pixelLabel);

  pixelSpin = new QDoubleSpinBox (this);
  pixelSpin->setDecimals (2);
  pixelSpin->setRange (1.0, 100.0);
  pixelSpin->setSingleStep (10.0);
  pixelSpin->setValue (options->pixel_size);
  pixelSpin->setWrapping (TRUE);
  pixelSpin->setToolTip (tr ("Change the camera pixel size"));
  pixelSpin->setWhatsThis (pixelText);
  pixelBoxLayout->addWidget (pixelSpin);

  pBoxLayout->addLayout (pixelBoxLayout);



  QHBoxLayout *colBoxLayout = new QHBoxLayout;

  QLabel *colLabel = new QLabel (tr ("Camera Column Offset"), this);
  colLabel->setToolTip (tr ("Change the camera column offset"));
  colLabel->setWhatsThis (colText);
  colBoxLayout->addWidget (colLabel);

  colSpin = new QDoubleSpinBox (this);
  colSpin->setDecimals (2);
  colSpin->setRange (-500.0, 500.0);
  colSpin->setSingleStep (10.0);
  colSpin->setValue (options->column_offset);
  colSpin->setWrapping (TRUE);
  colSpin->setToolTip (tr ("Change the camera column offset"));
  colSpin->setWhatsThis (colText);
  colBoxLayout->addWidget (colSpin);

  pBoxLayout->addLayout (colBoxLayout);


  QHBoxLayout *rowBoxLayout = new QHBoxLayout;

  QLabel *rowLabel = new QLabel (tr ("Camera Row Offset"), this);
  rowLabel->setToolTip (tr ("Change the camera row offset"));
  rowLabel->setWhatsThis (rowText);
  rowBoxLayout->addWidget (rowLabel);

  rowSpin = new QDoubleSpinBox (this);
  rowSpin->setDecimals (2);
  rowSpin->setRange (-500.0, 500.0);
  rowSpin->setSingleStep (10.0);
  rowSpin->setValue (options->row_offset);
  rowSpin->setWrapping (TRUE);
  rowSpin->setToolTip (tr ("Change the camera row offset"));
  rowSpin->setWhatsThis (rowText);
  rowBoxLayout->addWidget (rowSpin);

  pBoxLayout->addLayout (rowBoxLayout);


  hbox->addWidget (pBox);


  registerField ("cellSpin", cellSpin, "value");
  registerField ("timeSpin", timeSpin, "value");
  registerField ("rollSpin", rollSpin, "value");
  registerField ("pitchSpin", pitchSpin, "value");
  registerField ("headingSpin", headingSpin, "value");
  registerField ("focalSpin", focalSpin, "value");
  registerField ("pixelSpin", pixelSpin, "value");
  registerField ("colSpin", colSpin, "value");
  registerField ("rowSpin", rowSpin, "value");
}



void mosaicPage::slotTxtFileBrowse ()
{
  QStringList         files, filters;
  QString             file;


  QFileDialog *fd = new QFileDialog (this, tr ("pfmChartsImage Select Mission Parameters File"));
  fd->setViewMode (QFileDialog::List);


  //  Always add the current working directory and the last used directory to the sidebar URLs in case we're running from the command line.
  //  This function is in the nvutility library.

  setSidebarUrls (fd, options->param_dir);


  filters << tr ("Text (*.txt)");

  fd->setFilters (filters);
  fd->setFileMode (QFileDialog::ExistingFile);
  fd->selectFilter (tr ("Text (*.txt)"));

  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      QString txt_file_name = files.at (0);


      if (!txt_file_name.isEmpty())
        {
          txt_file_edit->setText (txt_file_name);

	  NV_CHAR txt_file[512];
	  strcpy (txt_file, txt_file_name.toAscii ());

	  FILE *fp;

	  if ((fp = fopen (txt_file, "r")) == NULL)
	    {
	      QMessageBox::warning (this, tr ("Select Mission Parameters File"),
				    tr ("The file ") + txt_file_name + 
				    tr (" could not be opened.") +
				    tr ("  The error message returned was:\n\n") +
				    QString (strerror (errno)));

              return;
	    }


	  NV_CHAR string[128];
	  NV_FLOAT64 roll_bias = 0.0, pitch_bias = 0.0, heading_bias = 0.0, pixel_size = 0.0, focal_length = 0.0, 
	    col_offset = 0.0, row_offset = 0.0, time_delay = 0.0;
          NV_INT32 ms;

	  while (fgets (string, sizeof (string), fp) != NULL)
	    {
	      if (strstr (string, "camera_boresight_roll:"))
		sscanf (string, "camera_boresight_roll: %lf", &roll_bias);

	      if (strstr (string, "camera_boresight_pitch:"))
		sscanf (string, "camera_boresight_pitch: %lf", &pitch_bias);

	      if (strstr (string, "camera_boresight_heading:"))
		sscanf (string, "camera_boresight_heading: %lf", &heading_bias);

	      if (strstr (string, "pixel_size:"))
		sscanf (string, "pixel_size: %lf", &pixel_size);

	      if (strstr (string, "focal_length:"))
		sscanf (string, "focal_length: %lf", &focal_length);

	      if (strstr (string, "principal_point_offsets:"))
		sscanf (string, "principal_point_offsets: %lf, %lf", &col_offset, &row_offset);

	      if (strstr (string, "camera_trig_delay:"))
                {
                  sscanf (string, "camera_trig_delay: %d", &ms);
                  time_delay = (NV_FLOAT64) ms / 1000.0;
                }
	    }
	  fclose (fp);

	  timeSpin->setValue (time_delay);
	  rollSpin->setValue (roll_bias);
	  pitchSpin->setValue (pitch_bias);
	  headingSpin->setValue (heading_bias);
	  focalSpin->setValue (focal_length);
	  pixelSpin->setValue (pixel_size);
	  colSpin->setValue (col_offset);
	  rowSpin->setValue (row_offset);
        }

      options->param_dir = fd->directory ().absolutePath ();
    }
}
