
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



#include "lockValue.hpp"
#include "lockValueHelp.hpp"


//!  This dialog is created when we want to change the color or lock the value of the minimum or maximum scale box.

lockValue::lockValue (QWidget * parent, OPTIONS *op, MISC *mi, NV_BOOL min, NV_INT32 attribute):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  mod_options = *options;
  misc = mi;
  min_flag = min;
  attr = attribute;


  setModal (TRUE);


  //  This is just too confusing.  If we're using anything other than depth we have to flip the min and max with the exception of the
  //  color.  This is because we're hydrographers and positive Z is down for depth/elevation but positive Z is up for everything else.

  QString type, ctype;
  NV_FLOAT32 value;
  NV_INT32 color;
  NV_BOOL lock;
  QString valueText, spinText;

  if (min_flag)
    {
      if (attr)
        {
          type = tr ("maximum");
          ctype = tr ("Maximum");
          value = mod_options.max_hsv_value[attr];
          color = mod_options.min_hsv_color[attr];
          lock = mod_options.max_hsv_locked[attr];
          valueText = maxValueText;
          spinText = maxSpinText;
        }
      else
        {
          type = tr ("minimum");
          ctype = tr ("Minimum");
          value = mod_options.min_hsv_value[attr];
          color = mod_options.min_hsv_color[attr];
          lock = mod_options.min_hsv_locked[attr];
          valueText = minValueText;
          spinText = minSpinText;
        }
    }
  else
    {
      if (attr)
        {
          type = tr ("minimum");
          ctype = tr ("Minimum");
          value = mod_options.min_hsv_value[attr];
          color = mod_options.max_hsv_color[attr];
          lock = mod_options.min_hsv_locked[attr];
          valueText = minValueText;
          spinText = minSpinText;
        }
      else
        {
          type = tr ("maximum");
          ctype = tr ("Maximum");
          value = mod_options.max_hsv_value[attr];
          color = mod_options.max_hsv_color[attr];
          lock = mod_options.max_hsv_locked[attr];
          valueText = maxValueText;
          spinText = maxSpinText;
        }
    }


  //  Make the default single step be the gap between scale box values.

  NV_FLOAT64 single_step;
  if (attribute)
    {
      single_step = misc->attr_color_range / (NV_FLOAT64) (NUM_SCALE_LEVELS - 1);
    }
  else
    {
      single_step = misc->color_range_z / (NV_FLOAT64) (NUM_SCALE_LEVELS - 1);
    }


  setWindowTitle (tr ("pfmEdit ") + ctype + tr (" Scale Lock"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QButtonGroup *lockGrp = new QButtonGroup (this);
  connect (lockGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotLock (int)));
  lockGrp->setExclusive (TRUE);

  QHBoxLayout *hbox = new QHBoxLayout;
  vbox->addLayout (hbox);

  QGroupBox *lvalueBox = new QGroupBox (ctype + tr (" HSV value"), this);
  QHBoxLayout *lvalueBoxLayout = new QHBoxLayout;
  lvalueBox->setLayout (lvalueBoxLayout);
  lvalue = new QDoubleSpinBox (lvalueBox);
  lvalue->setDecimals (2);
  lvalue->setRange (-9500.0, 11500.0);
  lvalue->setSingleStep (single_step);
  lvalue->setToolTip (tr ("Change the ") + type + tr ("HSV data value"));
  lvalue->setValue (value);
  connect (lvalue, SIGNAL (valueChanged (double)), this, SLOT (slotLvalueValueChanged (double)));
  lvalue->setWhatsThis (valueText);
  lvalueBoxLayout->addWidget (lvalue);

  QPushButton *bLock = new QPushButton (tr ("Lock"), this);
  bLock->setToolTip (tr ("Lock the ") + type + tr (" value"));
  bLock->setWhatsThis (lockText);
  bLock->setCheckable (TRUE);
  lockGrp->addButton (bLock, 1);
  lvalueBoxLayout->addWidget (bLock);

  QPushButton *bUnlock = new QPushButton (tr ("Unlock"), this);
  bUnlock->setToolTip (tr ("Unlock the ") + type + tr (" value"));
  bUnlock->setWhatsThis (unlockText);
  bUnlock->setCheckable (TRUE);
  lockGrp->addButton (bUnlock, 0);
  lvalueBoxLayout->addWidget (bUnlock);

  if (lock)
    {
      bLock->setChecked (TRUE);
    }
  else
    {
      bUnlock->setChecked (TRUE);
    }

  hbox->addWidget (lvalueBox);


  QGroupBox *colorBox = new QGroupBox (ctype + tr ("HSV color value"), this);
  QHBoxLayout *colorBoxLayout = new QHBoxLayout;
  colorBox->setLayout (colorBoxLayout);
  colorSpin = new QSpinBox (colorBox);
  colorSpin->setRange (0, 315);
  colorSpin->setSingleStep (5);
  colorSpin->setToolTip (tr ("Change the HSV color value used for the ") + type + tr (" value in the surface"));
  colorSpin->setWhatsThis (spinText);
  colorSpinPalette = colorSpin->palette ();
  connect (colorSpin, SIGNAL (valueChanged (int)), this, SLOT (slotColorSpinValueChanged (int)));
  colorBoxLayout->addWidget (colorSpin);
  hbox->addWidget (colorBox);

  slotColorSpinValueChanged (color);

  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *acceptButton = new QPushButton (tr ("Accept"), this);
  acceptButton->setToolTip (tr ("Accept changes and close dialog"));
  connect (acceptButton, SIGNAL (clicked ()), this, SLOT (slotAcceptLockValue ()));
  actions->addWidget (acceptButton);

  QPushButton *rejectButton = new QPushButton (tr ("Reject"), this);
  rejectButton->setToolTip (tr ("Reject changes and close dialog"));
  connect (rejectButton, SIGNAL (clicked ()), this, SLOT (slotRejectLockValue ()));
  actions->addWidget (rejectButton);


  show ();
}



lockValue::~lockValue ()
{
}



void
lockValue::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
lockValue::slotLock (int locked)
{
  if ((min_flag && !attr) || (!min_flag && attr))
    {
      if (locked)
        {
          mod_options.min_hsv_value[attr] = (NV_FLOAT32) lvalue->value ();
          mod_options.min_hsv_locked[attr] = NVTrue;
        }
      else
        {
          mod_options.min_hsv_value[attr] = options->min_hsv_value[attr];
          lvalue->setValue (mod_options.min_hsv_value[attr]);
          mod_options.min_hsv_locked[attr] = NVFalse;
        }
    }
  else
    {
      if (locked)
        {
          mod_options.max_hsv_value[attr] = (NV_FLOAT32) lvalue->value ();
          mod_options.max_hsv_locked[attr] = NVTrue;
        }
      else
        {
          mod_options.max_hsv_value[attr] = options->max_hsv_value[attr];
          lvalue->setValue (mod_options.max_hsv_value[attr]);
          mod_options.max_hsv_locked[attr] = NVFalse;
        }
    }
}



void
lockValue::slotLvalueValueChanged (double value)
{
  if ((min_flag && !attr) || (!min_flag && attr))
    {
      mod_options.min_hsv_value[attr] = value;
    }
  else
    {
      mod_options.max_hsv_value[attr] = value;
    }
}



void
lockValue::slotColorSpinValueChanged (int value)
{
  if (min_flag)
    {
      if (value < mod_options.max_hsv_color[attr])
        {
          mod_options.min_hsv_color[attr] = value;

          QColor clr;

          clr.setHsv (mod_options.min_hsv_color[attr], 255, 255, 255);

          colorSpinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
          colorSpinPalette.setColor (QPalette::Normal, QPalette::Button, clr);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::Button, clr);
          colorSpinPalette.setColor (QPalette::Normal, QPalette::Window, clr);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::Window, clr);
          colorSpin->setPalette (colorSpinPalette);
          colorSpin->setValue (mod_options.min_hsv_color[attr]);
        }
    }
  else
    {
      if (value > mod_options.min_hsv_color[attr])
        {
          mod_options.max_hsv_color[attr] = value;

          QColor clr;

          clr.setHsv (mod_options.max_hsv_color[attr], 255, 255, 255);

          colorSpinPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
          colorSpinPalette.setColor (QPalette::Normal, QPalette::Button, clr);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::Button, clr);
          colorSpinPalette.setColor (QPalette::Normal, QPalette::Window, clr);
          colorSpinPalette.setColor (QPalette::Inactive, QPalette::Window, clr);
          colorSpin->setPalette (colorSpinPalette);
          colorSpin->setValue (mod_options.max_hsv_color[attr]);
        }
    }
}



void
lockValue::slotAcceptLockValue ()
{
  hide ();

  *options = mod_options;

  emit lockValueDone (NVTrue);

  close ();
}



void
lockValue::slotRejectLockValue ()
{
  hide ();

  emit lockValueDone (NVFalse);

  close ();
}
