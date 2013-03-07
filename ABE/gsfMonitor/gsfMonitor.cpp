
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



#include "gsfMonitor.hpp"
#include "gsfMonitorHelp.hpp"


NV_FLOAT64 settings_version = 1.00;


gsfMonitor::gsfMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;
  
  lock_track = NVFalse;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);
  setFocus (Qt::ActiveWindowFocusReason);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/gsf_monitor.png"));


  kill_switch = ANCILLARY_FORCE_EXIT;

  NV_INT32 option_index = 0;
  while (NVTrue) 
    {
      static struct option long_options[] = {{"shared_memory_key", required_argument, 0, 0},
                                             {"kill_switch", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;

      switch (c) 
        {
        case 0:

          switch (option_index)
            {
            case 0:
              sscanf (optarg, "%d", &key);
              break;

            case 1:
              sscanf (optarg, "%d", &kill_switch);
              break;

            default:
              NV_CHAR tmp;
              sscanf (optarg, "%1c", &tmp);
              ac[option_index] = (NV_U_INT32) tmp;
              break;
            }
          break;
        }
    }


  envin ();


  /******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY **************************************** \

      This is a little note about the use of shared memory within the Area-Based Editor (ABE) programs.  If you read
      the Qt documentation (or anyone else's documentation) about the use of shared memory they will say "Dear [insert
      name of omnipotent being of your choice here], whatever you do, always lock shared memory when you use it!".
      The reason they say this is that access to shared memory is not atomic.  That is, reading shared memory and then
      writing to it is not a single operation.  An example of why this might be important - two programs are running,
      the first checks a value in shared memory, sees that it is a zero.  The second program checks the same location
      and sees that it is a zero.  These two programs have different actions they must perform depending on the value
      of that particular location in shared memory.  Now the first program writes a one to that location which was
      supposed to tell the second program to do something but the second program thinks it's a zero.  The second program
      doesn't do what it's supposed to do and it writes a two to that location.  The two will tell the first program 
      to do something.  Obviously this could be a problem.  In real life, this almost never occurs.  Also, if you write
      your program properly you can make sure this doesn't happen.  In ABE we almost never lock shared memory because
      something much worse than two programs getting out of sync can occur.  If we start a program and it locks shared
      memory and then dies, all the other programs will be locked up.  When you look through the ABE code you'll see
      that we very rarely lock shared memory, and then only for very short periods of time.  This is by design.

  \******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY ****************************************/


  //  Get the shared memory area.  If it doesn't exist, quit.  It should have already been created 
  //  by pfmView or geoSwath.

  QString skey;
  skey.sprintf ("%d_abe", key);

  abeShare = new QSharedMemory (skey);

  if (!abeShare->attach (QSharedMemory::ReadWrite))
    {
      fprintf (stderr, "\n\nError retrieving shared memory segment in %s.\n\n", argv[0]);
      exit (-1);
    }

  abe_share = (ABE_SHARE *) abeShare->data ();


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  QFrame *frame = new QFrame (this, 0);
  
  setCentralWidget (frame);

  QVBoxLayout *vBox = new QVBoxLayout ();
  vBox->setSpacing(0);
  frame->setLayout (vBox);

  listBox = new QTextEdit (this);
  listBox->setReadOnly (TRUE);


  vBox->addWidget (listBox);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);


  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QAction *filePrefsAction= new QAction (tr ("&Prefs"), this);
  filePrefsAction->setStatusTip (tr ("Set Preferences"));
  connect (filePrefsAction, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (filePrefsAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileQuitAction);


  //  Setup the help menu.  I like leaving the About Qt in since this is
  //  a really nice package - and it's open source.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about gsfMonitor"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *aboutQtAct = new QAction (tr ("About&Qt"), this);
  aboutQtAct->setShortcut (tr ("Ctrl+Q"));
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QAction *whatThisAct = QWhatsThis::createAction (this);

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (aboutQtAct);
  helpMenu->addAction (whatThisAct);


  //  Setup the tracking timer

  QTimer *track = new QTimer (this);
  connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
  track->start (10);
}



gsfMonitor::~gsfMonitor ()
{
}


void gsfMonitor::closeEvent (QCloseEvent * event __attribute__ ((unused)))
{
  slotQuit ();
}

  

void 
gsfMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  force_redraw = NVTrue;
}



void
gsfMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  Timer - timeout signal.  Very much like an X workproc.

void
gsfMonitor::trackCursor ()
{
  NV_INT32                year, day, mday, month, hour, minute, gsf_handle = -1;
  NV_FLOAT32              second;
  static ABE_SHARE        l_share;
  QString                 string, string2;
  NV_CHAR                 ltstring[30], lnstring[30], hem;
  NV_FLOAT64              deg, min, sec;
  gsfDataID               gsf_data_id;
  gsfRecords              gsf_record;



  //  Locked, more than likely because we're having to index a GSF file.

  if (lock_track) return;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT || abe_share->key == kill_switch) slotQuit ();


  //  Check for GSF, no memory lock, record change, force_redraw.

  if ((abe_share->mwShare.multiType[0] == PFM_GSF_DATA && abe_share->key < NV_INT32_MAX && l_share.nearest_point != abe_share->nearest_point) || force_redraw)
    {
      force_redraw = NVFalse;


      abeShare->lock ();
      l_share = *abe_share;
      abeShare->unlock ();


      QString ndx_file;

      ndx_file = l_share.nearest_filename;
      ndx_file.replace (ndx_file.length () - 3, 1, "n");

      FILE *fp;


      //  Check to see if we need to index the input file.

      if ((fp = fopen (ndx_file.toAscii (), "r")) == NULL)
        {
          QMessageBox::warning (this, tr ("gsfMonitor"), tr ("Indexing GSF file, please be patient."));

          lock_track = NVTrue;

          if (gsfOpen (l_share.nearest_filename, GSF_READONLY_INDEX, &gsf_handle)) 
            {
              QMessageBox::warning (this, tr ("Open GSF File"), tr ("Unable to index GSF file"));
              lock_track = NVFalse;
              return;
            }
          gsfClose (gsf_handle);

          lock_track = NVFalse;
        }
      else
        {
          fclose (fp);
        }


      //  Open the GSF file and read the data.

      if (gsfOpen (l_share.nearest_filename, GSF_READONLY_INDEX, &gsf_handle)) 
        {
          QMessageBox::warning (this, tr ("gsfMonitor"), tr ("Unable to open GSF file"));
          return;
        }


      //  Get the GSF version and set it in the title

      gsfSeek (gsf_handle, GSF_REWIND);
      gsfRead (gsf_handle, GSF_NEXT_RECORD, &gsf_data_id, &gsf_record, NULL, 0);
      gsfSeek (gsf_handle, GSF_REWIND);
      setWindowTitle (QString (VERSION) + " : " + l_share.nearest_filename + " : " + QString (gsf_record.header.version));


      //  Erase the data

      listBox->clear ();


      //  Get the sensor type.

      memset (&gsf_record, 0, sizeof (gsfRecords));

      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = 1;
      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0) < 0) 
        {
          QString errmsg;

          errmsg.sprintf (tr ("Error reading record 1 : %s").toAscii (), gsfStringError ());

          QMessageBox::warning (this, tr ("gsfMonitor"), errmsg);

          return;
        }

      listBox->insertHtml (tr ("<b>Sensor : </b>") + QString (gsfGetSonarTextName (&gsf_record.mb_ping)) + "<br>");


      //  Read the requested record.

      memset (&gsf_record, 0, sizeof (gsfRecords));

      gsf_data_id.recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      gsf_data_id.record_number = l_share.mwShare.multiRecord[0];
      NV_INT32 beam = l_share.mwShare.multiSubrecord[0];

      if (gsfRead (gsf_handle, GSF_RECORD_SWATH_BATHYMETRY_PING, &gsf_data_id, &gsf_record, NULL, 0) < 0) 
        {
          gsfClose (gsf_handle);

          QString errmsg;

          errmsg.sprintf (tr ("Error reading record %d : %s").toAscii (), gsf_data_id.record_number, gsfStringError ());

          QMessageBox::warning (this, tr ("Read GSF File"), errmsg);

          return;
        }


      gsfClose (gsf_handle);


      NV_FLOAT64 lat = gsf_record.mb_ping.latitude;
      NV_FLOAT64 lon = gsf_record.mb_ping.longitude;
      NV_FLOAT64 ang1 = gsf_record.mb_ping.heading + 90.0;
      NV_FLOAT64 ang2 = gsf_record.mb_ping.heading;
      NV_FLOAT32 dep = 0.0, nom_dep = 0.0;
      NV_F64_COORD2 xy2, nxy;


      //  Make sure position and ping is valid.   

      if ((lat <= 90.0) && (lon <= 180.0) && !(gsf_record.mb_ping.ping_flags & GSF_IGNORE_PING))
        {
          //  Check for 1 beam multibeam.

          if (gsf_record.mb_ping.number_beams == 1)
            {
              nxy.x = lon;
              nxy.y = lat;

              nom_dep = 0.0;
              if (gsf_record.mb_ping.nominal_depth != NULL) nom_dep = gsf_record.mb_ping.nominal_depth[0];

              dep = gsf_record.mb_ping.depth[0];
            }
          else
            {
              nom_dep = 0.0;
              if (gsf_record.mb_ping.nominal_depth != NULL) nom_dep = gsf_record.mb_ping.nominal_depth[beam];

              dep = gsf_record.mb_ping.depth[beam];


              if (dep != 0.0 && gsf_record.mb_ping.beam_flags != NULL &&
                  !(check_flag (gsf_record.mb_ping.beam_flags[beam], HMPS_IGNORE_NULL_BEAM))) 
                {
                  //  Adjust for cross track position.  

                  NV_FLOAT64 lateral = gsf_record.mb_ping.across_track[beam];
                  newgp (lat, lon, ang1, lateral, &nxy.y, &nxy.x);


                  //  If the along track array is present then use it 
                
                  if (gsf_record.mb_ping.along_track != (NV_FLOAT64 *) NULL) 
                    {
                      xy2.y = nxy.y;
                      xy2.x = nxy.x;
                      lateral = gsf_record.mb_ping.along_track[beam];

                      newgp (xy2.y, xy2.x, ang2, lateral, &nxy.y, &nxy.x);
                    }
                }
            }
        }


      cvtime (gsf_record.mb_ping.ping_time.tv_sec, gsf_record.mb_ping.ping_time.tv_nsec, &year, &day, &hour, &minute, &second);
      jday2mday (year, day, &month, &mday);
      month++;
      string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day,
                      hour, minute, second);
      listBox->insertHtml (string);

      strcpy (ltstring, fixpos (nxy.y, &deg, &min, &sec, &hem, POS_LAT, pos_format));
      string.sprintf (tr ("<b>Latitude : </b>%s<br>").toAscii (), ltstring);
      listBox->insertHtml (string);

      strcpy (lnstring, fixpos (nxy.x, &deg, &min, &sec, &hem, POS_LON, pos_format));
      string.sprintf (tr ("<b>Longitude : </b>%s<br>").toAscii (), lnstring);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Ping number : </b>%d<br>").toAscii (), gsf_data_id.record_number);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Beam number : </b>%d<br>").toAscii (), beam);
      listBox->insertHtml (string);

      if (gsf_record.mb_ping.depth != NULL)
        {
          string.sprintf (tr ("<b>Depth : </b>%f<br>").toAscii (), dep);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.nominal_depth != NULL)
        {
          string.sprintf (tr ("<b>Nominal Depth : </b>%f<br>").toAscii (), nom_dep);
          listBox->insertHtml (string);
        }

      string.sprintf (tr ("<b>Height above ellipsoid : </b>%f<br>").toAscii (), gsf_record.mb_ping.height);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Ellipsoid/datum separation : </b>%f<br>").toAscii (), gsf_record.mb_ping.sep);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Number of beams : </b>%d<br>").toAscii (), gsf_record.mb_ping.number_beams);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Center beam : </b>%d<br>").toAscii (), gsf_record.mb_ping.center_beam);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Ping flags : </b>%x<br>").toAscii (), gsf_record.mb_ping.ping_flags);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Tide corrector : </b>%f<br>").toAscii (), gsf_record.mb_ping.tide_corrector);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>GPS Tide corrector : </b>%f<br>").toAscii (), gsf_record.mb_ping.gps_tide_corrector);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Depth corrector : </b>%f<br>").toAscii (), gsf_record.mb_ping.depth_corrector);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Heading : </b>%f<br>").toAscii (), gsf_record.mb_ping.heading);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Pitch : </b>%f<br>").toAscii (), gsf_record.mb_ping.pitch);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Roll : </b>%f<br>").toAscii (), gsf_record.mb_ping.roll);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Heave : </b>%f<br>").toAscii (), gsf_record.mb_ping.heave);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Course : </b>%f<br>").toAscii (), gsf_record.mb_ping.course);
      listBox->insertHtml (string);

      string.sprintf (tr ("<b>Speed : </b>%f<br>").toAscii (), gsf_record.mb_ping.speed);
      listBox->insertHtml (string);

      if (gsf_record.mb_ping.across_track != NULL)
        {
          string.sprintf (tr ("<b>Across track : </b>%f<br>").toAscii (), gsf_record.mb_ping.across_track[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.along_track != NULL)
        {
          string.sprintf (tr ("<b>Along track : </b>%f<br>").toAscii (), gsf_record.mb_ping.along_track[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.travel_time != NULL)
        {
          string.sprintf (tr ("<b>Travel time : </b>%f<br>").toAscii (), gsf_record.mb_ping.travel_time[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.beam_angle != NULL)
        {
          string.sprintf (tr ("<b>Beam angle : </b>%f<br>").toAscii (), gsf_record.mb_ping.beam_angle[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.mc_amplitude != NULL)
        {
          string.sprintf (tr ("<b>Mean calibrated amplitude : </b>%f<br>").toAscii (), gsf_record.mb_ping.mc_amplitude[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.mr_amplitude != NULL)
        {
          string.sprintf (tr ("<b>Mean relative amplitude : </b>%f<br>").toAscii (), gsf_record.mb_ping.mr_amplitude[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.echo_width != NULL)
        {
          string.sprintf (tr ("<b>Echo width : </b>%f<br>").toAscii (), gsf_record.mb_ping.echo_width[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.quality_factor != NULL)
        {
          string.sprintf (tr ("<b>Simrad quality factor : </b>%f<br>").toAscii (), gsf_record.mb_ping.quality_factor[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.receive_heave != NULL)
        {
          string.sprintf (tr ("<b>Receive heave : </b>%f<br>").toAscii (), gsf_record.mb_ping.receive_heave[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.depth_error != NULL)
        {
          string.sprintf (tr ("<b>Depth error : </b>%f<br>").toAscii (), gsf_record.mb_ping.depth_error[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.across_track_error != NULL)
        {
          string.sprintf (tr ("<b>Across track error : </b>%f<br>").toAscii (), gsf_record.mb_ping.across_track_error[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.along_track_error != NULL)
        {
          string.sprintf (tr ("<b>Along track error : </b>%f<br>").toAscii (), gsf_record.mb_ping.along_track_error[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.quality_flags != NULL)
        {
          string.sprintf (tr ("<b>Reson quality flags (hex) : </b>%x<br>").toAscii (), gsf_record.mb_ping.quality_flags[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.beam_flags != NULL)
        {
          string.sprintf (tr ("<b>Beam flags (hex) : </b>%x<br>").toAscii (), gsf_record.mb_ping.beam_flags[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.signal_to_noise != NULL)
        {
          string.sprintf (tr ("<b>Signal to noise : </b>%f<br>").toAscii (), gsf_record.mb_ping.signal_to_noise[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.beam_angle_forward != NULL)
        {
          string.sprintf (tr ("<b>Beam angle forward : </b>%f<br>").toAscii (), gsf_record.mb_ping.beam_angle_forward[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.vertical_error != NULL)
        {
          string.sprintf (tr ("<b>Vertical error : </b>%f<br>").toAscii (), gsf_record.mb_ping.vertical_error[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.horizontal_error != NULL)
        {
          string.sprintf (tr ("<b>Horizontal error : </b>%f<br>").toAscii (), gsf_record.mb_ping.horizontal_error[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.sector_number != NULL)
        {
          string.sprintf (tr ("<b>Transit sector number : </b>%d<br>").toAscii (), gsf_record.mb_ping.sector_number[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.detection_info != NULL)
        {
          string.sprintf (tr ("<b>Bottom detection method : </b>%d<br>").toAscii (), gsf_record.mb_ping.detection_info[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.incident_beam_adj != NULL)
        {
          string.sprintf (tr ("<b>Incident beam adjustment : </b>%f<br>").toAscii (), gsf_record.mb_ping.incident_beam_adj[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.system_cleaning != NULL)
        {
          string.sprintf (tr ("<b>System data cleaining info : </b>%d<br>").toAscii (), gsf_record.mb_ping.system_cleaning[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.doppler_corr != NULL)
        {
          string.sprintf (tr ("<b>Doppler correction : </b>%f<br>").toAscii (), gsf_record.mb_ping.doppler_corr[beam]);
          listBox->insertHtml (string);
        }

      if (gsf_record.mb_ping.brb_inten != NULL)
        {
          string.sprintf (tr ("<b>Bathymetric receive beam intensity : </b>%f<br>").toAscii (), gsf_record.mb_ping.brb_inten[beam]);
          listBox->insertHtml (string);
        }
    }
}



//  A bunch of slots.

void 
gsfMonitor::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void
gsfMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("gsfMonitor Preferences"));
  prefsD->setModal (TRUE);

  QVBoxLayout *vbox = new QVBoxLayout (prefsD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *fbox = new QGroupBox (tr ("Position Format"), prefsD);
  fbox->setWhatsThis (bGrpText);

  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  bGrp = new QButtonGroup (prefsD);
  bGrp->setExclusive (TRUE);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPosClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  QHBoxLayout *fboxSplit = new QHBoxLayout;
  QVBoxLayout *fboxLeft = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxLeft->addWidget (hd__);
  fboxRight->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);

  bGrp->button (pos_format)->setChecked (TRUE);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (prefsD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.xpm"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  bRestoreDefaults = new QPushButton (tr ("Restore Defaults"), this);
  bRestoreDefaults->setToolTip (tr ("Restore all preferences to the default state"));
  bRestoreDefaults->setWhatsThis (restoreDefaultsText);
  connect (bRestoreDefaults, SIGNAL (clicked ()), this, SLOT (slotRestoreDefaults ()));
  actions->addWidget (bRestoreDefaults);

  QPushButton *closeButton = new QPushButton (tr ("Close"), prefsD);
  closeButton->setToolTip (tr ("Close the preferences dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);


  prefsD->show ();
}



void
gsfMonitor::slotPosClicked (int id)
{
  pos_format = id;
}



void
gsfMonitor::slotClosePrefs ()
{
  prefsD->close ();
}



void
gsfMonitor::slotRestoreDefaults ()
{
  pos_format = POS_HDMS;
  width = 324;
  height = 360;
  window_x = 0;
  window_y = 0;


  force_redraw = NVTrue;
}



void
gsfMonitor::about ()
{
  QMessageBox::about (this, VERSION,
                      "gsfMonitor - GSF ping data monitor."
                      "\n\nAuthor : Jan C. Depner (jan.depner@navy.mil)");
}


void
gsfMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

void
gsfMonitor::envin ()
{
  NV_FLOAT64 saved_version = 1.01;


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;


  QSettings settings (tr ("navo.navy.mil"), tr ("gsfMonitor"));
  settings.beginGroup (tr ("gsfMonitor"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  pos_format = settings.value (tr ("position form"), pos_format).toInt ();

  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();


  this->restoreState (settings.value (tr ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
gsfMonitor::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("gsfMonitor"));
  settings.beginGroup (tr ("gsfMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
