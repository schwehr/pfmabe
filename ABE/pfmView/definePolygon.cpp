
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



#include "definePolygon.hpp"
#include "definePolygonHelp.hpp"

#include "version.hpp"


//!  This is the "Define Feature Polygon" dialog.

definePolygon::definePolygon (QWidget *pt, OPTIONS *op, MISC *mi):
  QDialog (pt, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  polyTimer = NULL;
  parent = pt;
  orig_chart_scale = mod_chart_scale = options->chart_scale;


  setWindowTitle (tr ("pfmView Define Feature Polygon"));

  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *csBox = new QGroupBox (tr ("Chart scale"), this);
  QHBoxLayout *csBoxLayout = new QHBoxLayout;
  csBox->setLayout (csBoxLayout);
  chartScale = new QSpinBox (csBox);
  chartScale->setRange (0, 3000000);
  chartScale->setSingleStep (5000);
  chartScale->setToolTip (tr ("Change the chart scale"));
  chartScale->setWhatsThis (defChartScaleText);
  chartScale->setValue (options->chart_scale);
  connect (chartScale, SIGNAL (valueChanged (int)), this, SLOT (slotChartScale (int)));
  csBoxLayout->addWidget (chartScale);
  vbox->addWidget (csBox);


  QGroupBox *gBox = new QGroupBox (tr ("Group features"), this);
  QHBoxLayout *gBoxLayout = new QHBoxLayout;
  gBox->setLayout (gBoxLayout);
  grpFeatures = new QCheckBox (this);
  grpFeatures->setChecked (options->group_features);
  grpFeatures->setToolTip (tr ("Automatically group features inside polygon"));
  grpFeatures->setWhatsThis (grpFeaturesText);
  connect (grpFeatures, SIGNAL (stateChanged (int)), this, SLOT (slotGrpFeatures (int)));
  gBoxLayout->addWidget (grpFeatures);
  vbox->addWidget (gBox);


  bPolygon = new QPushButton (tr ("Define polygon"), this);
  bPolygon->setToolTip (tr ("Define a polygonal area for a group of features."));
  connect (bPolygon, SIGNAL (clicked ()), this, SLOT (slotPolygon ()));
  bPolygon->setWhatsThis (defPolygonText);
  vbox->addWidget (bPolygon);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *applyButton = new QPushButton (tr ("Apply"), this);
  applyButton->setToolTip (tr ("Apply feature polygon definition"));
  applyButton->setWhatsThis (applyDefinePolygonText);
  connect (applyButton, SIGNAL (clicked ()), this, SLOT (slotApplyDefinePolygon ()));
  actions->addWidget (applyButton);

  QPushButton *cancelButton = new QPushButton (tr ("Cancel"), this);
  cancelButton->setToolTip (tr ("Discard feature polygon definition"));
  cancelButton->setWhatsThis (cancelDefinePolygonText);
  connect (cancelButton, SIGNAL (clicked ()), this, SLOT (slotCancelDefinePolygon ()));
  actions->addWidget (cancelButton);
}



definePolygon::~definePolygon ()
{
}



void
definePolygon::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
definePolygon::slotChartScale (int value)
{
  mod_chart_scale = value;

  misc->feature_poly_radius = (0.005 * mod_chart_scale) / 2.0;
}



void 
definePolygon::slotGrpFeatures (int value)
{
  if (value)
    {
      options->group_features = NVTrue;
    }
  else
    {
      options->group_features = NVFalse;
    }
}



void
definePolygon::slotApplyDefinePolygon ()
{
  options->chart_scale = mod_chart_scale;


  //  First make sure that we have no pre-existing parents or children inside the polygon.  Also, find the shoalest
  //  feature in the polygon.

  NV_FLOAT32 min_z = 99999999.9;
  NV_INT32 min_feature = 0;

  for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
    {
      if (inside_polygon2 (bfd_polygon.longitude, bfd_polygon.latitude, bfd_record.poly_count, misc->feature[i].longitude,
                           misc->feature[i].latitude))
        {
          if (options->group_features && misc->feature[i].parent_record)
            {
              QMessageBox::warning (this, tr ("pfmView"),
                                    tr ("A sub-feature already exists within the polygon.\nPlease un-group the features first."));
              return;
            }

          if (options->group_features && misc->feature[i].child_record)
            {
              QMessageBox::warning (this, tr ("pfmView"),
                                    tr ("A master feature already exists within the polygon.\nPlease un-group the features first."));
              return;
            }

          if (misc->feature[i].depth < min_z)
            {
              min_z = misc->feature[i].depth;
              min_feature = i;
            }
        }
    }


  NV_INT32 poly_count = bfd_record.poly_count;
  NV_INT32 poly_type = bfd_record.poly_type;


  //  Read the shoalest record.

  if (binaryFeatureData_read_record (misc->bfd_handle, min_feature, &bfd_record) < 0)
    {
      QString msg = QString (binaryFeatureData_strerror ());
      QMessageBox::warning (this, tr ("pfmView Define Feature Polygon"), tr ("Unable to read feature record\nReason: ") + msg);
      slotCancelDefinePolygon ();
    }


  //  Save the polygon count and type.

  bfd_record.poly_count = poly_count;
  bfd_record.poly_type = poly_type;


  //  Write the polygon.

  binaryFeatureData_write_record (misc->bfd_handle, min_feature, &bfd_record, &bfd_polygon, NULL);


  //  Group the sub-features if requested.

  if (options->group_features)
    {
      BFDATA_RECORD rec = bfd_record;
      NV_U_INT32 prev_rec = rec.record_number + 1;
      NV_BOOL first = NVTrue;

      for (NV_U_INT32 i = 0 ; i < misc->bfd_header.number_of_records ; i++)
        {
          if (inside_polygon2 (bfd_polygon.longitude, bfd_polygon.latitude, bfd_record.poly_count, misc->feature[i].longitude,
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
    }


  misc->def_feature_poly = NVFalse;
  misc->poly_count = 0;

  emit dataChangedSignal ();

  close ();
}



void
definePolygon::slotCancelDefinePolygon ()
{
  misc->def_feature_poly = NVFalse;
  misc->poly_count = 0;


  //  Force the redraw since we played with the target display no matter what.

  emit dataChangedSignal ();

  close ();
}



void 
definePolygon::slotPolygon ()
{
  if (!mod_chart_scale)
    {
      QMessageBox::warning (this, tr ("pfmView"),
                            tr ("You must define a chart scale (other than 0) before defining the polygon."));
      return;
    }


  if (mod_chart_scale != orig_chart_scale) emit chartScaleChangedSignal ();


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



/*!
  Timer to check to see if the user has finished drawing a polygon in pfmView (sets the
  variable misc->feature_polygon_flag to -1 or 1 for discarded or accepted).
*/

void 
definePolygon::slotPolyTimer ()
{
  //void smooth_contour (NV_INT32, NV_INT32 *, NV_FLOAT64 *, NV_FLOAT64 *);

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


          bfd_record.poly_type = 1;
          bfd_record.length = max_dist;
          bfd_record.horizontal_orientation = max_az;


          /*  Smooth the polygon (maybe in the future - it creates way too many points).

          NV_INT32 num_points = misc->poly_count;
          NV_INT32 bytes = (((5 * (num_points - 1)) + 1) * sizeof (NV_FLOAT64));

          NV_FLOAT64 *poly_x = (NV_FLOAT64 *) malloc (bytes);
          NV_FLOAT64 *poly_y = (NV_FLOAT64 *) malloc (bytes);
          if (poly_y == NULL)
            {
              perror (__FILE__);
              exit (-1);
            }

          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              poly_x[i] = misc->polygon_x[i];
              poly_y[i] = misc->polygon_y[i];
            }

          smooth_contour (5, &num_points, poly_x, poly_y);

          for (NV_INT32 i = 0 ; i < num_points ; i++)
            {
              misc->polygon_x[i] = poly_x[i];
              misc->polygon_y[i] = poly_y[i];
            }
          misc->poly_count = num_points;
          */


          bfd_record.poly_count = misc->poly_count;
          for (NV_INT32 i = 0 ; i < misc->poly_count ; i++)
            {
              bfd_polygon.longitude[i] = misc->polygon_x[i];
              bfd_polygon.latitude[i] = misc->polygon_y[i];
            }
        }


      emit dataChangedSignal ();


      //  Set the window size and location from what we saved when we hid the dialog.

      this->resize (window_width, window_height);
      this->move (window_x, window_y);

      show ();
    }
}
