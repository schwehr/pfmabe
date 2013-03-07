#include "lidarMonitor.hpp"
#include "lidarMonitorHelp.hpp"


NV_FLOAT64 settings_version = 1.00;


lidarMonitor::lidarMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;
  
  lock_track = NVFalse;


  QResource::registerResource ("/icons.rcc");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);
  setFocus (Qt::ActiveWindowFocusReason);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts_monitor.png"));


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


  setWindowTitle (QString (VERSION));


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


  //  Open all the PFM structures that are being viewed in pfmEdit/pfmView

  num_pfms = abe_share->pfm_count;

  for (NV_INT32 pfm = 0 ; pfm < num_pfms ; pfm++)
    {
      open_args[pfm] = abe_share->open_args[pfm];

      if ((pfm_handle[pfm] = open_existing_pfm_file (&open_args[pfm])) < 0)
	{
	  QMessageBox::warning (this, tr ("Open PFM Structure"),
				tr ("The file ") + QDir::toNativeSeparators (QString (abe_share->open_args[pfm].list_path)) + 
				tr (" is not a PFM handle or list file or there was an error reading the file.") +
				tr ("  The error message returned was:\n\n") +
				QString (pfm_error_str (pfm_error)));
	}
    }


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  //  Set all of the default values.
  
   
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
  aboutAct->setStatusTip (tr ("Information about lidarMonitor"));
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



lidarMonitor::~lidarMonitor ()
{
}


void lidarMonitor::closeEvent (QCloseEvent * event __attribute__ ((unused)))
{
  slotQuit ();
}

  

void 
lidarMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  force_redraw = NVTrue;
}



void
lidarMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



//  Timer - timeout signal.  Very much like an X workproc.

void
lidarMonitor::trackCursor ()
{
  NV_INT32                year, day, mday, month, hour, minute, type, system_num, wlf_handle = -1, hawkeye_handle = -1, czmil_handle = -1;
  NV_FLOAT32              second;
  static ABE_SHARE        l_share;
  FILE                    *cfp = NULL;
  HOF_HEADER_T            hof_header;
  TOF_HEADER_T            tof_header;
  WLF_HEADER              wlf_header;
  HAWKEYE_META_HEADER     *hawkeye_meta_header;
  HAWKEYE_CONTENTS_HEADER *hawkeye_contents_header;
  CZMIL_CXY_Header        czmil_cxy_header;
  HYDRO_OUTPUT_T          hof_record;
  TOPO_OUTPUT_T           tof_record;
  WLF_RECORD              wlf_record;
  HAWKEYE_RECORD          hawkeye_record;
  CZMIL_CXY_Data          czmil_record;
  //CZMIL_CWF_Data          czmil_cwf;
  QString                 string, string2;
  NV_BOOL                 secondary;
  NV_CHAR                 ltstring[30], lnstring[30], hem;
  NV_FLOAT64              deg, min, sec;


  //  We want to exit if we have locked the tracker to update our saved waveforms (in slotPlotWaves).

  if (lock_track) return;


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
      abe_share->key == kill_switch) slotQuit ();


  //  We can force an update using the same key as waveMonitor since they're looking at the same sahred memory area.

  if (abe_share->modcode == WAVEMONITOR_FORCE_REDRAW) force_redraw = NVTrue; 


  //  Check for HOF, TOF, WLF, HAWKEYE, CZMIL, no memory lock, record change, force_redraw.

  if (((abe_share->mwShare.multiType[0] == PFM_SHOALS_1K_DATA || abe_share->mwShare.multiType[0] == PFM_SHOALS_TOF_DATA ||
        abe_share->mwShare.multiType[0] == PFM_CHARTS_HOF_DATA || abe_share->mwShare.multiType[0] == PFM_WLF_DATA ||
        abe_share->mwShare.multiType[0] == PFM_HAWKEYE_HYDRO_DATA || abe_share->mwShare.multiType[0] == PFM_HAWKEYE_TOPO_DATA
        || abe_share->mwShare.multiType[0] == PFM_CZMIL_DATA) && abe_share->key < NV_INT32_MAX && l_share.nearest_point != abe_share->nearest_point) ||
      force_redraw)
    {
      force_redraw = NVFalse;


      abeShare->lock ();
      l_share = *abe_share;
      abeShare->unlock ();


      //  Open the CHARTS file and read the data.

      if (strstr (l_share.nearest_filename, ".hof"))
        {
          cfp = open_hof_file (l_share.nearest_filename);

          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = PFM_CHARTS_HOF_DATA;
        }
      else if (strstr (l_share.nearest_filename, ".tof"))
        {
          cfp = open_tof_file (l_share.nearest_filename);


          if (cfp == NULL)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (strerror (errno));
              statusBar ()->showMessage (string);
              
              return;
            }
          type = PFM_SHOALS_TOF_DATA;
        }
      else if (strstr (l_share.nearest_filename, ".bin"))
        {
          hawkeye_handle = hawkeye_open_file (l_share.nearest_filename, &hawkeye_meta_header, &hawkeye_contents_header, HAWKEYE_READONLY);


          if (hawkeye_handle < 0)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (hawkeye_strerror ());
              statusBar ()->showMessage (string);
              
              return;
            }


          //  We don't have to differentiate between hydro and topo for this app since the format is the same.

          type = PFM_HAWKEYE_HYDRO_DATA;
        }
      else if (strstr (l_share.nearest_filename, ".cxy"))
        {
          czmil_handle = czmil_open_file (l_share.nearest_filename, &czmil_cxy_header, CZMIL_READONLY, NVFalse);
          //czmil_handle = czmil_open_file (l_share.nearest_filename, &czmil_cxy_header, CZMIL_READONLY, NVTrue);


          if (czmil_handle != CZMIL_SUCCESS)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (czmil_strerror ());
              statusBar ()->showMessage (string);
              
              return;
            }


          //  We don't have to differentiate between hydro and topo for this app since the format is the same.

          type = PFM_CZMIL_DATA;
        }
      else
        {
          wlf_handle = wlf_open_file (l_share.nearest_filename, &wlf_header, WLF_READONLY);


          if (wlf_handle < 0)
            {
              string = tr ("Error opening ") + QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                QString (wlf_strerror ());
              statusBar ()->showMessage (string);
              
              return;
            }

          type = PFM_WLF_DATA;
        }


      //  Erase the data

      listBox->clear ();


      switch (type)
        {
        case PFM_SHOALS_1K_DATA:
        case PFM_CHARTS_HOF_DATA:
          hof_read_header (cfp, &hof_header);

          system_num = hof_header.text.ab_system_number - 1;

          hof_read_record (cfp, l_share.mwShare.multiRecord[0], &hof_record);


          if (hof_record.sec_depth == -998.0) 
            {        
              secondary = NVFalse;
            }
          else
            {        
              secondary = NVTrue;
            }


          //  Populate the textEdit box.

          string.sprintf (tr ("<b>Time Stamp : </b>").toAscii ());
          string2.sprintf (NV_INT64_SPECIFIER, hof_record.timestamp);
          listBox->insertHtml (string + string2 + "<br>");

          charts_cvtime (hof_record.timestamp, &year, &day, &hour, &minute, &second);
          charts_jday2mday (year, day, &month, &mday);
          month++;
          string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, 
                              minute, second);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>System number : </b>%d<br>").toAscii (), system_num);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>HAPS version : </b>%d<br>").toAscii (), hof_record.haps_version);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Position confidence : </b>%d<br>").toAscii (), hof_record.position_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Status : </b>%x<br>").toAscii (), hof_record.status);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Suggested delete/keep/swap : </b>%d<br>").toAscii (), hof_record.suggested_dks);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Suspect status : </b>%x<br>").toAscii (), hof_record.suspect_status);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Tide status : </b>%x<br>").toAscii (), hof_record.tide_status & 0x3);
          listBox->insertHtml (string);

          strcpy (ltstring, fixpos (hof_record.latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          string.sprintf (tr ("<b>Latitude : </b>%s<br>").toAscii (), ltstring);
          listBox->insertHtml (string);

          strcpy (lnstring, fixpos (hof_record.longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
          string.sprintf (tr ("<b>Longitude : </b>%s<br>").toAscii (), lnstring);
          listBox->insertHtml (string);

          strcpy (ltstring, fixpos (hof_record.sec_latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          string.sprintf (tr ("<b>Secondary latitude : </b>%f<br>").toAscii (), hof_record.sec_latitude);
          listBox->insertHtml (string);

          strcpy (lnstring, fixpos (hof_record.sec_longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
          string.sprintf (tr ("<b>Secondary longitude : </b>%f<br>").toAscii (), hof_record.sec_longitude);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Correct depth : </b>%f<br>").toAscii (), hof_record.correct_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Correct secondary depth : </b>%f<br>").toAscii (), hof_record.correct_sec_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Abbreviated depth confidence (ABDC) : </b>%d<br>").toAscii (), hof_record.abdc);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Secondary abbreviated depth confidence : </b>%d<br>").toAscii (), hof_record.sec_abdc);
          listBox->insertHtml (string);

          if (hof_record.data_type)
            {
              listBox->insertHtml (tr ("<b>Data type : </b>KGPS<br>"));
            }
          else
            {
              listBox->insertHtml (tr ("<b>Data type : </b>DGPS<br>"));
            }

          string.sprintf (tr ("<b>Tide corrected depth : </b>%f<br>").toAscii (), hof_record.tide_cor_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Reported depth : </b>%f<br>").toAscii (), hof_record.reported_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Result depth : </b>%f<br>").toAscii (), hof_record.result_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Secondary depth : </b>%f<br>").toAscii (), hof_record.sec_depth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Wave height : </b>%f<br>").toAscii (), hof_record.wave_height);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Elevation : </b>%f<br>").toAscii (), hof_record.elevation);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Topo : </b>%f<br>").toAscii (), hof_record.topo);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Altitude : </b>%f<br>").toAscii (), hof_record.altitude);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS elevation : </b>%f<br>").toAscii (), hof_record.kgps_elevation);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS result elevation : </b>%f<br>").toAscii (), hof_record.kgps_res_elev);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS secondary elevation : </b>%f<br>").toAscii (), hof_record.kgps_sec_elev);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS topo : </b>%f<br>").toAscii (), hof_record.kgps_topo);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS datum : </b>%f<br>").toAscii (), hof_record.kgps_datum);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS water level : </b>%f<br>").toAscii (), hof_record.kgps_water_level);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bottom confidence : </b>%f<br>").toAscii (), hof_record.bot_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Secondary bottom confidence : </b>%f<br>").toAscii (), hof_record.sec_bot_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Nadir angle : </b>%f<br>").toAscii (), hof_record.nadir_angle);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Scanner azimuth : </b>%f<br>").toAscii (), hof_record.scanner_azimuth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Surface figure of merit (FOM) apd : </b>%f<br>").toAscii (), hof_record.sfc_fom_apd);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Surface figure of merit (FOM) ir : </b>%f<br>").toAscii (), hof_record.sfc_fom_ir);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Surface figure of merit (FOM) raman : </b>%f<br>").toAscii (), hof_record.sfc_fom_ram);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Depth confidence : </b>%d<br>").toAscii (), hof_record.depth_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Secondary depth confidence : </b>%d<br>").toAscii (), hof_record.sec_depth_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Warnings : </b>%d<br>").toAscii (), hof_record.warnings);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Warnings2 : </b>%d<br>").toAscii (), hof_record.warnings2);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Warnings3 : </b>%d<br>").toAscii (), hof_record.warnings3);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Calc_bfom_thresh_times10[0] : </b>%d<br>").toAscii (), hof_record.calc_bfom_thresh_times10[0]);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Calc_bfom_thresh_times10[1] : </b>%d<br>").toAscii (), hof_record.calc_bfom_thresh_times10[1]);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Calc_bot_run_required[0] : </b>%d<br>").toAscii (), hof_record.calc_bot_run_required[0]);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Calc_bot_run_required[1] : </b>%d<br>").toAscii (), hof_record.calc_bot_run_required[1]);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bot_bin_first : </b>%d<br>").toAscii (), hof_record.bot_bin_first    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bot_bin_second : </b>%d<br>").toAscii (), hof_record.bot_bin_second    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bot_bin_used_pmt : </b>%d<br>").toAscii (), hof_record.bot_bin_used_pmt    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sec_bot_bin_used_pmt : </b>%d<br>").toAscii (), hof_record.sec_bot_bin_used_pmt    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bot_bin_used_apd : </b>%d<br>").toAscii (), hof_record.bot_bin_used_apd    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sec_bot_bin_used_apd : </b>%d<br>").toAscii (), hof_record.sec_bot_bin_used_apd);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Bot_channel : </b>%d<br>").toAscii (), hof_record.bot_channel        );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sec_bot_chan : </b>%d<br>").toAscii (), hof_record.sec_bot_chan    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sfc_bin_apd : </b>%d<br>").toAscii (), hof_record.sfc_bin_apd        );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sfc_bin_ir : </b>%d<br>").toAscii (), hof_record.sfc_bin_ir            );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sfc_bin_ram : </b>%d<br>").toAscii (), hof_record.sfc_bin_ram        );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sfc_channel_used : </b>%d<br>").toAscii (), hof_record.sfc_channel_used    );
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Ab_dep_conf : </b>%d<br>").toAscii (), hof_record.ab_dep_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Sec_ab_dep_conf : </b>%d<br>").toAscii (), hof_record.sec_ab_dep_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS_abd_conf : </b>%d<br>").toAscii (), hof_record.kgps_abd_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>KGPS_sec_abd_conf : </b>%d<br>").toAscii (), hof_record.kgps_sec_abd_conf);
          listBox->insertHtml (string);

          fclose (cfp);
          break;


        case PFM_SHOALS_TOF_DATA:
          tof_read_header (cfp, &tof_header);

          system_num = tof_header.text.ab_system_number - 1;

          tof_read_record (cfp, l_share.mwShare.multiRecord[0], &tof_record);


          //  Populate the textEdit box.

          string.sprintf (tr ("<b>Time Stamp : </b>").toAscii ());
          string2.sprintf (NV_INT64_SPECIFIER, tof_record.timestamp);
          listBox->insertHtml (string + string2 + "<br>");

          charts_cvtime (tof_record.timestamp, &year, &day, &hour, &minute, &second);
          charts_jday2mday (year, day, &month, &mday);
          month++;
          string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, 
                              minute, second);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>System number : </b>%d<br>").toAscii (), system_num);
          listBox->insertHtml (string);

          strcpy (ltstring, fixpos (tof_record.latitude_first, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          string.sprintf (tr ("<b>Latitude first : </b>%f<br>").toAscii (), tof_record.latitude_first);
          listBox->insertHtml (string);

          strcpy (lnstring, fixpos (tof_record.longitude_first, &deg, &min, &sec, &hem, POS_LON, pos_format));
          string.sprintf (tr ("<b>Longitude first : </b>%f<br>").toAscii (), tof_record.longitude_first);
          listBox->insertHtml (string);

          strcpy (ltstring, fixpos (tof_record.latitude_last, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          string.sprintf (tr ("<b>Latitude last : </b>%f<br>").toAscii (), tof_record.latitude_last);
          listBox->insertHtml (string);

          strcpy (lnstring, fixpos (tof_record.longitude_last, &deg, &min, &sec, &hem, POS_LON, pos_format));
          string.sprintf (tr ("<b>Longitude last : </b>%f<br>").toAscii (), tof_record.longitude_last);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Elevation first : </b>%f<br>").toAscii (), tof_record.elevation_first);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Elevation last : </b>%f<br>").toAscii (), tof_record.elevation_last);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Scanner azimuth : </b>%f<br>").toAscii (), tof_record.scanner_azimuth);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Nadir angle : </b>%f<br>").toAscii (), tof_record.nadir_angle);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Confidence first : </b>%d<br>").toAscii (), tof_record.conf_first);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Confidence last : </b>%d<br>").toAscii (), tof_record.conf_last);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Intensity first : </b>%hhd<br>").toAscii (), tof_record.intensity_first);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Intensity last : </b>%hhd<br>").toAscii (), tof_record.intensity_last);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Classification status : </b>%x<br>").toAscii (), tof_record.classification_status);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>TBD 1 : </b>%x<br>").toAscii (), tof_record.tbd_1);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Position conf : </b>%d<br>").toAscii (), tof_record.pos_conf);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>TBD 2 : </b>%x<br>").toAscii (), tof_record.tbd_2);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Result elevation first : </b>%f<br>").toAscii (), tof_record.result_elevation_first);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Result elevation last : </b>%f<br>").toAscii (), tof_record.result_elevation_last);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Altitude : </b>%f<br>").toAscii (), tof_record.altitude);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>TBD float : </b>%f<br>").toAscii (), tof_record.tbdfloat);
          listBox->insertHtml (string);

          fclose (cfp);
          break;


        case PFM_WLF_DATA:

          wlf_read_record (wlf_handle, l_share.mwShare.multiRecord[0], &wlf_record, NVFalse, NULL);


          //  Populate the textEdit box.

          cvtime (wlf_record.tv_sec, wlf_record.tv_nsec, &year, &day, &hour, &minute, &second);
          jday2mday (year, day, &month, &mday);
          month++;
          string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, 
                              minute, second);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>System : </b>%s<br>").toAscii (), wlf_header.system);
          listBox->insertHtml (string);

          if (wlf_header.opt.horizontal_uncertainty_present)
            {
              string.sprintf (tr ("<b>Horizontal uncertainty : </b>%f<br>").toAscii (), wlf_record.horizontal_uncertainty);
              listBox->insertHtml (string);
            }
              
          if (wlf_header.opt.vertical_uncertainty_present)
            {
              string.sprintf (tr ("<b>Vertical uncertainty : </b>%f<br>").toAscii (), wlf_record.vertical_uncertainty);
              listBox->insertHtml (string);
            }
              
          strcpy (lnstring, fixpos (wlf_record.x, &deg, &min, &sec, &hem, POS_LON, pos_format));
          string.sprintf (tr ("<b>Longitude : </b>%s<br>").toAscii (), lnstring);
          listBox->insertHtml (string);

          strcpy (ltstring, fixpos (wlf_record.y, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          string.sprintf (tr ("<b>Latitude : </b>%s<br>").toAscii (), ltstring);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Elevation : </b>%f<br>").toAscii (), wlf_record.z);
          listBox->insertHtml (string);

          if (wlf_header.opt.sensor_position_present)
            {
              strcpy (lnstring, fixpos (wlf_record.sensor_x, &deg, &min, &sec, &hem, POS_LON, pos_format));
              string.sprintf (tr ("<b>Sensor longitude : </b>%s<br>").toAscii (), lnstring);
              listBox->insertHtml (string);

              strcpy (ltstring, fixpos (wlf_record.sensor_y, &deg, &min, &sec, &hem, POS_LAT, pos_format));
              string.sprintf (tr ("<b>Sensor latitude : </b>%s<br>").toAscii (), ltstring);
              listBox->insertHtml (string);

              string.sprintf (tr ("<b>Sensor altitude : </b>%f<br>").toAscii (), wlf_record.sensor_z);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.sensor_attitude_present)
            {
              string.sprintf (tr ("<b>Sensor roll : </b>%f<br>").toAscii (), wlf_record.sensor_roll);
              listBox->insertHtml (string);

              string.sprintf (tr ("<b>Sensor pitch : </b>%f<br>").toAscii (), wlf_record.sensor_pitch);
              listBox->insertHtml (string);

              string.sprintf (tr ("<b>Sensor heading : </b>%f<br>").toAscii (), wlf_record.sensor_heading);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.scan_angle_present)
            {
              string.sprintf (tr ("<b>Scan angle : </b>%f<br>").toAscii (), wlf_record.scan_angle);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.nadir_angle_present)
            {
              string.sprintf (tr ("<b>Nadir angle : </b>%f<br>").toAscii (), wlf_record.nadir_angle);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.water_surface_present)
            {
              string.sprintf (tr ("<b>Water surface : </b>%f<br>").toAscii (), wlf_record.water_surface);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.z_offset_present)
            {
              string.sprintf (tr ("<b>Z offset : </b>%f<br>").toAscii (), wlf_record.z_offset);
              listBox->insertHtml (string);
            }

          string.sprintf (tr ("<b>Number of returns : </b>%d<br>").toAscii (), wlf_record.number_of_returns);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Return number : </b>%d<br>").toAscii (), wlf_record.return_number);
          listBox->insertHtml (string);

          if (wlf_header.opt.point_source_present)
            {
              string.sprintf (tr ("<b>Point source : </b>%d<br>").toAscii (), wlf_record.point_source);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.edge_of_flight_line_present)
            {
              string.sprintf (tr ("<b>Edge of flightline : </b>%d<br>").toAscii (), wlf_record.edge_of_flight_line);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.intensity_present)
            {
              string.sprintf (tr ("<b>Intensity : </b>%f<br>").toAscii (), wlf_record.intensity);
              listBox->insertHtml (string);
            }

          for (NV_INT32 i = 0 ; i < WLF_MAX_ATTR ; i++)
            {
              if (wlf_header.opt.attr_present[i])
                {
                  string.sprintf (tr ("<b>%s : </b>%f<br>").toAscii (), wlf_header.attr_name[i], wlf_record.attribute[i]);
                  listBox->insertHtml (string);
                }
            }

          if (wlf_header.opt.rgb_present)
            {
              string.sprintf (tr ("<b>Red/Green/Blue : </b>%d/%d/%d<br>").toAscii (), wlf_record.red, wlf_record.green, wlf_record.blue);
              listBox->insertHtml (string);
            }

          if (wlf_header.opt.reflectance_present)
            {
              string.sprintf (tr ("<b>Reflectance : </b>%f<br>").toAscii (), wlf_record.reflectance);
              listBox->insertHtml (string);
            }

          string.sprintf (tr ("<b>Classification : </b>%d<br>").toAscii (), wlf_record.classification);
          listBox->insertHtml (string);

          string.sprintf (tr ("<b>Status : </b>%x<br>").toAscii (), wlf_record.status);
          listBox->insertHtml (string);

          wlf_close_file (wlf_handle);
          break;


        case PFM_HAWKEYE_HYDRO_DATA:
        case PFM_HAWKEYE_TOPO_DATA:

          hawkeye_read_record (hawkeye_handle, l_share.mwShare.multiRecord[0], &hawkeye_record);


          //  Populate the textEdit box.

          if (hawkeye_contents_header->available.Timestamp)
            {
              cvtime (hawkeye_record.tv_sec, hawkeye_record.tv_nsec, &year, &day, &hour, &minute, &second);
              jday2mday (year, day, &month, &mday);
              month++;
              string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, 
                              minute, second);
              listBox->insertHtml (string);

              string.sprintf (tr ("<b>Timestamp : </b>%.9f<br>").toAscii (), hawkeye_record.Timestamp);
              listBox->insertHtml (string);
            }

          if (hawkeye_contents_header->available.Surface_Latitude)
            {
              strcpy (ltstring, fixpos (hawkeye_record.Surface_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
              string.sprintf (tr ("<b>Surface_Latitude : </b>%s<br>").toAscii (), ltstring);
              listBox->insertHtml (string);
            }
              
          if (hawkeye_contents_header->available.Surface_Longitude)
            {
              strcpy (lnstring, fixpos (hawkeye_record.Surface_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
              string.sprintf (tr ("<b>Surface_Longitude : </b>%s<br>").toAscii (), lnstring);
              listBox->insertHtml (string);
            }
              
          if (hawkeye_contents_header->available.Surface_Northing)
            {
              string.sprintf (tr ("<b>Surface_Northing : </b>%f<br>").toAscii (), hawkeye_record.Surface_Northing);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_Easting)
            {
              string.sprintf (tr ("<b>Surface_Easting </b>%f<br>").toAscii (), hawkeye_record.Surface_Easting);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_Altitude)
            {
              string.sprintf (tr ("<b>Surface_Altitude </b>%f<br>").toAscii (), hawkeye_record.Surface_Altitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Interpolated_Surface_Altitude)
            {
              string.sprintf (tr ("<b>Interpolated_Surface_Altitude </b>%f<br>").toAscii (), hawkeye_record.Interpolated_Surface_Altitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_Northing_StdDev)
            {
              string.sprintf (tr ("<b>Surface_Northing_StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Northing_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_Easting_StdDev)
            {
              string.sprintf (tr ("<b>Surface_Easting_StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Easting_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_Altitude_StdDev)
            {
              string.sprintf (tr ("<b>Surface_Altitude_StdDev </b>%f<br>").toAscii (), hawkeye_record.Surface_Altitude_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Latitude)
            {
              strcpy (ltstring, fixpos (hawkeye_record.Point_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
              string.sprintf (tr ("<b>Point_Latitude : </b>%s<br>").toAscii (), ltstring);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Longitude)
            {
              strcpy (lnstring, fixpos (hawkeye_record.Point_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
              string.sprintf (tr ("<b>Point_Longitude : </b>%s<br>").toAscii (), lnstring);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Northing)
            {
              string.sprintf (tr ("<b>Point_Northing </b>%f<br>").toAscii (), hawkeye_record.Point_Northing);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Easting)
            {
              string.sprintf (tr ("<b>Point_Easting </b>%f<br>").toAscii (), hawkeye_record.Point_Easting);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Altitude)
            {
              string.sprintf (tr ("<b>Point_Altitude </b>%f<br>").toAscii (), hawkeye_record.Point_Altitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Northing_StdDev)
            {
              string.sprintf (tr ("<b>Point_Northing_StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Northing_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Easting_StdDev)
            {
              string.sprintf (tr ("<b>Point_Easting_StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Easting_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Point_Altitude_StdDev)
            {
              string.sprintf (tr ("<b>Point_Altitude_StdDev </b>%f<br>").toAscii (), hawkeye_record.Point_Altitude_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Azimuth)
            {
              string.sprintf (tr ("<b>Azimuth </b>%f<br>").toAscii (), hawkeye_record.Azimuth);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Air_Nadir_Angle)
            {
              string.sprintf (tr ("<b>Air_Nadir_Angle </b>%f<br>").toAscii (), hawkeye_record.Air_Nadir_Angle);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Water_Nadir_Angle)
            {
              string.sprintf (tr ("<b>Water_Nadir_Angle </b>%f<br>").toAscii (), hawkeye_record.Water_Nadir_Angle);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Slant_range)
            {
              string.sprintf (tr ("<b>Slant_range </b>%f<br>").toAscii (), hawkeye_record.Slant_range);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Slant_Range_Comp_Model)
            {
              string.sprintf (tr ("<b>Slant_Range_Comp_Model </b>%d<br>").toAscii (), hawkeye_record.Slant_Range_Comp_Model);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Wave_Height)
            {
              string.sprintf (tr ("<b>Wave_Height </b>%f<br>").toAscii (), hawkeye_record.Wave_Height);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Water_Quality_Correction)
            {
              string.sprintf (tr ("<b>Water_Quality_Correction </b>%f<br>").toAscii (), hawkeye_record.Water_Quality_Correction);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Tidal_Correction)
            {
              string.sprintf (tr ("<b>Tidal_Correction </b>%f<br>").toAscii (), hawkeye_record.Tidal_Correction);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Depth)
            {
              string.sprintf (tr ("<b>Depth </b>%f<br>").toAscii (), hawkeye_record.Depth);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Depth_Amplitude)
            {
              string.sprintf (tr ("<b>Depth_Amplitude </b>%f<br>").toAscii (), hawkeye_record.Depth_Amplitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Depth_Class)
            {
              string.sprintf (tr ("<b>Depth_Class </b>%d<br>").toAscii (), hawkeye_record.Depth_Class);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Depth_Confidence)
            {
              string.sprintf (tr ("<b>Depth_Confidence </b>%f<br>").toAscii (), hawkeye_record.Depth_Confidence);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Pixel_Index)
            {
              string.sprintf (tr ("<b>Pixel_Index </b>%d<br>").toAscii (), hawkeye_record.Pixel_Index);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scanner_Angle_X)
            {
              string.sprintf (tr ("<b>Scanner_Angle_X </b>%f<br>").toAscii (), hawkeye_record.Scanner_Angle_X);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scanner_Angle_Y)
            {
              string.sprintf (tr ("<b>Scanner_Angle_Y </b>%f<br>").toAscii (), hawkeye_record.Scanner_Angle_Y);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Latitude)
            {
              strcpy (ltstring, fixpos (hawkeye_record.Aircraft_Latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
              string.sprintf (tr ("<b>Aircraft_Latitude : </b>%s<br>").toAscii (), ltstring);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Longitude)
            {
              strcpy (lnstring, fixpos (hawkeye_record.Aircraft_Longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
              string.sprintf (tr ("<b>Aircraft_Longitude : </b>%s<br>").toAscii (), lnstring);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Northing)
            {
              string.sprintf (tr ("<b>Aircraft_Northing </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Northing);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Easting)
            {
              string.sprintf (tr ("<b>Aircraft_Easting </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Easting);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Altitude)
            {
              string.sprintf (tr ("<b>Aircraft_Altitude </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Altitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Roll)
            {
              string.sprintf (tr ("<b>Aircraft_Roll </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Roll);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Pitch)
            {
              string.sprintf (tr ("<b>Aircraft_Pitch </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Pitch);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Heading)
            {
              string.sprintf (tr ("<b>Aircraft_Heading </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Heading);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Northing_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Northing_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Northing_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Easting_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Easting_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Easting_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Altitude_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Altitude_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Altitude_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Roll_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Roll_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Roll_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Pitch_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Pitch_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Pitch_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Aircraft_Heading_StdDev)
            {
              string.sprintf (tr ("<b>Aircraft_Heading_StdDev </b>%f<br>").toAscii (), hawkeye_record.Aircraft_Heading_StdDev);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Extracted_Waveform_Attributes)
            {
              string.sprintf (tr ("<b>Extracted_Waveform_Attributes </b>%d<br>").toAscii (), hawkeye_record.Extracted_Waveform_Attributes);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Receiver_Data_Used)
            {
              string.sprintf (tr ("<b>Receiver_Data_Used </b>%d<br>").toAscii (), hawkeye_record.Receiver_Data_Used);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Manual_Output_Screening_Flags)
            {
              string.sprintf (tr ("<b>Manual_Output_Screening_Flags </b>%x<br>").toAscii (), hawkeye_record.Manual_Output_Screening_Flags);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Amplitude)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Amplitude </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Amplitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Classification)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Classification </b>%d<br>").toAscii (), hawkeye_record.Waveform_Peak_Classification);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Contrast)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Contrast </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Contrast);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Debug_Flags)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Debug_Flags </b>%x<br>").toAscii (), hawkeye_record.Waveform_Peak_Debug_Flags);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Attributes)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Attributes </b>%d<br>").toAscii (), hawkeye_record.Waveform_Peak_Attributes);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Jitter)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Jitter </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Jitter);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Position)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Position </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Position);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Pulsewidth)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Pulsewidth </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Pulsewidth);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_SNR)
            {
              string.sprintf (tr ("<b>Waveform_Peak_SNR </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_SNR);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scan_Direction_Flag)
            {
              string.sprintf (tr ("<b>Scan_Direction_Flag </b>%d<br>").toAscii (), hawkeye_record.Scan_Direction_Flag);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Edge_of_Flightline)
            {
              string.sprintf (tr ("<b>Edge_of_Flightline </b>%d<br>").toAscii (), hawkeye_record.Edge_of_Flightline);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scan_Angle_Rank)
            {
              string.sprintf (tr ("<b>Scan_Angle_Rank </b>%d<br>").toAscii (), hawkeye_record.Scan_Angle_Rank);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.SelectBottomCase)
            {
              string.sprintf (tr ("<b>SelectBottomCase </b>%d<br>").toAscii (), hawkeye_record.SelectBottomCase);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Elevation)
            {
              string.sprintf (tr ("<b>Elevation </b>%f<br>").toAscii (), hawkeye_record.Elevation);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Return_Number)
            {
              string.sprintf (tr ("<b>Return_Number </b>%d<br>").toAscii (), hawkeye_record.Return_Number);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Number_of_Returns)
            {
              string.sprintf (tr ("<b>Number_of_Returns </b>%d<br>").toAscii (), hawkeye_record.Number_of_Returns);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Surface_altitude_interpolated_tide_corrected)
            {
              string.sprintf (tr ("<b>Surface_altitude_interpolated_tide_corrected </b>%f<br>").toAscii (),
                              hawkeye_record.Surface_altitude_interpolated_tide_corrected);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.IR_polarisation_ratio)
            {
              string.sprintf (tr ("<b>IR_polarisation_ratio </b>%f<br>").toAscii (), hawkeye_record.IR_polarisation_ratio);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Spatial_Quality)
            {
              string.sprintf (tr ("<b>Spatial_Quality </b>%f<br>").toAscii (), hawkeye_record.Spatial_Quality);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.IR_AMPLITUDE)
            {
              string.sprintf (tr ("<b>IR_AMPLITUDE </b>%f<br>").toAscii (), hawkeye_record.IR_AMPLITUDE);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scan_Position)
            {
              string.sprintf (tr ("<b>Scan_Position </b>%d<br>").toAscii (), hawkeye_record.Scan_Position);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.K_Lidar)
            {
              string.sprintf (tr ("<b>K_Lidar </b>%f<br>").toAscii (), hawkeye_record.K_Lidar);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.K_Lidar_Average)
            {
              string.sprintf (tr ("<b>K_Lidar_Average </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Average);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.K_Lidar_Deep_Momentary)
            {
              string.sprintf (tr ("<b>K_Lidar_Deep_Momentary </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Deep_Momentary);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.K_Lidar_Shallow_Momentary)
            {
              string.sprintf (tr ("<b>K_Lidar_Shallow_Momentary </b>%f<br>").toAscii (), hawkeye_record.K_Lidar_Shallow_Momentary);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Relative_reflectivity)
            {
              string.sprintf (tr ("<b>Relative_reflectivity </b>%f<br>").toAscii (), hawkeye_record.Relative_reflectivity);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Waveform_Peak_Gain)
            {
              string.sprintf (tr ("<b>Waveform_Peak_Gain </b>%f<br>").toAscii (), hawkeye_record.Waveform_Peak_Gain);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Adjusted_Amplitude)
            {
              string.sprintf (tr ("<b>Adjusted_Amplitude </b>%f<br>").toAscii (), hawkeye_record.Adjusted_Amplitude);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Water_Travel_Time)
            {
              string.sprintf (tr ("<b>Water_Travel_Time </b>%f<br>").toAscii (), hawkeye_record.Water_Travel_Time);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Attenuation_c)
            {
              string.sprintf (tr ("<b>Attenuation_c </b>%f<br>").toAscii (), hawkeye_record.Attenuation_c);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Absorption_a)
            {
              string.sprintf (tr ("<b>Absorption_a </b>%f<br>").toAscii (), hawkeye_record.Absorption_a);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Scattering_s)
            {
              string.sprintf (tr ("<b>Scattering_s </b>%f<br>").toAscii (), hawkeye_record.Scattering_s);
              listBox->insertHtml (string);
            }
          if (hawkeye_contents_header->available.Backscattering_Bb)
            {
              string.sprintf (tr ("<b>Backscattering_Bb </b>%f<br>").toAscii (), hawkeye_record.Backscattering_Bb);
              listBox->insertHtml (string);
            }

          hawkeye_close_file (hawkeye_handle);
          break;


        case PFM_CZMIL_DATA:

          czmil_read_cxy_record (czmil_handle, l_share.mwShare.multiRecord[0], &czmil_record);


          //  These are to be used for testing CZMIL waveform compression/decompression.

          //czmil_read_cwf_record (czmil_handle, l_share.mwShare.multiRecord[0], &czmil_cwf);
          //czmil_dump_cwf_record (czmil_cwf);


          //  Populate the textEdit box.

          czmil_cvtime (czmil_record.channel[CZMIL_DEEP_CHANNEL].timestamp, &year, &day, &hour, &minute, &second);
          jday2mday (year, day, &month, &mday);
          month++;
          string.sprintf (tr ("<b>Date/Time : </b>%d-%02d-%02d (%03d) %02d:%02d:%05.2f<br>").toAscii (), year + 1900, month, mday, day, hour, 
                          minute, second);
          listBox->insertHtml (string);

          for (NV_INT32 i = 0 ; i < 9 ; i++)
            {
              strcpy (lnstring, fixpos (czmil_record.channel[i].longitude, &deg, &min, &sec, &hem, POS_LON, pos_format));
              string.sprintf (tr ("<b>Channel[%d] longitude : </b>%s<br>").toAscii (), i, lnstring);
              listBox->insertHtml (string);

              strcpy (ltstring, fixpos (czmil_record.channel[i].latitude, &deg, &min, &sec, &hem, POS_LAT, pos_format));
              string.sprintf (tr ("<b>Channel[%d] latitude : </b>%s<br>").toAscii (), i, ltstring);
              listBox->insertHtml (string);

              for (NV_INT32 j = 0 ; j < 4 ; j++)
                {
                  string.sprintf (tr ("<b>Channel[%d] return elevation[%d] : </b>%f<br>").toAscii (), i, j, czmil_record.channel[i].return_elevation[j]);
                  listBox->insertHtml (string);
                }

              string.sprintf (tr ("<b>Channel[%d] waveform class : </b>%f<br>").toAscii (), i, czmil_record.channel[i].waveform_class);
              listBox->insertHtml (string);
            }

          czmil_close_file (czmil_handle);
          break;
        }
    }
}



//  A bunch of slots.

void 
lidarMonitor::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;


  envout ();


  //  Let go of the shared memory.

  abeShare->detach ();


  exit (0);
}



void
lidarMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("lidarMonitor Preferences"));
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
lidarMonitor::slotPosClicked (int id)
{
  pos_format = id;
}



void
lidarMonitor::slotClosePrefs ()
{
  prefsD->close ();
}



void
lidarMonitor::slotRestoreDefaults ()
{
  pos_format = POS_HDMS;
  width = 324;
  height = 600;
  window_x = 0;
  window_y = 0;


  force_redraw = NVTrue;
}



void
lidarMonitor::about ()
{
  QMessageBox::about (this, VERSION,
                      "lidarMonitor - CHARTS HOF and TOF monitor."
                      "\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)");
}


void
lidarMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

void
lidarMonitor::envin ()
{
  NV_FLOAT64 saved_version = 1.01;


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;


  QSettings settings (tr ("navo.navy.mil"), tr ("lidarMonitor"));
  settings.beginGroup (tr ("lidarMonitor"));

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
lidarMonitor::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("lidarMonitor"));
  settings.beginGroup (tr ("lidarMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
