#include "trackLine.hpp"
#include "trackLineHelp.hpp"

trackLine::trackLine (QWidget *parent)
  : QWizard (parent, 0)
{
  void set_defaults (OPTIONS *options);


  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/trackLineWatermark.png"));


  set_defaults (&options);


  envin (&options);


  //  Set the window size and location from the defaults

  this->resize (options.width, options.height);
  this->move (options.window_x, options.window_y);


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  setPage (0, new startPage (this, &options));
  setPage (1, new optionsPage (this, &options));
  setPage (2, new inputPage (this, &options, &inputFiles));
  setPage (3, new runPage (this, &progress, &trackList));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Run the trackline generation process"));
  button (QWizard::CustomButton1)->setWhatsThis (runText);
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}


void trackLine::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);

  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      button (QWizard::CustomButton1)->setEnabled (TRUE);
      break;
    }
}



void trackLine::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      break;
    }
}



void 
trackLine::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void 
trackLine::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  options.heading[TRACKLINE_GSF] = field ("headingGSF").toDouble ();
  options.speed[TRACKLINE_GSF] = field ("speedGSF").toDouble ();
  options.heading[TRACKLINE_POS] = field ("headingPOS").toDouble ();
  options.speed[TRACKLINE_POS] = field ("speedPOS").toDouble ();
  options.heading[TRACKLINE_WLF] = field ("headingWLF").toDouble ();
  options.speed[TRACKLINE_WLF] = field ("speedWLF").toDouble ();
  options.heading[TRACKLINE_HWK] = field ("headingHWK").toDouble ();
  options.speed[TRACKLINE_HWK] = field ("speedHWK").toDouble ();


  QString outputFile = field ("outputFile").toString ();


  NV_INT32 points = 0;
  NV_INT32 track_points = 0;

  start_heading = 0.0;


  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  options.window_x = tmp.x ();
  options.window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  options.width = tmp.width ();
  options.height = tmp.height ();


  //  Get the files from the QTextEdit box on the inputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList sort_files;

  sort_files.clear ();

  do
    {
      sort_files << inputCursor.block ().text ();
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  sort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (NV_INT32 i = 0 ; i < sort_files.size () ; i++)
    {
      name = sort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);
  QApplication::setOverrideCursor (Qt::WaitCursor);


  NV_CHAR                   filename[512];
  NV_INT32                  gsf_handle = -1, wlf_handle = -1, hawkeye_handle = -1, hour, min, ping, eof, percent = 0, old_percent = -1;
  NV_FLOAT32                sec;
  NV_BOOL                   first = NVTrue, got_data;
  INFO                      data, prev_data, last_data = {0.0, 0.0, 0.0, 0.0, 0.0, 0, 0};
  FILE                      *fp = NULL, *ofp = NULL;
  gsfDataID                 gsf_id;
  gsfRecords                gsf_rec;
  POS_OUTPUT_T              pos;
  WLF_HEADER                wlf_header;
  WLF_RECORD                wlf_record;
  HAWKEYE_META_HEADER       *hawkeye_meta_header;
  HAWKEYE_CONTENTS_HEADER   *hawkeye_contents_header;
  HAWKEYE_RECORD            hawkeye_record;
  QListWidgetItem           *cur;
  QString                   tmpString;
  extern NV_INT32           gsfError;


  //  Add the extension if needed.

  if (!outputFile.endsWith (".trk")) outputFile.append (".trk");


  if ((ofp = fopen (outputFile.toAscii (), "w")) == NULL)
    {
      QMessageBox::critical (this, tr ("trackLine"), tr ("Unable to open output file %1!").arg (outputFile));
      exit (-1);
    }


  //  Save the output directory.  It might have been input manually instead of browsed.

  options.output_dir = QFileInfo (outputFile).absoluteDir ().absolutePath ();


  //  Save the options to the QSettings file.

  envout (&options);


  NV_INT32 file_count = input_files.size ();


  for (NV_INT32 i = 0 ; i < file_count ; i++)
    {
      strcpy (filename, input_files.at (i).toAscii ());


      progress.fbar->reset ();
      progress.fbar->setRange (0, 100);
      tmpString = QString (tr ("Processing file %1 of %2 - %3").arg (i + 1).arg (file_count).arg (QFileInfo (filename).fileName ()));
      progress.fbox->setTitle (tmpString);
      qApp->processEvents ();


      progress.fbar->setWhatsThis (tr ("Progress of input file processing"));


      first = NVTrue;
      ping = 0;


      //  Open the input files and read the data.

      fprintf (ofp, "FILE %03d = %s\n", i, filename);

      got_data = NVFalse;

      type = TRACKLINE_GSF;
      if (strstr (filename, ".pos") || strstr (filename, ".out") || strstr (filename, ".POS") || strstr (filename, ".OUT")) type = TRACKLINE_POS;
      if (strstr (filename, ".wlf") || strstr (filename, ".wtf") || strstr (filename, ".whf")) type = TRACKLINE_WLF;
      if (strstr (filename, ".bin")) type = TRACKLINE_HWK;


      switch (type)
	{
        case TRACKLINE_GSF:

          if (!gsfOpen (filename, GSF_READONLY_INDEX, &gsf_handle))
            {
              time_t tv_sec;
              long tv_nsec;

              NV_INT32 records = gsfIndexTime (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, -1, &tv_sec, &tv_nsec);

              for (NV_INT32 j = 0 ; j < records ; j++)
                {
                  gsf_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
                  gsf_id.record_number = j + 1;
                  if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_id, &gsf_rec, NULL, 0) < 0) break;

                  if (!(gsf_rec.mb_ping.ping_flags & GSF_IGNORE_PING) && gsf_rec.mb_ping.longitude != 0.0 && gsf_rec.mb_ping.latitude != 0.0 &&
                      fabs (gsf_rec.mb_ping.longitude) <= 180.0 && fabs (gsf_rec.mb_ping.latitude) <= 90.0)
                    {
                      ping++;

                      got_data = NVTrue;

                      points++;

                      data.f = i;
                      data.t = (NV_FLOAT64) gsf_rec.mb_ping.ping_time.tv_sec + (NV_FLOAT64) gsf_rec.mb_ping.ping_time.tv_nsec / 1000000000.0;
                      data.p = ping;
                      data.y = gsf_rec.mb_ping.latitude;
                      data.x = gsf_rec.mb_ping.longitude;
                      data.h = gsf_rec.mb_ping.heading;


                      last_data = data;


                      if (first)
                        {
                          data.s = 0.0;

                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);
                          first = NVFalse;

                          track_points++;
                        }
                      else if (changepos (&data, &prev_data))
                        {
                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);

                          track_points++;
                        }

                      prev_data = data;


                      percent = (NV_INT32) roundf (((NV_FLOAT32) (j) / records) * 100.0);
                      if (percent - old_percent >= 5 || old_percent > percent)
                        {
                          progress.fbar->setValue (percent);
                          old_percent = percent;

                          qApp->processEvents ();
                        }
                    }
                }

              if (got_data)
                {
                  hour = (int) last_data.t / 3600;
                  min = ((int) last_data.t % 3600) / 60;
                  sec = fmod (last_data.t, 60.0) * 60.0;

                  fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", last_data.f, last_data.p, last_data.y, last_data.x, last_data.h,
                           last_data.s, hour, min, sec);

                  track_points++;
                }

              gsfClose (gsf_handle);

              progress.fbar->setValue (100);
              qApp->processEvents ();
            }
          else
            {
              cur = new QListWidgetItem (QString (gsfStringError ()));
              trackList->addItem (cur);
              trackList->setCurrentItem (cur);
              trackList->scrollToItem (cur);

              if (gsfError != GSF_FOPEN_ERROR) gsfClose (gsf_handle);
            }

          break;


	case TRACKLINE_POS:

	  if ((fp = open_pos_file (filename)) != NULL)
	    {
              fseek (fp, 0, SEEK_END);
              eof = ftell (fp);
              fseek (fp, 0, SEEK_SET);


              //  Set the progress bar.


	      while (!pos_read_record (fp, &pos))
		{
		  ping++;

		  got_data = NVTrue;

		  points++;

		  data.f = i;
		  data.t = pos.gps_time;
		  data.p = ping;
		  data.y = pos.latitude * RAD_TO_DEG;
		  data.x = pos.longitude * RAD_TO_DEG;
		  data.h = (pos.platform_heading - pos.wander_angle) * RAD_TO_DEG;


                  last_data = data;


		  if (first)
		    {
                      data.s = 0.0;

                      hour = (int) data.t / 3600;
                      min = ((int) data.t % 3600) / 60;
                      sec = fmod (data.t, 60.0) * 60.0;


                      //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                      //  speeds using two adjacent points.

                      start_heading = data.h;

                      fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);
		      first = NVFalse;

                      track_points++;
		    }
		  else if (changepos (&data, &prev_data))
		    {
                      hour = (int) data.t / 3600;
                      min = ((int) data.t % 3600) / 60;
                      sec = fmod (data.t, 60.0) * 60.0;


                      //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                      //  speeds using two adjacent points.

                      start_heading = data.h;

                      fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);

                      track_points++;
		    }

                  prev_data = data;


                  //  We don't want to eat up all of the free time on the system with a progress bar.

                  percent = (NV_INT32) roundf (((NV_FLOAT32) (ftell (fp)) / eof) * 100.0);
                  if (percent - old_percent >= 5 || old_percent > percent)
                    {
                      progress.fbar->setValue (percent);
                      old_percent = percent;

                      qApp->processEvents ();
                    }
		}


              if (got_data)
                {
                  hour = (int) last_data.t / 3600;
                  min = ((int) last_data.t % 3600) / 60;
                  sec = fmod (last_data.t, 60.0) * 60.0;

                  fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", last_data.f, last_data.p, last_data.y, last_data.x, last_data.h,
                           last_data.s, hour, min, sec);

                  track_points++;
                }

              fclose (fp);

              progress.fbar->setValue (100);
              qApp->processEvents ();
	    }
	  break;


        case TRACKLINE_WLF:

          if ((wlf_handle = wlf_open_file (filename, &wlf_header, WLF_READONLY)) < 0)
            {
              cur = new QListWidgetItem (QString (wlf_strerror ()));
              trackList->addItem (cur);
              trackList->setCurrentItem (cur);
              trackList->scrollToItem (cur);
            }
          else
            {
              if (!(wlf_header.opt.sensor_position_present && wlf_header.opt.sensor_attitude_present))
                {
                  cur = new QListWidgetItem (tr ("This program requires sensor position and attitude data for WLF files."));
                  trackList->addItem (cur);
                  trackList->setCurrentItem (cur);
                  trackList->scrollToItem (cur);
                }
              else
                {
                  for (NV_U_INT32 j = 0 ; j < wlf_header.number_of_records ; j++)
                    {
                      if (wlf_read_record (wlf_handle, j, &wlf_record, NVFalse, NULL))
                        {
                          cur = new QListWidgetItem (QString (wlf_strerror ()));
                          trackList->addItem (cur);
                          trackList->setCurrentItem (cur);
                          trackList->scrollToItem (cur);
                          break;
                        }

                      ping++;

                      got_data = NVTrue;

                      points++;

                      data.f = i;
                      data.t = (NV_FLOAT64) wlf_record.tv_sec + (NV_FLOAT64) wlf_record.tv_nsec / 1000000000.0;
                      data.p = ping;
                      data.y = wlf_record.sensor_y;
                      data.x = wlf_record.sensor_z;
                      data.h = wlf_record.sensor_heading;

                      last_data = data;

                      if (first)
                        {
                          data.s = 0.0;

                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);
                          first = NVFalse;

                          track_points++;
                        }
                      else if (changepos (&data, &prev_data))
                        {
                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);

                          track_points++;
                        }

                      prev_data = data;


                      //  We don't want to eat up all of the free time on the system with a progress bar.

                      percent = (NV_INT32) roundf (((NV_FLOAT32) (j) / wlf_header.number_of_records) * 100.0);
                      if (percent - old_percent >= 5 || old_percent > percent)
                        {
                          progress.fbar->setValue (percent);
                          old_percent = percent;

                          qApp->processEvents ();
                        }
                    }


                  if (got_data)
                    {
                      hour = (int) last_data.t / 3600;
                      min = ((int) last_data.t % 3600) / 60;
                      sec = fmod (last_data.t, 60.0) * 60.0;

                      fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", last_data.f, last_data.p, last_data.y, last_data.x, last_data.h,
                               last_data.s, hour, min, sec);

                      track_points++;
                    }
                }

              wlf_close_file (wlf_handle);
            }

          progress.fbar->setValue (100);
          qApp->processEvents ();

          break;


        case TRACKLINE_HWK:

          if ((hawkeye_handle = hawkeye_open_file (filename, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_READONLY)) < 0)
            {
              cur = new QListWidgetItem (QString (hawkeye_strerror ()));
              trackList->addItem (cur);
              trackList->setCurrentItem (cur);
              trackList->scrollToItem (cur);
            }
          else
            {
              //  Check for the required fields.

              if (!hawkeye_contents_header->available.Timestamp || !hawkeye_contents_header->available.Aircraft_Latitude ||
                  !hawkeye_contents_header->available.Aircraft_Longitude || !hawkeye_contents_header->available.Aircraft_Heading)
                {
                  cur = new QListWidgetItem (tr ("This program requires Timestamp, Aircraft_Latitude, Aircraft_Longitude, and Aircraft_Heading data for HAWKEYE files."));
                  trackList->addItem (cur);
                  trackList->setCurrentItem (cur);
                  trackList->scrollToItem (cur);
                }
              else
                {
                  for (NV_U_INT32 j = 0 ; j < hawkeye_contents_header->NbrOfPointRecords ; j++)
                    {
                      if (hawkeye_read_record (hawkeye_handle, j, &hawkeye_record))
                        {
                          cur = new QListWidgetItem (QString (hawkeye_strerror ()));
                          trackList->addItem (cur);
                          trackList->setCurrentItem (cur);
                          trackList->scrollToItem (cur);
                          break;
                        }

                      ping++;

                      got_data = NVTrue;

                      points++;

                      data.f = i;
                      data.t = (NV_FLOAT64) hawkeye_record.tv_sec + (NV_FLOAT64) hawkeye_record.tv_nsec / 1000000000.0;
                      data.p = ping;
                      data.y = hawkeye_record.Aircraft_Latitude;
                      data.x = hawkeye_record.Aircraft_Longitude;
                      data.h = hawkeye_record.Aircraft_Heading;

                      if (first)
                        {
                          data.s = 0.0;

                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);
                          first = NVFalse;

                          track_points++;
                        }
                      else if (changepos (&data, &prev_data))
                        {
                          hour = (int) data.t / 3600;
                          min = ((int) data.t % 3600) / 60;
                          sec = fmod (data.t, 60.0) * 60.0;


                          //  Save the last good heading since we want to compare it against the current heading.  Conversely, we want to compare
                          //  speeds using two adjacent points.

                          start_heading = data.h;

                          fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", data.f, data.p, data.y, data.x, data.h, data.s, hour, min, sec);

                          track_points++;
                        }

                      prev_data = data;


                      //  We don't want to eat up all of the free time on the system with a progress bar.

                      percent = (NV_INT32) roundf (((NV_FLOAT32) (j) / hawkeye_contents_header->NbrOfPointRecords) * 100.0);
                      if (percent - old_percent >= 5 || old_percent > percent)
                        {
                          progress.fbar->setValue (percent);
                          old_percent = percent;

                          qApp->processEvents ();
                        }
                    }


                  if (got_data)
                    {
                      hour = (int) last_data.t / 3600;
                      min = ((int) last_data.t % 3600) / 60;
                      sec = fmod (last_data.t, 60.0) * 60.0;

                      fprintf (ofp, "+,%d,%d,%.9f,%.9f,%f,%f,XXXX,XXX,%02d,%02d,%f\n", last_data.f, last_data.p, last_data.y, last_data.x, last_data.h,
                               last_data.s, hour, min, sec);

                      track_points++;
                    }
                }

              hawkeye_close_file (hawkeye_handle);
            }

          progress.fbar->setValue (100);
          qApp->processEvents ();

          break;
	}


      cur = new QListWidgetItem (tr ("Completed processing file %1 of %2 - %3").arg (i + 1).arg (file_count).arg (QFileInfo (filename).fileName ()));
      trackList->addItem (cur);
      trackList->setCurrentItem (cur);
      trackList->scrollToItem (cur);
    }


  fclose (ofp);


  cur = new QListWidgetItem (tr ("\n\nDetermined %1 trackline points from %2 positions.\n\n").arg (track_points).arg (points));
  trackList->addItem (cur);
  trackList->setCurrentItem (cur);
  trackList->scrollToItem (cur);


  QApplication::restoreOverrideCursor ();
  button (QWizard::FinishButton)->setEnabled (TRUE);
}



NV_INT32 trackLine::heading_change (INFO *data)
{
  NV_FLOAT64          heading1 = start_heading, heading2 = data->h;


  //  Check for northerly heading and adjust if necessary

  if (heading1 > (360 - options.heading[type]) && heading2 < options.heading[type]) heading2 += 360;
  if (heading2 > (360 - options.heading[type]) && heading1 < options.heading[type]) heading1 += 360;

  return (fabs (heading1 - heading2) >= options.heading[type]);
}



NV_BOOL trackLine::changepos (INFO *prev_data, INFO *data)
{
  NV_FLOAT64          dist;
  NV_FLOAT64          az;
  NV_BOOL             change;


  change = NVFalse;


  //  Heading change greater than options.heading[type].

  if (heading_change (data))
    {
      change = NVTrue;
    }


  //  Time change greater than one minute.

  else if (data->t - prev_data->t > 60.0)
    {
      change = NVTrue;
    }


  //  Speed greater than options.speed[type].

  else
    {
      invgp (NV_A0, NV_B0, prev_data->y, prev_data->x, data->y, data->x, &dist, &az);


      data->s = dist / (data->t - prev_data->t) * MPS_TO_KNOTS;


      if (data->s > options.speed[type])
        {
          change = NVTrue;
        }
    }

  return (change);
}
