
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



#include "imagePage.hpp"
#include "imagePageHelp.hpp"

imagePage::imagePage (QWidget *parent, OPTIONS *op):
  QWizardPage (parent)
{
  options = op;
  hold_display = NVFalse;


  setTitle (tr ("Image parameters"));

  setPixmap (QWizard::WatermarkPixmap, QPixmap(":/icons/pfmGeotiffWatermark.png"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);
  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout (hbox);


  QGroupBox *pBox = new QGroupBox (tr ("Parameter Settings"), this);
  QVBoxLayout *pBoxLayout = new QVBoxLayout;
  pBox->setLayout (pBoxLayout);


  QHBoxLayout *angBoxLayout = new QHBoxLayout;

  QLabel *sunAzLabel = new QLabel (tr ("Sun Azimuth"), this);
  sunAzLabel->setToolTip (tr ("Change the sun angle (0.0-360.0)"));
  sunAzLabel->setWhatsThis (sunAzText);
  angBoxLayout->addWidget (sunAzLabel);

  sunAz = new QDoubleSpinBox (this);
  sunAz->setDecimals (1);
  sunAz->setRange (0.0, 359.9);
  sunAz->setSingleStep (1.0);
  sunAz->setValue (options->azimuth);
  sunAz->setWrapping (TRUE);
  sunAz->setToolTip (tr ("Change the sun angle (0.0-360.0)"));
  sunAz->setWhatsThis (sunAzText);
  connect (sunAz, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  angBoxLayout->addWidget (sunAz);

  pBoxLayout->addLayout (angBoxLayout);


  QHBoxLayout *eleBoxLayout = new QHBoxLayout;

  QLabel *sunElLabel = new QLabel (tr ("Sun Elevation"), this);
  sunElLabel->setToolTip (tr ("Change the sun elevation (0.0-90.0)"));
  sunElLabel->setWhatsThis (sunElText);
  eleBoxLayout->addWidget (sunElLabel);

  sunEl = new QDoubleSpinBox (this);
  sunEl->setDecimals (1);
  sunEl->setRange (0.0, 90.0);
  sunEl->setSingleStep (1.0);
  sunEl->setValue (options->elevation);
  sunEl->setWrapping (TRUE);
  sunEl->setToolTip (tr ("Change the sun elevation (0.0-90.0)"));
  sunEl->setWhatsThis (sunElText);
  connect (sunEl, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  eleBoxLayout->addWidget (sunEl);

  pBoxLayout->addLayout (eleBoxLayout);



  QHBoxLayout *exaBoxLayout = new QHBoxLayout;

  QLabel *sunExLabel = new QLabel (tr ("Sun Exaggeration"), this);
  sunExLabel->setToolTip (tr ("Change the sun Z exaggeration (1.0-10.0)"));
  sunExLabel->setWhatsThis (sunExText);
  exaBoxLayout->addWidget (sunExLabel);

  sunEx = new QDoubleSpinBox (this);
  sunEx->setDecimals (1);
  sunEx->setRange (1.0, 10.0);
  sunEx->setSingleStep (1.0);
  sunEx->setValue (options->exaggeration);
  sunEx->setWrapping (TRUE);
  sunEx->setToolTip (tr ("Change the sun Z exaggeration (1.0-10.0)"));
  sunEx->setWhatsThis (sunExText);
  connect (sunEx, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  exaBoxLayout->addWidget (sunEx);

  pBoxLayout->addLayout (exaBoxLayout);



  QHBoxLayout *satBoxLayout = new QHBoxLayout;

  QLabel *satLabel = new QLabel (tr ("Color Saturation"), this);
  satLabel->setToolTip (tr ("Change the color saturation (0.0-1.0)"));
  satLabel->setWhatsThis (saturationText);
  satBoxLayout->addWidget (satLabel);

  satSpin = new QDoubleSpinBox (this);
  satSpin->setDecimals (2);
  satSpin->setRange (0.0, 1.0);
  satSpin->setSingleStep (0.05);
  satSpin->setValue (options->saturation);
  satSpin->setWrapping (TRUE);
  satSpin->setToolTip (tr ("Change the color saturation (0.0-1.0)"));
  satSpin->setWhatsThis (saturationText);
  connect (satSpin, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  satBoxLayout->addWidget (satSpin);

  pBoxLayout->addLayout (satBoxLayout);


  QHBoxLayout *valBoxLayout = new QHBoxLayout;

  QLabel *valLabel = new QLabel (tr ("Color Value"), this);
  valLabel->setToolTip (tr ("Change the color value (0.0-1.0)"));
  valLabel->setWhatsThis (valueText);
  valBoxLayout->addWidget (valLabel);

  valSpin = new QDoubleSpinBox (this);
  valSpin->setDecimals (2);
  valSpin->setRange (0.0, 1.0);
  valSpin->setSingleStep (0.05);
  valSpin->setValue (options->value);
  valSpin->setWrapping (TRUE);
  valSpin->setToolTip (tr ("Change the color value (0.0-1.0)"));
  valSpin->setWhatsThis (valueText);
  connect (valSpin, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  valBoxLayout->addWidget (valSpin);

  pBoxLayout->addLayout (valBoxLayout);

 
  QHBoxLayout *startBoxLayout = new QHBoxLayout;

  startLabel = new QLabel (tr ("Start Color"), this);
  startPalette = startLabel->palette ();
  startLabel->setAutoFillBackground (TRUE);
  startLabel->setToolTip (tr ("Change the start color value (0.0-360.0)"));
  startLabel->setWhatsThis (start_hueText);
  startBoxLayout->addWidget (startLabel);

  startSpin = new QDoubleSpinBox (this);
  startSpin->setDecimals (1);
  startSpin->setRange (0.0, 360.0);
  startSpin->setSingleStep (1.0);
  startSpin->setValue (options->start_hsv);
  startSpin->setWrapping (TRUE);
  startSpin->setToolTip (tr ("Change the start color value (0.0-360.0)"));
  startSpin->setWhatsThis (start_hueText);
  connect (startSpin, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  startBoxLayout->addWidget (startSpin);

  pBoxLayout->addLayout (startBoxLayout);


  QHBoxLayout *endBoxLayout = new QHBoxLayout;

  endLabel = new QLabel (tr ("End Color"), this);
  endPalette = endLabel->palette ();
  endLabel->setAutoFillBackground (TRUE);
  endLabel->setToolTip (tr ("Change the end color value (0.0-360.0)"));
  endLabel->setWhatsThis (end_hueText);
  endBoxLayout->addWidget (endLabel);

  endSpin = new QDoubleSpinBox (this);
  endSpin->setDecimals (1);
  endSpin->setRange (0.0, 360.0);
  endSpin->setSingleStep (1.0);
  endSpin->setValue (options->end_hsv);
  endSpin->setWrapping (TRUE);
  endSpin->setToolTip (tr ("Change the end color value (0.0-360.0)"));
  endSpin->setWhatsThis (end_hueText);
  connect (endSpin, SIGNAL (valueChanged (double)), this, SLOT (slotParamChanged (double)));
  endBoxLayout->addWidget (endSpin);

  pBoxLayout->addLayout (endBoxLayout);


  hbox->addWidget (pBox, 1);


  QGroupBox *sBox = new QGroupBox (tr ("Sample Settings"), this);
  QHBoxLayout *sampBoxLayout = new QHBoxLayout;
  sBox->setLayout (sampBoxLayout);
  QVBoxLayout *sBoxLeftLayout = new QVBoxLayout;
  QVBoxLayout *sBoxRightLayout = new QVBoxLayout;
  sampBoxLayout->addLayout (sBoxLeftLayout);
  sampBoxLayout->addLayout (sBoxRightLayout);


  lgs = new QRadioButton (tr ("Light Gray Scale"));
  mgs = new QRadioButton (tr ("Medium Gray Scale"));
  r2b = new QRadioButton (tr ("Red To Blue"));
  lrb = new QRadioButton (tr ("Light Red To Blue"));
  r2m = new QRadioButton (tr ("Red To Magenta"));
  m2g = new QRadioButton (tr ("Magenta To Green"));

  QButtonGroup *sample_group = new QButtonGroup (this);
  sample_group->setExclusive (TRUE);
  connect (sample_group, SIGNAL (buttonClicked (int)), this, SLOT (slotSampleGroupClicked (int)));

  sample_group->addButton (lgs, 0);
  sample_group->addButton (mgs, 1);
  sample_group->addButton (r2b, 2);
  sample_group->addButton (lrb, 3);
  sample_group->addButton (r2m, 4);
  sample_group->addButton (m2g, 5);

  sBoxLeftLayout->addWidget (lgs);
  sBoxLeftLayout->addWidget (mgs);
  sBoxLeftLayout->addWidget (r2b);
  sBoxLeftLayout->addWidget (lrb);
  sBoxLeftLayout->addWidget (r2m);
  sBoxLeftLayout->addWidget (m2g);
  
  lgs->setWhatsThis (sample0Text);
  mgs->setWhatsThis (sample1Text);
  r2b->setWhatsThis (sample2Text);
  lrb->setWhatsThis (sample3Text);
  r2m->setWhatsThis (sample4Text);
  m2g->setWhatsThis (sample5Text);


  sample_label = new QLabel (sBox);
  sample_label->setPixmap (options->sample_pixmap);
  sample_label->show ();
  sBoxRightLayout->addWidget (sample_label);


  hbox->addWidget (sBox);


  QGroupBox *rBox = new QGroupBox (tr ("Restart color map at zero"), this);
  QHBoxLayout *rBoxLayout = new QHBoxLayout;
  rBox->setLayout (rBoxLayout);
  restart_check = new QCheckBox (rBox);
  restart_check->setToolTip (tr ("Start colormap over at zero"));
  restart_check->setWhatsThis (restartText);
  restart_check->setChecked (options->restart);
  rBoxLayout->addWidget (restart_check);

  connect (restart_check, SIGNAL (clicked ()), this, SLOT (slotRestartClicked (void)));

  vbox->addWidget (rBox);


  display_sample_data ();


  registerField ("sunAz", sunAz, "value");
  registerField ("sunEl", sunEl, "value");
  registerField ("sunEx", sunEx, "value");
  registerField ("satSpin", satSpin, "value");
  registerField ("valSpin", valSpin, "value");
  registerField ("startSpin", startSpin, "value");
  registerField ("endSpin", endSpin, "value");
}



void imagePage::enable (NV_BOOL state)
{
  sunAz->setEnabled (state);
  sunEl->setEnabled (state);
  sunEx->setEnabled (state);
  satSpin->setEnabled (state);
  valSpin->setEnabled (state);
  startSpin->setEnabled (state);
  endSpin->setEnabled (state);
  restart_check->setEnabled (state);
  lgs->setEnabled (state);
  mgs->setEnabled (state);
  r2b->setEnabled (state);
  lrb->setEnabled (state);
  r2m->setEnabled (state);
  m2g->setEnabled (state);

  display_sample_data ();
}



void imagePage::slotRestartClicked ()
{
  if (restart_check->checkState ())
    {
      options->restart = NVTrue;
    }
  else
    {
      options->restart = NVFalse;
    }

  display_sample_data ();
}



//  We don't really care about the value, we just want to trigger the redraw of the sample.

void imagePage::slotParamChanged (double d __attribute__ ((unused)))
{
  //  Don't trigger every time slotSampleGroupClicked (below) changes a parameter.

  if (!hold_display) display_sample_data ();
}



void imagePage::slotSampleGroupClicked (int id)
{
  hold_display = NVTrue;

  switch (id)
    {
    case 0:
      satSpin->setValue (0.0);
      valSpin->setValue (0.75);
      startSpin->setValue (0.0);
      endSpin->setValue (240.0);

      break;

    case 1:
      satSpin->setValue (0.0);
      valSpin->setValue (0.35);
      startSpin->setValue (0.0);
      endSpin->setValue (240.0);

      break;

    case 2:
      satSpin->setValue (1.0);
      valSpin->setValue (0.0);
      startSpin->setValue (0.0);
      endSpin->setValue (240.0);

      break;

    case 3:
      satSpin->setValue (0.75);
      valSpin->setValue (0.75);
      startSpin->setValue (0.0);
      endSpin->setValue (240.0);

      break;

    case 4:
      satSpin->setValue (1.0);
      valSpin->setValue (0.0);
      startSpin->setValue (0.0);
      endSpin->setValue (315.0);

      break;

    case 5:
      satSpin->setValue (1.0);
      valSpin->setValue (0.0);
      startSpin->setValue (315.0);
      endSpin->setValue (120.0);

      break;
    }

  hold_display = NVFalse;

  display_sample_data ();
}



void imagePage::display_sample_data ()
{
  NV_FLOAT32          row[2][SAMPLE_WIDTH], range[2] = {0.0, 0.0}, shade_factor;
  NV_INT32            c_index = 0, hue, sat;
  NV_BOOL             cross_zero = NVFalse;

  void palshd (int num_shades, int num_hues, float start_hue, float end_hue, 
               float min_saturation, float max_saturation, float min_value, 
               float max_value, int start_color, QColor color_array[]);


  if (restart_check->checkState () && options->sample_min < 0.0)
    {
      range[0] = -options->sample_min;
      range[1] = options->sample_max;

      cross_zero = NVTrue;
    }
  else
    {
      range[0] = options->sample_max - options->sample_min;

      cross_zero = NVFalse;
    }


  options->sunopts.azimuth = sunAz->value ();
  options->sunopts.elevation = sunEl->value ();
  options->sunopts.exag = sunEx->value ();
  options->sunopts.power_cos = 1.0;
  options->sunopts.num_shades = 50;
  options->sunopts.min_shade = 0.0;
  options->sunopts.sun = sun_unv (options->sunopts.azimuth, options->sunopts.elevation);


  if (options->grey)
    {
      palshd (NUMSHADES, NUMHUES, 240.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0, options->color_array);
    }
  else
    {
      palshd (NUMSHADES, NUMHUES, (NV_FLOAT32) endSpin->value (), (NV_FLOAT32) startSpin->value (), 
              (NV_FLOAT32) satSpin->value (), (NV_FLOAT32) satSpin->value (), (NV_FLOAT32) valSpin->value (),
              1.0, 0, options->color_array);
    }

  //  Set the start and end label background colors

  hue = options->color_array[(NUMHUES - 1) * NUMSHADES].hue ();
  sat = options->color_array[(NUMHUES - 1) * NUMSHADES].saturation ();
  if (hue > 210 && hue < 280 && sat > 128)
    {
      startPalette.setColor (QPalette::Normal, QPalette::WindowText, Qt::white);
      startPalette.setColor (QPalette::Inactive, QPalette::WindowText, Qt::white);
    }
  else
    {
      startPalette.setColor (QPalette::Normal, QPalette::WindowText, Qt::black);
      startPalette.setColor (QPalette::Inactive, QPalette::WindowText, Qt::black);
    }
  startPalette.setColor (QPalette::Normal, QPalette::Window, options->color_array[(NUMHUES - 1) * NUMSHADES]);
  startPalette.setColor (QPalette::Inactive, QPalette::Window, options->color_array[(NUMHUES - 1) * NUMSHADES]);
  startLabel->setPalette (startPalette);


  hue = options->color_array[0].hue ();
  sat = options->color_array[0].saturation ();
  if (hue > 210 && hue < 280 && sat > 128)
    {
      endPalette.setColor (QPalette::Normal, QPalette::WindowText, Qt::white);
      endPalette.setColor (QPalette::Inactive, QPalette::WindowText, Qt::white);
    }
  else
    {
      endPalette.setColor (QPalette::Normal, QPalette::WindowText, Qt::black);
      endPalette.setColor (QPalette::Inactive, QPalette::WindowText, Qt::black);
    }
  endPalette.setColor (QPalette::Normal, QPalette::Window, options->color_array[0]);
  endPalette.setColor (QPalette::Inactive, QPalette::Window, options->color_array[0]);
  endLabel->setPalette (endPalette);


  QBrush brush;
  options->sample_pixmap.fill (this, 0, 0);

  QPainter painter;
  painter.begin (&options->sample_pixmap);


  for (NV_INT32 i = 0 ; i < SAMPLE_HEIGHT ; i++)
    {
      for (NV_INT32 j = 0 ; j < SAMPLE_WIDTH ; j++)
        {
          if (i) row[0][j] = row[1][j];
          row[1][j] = (NV_FLOAT32) options->sample_data[i][j];
        }

      if (i && i < SAMPLE_HEIGHT)
        {
          for (NV_INT32 k = 0 ; k < SAMPLE_WIDTH ; k++)
            {
              if (cross_zero)
                {
                  if (row[0][k] < 0.0)
                    {
                      c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf ((row[0][k] - options->sample_min) / range[0] * 
                                                                         NUMHUES))) * NUMSHADES;
                    }
                  else
                    {
                      c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf (row[0][k]) / range[1] * NUMHUES)) *
                        NUMSHADES;
                    }
                }
              else
                {
                  c_index = (NV_INT32) (NUMHUES - (NV_INT32) (fabsf ((row[0][k] - options->sample_min) / range[0] * NUMHUES))) *
                    NUMSHADES;
                }


              //  IMPORTANT NOTE: The cell sizes are hardwired for the sample data in icons/data.dat.  I wouldn't
              //  recommend trying to change any of this.

              shade_factor = sunshade (row[0], row[1], k, &options->sunopts, 185.0, 185.0);

              if (shade_factor < 0.0) shade_factor = options->sunopts.min_shade;

              if (shade_factor > 1.0) shade_factor = 1.0;

              c_index -= NINT (NUMSHADES * shade_factor + 0.5);


              painter.setPen (Qt::NoPen);

              brush.setStyle (Qt::SolidPattern);
              brush.setColor (options->color_array[c_index]);
              painter.setPen (options->color_array[c_index]);

              painter.fillRect (k, SAMPLE_HEIGHT - i, 1, 1, brush);
            }
        }
    }

  painter.end ();

  sample_label->setPixmap (options->sample_pixmap);
}
