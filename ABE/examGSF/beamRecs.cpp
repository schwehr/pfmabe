
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



#include "beamRecs.hpp"
#include "beamRecsHelp.hpp"


extern int gsfError;


beamRecs::beamRecs (QWidget * parent, NV_INT32 gsfHandle, NV_INT32 num_ping, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  gsf_handle = gsfHandle;
  ping = num_ping;
  data = NULL;
  options = op;
  misc = mi;


  //  Set the window size and location from the defaults

  this->resize (options->beam_width, options->beam_height);
  this->move (options->beam_window_x, options->beam_window_y);


  setSizeGripEnabled (TRUE);


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  populateTable ();


  vbox->addWidget (beamTable, 0, 0);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the beam records dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseBeamRecs ()));
  actions->addWidget (closeButton);
}



beamRecs::~beamRecs ()
{
}



void 
beamRecs::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = frameGeometry ();

  options->beam_window_x = tmp.x ();
  options->beam_window_y = tmp.y ();


  options->beam_width = tmp.width ();
  options->beam_height = tmp.height ();
}



void 
beamRecs::slotCloseBeamRecs ()
{
  close ();
}



void
beamRecs::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
beamRecs::populateTable ()
{
  NV_INT32 numrecs = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING);


  start = ping - 1;
  if (start < 1) start = 1;

  end = start + 2;
  if (end > numrecs) 
    {
      end = numrecs;
      start = end - 2;
    }


  QString tmp;

  tmp.sprintf (tr ("examGSF Beam Data - Pings %d, %d, %d").toAscii (), start, start + 1, end);
  setWindowTitle (tmp);


  NV_INT32 i, j, k, m, n;


  //  Read the record to get beams, sensor type, and available subrecords.

  memset (&gsf_record, 0, sizeof (gsfRecords));
  gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
  gsf_data_id.record_number = ping;

  gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0);


  //  Since all records are populated the same in a GSF file we only need to check one record for
  //  the available fields.

  for (m = 0 ; m < NUM_ARRAYS ; m++) data_flag[m] = NVTrue;

  if (gsf_record.mb_ping.depth == NULL) data_flag[0] = NVFalse;
  if (gsf_record.mb_ping.nominal_depth == NULL) data_flag[1] = NVFalse;
  if (gsf_record.mb_ping.across_track == NULL) data_flag[2] = NVFalse;
  if (gsf_record.mb_ping.along_track == NULL) data_flag[3] = NVFalse;
  if (gsf_record.mb_ping.travel_time == NULL) data_flag[4] = NVFalse;
  if (gsf_record.mb_ping.beam_angle == NULL) data_flag[5] = NVFalse;
  if (gsf_record.mb_ping.mc_amplitude == NULL) data_flag[6] = NVFalse;
  if (gsf_record.mb_ping.mr_amplitude == NULL) data_flag[7] = NVFalse;
  if (gsf_record.mb_ping.echo_width == NULL) data_flag[8] = NVFalse;
  if (gsf_record.mb_ping.quality_factor == NULL) data_flag[9] = NVFalse;
  if (gsf_record.mb_ping.receive_heave == NULL) data_flag[10] = NVFalse;
  if (gsf_record.mb_ping.depth_error == NULL) data_flag[11] = NVFalse;
  if (gsf_record.mb_ping.across_track_error == NULL) data_flag[12] = NVFalse;
  if (gsf_record.mb_ping.along_track_error == NULL) data_flag[13] = NVFalse;
  if (gsf_record.mb_ping.quality_flags == NULL) data_flag[14] = NVFalse;
  if (gsf_record.mb_ping.beam_flags == NULL) data_flag[15] = NVFalse;
  if (gsf_record.mb_ping.signal_to_noise == NULL) data_flag[16] = NVFalse;
  if (gsf_record.mb_ping.beam_angle_forward == NULL) data_flag[17] = NVFalse;
  if (gsf_record.mb_ping.vertical_error == NULL) data_flag[18] = NVFalse;
  if (gsf_record.mb_ping.horizontal_error == NULL) data_flag[19] = NVFalse;
  if (gsf_record.mb_ping.sector_number == NULL) data_flag[20] = NVFalse;
  if (gsf_record.mb_ping.detection_info == NULL) data_flag[21] = NVFalse;
  if (gsf_record.mb_ping.incident_beam_adj == NULL) data_flag[22] = NVFalse;
  if (gsf_record.mb_ping.system_cleaning == NULL) data_flag[23] = NVFalse;
  if (gsf_record.mb_ping.doppler_corr == NULL) data_flag[24] = NVFalse;


  rowLabel[0] = tr ("Depth ") + misc->depth_string[options->depth_units];
  rowLabel[1] = tr ("Nominal Depth ") + misc->depth_string[options->depth_units];
  rowLabel[2] = tr ("Across Track");
  rowLabel[3] = tr ("Along Track");
  rowLabel[4] = tr ("Travel Time");
  rowLabel[5] = tr ("Beam Angle");
  rowLabel[6] = tr ("MC Amplitude");
  rowLabel[7] = tr ("MR Amplitude");
  rowLabel[8] = tr ("Echo Width");
  rowLabel[9] = tr ("Quality Factor");
  rowLabel[10] = tr ("Receive Heave ") + misc->depth_string[options->depth_units];
  rowLabel[11] = tr ("Depth Error ") + misc->depth_string[options->depth_units];
  rowLabel[12] = tr ("Across Track Error");
  rowLabel[13] = tr ("Along Track Error");
  rowLabel[14] = tr ("Quality Flags");
  rowLabel[15] = tr ("Beam Flags");
  rowLabel[16] = tr ("Signal To Noise");
  rowLabel[17] = tr ("Beam Angle Forward");
  rowLabel[18] = tr ("Vertical Error ") + misc->depth_string[options->depth_units];
  rowLabel[19] = tr ("Horizontal Error ") + misc->depth_string[options->depth_units];
  rowLabel[20] = tr ("Transit sector ");
  rowLabel[21] = tr ("Detection info ");
  rowLabel[22] = tr ("Incident beam adj. ");
  rowLabel[23] = tr ("System cleaning ");
  rowLabel[24] = tr ("Doppler correction ");
  rowLabel[25] = tr ("     ");


  beams = gsf_record.mb_ping.number_beams;
  sensor_type = gsf_record.mb_ping.sensor_id;


  beamTable = new QTableWidget (62, beams, this);
  beamTable->setWhatsThis (beamTableText);
  beamTable->setAlternatingRowColors (TRUE);
  vheader = beamTable->verticalHeader ();
  vheader->setToolTip (tr ("Click the header to display a graphic profile of the data"));
  connect (vheader, SIGNAL (sectionClicked (int)), this, SLOT (slotProfile (int)));


  //  Read all three records and populate the table.

  for (i = start ; i <= end ; i++)
    {
      j = i - start;
      k = j * NUM_ARRAYS + j;


      //  Read each record.

      memset (&gsf_record, 0, sizeof (gsfRecords));
      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = i;

      gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0);


      //  Set all possible rows.  We'll hide the non-available ones later.

      for (NV_INT32 rownum = 0 ; rownum < NUM_ROWS ; rownum++)
        beamTable->setVerticalHeaderItem (k + rownum, new QTableWidgetItem (rowLabel[rownum]));


      //  Highlight the requested ping.

      if (j == 1) beamTable->selectRow (k);


      //  Insert the data or hide the row.

      for (m = 0 ; m < NUM_ARRAYS ; m++) 
        {
          if (!data_flag[m]) 
            {
              beamTable->hideRow (k + m);
            }
          else
            {
              for (n = 0 ; n < beams ; n++)
                {
                  switch (m)
                    {
                    case 0:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.depth[n] * misc->depth_factor[options->depth_units]);
                      break;

                    case 1:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.nominal_depth[n] *
                                   misc->depth_factor[options->depth_units]);
                      break;

                    case 2:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.across_track[n]);
                      break;

                    case 3:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.along_track[n]);
                      break;

                    case 4:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.travel_time[n]);
                      break;

                    case 5:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.beam_angle[n]);
                      break;

                    case 6:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.mc_amplitude[n]);
                      break;

                    case 7:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.mr_amplitude[n]);
                      break;

                    case 8:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.echo_width[n]);
                      break;

                    case 9:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.quality_factor[n]);
                      break;

                    case 10:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.receive_heave[n] *
                                   misc->depth_factor[options->depth_units]);
                      break;

                    case 11:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.depth_error[n] * misc->depth_factor[options->depth_units]);
                      break;

                    case 12:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.across_track_error[n]);
                      break;

                    case 13:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.along_track_error[n]);
                      break;

                    case 14:
                      tmp.sprintf ("%02x", gsf_record.mb_ping.quality_flags[n]);
                      break;

                    case 15:
                      tmp.sprintf ("%02x", gsf_record.mb_ping.beam_flags[n]);
                      break;

                    case 16:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.signal_to_noise[n]);
                      break;

                    case 17:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.beam_angle_forward[n]);
                      break;

                    case 18:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.vertical_error[n] *
                                   misc->depth_factor[options->depth_units]);
                      break;

                    case 19:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.horizontal_error[n]);
                      break;

                    case 20:
                      tmp.sprintf ("%03d", gsf_record.mb_ping.sector_number[n]);
                      break;

                    case 21:
                      tmp.sprintf ("%03d", gsf_record.mb_ping.detection_info[n]);
                      break;

                    case 22:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.incident_beam_adj[n]);
                      break;

                    case 23:
                      tmp.sprintf ("%03d", gsf_record.mb_ping.system_cleaning[n]);
                      break;

                    case 24:
                      tmp.sprintf ("%0.3f", gsf_record.mb_ping.doppler_corr[n]);
                      break;
                    }
                  beamTable->setItem (k + m, n, new QTableWidgetItem (tmp));
                }
            }
        }
    }


  for (i = 0 ; i < beams ; i++)
    {
      tmp.sprintf (tr ("Beam\n%03d").toAscii (), i + 1);
      beamTable->setHorizontalHeaderItem (i, new QTableWidgetItem (tmp));
    }

  QScrollBar *sb = beamTable->verticalScrollBar ();
  sb->setValue (6);
}



void
beamRecs::slotProfile (NV_INT32 logicalIndex)
{
  profileD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  profileD->setWhatsThis (tr ("This is an across track profile of the data type selected by clicking on the data header on the left side of the display.  ") +
                          tr ("As you move the mouse over the display the two boxes in thge lower left will display the approximate beam number and the ") +
                          tr ("data value.  Move the mouse over the rightmost box to find out the units for the data value."));


  //  Set the window size and location from the defaults

  profileD->resize (options->profile_width, options->profile_height);
  profileD->move (options->profile_window_x, options->profile_window_y);
  connect (profileD, SIGNAL (finished (int)), this, SLOT (slotProfileFinished (int)));

  profileD->setSizeGripEnabled (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (profileD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  NV_INT32 rec = start + logicalIndex / NUM_ROWS;
  QString tmp;
  tmp.sprintf ("examGSF - Record %d - ", rec);

  tmp += rowLabel[logicalIndex % NUM_ROWS];
  profileD->setWindowTitle (tmp);


  memset (&gsf_record, 0, sizeof (gsfRecords));
  gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
  gsf_data_id.record_number = rec;

  gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0);


  NV_FLOAT64 *dataPtr;
  NV_INT32 ndx = (logicalIndex - logicalIndex / NUM_ROWS) % NUM_ARRAYS;

  switch (ndx)
    {
    case 0:
      dataPtr = gsf_record.mb_ping.depth;
      break;

    case 1:
      dataPtr = gsf_record.mb_ping.nominal_depth;
      break;

    case 2:
      dataPtr = gsf_record.mb_ping.across_track;
      break;

    case 3:
      dataPtr = gsf_record.mb_ping.along_track;
      break;

    case 4:
      dataPtr = gsf_record.mb_ping.travel_time;
      break;

    case 5:
      dataPtr = gsf_record.mb_ping.beam_angle;
      break;

    case 6:
      dataPtr = gsf_record.mb_ping.mc_amplitude;
      break;

    case 7:
      dataPtr = gsf_record.mb_ping.mr_amplitude;
      break;

    case 8:
      dataPtr = gsf_record.mb_ping.echo_width;
      break;

    case 9:
      dataPtr = gsf_record.mb_ping.quality_factor;
      break;

    case 10:
      dataPtr = gsf_record.mb_ping.receive_heave;
      break;

    case 11:
      dataPtr = gsf_record.mb_ping.depth_error;
      break;

    case 12:
      dataPtr = gsf_record.mb_ping.across_track_error;
      break;

    case 13:
      dataPtr = gsf_record.mb_ping.along_track_error;
      break;

    case 16:
      dataPtr = gsf_record.mb_ping.signal_to_noise;
      break;

    case 17:
      dataPtr = gsf_record.mb_ping.beam_angle_forward;
      break;

    case 18:
      dataPtr = gsf_record.mb_ping.vertical_error;
      break;

    case 19:
      dataPtr = gsf_record.mb_ping.horizontal_error;
      break;

    default:
      return;
    }

  data = (NV_F64_COORD2 *) realloc (data, beams * sizeof (NV_F64_COORD2));
  data_bounds.min_x = 999999999.0;
  data_bounds.max_x = -999999999.0;
  data_bounds.min_y = 999999999.0;
  data_bounds.max_y = -999999999.0;

  for (NV_INT32 i = 0 ; i < beams ; i++)
    {
      data[i].x = (NV_FLOAT64) i;

      if (check_flag (gsf_record.mb_ping.beam_flags[i], HMPS_IGNORE_NULL_BEAM) ||
          check_flag (gsf_record.mb_ping.beam_flags[i], HMPS_IGNORE_MANUALLY_EDITED) ||
          check_flag (gsf_record.mb_ping.beam_flags[i], HMPS_IGNORE_FILTER_EDITED))
        {
          data[i].y = PROFILE_NULL;
        }
      else
        {
          //  Adjust for depth units

          if (ndx == 0 || ndx == 1 || ndx == 10 || ndx == 11 || ndx == 18)
            {
              data[i].y = dataPtr[i] * misc->depth_factor[options->depth_units];
            }
          else
            {
              data[i].y = dataPtr[i];
            }

          data_bounds.min_x = qMin (data_bounds.min_x, data[i].x);
          data_bounds.max_x = qMax (data_bounds.max_x, data[i].x);

          data_bounds.min_y = qMin (data_bounds.min_y, data[i].y);
          data_bounds.max_y = qMax (data_bounds.max_y, data[i].y);
        }
    }


  //  Add 2% to ranges.

  data_bounds.range_x = data_bounds.max_x - data_bounds.min_x;
  data_bounds.range_y = data_bounds.max_y - data_bounds.min_y;

  data_bounds.min_x -= data_bounds.range_x * 0.02;
  data_bounds.max_x += data_bounds.range_x * 0.02;
  data_bounds.min_y -= data_bounds.range_y * 0.02;
  data_bounds.max_y += data_bounds.range_y * 0.02;

  data_bounds.range_x = data_bounds.max_x - data_bounds.min_x;
  data_bounds.range_y = data_bounds.max_y - data_bounds.min_y;


  //  Set the map values from the defaults

  mapdef.projection = NO_PROJECTION;
  mapdef.draw_width = options->profile_width;
  mapdef.draw_height = options->profile_height;
  mapdef.overlap_percent = 5;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;

  mapdef.coasts = NVFalse;
  mapdef.landmask = NVFalse;

  mapdef.border = 0;
  mapdef.coast_color = Qt::white;
  mapdef.grid_color = Qt::white;
  mapdef.background_color = Qt::white;


  mapdef.initial_bounds.min_x = data_bounds.min_x;
  mapdef.initial_bounds.min_y = data_bounds.min_y;
  mapdef.initial_bounds.max_x = data_bounds.max_x;
  mapdef.initial_bounds.max_y = data_bounds.max_y;


  //  Make the map.

  data_profile = new nvMap (this, &mapdef);


  //  Connect to the signals from the map class.

  connect (data_profile, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotDataProfileMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (data_profile, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (data_profile, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPlotProfile (NVMAP_DEF)));


  vbox->addWidget (data_profile, 0, 0);


  dataBar = new QStatusBar (profileD);
  dataBar->setSizeGripEnabled (FALSE);
  dataBar->show ();
  vbox->setStretchFactor (data_profile, 1);
  vbox->addWidget (dataBar);

  beamLabel = new QLabel (" 000 ", this);
  beamLabel->setToolTip (tr ("Beam"));

  dataLabel = new QLabel (" 00000.0 ", this);
  dataLabel->setToolTip (rowLabel[logicalIndex % NUM_ROWS]);

  dataBar->addWidget (beamLabel);
  dataBar->addWidget (dataLabel);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the profile dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseProfile ()));
  actions->addWidget (closeButton);


  profileD->show ();


  data_profile->enableSignals ();

  data_profile->redrawMapArea (NVTrue);

  data_profile->setCursor (Qt::ArrowCursor);
}



void 
beamRecs::slotDataProfileMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 x, NV_FLOAT64 y)
{
  QString tmp;

  tmp.sprintf ("%03d", NINT (x));
  beamLabel->setText (tmp);

  tmp.sprintf ("%f", y);
  dataLabel->setText (tmp);
}



void 
beamRecs::slotPlotProfile (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  //  Draw the data profile.

  NV_INT32 start = -1;
  for (NV_INT32 i = 0 ; i < beams ; i++)
    {
      if (data[i].y != PROFILE_NULL)
        {
          start = i;
          break;
        }
    }
  if (start == -1) return;


  NV_INT32 j = start;
  NV_INT32 i = start + 1;
  while (i < beams)
    {
      if (data[i].y != PROFILE_NULL)
        {
          data_profile->drawLine (data[j].x, data[j].y, data[i].x, data[i].y, 2, Qt::black, NVTrue, Qt::SolidLine);
          j = i;
        }
      i++;
    }
}



void 
beamRecs::slotProfileFinished (int result __attribute__ ((unused)))
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = profileD->frameGeometry ();

  options->profile_window_x = tmp.x ();
  options->profile_window_y = tmp.y ();


  options->profile_width = tmp.width ();
  options->profile_height = tmp.height ();
}



void 
beamRecs::slotCloseProfile ()
{
  profileD->close ();
}



void 
beamRecs::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  data_profile->redrawMapArea (NVTrue);
}
