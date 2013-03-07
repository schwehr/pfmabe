#include "chartsLAS.hpp"


NV_FLOAT64 settings_version = 2.0;
NV_INT16 datums[2] = {GCS_WGS_84, GCS_NAD83};


chartsLAS::chartsLAS (QWidget *parent)
  : QWizard (parent)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/chartsLASWatermark.png"));



  envin ();


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);

  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (window_width, window_height);
  this->move (window_x, window_y);

  setPage (0, new startPage (this, geoid03, datum, filePrefix));
  setPage (1, new areaInputPage (this, &inputAreaFilter, &inputAreas));
  setPage (2, new fileInputPage (this, &inputFileFilter, &inputFiles));
  setPage (3, new runPage (this, &progress));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the conversion process"));
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



chartsLAS::~chartsLAS ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  window_width = tmp.width ();
  window_height = tmp.height ();


  envout ();
}



void chartsLAS::initializePage (int id)
{
  button (QWizard::HelpButton)->setIcon (QIcon (":/icons/contextHelp.xpm"));
  button (QWizard::CustomButton1)->setEnabled (FALSE);

  switch (id)
    {
    case 0:
      input_files.clear ();
      break;

    case 1:
      break;

    case 2:
      break;

    case 3:
      button (QWizard::CustomButton1)->setEnabled (TRUE);

      outputDir = field ("out_edit").toString ();
      filePrefix = field ("pre_edit").toString ();
      geoid03 = field ("geoid").toBool ();
      datum = field ("hDatum").toInt ();

      break;
    }
}



void chartsLAS::cleanupPage (int id)
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



//  This is where the fun stuff happens.

void 
chartsLAS::slotCustomButtonClicked (int id __attribute__ ((unused)))
{
  QApplication::setOverrideCursor (Qt::WaitCursor);


  button (QWizard::FinishButton)->setEnabled (FALSE);
  button (QWizard::BackButton)->setEnabled (FALSE);
  button (QWizard::CustomButton1)->setEnabled (FALSE);


  //  Get the files from the QTextEdit box on the fileInputPage.

  QTextCursor inputCursor = inputFiles->textCursor ();

  inputCursor.setPosition (0);


  QStringList isort_files;

  isort_files.clear ();

  do
    {
      isort_files << inputCursor.block ().text ();
    } while (inputCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  isort_files.sort ();


  //  Remove dupes and place into input_files.

  QString name, prev_name = "";
  input_files.clear ();

  for (NV_INT32 i = 0 ; i < isort_files.size () ; i++)
    {
      name = isort_files.at (i);

      if (name != prev_name)
        {
          input_files.append (name);
          prev_name = name;
        }
    }


  //  Get the files from the QTextEdit box on the areaInputPage.

  QTextCursor areaCursor = inputAreas->textCursor ();

  areaCursor.setPosition (0);


  QStringList asort_files;

  asort_files.clear ();

  do
    {
      asort_files << areaCursor.block ().text ();
    } while (areaCursor.movePosition (QTextCursor::NextBlock));


  //  Sort so we can remove dupes.

  asort_files.sort ();


  //  Remove dupes and place into area_files.

  prev_name = "";
  input_areas.clear ();

  for (NV_INT32 i = 0 ; i < asort_files.size () ; i++)
    {
      name = asort_files.at (i);

      if (name != prev_name)
        {
          input_areas.append (name);
          prev_name = name;
        }
    }


  input_area_count = input_areas.size ();

  input_area_def = new AREA_DEFINITION[input_area_count];


  //  Get all of the input area polygons.

  for (NV_INT32 i = 0 ; i < input_area_count ; i++)
    {
      input_area_def[i].name = input_areas.at (i);

      input_area_def[i].poly_count = 0;

      NV_CHAR area_name[512];

      strcpy (area_name, input_area_def[i].name.toAscii ());

      if (get_area_mbr (area_name, &input_area_def[i].poly_count, input_area_def[i].polygon_x, input_area_def[i].polygon_y, &input_area_def[i].mbr))
	{
          //  Check to see if this is a rectangle.

          input_area_def[i].rect_flag = polygon_is_rectangle (input_area_def[i].poly_count, input_area_def[i].polygon_x, input_area_def[i].polygon_y);


	  //  Get the UTM zone from the center of the area.

	  NV_FLOAT64 central_meridian = input_area_def[i].mbr.min_x + (input_area_def[i].mbr.max_x - input_area_def[i].mbr.min_x) / 2.0;

          NV_CHAR string[60];
          if (input_area_def[i].mbr.max_y < 0.0)
            {
              sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +south +lon_0=%.9f", central_meridian);
            }
          else
            {
              sprintf (string, "+proj=utm -ellps=WGS84 +datum=WGS84 +lon_0=%.9f", central_meridian);
            }

          if (!(input_area_def[i].pj_utm = pj_init_plus (string)))
            {
              QMessageBox::critical (this, tr ("chartsLAS"), tr ("Error initializing UTM projection\n"));
              exit (-1);
            }

          if (!(input_area_def[i].pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84")))
            {
              QMessageBox::critical (this, tr ("chartsLAS"), tr ("Error initializing latlon projection\n"));
              exit (-1);
            }

          input_area_def[i].zone = (NV_INT32) (31.0 + central_meridian / 6.0);
          if (input_area_def[i].zone >= 61) input_area_def[i].zone = 60;	
          if (input_area_def[i].zone <= 0) input_area_def[i].zone = 1;
	}
      else
	{
	  QMessageBox::warning (this, tr ("chartsLAS Open area files"), tr ("The file ") + input_area_def[i].name + 
				tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
				QString (strerror (errno)));
	}
    }


  //  Make the overall directory and the directories for each area under the overall directory.

  QDir topDir = QDir (outputDir);

  if (!topDir.exists ())
    {
      if (!topDir.mkpath (outputDir))
        {
          perror (outputDir.toAscii ());
          exit (-1);
        }
    }


  for (NV_INT32 i = 0 ; i < input_area_count ; i++)
    {
      if (input_area_def[i].poly_count)
        {
          input_area_def[i].dir_name = outputDir + SEPARATOR;

          input_area_def[i].name = QFileInfo (input_area_def[i].name).baseName ();

          input_area_def[i].dir_name += input_area_def[i].name;

          QDir tmpDir = QDir (input_area_def[i].dir_name);
          if (!tmpDir.exists ())
            {
              if (!tmpDir.mkpath (input_area_def[i].dir_name))
                {
                  perror (input_area_def[i].dir_name.toAscii ());
                  exit (-1);
                }
            }
        }
    }


  input_file_count = input_files.size ();

  input_file_def = new FILE_DEFINITION[input_file_count];


  for (NV_INT32 i = 0 ; i < input_file_count ; i++)
    {
      input_file_def[i].name = input_files.at (i);

      if (input_file_def[i].name.endsWith (".tof"))
        {
          input_file_def[i].type = 1;
        }
      else
        {
          input_file_def[i].type = 0;
        }
    }


  //  Main processing loop

  TOF_HEADER_T                    tof_header;
  TOPO_OUTPUT_T                   tof;
  HOF_HEADER_T                    hof_header;
  HYDRO_OUTPUT_T                  hof;
  NV_INT32                        prev_hit = -1;


  progress.obar->setRange (0, input_file_count * 100);


  //  Loop through each input file.

  FILE *fp;
  NV_CHAR string[1024];


  for (NV_INT32 i = 0 ; i < input_file_count ; i++)
    {
      for (NV_INT32 j = 0 ; j < input_area_count ; j++)
        {
          input_area_def[j].writer = NULL;
          input_area_def[j].header = NULL;
          input_area_def[j].opened = NVFalse;
        }


      strcpy (string, input_file_def[i].name.toAscii ());


      QString status;
      status.sprintf ("Processing file %d of %d : ", i + 1, input_file_count);
      status += QFileInfo (input_file_def[i].name).fileName ();

      QListWidgetItem *stat = new QListWidgetItem (status);

      progress.list->addItem (stat);
      progress.list->setCurrentItem (stat);
      progress.list->scrollToItem (stat);


      //  TOF file section

      if (input_file_def[i].type)
        {
          //  Try to open the input file.

          if ((fp = open_tof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("chartsLAS Open input files"), tr ("The file ") + input_file_def[i].name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              tof_read_header (fp, &tof_header);


              //  Check for files older than 10/07/2011.  At that point we changed the way we load TOF data into PFM
              //  so that we load first returns even if the second return is bad.

              ver_dep_flag = NVFalse;
              NV_INT64 hd_start_time;
              sscanf (tof_header.text.start_time, NV_INT64_SPECIFIER, &hd_start_time);
              if (hd_start_time < 1317945600000000LL) ver_dep_flag = NVTrue;


              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (input_file_def[i].name).fileName ());

              progress.fbar->setRange (0, tof_header.text.number_shots);


              NV_INT32 prog_count = 0;


              //  Loop through the entire file reading each record.

              while (tof_read_record (fp, TOF_NEXT_RECORD, &tof))
                {
                  for (NV_INT32 k = 0 ; k < 2 ; k++)
                    {
                      NV_FLOAT64 lat, lon;

                      if (k)
                        {
                          lat = tof.latitude_last;
                          lon = tof.longitude_last;
                        }
                      else
                        {
                          lat = tof.latitude_first;
                          lon = tof.longitude_first;
                        }


                      //  Check against last area hit first then if it's not in that box check the rest.

                      NV_INT32 hit = -1;

                      if (prev_hit != -1)
                        {
                          if (input_area_def[prev_hit].rect_flag)
                            {
                              if (lon >= input_area_def[prev_hit].mbr.min_x && lon <= input_area_def[prev_hit].mbr.max_x &&
                                  lat >= input_area_def[prev_hit].mbr.min_y && lat <= input_area_def[prev_hit].mbr.max_y) hit = prev_hit;
                            }
                          else
                            {
                              if (inside_polygon2 (input_area_def[prev_hit].polygon_x, input_area_def[prev_hit].polygon_y, 
                                                   input_area_def[prev_hit].poly_count, lon, lat)) hit = prev_hit;
                            }
                        }


                      //  If the point wasn't in the previous area, beat the input point against all of the area boundaries to find
                      //  out which one it is in (if any).

                      if (hit == -1)
                        {
                          for (NV_INT32 j = 0 ; j < input_area_count ; j++)
                            {
                              if (input_area_def[j].rect_flag)
                                {
                                  if (lon >= input_area_def[j].mbr.min_x && lon <= input_area_def[j].mbr.max_x &&
                                      lat >= input_area_def[j].mbr.min_y && lat <= input_area_def[j].mbr.max_y)
                                    {
                                      hit = j;
                                      break;
                                    }
                                }
                              else
                                {
                                  if (inside_polygon2 (input_area_def[j].polygon_x, input_area_def[j].polygon_y, 
                                                       input_area_def[j].poly_count, lon, lat))
                                    {
                                      hit = j;
                                      break;
                                    }
                                }
                            }
                        }
                      prev_hit = hit;


                      //  If the point is in one of the areas, we need to create or open an LAS file and a pair of ASCII
                      //  files.

                      if (hit >= 0)
                        {
                          QString new_file, asc_file, ext;
                          ext.sprintf (".ta%1d", k + 1);

                          new_file = input_area_def[hit].dir_name + "/" + filePrefix + "__Area_" + input_area_def[hit].name + "__" + 
                            QFileInfo (input_file_def[i].name).baseName () + ".las";
                          asc_file = input_area_def[hit].dir_name + "/" + filePrefix + "__Area_" + input_area_def[hit].name + ext;


                          NV_CHAR nf[1024], af[2][1024];
                          strcpy (nf, new_file.toAscii ());
                          strcpy (af[k], asc_file.toAscii ());


                          QFileInfo new_file_info (new_file);
                          QFileInfo asc_file_info (asc_file);


                          //  If the las file for this area already exists but we didn't create it (opened = NVFalse) then
                          //  we need to get rid of it.

                          if (new_file_info.exists () && !input_area_def[hit].opened) remove (nf);


                          //  If the ASCII file doesn't exist, create it and write the header

                          if (!asc_file_info.exists ())
                            {
                              if ((input_area_def[hit].ascfp[k] = fopen64 (af[k], "w")) == NULL)
                                {
                                  perror (af[k]);
                                  exit (-1);
                                }

                              fprintf (input_area_def[hit].ascfp[k], 
                                       "# LONGITUDE, LATITUDE, UTM ZONE, EASTING, NORTHING, ELEV, ELEV (ellipsoid),  YYYY/MM/DD,HH:MM:SS.SSSSSS INTENSITY\n");
                            }


                          //  If the ASCII file exists, open it.

                          else
                            {
                              if (input_area_def[hit].ascfp[k] == NULL)
                                {
                                  if ((input_area_def[hit].ascfp[k] = fopen64 (af[k], "a")) == NULL)
                                    {
                                      perror (af[k]);
                                      exit (-1);
                                    }
                                }
                            }



                          //  If the LAS file doesn't exist, create it and write the dummy header (place holder)

                          if (!new_file_info.exists ())
                            {
                              //  write partial header as placeholder

                              input_area_def[hit].tmp_num_recs[0] = 0;
                              input_area_def[hit].tmp_num_recs[1] = 0;

                              input_area_def[hit].min.x = 99999999999.0;
                              input_area_def[hit].max.x = -99999999999.0;
                              input_area_def[hit].min.y = 99999999999.0;
                              input_area_def[hit].max.y = -99999999999.0;
                              input_area_def[hit].min.z = 99999999999.0;
                              input_area_def[hit].max.z = -99999999999.0;

                              if (!writeLASHeader (nf, tof_header, &input_area_def[hit]))
                                {
                                  LASError_Print ("Could not open LAS file");
                                  exit (-1);
                                }
                            }


                          //  Write out the LAS record

                          writeLASRecord (tof, &input_area_def[hit], k);


                          //  Write out the ASCII record

                          writeTOFASCRecord (tof, k, &input_area_def[hit]);
                        }
                    }


                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (NV_INT32) (((NV_FLOAT32) prog_count / 
                                                                  (NV_FLOAT32) tof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (tof_header.text.number_shots);


              //  Write final header for each LAS file and close the LAS files and ASCII files

              for (NV_INT32 i = 0 ; i < input_area_count ; i++)
                {
                  if (input_area_def[i].opened) 
                    {
                      writeLASHeader (NULL, tof_header, &input_area_def[i]);

                      if (input_area_def[i].ascfp[0] != NULL) fclose (input_area_def[i].ascfp[0]);
                      if (input_area_def[i].ascfp[1] != NULL) fclose (input_area_def[i].ascfp[1]);
                      input_area_def[i].ascfp[0] = NULL;
                      input_area_def[i].ascfp[1] = NULL;
                    }
                  input_area_def[i].opened = NVFalse;
                }
            }
        }


      //  HOF file section

      else
        {
          //  Try to open the HOF file

          if ((fp = open_hof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("Open input files"), tr ("The file ") + input_file_def[i].name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              hof_read_header (fp, &hof_header);

              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (input_file_def[i].name).fileName ());

              progress.fbar->setRange (0, hof_header.text.number_shots);


              NV_INT32 prog_count = 0;


              //  Loop through the entire file

              while (hof_read_record (fp, HOF_NEXT_RECORD, &hof))
                {
                  NV_FLOAT64 lat = hof.latitude, lon = hof.longitude;


                  //  Check against last area hit first then if it's not in that box check the rest.

                  NV_INT32 hit = -1;

                  if (prev_hit != -1)
                    {
                      if (input_area_def[prev_hit].rect_flag)
                        {
                          if (lon >= input_area_def[prev_hit].mbr.min_x && lon <= input_area_def[prev_hit].mbr.max_x &&
                              lat >= input_area_def[prev_hit].mbr.min_y && lat <= input_area_def[prev_hit].mbr.max_y) hit = prev_hit;
                        }
                      else
                        {
                          if (inside_polygon2 (input_area_def[prev_hit].polygon_x, input_area_def[prev_hit].polygon_y, 
                                               input_area_def[prev_hit].poly_count, lon, lat)) hit = prev_hit;
                        }
                    }


                  //  If the point wasn't in the previous area, beat the input point against all of the area boundaries to find out which one it is in (if any).

                  if (hit == -1)
                    {
                      for (NV_INT32 j = 0 ; j < input_area_count ; j++)
                        {
                          if (input_area_def[j].rect_flag)
                            {
                              if (lon >= input_area_def[j].mbr.min_x && lon <= input_area_def[j].mbr.max_x &&
                                  lat >= input_area_def[j].mbr.min_y && lat <= input_area_def[j].mbr.max_y)
                                {
                                  hit = j;
                                  break;
                                }
                            }
                          else
                            {
                              if (inside_polygon2 (input_area_def[j].polygon_x, input_area_def[j].polygon_y, 
                                                   input_area_def[j].poly_count, lon, lat))
                                {
                                  hit = j;
                                  break;
                                }
                            }
                        }
                    }
                  prev_hit = hit;


                  //  If the point is in one of the areas we need to create or open the ASCII output file

                  if (hit >= 0)
                    {
                      QString asc_file;
                      asc_file = input_area_def[hit].dir_name + QString ((NV_CHAR) SEPARATOR) + filePrefix + "__Area_" +
                        input_area_def[hit].name + ".has"; 

                      NV_CHAR af[1024];
                      strcpy (af, asc_file.toAscii ());


                      QFileInfo asc_file_info (asc_file);


                      //  If the file doesn't exist, create it

                      if (!asc_file_info.exists ())
                        {
                          if ((input_area_def[hit].ascfp[0] = fopen64 (af, "w")) == NULL)
                            {
                              perror (af);
                              exit (-1);
                            }

                          fprintf (input_area_def[hit].ascfp[0], 
                                   "# LONGITUDE, LATITUDE, UTM ZONE, EASTING, NORTHING, ELEV, ELEV (ellipsoid), YYYY/MM/DD,HH:MM:SS.SSSSSS\n");


                          //  Dummy the opened value to signify we opened this file

                          input_area_def[hit].opened = NVTrue;
                        }


                      //  If it does exist, open it

                      else
                        {
                          if (!input_area_def[hit].opened)
                            {
                              if ((input_area_def[hit].ascfp[0] = fopen64 (af, "a")) == NULL)
                                {
                                  perror (af);
                                  exit (-1);
                                }


                              //  Dummy the opened value to signify we opened this file

                              input_area_def[hit].opened = NVTrue;
                            }
                        }


                      //  Write the HOF record to the ASCII file

                      writeHOFASCRecord (hof, &input_area_def[hit]);
                    }


                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (NV_INT32) (((NV_FLOAT32) prog_count / 
                                                                  (NV_FLOAT32) hof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (hof_header.text.number_shots);


              //  Close the output files.

              for (NV_INT32 i = 0 ; i < input_area_count ; i++)
                {
                  if (input_area_def[i].opened) fclose (input_area_def[i].ascfp[0]);

                  input_area_def[i].opened = NVFalse;
                }
            }
        }
    }

  progress.obar->setValue (input_file_count * 100);


  button (QWizard::FinishButton)->setEnabled (TRUE);
  button (QWizard::CancelButton)->setEnabled (FALSE);


  QApplication::restoreOverrideCursor ();
  qApp->processEvents ();


  progress.list->addItem (" ");
  QListWidgetItem *cur = new QListWidgetItem (tr ("Processing complete, press Finish to exit."));

  progress.list->addItem (cur);
  progress.list->setCurrentItem (cur);
  progress.list->scrollToItem (cur);
}



//  Get the users defaults.

void
chartsLAS::envin ()
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  inputAreaFilter = tr ("ACE area files (*.afs)");
  inputFileFilter = tr ("TOF (*.tof)");
  filePrefix = "";
  window_x = 0;
  window_y = 0;
  window_width = 900;
  window_height = 500;
  geoid03 = NVFalse;
  datum = 0;


  QSettings settings (tr ("navo.navy.mil"), tr ("chartsLAS"));

  settings.beginGroup (tr ("chartsLAS"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  inputAreaFilter = settings.value (tr ("input area filter"), inputAreaFilter).toString ();
  inputFileFilter = settings.value (tr ("input file filter"), inputFileFilter).toString ();

  filePrefix = settings.value (tr ("file prefix"), filePrefix).toString ();

  window_width = settings.value (tr ("width"), window_width).toInt ();
  window_height = settings.value (tr ("height"), window_height).toInt ();
  window_x = settings.value (tr ("x position"), window_x).toInt ();
  window_y = settings.value (tr ("y position"), window_y).toInt ();

  geoid03 = settings.value (tr ("geoid correction"), geoid03).toBool ();

  datum = settings.value (tr ("horizontal datum"), datum).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void
chartsLAS::envout ()
{
  QSettings settings (tr ("navo.navy.mil"), tr ("chartsLAS"));

  settings.beginGroup (tr ("chartsLAS"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("input area filter"), inputAreaFilter);
  settings.setValue (tr ("input file filter"), inputFileFilter);

  settings.setValue (tr ("file prefix"), filePrefix);

  settings.setValue (tr ("width"), window_width);
  settings.setValue (tr ("height"), window_height);
  settings.setValue (tr ("x position"), window_x);
  settings.setValue (tr ("y position"), window_y);

  settings.setValue (tr ("geoid correction"), geoid03);

  settings.setValue (tr ("horizontal datum"), datum);

  settings.endGroup ();
}



void 
chartsLAS::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}



NV_BOOL 
chartsLAS::writeLASHeader (NV_CHAR *file, TOF_HEADER_T tof_header, AREA_DEFINITION *area_def)
{
  NV_CHAR                         string[128];
  NV_INT32                        year, month, day, jday;
  time_t                          tv_sec;
  NV_INT32                        tv_nsec;
  struct tm                       tm;
  LASVLRH                         vlr = NULL;
  union
  {
    GEOKEYDIRECTORYTAG_RECORD       record;
    uint8_t                         data[24];
  } var;



  if (!area_def->opened)
    {
      area_def->header = LASHeader_Create ();

      strncpy (string, &tof_header.text.file_type[1], 12);

      sprintf (&string[12], "- system %d", tof_header.text.ab_system_number);

      LASHeader_SetSystemId (area_def->header, string);
      LASHeader_SetSoftwareId (area_def->header, VERSION);

      LASHeader_SetDataFormatId (area_def->header, 1);

      LASHeader_SetScale (area_def->header, 0.0000001, 0.0000001, 0.001);
      LASHeader_SetOffset (area_def->header, 0.0, 0.0, 0.0);

      sprintf (string, "JALBTCX - %s", tof_header.text.flight_date);

      LASHeader_SetProjectId (area_def->header, string);

      sscanf (tof_header.text.flight_date, "%04d%02d%02d", &year, &month, &day);
      mday2jday (year, month, day, &jday);


      //  tm struct wants years since 1900!!!

      tm.tm_year = year - 1900;
      tm.tm_mon = month - 1;
      tm.tm_mday = day;
      tm.tm_hour = 0;
      tm.tm_min = 0;
      tm.tm_sec = 0;
      tm.tm_isdst = -1;

      putenv ("TZ=GMT");
      tzset ();


      //  Get seconds from the epoch (01-01-1970) for the flight date.  This will also give us the day of the week
      //  for the GPS seconds of week calculation.

      tv_sec = mktime (&tm);
      tv_nsec = 0;


      //  Subtract the number of days since Saturday midnight (Sunday morning) in seconds.

      tv_sec = tv_sec - (tm.tm_wday * 86400);
      area_def->start_week = tv_sec;


      LASHeader_SetCreationDOY (area_def->header, jday);
      LASHeader_SetCreationYear (area_def->header, year);



      //  Set the required variable length record.

      vlr = LASVLR_Create ();

      LASVLR_SetUserId (vlr, "LASF_Projection");
      LASVLR_SetRecordId (vlr, 34735);


      /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).  */

      LASVLR_SetRecordLength (vlr, 24/*32*/);
      LASVLR_SetDescription (vlr, "GeoKeyDirTagDesc");


      memset (&var.record, 0, sizeof (GEOKEYDIRECTORYTAG_RECORD));

      var.record.key_directory_version = 1;
      var.record.key_revision = 1;
      var.record.minor_revision = 0;
      var.record.number_of_keys = 3;


      //  You can look these up in the GeoTIFF spec (appendices) basically they mean...


      //  Key 1

      //  GTModelTypeGeoKey (1024)

      var.record.key_id_0 = 1024;


      //  Empty  (0, and 1)

      var.record.TIFF_tag_location_0 = 0;
      var.record.count_0 = 1;


      //  ModelTypeGeographic  (2)   Geographic latitude-longitude System

      var.record.value_offset_0 = 2;


      //  Key 2

      //  GeographicTypeGeoKey (2048)

      var.record.key_id_1 = 2048;


      //  Empty (0, and 1)

      var.record.TIFF_tag_location_1 = 0;
      var.record.count_1 = 1;


      //  GCS_WGS_84 (4326)  GCS_NAD83 (4269)

      var.record.value_offset_1 = datums[datum];


      /*  Whenever NIIRS10 fixes the Geocue plugin for Arc we'll go back to having the third key (linear units).
      //  Key 3

      //  ProjLinearUnitsGeoKey (3076)

      var.record.key_id_2 = 3076;


      //  Empty (0, and 1)

      var.record.TIFF_tag_location_2 = 0;
      var.record.count_2 = 1;


      //  Linear_Meter (9001)

      var.record.value_offset_2 = 9001;
      */


      LASVLR_SetData (vlr, var.data, 24/*32*/);


      LASHeader_AddVLR (area_def->header, vlr);


      area_def->writer = LASWriter_Create (file, area_def->header, LAS_MODE_WRITE);


      area_def->opened = NVTrue;


      if (!area_def->writer) return (NVFalse);
    }
  else
    {
      LASHeader_SetPointRecordsByReturnCount (area_def->header, 0, area_def->tmp_num_recs[0]);
      LASHeader_SetPointRecordsByReturnCount (area_def->header, 1, area_def->tmp_num_recs[1]);

      LASHeader_SetPointRecordsCount (area_def->header, area_def->tmp_num_recs[0] + area_def->tmp_num_recs[1]);

      LASHeader_SetMin (area_def->header, area_def->min.x, area_def->min.y, area_def->min.z);
      LASHeader_SetMax (area_def->header, area_def->max.x, area_def->max.y, area_def->max.z);

      if (LASWriter_WriteHeader (area_def->writer, area_def->header))
        {
          LASError_Print ("Could not write LAS header");
          exit (-1);
        }

      LASWriter_Destroy (area_def->writer);
    }

  return (NVTrue);
}



void 
chartsLAS::writeLASRecord (TOPO_OUTPUT_T tof, AREA_DEFINITION *area_def, NV_INT32 k)
{
  NV_INT32                        scan_direction, edge_of_flightline, intens;
  LASPointH                       las = NULL;
  NV_FLOAT64                      lat, lon, z;


  //  We're blowing off the edge_of_flightline 'cause we really don't care ;-)

  edge_of_flightline = 0;


  scan_direction = 1;
  if (tof.scanner_azimuth < 0.0) scan_direction = 0;


  if (k)
    {
      //  A confidence value of 50 or below is bad.

      if (tof.elevation_last == -998.0 || tof.conf_last <= 50) return;

      area_def->tmp_num_recs[1]++;

      lat = tof.latitude_last;
      lon = tof.longitude_last;
      z = tof.elevation_last;
      intens = tof.intensity_last;
    }
  else
    {
      if (fabsf (tof.elevation_last - tof.elevation_first) <= 0.05) return;


      //  A confidence value of 50 or below is bad.

      if (tof.elevation_first == -998.0 || tof.conf_first <= 50) return;


      //  We don't load first returns if the last return was bad in files prior to 10/07/2011.

      if (ver_dep_flag && tof.elevation_last <= -998.0) return;


      area_def->tmp_num_recs[0]++;

      lat = tof.latitude_first;
      lon = tof.longitude_first;
      z = tof.elevation_first;
      intens = tof.intensity_first;
    }


  las = LASPoint_Create ();


  //  We're really setting the LASF 1.1 classification bits here but since classification
  //  wasn't really defined in 1.0 it shouldn't matter.


  //   Classification status byte (bits defined in GCSCnsts.h)

  //     bit 0 = classified        (1=classified; 0=not classified)
  //     bit 1 = non-bare_earth1   (0x02);
  //     bit 2 = non-bare_earth2   (0x04);
  //     bit 3 = water             (0x08);   
  //     rest TBD.

  //     If only the first bit is set it is BARE EARTH.
  //     If value is 0, it has not been classified.
  //     Yes, this is a weird definition...
  //     In future the two non bare earth bits will be split into
  //     buildings/vegetation.

  if (tof.classification_status & 0x01)
    {
      if (!(tof.classification_status & 0x0e))
        {
          LASPoint_SetClassification (las, 2);
        }
      else if (tof.classification_status & 0x02)
        {
          LASPoint_SetClassification (las, 6);
        }
      else if (tof.classification_status & 0x04)
        {
          LASPoint_SetClassification (las, 4);
        }
      else if (tof.classification_status & 0x08)
        {
          LASPoint_SetClassification (las, 9);
        }
    }


  LASPoint_SetScanAngleRank (las, NINT (tof.scanner_azimuth));

  LASPoint_SetTime (las, (NV_FLOAT64) tof.timestamp / 1000000.0 - area_def->start_week);


  //  If we are correcting to orthometric height... 

  if (geoid03)
    {
      NV_FLOAT32 value = get_geoid03 (lat, lon);

      if (value != -999.0)
        {
          z -= value;
        }
    }


  NV_U_BYTE num_ret = 1;
  if (tof.elevation_last != -998.0 && tof.conf_last > 50 && tof.elevation_first != -998.0 && tof.conf_first > 50 &&
      fabsf (tof.elevation_last - tof.elevation_first) > 0.05) num_ret = 2;


  LASPoint_SetX (las, lon);
  LASPoint_SetY (las, lat);
  LASPoint_SetZ (las, z);
  LASPoint_SetIntensity (las, intens);
  LASPoint_SetReturnNumber (las, k + 1);
  LASPoint_SetNumberOfReturns (las, num_ret);
  LASPoint_SetScanDirection (las, scan_direction);
  LASPoint_SetFlightLineEdge (las, edge_of_flightline);


  if (LASWriter_WritePoint (area_def->writer, las))
    {
      LASError_Print ("Could not write LAS point");
      exit (-1);
    }


  LASPoint_Destroy (las);
  las = NULL;

  area_def->min.x = MIN (area_def->min.x, lon);
  area_def->min.y = MIN (area_def->min.y, lat);
  area_def->min.z = MIN (area_def->min.z, z);
  area_def->max.x = MAX (area_def->max.x, lon);
  area_def->max.y = MAX (area_def->max.y, lat);
  area_def->max.z = MAX (area_def->max.z, z);
}



void 
chartsLAS::writeTOFASCRecord (TOPO_OUTPUT_T tof, NV_INT32 k, AREA_DEFINITION *area_def)
{
  NV_INT32        year, day, hour, minute, month, mday, intens;
  NV_FLOAT32      second, elev, res_elev;
  NV_FLOAT64      lat, lon;


  charts_cvtime (tof.timestamp, &year, &day, &hour, &minute, &second);
  charts_jday2mday (year, day, &month, &mday);
  month++;


  if (k)
    {
      //  A confidence value of 50 or below is bad.

      if (tof.elevation_last == -998.0 || tof.conf_last <= 50) return;

      lat = tof.latitude_last;
      lon = tof.longitude_last;
      elev = tof.elevation_last;
      res_elev = tof.result_elevation_last;
      intens = tof.intensity_last;
    }
  else
    {
      //  A confidence value of 50 or below is bad.

      if (tof.elevation_first == -998.0 || tof.conf_first <= 50) return;


      //  We don't load first returns if the last return was bad in files prior to 10/07/2011.

      if (ver_dep_flag && tof.elevation_last <= -998.0) return;


      lat = tof.latitude_first;
      lon = tof.longitude_first;
      elev = tof.elevation_first; 
      res_elev = tof.result_elevation_first;
      intens = tof.intensity_first;
    }


  NV_FLOAT64 x, y;


  //  If we are correcting to orthometric height... 

  if (geoid03)
    {
      NV_FLOAT32 value = get_geoid03 (lat, lon);

      if (value != -999.0) elev -= value;
    }


  x = lon * NV_DEG_TO_RAD;
  y = lat * NV_DEG_TO_RAD;
  pj_transform (area_def->pj_latlon, area_def->pj_utm, 1, 1, &x, &y, NULL);

  fprintf (area_def->ascfp[k], "%0.9lf,%0.9lf,%d,%0.3f,%0.3f,%0.2f,%0.2f,%d/%02d/%02d,%02d:%02d:%09.6f,%d\n",
           lon, lat, area_def->zone, x, y, elev, res_elev, year + 1900, month, mday, hour, minute, second, intens);
}



void 
chartsLAS::writeHOFASCRecord (HYDRO_OUTPUT_T hof, AREA_DEFINITION *area_def)
{
  NV_INT32        year, day, hour, minute, month, mday;
  NV_FLOAT32      second;


  if (!(hof.status & AU_STATUS_DELETED_BIT) && hof.abdc >= 70 && hof.correct_depth != -998.0)
    {
      charts_cvtime (hof.timestamp, &year, &day, &hour, &minute, &second);
      charts_jday2mday (year, day, &month, &mday);
      month++;


      NV_FLOAT64 x, y;
      NV_FLOAT32 z, ze;


      //  If we are correcting to orthometric height... 

      if (geoid03 && hof.data_type)
        {
          NV_FLOAT32 value = get_geoid03 (hof.latitude, hof.longitude);

          if (value != -999.0)
            {
              z = hof.correct_depth - value;
            }
          else
            {
              z = hof.correct_depth;
            }
          ze = hof.kgps_elevation;
        }
      else
        {
          z = hof.correct_depth;
          ze = hof.result_depth;
        }


      x = hof.longitude * NV_DEG_TO_RAD;
      y = hof.latitude * NV_DEG_TO_RAD;
      pj_transform (area_def->pj_latlon, area_def->pj_utm, 1, 1, &x, &y, NULL);

      fprintf (area_def->ascfp[0], "%0.9lf,%0.9lf,%d,%0.3f,%0.3f,%0.2f,%0.2f,%d/%02d/%02d,%02d:%02d:%09.6f\n",  
               hof.longitude, hof.latitude, area_def->zone, x, y, z, ze, year + 1900, month, mday, hour, minute, second);
    }
}
