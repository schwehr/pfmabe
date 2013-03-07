
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



#include "otherRecs.hpp"
#include "otherRecsHelp.hpp"


extern int gsfError;


otherRecs::otherRecs (QWidget * parent, NV_INT32 gsfHandle, OPTIONS *op, MISC *mi):
  QDialog (parent, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  options = op;
  misc = mi;
  extendD = NULL;
  svProfileD = NULL;


  //  Set the window size and location from the defaults

  this->resize (options->other_width, options->other_height);
  this->move (options->other_window_x, options->other_window_y);


  setSizeGripEnabled (TRUE);


  gsf_handle = gsfHandle;

  numrecs = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING);

  svData = NULL;


  id_name[GSF_RECORD_HISTORY] = tr ("History");
  id_name[GSF_RECORD_SWATH_BATHY_SUMMARY] = tr ("Bathy Summary");
  id_name[GSF_RECORD_COMMENT] = tr ("Comment");
  id_name[GSF_RECORD_PROCESSING_PARAMETERS] = tr ("Processing Parameters");
  id_name[GSF_RECORD_SENSOR_PARAMETERS] = tr ("Sensor Parameters");
  id_name[GSF_RECORD_NAVIGATION_ERROR] = tr ("Navigation Error");
  id_name[GSF_RECORD_SOUND_VELOCITY_PROFILE] = tr ("Sound Velocity Profile");
  id_name[GSF_RECORD_HV_NAVIGATION_ERROR] = tr ("H/V Navigation Error");


  time_t sec;
  long nsec;
  gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, 1, &sec, &nsec);
  start_nsec = (NV_U_INT64) sec * 1000000000 + (NV_U_INT64) nsec;

  gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, numrecs, &sec, &nsec);
  end_nsec = (NV_U_INT64) sec * 1000000000 + (NV_U_INT64) nsec;


  //  Get the number of other records in the file.

  numhist = gsfGetNumberRecords (gsf_handle, GSF_RECORD_HISTORY);
  numsumm = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SWATH_BATHY_SUMMARY);
  numcomm = gsfGetNumberRecords (gsf_handle, GSF_RECORD_COMMENT);
  numproc = gsfGetNumberRecords (gsf_handle, GSF_RECORD_PROCESSING_PARAMETERS);
  numsens = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SENSOR_PARAMETERS);
  numnavi = gsfGetNumberRecords (gsf_handle, GSF_RECORD_NAVIGATION_ERROR);
  numsvpr = gsfGetNumberRecords (gsf_handle, GSF_RECORD_SOUND_VELOCITY_PROFILE);
  numhvnv = gsfGetNumberRecords (gsf_handle, GSF_RECORD_HV_NAVIGATION_ERROR);

  if (numhist == -1) numhist = 0;
  if (numsumm == -1) numsumm = 0;
  if (numcomm == -1) numcomm = 0;
  if (numproc == -1) numproc = 0;
  if (numsens == -1) numsens = 0;
  if (numnavi == -1) numnavi = 0;
  if (numsvpr == -1) numsvpr = 0;
  if (numhvnv == -1) numhvnv = 0;
  numothr = numhist + numsumm + numcomm + numproc + numsens + numnavi + numsvpr + numhvnv;
  digits = (NV_INT32) (log10 ((NV_FLOAT64) numothr)) + 1;


  setWindowTitle (tr ("examGSF Other GSF Records"));


  QVBoxLayout *vbox = new QVBoxLayout (this);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  otherTable = new QTableWidget (numothr, 2, this);
  otherTable->setWhatsThis (otherTableText);
  vheader = otherTable->verticalHeader ();
  vheader->setToolTip (tr ("Click on the record number to display the record information"));
  connect (vheader, SIGNAL (sectionClicked (int)), this, SLOT (slotExtend (int)));


  otherTable->setHorizontalHeaderItem (0, new QTableWidgetItem (tr ("Record\nType")));
  otherTable->setHorizontalHeaderItem (1, new QTableWidgetItem (tr ("Date (Julian Day) Time")));


  vbox->addWidget (otherTable, 0, 0);


  rid = (NV_INT32 *) calloc (numothr, sizeof (NV_INT32));
  rec = (NV_INT32 *) calloc (numothr, sizeof (NV_INT32));
  rec_time = (struct timespec *) calloc (numothr, sizeof (struct timespec));


  NV_INT32 i, cnt = 0;

  for (i = 1 ; i <= numsvpr ; i++)
    {
      if (readRecord (GSF_RECORD_SOUND_VELOCITY_PROFILE, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.svp.application_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numproc ; i++)
    {
      if (readRecord (GSF_RECORD_PROCESSING_PARAMETERS, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.process_parameters.param_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numsens ; i++)
    {
      if (readRecord (GSF_RECORD_SENSOR_PARAMETERS, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.sensor_parameters.param_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numnavi ; i++)
    {
      if (readRecord (GSF_RECORD_NAVIGATION_ERROR, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.nav_error.nav_error_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numhvnv ; i++)
    {
      if (readRecord (GSF_RECORD_HV_NAVIGATION_ERROR, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.hv_nav_error.nav_error_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numsumm ; i++)
    {
      if (readRecord (GSF_RECORD_SWATH_BATHY_SUMMARY, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.summary.start_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numcomm ; i++)
    {
      if (readRecord (GSF_RECORD_COMMENT, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.comment.comment_time;

          populateTable (cnt++);
        }
    }

  for (i = 1 ; i <= numhist ; i++)
    {
      if (readRecord (GSF_RECORD_HISTORY, i, &gsf_data_id, &gsf_record))
        {
          rec[cnt] = i;
          rid[cnt] = gsf_data_id.recordID;
          rec_time[cnt] = gsf_record.history.history_time;

          populateTable (cnt++);
        }
    }

  otherTable->resizeColumnsToContents ();
  otherTable->resizeRowsToContents ();

  NV_INT32 width = 0;
  for (NV_INT32 i = 0 ; i < 4 ; i++) width += otherTable->columnWidth (i);
  width = qMin (800, width + 60);

  NV_INT32 height = otherTable->rowHeight (0) * numothr;
  height = qMin (800, height + 60);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the ancillary records dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseOtherRecs ()));
  actions->addWidget (closeButton);


  resize (width, height);
}


otherRecs::~otherRecs ()
{
  free (rid);
  free (rec);
  free (rec_time);
}



void 
otherRecs::closeEvent (QCloseEvent * e __attribute__ ((unused)))
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = frameGeometry ();

  options->other_window_x = tmp.x ();
  options->other_window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = geometry ();
  options->other_width = tmp.width ();
  options->other_height = tmp.height ();

  emit closedSignal ();
}



void 
otherRecs::slotCloseOtherRecs ()
{
  close ();
}



void
otherRecs::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



NV_BOOL 
otherRecs::readRecord (NV_INT32 dataID, NV_INT32 rec_num, gsfDataID *gsf_data_id, gsfRecords *gsf_record)
{
  memset (gsf_record, 0, sizeof (gsfRecords));

  gsf_data_id->recordID = dataID;
  gsf_data_id->record_number = rec_num;

  if (gsfRead (gsf_handle, dataID, gsf_data_id, gsf_record, NULL, 0) < 0) 
    {
      if (gsfError == GSF_READ_TO_END_OF_FILE) return (NVFalse);


      QString errmsg;

      errmsg.sprintf (tr ("Error reading record %d : %s").toAscii (), rec_num, gsfStringError ());

      QMessageBox::warning (this, tr ("Read GSF File"), errmsg);

      return (NVFalse);
    }
  return (NVTrue);
}



void 
otherRecs::populateTable (NV_INT32 row)
{
  QString timeString, tmp;
  NV_INT32 year, day, hour, minute, month, mday;
  NV_FLOAT32 second;
  NV_CHAR format[5];



  cvtime (rec_time[row].tv_sec, rec_time[row].tv_nsec, &year, &day, &hour, &minute, &second);
  jday2mday (year, day, &month, &mday);
  month++;

  timeString.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f   ", year + 1900, month, mday, day, hour, 
                      minute, second);


  sprintf (format, "%%0%dd", digits);
  tmp.sprintf (format, rec[row]);
  otherTable->setVerticalHeaderItem (row, new QTableWidgetItem (tmp));


  otherTable->setItem (row, 0, new QTableWidgetItem (id_name[rid[row]]));
  otherTable->setItem (row, 1, new QTableWidgetItem (timeString));
}



void
otherRecs::slotExtend (int id)
{
  if (extendD) extendD->close ();

  extendD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  extendD->setWhatsThis (tr ("This dialog contains ancillary data for the record type selected.  Data should be displayed as <i>NAME=VALUE</i> pairs.  ") +
                         tr ("If the selected record type was <b>Sound Velocity Profile</b> a graphical profile of the sound velocity against the depth ") +
                         tr ("will also be displayed"));


  //  Set the window size and location from the defaults

  extendD->resize (options->extended_width, options->extended_height);
  extendD->move (options->extended_window_x, options->extended_window_y);
  connect (extendD, SIGNAL (finished (int)), this, SLOT (slotExtendFinished (int)));

  extendD->setSizeGripEnabled (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (extendD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QTextEdit *tEdit = new QTextEdit (extendD);
  tEdit->setReadOnly (TRUE);


  NV_CHAR hem;
  NV_FLOAT64 deg, min, sec;
  QString tmp, recordID, timeString, latString = " ", lonString = " ", dataString = " ";
  NV_INT32 year, day, hour, minute, month, mday, i;
  NV_FLOAT32 second;


  gsf_data_id.recordID = rid[id];
  gsf_data_id.record_number = rec[id];

  gsfRead (gsf_handle, rid[id], &gsf_data_id, &gsf_record, NULL, 0); 


  tmp.sprintf (tr (" Record %d").toAscii (), rec[id]);
  tmp.prepend (tr ("examGSF - ") + id_name[rid[id]]);
  extendD->setWindowTitle (tmp);


  switch (rid[id])
    {
    case GSF_RECORD_SOUND_VELOCITY_PROFILE:
      latString.sprintf ("%s", fixpos (gsf_record.svp.latitude, &deg, &min, &sec, &hem, POS_LAT, options->position_form));
      lonString.sprintf ("%s", fixpos (gsf_record.svp.longitude, &deg, &min, &sec, &hem, POS_LON, options->position_form));
      tmp = tr ("<i>Position:</i> ") + latString + " " + lonString;

      tEdit->append (tmp);


      cvtime (gsf_record.svp.observation_time.tv_sec, gsf_record.svp.observation_time.tv_nsec, &year, &day, &hour, 
              &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Observation Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, 
                   mday, day, hour, minute, second);

      tEdit->append (tmp);


      cvtime (gsf_record.svp.application_time.tv_sec, gsf_record.svp.application_time.tv_nsec, &year, &day, &hour, 
              &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Application Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, 
                   mday, day, hour, minute, second);

      tEdit->append (tmp);


      tmp.sprintf (tr ("<i>Number of points:</i> %d").toAscii (), gsf_record.svp.number_points);

      tEdit->append (tmp);

      sv_length = gsf_record.svp.number_points;

      svData = (NV_F64_COORD2 *) realloc (svData, sv_length * sizeof (NV_F64_COORD2));
      sv_bounds.min_x = 999999999.0;
      sv_bounds.max_x = -999999999.0;
      sv_bounds.min_y = 999999999.0;
      sv_bounds.max_y = -999999999.0;

      for (i = 0 ; i < sv_length ; i++)
        {
          svData[i].x = (NV_FLOAT64) gsf_record.svp.sound_speed[i];
          svData[i].y = (NV_FLOAT64) gsf_record.svp.depth[i] * misc->depth_factor[options->depth_units];

          sv_bounds.min_x = qMin (sv_bounds.min_x, svData[i].x);
          sv_bounds.max_x = qMax (sv_bounds.max_x, svData[i].x);

          sv_bounds.min_y = qMin (sv_bounds.min_y, svData[i].y);
          sv_bounds.max_y = qMax (sv_bounds.max_y, svData[i].y);

          tmp.sprintf (tr ("<i>Point:</i> %d   <i>Depth :</i>   %.2f   <i>Sound Speed:</i> %.2f").toAscii (), i, 
                       gsf_record.svp.depth[i] * misc->depth_factor[options->depth_units], gsf_record.svp.sound_speed[i]);
          tmp.insert (28, misc->depth_string[options->depth_units]);

          tEdit->append (tmp);
        }


      //  Add 2% to ranges.

      sv_bounds.range_x = sv_bounds.max_x - sv_bounds.min_x;
      sv_bounds.range_y = sv_bounds.max_y - sv_bounds.min_y;

      sv_bounds.min_x -= sv_bounds.range_x * 0.02;
      sv_bounds.max_x += sv_bounds.range_x * 0.02;
      sv_bounds.min_y -= sv_bounds.range_y * 0.02;
      sv_bounds.max_y += sv_bounds.range_y * 0.02;

      sv_bounds.range_x = sv_bounds.max_x - sv_bounds.min_x;
      sv_bounds.range_y = sv_bounds.max_y - sv_bounds.min_y;


      plotSvProfile (rec[id]);

      break;


    case GSF_RECORD_PROCESSING_PARAMETERS:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);


      tmp.sprintf (tr ("<i>Number of parameters:</i> %d").toAscii (), gsf_record.process_parameters.number_parameters);

      tEdit->append (tmp);


      for (i = 0 ; i < gsf_record.process_parameters.number_parameters ; i++)
        {
          tmp.sprintf ("%s", gsf_record.process_parameters.param[i]);

          tEdit->append (tmp);
        }

      break;


    case GSF_RECORD_SENSOR_PARAMETERS:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);


      tmp.sprintf (tr ("<i>Number of parameters:</i> %d").toAscii (), gsf_record.sensor_parameters.number_parameters);

      tEdit->append (tmp);


      for (i = 0 ; i < gsf_record.sensor_parameters.number_parameters ; i++)
        {
          tmp.sprintf ("%s", gsf_record.sensor_parameters.param[i]);

          tEdit->append (tmp);
        }

      break;


    case GSF_RECORD_COMMENT:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);

      tmp.sprintf ("%s", gsf_record.comment.comment);

      tEdit->append (tmp);

      break;


    case GSF_RECORD_HISTORY:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);

      tmp.sprintf (tr ("<i>Host Name:</i>  %s").toAscii (), gsf_record.history.host_name);

      tEdit->append (tmp);

      tmp.sprintf (tr ("<i>Operator Name:</i>  %s").toAscii (), gsf_record.history.operator_name);

      tEdit->append (tmp);

      tmp.sprintf (tr ("<i>Command:</i> %s").toAscii (), gsf_record.history.command_line);

      tEdit->append (tmp);

      tmp.sprintf (tr ("<i>Comment:</i> %s").toAscii (), gsf_record.history.comment);

      tEdit->append (tmp);

      break;


    case GSF_RECORD_NAVIGATION_ERROR:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);

      tmp.sprintf (tr ("Latitude Error: %09.3f").toAscii (), gsf_record.nav_error.latitude_error);

      tEdit->append (tmp);

      tmp.sprintf (tr ("Longitude Error: %09.3f").toAscii (), gsf_record.nav_error.longitude_error);

      break;


    case GSF_RECORD_SWATH_BATHY_SUMMARY:
      cvtime (gsf_record.summary.start_time.tv_sec, gsf_record.summary.start_time.tv_nsec, &year, &day, &hour, 
              &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Start Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, 
                   mday, day, hour, minute, second);

      tEdit->append (tmp);


      cvtime (gsf_record.summary.end_time.tv_sec, gsf_record.summary.end_time.tv_nsec, &year, &day, &hour, 
              &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>End Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, 
                   mday, day, hour, minute, second);

      tEdit->append (tmp);


      latString.sprintf (tr ("<i>Minimum Latitude:</i> %s").toAscii (), 
                         fixpos (gsf_record.summary.min_latitude, &deg, &min, &sec, &hem, POS_LAT, options->position_form));

      tEdit->append (latString);


      lonString.sprintf (tr ("<i>Minimum Longitude:</i> %s").toAscii (),
                         fixpos (gsf_record.summary.min_longitude, &deg, &min, &sec, &hem, POS_LON, options->position_form));

      tEdit->append (lonString);


      latString.sprintf (tr ("<i>Maximum Latitude:</i> %s").toAscii (),
                         fixpos (gsf_record.summary.max_latitude, &deg, &min, &sec, &hem, POS_LAT, options->position_form));

      tEdit->append (latString);


      lonString.sprintf (tr ("<i>Maximum Longitude:</i> %s").toAscii (),
                         fixpos (gsf_record.summary.max_longitude, &deg, &min, &sec, &hem, POS_LON, options->position_form));

      tEdit->append (lonString);


      tmp.sprintf (tr ("<i>Minimum Depth :</i> %0.2f").toAscii (), gsf_record.summary.min_depth * 
                   misc->depth_factor[options->depth_units]);
      tmp.insert (17, misc->depth_string[options->depth_units]);
      tEdit->append (tmp);



      tmp.sprintf (tr ("<i>Maximum Depth :</i> %0.2f").toAscii (), gsf_record.summary.max_depth * 
                   misc->depth_factor[options->depth_units]);
      tmp.insert (17, misc->depth_string[options->depth_units]);
      tEdit->append (tmp);

      break;


    case GSF_RECORD_HV_NAVIGATION_ERROR:
      cvtime (rec_time[id].tv_sec, rec_time[id].tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;

      tmp.sprintf (tr ("<i>Time:</i> %d-%02d-%02d (%03d) %02d:%02d:%05.2f").toAscii (), year + 1900, month, mday, day,
                   hour, minute, second);

      tEdit->append (tmp);


      if (gsf_record.hv_nav_error.horizontal_error == GSF_NULL_HORIZONTAL_ERROR)
        {
          tmp.sprintf (tr ("<i>Horizontal Error: Unknown</i>").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("<i>Horizontal Error:</i> %09.3f").toAscii (), gsf_record.hv_nav_error.horizontal_error);
        }

      tEdit->append (tmp);


      if (gsf_record.hv_nav_error.vertical_error == GSF_NULL_HORIZONTAL_ERROR)
        {
          tmp.sprintf (tr ("<i>Vertical Error: Unknown</i>").toAscii ());
        }
      else
        {
          tmp.sprintf (tr ("<i>Vertical Error :</i> %09.3f").toAscii (), gsf_record.hv_nav_error.vertical_error * 
                       misc->depth_factor[options->depth_units]);
          tmp.insert (18, misc->depth_string[options->depth_units]);
        }

      tEdit->append (tmp);


      tmp.sprintf (tr ("<i>Position Type:</i> %s").toAscii (), gsf_record.hv_nav_error.position_type);

      tEdit->append (tmp);

      break;
    }


  vbox->addWidget (tEdit, 0, 0);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), this);
  closeButton->setToolTip (tr ("Close the extended data dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotCloseExtend ()));
  actions->addWidget (closeButton);


  extendD->show ();


  QScrollBar *vbar = tEdit->verticalScrollBar ();

  vbar->setSliderPosition (vbar->minimum ());


  //if (rid[id] == GSF_RECORD_SOUND_VELOCITY_PROFILE) svProfileD->raise ();
}



void 
otherRecs::slotExtendFinished (int result __attribute__ ((unused)))
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = extendD->frameGeometry ();

  options->extended_window_x = tmp.x ();
  options->extended_window_y = tmp.y ();


  options->extended_width = tmp.width ();
  options->extended_height = tmp.height ();


  if (gsf_data_id.recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE) slotCloseProfile ();
}



void 
otherRecs::slotCloseExtend ()
{
  extendD->close ();
}



void
otherRecs::plotSvProfile (NV_INT32 recnum)
{
  if (svProfileD) svProfileD->close ();

  svProfileD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  svProfileD->setWhatsThis (tr ("This is a profile of the sound velocity against the depth.  ") +
                            tr ("As you move the mouse over the display the two boxes in thge lower left will display the approximate depth in meters ") +
                            tr ("and the sound velocity in meters/second."));


  //  Set the window size and location from the defaults

  svProfileD->resize (options->svp_width, options->svp_height);
  svProfileD->move (options->svp_window_x, options->svp_window_y);
  connect (svProfileD, SIGNAL (finished (int)), this, SLOT (slotSvProfileFinished (int)));

  svProfileD->setSizeGripEnabled (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (svProfileD);
  vbox->setMargin (5);
  vbox->setSpacing (5);


  QString tmp;
  tmp.sprintf ("examGSF Sound Velocity Profile - %d", recnum);

  svProfileD->setWindowTitle (tmp);


  //  Set the map values from the defaults

  mapdef.projection = NO_PROJECTION;
  mapdef.draw_width = options->svp_width;
  mapdef.draw_height = options->svp_height;
  mapdef.overlap_percent = 5;
  mapdef.grid_inc_x = 0.0;
  mapdef.grid_inc_y = 0.0;

  mapdef.coasts = NVFalse;
  mapdef.landmask = NVFalse;

  mapdef.border = 0;
  mapdef.coast_color = Qt::white;
  mapdef.grid_color = Qt::white;
  mapdef.background_color = Qt::white;


  mapdef.initial_bounds.min_x = sv_bounds.min_x;
  mapdef.initial_bounds.min_y = sv_bounds.max_y;
  mapdef.initial_bounds.max_x = sv_bounds.max_x;
  mapdef.initial_bounds.max_y = sv_bounds.min_y;


  //  Make the map.

  sv_profile = new nvMap (this, &mapdef);


  //  Connect to the signals from the map class.

  connect (sv_profile, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, 
           SLOT (slotSvProfileMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
  connect (sv_profile, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (sv_profile, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPlotProfile (NVMAP_DEF)));


  vbox->addWidget (sv_profile, 0, 0);


  svBar = new QStatusBar (svProfileD);
  svBar->setSizeGripEnabled (FALSE);
  svBar->show ();
  vbox->setStretchFactor (sv_profile, 1);
  vbox->addWidget (svBar);

  depLabel = new QLabel (" 000 ", this);
  depLabel->setToolTip (tr ("Depth (meters)"));

  svLabel = new QLabel (" 00000.0 ", this);
  svLabel->setToolTip (tr ("Sound Velocity (meters/second)"));

  svBar->addWidget (svLabel);
  svBar->addWidget (depLabel);


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


  svProfileD->show ();


  sv_profile->enableSignals ();

  sv_profile->redrawMapArea (NVTrue);

  sv_profile->setCursor (Qt::ArrowCursor);
}



void 
otherRecs::slotSvProfileMouseMove (QMouseEvent *e __attribute__ ((unused)), NV_FLOAT64 x, NV_FLOAT64 y)
{
  QString tmp;

  tmp.sprintf ("%0.1f", y);
  depLabel->setText (tmp);

  tmp.sprintf ("%0.1f", x);
  svLabel->setText (tmp);
}



void 
otherRecs::slotPlotProfile (NVMAP_DEF l_mapdef __attribute__ ((unused)))
{
  //  Draw the data profile.

  for (NV_INT32 i = 1 ; i < sv_length ; i++)
    sv_profile->drawLine (svData[i].x, svData[i].y, svData[i - 1].x, svData[i - 1].y, 2, Qt::black, NVTrue, Qt::SolidLine);
}



void 
otherRecs::slotSvProfileFinished (int result __attribute__ ((unused)))
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = svProfileD->frameGeometry ();

  options->svp_window_x = tmp.x ();
  options->svp_window_y = tmp.y ();


  options->svp_width = tmp.width ();
  options->svp_height = tmp.height ();
}



void 
otherRecs::slotCloseProfile ()
{
  svProfileD->close ();
}



void 
otherRecs::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  sv_profile->redrawMapArea (NVTrue);
}
