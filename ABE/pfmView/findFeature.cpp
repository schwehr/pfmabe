
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



#include "findFeature.hpp"
#include "findFeatureHelp.hpp"

//!  This is the find feature dialog.

findFeature::findFeature (QWidget * parent, nvMap *co, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  srch_strng = "";
  options = op;
  misc = mi;
  cov = co;
  hot_id = prev_hot_id = -1;


  setWindowTitle (tr ("pfmView Find Feature"));
  resize (500, 500);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QGroupBox *sbox = new QGroupBox (tr ("Search string"), this);
  QVBoxLayout *sboxLayout = new QVBoxLayout;
  sbox->setLayout (sboxLayout);


  searchString = new QLineEdit (parent);
  searchString->setText (srch_strng);
  searchString->setWhatsThis (searchText);
  connect (searchString, SIGNAL (textEdited (const QString &)), this, SLOT (slotTextEdited (const QString &)));
  sboxLayout->addWidget (searchString);
  vbox->addWidget (sbox);


  QGroupBox *tbox = new QGroupBox (tr ("Feature Information"), this);
  QVBoxLayout *tboxLayout = new QVBoxLayout;
  tbox->setLayout (tboxLayout);


  featureBox = new QListWidget (this);
  featureBox->setWhatsThis (featureBoxText);
  featureBox->setSelectionMode (QAbstractItemView::SingleSelection);
  connect (featureBox, SIGNAL (itemClicked (QListWidgetItem *)), this, SLOT (slotItemClicked (QListWidgetItem *)));
  tboxLayout->addWidget (featureBox);

  vbox->addWidget (tbox, 0, 0);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the Find Feature dialog"));
  closeButton->setWhatsThis (closeFTText);
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClose ()));
  actions->addWidget (closeButton);


  findTimer = new QTimer (this);
  connect (findTimer, SIGNAL (timeout ()), SLOT (slotFindTimer ()));


  misc->statusProg->setRange (0, misc->bfd_header.number_of_records);
  misc->statusProgLabel->setText (tr (" Loading features "));
  misc->statusProgPalette.setColor (QPalette::Normal, QPalette::Window, Qt::green);
  misc->statusProgLabel->setPalette (misc->statusProgPalette);
  misc->statusProg->setTextVisible (TRUE);
  qApp->processEvents();


  QString featureInfo;
  for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      misc->statusProg->setValue (i);
      qApp->processEvents();

      if (misc->feature[i].confidence_level || misc->abe_share->layer_type == AVERAGE_DEPTH || 
          misc->abe_share->layer_type == MIN_DEPTH || misc->abe_share->layer_type == MAX_DEPTH)
        {
          //  Only include those that are inside our PFM area.

          if (misc->feature[i].latitude >= misc->abe_share->open_args[0].head.mbr.min_y &&
              misc->feature[i].latitude <= misc->abe_share->open_args[0].head.mbr.max_y &&
              misc->feature[i].longitude >= misc->abe_share->open_args[0].head.mbr.min_x &&
              misc->feature[i].longitude <= misc->abe_share->open_args[0].head.mbr.max_x)
            {
              featureInfo.sprintf (tr ("%5d : ").toAscii (), i);
              featureInfo += QString (misc->feature[i].description) + "  :  " + QString (misc->feature[i].remarks);

              featureList += featureInfo;
            }
        }
    }


  misc->statusProg->reset ();
  misc->statusProg->setTextVisible (FALSE);
  qApp->processEvents();

  featureBox->addItems (featureList);

  show ();
}



findFeature::~findFeature ()
{
}



void
findFeature::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//!  Blink the feature in the coverage map using a 1/2 second timer

void 
findFeature::slotFindTimer ()
{
  static NV_BOOL flag = NVFalse;

  QColor c1;


  if (flag)
    {
      c1 = options->cov_feature_color;
    }
  else
    {
      c1 = options->cov_inv_feature_color;
    }

  flag = !flag;


  cov->fillRectangle (misc->feature[hot_id].longitude, misc->feature[hot_id].latitude, COV_FEATURE_SIZE, 
                      COV_FEATURE_SIZE, c1, NVTrue);
}



//!  Reset the feature to its original color

void 
findFeature::resetFeature (int id)
{
  QColor c1;

  if (misc->feature[id].confidence_level)
    {
      c1 = options->cov_feature_color;
    }
  else
    {
      c1 = options->cov_inv_feature_color;
    }

  cov->fillRectangle (misc->feature[id].longitude, misc->feature[id].latitude, COV_FEATURE_SIZE, COV_FEATURE_SIZE, c1, NVTrue);
}



void
findFeature::slotTextEdited (const QString &text)
{
  featureBox->clear ();

  QStringList result;
  result = featureList.filter (text, Qt::CaseInsensitive);

  featureBox->addItems (result);
}



void
findFeature::slotItemClicked (QListWidgetItem *item)
{
  QString text = item->text ();

  QString id = text.section (':', 0, 0);

  hot_id = id.toInt ();


  //  Set the previously highlighted feature back to it's original color.

  if (prev_hot_id != -1) resetFeature (prev_hot_id);
  prev_hot_id = hot_id;

  if (!findTimer->isActive ()) findTimer->start (500);
}



void
findFeature::slotClose ()
{
  findTimer->stop ();

  if (hot_id != -1) resetFeature (prev_hot_id);


  close ();
}
