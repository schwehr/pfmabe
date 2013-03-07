#include "CZMILwaveMonitor.hpp"
#include "CZMILwaveMonitorHelp.hpp"

/* chl revision 08_30_2011 */
//NV_FLOAT64 settings_version = 3.02;
NV_FLOAT64 settings_version = 3.04;
/* end revision */


CZMILwaveMonitor::CZMILwaveMonitor (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent):
  QMainWindow (parent, 0)
{	

    extern char     *optarg;

	filError = NULL;

	for (NV_INT32 i = 0 ; i < NUM_WAVEFORMS ; i++) display_wave[i] = NVTrue;
	
	lock_track = NVFalse;

	QResource::registerResource ("/icons.rcc");


	//  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

	setFocusPolicy (Qt::WheelFocus);
	setFocus (Qt::ActiveWindowFocusReason); 


	//  Set the main icon

	setWindowIcon (QIcon (":/icons/wave_monitor.xpm"));


	kill_switch = ANCILLARY_FORCE_EXIT;

	NV_INT32 option_index = 0;
	while (NVTrue) 
	{
	  static struct option long_options[] = {{"actionkey00", required_argument, 0, 0},
											 {"actionkey01", required_argument, 0, 0},
											 {"actionkey02", required_argument, 0, 0},
											 {"actionkey03", required_argument, 0, 0},
											 {"actionkey04", required_argument, 0, 0},
											 {"actionkey05", required_argument, 0, 0},
											 {"shared_memory_key", required_argument, 0, 0},
											 {"kill_switch", required_argument, 0, 0},
											 {0, no_argument, 0, 0}};

	  NV_CHAR c = (NV_CHAR) getopt_long (*argc, argv, "o", long_options, &option_index);
	  if (c == -1) break;

	  switch (c) 
		{
		case 0:

		  switch (option_index)
			{
			case 6:
			  sscanf (optarg, "%d", &key);
			  break;

			case 7:
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


	//  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

	primaryColorNear = Qt::green;
	secondaryColorNear = Qt::red;
	transPrimaryColorNear = primaryColorNear;
	transPrimaryColorNear.setAlpha (WAVE_ALPHA);
	transSecondaryColorNear = secondaryColorNear;
	transSecondaryColorNear.setAlpha (WAVE_ALPHA);

	drawReference = NVFalse;
	numReferences = 0;

	envin ();
	
	setWindowTitle (QString (VERSION));


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


	//  Allocate the colors to be used for color by line.  These are full value and half value colors.
	//  We're going to range the colors from 200 to 0 then wrap around from 360 to 300 which gives us a 
	//  range of 260.  The colors transitions for this don't need to be smooth since the locations are
	//  somewhat random based on the line number.  These colors never change (unlike the color by depth or 
	//  attribute colors) but the alpha value may change.

	NV_FLOAT32 hue_inc = 260.0 / (NV_FLOAT32) (NUMSHADES + 1);

	for (NV_INT32 i = 0 ; i < 2 ; i++)
	{
	  NV_FLOAT32 start_hue = 200.0;

	  for (NV_INT32 j = 0 ; j < NUMSHADES ; j++)
		{
		  NV_INT32 hue = (NV_INT32) (start_hue - (NV_FLOAT32) j * hue_inc);
		  if (hue < 0) hue = 360 + hue;

		  NV_INT32 k = NUMSHADES * i + j;


		  if (i)
			{
			  colorArray[k].setHsv (hue, 255, 127, 255);
			}
		  else
			{
			  colorArray[k].setHsv (hue, 255, 255, 255);
			}
		}
	}


	//  Set all of the default values.  

	/* 0: deep, 1: shallow1, 2: shallow2, 3: shallow3, 4: shallow4
	 5: shallow5, 6: shallow6, 7: shallow7, 8: ir */
	 
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
	
		uTimeBound[i] = UPPER_TIME_BOUND;
		lTimeBound[i]= LOWER_TIME_BOUND;
		lCountBound[i] = LOWER_COUNT_BOUND;
		uCountBound[i] = UPPER_COUNT_BOUND;
	}	

	nonFileIO_force_redraw = NVFalse;

	for (int i = 0; i < NUM_WAVEFORMS; i++) mapBlocked[i] = NVFalse;
	
	wave_read = 0;
	active_window_id = getpid ();	

	//  Let pfmEdit know that we are up and running.

	/* Bit 1 constitutes CZMIL Waveform Viewer */    
    abe_share->mwShare.waveMonitorRunning |= 0x02;
    
	zoomFlag = NVFalse;
	zoomIsActive = NVFalse;
	abe_share->mwShare.multiMode = multiWaveMode;  

	abe_share->mwShare.multiPresent[0] = -1;
	for (NV_INT32 i = 1; i < MAX_STACK_POINTS; i++) abe_share->mwShare.multiPresent[i] = -1;

	abe_share->mwShare.multiSwitch = NVFalse;

	GetLabelSpacing ();
	
	//  Set the map values from the defaults

	mapdef.projection = NO_PROJECTION;
	mapdef.draw_width = width;
	mapdef.draw_height = height;
	mapdef.overlap_percent = 5;
	mapdef.grid_inc_x = 0.0;
	mapdef.grid_inc_y = 0.0;

	mapdef.coasts = NVFalse;
	mapdef.landmask = NVFalse;

	mapdef.border = 0;
	mapdef.coast_color = Qt::white;
	mapdef.grid_color = QColor (160, 160, 160, 127);

	mapdef.background_color = backgroundColor;

	mapdef.initial_bounds.min_x = 0;
	mapdef.initial_bounds.min_y = 0;

	mapdef.initial_bounds.max_x = 500;
	mapdef.initial_bounds.max_y = 500;


	QFrame *frame = new QFrame (this, 0);	
  
	setCentralWidget (frame);

	for (int i = 0; i < NUM_WAVEFORMS; i++) {
		map[i] = new nvMap (this, &mapdef);
		map[i]->setWhatsThis (mapText);
	}	

  	QHBoxLayout *hBox = new QHBoxLayout (frame);
	QVBoxLayout *vBox = new QVBoxLayout;
	QVBoxLayout *vBox2 = new QVBoxLayout;	
	
	statusBar = new QStatusBar (frame);
	statusBar->setSizeGripEnabled (FALSE);
	statusBar->show ();

	QHBoxLayout * hBox1 = new QHBoxLayout();
	
	hBox1->addWidget (map[0]);
	hBox1->addWidget (map[1]);
	hBox1->addWidget (map[2]);	

	vBox2->addLayout(hBox1);
	vBox2->addWidget (statusBar);	

	QHBoxLayout * hBox2 = new QHBoxLayout();

	hBox2->addWidget (map[3]);
	hBox2->addWidget (map[4]);		
	hBox2->addWidget (map[5]);
	
	QHBoxLayout * hBox3 = new QHBoxLayout();
	hBox3->addWidget (map[6]);
	hBox3->addWidget (map[7]);
	hBox3->addWidget (map[8]);	
	
	vBox->addLayout (vBox2);  
	vBox->addLayout (hBox2);	
	vBox->addLayout (hBox3);
	
	hBox->addLayout (vBox);		
	
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) map[i]->setVisible(savedSettings.visible[i]);

	nameLabel = new QLabel ("                                        ", this);
	nameLabel->setAlignment (Qt::AlignLeft);
	nameLabel->setMinimumSize (nameLabel->sizeHint ());
	nameLabel->setWhatsThis (nameLabelText);
	nameLabel->setToolTip (tr ("File name"));

	simDepth = new QLabel ("0000.00", this);
	simDepth->setAlignment (Qt::AlignCenter);
	
	simDepth->setMinimumSize (simDepth->sizeHint ());
	simDepth->setWhatsThis (simDepthText);
	simDepth->setToolTip (tr ("Tide/datum corrected depth"));
	simDepth->setMinimumSize (simDepth->sizeHint ());
	simDepth->setWhatsThis (simDepthText);
	simDepth->setToolTip (tr ("Simulated depth"));
	
	simDepth1 = new QLabel ("0000.00", this);
	simDepth1->setAlignment (Qt::AlignCenter);
	simDepth1->setMinimumSize (simDepth1->sizeHint ());
	simDepth1->setWhatsThis (simDepth1Text);
	simDepth1->setToolTip (tr ("Simulated depth1"));

	simDepth2 = new QLabel ("00", this);
	simDepth2->setAlignment (Qt::AlignCenter);
	simDepth2->setMinimumSize (simDepth2->sizeHint ());
	simDepth2->setWhatsThis (simDepth2Text);
	simDepth2->setToolTip (tr ("Simulated depth 2"));

	simKGPSElev = new QLabel ("00", this);
	simKGPSElev->setAlignment (Qt::AlignCenter);
	simKGPSElev->setMinimumSize (simKGPSElev->sizeHint ());
	simKGPSElev->setWhatsThis (simKGPSElevText);
	simKGPSElev->setToolTip (tr ("Simulated KGPS elevation"));  

	statusBar->addWidget (nameLabel, 1);
	statusBar->addWidget (simDepth);
	statusBar->addWidget (simDepth1);
	statusBar->addWidget (simDepth2);
	statusBar->addWidget (simKGPSElev);  
		
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
		
		connect (map[i], SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
					SLOT (slotMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
					
		connect (map[i], SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this,
					SLOT (slotMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
					
		connect (map[i], SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
		
		connect (map[i], SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotPlotWaves (NVMAP_DEF)));
	}	

	contextMenu = new QMenu (this);
	QAction *contextZoomIn = contextMenu->addAction (tr ("Zoom In"));
	QAction *contextResetZoom = contextMenu->addAction (tr ("Reset Zoom"));
	connect (contextZoomIn, SIGNAL (triggered ()), this, SLOT (slotZoomIn ()));

	connect (contextResetZoom, SIGNAL (triggered()), this, SLOT (slotContextResetZoom()));

	//  Setup the file menu.

	QAction *fileQuitAction = new QAction (tr ("&Quit"), this);


	fileQuitAction->setStatusTip (tr ("Exit from application"));
	connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

	QAction *filePrefsAction= new QAction (tr ("&Prefs"), this);
	filePrefsAction->setStatusTip (tr ("Set Preferences"));
	connect (filePrefsAction, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));


	QAction *fileOpen= new QAction (tr ("&Open"), this);
	fileOpen->setStatusTip (tr ("Open Waveform File"));
	connect (fileOpen, SIGNAL (triggered ()), this, SLOT (slotOpen()));

	QAction *fileSave= new QAction (tr ("&Save"), this);
	fileSave->setStatusTip (tr ("Save Waveform File"));
	connect (fileSave, SIGNAL (triggered ()), this, SLOT (slotSave()));

	/* we will only allow Optech to save the waveforms, this will not be included with the
	 customer version.  We will check for an environment variable called WAVEFORM_MONITOR_MODE */

	NV_CHAR * env;
	env = getenv ("WAVEFORM_MONITOR_MODE");

	if (!env || strcmp (env, "OPTECH") != 0) 
	  fileSave->setEnabled (NVFalse);


	QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
	fileMenu->addAction (filePrefsAction);
	fileMenu->addSeparator ();
	fileMenu->addAction (fileOpen);
	fileMenu->addAction (fileSave);
	fileMenu->addSeparator ();
	fileMenu->addAction (fileQuitAction);

	// 
	// We construct a View menu having the following menu choices on it:
	// Toggle Line - allows us to look at the waveforms in point or line mode
	// Intensity Meter - provides a cross section of all sensors along a bin
	// Zoom In - allows for a waveform signature zoom
	// Reset Zoom - brings us back out to the original waveform
	// All connections for these menu items are set up here.
	//


	QAction * viewLineAct = new QAction (tr ("Toggle &Line"), this);
	viewLineAct->setStatusTip ("Toggle the waveform display");
	connect (viewLineAct, SIGNAL (triggered()), this, SLOT (slotMode()));

	QAction * viewIntMeterAct = new QAction (tr ("Toggle &Intensity Meter"), this);
	viewIntMeterAct->setStatusTip ("Toggle the intensity meter");
	connect (viewIntMeterAct, SIGNAL (triggered()), this, SLOT (slotIntMeter()));


	// Here, we create a signal mapper who takes in a collection of signals that will go to the
	// same place.  The argument to the mutual slot will be the widget and that mapping is achieved
	// with the setMapping method.  The slot is slotPaneToggle ().  We also set the checkboxes of
	// the menu actions based on the saved settings of the last session.

	QSignalMapper * signalMapper = new QSignalMapper (this);

	// deep pane mapping

	QAction * deepChanAct = new QAction (tr ("Deep Channel Pane"), this);
	deepChanAct->setStatusTip ("Hide/Show Deep Channel Pane");
	deepChanAct->setCheckable (true);
	deepChanAct->setChecked (savedSettings.visible[0]);

	connect(deepChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));

	signalMapper->setMapping(deepChanAct, deepChanAct);

	// shallow 1 pane mapping

	QAction * shal1ChanAct = new QAction (tr ("Shallow 1 Channel Pane"), this);
	shal1ChanAct->setStatusTip ("Hide/Show Shallow 1 Channel Pane");
	shal1ChanAct->setCheckable (true);  
	shal1ChanAct->setChecked (savedSettings.visible[1]);

	connect(shal1ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));

	signalMapper->setMapping(shal1ChanAct, shal1ChanAct);

	// shallow2 pane mapping

	QAction * shal2ChanAct = new QAction (tr ("Shallow 2 Channel Pane"), this);
	shal2ChanAct->setStatusTip ("Hide/Show Shallow 2 Channel Pane");
	shal2ChanAct->setCheckable (true);
	shal2ChanAct->setChecked (savedSettings.visible[2]);

	connect(shal2ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));  
	signalMapper->setMapping(shal2ChanAct, shal2ChanAct);

	// shallow3 pane mapping

	QAction * shal3ChanAct = new QAction (tr ("Shallow 3 Channel Pane"), this);
	shal3ChanAct->setStatusTip ("Hide/Show Shallow 3 Channel Pane");
	shal3ChanAct->setCheckable (true);
	shal3ChanAct->setChecked (savedSettings.visible[3]);

	connect(shal3ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(shal3ChanAct, shal3ChanAct);
		
	// shallow4 pane mapping

	QAction * shal4ChanAct = new QAction (tr ("Shallow 4 Channel Pane"), this);
	shal4ChanAct->setStatusTip ("Hide/Show Shallow 4 Channel Pane");
	shal4ChanAct->setCheckable (true);
	shal4ChanAct->setChecked (savedSettings.visible[4]);

	connect(shal4ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(shal4ChanAct, shal4ChanAct);
	
	// shallow5 pane mapping

	QAction * shal5ChanAct = new QAction (tr ("Shallow 5 Channel Pane"), this);
	shal5ChanAct->setStatusTip ("Hide/Show Shallow 5 Channel Pane");
	shal5ChanAct->setCheckable (true);
	shal5ChanAct->setChecked (savedSettings.visible[5]);

	connect(shal5ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(shal5ChanAct, shal5ChanAct);
	
	// shallow6 pane mapping

	QAction * shal6ChanAct = new QAction (tr ("Shallow 6 Channel Pane"), this);
	shal6ChanAct->setStatusTip ("Hide/Show Shallow 6 Channel Pane");
	shal6ChanAct->setCheckable (true);
	shal6ChanAct->setChecked (savedSettings.visible[6]);

	connect(shal6ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(shal6ChanAct, shal6ChanAct);
	
	// shallow7 pane mapping

	QAction * shal7ChanAct = new QAction (tr ("Shallow 7 Channel Pane"), this);
	shal7ChanAct->setStatusTip ("Hide/Show Shallow 7 Channel Pane");
	shal7ChanAct->setCheckable (true);
	shal7ChanAct->setChecked (savedSettings.visible[7]);

	connect(shal7ChanAct, SIGNAL(triggered()), signalMapper, SLOT(map()));
	signalMapper->setMapping(shal7ChanAct, shal7ChanAct);

	// ir pane mapping

	QAction * irSensorAct = new QAction (tr ("IR Channel Pane"), this);
	irSensorAct->setStatusTip ("Hide/Show IR Channel Pane");
	irSensorAct->setCheckable (true);
	irSensorAct->setChecked (savedSettings.visible[8]);

	connect(irSensorAct, SIGNAL(triggered()), signalMapper, SLOT(map()));

	signalMapper->setMapping(irSensorAct, irSensorAct);	

	connect(signalMapper, SIGNAL(mapped(QObject *)), this, SLOT(slotPaneToggle (QObject *))); 

	QAction * viewZoomInAct = new QAction (tr ("&Zoom In"), this);
	viewZoomInAct->setStatusTip ("Zoom in on a waveform");
	connect (viewZoomInAct, SIGNAL (triggered ()), this, SLOT (slotZoomIn ()));

	QAction * viewResetZoomAct = new QAction (tr ("&Reset Zoom"), this);
	viewResetZoomAct->setStatusTip ("Reset all zooms");
	connect (viewResetZoomAct, SIGNAL (triggered ()), this, SLOT (slotResetZoom ()));


	QMenu * viewMenu = menuBar()->addMenu (tr ("&View"));
	viewMenu->addAction (viewLineAct);
	viewMenu->addAction (viewIntMeterAct);
	viewMenu->addSeparator();

	// Add the pane display toggle menu items
	
	/* chl revision 08_29_2011 */
	
	QMenu * togglePaneMenu = new QMenu (tr ("Pane Visibility"));

	// viewMenu->addAction (deepChanAct);
	// viewMenu->addAction (shal1ChanAct);	
	// viewMenu->addAction (shal2ChanAct);
	// viewMenu->addAction (shal3ChanAct);

	// viewMenu->addAction (shal4ChanAct);
	// viewMenu->addAction (shal5ChanAct);
	// viewMenu->addAction (shal6ChanAct);
	// viewMenu->addAction (shal7ChanAct);
		
	// viewMenu->addAction (irSensorAct);
	
	togglePaneMenu->addAction (deepChanAct);
	togglePaneMenu->addAction (shal1ChanAct);	
	togglePaneMenu->addAction (shal2ChanAct);
	togglePaneMenu->addAction (shal3ChanAct);

	togglePaneMenu->addAction (shal4ChanAct);
	togglePaneMenu->addAction (shal5ChanAct);
	togglePaneMenu->addAction (shal6ChanAct);
	togglePaneMenu->addAction (shal7ChanAct);
		
	togglePaneMenu->addAction (irSensorAct);
	
	viewMenu->addMenu (togglePaneMenu);
	
	/* end revision */

	viewMenu->addSeparator();  

	viewMenu->addAction (viewZoomInAct);
	viewMenu->addAction (viewResetZoomAct);


	// 
	// Here, we set up the Multi-Wave menu which has the modes for 
	// looking at the waveforms.  These options include:
	// Nearest Neighbor - shows the MAX_STACK_POINTS - 1 nearest waveforms
	// Single Waveform - only shows what's under the cursor in pfmEdit
	//

	nearAct = new QAction (tr ("Nearest Neighbors"), this);
	nearAct->setStatusTip ("Shows the eight closest waveforms");

	nearAct->setCheckable (true);  

	connect (nearAct, SIGNAL (triggered()), this, SLOT (slotNear()));

	noneAct = new QAction (tr ("Single Waveform Mode"), this);
	noneAct->setStatusTip ("Only shows the targeted waveform");
	noneAct->setCheckable (true);
	connect (noneAct, SIGNAL (triggered()), this, SLOT (slotNone()));

	QActionGroup * multiWaveGrp = new QActionGroup (this);

	multiWaveGrp->addAction (nearAct);   
	multiWaveGrp->addAction (noneAct);

	if (multiWaveMode == 1)
	{
	  noneAct->setChecked (true);
	}
	else
	{	  
	  nearAct->setChecked (true);	  
	}

	QMenu *multiMenu = menuBar ()->addMenu (tr ("&Multi-Wave"));
	multiMenu->addAction (nearAct);
	multiMenu->addAction (noneAct);

	// Create the Tools menu and color coding submenu.  On the submenu are the option to color
	// code by cursor or flightline.  The options on the submenu are part of an action group meaning
	// they are mutually exclusive.

	QMenu * toolsMenu = menuBar()->addMenu (tr ("&Tools"));

	QMenu * colorCodingMenu = new QMenu (tr ("Color Coding"));

	QAction * ccByCursor = new QAction (tr ("By Cursor"), this);
	ccByCursor->setCheckable (true);
	ccByCursor->setStatusTip (tr ("Color codes waveforms based on pfmEdit's cursor colors"));
	connect (ccByCursor, SIGNAL (triggered()), this, SLOT (slotColorByCursor()));

	QAction * ccByFlightline = new QAction (tr ("By Flightline"), this);
	ccByFlightline->setCheckable (true);
	ccByFlightline->setStatusTip (tr ("Color codes waveforms based on flighline color"));
	connect (ccByFlightline, SIGNAL (triggered()), this, SLOT (slotColorByFlightline()));

	QActionGroup * ccGroup = new QActionGroup (this);
	ccGroup->addAction (ccByCursor);
	ccGroup->addAction (ccByFlightline);  

	ccMode ? ccByFlightline->setChecked (true) : ccByCursor->setChecked (true);

	colorCodingMenu->addAction (ccByCursor);
	colorCodingMenu->addAction (ccByFlightline);

	toolsMenu->addMenu (colorCodingMenu);  

	// place the Reference Marks menu under the Tools menu option

	QMenu * referenceMenu = new QMenu (tr ("Reference Waveforms"));

	toggleReference = new QAction (tr("Toggle Overlay Display"), this);
	toggleReference->setCheckable (true);
	toggleReference->setChecked (false);
	toggleReference->setStatusTip (tr ("Shows/hides the reference waveforms of an opened .wfm file"));
	toggleReference->setEnabled (false);
	connect (toggleReference, SIGNAL (toggled (bool)), this, SLOT (slotToggleReference (bool)));

	clearReference = new QAction (tr("Clear Reference Marks"), this);
	clearReference->setStatusTip (tr ("Clears the reference waveforms from memory"));
	clearReference->setEnabled (false);
	connect (clearReference, SIGNAL (triggered ()), this, SLOT (slotClearReference ()));

	referenceMenu->addAction (toggleReference);
	referenceMenu->addAction (clearReference);

	toolsMenu->addMenu (referenceMenu);  

	//  Setup the help menu.  I like leaving the About Qt in since this is
	//  a really nice package - and it's open source.

	QAction *aboutAct = new QAction (tr ("&About"), this);
	aboutAct->setShortcut (tr ("Ctrl+A"));
	aboutAct->setStatusTip (tr ("Information about CZMILwaveMonitor"));
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

	for (NV_INT32 i = 0 ; i < NUM_WAVEFORMS ; i++)
	{
	  map[i]->setCursor (Qt::ArrowCursor);
	  map[i]->enableSignals ();	 

	  /* chl revision 08_23_2011 */
	  
	  chanBounds[i].length = UPPER_TIME_BOUND + 1;	

	  chanBounds[i].min_y = 0;
	  chanBounds[i].max_y = 1023;
					
	  chanBounds[i].max_x = chanBounds[i].length;
	  chanBounds[i].min_x = 0;
					
      chanBounds[i].range_x = chanBounds[i].max_x - chanBounds[i].min_x;
					
	  //  Add 5% to the X axis.
					
	  // 
	  //  Here, depending on which layout of the panes we are using (vertical/
	  //  horizontal), we calculate the padding before we get to the axes.
      //
					
	  chanBounds[i].max_x = chanBounds[i].max_x + NINT (((NV_FLOAT32) chanBounds[i].range_x * 0.05 + 1));
	  chanBounds[i].range_x = chanBounds[i].max_x - chanBounds[i].min_x;
					
	  chanBounds[i].range_y = chanBounds[i].max_y - chanBounds[i].min_y;	  
	  
	  /* end revision */
	} 

	 QTimer * track = new QTimer (this);
	 connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
	 track->start (10);  	  
}



CZMILwaveMonitor::~CZMILwaveMonitor ()
{
}


// This slot is attached to the pane display signal mapper and will dynamically hide/
// display panes

void CZMILwaveMonitor::slotPaneToggle (QObject * id) {

	QAction * realID = (QAction *) id;
	int deltaSize, windowOffset, totalCurHeight;
	int row1Height, row2Height, row3Height;

	deltaSize = totalCurHeight = 0;
	
	row1Height = map[0]->size().rheight();
	
	for (NV_INT32 i = 0; i < 3; i++) {
		if (map[i]->isVisible()) {
			row1Height = map[i]->size().rheight();
			break;
		}
	}
	
	row2Height = map[3]->size().rheight();
	
	for (NV_INT32 i = 3; i < 6; i++) {
		if (map[i]->isVisible()) {
			row2Height = map[i]->size().rheight();
			break;
		}
	}
	
	row3Height = map[6]->size().rheight();
	
	for (NV_INT32 i = 6; i < 8; i++) {
		if (map[i]->isVisible()) {
			row3Height = map[i]->size().rheight();
			break;
		}
	}	
	
	if (map[0]->isVisible() || map[1]->isVisible() || map[2]->isVisible()) 
		totalCurHeight += row1Height;
		
	if (map[3]->isVisible() || map[4]->isVisible() || map[5]->isVisible()) 
		totalCurHeight += row2Height;
		
	if (map[6]->isVisible() || map[7]->isVisible() || map[8]->isVisible()) 
		totalCurHeight += row3Height;	

	windowOffset = this->size().rheight() - totalCurHeight;	

	/* changed fixed for new indexing (swapped IR and TBD */	

	if (realID->text () == "Deep Channel Pane") map[0]->setVisible (realID->isChecked ());
	else if (realID->text () == "Shallow 1 Channel Pane") map[1]->setVisible (realID->isChecked ()); 
	else if (realID->text () == "Shallow 2 Channel Pane") map[2]->setVisible (realID->isChecked ());
	else if (realID->text () == "Shallow 3 Channel Pane") map[3]->setVisible (realID->isChecked ()); 
	else if (realID->text () == "Shallow 4 Channel Pane") map[4]->setVisible (realID->isChecked ());		
	else if (realID->text () == "Shallow 5 Channel Pane") map[5]->setVisible (realID->isChecked ());		
	else if (realID->text () == "Shallow 6 Channel Pane") map[6]->setVisible (realID->isChecked ());		
	else if (realID->text () == "Shallow 7 Channel Pane") map[7]->setVisible (realID->isChecked ());		
	else if (realID->text () == "IR Channel Pane") map[8]->setVisible (realID->isChecked ());			
	
	if (map[0]->isVisible () || map[1]->isVisible() || map[2]->isVisible()) 
		deltaSize += row1Height;
		
	if (map[3]->isVisible() || map[4]->isVisible() || map[5]->isVisible())
		deltaSize += row2Height;
		
	if (map[6]->isVisible() || map[7]->isVisible() || map[8]->isVisible())
		deltaSize += row3Height;	

	this->resize(this->size().rwidth(), windowOffset + deltaSize);
}




//
// Slot:		slotToggleReference
//
// This slot will be called automagically when the user selects the "Toggle Overlay" option from
// the menu.  It will set the key flags or variables to enable the reference waveforms to be 
// shown or hidden within the sensor panes.
//

void CZMILwaveMonitor::slotToggleReference (bool toggleStatus) {

	drawReference = toggleStatus;		
	
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) 
		if (map[i]->isVisible()) 
			map[i]->redrawMapArea (NVTrue);	
}

//
// Slot:		slotClearReference 
//
// This slot will set any needed flags to halt the drawing of the reference waveforms and will
// clean up any memory structures that housed the reference marks.
//

void CZMILwaveMonitor::slotClearReference () {	

	// uncomment this since I don't have this function over here.
	clearReferenceData ();	
	
	for (int i = 0; i < NUM_WAVEFORMS; i++) 
		if (map[i]->isVisible()) 
			map[i]->redrawMapArea (NVTrue);
	
	toggleReference->setChecked (false);
	toggleReference->setEnabled (false);
	clearReference->setEnabled (false);
}



// 
// slot:        slotNone
//                                        
// This slot will fire when the Single-Waveform mode is selected off of
// the menu.  We redraw the CZMILwaveMonitor module to reflect this change.
//

void CZMILwaveMonitor::slotNone ()
{
  multiWaveMode = 1;

  abe_share->mwShare.multiSwitch = NVTrue;
  abe_share->mwShare.multiMode = multiWaveMode;
  force_redraw = NVTrue;
}



// 
// slot:        slotNear
//                                        
// This slot will fire when the Nearest Neighbor mode is selected off of
// the menu.  We redraw the CZMILwaveMonitor module to reflect this change.
//

void CZMILwaveMonitor::slotNear ()
{
  multiWaveMode = 0;
  abe_share->mwShare.multiSwitch = NVTrue;
  abe_share->mwShare.multiMode = multiWaveMode;
  force_redraw = NVTrue;
}



// There are two slots defined here that are fired when the user selects one of the options off
// of the Color Coding submenu on the Tools menu.
//

void CZMILwaveMonitor::slotColorByCursor (void) {

	ccMode = CC_BY_CURSOR;
	nonFileIO_force_redraw = NVTrue;
}


void CZMILwaveMonitor::slotColorByFlightline (void) {

	ccMode = CC_BY_FLIGHTLINE;
	nonFileIO_force_redraw = NVTrue;
}


//
// event:        closeEvent
//
// This event will be called when the CZMILwaveMonitor module is closed using the stupid
// little X button.
//

void CZMILwaveMonitor::closeEvent (QCloseEvent * event __attribute__ ((unused)))
{
  slotQuit ();
}


//
// method:        GetLabelSpacing
//
// This function will create fonts that are to be used in the different
// sensor panes to denote the tick marks and legend labels.  We calculate
// the width and height of a Charter font of points 12 and 8.  We also calculate
// the amount of space in pixels to get to either the Time or Count axis.
// Lastly, we calculate the width of the text "Time (ns)" and "Counts".  All
// of these values will be used at a later time to build the individual
// panes.
//

void
CZMILwaveMonitor::GetLabelSpacing (void)
{
  QFont font ("Charter", 12);
  QFont font2 ("Charter", 8);

  QFontMetrics fm (font);
  QFontMetrics fm2 (font2);

  pt12Height = fm.height();
  pt8Height = fm2.height();

  verLblSpace = LBL_PIX_SPACING + pt12Height + LBL_PIX_SPACING + pt8Height + LBL_PIX_SPACING;

  pt12Width = fm.width ("X");
  gridWidth = fm2.width ("1024");

  horLblSpace = LBL_PIX_SPACING + pt12Width + LBL_PIX_SPACING + gridWidth + LBL_PIX_SPACING;

  timeWidth = fm.width ("Time (ns)");
  countsWidth = fm.width ("Counts");
}


  

void 
CZMILwaveMonitor::slotResize (QResizeEvent *e __attribute__ ((unused)))
{

  //
  // Here, we find the offsets pixelwise as to where the beginning and
  // end of our Time axis is.  These values get placed in sigBegin and
  // in sigEnd.
  //

  NV_INT32 dummy;
  
  // here, we grap the sening map and store the new begin/end axis position in pixels for each
  // sensor map

  nvMap * sendingMap = (nvMap *) sender ();  

  NV_INT32 mapType = -1;
  
  // we always want to get the axisPixelBegin/End combo based on the full extent of the axis no
  // matter if we are zooming or not.  We are only concerned with the pixel locations and they
  // must be consistent.  Therefore, we don't play off of the current lTimeBound/uTimeBound arrays,
  // we play off of a nonzoomed axis.  

  NV_INT32 lCount, lTime, uTime;
  
  // change lTime to 1 for proper scaling.  there is no time bin of 0.

  lTime = LOWER_TIME_BOUND;
  lCount = LOWER_COUNT_BOUND;

  mapType = GetSensorMapType(sendingMap);	
  
  uTime = UPPER_TIME_BOUND;  
	  
  scaleWave (lTime, lCount, &(axisPixelBegin[mapType]), &dummy, mapType, sendingMap->getMapdef());
  scaleWave (uTime, lCount, &(axisPixelEnd[mapType]), &dummy, mapType, sendingMap->getMapdef()); 
  
  // Because when we resize we can't be in another window (that's an assumption), we can call
  // the panes redrawMapArea  
  
  sendingMap->redrawMapArea (NVTrue); 
}



void
CZMILwaveMonitor::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}


//
// method:        clampToLegend
//
// This function handles unruly users who try to zoom into places outside
// of the waveform signature.
//

void CZMILwaveMonitor::clampToLegend(NV_INT32 zoomLength, NV_INT32 &timeBin, NV_INT32 &timeBin2, NV_INT32 &timeBin3, NV_INT32 &timeBin4)
{
  // if the user selects a region outside of the waveform signature area, reset them
  // to the full extents

  	if ((timeBin < LOWER_TIME_BOUND && timeBin2 < LOWER_TIME_BOUND) || 
		(timeBin3 < LOWER_COUNT_BOUND && timeBin4 < LOWER_COUNT_BOUND) ||
        (timeBin > zoomLength && timeBin2 > zoomLength) || 
		(timeBin3 > UPPER_COUNT_BOUND && timeBin4 > UPPER_COUNT_BOUND))
    {
      timeBin = LOWER_TIME_BOUND;
      timeBin2= zoomLength-1;
      timeBin3= LOWER_COUNT_BOUND;
      timeBin4= UPPER_COUNT_BOUND;
    }

  // check individual out of bounds and bring them back to the boundary

  else
    {
      if (timeBin < LOWER_TIME_BOUND) timeBin = LOWER_TIME_BOUND;
      if (timeBin > zoomLength)  timeBin = zoomLength-1;
      if (timeBin2 > zoomLength) timeBin2 = zoomLength-1;
      if (timeBin2 < LOWER_TIME_BOUND) timeBin2 = LOWER_TIME_BOUND;
      if (timeBin3 < LOWER_COUNT_BOUND) timeBin3 = LOWER_COUNT_BOUND;
      if (timeBin3 > UPPER_COUNT_BOUND) timeBin3 = UPPER_COUNT_BOUND;
      if (timeBin4 < LOWER_COUNT_BOUND) timeBin4 = LOWER_COUNT_BOUND;
      if (timeBin4 > UPPER_COUNT_BOUND) timeBin4 = UPPER_COUNT_BOUND;
    }	
}



//
// method:        storeZoomBounds
//
// In this method, zoomLength represents the Time axis extent.  Currently,
// the time axis of the CZMIL simulated data set is 64.  We then proceed to
// get the zoom boundaries in our unwavering coordinate system (1-zoomLength)
// along Time; 0-1023 along Counts.  We order the points so timeBin1 (llX), 
// timeBin2(urX), timeBin3 (llY) and timeBin4 (urY).  Then based on the current 
// boundaries of each pane stored in lTimeBound, uTimeBound, lCountBound, and 
// uCountBound, we take the ratio in our unwavering coordinates and apply it 
// to the zoomed coordinates to get new zoom coordinates that we overwrite into 
// lTimeBound, uTimeBound, lCountBound, and uCountBound.  The indexes into these
// arrays refer to the sensors themselves.
//

void CZMILwaveMonitor::storeZoomBounds(nvMap * l_map, NV_INT32 x, NV_INT32 y)
{
	
	NV_INT32 timeLegendBegin, timeLegendEnd, countLegendBegin, countLegendEnd, zoomLength, 
           timeBin, timeBin2, timeBin3, timeBin4 = 0;
  
	//Determine which map is passed in and get its length for the Time axis.
	
	NV_INT32 mi;		
	
	for (mi = 0; mi < NUM_WAVEFORMS; mi++) {
		if (l_map == map[mi]) break;
	}
	
	zoomLength = chanBounds[mi].length;
		
	// get the corner points of the selection box

	scaleWave (LOWER_TIME_BOUND, LOWER_COUNT_BOUND, &timeLegendBegin, &countLegendBegin, mi, 
				l_map->getMapdef ());
				
	scaleWave (zoomLength-1, UPPER_COUNT_BOUND, &timeLegendEnd, &countLegendEnd, mi, l_map->getMapdef());
	
	timeBin = NINT ((zoomFirstPos.x() - timeLegendBegin) / 
			  (NV_FLOAT32)(timeLegendEnd - timeLegendBegin) * (zoomLength - 2)) + 1;
	timeBin2 = NINT ((x - timeLegendBegin) / 
			   (NV_FLOAT32)(timeLegendEnd - timeLegendBegin) * (zoomLength - 2)) + 1;
			   
	timeBin3 = NINT ((zoomFirstPos.y() - countLegendBegin) / 
			   (NV_FLOAT32)(countLegendEnd - countLegendBegin) * UPPER_COUNT_BOUND) + 1;
	timeBin4 = NINT ((y - countLegendBegin) / 
			   (NV_FLOAT32)(countLegendEnd - countLegendBegin) * UPPER_COUNT_BOUND) + 1;
			   
	clampToLegend(zoomLength, timeBin, timeBin2, timeBin3, timeBin4);


	// after reordering LL = (timeBin, timeBin3) UR = (timeBin2, timeBin4)

	if (timeBin > timeBin2)
	{
		NV_INT32 temp;
		temp = timeBin2;
		timeBin2 = timeBin;
		timeBin = temp;
	}
	if (timeBin3 > timeBin4)
	{
		NV_INT32 temp;
		temp = timeBin4;
		timeBin4 = timeBin3;
		timeBin3 = temp;
	}


	// here is the extracted function call
	setZoomBounds( mi, zoomLength, timeBin, timeBin2, timeBin3, timeBin4 );
}



//

// Functionality extracted from previous version of storeZoomBounds.
// Function actually saves the calculated bounds into the time and count bound structures
void 
CZMILwaveMonitor::setZoomBounds ( NV_INT32 mapIndex, NV_INT32 zoomLength, NV_INT32 timeBin, NV_INT32 timeBin2, NV_INT32 timeBin3, NV_INT32 timeBin4  )
{
	// based on ratio of selection box, calculate new extents from current extents stored
	// in lTimeBound, uTimeBound, lCountBound, and uCountBound

	NV_INT32 timeTemp = lTimeBound[mapIndex], countTemp = lCountBound[mapIndex];

	NV_INT32 lTime = (NV_INT32)(timeTemp +(uTimeBound[mapIndex] -timeTemp +1)*((timeBin -1)/(NV_FLOAT32)(zoomLength - 1)));
	NV_INT32 uTime = (NV_INT32)(timeTemp +(uTimeBound[mapIndex] -timeTemp +1)*((timeBin2-1)/(NV_FLOAT32)(zoomLength - 1)));
	
	NV_INT32 lCount = (NV_INT32)(countTemp+(uCountBound[mapIndex]-countTemp+1)*
							(timeBin3/(NV_FLOAT32)(UPPER_COUNT_BOUND)));
							
	NV_INT32 uCount = (NV_INT32)(countTemp+(uCountBound[mapIndex]-countTemp+1)*
							(timeBin4/(NV_FLOAT32)(UPPER_COUNT_BOUND)));
							
	if (((uCount - lCount) > 1) && ((uTime - lTime) > 1)) {
		lTimeBound[mapIndex] = lTime;
		uTimeBound[mapIndex] = uTime;
		lCountBound[mapIndex] = lCount;
		uCountBound[mapIndex] = uCount;
	}

}



//
// method:        leftMouse
//
// This method is called when the left mouse button is clicked.  This
// function handles the actual zooming protocol.
//

void 
CZMILwaveMonitor::leftMouse (NV_INT32 x, NV_INT32 y, nvMap * l_map)
{

  // if we are zooming

  if (zoomFlag)
    {
      // if we have set our first pivot point, then we are finishing the zoom selection

      if (zoomIsActive)
        {
          // we make sure we're in the proper pane

          if (l_map == zoomMap)
            {
              // pass in our second point, turn zooming off and redraw the map
              // to get the updated view

              storeZoomBounds(l_map, x, y);
              zoomIsActive = NVFalse;
              zoomFlag = NVFalse;			  

			  l_map->redrawMapArea (NVTrue);
            }
        }

      // we are selecting our first point of the zoom selection

      else
        {
          // store the map we are in
                  
          zoomMap = l_map;

          // ready for the second point, turn the flag on and store the first point
          // in zoomFirstPos

          zoomIsActive = NVTrue;
          zoomFirstPos = zoomSecondPos = QPoint(x, y);
        }
    }
}


void 
CZMILwaveMonitor::midMouse (NV_FLOAT64 x __attribute__ ((unused)), NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Placeholder
}




// modified right click to store current zooming pane
void CZMILwaveMonitor::rightMouse (NV_INT32 x, NV_INT32 y, nvMap * l_map)
{

  if (!zoomIsActive) {

    contextMenu->move(x,y);
    contextMenu->show();
	zoomOutMap = l_map;
  }
}



void 
CZMILwaveMonitor::slotMousePress (QMouseEvent * e __attribute__ ((unused)), NV_FLOAT64 x __attribute__ ((unused)),
                             NV_FLOAT64 y __attribute__ ((unused)))
{
  
  nvMap *l_map = (nvMap *) sender ();  

  if (e->button () == Qt::LeftButton) leftMouse (e->pos().x(), e->pos().y(), l_map);
  if (e->button () == Qt::MidButton) midMouse (x, y);  	
  if (e->button () == Qt::RightButton) rightMouse (e->globalX(), e->globalY(), l_map);
}

// 
// method:        loadRectangleCoords
//
// This function is currently not being used but what it does is create
// all four coordinates of the zoom selection box.
//

QPoint * CZMILwaveMonitor::loadRectangleCoords(QPoint zoomSecondPos)
{
  static QPoint box[4];

  if (zoomFirstPos.x() < zoomSecondPos.x())
    {
      box[0].setX(zoomFirstPos.x());
      box[1].setX(zoomFirstPos.x());
      box[2].setX(zoomSecondPos.x());
      box[3].setX(zoomSecondPos.x());
    }
  else
    {
      box[0].setX(zoomSecondPos.x());
      box[1].setX(zoomSecondPos.x());
      box[2].setX(zoomFirstPos.x());
      box[3].setX(zoomFirstPos.x());
    }
  if (zoomFirstPos.y() < zoomSecondPos.y())
    {
      box[0].setY(zoomFirstPos.y());
      box[3].setY(zoomFirstPos.y());
      box[1].setY(zoomSecondPos.y());
      box[2].setY(zoomSecondPos.y());
    }
  else
    {
      box[1].setY(zoomFirstPos.y());
      box[2].setY(zoomFirstPos.y());
      box[0].setY(zoomSecondPos.y());
      box[3].setY(zoomSecondPos.y());
    }
  return &(box[0]);
}




//
// Method:				GetSensorMapType
//
// This method will ingest a map in question and compare it against the sensor array and
// return the sensor type as an integer.  The outgoing value will either be PMT, APD, TBDD,
// RAMAN, or IR
//

NV_INT32 CZMILwaveMonitor::GetSensorMapType (nvMap * mapInQuestion) {

    if (mapInQuestion == map[0]) return CZMIL_DEEP;
	else if (mapInQuestion == map[1]) return CZMIL_SHALLOW1;
	else if (mapInQuestion == map[2]) return CZMIL_SHALLOW2;
	else if (mapInQuestion == map[3]) return CZMIL_SHALLOW3;
	else if (mapInQuestion == map[4]) return CZMIL_SHALLOW4;
	else if (mapInQuestion == map[5]) return CZMIL_SHALLOW5;
	else if (mapInQuestion == map[6]) return CZMIL_SHALLOW6;
	else if (mapInQuestion == map[7]) return CZMIL_SHALLOW7;
	else return CZMIL_IR;	
}



//
// slotMouseMove
//
// This function will be called when a mouse is moved across a map pane.
// If we are zooming, we are updating the position of the cursor in zoomSecondPos.
// We also may be showing the intensity meter so we capture which time 
// bin we are on based on the difference between the beginning of the axis
// and the pixel we are currently on.
//
void
CZMILwaveMonitor::slotMouseMove (QMouseEvent *e, NV_FLOAT64 x __attribute__ ((unused)), 
                            NV_FLOAT64 y __attribute__ ((unused)))
{
  //  Let other ABE programs know which window we're in.

  abe_share->active_window_id = active_window_id;


  if (zoomFlag && zoomIsActive)
    {
      zoomSecondPos = e->pos();
      
	  
	  // we set all sensor maps to being blocked and only release the sensor map that
	  // we are working on.  Therefore, in slotTrackCursor, only the released map will
	  // get processed.  We also take it on the non-file I/O route so it will be quick
	  // to update

	  for (int i = 0; i < 5; i++) mapBlocked[i] = NVTrue;
	  
	  mapBlocked[GetSensorMapType (zoomMap)] = NVFalse;
	  nonFileIO_force_redraw = NVTrue;	  
    }
  else
    { 
      if (showIntMeter)
        {
		  
		    // grab the sending map, store the mapType as an index and the length so we don't have
			// to duplicate code.  get the binHighlighted value.  This value represents a location
			// on the time axis which is sensor dependent.  For example on the IR axis this value
			// could be within the range of 0-200.

			nvMap *sendingMap = (nvMap *) (sender ());

			NV_INT32 mapType, length = 0;
			
			// We get the sensor type, store the length respectively and block all other maps with
			// the exception of the sendingMap.  Thus, we will process only sendingMap in slotTrackCursor
			//

			mapType = GetSensorMapType (sendingMap);

			length = chanBounds[mapType].length;

		    binHighlighted[mapType] = NINT ((e->x() - axisPixelBegin[mapType]) / (NV_FLOAT32)(axisPixelEnd[mapType] - axisPixelBegin[mapType]) * (length - 2)) + 1;
			
		    // block all maps except the sending map, take the nonFileIO route in trackCursor
			
			for (int i = 0; i < NUM_WAVEFORMS; i++) mapBlocked[i] = NVTrue;
						
		    mapBlocked[mapType] = NVFalse;
		  
		    nonFileIO_force_redraw = NVTrue;		  
        }  
    }
}

//
// method:        drawMultiGrid
//
// This method will print the intensity information for all multiple
// waveforms in a grid-like pattern in the top right of the screen.
//

/* chl revision 08_24_2011 */
//void CZMILwaveMonitor::drawMultiGrid(NV_INT16 multiWave[][64], NVMAP_DEF l_mapdef, nvMap * l_map)
void CZMILwaveMonitor::drawMultiGrid(CZMIL_CWF_Data multiWave[], NVMAP_DEF l_mapdef, nvMap * l_map)
/* end revision */

{ 
  
  NV_INT32 colStart = l_mapdef.draw_width - 110;
  NV_INT32 rowStart = 60;
  QColor tempColor;
  QString tempString;
  NV_INT32 lTime, uTime;  
	
  // reduced code down to for loop and defaulted mapIndex to 0

  NV_INT32 mapIndex = 0;  
  
  for (int i = 1; i < NUM_WAVEFORMS; i++ ) 
  
	if ( l_map == map[i] ) { 
		mapIndex = i; 
		break;
	}
	
  lTime = lTimeBound[mapIndex];  
  uTime = uTimeBound[mapIndex];	
  
  for (NV_INT32 i = 0 ; i < 3 ; i++)
    {
      for (NV_INT32 j = 0 ; j < 3 ; j++)
        {
          if (abe_share->mwShare.multiPresent[j + (i * 3)] != -1)
            {
              tempColor.setRgb (abe_share->mwShare.multiColors[j + (i * 3)].r,abe_share->mwShare.multiColors[j + (i * 3)].g,
                                abe_share->mwShare.multiColors[j + (i * 3)].b);
              tempColor.setAlpha (abe_share->mwShare.multiColors[j + (i * 3)].a);			  

			   if (scaledBinHighlighted[mapIndex] <=uTime && scaledBinHighlighted[mapIndex] >=lTime)
                {
				
				  /* chl revision 08_24_2011 */
				  
				  NV_U_INT16 * wData;
				  NV_U_BYTE * ndx;
				  
				  getWaveformData (mapIndex, wData, ndx, &(multiWave[j + (i * 3)]));
				  
				  //tempString.sprintf("%d", multiWave[j+(i*3)][scaledBinHighlighted[mapIndex]]);				  
				  tempString.sprintf("%d", wData[scaledBinHighlighted[mapIndex]]);				  
				  
				  /* end revision */
                }
              else
                {
                  tempString.sprintf("N/A");
                }
				
              l_map->drawText(tempString, colStart + (j*35), rowStart + (i*20), tempColor, NVTrue);
            }
        }
    } 
}




// function revamped to account for reference lines
//
// method:        drawMultiWaves
//
// This method draws mulitple waveforms.  This code was "borrowed" from
// Jan's function that draws a single waveform.  We'll go in deeper within
// the function.
//

void CZMILwaveMonitor::drawMultiWaves (nvMap *l_map, NVMAP_DEF l_mapdef)
{
	// we allocate space for czmil information for the
	// maximum number of waveforms
	
	/* chl revision 08_24_2011 */
	NV_U_INT16 * dat[MAX_STACK_POINTS];
	NV_U_BYTE * ndx[MAX_STACK_POINTS];
	/* end revision */

	static NV_INT32         save_rec_multi[MAX_STACK_POINTS];

	/* chl revision 08_24_2011 */
	//static NV_INT16		    save_wave_multi[MAX_STACK_POINTS][64];
	static CZMIL_CWF_Data	save_wave_multi[MAX_STACK_POINTS];
	/* end revision */
	
	static CZMIL_CXY_Data   save_czmil_multi[MAX_STACK_POINTS];	
	
	/* chl revision 08_24_2011 */
	//static QString          save_line_multi[MAX_STACK_POINTS];
	/* end revision */
	
	NV_INT32                pix_x[2], pix_y[2];
	QColor                  curWaveColorNear, curPrimaryColorNear, curSecondaryColorNear, tempColor;
	NV_INT32                wave_type;
	NV_INT32                lTime, uTime, lCount, uCount, length;
	NV_INT32                scaledTimeValue, scaledCountValue;
	NV_BOOL                 currentValid = NVFalse;
	NV_BOOL                 previousValid = NVFalse;


	// Determine what type of waveform is passed in.  we store the current boundaries
	// of the axes in lTime, uTime, lCount, uCount.  We could be in a zoomed state. We
	// also must store the length of the time bin in length since it varies based on 
	// sensor.

	NV_INT32 lengths[NUM_WAVEFORMS];
	
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) lengths[i] = chanBounds[i].length;
	
	wave_type = GetSensorMapType(l_map);
	length = lengths[wave_type];
	lTime = lTimeBound[wave_type];
	uTime = uTimeBound[wave_type];
	lCount = lCountBound[wave_type];
	uCount = uCountBound[wave_type];
	
	// we loop through the multiple waveforms and for any that exist, we store the sensor
	// information into save_wave_multi.  This information has been already processed in
	// processMultiWaves.  The czmil record, record number, and line information are also
	// stored.

	for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
	{
		/*		
		changed next line check from multiPresent to multiLocation == -1 to indicate no data
		*/
		if (abe_share->mwShare.multiLocation[i] != -1)
		{
			//  Because the trackCursor function may be changing the data while we're still plotting it we save it
			//  to this static structure;
			
			/* chl revision 08_24_2011 */
			
			// switch (wave_type) {
			
				// case CZMIL_DEEP:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].deep[j];
						
					// break;
					
				// case CZMIL_SHALLOW1:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[0][j];
						
					// break;
					
				// case CZMIL_SHALLOW2:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[1][j];
						
					// break;
					
				// case CZMIL_SHALLOW3:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[2][j];
						
					// break;
					
				// case CZMIL_SHALLOW4:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[3][j];
						
					// break;
					
				// case CZMIL_SHALLOW5:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[4][j];
						
					// break;
					
				// case CZMIL_SHALLOW6:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[5][j];
						
					// break;
					
				// case CZMIL_SHALLOW7:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].shallow[6][j];
						
					// break;
					
				// case CZMIL_IR:
					
					// for (NV_INT32 j = 0; j < 64; j++) 
						// save_wave_multi[i][j] = wave_data_multi[i].ir[j];
						
					// break;
			// }			
			
			save_wave_multi[i] = wave_data_multi[i];
			getWaveformData (wave_type, dat[i], ndx[i], &(save_wave_multi[i]));			
			
			save_czmil_multi[i] = czmil_record_multi[i];			
			save_rec_multi[i] = recnum_multi[i];
			//save_line_multi[i] = line_name_multi[i];
			
			/* end revision */
		}
		else
		{
			save_rec_multi[i] = -1;
		}
	}


	// if we have the intensity meter turned on, show the stats

	if (showIntMeter) drawMultiGrid (save_wave_multi, l_mapdef, l_map);
	
	//  Loop through the multiple waveforms skipping slots that are empty.
	//  Loop backwards so that the point of interest is plotted on top.

	for (NV_INT32 i = MAX_STACK_POINTS - 1 ; i >= 0 ; i--)
	{
		// if we have a valid record number

		if (save_rec_multi[i] != -1)
		{					
			
			// set tempColor equal to whatever respective cursor in pfmEdit is or set color equal to
			// flightline color

			if (ccMode == CC_BY_CURSOR) 
				tempColor.setRgb (abe_share->mwShare.multiColors[i].r, abe_share->mwShare.multiColors[i].g, 
					abe_share->mwShare.multiColors[i].b);
			else
				tempColor = colorArray[abe_share->mwShare.multiFlightlineIndex[i]];

			tempColor.setAlpha (abe_share->mwShare.multiColors[i].a);
	
			curWaveColorNear = tempColor;

			curPrimaryColorNear = transPrimaryColorNear;
			curSecondaryColorNear = transSecondaryColorNear;

			/* chl revision 08_24_2011 */
			//if (wave_read_multi[i])
			if (wave_read_multi[i] == CZMIL_SUCCESS)
			/* end revision */
			{
				
				/* chl revision 08_24_2011 */
				
				NV_INT32 ndxStart, ndxEnd;
				
				ndxStart = (lTime - 1) / 64;
				ndxEnd = (uTime - 1) / 64;				
				
				trimWaveformNdxs (ndx[i], ndxStart, ndxEnd);				
				
				/* end revision */
				
				currentValid = previousValid = NVFalse;
				
				/* chl revision 08_24_2011 */
				
				for (NV_INT32 x = ndxStart; x <= ndxEnd; x++) {
				
					if (!ndx[x]) {
						previousValid = NVFalse;
						continue;
					}
					
				/* end revision */

					// Here, we loop through the time axis and if the intensity values for the
					// time axis fall within the lCount-uCount range, we find the point projection
					// so we can later plot it.  We are also keeping track with the streak of valid
					// points since in a zoomed state there could be breaks in the line segments
					// as values fall in and out of the area.

					/* chl revision 08_24_2011 */
					
					//for (NV_INT32 j = lTime ; j <= uTime ; j++)
					for (NV_INT32 j = x * 64 ; j < (x * 64 + 64) ; j++)
					{

						//if ((save_wave_multi[i][j] >= lCount) && (save_wave_multi[i][j] <= uCount)) 
						if ((dat[i][j] >= lCount) && (dat[i][j] <= uCount) && (j >= lTime) && (j <= uTime)) 
						{
							currentValid = NVTrue;

							scaledTimeValue = (NV_INT32)(((j - lTime) / ((NV_FLOAT32)(uTime - lTime)) * (length - 2)) + 1);						
								
							// scaledCountValue = (NV_INT32)(( save_wave_multi[i][j] - lCount) / 
								// ((NV_FLOAT32)(uCount - lCount)) * UPPER_COUNT_BOUND);		

							scaledCountValue = (NV_INT32)(( dat[i][j] - lCount) / 
								((NV_FLOAT32)(uCount - lCount)) * UPPER_COUNT_BOUND);	
								
					/* end revision */

							scaleWave (scaledTimeValue, scaledCountValue, &pix_x[1], &pix_y[1], wave_type, l_mapdef);
						}
						else
						{
							currentValid = NVFalse;
						}


						// either make a line or draw a dot.  we will only draw a line if the
						// previous point was valid or within the current area

						if (wave_line_mode)
						{
							if (currentValid && previousValid)
							{
								l_map->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], curWaveColorNear, 2, NVFalse,
									Qt::SolidLine);
							}
						}
						else
						{
							if (currentValid) l_map->fillRectangle (pix_x[0], pix_y[0], SPOT_SIZE, SPOT_SIZE,
								curWaveColorNear, NVFalse);
						}

						pix_x[0] = pix_x[1];
						pix_y[0] = pix_y[1];
						previousValid = currentValid;
					}
					
				/* chl revision 08_24_2011 */
				}
				/* end revision */
			} 
		}
	}	

	/* chl revision 08_18_2011 */
	
	// sim_depth.sprintf ("%.2f", save_czmil_multi[0].deep.depth[0]);
	// sim_depth1.sprintf ("%.2f", save_czmil_multi[0].deep.depth[1]);
	// sim_depth2.sprintf ("%.2f", save_czmil_multi[0].deep.depth[2]);
	// sim_kgps_elev.sprintf ("%.2f", save_czmil_multi[0].deep.kgps_elevation);
	
	/* chl revision 08_26_2011 */
	
	// sim_depth.sprintf ("%.2f", save_czmil_multi[0].deep.return_elevation[1]);
	// sim_depth1.sprintf ("%.2f", save_czmil_multi[0].deep.return_elevation[2]);
	// sim_depth2.sprintf ("%.2f", save_czmil_multi[0].deep.return_elevation[3]);
	// sim_kgps_elev.sprintf ("%.2f", save_czmil_multi[0].deep.return_elevation[0]);
	
	sim_depth.sprintf ("%.2f", save_czmil_multi[0].channel[CZMIL_DEEP_CHANNEL].return_elevation[1]);
	sim_depth1.sprintf ("%.2f", save_czmil_multi[0].channel[CZMIL_DEEP_CHANNEL].return_elevation[2]);
	sim_depth2.sprintf ("%.2f", save_czmil_multi[0].channel[CZMIL_DEEP_CHANNEL].return_elevation[3]);
	sim_kgps_elev.sprintf ("%.2f", save_czmil_multi[0].channel[CZMIL_DEEP_CHANNEL].return_elevation[0]);
	
	/* end revision */
	
	/* end revision */
}



//
// method:        processMultiWaves
//
// This method will open the hof file and waveform files and load the
// data for each valid waveform.
//

void CZMILwaveMonitor::processMultiWaves (ABE_SHARE l_share)
{	

  NV_INT32 czmilHnd;  

  l_abe_share = *abe_share;

  // loop through the waveforms, the valid ones anyways

  for (NV_INT32 i = 0 ; i < MAX_STACK_POINTS ; i++)
    {
      //  Make sure this isn't an empty slot

	  if (l_abe_share.mwShare.multiLocation[i] != -1)
        {
            wave_read_multi[i] = 0;

            NV_INT16 type;
            read_list_file (pfm_handle[l_abe_share.mwShare.multiPfm[i]], 
							l_abe_share.mwShare.multiFile[i], czmil_path_multi[i], &type);
					
			/* incorporating PFM_CZMIL_DATA */
					
			if (type == PFM_CZMIL_DATA)					
            {         
              strcpy (line_name_multi[i], read_line_file (pfm_handle[l_abe_share.mwShare.multiPfm[i]],
                                                          l_abe_share.mwShare.multiLine[i]));

			  strcpy (wave_path_multi[i], czmil_path_multi[i]);
			  			
			  /* chl revision 08_18_2011 */
			  //CZMIL_Header czmilHeader;
			  CZMIL_CXY_Header czmilHeader;
			  /* end revision */
				
			  czmilHnd = czmil_open_file (czmil_path_multi[i], &czmilHeader, CZMIL_READONLY, NVTrue);

              if (czmilHnd < 0)
                {
                  if (filError) filError->close ();
                  filError = new QMessageBox (QMessageBox::Warning, tr ("Open CZMIL file"), tr ("Error opening ") + 
                                              QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
                                              QString (strerror (errno)), QMessageBox::NoButton, this, 
                                              (Qt::WindowFlags) Qt::WA_DeleteOnClose);
                  filError->show ();
                  return;
                }				

				
			  if (czmilHnd >= 0)
                {
                  //  Save for sWaves.
            
                  recnum_multi[i] = l_abe_share.mwShare.multiRecord[i];    
				  czmil_read_cxy_record (czmilHnd, recnum_multi[i], &(czmil_record_multi[i]));
				  
				  /* chl revision 08_24_2011 */
				  //wave_read_multi[i] = czmil_read_cwf_record (czmilHnd, l_abe_share.mwShare.multiRecord[i], &wave_data);            
				  wave_read_multi[i] = czmil_read_cwf_record (czmilHnd, l_abe_share.mwShare.multiRecord[i], &(wave_data_multi[i]));            
				  /* end revision */
                  
                  czmil_close_file (czmilHnd);
					
				  /* chl revision 08_24_2011 */
				  
                  //// load the waveform data depending for all sensors
				  
				  // for (NV_INT32 j = 0; j < 64; j++) {
				  
					// wave_data_multi[i].deep[j] = wave_data.deep[j];
					
					// for (NV_INT32 k = 0; k < 7; k++) 
						// wave_data_multi[i].shallow[k][j] = wave_data.shallow[k][j];
						
					// wave_data_multi[i].ir[j] = wave_data.ir[j];				  
				  // }	
				  
				  /* end revision */
				  
                }				
            }
        }
    }
}



//  Timer - timeout signal.  Very much like an X workproc.

void
CZMILwaveMonitor::trackCursor ()
{
  NV_INT32                year, day, mday, month, hour, minute;  
  NV_FLOAT32              second;
  static ABE_SHARE        l_share;    
  NV_INT32	czmilHnd;   
  static NV_BOOL          first = NVTrue;  

  //  We want to exit if we have locked the tracker to update our saved waveforms (in slotPlotWaves).

  if (lock_track) return;

  
  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
  //  has been placeed in abe_share->key.

  if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
      abe_share->key == kill_switch) slotQuit ();

  
  //  Check for action keys.

  if (abe_share->key == ac[0])
    {
      if (multiWaveMode)
	{
	  abe_share->key = 0;
	  abe_share->modcode = NO_ACTION_REQUIRED;

	  nearAct->setChecked (TRUE);
	  slotNear ();
	}
      else
	{
	  abe_share->key = 0;
	  abe_share->modcode = NO_ACTION_REQUIRED;

	  noneAct->setChecked (TRUE);
	  slotNone ();
	}
    }


  for (NV_INT32 i = 1 ; i < 6 ; i++)
     {
      if (abe_share->key == ac[i])
        {
          abe_share->key = 0;
          abe_share->modcode = NO_ACTION_REQUIRED;

          if (display_wave[i - 1])
            {
              display_wave[i - 1] = NVFalse;
            }
          else
            {
              display_wave[i - 1] = NVTrue;
            }

          force_redraw = NVTrue;

          break;
        }
    }


  if (abe_share->modcode == WAVEMONITOR_FORCE_REDRAW) force_redraw = NVTrue; 


  //  Check for CZMIL, no memory lock, record change, key press, force_redraw.  

  if (((abe_share->mwShare.multiType[0] == PFM_CZMIL_DATA) && abe_share->key < NV_INT32_MAX &&
       l_share.nearest_point != abe_share->nearest_point) || force_redraw 	   
	   || nonFileIO_force_redraw)	    
  {

	  force_redraw = NVFalse;	

	 
      if (!nonFileIO_force_redraw) {	  

		  l_share = *abe_share;

		  // 
		  // if we are in nearest-neighbors mode and all slots are filled
		  // we process the data for the multiple waveforms
		  //

		  if (multiWaveMode == 0 && abe_share->mwShare.multiNum == MAX_STACK_POINTS) 
				processMultiWaves (l_share);

		  // We do not need to open this file for the primary cursor if it is frozen

		  if (abe_share->mwShare.multiPresent[0] != 0) {

		  	  //  Open the CHARTS files and read the data.

			  NV_CHAR string[512];

			  strcpy (string, l_share.nearest_filename);
			  db_name.sprintf ("%s", pfm_basename (l_share.nearest_filename));


			  strcpy (string, l_share.nearest_filename);
			
			  sprintf (&string[strlen (string) - 4], ".cwf"); 			  
			  
			  strcpy (wave_path, string);			  
			  
			  /* chl revision 08_18_2011 */
			  //CZMIL_Header czmilHeader;
			  CZMIL_CXY_Header czmilHeader;
			  /* end revision */
			  
			  czmilHnd = czmil_open_file (l_share.nearest_filename, &czmilHeader, CZMIL_READONLY, NVTrue);
			  
			  if (czmilHnd < 0) {
				  if (filError) filError->close ();
				  filError = new QMessageBox (QMessageBox::Warning, tr ("Open CZMIL file"), tr ("Error opening ") + 
											  QDir::toNativeSeparators (QString (l_share.nearest_filename)) + " : " + 
											  QString (strerror (errno)), QMessageBox::NoButton, this, 
											  (Qt::WindowFlags) Qt::WA_DeleteOnClose);
				  filError->show ();
				  return;
			  }
			  
			  if (czmilHnd >= 0) 
				{
				  
				  //  Save for slotPlotWaves.

				  recnum = l_share.mwShare.multiRecord[0];
				  
				  strcpy (filename, l_share.nearest_filename);
				  
				  czmil_read_cxy_record (czmilHnd, recnum, &czmil_record);
				  wave_read = czmil_read_cwf_record (czmilHnd, recnum, &wave_data);
				  
				  czmil_close_file (czmilHnd);				  
				  
				  /* chl revision 08_23_2011 */
				  /* move to constructor, no sense to have this here. */
				  
				  // for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) 
					// chanBounds[i].length = UPPER_TIME_BOUND + 1;		

				  /* end revision */
				  
				  /* chl revision 08_26_2011 */
				  //czmil_cvtime (czmil_record.deep.timestamp, &year, &day, &hour, &minute, &second);
				  czmil_cvtime (czmil_record.channel[CZMIL_DEEP_CHANNEL].timestamp, &year, &day, &hour, &minute, &second);
				  /* end revision */
				  
				  czmil_jday2mday(year, day, &month, &mday);
				  month++;

				  date_time.sprintf ("%d-%02d-%02d (%03d) %02d:%02d:%05.2f", year + 1900, month, mday, day, hour, 
									 minute, second);
				  l_share.key = 0;
				  abe_share->key = 0;
				  
				  abe_share->modcode = PFM_CZMIL_DATA;
				  
				  
				  /* chl revision 08_23_2011 */
				  /* this can be done in the constructor, wasting cycles here. */
				  
				  //  Set the min and max values.
				  
				  // for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
				  
					// chanBounds[i].min_y = 0;
					// chanBounds[i].max_y = 1023;
					
					// chanBounds[i].max_x = chanBounds[i].length;
					// chanBounds[i].min_x = 0;
					
					// chanBounds[i].range_x = chanBounds[i].max_x - chanBounds[i].min_x;
					
					//// Add 5% to the X axis.
					
					
				    //// Here, depending on which layout of the panes we are using (vertical/
				    //// horizontal), we calculate the padding before we get to the axes.
				    
					
					// chanBounds[i].max_x = chanBounds[i].max_x + NINT (((NV_FLOAT32) chanBounds[i].range_x * 0.05 + 1));
					// chanBounds[i].range_x = chanBounds[i].max_x - chanBounds[i].min_x;
					
					// chanBounds[i].range_y = chanBounds[i].max_y - chanBounds[i].min_y;
				  
				  // }

				  /* end revision */
				  
				}			  
			}	  
	  }

	  nonFileIO_force_redraw = NVFalse;	 
	 
	  // We only want to draw the maps if they are visible

	    for (int i = 0; i < NUM_WAVEFORMS; i++) 
				  
		  // only redraw the maps that are not blocked.  
		  // but in special situations like intensity meter or zoom drawing only one will not be
		  // blocked.

		  if (map[i]->isVisible ()&& (!mapBlocked[i])) map[i]->redrawMapArea (NVTrue);
		  else mapBlocked[i] = NVFalse;		  
	}	

  //  Display the startup info message the first time through.

  if (first)
    {
      QString startupMessageText = 
        tr ("The following action keys are available in ") + parentName + tr (":\n\n") +		
        QString (ac[0]) + tr (" = Toggle between nearest neighbor and single waveform display mode\n") + 
		
		QString (ac[1]) + tr (" = Toggle Deep on or off\n") + 
        QString (ac[2]) + tr (" = Toggle Shallow1 on or off\n") + 
        QString (ac[3]) + tr (" = Toggle Shallow2 on or off\n") + 
        QString (ac[4]) + tr (" = Toggle Shallow3 on or off\n") + 
        QString (ac[5]) + tr (" = Toggle Shallow4 on or off\n") + 		
		
		/* chl revision 08_30_2011 */
		/* there is no adjustment here.  I just wanted to point out that we need to add
		   more channels but I didn't want to mess with the action keys quite yet */
		/* end revision */
		
        tr ("You can change these key values in the ") + parentName + "\n" +
        tr ("Preferences->Ancillary Programs window\n\n\n") + 
        tr ("You can turn off this startup message in the\n") + 
        tr ("CZMILwaveMonitor Preferences window.");

      if (startup_message) QMessageBox::information (this, tr ("CZMILwaveMonitor Startup Message"), startupMessageText);

      first = NVFalse;
    }
}



//  Signal from the map class.

void 
CZMILwaveMonitor::slotKeyPress (QKeyEvent *e)
{
  NV_CHAR key[20];
  strcpy (key, e->text ().toAscii ());

  if (key[0] == (NV_CHAR) ac[0])
    {
      if (multiWaveMode)
	{
	  nearAct->setChecked (TRUE);
	  slotNear ();
	}
      else
	{
	  noneAct->setChecked (TRUE);
	  slotNone ();
	}
    }

  for (NV_INT32 i = 1 ; i < 6 ; i++) 
    {
      if (key[0] == (NV_CHAR) ac[i])
        {
          if (display_wave[i - 1])
            {
              display_wave[i - 1] = NVFalse;
            }
          else
            {
              display_wave[i - 1] = NVTrue;
            }

          force_redraw = NVTrue;

          break;
        }
    }
}



//  A bunch of slots.

void 
CZMILwaveMonitor::slotQuit ()
{
  envout ();

  //  Let pfmEdit know that we have quit.

  // lock the shared memory to set CZMILwaveMonitorRunning

  abeShare->lock ();

  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (abe_share->key != kill_switch) abe_share->killed = kill_switch;

  /* Bit 1 constitutes CZMIL waveform viewer */  
  abe_share->mwShare.waveMonitorRunning &= 0xfd;
    
  abeShare->unlock ();

  //  Let go of the shared memory.

  abeShare->detach ();

  if ( numReferences > 0 ) clearReferenceData();

  exit (0);
}



void 
CZMILwaveMonitor::slotMode ()
{
  wave_line_mode = !wave_line_mode;

  force_redraw = NVTrue;
}



//
// slot:        slotIntMeter
//
// This slot is fired when the user toggles the intensity meter.  We change
// our flag and if we are turning the meter on we change our cursor to 
// the pointing hand.  Otherwise, we go back to the standard.  
//

void
CZMILwaveMonitor::slotIntMeter () 
{
  showIntMeter = !showIntMeter;

  if (showIntMeter)
    {
      map[0]->setCursor (Qt::PointingHandCursor);
    }
  else
    {
      map[0]->setCursor (Qt::ArrowCursor);
    }

  force_redraw = NVTrue;
}



//
// slot:        slotZoomIn
//                                        
// This slot is called either from a menu selection or a right-menu context
// click.  It simply changes our zooming flag to true.
//

void
CZMILwaveMonitor::slotZoomIn()
{
  zoomFlag = NVTrue;
}



// fixed ltimebound in and utimebound in the following 3 functions
//
// slotresetzoom 
//	- delete rev 1.2 change
// slotContextResetZoom
//	- changed raman hard code from 200 to ir.length
//	- subtracted 1 from the lengths to properly set the utimebounds
// resetBounds
//	- set reset lTimeBound[mi] back to 1 instead of 0;

//
// slot:        slotResetZoom
//
// This slot is called from the Reset Zoom menu selection or right-click
// option.  We turn zooming off by changing zoomFlag.  We reset our time
// and count bounds to the original values and redraw the CZMILwaveMonitor.
//

void
CZMILwaveMonitor::slotResetZoom()
{
  zoomFlag = NVFalse; 
  
  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) 
	uTimeBound[i] = chanBounds[i].length - 1;  
  
  for (NV_INT32 i = 0 ; i < NUM_WAVEFORMS ; i++) {	  
	  
	  lTimeBound[i] = LOWER_TIME_BOUND;
      lCountBound[i] = LOWER_COUNT_BOUND;	  
      uCountBound[i] = UPPER_COUNT_BOUND;	  
    }  

  force_redraw = NVTrue;
}




// called from contextMenu's resetZoom action

void
CZMILwaveMonitor::slotContextResetZoom()
{
	zoomFlag = NVFalse;


	NV_INT32 mi = -1;
	
	for ( mi = 0; mi < NUM_WAVEFORMS; mi++ ) 
		if( zoomOutMap == map[mi] ) 
			break;

	if( mi < 0 ) return ; // default to zero if not found

	resetBounds( mi, chanBounds[mi].length - 1 );	

    nonFileIO_force_redraw = NVTrue;
}



void 
CZMILwaveMonitor::resetBounds(NV_INT32 mi, NV_INT32 mapLength ) {
	
	lTimeBound[mi] = LOWER_TIME_BOUND;
	uTimeBound[mi] = mapLength;
	lCountBound[mi] = LOWER_COUNT_BOUND;	
	uCountBound[mi] = UPPER_COUNT_BOUND;	
}




void 
CZMILwaveMonitor::setFields ()
{
  if (pos_format == "hdms") bGrp->button (0)->setChecked (TRUE);
  if (pos_format == "hdm") bGrp->button (1)->setChecked (TRUE);
  if (pos_format == "hd") bGrp->button (2)->setChecked (TRUE);
  if (pos_format == "dms") bGrp->button (3)->setChecked (TRUE);
  if (pos_format == "dm") bGrp->button (4)->setChecked (TRUE);
  if (pos_format == "d") bGrp->button (5)->setChecked (TRUE);


  if (startup_message)
    {
      sMessage->setCheckState (Qt::Checked);
    }
  else
    {
      sMessage->setCheckState (Qt::Unchecked);
    }
      

  NV_INT32 hue, sat, val;

  surfaceColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bSurfacePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bSurfacePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bSurfacePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bSurfacePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bSurfacePalette.setColor (QPalette::Normal, QPalette::Button, surfaceColor);
  bSurfacePalette.setColor (QPalette::Inactive, QPalette::Button, surfaceColor);
  bSurfaceColor->setPalette (bSurfacePalette);


  primaryColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bPrimaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bPrimaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bPrimaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bPrimaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bPrimaryPalette.setColor (QPalette::Normal, QPalette::Button, primaryColor);
  bPrimaryPalette.setColor (QPalette::Inactive, QPalette::Button, primaryColor);
  bPrimaryColor->setPalette (bPrimaryPalette);


  secondaryColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bSecondaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bSecondaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bSecondaryPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bSecondaryPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bSecondaryPalette.setColor (QPalette::Normal, QPalette::Button, secondaryColor);
  bSecondaryPalette.setColor (QPalette::Inactive, QPalette::Button, secondaryColor);
  bSecondaryColor->setPalette (bSecondaryPalette);


  backgroundColor.getHsv (&hue, &sat, &val);
  if (val < 128)
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bBackgroundPalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bBackgroundPalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bBackgroundPalette.setColor (QPalette::Normal, QPalette::Button, backgroundColor);
  bBackgroundPalette.setColor (QPalette::Inactive, QPalette::Button, backgroundColor);
  bBackgroundColor->setPalette (bBackgroundPalette);

  // we account for making the text of the button white or black based on the shading of 
  // the color of the button.  we then set the color of the pallete attached to the button
  // to that of what the user picked within the QColorDialog.  We update the palette.

  referenceColor.getHsv (&hue, &sat, &val);

  if (val < 128)
    {
      bReferencePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::white);
      bReferencePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::white);
    }
  else
    {
      bReferencePalette.setColor (QPalette::Normal, QPalette::ButtonText, Qt::black);
      bReferencePalette.setColor (QPalette::Inactive, QPalette::ButtonText, Qt::black);
    }
  bReferencePalette.setColor (QPalette::Normal, QPalette::Button, referenceColor);
  bReferencePalette.setColor (QPalette::Inactive, QPalette::Button, referenceColor);
  bReferenceColor->setPalette (bReferencePalette); 
}



// 
// method:        GetTickSpacing
//
// This method will get the intervals for our tick marks for our legends.
// For a range of < 10, we do single tick steps.  Anything over that is done
// by a tenth of the range.
//

void
CZMILwaveMonitor::GetTickSpacing (NV_INT32 lTimeBound, NV_INT32 uTimeBound, NV_INT32 lCountBound, NV_INT32 uCountBound,
                             NV_INT32 &timeSpacing, NV_INT32 &countSpacing)
{
  NV_INT32 timeRange, countRange;

  timeRange = uTimeBound - lTimeBound + 1;
  countRange = uCountBound - lCountBound;

  if (timeRange <= 10)
    {
      timeSpacing = 1;
    }
  else
    {
      timeSpacing = timeRange / 10;
    }

  if (countRange <= 10)
    {
      countSpacing = 1;
    }
  else
    {
      countSpacing = countRange / 10;
    }
}



//
// method:        InsertGridTicks
//
// This method will take the current bounds of the sensor type and proceed
// to display a tick mark with the proper numbers under them for either
// vertical or horizontal mode.
//

void
CZMILwaveMonitor::InsertGridTicks (NV_INT32 start, NV_INT32 end, NV_CHAR axis, NV_INT32 spacing, NV_INT32 type,
                              NV_INT32 axisIndex, nvMap * l_map)
{
  NV_INT32 pix_x, pix_y;
  QString value;
  NV_INT32 length = 0, scaledValue, lCount = 0, uCount = 0, lTime = 0, uTime = 0;


  // get the time and count bounds based on type
  /* revised and fixed for new indexing */  
  
  length = chanBounds[type].length;  
  
  lCount = lCountBound[type];
  uCount = uCountBound[type];
  lTime = lTimeBound[type];
  uTime = uTimeBound[type];

  // we move from the starting value to the end value

  for (NV_INT32 i = start; i <= end; i++) 
    {
      // if we've hit a spot for the tick mark, we will check which axis and which display
      // mode we are in (vertical/horizontal).  Then, we will draw a tick mark and the number
      // to place.

      if ((i % spacing) == 0) 
        {
          if ((axis == 'x') || (axis == 'X')) 
            {
              scaledValue = (NV_INT32)(((i - lTime) / ((NV_FLOAT32)(uTime - lTime)) * (length - 2)) + 1);                        
              scaleWave (scaledValue, axisIndex, &pix_x, &pix_y, type, l_map->getMapdef());
			  l_map->drawLine (pix_x, pix_y, pix_x, pix_y + LBL_PIX_SPACING - 2, Qt::white, 1, NVFalse, Qt::SolidLine);
			 
			  value.sprintf ("%3d", i);
              l_map->drawText (value, NINT (pix_x - gridWidth / 2.0), pix_y + LBL_PIX_SPACING + pt8Height, 90.0, 8,
                               Qt::white, NVTrue); 			  
            }
          else 
            {	
			  
			  scaledValue = (NV_INT32)((i - lCount) / ((NV_FLOAT32)(uCount - lCount)) * UPPER_COUNT_BOUND);
			  			  
			  scaleWave (axisIndex, scaledValue, &pix_x, &pix_y, type, l_map->getMapdef());

			  l_map->drawLine (pix_x, pix_y, pix_x - LBL_PIX_SPACING - 2, pix_y, Qt::white, 1, NVFalse, Qt::SolidLine);
			  	  

              value.sprintf ("%3d", i);
              l_map->drawText (value, pix_x - LBL_PIX_SPACING - gridWidth - 3, NINT (pix_y + pt8Height / 2.0), 90.0, 8,
                               Qt::white, NVTrue);
			  
            }
        }
    }
}



void
CZMILwaveMonitor::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  
  prefsD->setWindowTitle (tr ("CZMIL Waveform Viewer Preferences"));
    
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

  if (pos_format == "hdms") bGrp->button (0)->setChecked (TRUE);
  if (pos_format == "hdm") bGrp->button (1)->setChecked (TRUE);
  if (pos_format == "hd") bGrp->button (2)->setChecked (TRUE);
  if (pos_format == "dms") bGrp->button (3)->setChecked (TRUE);
  if (pos_format == "dm") bGrp->button (4)->setChecked (TRUE);
  if (pos_format == "d") bGrp->button (5)->setChecked (TRUE);


  QGroupBox *cbox = new QGroupBox (tr ("Colors"), this);
  QVBoxLayout *cboxLayout = new QVBoxLayout;
  cbox->setLayout (cboxLayout);
  QHBoxLayout *cboxTopLayout = new QHBoxLayout;

  // since we are adding another layout which is at the bottom, the former bottom becomes the middle
  // and we still maintain the bottom layout for the reference color QPushButton
    
  QHBoxLayout *cboxMiddleLayout = new QHBoxLayout;
  QHBoxLayout *cboxBottomLayout = new QHBoxLayout;

  cboxLayout->addLayout (cboxTopLayout);
  cboxLayout->addLayout (cboxMiddleLayout);
  cboxLayout->addLayout (cboxBottomLayout);

  bSurfaceColor = new QPushButton (tr ("Surface"), this);
  bSurfaceColor->setToolTip (tr ("Change surface return marker color"));
  bSurfaceColor->setWhatsThis (bSurfaceColor->toolTip ());
  bSurfacePalette = bSurfaceColor->palette ();
  connect (bSurfaceColor, SIGNAL (clicked ()), this, SLOT (slotSurfaceColor ()));
  cboxTopLayout->addWidget (bSurfaceColor);


  bPrimaryColor = new QPushButton (tr ("Primary"), this);
  bPrimaryColor->setToolTip (tr ("Change primary return marker color"));
  bPrimaryColor->setWhatsThis (bPrimaryColor->toolTip ());
  bPrimaryPalette = bPrimaryColor->palette ();
  connect (bPrimaryColor, SIGNAL (clicked ()), this, SLOT (slotPrimaryColor ()));
  cboxTopLayout->addWidget (bPrimaryColor);


  bSecondaryColor = new QPushButton (tr ("Secondary"), this);
  bSecondaryColor->setToolTip (tr ("Change secondary return marker color"));
  bSecondaryColor->setWhatsThis (bSecondaryColor->toolTip ());
  bSecondaryPalette = bSecondaryColor->palette ();
  connect (bSecondaryColor, SIGNAL (clicked ()), this, SLOT (slotSecondaryColor ()));

  
  // change from bottom to middle
  
  cboxMiddleLayout->addWidget (bSecondaryColor);

  bBackgroundColor = new QPushButton (tr ("Background"), this);
  bBackgroundColor->setToolTip (tr ("Change display background color"));
  bBackgroundColor->setWhatsThis (bBackgroundColor->toolTip ());
  bBackgroundPalette = bBackgroundColor->palette ();
  connect (bBackgroundColor, SIGNAL (clicked ()), this, SLOT (slotBackgroundColor ()));

  // change from bottom to middle
  
  cboxMiddleLayout->addWidget (bBackgroundColor);

  // adding the ability to change the reference mark overlay color

  bReferenceColor = new QPushButton (tr ("Reference"), this);
  bReferenceColor->setToolTip (tr ("Change reference overlay color"));
  bReferenceColor->setWhatsThis (bReferenceColor->toolTip());
  bReferencePalette = bReferenceColor->palette ();
  connect (bReferenceColor, SIGNAL (clicked ()), this, SLOT (slotReferenceColor ()));
  cboxBottomLayout->addWidget (bReferenceColor);

  vbox->addWidget (cbox, 1);


  QGroupBox *mBox = new QGroupBox (tr ("Display startup message"), this);
  QHBoxLayout *mBoxLayout = new QHBoxLayout;
  mBox->setLayout (mBoxLayout);
  sMessage = new QCheckBox (mBox);
  sMessage->setToolTip (tr ("Toggle display of startup message when program starts"));
  mBoxLayout->addWidget (sMessage);


  vbox->addWidget (mBox, 1);


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

  setFields ();

  prefsD->show ();
}


//
// Slot:		slotReferenceColor
//
// This slot will automagically be called when the user presses the "Reference" color button
// to change the appearance of the reference waveform signatures.  A color dialog box will be
// displayed allowing the user to select the new color.
//

void CZMILwaveMonitor::slotReferenceColor () {

  QColor clr;

  clr = QColorDialog::getColor (referenceColor, this, 
								tr ("CZMIL Waveform Viewer Reference Color"), 
								QColorDialog::ShowAlphaChannel);
  
  if (clr.isValid ()) referenceColor = clr;

  setFields ();  
  
  for (NV_INT32 i = 0 ; i < NUM_WAVEFORMS ; i++) 
	if (map[i]->isVisible()) 
		map[i]->redrawMapArea (NVTrue);  
}



// now saves last directory of opening a file


void CZMILwaveMonitor::slotOpen () {

	QFileDialog fd;
	if (!QDir(savedSettings.openDirectory).exists()  ) savedSettings.openDirectory = ".";

	QString fileName = fd.getOpenFileName(this, tr("Open Waveform File"), savedSettings.openDirectory, tr("Waveform Files (*.wfm)"));
	if ( fileName.isNull() ) return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

	savedSettings.openDirectory = QFileInfo(fileName).canonicalPath();

	QTextStream ts(&file);
	QString tok;

	do ts >> tok; while ( tok != "Time_Bins"  && !ts.atEnd() );

	if (!ts.atEnd()) {

		int numCols;
		for( numCols = 0; tok != "1"; ts >> tok ) numCols++;

		loadReferenceData(ts, numCols);
	}

	file.close();

	 // save refFileLbl and refFileWidth on a successful open

	if (drawReference) {

		QFileInfo refFileInfo (fileName);
		refFileLbl = refFileInfo.fileName ();
		
		// get type and size of font we are typing with

		QFont font("Charter", 12);
		QFontMetrics fontMetric (font);
		refFileWidth = fontMetric.width (refFileLbl);		
		
		for (int i = 0 ; i < NUM_WAVEFORMS; i++) 
			if (map[i]->isVisible()) 
				map[i]->redrawMapArea(NVTrue);		
	}	
}



void CZMILwaveMonitor::loadReferenceData(QTextStream &fileDat, int numCols) {

	allocateReferenceData ((numCols-1) / NUM_WAVEFORMS);
		
	QString tok;
	bool isInt;
	int val, r;
	int writes = 0;

	for (r = 0; r < UPPER_TIME_BOUND && !fileDat.atEnd(); r++) {
		
		if (r>0) fileDat >> tok; // skip the time_bin alraedy read for first so no skipping the first;

		for (int wave = 0; wave < numReferences; wave++) 
		
			for (int c = 0; c < NUM_WAVEFORMS && !fileDat.atEnd(); c++) {
			
				fileDat >> tok;			
				
				val = tok.toInt (&isInt, 10);
				
				if (isInt) { 
					writes++; 					
					referenceData[wave][c][r+1] = (NV_INT16)val; 					
				} // start saving data at referenceData[wave][0][1] to work with old code
			}

	}


	// set referenceData true if read through all 500 
	
	drawReference = (r == UPPER_TIME_BOUND);
	
	// we do want to redraw all the maps so we can see the reference waveforms but we need to
	// do it after we have the reference label established which is later on in slotOpen.
	// Therefore, I deleted the redrawing here.

	if (!drawReference) clearReferenceData ();	
	
	// assumption:  if we get here, all went well with the opening of the file.
	// we simply enable the reference mark entries.

	toggleReference->setEnabled (drawReference);
	toggleReference->setChecked (drawReference);
	clearReference->setEnabled  (drawReference);	
}


void CZMILwaveMonitor::allocateReferenceData(int numWaves) {

	clearReferenceData();
	
	NV_INT32 lengths[NUM_WAVEFORMS];

	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) lengths[i] = chanBounds[i].length;
	
	/* chl revision 08_18_2011 */
	
	// referenceData = (NV_INT16 ***) calloc( numWaves, sizeof(NV_INT16 **) );	

	// for (int i = 0; i < numWaves; i ++ ) {
		
		// referenceData[i] = (NV_INT16 **) calloc ( 5, sizeof (NV_INT16 *));	
	
		// for (int j = 0; j < NUM_WAVEFORMS; j++) 
			// referenceData[i][j] = (NV_INT16 *) calloc (lengths[j], sizeof (NV_INT16));	
	// }
	
	referenceData = (NV_U_INT16 ***) calloc( numWaves, sizeof(NV_U_INT16 **) );	

	for (int i = 0; i < numWaves; i ++ ) {
		
		referenceData[i] = (NV_U_INT16 **) calloc ( 5, sizeof (NV_U_INT16 *));	
	
		for (int j = 0; j < NUM_WAVEFORMS; j++) 
			referenceData[i][j] = (NV_U_INT16 *) calloc (lengths[j], sizeof (NV_U_INT16));	
	}
	
	/* end revision */
	
	numReferences = numWaves;
}

void CZMILwaveMonitor::clearReferenceData() {

	if (numReferences > 0) {
		for (int i = 0; i < numReferences; i ++ ) {		
			
			for (int j = 0; j < NUM_WAVEFORMS; j++) free(referenceData[i][j]);			
			free(referenceData[i]);
		}
		free( referenceData);
		numReferences = 0;
		drawReference = NVFalse;
	}
}


// now saves last directory of saving a file

void CZMILwaveMonitor::slotSave () {

	QFileDialog fd;
	if (!QDir(savedSettings.saveDirectory).exists() ) savedSettings.saveDirectory = ".";

	QString saveName = fd.getSaveFileName(this, tr("Save Waveform File"), savedSettings.saveDirectory, tr("Waveform Files (*.wfm)"));

	if (!saveName.isNull() ) {

		ofstream outFile(qPrintable(saveName));
		if ( outFile.is_open() ) {
			writeWaveformHeader( outFile, saveName );
			writeWaveformData(outFile);

			outFile.close();
			savedSettings.saveDirectory = QFileInfo(saveName).canonicalPath();

		} else {
			QMessageBox::warning (this, tr ("File not saved"), tr ("Unable to open ") + saveName );
		} 

	}

}


void CZMILwaveMonitor::writeWaveformHeader(ofstream &outFile, QString &fileName) {

	QString pfmFiles = tr("PFM File") + ( (num_pfms <= 1) ? "" : "s" );
		
	
	for (NV_INT32 pfm = 0 ; pfm < num_pfms ; pfm++) pfmFiles += tr("\t") + tr(open_args[pfm].list_path) + "\n";

		outFile 
			<< "; " << qPrintable( QDateTime::currentDateTime().toString("MM/dd/yyyy h:mmap") ) << "\n"
			<< "; " << qPrintable( fileName ) << "\n"
			<< "; " << qPrintable( pfmFiles ) << "\n";

}


//
// method:		WriteWaveformData 
//
// This method will produce the waveform data portion of the .wfm file.  It will write the
// waveform data headings for all waveforms combining the record number and flightline.  It will
// then traverse through the timebins with the waveform information for all sensors.
//

void CZMILwaveMonitor::writeWaveformData (ofstream &outFile) {


	// only supports CZMIL data sets	
		
	if (abe_share->mwShare.multiType[0] == PFM_CZMIL_DATA) {
	
		outFile << ";------------------------------------\n";
		outFile << "; Waveform data\n"; 
		outFile << ";------------------------------------\n\n";

		outFile << "Time_Bins\t";

		char flightLn[20];
		
		QString sensorHeadings[NUM_WAVEFORMS] = {"DEEP_", "SHAL1_", "SHAL2_", "SHAL3_", 
													"SHAL4_", "SHAL5_", "SHAL6_", 
													"SHAL7_", "IR_"};
													
		QString appendage;
		
		int numWaveforms = multiWaveMode ? 1 : MAX_STACK_POINTS;

		for (int i = 0; i < numWaveforms; i++) {

			strcpy (flightLn, read_line_file (pfm_handle[abe_share->mwShare.multiPfm[i]], 
												abe_share->mwShare.multiLine[i]));

			// needed to get the gps time to be added to the header column labels
			
			QString gpsTime = RetrieveGPSTime (i);
			
			appendage.sprintf ("%s_%s_%d", qPrintable (gpsTime), flightLn, abe_share->mwShare.multiRecord[i]);

			appendage.remove (appendage.indexOf ("Line "), 5);

			for (int i = 0; i < NUM_WAVEFORMS; i++) 			
				outFile << qPrintable (sensorHeadings[i] + appendage) << "\t";
		}

		outFile << "\n";
		outFile.flush ();
		
		// no record 0.  

		for (int i = 1; i <= UPPER_TIME_BOUND; i++) {
		
			outFile << i << "\t";

			if (multiWaveMode) writeSingleTableEntry (outFile, i);
			else writeNNTableEntry (outFile, i);

			outFile << "\n";
		}

		outFile.flush ();
	}
}



//
// Method:		RetrieveGPSTime
//
// This function will return just the GPS time that is embedded within the .hof file.  This
// will usually be a string composed of 4 digits that will be used as header labels within
// a .wfm file.
//

QString CZMILwaveMonitor::RetrieveGPSTime (int index) {

	NV_INT16 type;	
	NV_CHAR czmil_file[512];
		
	QString gpsTime;

	read_list_file (pfm_handle[abe_share->mwShare.multiPfm[index]], abe_share->mwShare.multiFile[index],
						czmil_file, &type);

	gpsTime = QString (pfm_basename (czmil_file));
	
	gpsTime = gpsTime.section ("_", 3, 3);

	return gpsTime;
}



//
// method:		WriteSingleTableEntry
//
// This method accounts for the single waveform mode.  It will write out the deep, shallow, and ir channels.
// information.


void CZMILwaveMonitor::writeSingleTableEntry (ofstream &outfile, NV_INT32 timeIndex) {

	/* chl revision 08_26_2011 */
	
    // outfile << (int)wave_data.deep[timeIndex] << "\t";
	
	// for (NV_INT32 i = 0; i < 7; i++)
		// outfile << (int)wave_data.shallow[i][timeIndex] << "\t";
		
	// outfile << (int)wave_data.ir[timeIndex] << "\t";
	
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) 	
		outfile << (NV_INT32) wave_data.channel[i][timeIndex] << "\t";
	
	/* end revision */
}


//
// method:		WriteNNTableEntry
//
// This method accounts for the nearest neighbors mode.  It will write out the PMT, APD, TBDD, IR, and RAMAN
// information.
//

void CZMILwaveMonitor::writeNNTableEntry (ofstream &outfile, NV_INT32 timeIndex) {

	/* chl revision 08_26_2011 */
	
	// for (int i = 0; i < MAX_STACK_POINTS; i++) {			

		// outfile << (int)wave_data_multi[i].deep[timeIndex] << "\t";
	
		// for (NV_INT32 j = 0; j < 7; j++)
			// outfile << (int)wave_data_multi[i].shallow[j][timeIndex] << "\t";
		
		// outfile << (int)wave_data_multi[i].ir[timeIndex] << "\t";			
	// }
	
	for (NV_INT32 i = 0; i < MAX_STACK_POINTS; i++) {
	
		for (NV_INT32 j = 0; j < NUM_WAVEFORMS; j++) {		
			outfile << (NV_INT32)wave_data_multi[i].channel[j][timeIndex] << "\t";		
		}	
	}
	
	/* end revision */
}



void
CZMILwaveMonitor::slotPosClicked (int id)
{
  switch (id)
    {
    case 0:
    default:
      pos_format = "hdms";
      break;

    case 1:
      pos_format = "hdm";
      break;

    case 2:
      pos_format = "hd";
      break;

    case 3:
      pos_format = "dms";
      break;

    case 4:
      pos_format = "dm";
      break;

    case 5:
      pos_format = "d";
      break;
    }
}



void
CZMILwaveMonitor::slotClosePrefs ()
{
  if (sMessage->checkState () == Qt::Checked)
    {
      startup_message = NVTrue;
    }
  else
    {
      startup_message = NVFalse;
    }

  prefsD->close ();
}



void
CZMILwaveMonitor::slotSurfaceColor ()
{
  QColor clr;
  
  clr = QColorDialog::getColor (surfaceColor, this, 
								tr ("CZMIL Waveform Viewer Surface Marker Color"), 
								QColorDialog::ShowAlphaChannel);  

  if (clr.isValid ()) surfaceColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
CZMILwaveMonitor::slotPrimaryColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (primaryColor, this, 
								tr ("CZMIL Waveform Viewer Primary Marker Color"), 
								QColorDialog::ShowAlphaChannel);
  
  if (clr.isValid ()) primaryColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
CZMILwaveMonitor::slotSecondaryColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (secondaryColor, this, 
								tr ("CZMIL Waveform Viewer Secondary Marker Color"), 
								QColorDialog::ShowAlphaChannel);
  
  if (clr.isValid ()) secondaryColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void
CZMILwaveMonitor::slotBackgroundColor ()
{
  QColor clr;

  clr = QColorDialog::getColor (backgroundColor, this, 
								tr ("CZMIL Waveform Viewer Background Color"));
  
  if (clr.isValid ()) backgroundColor = clr;

  setFields ();

  force_redraw = NVTrue;
}



void 
CZMILwaveMonitor::scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef)
{
    	
	BOUNDS * b = &(chanBounds[type]);	

	*new_x = NINT (((NV_FLOAT32) (x - b->min_x) / b->range_x) * 
					(NV_FLOAT32) (l_mapdef.draw_width - horLblSpace)) + horLblSpace;	
	
    *new_y = NINT (((NV_FLOAT32) (b->max_y - (y - b->min_y)) / 
					(NV_FLOAT32) b->range_y) * (NV_FLOAT32) (l_mapdef.draw_height - verLblSpace));
	
}


void 
CZMILwaveMonitor::slotPlotWaves (NVMAP_DEF l_mapdef)
{

  static NV_INT32         save_rec;    
  static CZMIL_CWF_Data   save_wave;
  static CZMIL_CXY_Data   save_czmil;  
  static QString          save_name;
  NV_INT32                pix_x[2], pix_y[2];	

  // 
  //                      we have some new variables added here.  We declare a half opaque green
  //                      for the intensity meter.  scaledTimeValue and scaledCountValue are the
  //                      resultant time and count spots in the selection area after the zoom
  //                      for the current sensor.  The usingMapX parameters reference what pane
  //                      we are currently processing.  previousValid and currentValid are needed
  //                      for line segments to attach.  For a zoomed in region, the lines could
  //                      go in and out of the area. timeSpacing and countSpacing will hold the 
  //                      interval values for our tick marks
  //

  static QColor intMeterColor (0, 255, 0, 255);  // half opaque green    
  NV_BOOL usingMap[NUM_WAVEFORMS];
  static NV_BOOL firstTime[NUM_WAVEFORMS] = {NVTrue, NVTrue, NVTrue, NVTrue, NVTrue,
												NVTrue, NVTrue, NVTrue, NVTrue};
												
  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) usingMap[i] = NVFalse;
  
  NV_INT32 timeSpacing, countSpacing;
  nvMap *l_map = (nvMap *) (sender ());  
  
  // key variables that allows us not to duplicate code later down

  NV_INT32 mapIndex, mapLength;  

  //  If we haven't updated the point and read a new one, as they say in Poltergeist, "GET OOOOUUUUT!".

  /* chl revision 08_22_2011 */
  //if (!wave_read) return; 
  if (wave_read != CZMIL_SUCCESS) return;
  /* end revision */  

  //  Figure out which widget sent the signal  	
	
	NV_INT32 lengths[NUM_WAVEFORMS];
	for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) lengths[i] = chanBounds[i].length;
	
	mapIndex = GetSensorMapType( l_map );
	mapLength = lengths[mapIndex];		
	
	usingMap[mapIndex] = NVTrue;
	
	// change darkgray to referenceColor
	if ( drawReference  ) 
		for (int i=0; i< numReferences; i++ ) 
			drawSingleWave(l_map, l_mapdef, referenceData[i][mapIndex], mapIndex, mapLength, referenceColor );	

	// check to see if ref line should be drawn

  //  Because the trackCursor function may be changing the data while we're still plotting it we save it
  //  to this static structure.  lock_track stops trackCursor from updating while we're trying to get an
  //  atomic snapshot of the data for the latest point.

  lock_track = NVTrue; 
  
  save_wave = wave_data;   
  save_czmil = czmil_record;  
  save_rec = recnum;
  save_name = db_name;
  lock_track = NVFalse;


  //
  // The first time through for any sensor we set the beginning and
  // ending position for the intensity meter  
  //
  
  // Because we have to get the beginning/ending pixel positions of the time axis the first time
  // through this function, firstTime was changed to an array for each of the sensor maps.  So, for
  // each sensor map, store the beginning/ending axis pixel postiions and set the firstTime array
  // to false.  This is a cludgy way to do it and you would think that slotResize would handle this
  // but slotResize is not called upon window startup like OpenGL.  It is called only when the user
  // resizes the window.
  // 

  if (firstTime[mapIndex] && usingMap[mapIndex])  
    {			

	  scaleWave (lTimeBound[mapIndex], lCountBound[mapIndex], &(axisPixelBegin[mapIndex]), &pix_y[0], mapIndex, l_mapdef);
      scaleWave (uTimeBound[mapIndex], lCountBound[mapIndex], &(axisPixelEnd[mapIndex]), &pix_y[0], mapIndex, l_mapdef);
	  
	  firstTime[mapIndex] = NVFalse;
    }  
  

  //
  // if we are zooming and we have placed the first pivot point, draw
  // the current rectangle.
  //

  if (zoomFlag && zoomIsActive)
    {
		
      zoomMap->drawRectangle(zoomFirstPos.x(), zoomFirstPos.y(), zoomSecondPos.x(), 
                             zoomSecondPos.y(), Qt::white, 2, Qt::SolidLine, NVTrue);
	  
    }  

  // 
  // We check to see if we are drawing the intensity meter.  If so, we get
  // the bin that is highlighted and clamp it to 1-500 if we are past that
  // range.  We scale it if it is in a zoomed range.  We find the projected
  // spot for the pane depending on vertical or horizontal view.  Finally, we
  // draw the line.
  //

  else if (showIntMeter)  {
	  
	  // First off, clamp the bin highlighted to a 1-(mapLength -1) value depending on the sensor.
	  // Next, scale the binHighlighted based on the current zooming extents of the sensor pane. 
	  // Get the intensity meter pixel positions off of the binHighlighted and then draw the line
	  // in a lime green color.

	  if (binHighlighted[mapIndex] < 1) binHighlighted[mapIndex] = 1;
	  else if (binHighlighted[mapIndex] > (mapLength - 1)) binHighlighted[mapIndex] = mapLength - 1;	  

	  scaledBinHighlighted[mapIndex] = (NV_INT32) ((lTimeBound[mapIndex] + (((binHighlighted[mapIndex] - 1) / ((NV_FLOAT32)(mapLength - 2))) *
                                                          ((NV_FLOAT32) (uTimeBound[mapIndex] - lTimeBound[mapIndex]))) + 0.5));		  

      scaleWave (binHighlighted[mapIndex], LOWER_COUNT_BOUND, &pix_x[0], &pix_y[0], mapIndex, l_mapdef); 	  
      scaleWave (binHighlighted[mapIndex], UPPER_COUNT_BOUND, &pix_x[1], &pix_y[1], mapIndex, l_mapdef);		  
	  
	  l_map->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], intMeterColor, 1, NVFalse, Qt::SolidLine); 
	  
  }
	
  // just a simple move. scaled binhighlighted needs to be computed before a possible drawMultiWaves is called
  // if we are in nearest neighbor mode, draw the multiple waveforms
  //
  
  NV_BOOL usingAnyMap = NVFalse;
  
  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
	if (usingMap[i]) {
		usingAnyMap = NVTrue;
		break;
	}
  }  
  
  if (usingAnyMap && multiWaveMode == 0 && abe_share->mwShare.multiNum == MAX_STACK_POINTS) {	
    drawMultiWaves (l_map, l_mapdef);  
  }

  //
  // if we are not in nearest neighbor.  This
  // means that all cursors in pfmEdit will be represented in this code block.
  // This code block is focusing on displaying the current focused cursor. 
  //  
	
  if (multiWaveMode == 1 )
  {		
		// Here, we check to see which mode we are in and we load up the color by cursor or flightline

		if (ccMode == CC_BY_CURSOR) 
		  waveColor.setRgb (abe_share->mwShare.multiColors[0].r, abe_share->mwShare.multiColors[0].g,
							abe_share->mwShare.multiColors[0].b);
		else 
		  waveColor = colorArray[abe_share->mwShare.multiFlightlineIndex[0]];		

        waveColor.setAlpha (abe_share->mwShare.multiColors[0].a); 
		
		// draw wave form			
				
		/* chl revision 08_18_2011 */
		
		// NV_INT16 * dat = (mapIndex == CZMIL_DEEP) ? save_wave.deep :
						 // (mapIndex == CZMIL_SHALLOW1) ? save_wave.shallow[0] :
						 // (mapIndex == CZMIL_SHALLOW2) ? save_wave.shallow[1] :
						 // (mapIndex == CZMIL_SHALLOW3) ? save_wave.shallow[2] :
						 // (mapIndex == CZMIL_SHALLOW4) ? save_wave.shallow[3] :
						 // (mapIndex == CZMIL_SHALLOW5) ? save_wave.shallow[4] :
						 // (mapIndex == CZMIL_SHALLOW6) ? save_wave.shallow[5] :
						 // (mapIndex == CZMIL_SHALLOW7) ? save_wave.shallow[6] :
						 // save_wave.ir;						 
		
		/* chl revision 08_22_2011 */
		
		// NV_U_INT16 * dat = (mapIndex == CZMIL_DEEP) ? save_wave.deep :
						 // (mapIndex == CZMIL_SHALLOW1) ? save_wave.shallow[0] :
						 // (mapIndex == CZMIL_SHALLOW2) ? save_wave.shallow[1] :
						 // (mapIndex == CZMIL_SHALLOW3) ? save_wave.shallow[2] :
						 // (mapIndex == CZMIL_SHALLOW4) ? save_wave.shallow[3] :
						 // (mapIndex == CZMIL_SHALLOW5) ? save_wave.shallow[4] :
						 // (mapIndex == CZMIL_SHALLOW6) ? save_wave.shallow[5] :
						 // (mapIndex == CZMIL_SHALLOW7) ? save_wave.shallow[6] :
						 // save_wave.ir;	

		/* end revision */
						 
		/* end revision */

		/* chl revision 08_22_2011 */
		//drawSingleWave(l_map, l_mapdef, dat, mapIndex, mapLength, waveColor );
		drawSingleWave(l_map, l_mapdef, &save_wave, mapIndex, mapLength, waveColor );
		/* end revision */
	
	/* chl revision 08_18_2011 */
	
	// sim_depth.sprintf ("%.2f", save_czmil.deep.depth[0]);
	// sim_depth1.sprintf ("%.2f", save_czmil.deep.depth[1]);
	// sim_depth2.sprintf ("%.2f", save_czmil.deep.depth[2]);
	// sim_kgps_elev.sprintf ("%.2f", save_czmil.deep.kgps_elevation);	
	
	/* chl revision 08_26_2011 */
	
	// sim_depth.sprintf ("%.2f", save_czmil.deep.return_elevation[1]);
	// sim_depth1.sprintf ("%.2f", save_czmil.deep.return_elevation[2]);
	// sim_depth2.sprintf ("%.2f", save_czmil.deep.return_elevation[3]);
	// sim_kgps_elev.sprintf ("%.2f", save_czmil.deep.return_elevation[0]);	
	
	sim_depth.sprintf ("%.2f", save_czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[1]);
	sim_depth1.sprintf ("%.2f", save_czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[2]);
	sim_depth2.sprintf ("%.2f", save_czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[3]);
	sim_kgps_elev.sprintf ("%.2f", save_czmil.channel[CZMIL_DEEP_CHANNEL].return_elevation[0]);	
	
	/* end revision */
	
	/* end revision */
  }

	
  //  Set the status bar labels

  nameLabel->setText (save_name);  
  simDepth->setText (sim_depth);
  simDepth1->setText (sim_depth1);
  simDepth2->setText (sim_depth2);
  simKGPSElev->setText (sim_kgps_elev);   

  NV_INT32 w = l_mapdef.draw_width, h = l_mapdef.draw_height;

  //
  // 110 is hardcoded in to provide the same spacing.  i'm just placing
  // the labels at the top right to get out of the way of the signatures. 
  // if we have the intensity meter up, set the labels
  //

  if (showIntMeter && !(zoomIsActive==NVTrue  && zoomFlag == NVTrue))
    {      

		// Since the intensity meters are independent and not linked, we had to point them to the 
		// proper spots in the scaledBinHighlighted array.  This block of code could be cleaned up 
		// by the way.	     
	  
	  /* chl revision 08_18_2011 */
	  //NV_INT16 * waveformData;
	  NV_U_INT16 * waveformData;
	  /* end revision */
	  
	  /* chl revision 08_26_2011 */
	  
	  // switch (mapIndex) {
	  
		// case CZMIL_DEEP:	
			// waveformData = save_wave.deep;
			// break;
			
		// case CZMIL_SHALLOW1:
			// waveformData = save_wave.shallow[0];
			// break;
			
		// case CZMIL_SHALLOW2:
			// waveformData = save_wave.shallow[1];
			// break;
			
		// case CZMIL_SHALLOW3:
			// waveformData = save_wave.shallow[2];
			// break;
			
		// case CZMIL_SHALLOW4:
			// waveformData = save_wave.shallow[3];
			// break;
			
		// case CZMIL_SHALLOW5:
			// waveformData = save_wave.shallow[4];
			// break;
			
		// case CZMIL_SHALLOW6:
			// waveformData = save_wave.shallow[5];
			// break;
			
		// case CZMIL_SHALLOW7:
			// waveformData = save_wave.shallow[6];
			// break;
			
		// case CZMIL_IR:
			// waveformData = save_wave.ir;
			// break;	  
	  // }
	  
	  waveformData = save_wave.channel[mapIndex];
	  
	  /* end revision */

	  if (usingMap[mapIndex]) {
	  
		if ((scaledBinHighlighted[mapIndex] > uTimeBound[mapIndex]) ||
			(scaledBinHighlighted[mapIndex] < lTimeBound[mapIndex]))
			chanTxtLbl[mapIndex] = "N/A";
		else
			chanTxtLbl[mapIndex].sprintf ("%3d", waveformData[scaledBinHighlighted[mapIndex]]);
	  
	  }
	  
      binTxtLbl.sprintf ("Bin #%3d", scaledBinHighlighted[mapIndex]);      	  
    }

  NV_INT32 fromTop = 110;
  
  if (showIntMeter && multiWaveMode == 1) 
    { 	  
	  if (usingMap[mapIndex])
		l_map->drawText (chanTxtLbl[mapIndex], w - 35, fromTop - 70, Qt::white, NVTrue);      
    }
  
	static QString lbls[NUM_WAVEFORMS] = {"Deep", "Shal1", "Shal2", "Shal3", "Shal4",
											"Shal5", "Shal6", "Shal7", "IR"};   
											
	int lblXOffset    = 65; 
	int binLblXOffset = 140; 
		
	if (showIntMeter) l_map->drawText ( binTxtLbl, w - binLblXOffset, fromTop - 90, intMeterColor, NVTrue); 
	
  l_map->drawText (lbls[mapIndex], w - lblXOffset, fromTop - 90, Qt::white, NVTrue);
	
  // draw the reference overlay file in the event that we are visualizing them

  if (drawReference) l_map->drawText (refFileLbl, NINT (w * 0.5 - refFileWidth / 2.0), fromTop - 90, referenceColor, NVTrue);

  l_map->drawText ("Time (ns)", NINT (w * 0.5 - timeWidth / 2.0), h - LBL_PIX_SPACING, Qt::white, NVTrue); 
  l_map->drawText ("Counts", LBL_PIX_SPACING, NINT (h * 0.5 - countsWidth / 2.0), 180.0, 12, Qt::white, NVTrue);

  // draw axis lines  
  
  scaleWave (UPPER_TIME_BOUND, LOWER_COUNT_BOUND, &pix_x[0], &pix_y[0], CZMIL_DEEP, l_mapdef);
  
  l_map->drawLine (horLblSpace, h - verLblSpace, pix_x[0], h - verLblSpace, Qt::white, 1, NVFalse, Qt::SolidLine); 
  
  scaleWave (LOWER_TIME_BOUND, UPPER_COUNT_BOUND, &pix_x[0], &pix_y[0], CZMIL_DEEP, l_mapdef);
  
  l_map->drawLine (horLblSpace, h - verLblSpace, horLblSpace, pix_y[0], Qt::white, 1, NVFalse, Qt::SolidLine);   

  //
  // draw grid lines with text:  the 'Y' axis grid marks on the counts 
  // axis run down.  column '0' of the time axis.  The 'X' axis grid 
  // marks on the time axis run along the counts axis at row 0.  Here
  // we insert the ticks with numbers.
  //
  
	/* revised and fixed for new indexing*/
	GetTickSpacing (lTimeBound[mapIndex], uTimeBound[mapIndex], lCountBound[mapIndex], uCountBound[mapIndex], timeSpacing, countSpacing);
	InsertGridTicks (lTimeBound[mapIndex], uTimeBound[mapIndex], 'X', timeSpacing, mapIndex, 0, l_map);
	InsertGridTicks (lCountBound[mapIndex], uCountBound[mapIndex], 'Y', countSpacing, mapIndex, 0, l_map);                

  l_map->setCursor (Qt::ArrowCursor);
}




void 
CZMILwaveMonitor::drawX (NV_INT32 x, NV_INT32 y, NV_INT32 size, NV_INT32 width, QColor color, nvMap * l_map)
{
  NV_INT32 hs = size / 2;
  
  l_map->drawLine (x - hs, y + hs, x + hs, y - hs, color, width, NVTrue, Qt::SolidLine);
  l_map->drawLine (x + hs, y + hs, x - hs, y - hs, color, width, NVTrue, Qt::SolidLine);
}


/* chl revision 08_22_2011 */

void CZMILwaveMonitor::getWaveformData (NV_INT32 mapIndex, NV_U_INT16 * &data, NV_U_BYTE * &ndx,
										CZMIL_CWF_Data * wData) {
	
	/* chl revision 08_26_2011 */
	
	// switch (mapIndex) {
	
		// case CZMIL_DEEP:
			// data = wData->deep;
			// ndx = wData->deep_ndx;
			// break;
		
		// case CZMIL_SHALLOW1:
			// data = wData->shallow[0];
			// ndx = wData->shallow_ndx[0];
			// break;
			
		// case CZMIL_SHALLOW2:
			// data = wData->shallow[1];
			// ndx = wData->shallow_ndx[1];
			// break;
			
		// case CZMIL_SHALLOW3:
			// data = wData->shallow[2];
			// ndx = wData->shallow_ndx[2];
			// break;
			
		// case CZMIL_SHALLOW4:
			// data = wData->shallow[3];
			// ndx = wData->shallow_ndx[3];
			// break;
			
		// case CZMIL_SHALLOW5:
			// data = wData->shallow[4];
			// ndx = wData->shallow_ndx[4];
			// break;
			
		// case CZMIL_SHALLOW6:
			// data = wData->shallow[5];
			// ndx = wData->shallow_ndx[5];
			// break;
			
		// case CZMIL_SHALLOW7:
			// data = wData->shallow[6];
			// ndx = wData->shallow_ndx[6];
			// break;
			
		// case CZMIL_IR:
			// data = wData->ir;
			// ndx = wData->ir_ndx;
			// break;
	// }
	
	data = wData->channel[mapIndex];
	ndx = wData->channel_ndx[mapIndex];

	/* end revision */
}


NV_BOOL CZMILwaveMonitor::trimWaveformNdxs (NV_U_BYTE * ndx, NV_INT32 &start, NV_INT32 &end) {

	NV_BOOL valid = NVTrue;
	
	while (ndx[start] == 0) {	
		start++;
		if (start == CZMIL_MAX_PACKETS) return NVFalse;
	}
	
	while (ndx[end] == 0) {
		end--;
		if (end < 0) return NVFalse;
	}
	
	return valid;
}
	

/* end revision */


/* chl revision 08_18_2011 */
//void CZMILwaveMonitor::drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, NV_INT16 * dat, NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor ) {

/* chl revision 08_22_2011 */
//void CZMILwaveMonitor::drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, NV_U_INT16 * dat, NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor ) {
void CZMILwaveMonitor::drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, CZMIL_CWF_Data * wData, NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor ) {
/* end revision */
/* end revision */	

    NV_BOOL previousValid = NVFalse, currentValid = NVFalse;

    NV_INT32 pix_x[2], pix_y[2], scaledTimeValue, scaledCountValue;		
	
	NV_INT32 uTime = uTimeBound[mapIndex];
	
	/* chl revision 08_22_2011 */
	
	NV_INT32 lTime = lTimeBound[mapIndex];
	NV_INT32 ndxStart = (lTime - 1) / 64; 		// time starts at 1
	NV_INT32 ndxEnd = (uTime - 1) / 64;			// time starts at 1
	
	NV_U_INT16 * dat;
	NV_U_BYTE * ndx;
	
	getWaveformData (mapIndex, dat, ndx, wData);		
	trimWaveformNdxs (ndx, ndxStart, ndxEnd);
		
	for (NV_INT32 i = ndxStart; i <= ndxEnd; i++) {
	
		if (!ndx[i]) {
			previousValid = NVFalse;
			continue;
		}
		
		for (NV_INT32 j = (i * 64); j < (i * 64 + 64); j++) {
		
			if ((dat[j] >= lCountBound[mapIndex]) && (dat[j] <= uCountBound[mapIndex]) &&
				(j >= lTime) && (j <= uTime)) {
				
		
			// for (NV_INT32 i = lTimeBound[mapIndex]; i <= uTime; i++) {

			// if ((dat[i] >= lCountBound[mapIndex]) && (dat[i] <= uCountBound[mapIndex])) {

				currentValid = NVTrue;
				
				// scaledTimeValue = (NV_INT32) (((i - lTimeBound[mapIndex]) / 
									// ((NV_FLOAT32) (uTimeBound[mapIndex] - lTimeBound[mapIndex])) * 
									// (mapLength - 2)) + 1);			
								
				// scaledCountValue = (NV_INT32) ((dat[i] - lCountBound[mapIndex]) / 
									// ((NV_FLOAT32) (uCountBound[mapIndex] - lCountBound[mapIndex])) * 
									// UPPER_COUNT_BOUND);
									
				scaledTimeValue = (NV_INT32) (((j - lTimeBound[mapIndex]) / 
									((NV_FLOAT32) (uTimeBound[mapIndex] - lTimeBound[mapIndex])) * 
									(mapLength - 2)) + 1);			
								
				scaledCountValue = (NV_INT32) ((dat[j] - lCountBound[mapIndex]) / 
									((NV_FLOAT32) (uCountBound[mapIndex] - lCountBound[mapIndex])) * 
									UPPER_COUNT_BOUND);
								
				scaleWave (scaledTimeValue, scaledCountValue, &pix_x[1], &pix_y[1], mapIndex, 
								l_mapdef);
			}

			else currentValid = NVFalse;


			if (wave_line_mode) {
				if (currentValid && previousValid) l_map->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], waveColor, 2, NVFalse, Qt::SolidLine);
			}

			else if (currentValid) l_map->fillRectangle (pix_x[0], pix_y[0], SPOT_SIZE, SPOT_SIZE, waveColor, NVFalse);

			pix_x[0] = pix_x[1];
			pix_y[0] = pix_y[1];
			previousValid = currentValid;
		}	
	}
	
	/* end revision */
}



/* chl revision 08_22_2011 */
void CZMILwaveMonitor::drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, NV_U_INT16 * dat, NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor ) {

	NV_BOOL previousValid = NVFalse, currentValid = NVFalse;
    NV_INT32 pix_x[2], pix_y[2], scaledTimeValue, scaledCountValue;		
	NV_INT32 uTime = uTimeBound[mapIndex];	
	
	for (NV_INT32 i = lTimeBound[mapIndex]; i <= uTime; i++) {

		if ((dat[i] >= lCountBound[mapIndex]) && (dat[i] <= uCountBound[mapIndex])) {

			currentValid = NVTrue;
			scaledTimeValue = (NV_INT32) (((i - lTimeBound[mapIndex]) / 
								((NV_FLOAT32) (uTimeBound[mapIndex] - lTimeBound[mapIndex])) * 
								(mapLength - 2)) + 1);			
								
			scaledCountValue = (NV_INT32) ((dat[i] - lCountBound[mapIndex]) / 
								((NV_FLOAT32) (uCountBound[mapIndex] - lCountBound[mapIndex])) * 
								UPPER_COUNT_BOUND);
								
			scaleWave (scaledTimeValue, scaledCountValue, &pix_x[1], &pix_y[1], mapIndex, 
							l_mapdef);
		}

		else currentValid = NVFalse;


		if (wave_line_mode) {
			if (currentValid && previousValid) l_map->drawLine (pix_x[0], pix_y[0], pix_x[1], pix_y[1], waveColor, 2, NVFalse, Qt::SolidLine);
		}

		else if (currentValid) l_map->fillRectangle (pix_x[0], pix_y[0], SPOT_SIZE, SPOT_SIZE, waveColor, NVFalse);

		pix_x[0] = pix_x[1];
		pix_y[0] = pix_y[1];
		previousValid = currentValid;
	}	
}

/* end revision */


void
CZMILwaveMonitor::slotRestoreDefaults ()
{	

  static NV_BOOL first = NVTrue;

  pos_format = "hdms";
  width = WAVE_X_SIZE;
  height = WAVE_Y_SIZE;
  window_x = 0;
  window_y = 0;  
  
  /* set some more variables to defaults in the event that this module is being
     run for the first time. */
	 
  wave_line_mode = 1;	// turn on lines
  showIntMeter = 0;		// turn off intensity meter
  
  // make all panes visible
  
  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) savedSettings.visible[i] = NVTrue;
  
  savedSettings.saveDirectory = savedSettings.openDirectory = ".";
  
  ccMode = 0; 

  surfaceColor = Qt::yellow;
  primaryColor = Qt::green;

  /* we have a weird situation where first time the app is installed, primaryCursor's
     alpha value is 0, so we will force it to be totatlly opaque. */

  primaryColor.setAlpha (255);  

  secondaryColor = Qt::red;
  backgroundColor = Qt::black;
	
  // added default color of reference overlays

  referenceColor = Qt::darkGray; 

  /* chl revision 08_30_2011 */
  //startup_message = NVTrue;
  startup_message = NVFalse;
  /* end revision */
  
  multiWaveMode = 1;

  //  The first time will be called from envin and the prefs dialog won't exist yet.

  if (!first) setFields ();
  first = NVFalse;

  force_redraw = NVTrue;
}



void
CZMILwaveMonitor::about ()
{

  QMessageBox::about (this, VERSION,
                      "CZMIL Waveform Viewer"
                      "\n\nAuthor : Jan C. Depner (depnerj@navo.navy.mil)");  
}


void
CZMILwaveMonitor::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}



//  Get the users defaults.

void
CZMILwaveMonitor::envin ()
{
  /* chl revision 08_30_2011 */
  //NV_FLOAT64 saved_version = 4.02;
  NV_FLOAT64 saved_version = 4.04;
  /* end revision */


  // Set Defaults so the if keys don't exist the parms are defined

  slotRestoreDefaults ();
  force_redraw = NVFalse;

  QSettings settings (tr ("navo.navy.mil"), tr ("CZMILwaveMonitor"));
  settings.beginGroup (tr ("CZMILwaveMonitor"));

  saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();

  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;
  
  pos_format = settings.value (tr ("position form"), pos_format).toString ();

  width = settings.value (tr ("width"), width).toInt ();

  height = settings.value (tr ("height"), height).toInt ();

  window_x = settings.value (tr ("window x"), window_x).toInt ();

  window_y = settings.value (tr ("window y"), window_y).toInt ();


  startup_message = settings.value (tr ("Display Startup Message"), startup_message).toBool ();


  wave_line_mode = settings.value (tr ("Wave line mode flag"), wave_line_mode).toBool ();

  // 
  // set the inensity meter flag in the settings as well as the multiwave
  // mode (0: NN, 1:SWM)
  //

  showIntMeter = settings.value (tr ("Intensity Meter"), showIntMeter).toBool();
  multiWaveMode = settings.value (tr ("Multiwave Mode"), multiWaveMode).toInt();  
	
  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
	QString label = QString ("Channel %1 Visible").arg(i);
	savedSettings.visible[i] = settings.value (label, true).toBool();
  }  
  
  savedSettings.saveDirectory = settings.value ( tr ("saveDirectory"), QDir(".").canonicalPath() ).toString();
  savedSettings.openDirectory = settings.value ( tr ("openDirectory"), QDir(".").canonicalPath() ).toString();
  
  NV_INT32 red, green, blue, alpha;

  red = settings.value (tr ("Surface color/red"), surfaceColor.red ()).toInt ();
  green = settings.value (tr ("Surface color/green"), surfaceColor.green ()).toInt ();
  blue = settings.value (tr ("Surface color/blue"), surfaceColor.blue ()).toInt ();
  alpha = settings.value (tr ("Surface color/alpha"), surfaceColor.alpha ()).toInt ();
  surfaceColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Primary color/red"), primaryColor.red ()).toInt ();
  green = settings.value (tr ("Primary color/green"), primaryColor.green ()).toInt ();
  blue = settings.value (tr ("Primary color/blue"), primaryColor.blue ()).toInt ();
  alpha = settings.value (tr ("Primary color/alpha"), primaryColor.alpha ()).toInt ();
  primaryColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Secondary color/red"), secondaryColor.red ()).toInt ();
  green = settings.value (tr ("Secondary color/green"), secondaryColor.green ()).toInt ();
  blue = settings.value (tr ("Secondary color/blue"), secondaryColor.blue ()).toInt ();
  alpha = settings.value (tr ("Secondary color/alpha"), secondaryColor.alpha ()).toInt ();
  secondaryColor.setRgb (red, green, blue, alpha);

  red = settings.value (tr ("Background color/red"), backgroundColor.red ()).toInt ();
  green = settings.value (tr ("Background color/green"), backgroundColor.green ()).toInt ();
  blue = settings.value (tr ("Background color/blue"), backgroundColor.blue ()).toInt ();
  alpha = settings.value (tr ("Background color/alpha"), backgroundColor.alpha ()).toInt ();
  backgroundColor.setRgb (red, green, blue, alpha);

  // adding reference color as another thing to remember

  red = settings.value (tr ("Reference color/red"), referenceColor.red ()).toInt ();
  green = settings.value (tr ("Reference color/green"), referenceColor.green ()).toInt ();
  blue = settings.value (tr ("Reference color/blue"), referenceColor.blue ()).toInt ();
  alpha = settings.value (tr ("Reference color/alpha"), referenceColor.alpha ()).toInt ();
  referenceColor.setRgb (red, green, blue, alpha);

  // get the settings for the color code mode

  ccMode = settings.value (tr ("Color code setting"), 0).toInt ();

  this->restoreState (settings.value (tr ("main window state")).toByteArray (), (NV_INT32) (settings_version * 100.0));

  settings.endGroup ();
}




//  Save the users defaults.

void
CZMILwaveMonitor::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();

  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  QSettings settings (tr ("navo.navy.mil"), tr ("CZMILwaveMonitor"));
  settings.beginGroup (tr ("CZMILwaveMonitor"));


  settings.setValue (tr ("settings version"), settings_version);

  settings.setValue (tr ("position form"), pos_format);

  settings.setValue (tr ("width"), width);

  settings.setValue (tr ("height"), height);

  settings.setValue (tr ("window x"), window_x);

  settings.setValue (tr ("window y"), window_y);


  settings.setValue (tr ("Display Startup Message"), startup_message);


  settings.setValue (tr ("Wave line mode flag"), wave_line_mode);  

  settings.setValue (tr ("Intensity Meter"), showIntMeter);
  settings.setValue (tr ("Multiwave Mode"), multiWaveMode);		

  for (NV_INT32 i = 0; i < NUM_WAVEFORMS; i++) {
    QString label = QString ("Channel %1 Visible").arg(i);
	settings.setValue (label, map[i]->isVisible());
  }  

  settings.setValue (tr ("saveDirectory"), savedSettings.saveDirectory );  
  settings.setValue (tr ("openDirectory"), savedSettings.openDirectory );  	

  settings.setValue (tr ("Surface color/red"), surfaceColor.red ());
  settings.setValue (tr ("Surface color/green"), surfaceColor.green ());
  settings.setValue (tr ("Surface color/blue"), surfaceColor.blue ());
  settings.setValue (tr ("Surface color/alpha"), surfaceColor.alpha ());

  settings.setValue (tr ("Primary color/red"), primaryColor.red ());
  settings.setValue (tr ("Primary color/green"), primaryColor.green ());
  settings.setValue (tr ("Primary color/blue"), primaryColor.blue ());
  settings.setValue (tr ("Primary color/alpha"), primaryColor.alpha ());

  settings.setValue (tr ("Secondary color/red"), secondaryColor.red ());
  settings.setValue (tr ("Secondary color/green"), secondaryColor.green ());
  settings.setValue (tr ("Secondary color/blue"), secondaryColor.blue ());
  settings.setValue (tr ("Secondary color/alpha"), secondaryColor.alpha ());

  settings.setValue (tr ("Background color/red"), backgroundColor.red ());
  settings.setValue (tr ("Background color/green"), backgroundColor.green ());
  settings.setValue (tr ("Background color/blue"), backgroundColor.blue ());
  settings.setValue (tr ("Background color/alpha"), backgroundColor.alpha ());
  
  // saving reference color

  settings.setValue (tr ("Reference color/red"), referenceColor.red ());
  settings.setValue (tr ("Reference color/green"), referenceColor.green ());
  settings.setValue (tr ("Reference color/blue"), referenceColor.blue ());
  settings.setValue (tr ("Reference color/alpha"), referenceColor.alpha ());

  // save the color code mode setting

  settings.setValue (tr ("Color code setting"), ccMode);
  
  settings.setValue (tr ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}
