
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



#include "charts2LAS.hpp"


NV_FLOAT64 settings_version = 2.0;
NV_INT16 datums[2] = {GCS_WGS_84, GCS_NAD83};


charts2LAS::charts2LAS (QWidget *parent)
  : QWizard (parent)
{
  QResource::registerResource ("/icons.rcc");


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts2LASWatermark.png"));


  envin ();


  setWizardStyle (QWizard::ClassicStyle);


  setOption (HaveHelpButton, TRUE);
  setOption (ExtendedWatermarkPixmap, FALSE);


  connect (this, SIGNAL (helpRequested ()), this, SLOT (slotHelpClicked ()));


  //  Set the window size and location from the defaults

  this->resize (window_width, window_height);
  this->move (window_x, window_y);

  setPage (0, new startPage (this, invalid, geoid03, datum));
  setPage (1, new fileInputPage (this, &inputFileFilter, &inputFiles));
  setPage (2, new runPage (this, &progress));


  setButtonText (QWizard::CustomButton1, tr("&Run"));
  setOption (QWizard::HaveCustomButton1, TRUE);
  button (QWizard::CustomButton1)->setToolTip (tr ("Start the conversion process"));
  connect (this, SIGNAL (customButtonClicked (int)), this, SLOT (slotCustomButtonClicked (int)));


  setStartId (0);
}



charts2LAS::~charts2LAS ()
{
}



void charts2LAS::initializePage (int id)
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

      button (QWizard::CustomButton1)->setEnabled (TRUE);

      invalid = field ("invalid").toBool ();
      geoid03 = field ("geoid").toBool ();
      datum = field ("hDatum").toInt ();


      //  Use frame geometry to get the absolute x and y.

      QRect tmp = this->frameGeometry ();
      window_x = tmp.x ();
      window_y = tmp.y ();


      //  Use geometry to get the width and height.

      tmp = this->geometry ();
      window_width = tmp.width ();
      window_height = tmp.height ();


      envout ();


      break;
    }
}



void charts2LAS::cleanupPage (int id)
{
  switch (id)
    {
    case 0:
      break;

    case 1:
      break;

    case 2:
      break;
    }
}



//  This is where the fun stuff happens.

void 
charts2LAS::slotCustomButtonClicked (int id __attribute__ ((unused)))
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


  input_file_count = input_files.size ();


  //  Main processing loop

  TOF_HEADER_T                    tof_header;
  TOPO_OUTPUT_T                   tof;
  HOF_HEADER_T                    hof_header;
  HYDRO_OUTPUT_T                  hof;
  NV_INT32                        type;
  NV_INT32                        tmp_num_recs[2];
  NV_INT64                        start_week;
  NV_F64_COORD3                   las_min;
  NV_F64_COORD3                   las_max;
  LASWriterH                      writer;
  LASHeaderH                      header;
  NV_CHAR                         string[1024];
  NV_INT32                        year, month, day, jday, hour, minute, mday;
  NV_FLOAT32                      second;
  time_t                          tv_sec;
  NV_INT32                        tv_nsec;
  struct tm                       tm;
  LASVLRH                         vlr = NULL;
  NV_INT32                        scan_direction, edge_of_flightline, intens;
  LASPointH                       las = NULL;
  NV_FLOAT64                      lat, lon, z, ze;

  union
  {
    GEOKEYDIRECTORYTAG_RECORD     record;
    uint8_t                       data[24];
  } var;


  progress.obar->setRange (0, input_file_count * 100);


  //  Loop through each input file.

  FILE *fp;


  for (NV_INT32 i = 0 ; i < input_file_count ; i++)
    {
      name = input_files.at (i);

      if (name.endsWith (".tof"))
        {
          type = 1;
        }
      else
        {
          type = 0;
        }


      writer = NULL;
      header = NULL;


      strcpy (string, name.toAscii ());


      QString status;
      status.sprintf ("Processing file %d of %d : ", i + 1, input_file_count);
      status += QFileInfo (name).fileName ();

      QListWidgetItem *stat = new QListWidgetItem (status);

      progress.list->addItem (stat);
      progress.list->setCurrentItem (stat);
      progress.list->scrollToItem (stat);


      //  TOF file section

      if (type)
        {
          //  Try to open the input file.

          if ((fp = open_tof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("charts2LAS Open input files"), tr ("The file ") + name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              tof_read_header (fp, &tof_header);


              //  Check for files older than 10/07/2011.  At that point we changed the way we load TOF data into PFM
              //  so that we load first returns even if the second return is bad.

              NV_BOOL ver_dep_flag = NVFalse;
              NV_INT64 hd_start_time;
              sscanf (tof_header.text.start_time, NV_INT64_SPECIFIER, &hd_start_time);
              if (hd_start_time < 1317945600000000LL) ver_dep_flag = NVTrue;


              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (name).fileName ());

              progress.fbar->setRange (0, tof_header.text.number_shots);


              QString new_file = name.replace (".tof", "_t.las");

              QFileInfo new_file_info (new_file);

              NV_CHAR file[1024];
              strcpy (file, new_file.toAscii ());


              //  write partial header as placeholder

              tmp_num_recs[0] = 0;
              tmp_num_recs[1] = 0;

              las_min.x = 99999999999.0;
              las_max.x = -99999999999.0;
              las_min.y = 99999999999.0;
              las_max.y = -99999999999.0;
              las_min.z = 99999999999.0;
              las_max.z = -99999999999.0;


              header = LASHeader_Create ();

              strncpy (string, &tof_header.text.file_type[1], 12);

              sprintf (&string[12], "- system %d", tof_header.text.ab_system_number);

              LASHeader_SetSystemId (header, string);
              LASHeader_SetSoftwareId (header, VERSION);

              LASHeader_SetDataFormatId (header, 1);

              LASHeader_SetScale (header, 0.0000001, 0.0000001, 0.001);
              LASHeader_SetOffset (header, 0.0, 0.0, 0.0);

              sprintf (string, "JALBTCX - %s", tof_header.text.flight_date);

              LASHeader_SetProjectId (header, string);

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
              start_week = tv_sec;


              LASHeader_SetCreationDOY (header, jday);
              LASHeader_SetCreationYear (header, year);


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


              LASHeader_AddVLR (header, vlr);


              writer = LASWriter_Create (file, header, LAS_MODE_WRITE);


              if (!writer)
                {
                  LASError_Print ("Could not open LAS file");
                  exit (-1);
                }


              NV_INT32 prog_count = 0;


              //  Loop through the entire file reading each record.

              while (tof_read_record (fp, TOF_NEXT_RECORD, &tof))
                {
                  for (NV_INT32 k = 0 ; k < 2 ; k++)
                    {
                      NV_FLOAT64 lat, lon;


                      //  Write out the LAS record


                      //  We're blowing off the edge_of_flightline 'cause we really don't care ;-)

                      edge_of_flightline = 0;


                      scan_direction = 1;
                      if (tof.scanner_azimuth < 0.0) scan_direction = 0;


                      NV_BOOL inv_flag = NVFalse;

                      if (k)
                        {
                          //  A confidence value of 50 or below is bad.

                          if (tof.elevation_last == -998.0 || (tof.conf_last <= 50 && !invalid)) continue;

                          if (tof.conf_last <= 50) inv_flag = NVTrue;

                          tmp_num_recs[1]++;

                          lat = tof.latitude_last;
                          lon = tof.longitude_last;
                          z = tof.elevation_last;
                          intens = tof.intensity_last;
                        }
                      else
                        {
                          //  A confidence value of 50 or below is bad.

                          if (fabsf (tof.elevation_last - tof.elevation_first) <= 0.05 || tof.elevation_first == -998.0 ||
                              (tof.conf_first <= 50 && !invalid)) continue;


                          //  We don't load first returns if the last return was bad in files prior to 10/07/2011.

                          if (ver_dep_flag && tof.elevation_last <= -998.0) continue;


                          if (tof.conf_first <= 50) inv_flag = NVTrue;

                          tmp_num_recs[0]++;

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
                      else
                        {
                          if (inv_flag) LASPoint_SetClassification (las, 27);
                        }


                      LASPoint_SetScanAngleRank (las, NINT (tof.scanner_azimuth));

                      LASPoint_SetTime (las, (NV_FLOAT64) tof.timestamp / 1000000.0 - start_week);


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
                      if (tof.elevation_last != -998.0 && (tof.conf_last > 50 || (tof.conf_last <= 50 && invalid)) &&
                          tof.elevation_first != -998.0 && (tof.conf_first > 50 || (tof.conf_first <= 50 && invalid)) &&
                          fabsf (tof.elevation_last - tof.elevation_first) > 0.05) num_ret = 2;


                      LASPoint_SetX (las, lon);
                      LASPoint_SetY (las, lat);
                      LASPoint_SetZ (las, z);
                      LASPoint_SetIntensity (las, intens);
                      LASPoint_SetReturnNumber (las, k + 1);
                      LASPoint_SetNumberOfReturns (las, num_ret);
                      LASPoint_SetScanDirection (las, scan_direction);
                      LASPoint_SetFlightLineEdge (las, edge_of_flightline);


                      if (LASWriter_WritePoint (writer, las))
                        {
                          LASError_Print ("Could not write LAS point");
                          exit (-1);
                        }


                      LASPoint_Destroy (las);
                      las = NULL;

                      las_min.x = qMin (las_min.x, lon);
                      las_min.y = qMin (las_min.y, lat);
                      las_min.z = qMin (las_min.z, z);
                      las_max.x = qMax (las_max.x, lon);
                      las_max.y = qMax (las_max.y, lat);
                      las_max.z = qMax (las_max.z, z);
                    }

                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (NV_INT32) (((NV_FLOAT32) prog_count / (NV_FLOAT32) tof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (tof_header.text.number_shots);


              //  Write final header for the LAS file and close the LAS file

              LASHeader_SetPointRecordsByReturnCount (header, 0, tmp_num_recs[0]);
              LASHeader_SetPointRecordsByReturnCount (header, 1, tmp_num_recs[1]);

              LASHeader_SetPointRecordsCount (header, tmp_num_recs[0] + tmp_num_recs[1]);

              LASHeader_SetMin (header, las_min.x, las_min.y, las_min.z);
              LASHeader_SetMax (header, las_max.x, las_max.y, las_max.z);

              if (LASWriter_WriteHeader (writer, header))
                {
                  LASError_Print ("Could not write LAS header");
                  exit (-1);
                }

              LASWriter_Destroy (writer);
            }
        }


      //  HOF file section

      else
        {
          //  Try to open the HOF file

          if ((fp = open_hof_file (string)) == NULL)
            {
              QMessageBox::warning (this, tr ("Open input files"), tr ("The file ") + name + 
                                    tr (" could not be opened.") + tr ("  The error message returned was:\n\n") +
                                    QString (strerror (errno)));
            }
          else
            {
              //  Read the header

              hof_read_header (fp, &hof_header);


              progress.fbar->reset ();

              progress.fbox->setTitle (QFileInfo (name).fileName ());

              progress.fbar->setRange (0, hof_header.text.number_shots);


              QString new_file = name.replace (".hof", "_h.las");

              QFileInfo new_file_info (new_file);

              NV_CHAR file[1024];
              strcpy (file, new_file.toAscii ());


              //  write partial header as placeholder

              tmp_num_recs[0] = 0;
              tmp_num_recs[1] = 0;

              las_min.x = 99999999999.0;
              las_max.x = -99999999999.0;
              las_min.y = 99999999999.0;
              las_max.y = -99999999999.0;
              las_min.z = 99999999999.0;
              las_max.z = -99999999999.0;


              header = LASHeader_Create ();

              strncpy (string, &hof_header.text.file_type[1], 12);

              sprintf (&string[12], "- system %d", hof_header.text.ab_system_number);

              LASHeader_SetSystemId (header, string);
              LASHeader_SetSoftwareId (header, VERSION);

              LASHeader_SetDataFormatId (header, 1);

              LASHeader_SetScale (header, 0.0000001, 0.0000001, 0.001);
              LASHeader_SetOffset (header, 0.0, 0.0, 0.0);

              sprintf (string, "JALBTCX - %s", hof_header.text.flight_date);

              LASHeader_SetProjectId (header, string);

              sscanf (hof_header.text.flight_date, "%04d%02d%02d", &year, &month, &day);
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
              start_week = tv_sec;


              LASHeader_SetCreationDOY (header, jday);
              LASHeader_SetCreationYear (header, year);


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


              LASHeader_AddVLR (header, vlr);


              writer = LASWriter_Create (file, header, LAS_MODE_WRITE);


              if (!writer)
                {
                  LASError_Print ("Could not open LAS file");
                  exit (-1);
                }


              NV_INT32 prog_count = 0;


              //  Loop through the entire file reading each record.

              while (hof_read_record (fp, HOF_NEXT_RECORD, &hof))
                {
                  if (!(hof.status & AU_STATUS_DELETED_BIT) && (hof.abdc >= 70 || invalid) && hof.correct_depth != -998.0)
                    {
                      charts_cvtime (hof.timestamp, &year, &day, &hour, &minute, &second);
                      charts_jday2mday (year, day, &month, &mday);
                      month++;


                      //  Write out the LAS record

                      //  We're blowing off the edge_of_flightline 'cause we really don't care ;-)

                      edge_of_flightline = 0;


                      scan_direction = 1;
                      if (hof.scanner_azimuth < 0.0) scan_direction = 0;


                      tmp_num_recs[0]++;

                      lat = hof.latitude;
                      lon = hof.longitude;


                      //  If we are correcting to orthometric height... 

                      if (geoid03 && hof.data_type)
                        {
                          NV_FLOAT32 value = get_geoid03 (lat, lon);

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


                      las = LASPoint_Create ();

                      LASPoint_SetScanAngleRank (las, NINT (hof.scanner_azimuth));

                      LASPoint_SetTime (las, (NV_FLOAT64) hof.timestamp / 1000000.0 - start_week);


                      LASPoint_SetX (las, lon);
                      LASPoint_SetY (las, lat);
                      LASPoint_SetZ (las, z);
                      LASPoint_SetReturnNumber (las, 1);
                      LASPoint_SetNumberOfReturns (las, 1);
                      LASPoint_SetScanDirection (las, scan_direction);
                      LASPoint_SetFlightLineEdge (las, edge_of_flightline);


                      //   Classification - set land data to 21, water to 29, invlaid to 27.

                      if (hof.abdc < 70)
                        {
                          LASPoint_SetClassification (las, 27);
                        }
                      else if (hof.abdc == 70)
                        {
                          LASPoint_SetClassification (las, 21);
                        }
                      else
                        {
                          LASPoint_SetClassification (las, 29);
                        }


                      if (LASWriter_WritePoint (writer, las))
                        {
                          LASError_Print ("Could not write LAS point");
                          exit (-1);
                        }


                      LASPoint_Destroy (las);
                      las = NULL;

                      las_min.x = qMin (las_min.x, lon);
                      las_min.y = qMin (las_min.y, lat);
                      las_min.z = qMin (las_min.z, z);
                      las_max.x = qMax (las_max.x, lon);
                      las_max.y = qMax (las_max.y, lat);
                      las_max.z = qMax (las_max.z, z);
                    }

                  progress.fbar->setValue (prog_count);

                  progress.obar->setValue (i * 100 + (NV_INT32) (((NV_FLOAT32) prog_count / (NV_FLOAT32) hof_header.text.number_shots) * 100.0));
                  qApp->processEvents ();

                  prog_count++;
                }

              fclose (fp);

              progress.fbar->setValue (hof_header.text.number_shots);


              //  Write final header for the LAS file and close the LAS file

              LASHeader_SetPointRecordsByReturnCount (header, 0, tmp_num_recs[0]);

              LASHeader_SetPointRecordsCount (header, tmp_num_recs[0]);

              LASHeader_SetMin (header, las_min.x, las_min.y, las_min.z);
              LASHeader_SetMax (header, las_max.x, las_max.y, las_max.z);

              if (LASWriter_WriteHeader (writer, header))
                {
                  LASError_Print ("Could not write LAS header");
                  exit (-1);
                }

              LASWriter_Destroy (writer);
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
charts2LAS::envin ()
{
  NV_FLOAT64 saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  inputFileFilter = tr ("TOF (*.tof)");
  window_x = 0;
  window_y = 0;
  window_width = 900;
  window_height = 500;
  invalid = NVFalse;
  geoid03 = NVFalse;
  datum = 0;


  QSettings settings (tr ("navo.navy.mil"), tr ("charts2LAS"));

  settings.beginGroup (tr ("charts2LAS"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  inputFileFilter = settings.value (tr ("input file filter"), inputFileFilter).toString ();

  window_width = settings.value (tr ("width"), window_width).toInt ();
  window_height = settings.value (tr ("height"), window_height).toInt ();
  window_x = settings.value (tr ("x position"), window_x).toInt ();
  window_y = settings.value (tr ("y position"), window_y).toInt ();

  invalid = settings.value (tr ("include invalid data"), invalid).toBool ();

  geoid03 = settings.value (tr ("geoid correction"), geoid03).toBool ();

  datum = settings.value (tr ("horizontal datum"), datum).toInt ();

  settings.endGroup ();
}




//  Save the users defaults.

void
charts2LAS::envout ()
{
  QSettings settings (tr ("navo.navy.mil"), tr ("charts2LAS"));

  settings.beginGroup (tr ("charts2LAS"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("input file filter"), inputFileFilter);

  settings.setValue (tr ("width"), window_width);
  settings.setValue (tr ("height"), window_height);
  settings.setValue (tr ("x position"), window_x);
  settings.setValue (tr ("y position"), window_y);

  settings.setValue (tr ("include invalid data"), invalid);

  settings.setValue (tr ("geoid correction"), geoid03);

  settings.setValue (tr ("horizontal datum"), datum);

  settings.endGroup ();
}



void 
charts2LAS::slotHelpClicked ()
{
  QWhatsThis::enterWhatsThisMode ();
}
