//
// source:		attributeViewer.cpp
// author:		Gary Morris
//
// This source code will support the Attribute Viewer module which will interface
// to the CZMIL Manual Editor's editing interfaces.  Shots will be received via shared
// memory and the Attribute Viewer will display these shots in attribute form along with
// statistical anaylysis and plotting mechanisms.
//


// include files

#include "attributeViewer.hpp"

/* chl revision 08_24_2011 */
//NV_FLOAT64 settings_version = 1.09;
NV_FLOAT64 settings_version = 1.11;
/* end revision */


//
// construct:		attributeViewer
//
// This constructor intializes the main window, adds the layouts of the main window, brings
// in saved settings, sets up signal/slot relationships, initializes class members, etc.
//

attributeViewer::attributeViewer (NV_INT32 *argc, NV_CHAR **argv, QWidget * parent) : QMainWindow (parent, 0) {

	extern char     *optarg;		

	lock_track = NVFalse;

	selecting = NVFalse;	

	QResource::registerResource ("/icons.rcc");

	//  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

	setFocusPolicy (Qt::WheelFocus);
	setFocus (Qt::ActiveWindowFocusReason);

	// set the default font and the preferences dialog to NULL

	QFont avFont ("Helvetica", 12, QFont::Normal);
	setFont (avFont);

	attPrefs = NULL;	
	spAxesDlg = NULL;	
	scaleDlg = NULL;
	
	//  Set the main icon

	setWindowIcon (QIcon (":/icons/attributeviewer.png"));

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
			}
			break;
		}
	}


	envin ();
	
	avData.setSpatialDimSize (spatialDimSize);
	
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

	for (NV_INT32 pfm = 0 ; pfm < num_pfms ; pfm++) {

		open_args[pfm] = abe_share->open_args[pfm];

		if ((pfm_handle[pfm] = open_existing_pfm_file (&open_args[pfm])) < 0) {

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

	QFrame *frame = new QFrame (this, 0);

	setCentralWidget (frame);	

	QVBoxLayout *vBox = new QVBoxLayout ();
	vBox->setSpacing(0);
	frame->setLayout (vBox);

	//  Setup the file menu.

	QAction *fileQuitAction = new QAction (tr ("&Quit"), this);

	fileQuitAction->setStatusTip (tr ("Exit from application"));
	connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

	// set up the "Preferences" menu item

	QAction *filePrefsAction= new QAction (tr ("&Preferences"), this);
	filePrefsAction->setStatusTip (tr ("Set Preferences"));
	connect (filePrefsAction, SIGNAL (triggered ()), this, SLOT (slotPrefs ()));

	QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));

	fileMenu->addAction (filePrefsAction);
	fileMenu->addSeparator ();

	fileMenu->addAction (fileQuitAction);	

	modeProfileAction = new QAction (tr ("&Profile"), this);
	modeProfileAction->setStatusTip ("Diplay Data Profile");
	modeProfileAction->setCheckable (true);
	connect (modeProfileAction, SIGNAL (triggered()), this, SLOT (slotGraphModeAction()));

	modeScatterPlotAction = new QAction (tr ("&ScatterPlot"), this);
	modeScatterPlotAction->setStatusTip ("Diplay Data ScatterPlot");
	modeScatterPlotAction->setCheckable (true);
	connect (modeScatterPlotAction, SIGNAL (triggered()), this, SLOT (slotGraphModeAction()));

	modeHistogramAction = new QAction (tr ("&Histogram"), this);
	modeHistogramAction->setStatusTip ("Display Data Histogram");
	modeHistogramAction->setCheckable (true);
	connect (modeHistogramAction, SIGNAL (triggered()), this, SLOT (slotGraphModeAction()));

	QActionGroup * modeAG = new QActionGroup (this);
	modeAG->addAction (modeProfileAction);  
	modeAG->addAction (modeScatterPlotAction);  
	modeAG->addAction (modeHistogramAction);

	if (graphModeEnvIn == GM_profile) modeProfileAction->setChecked(NVTrue);
	else if ( graphModeEnvIn == GM_scatterPlot ) modeScatterPlotAction->setChecked(NVTrue);
	else modeHistogramAction->setChecked(NVTrue);

	QMenu *graphMenu = menuBar ()->addMenu (tr ("&Graph"));
	graphMenu->addAction (modeProfileAction);
	graphMenu->addAction (modeScatterPlotAction);
	graphMenu->addAction (modeHistogramAction);  

	scaleAbsoluteAction = new QAction (tr ("&Absolute"), this);
	scaleAbsoluteAction->setStatusTip ("Scales data to predefined values");
	scaleAbsoluteAction->setCheckable (true);
	connect (scaleAbsoluteAction, SIGNAL (triggered()), this, SLOT (slotGraphScaleAction()));

	scaleROIAction = new QAction (tr ("&ROI"), this);
	scaleROIAction->setStatusTip ("Scales data relative to region of interest");
	scaleROIAction->setCheckable (true);
	connect (scaleROIAction, SIGNAL (triggered()), this, SLOT (slotGraphScaleAction()));
	
	scaleUserAction = new QAction (tr ("&User Defined"), this);
	scaleUserAction->setStatusTip ("Scales data based on the user's preference");
	scaleUserAction->setCheckable (true);
	connect (scaleUserAction, SIGNAL (triggered()), this, SLOT (slotGraphScaleAction()));
	
	QActionGroup * scalingAG = new QActionGroup (this);
	scalingAG->addAction (scaleAbsoluteAction);
	scalingAG->addAction (scaleROIAction);	
	scalingAG->addAction (scaleUserAction);	
	
	if (graphProps.scale == GS_Absolute) scaleAbsoluteAction->setChecked(NVTrue);
	else if (graphProps.scale == GS_Roi) scaleROIAction->setChecked(NVTrue);
	else scaleUserAction->setChecked(NVTrue);	
	
	QMenu *scalingMenu = new QMenu (tr ("&Scaling"));
	
	scalingMenu->addAction (scaleAbsoluteAction);
	scalingMenu->addAction (scaleROIAction);	
	scalingMenu->addAction (scaleUserAction);
	
	scalingMenu->addSeparator();
	
	QMenu * userDefinedScalesMenu = new QMenu(tr ("User Defined Scales"));
	
	scaleProfileDef = new QAction ("", this);
	scaleScatterXDef = new QAction ("", this);
	scaleScatterYDef = new QAction ("", this);

	setUserDefinedActionDefs();
	
	userDefinedScalesMenu->addAction (scaleProfileDef);
	userDefinedScalesMenu->addAction (scaleScatterXDef);
	userDefinedScalesMenu->addAction (scaleScatterYDef);
	
	scalingMenu->addMenu(userDefinedScalesMenu);
	
	QAction * changeUserDefinedAction = new QAction ("Change User Defined Scales", this);
	connect (changeUserDefinedAction, SIGNAL (triggered()), this, SLOT (slotChangeUserDefinedScales()));
		
	scalingMenu->addAction (changeUserDefinedAction);
	
	overplotOnAction = new QAction (tr ("&On"), this);
	overplotOnAction->setStatusTip ("Turn overplotting on");
	overplotOnAction->setCheckable (true);
	connect (overplotOnAction, SIGNAL (triggered()), this, SLOT (slotGraphOverplotAction()));

	overplotOffAction = new QAction (tr ("O&ff"), this);
	overplotOffAction->setStatusTip ("Turn overplotting off");
	overplotOffAction->setCheckable (true);
	connect (overplotOffAction, SIGNAL (triggered()), this, SLOT (slotGraphOverplotAction()));

	QActionGroup * overplotAG = new QActionGroup (this);
	overplotAG->addAction (overplotOnAction);
	overplotAG->addAction (overplotOffAction);

	if (overplotOnEnvIn) overplotOnAction->setChecked(NVTrue);
	else overplotOffAction->setChecked(NVTrue);
	
	QMenu *overplotMenu = new QMenu (tr ("&Overplotting"));
	
	overplotMenu->addAction (overplotOnAction);
	overplotMenu->addAction (overplotOffAction);

	selectionPolyAction = new QAction (tr ("&Polygon"), this);
	selectionPolyAction->setStatusTip ("Select shots with a polygon");
	selectionPolyAction->setCheckable (true);
	connect (selectionPolyAction, SIGNAL (triggered()), this, SLOT (slotGraphSelectionAction()));

	selectionRectAction = new QAction (tr ("&Rectangle"), this);
	selectionRectAction->setStatusTip ("Select shots with a rectangle");
	selectionRectAction->setCheckable (true);
	connect (selectionRectAction, SIGNAL (triggered()), this, SLOT (slotGraphSelectionAction()));

	QActionGroup *selectionAG = new QActionGroup (this);
	selectionAG->addAction (selectionPolyAction);
	selectionAG->addAction (selectionRectAction);

	if (selectionPolyEnvIn) selectionPolyAction->setChecked(NVTrue);
	else selectionRectAction->setChecked(NVTrue);
	
	QMenu *selectionMenu = new QMenu (tr ("S&election") );
	
	selectionMenu->addAction (selectionPolyAction);
	selectionMenu->addAction (selectionRectAction);

	/* make one Options menu choice with the Graph, Scaling, Overplotting, and Selection */
	/* menus as submenus */

	QMenu * optionsMenu = menuBar()->addMenu (tr ("&Options"));
	optionsMenu->addMenu (scalingMenu);
	
	/* chl revision 08_24_2011 */
	/* let's take overplotting off until we have a sufficient GUI system */
	//optionsMenu->addMenu (overplotMenu);
	/* end revision */
	
	optionsMenu->addMenu (selectionMenu);
	optionsMenu->addSeparator ();

	QAction * scatterAxesAction = new QAction (tr ("Change Scatterplot Axes"), this);
	connect (scatterAxesAction, SIGNAL (triggered ()), this, SLOT (slotChangeScatterAxes ()));

	optionsMenu->addAction (scatterAxesAction);		

	/*this function eventually call InitGraphProps which depends on menu 
	setting for overplotting, absolute scaling and graph mode (profile or histogram)*/
	AddAttributeScreen((QBoxLayout *)vBox);			

	//  Setup the help menu.  I like leaving the About Qt in since this is
	//  a really nice package - and it's open source.

	QAction *aboutAct = new QAction (tr ("&About"), this);
	aboutAct->setShortcut (tr ("Ctrl+A"));
	aboutAct->setStatusTip (tr ("Information about attributeViewer"));
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

	graphContextMenu = new QMenu (this);
	QAction *graphContextDelete = graphContextMenu->addAction (tr ("Delete"));
	connect (graphContextDelete, SIGNAL (triggered ()), this, SLOT (slotGraphDelete ()));

	graphSelectionContextMenu = new QMenu (this);
	QAction *graphSelectionContextDeleteAll = graphSelectionContextMenu ->addAction (tr ("Delete All"));
	QAction *graphSelectionContextCancel    = graphSelectionContextMenu ->addAction (tr ("Cancel"));
	connect (graphSelectionContextDeleteAll, SIGNAL (triggered ()), this, SLOT (slotGraphDeleteAll ()));
	connect (graphSelectionContextCancel   , SIGNAL (triggered ()), this, SLOT (slotGraphCancel    ()));

	//  Setup the tracking timer
	QTimer *track = new QTimer (this);
	connect (track, SIGNAL (timeout ()), this, SLOT (trackCursor ()));
	track->start (10);

	QTimer *halfSecond = new QTimer (this);
	connect (halfSecond , SIGNAL (timeout ()), this, SLOT (slotHalfSecond()));
	halfSecond->start(500);
	
	//  Let the other program(s) know that we are running.

	abeShare->lock ();
	
	/* chl revision 08_25_2011 */
	/* if there is no HOF AV */
	//abe_share->avShare.avRunning |= 0x02;  	
	abe_share->avShare.avRunning |= 0x01;  	
	/* end revision */
	
	/* if the default combo box attribute is a linkable entry, go ahead and send */
	
	NV_INT32 nameIndex;
	
	if (isAttributeLinkable (cbAOI->currentIndex(), nameIndex)) {
		SaveAVActionToShare (AVA_COLOR_LINK, &nameIndex, 1);
	}
		
	abeShare->unlock ();  		 
}


//
// deconstruct:		~attributeViewer
//
// The deconstructor does nothing.
//

attributeViewer::~attributeViewer () {
}


//
// method:		DisplayData
//
// This method will ingest the data index which refers to the attribute that is in focus
// from the combo box and display labels depending on the plotting mode (profile, scatterplot,
// or histogram).  The canvas plot area will also be refreshed.
//

void attributeViewer::DisplayData(int dataIndex) {

	DisplayLabels(dataIndex);		
	graph->redrawMapArea(NVTrue);		
}


//
// method:		DisplayLabels
//
// This method will set the stat labels (mean, max/min, standard deviation) of the current
// attribute.  These values have been calculated elsewhere.  This method simply displays
// them.
//

void attributeViewer::DisplayLabels(int dataIndex) {

	/* account for no values in the AV */
    
	if (avData.GetNumItems() == 0) {
		
		lblCenterPixel->setText ("No Value");
		lblMean->setText ("No Value");
		lblMin->setText ("No Value");
		lblMax->setText ("No Value");
		lblSD->setText ("No Value");		
	}
	else {	
	
		/* account for special situation of the bin/used attributes that may */
		/* contain invalid 0's */
		
		if (avData.isBinAttribute (dataIndex)) {
		
			NV_INT32 val = (NV_INT32)(avData.GetCenterPixel()[dataIndex]);
			
			if (val == 0) lblCenterPixel->setText ("0 (invalid)");
			else lblCenterPixel->setNum (val);
			
			if ((NV_INT32)(avData.GetMin(dataIndex)) == 0) {
				lblMin->setText ("n/a");
				lblMax->setText ("n/a");
			}
			else {
				lblMin->setNum(avData.GetMin(dataIndex));
				lblMax->setNum(avData.GetMax(dataIndex));
			}
			
			if ((NV_INT32)(avData.GetMean()) == 0) {
				lblMean->setText ("n/a");
				lblSD->setText("n/a");
			}
			else {
				lblMean->setNum(avData.GetMean());
				lblSD->setNum(avData.GetStd());
			}
		}
		else if (avData.isDepthAttribute (dataIndex)) {
		
			NV_FLOAT32 val = (NV_FLOAT32)(avData.GetCenterPixel()[dataIndex]);
			
			if ((NV_INT32)(val - 0.5) == -998) lblCenterPixel->setText ("-998 (invalid)");
			else lblCenterPixel->setNum (val);
			
			if ((NV_INT32)(avData.GetMin(dataIndex) - 0.5) == -998) {
				lblMin->setText ("n/a");
				lblMax->setText ("n/a");
			}
			else {
				lblMin->setNum(avData.GetMin(dataIndex));
				lblMax->setNum(avData.GetMax(dataIndex));
			}
			
			if ((NV_INT32)(avData.GetMean() - 0.5) == -998) {
				lblMean->setText ("n/a");
				lblSD->setText ("n/a");
			}
			else {
				lblMean->setNum(avData.GetMean());
				lblSD->setNum(avData.GetStd());
			}
		}
		else {		
			lblCenterPixel->setNum( avData.GetCenterPixel()[dataIndex] ); // tag001
			lblMean->setNum(avData.GetMean());
			lblMin->setNum(avData.GetMin(dataIndex));
			lblMax->setNum(avData.GetMax(dataIndex));
			lblSD->setNum(avData.GetStd());			
		}		
	}	
}  


//
// method:		DrawGraph
//
// This method will actually draw elements within the graphing window such as the labels
// and lines as well as the profile, scatterplot, or histogram.
//

void attributeViewer::DrawGraph(NVMAP_DEF graphMapDef __attribute__ ((unused))) {

	/* if we are not done loading and we have items, get out */
	
	if (avData.GetNumItems() != 0 && !avData.IsLoaded()) return;
	
	/* still want to see graph lines even if there are no shots left */

	static NV_INT32 numShots;		
	numShots = avData.GetNumItems();	
	
	DrawGraphLabelsAndLines();	

	static int dataIndex;
	dataIndex = cbAOI->currentIndex();
	
	/* separate out the profile from the scatterplot because overplotting is not allowed */
	/* within the scatterplot.  There are some other subtle differences that warrant a */
	/* new funtion solely dedicated to the scatterplot */	

	if (graphProps.mode == GM_profile) DrawShotsGraph(dataIndex, numShots);
	else if (graphProps.mode == GM_scatterPlot) DrawScatterplot (numShots);
	else DrawHistogram(dataIndex);		
}


//
// method:	DrawScatterplot
//
// This method will draw the axes lines, shots, and possibly selection for the scatterplot.
//

void attributeViewer::DrawScatterplot (NV_INT32 numShots) {

    static NV_INT32 numBotTicks = 11;	
	
	GraphScale scale;
	
	if (scaleAbsoluteAction->isChecked()) scale = GS_Absolute;	
	else if (scaleUserAction->isChecked()) scale = GS_User_Defined;	
	else scale = GS_Roi;
	
	// draw the left axis
	DrawLeftScale (avData.GetScaleMin (graphProps.scatterYIndex, scale, &graphProps, 1), 
					avData.GetScaleMax (graphProps.scatterYIndex, scale, &graphProps, 1), 2, 11, 
					graphProps.h, graphProps.drawHeight, graphProps.leftMargin, 
					graphProps.bottomMargin, graphProps.leftTickW, graphProps.lblPixSpacing, 
					graphProps.pt8Height, *graphProps.smFm);		
					
	// draw the bottom axis
	DrawBottomScale(avData.GetScaleMin (graphProps.scatterXIndex, scale, &graphProps, 0), 
					avData.GetScaleMax (graphProps.scatterXIndex, scale, &graphProps, 0), numBotTicks, 1, 2, 
					graphProps.drawWidth, graphProps.leftMargin, 
					graphProps.h - graphProps.bottomMargin, graphProps.bottomMargin, 
					graphProps.bottomTickH, graphProps.lblPixSpacing, graphProps.pt8Height, 
					*graphProps.smFm );	

	DrawShots(NUM_ATTRIBUTES, numShots, Qt::white);

	DrawSelection ();
}


//
// method:		DrawGraphLabelsAndLines
//
// This method will draw the left and bottom labels defining the axes.  It will also draw
// the actual axes lines.
//

void attributeViewer::DrawGraphLabelsAndLines() {

	// draw left label
	graph->drawText ( graphProps.leftLbl, 
		graphProps.pt12Height + graphProps.lblPixSpacing
		, NINT(graphProps.h/2.0 - graphProps.bottomMargin/2.0 + graphProps.leftLblW/2.0 + graphProps.topMargin/2.0)
		, 0.0, 12, Qt::white, NVTrue);		

	// draw right label
	graph->drawText ( graphProps.bottomLbl
		, NINT ( graphProps.leftMargin + (graphProps.w-graphProps.leftMargin-graphProps.rightMargin) * 0.5 - graphProps.bottomLblW / 2.0)
		, graphProps.h - graphProps.lblPixSpacing, Qt::white, NVTrue); 	
		
	graph->drawLine ( graphProps.leftMargin
		, graphProps.h-graphProps.bottomMargin + ((graphProps.mode != GM_histogram)?0:1)
		, graphProps.w-graphProps.rightMargin
		, graphProps.h-graphProps.bottomMargin + ((graphProps.mode != GM_histogram)?0:1)
		, Qt::red, 1, NVFalse, Qt::SolidLine);

	// draw y axis line
	graph->drawLine ( graphProps.leftMargin - ((graphProps.mode != GM_histogram)?0:1)
		, graphProps.h-graphProps.bottomMargin
		, graphProps.leftMargin - ((graphProps.mode != GM_histogram)?0:1)
		, graphProps.pt12Height
		, Qt::red, 1, NVFalse, Qt::SolidLine);	
}


//
// method:		DrawHistogram
//
// This method will draw the tick marks and scale markers of the left and bottom axes.
// The actual bars are drawn for the intervals and the null tick, null bar, and null count
// is also rendered here.
//

void attributeViewer::DrawHistogram( int dataIndex ) {

	avData.LoadHistogramData( histInterval, dataIndex);
	
	static AVHistItem *histItem;		histItem  = avData.GetHistItem(dataIndex);
	static NV_INT32 * counts;				counts = histItem->GetCounts();
	static NV_INT32 countMax;				countMax = histItem->GetCountScaleMax();
	static NV_INT32 numIntervals;		numIntervals = histItem->GetNumIntervals();
	static NV_FLOAT32 minScale;			minScale  = histItem->GetScaleMin();
	static NV_FLOAT32	maxScale;			maxScale  = histItem->GetScaleMax();
	static int nullW;								nullW = 3;
	static QString lblNull = QString("Null");
	static int lblNullW = graphProps.smFm->width(lblNull);
	static int nullX;								nullX = graphProps.leftMargin + graphProps.drawWidth - 2;
	static int nullSpace;						nullSpace = (int)ceil( graphProps.fm->width(QString("%1").arg(maxScale,0,'f', 2))/2.0f + graphProps.lblPixSpacing + lblNullW/2.0f );
	static NV_FLOAT32 barWidth;			barWidth = (graphProps.drawWidth - nullSpace) / ((NV_FLOAT32)numIntervals);
	static NV_INT32 numBotTicks = 11;
	
	DrawLeftScale(.0f, countMax, 0, numBotTicks, graphProps.h, graphProps.drawHeight, graphProps.leftMargin, graphProps.bottomMargin
		, graphProps.leftTickW, graphProps.lblPixSpacing, graphProps.pt8Height, *graphProps.smFm);

	DrawBottomScale(  minScale, maxScale, numBotTicks, 1, 2, graphProps.drawWidth - nullSpace
		, graphProps.leftMargin, graphProps.h-graphProps.bottomMargin, graphProps.bottomMargin
		, graphProps.bottomTickH, graphProps.lblPixSpacing, graphProps.pt8Height, *graphProps.smFm );

	static int bh;
	static QString lblBh;
	static int barX;

	for (int i=0; i<numIntervals; i++) {
		bh = (int)(graphProps.drawHeight * counts[i]/(NV_FLOAT32)countMax);
		barX = graphProps.leftMargin + (int)(i*barWidth);
		graph->fillRectangle (barX , graphProps.h-graphProps.bottomMargin, (int)barWidth, -bh , histBarColor, NVTrue );
		graph->drawRectangle (barX , graphProps.h-graphProps.bottomMargin, barX + (int)barWidth, graphProps.h - graphProps.bottomMargin - bh
			, histBarOutlineColor, 1, Qt::SolidLine, NVTrue );
	}

	// plot NULL count	
	if (counts[numIntervals]>0 ) // check prevents a dot from being drawn when 0
		graph->drawLine ( nullX, graphProps.h-graphProps.bottomMargin, nullX
		, graphProps.h - graphProps.bottomMargin - (int)(graphProps.drawHeight * counts[numIntervals]/(NV_FLOAT32)countMax)
		, histNullColor, nullW, NVFalse, Qt::SolidLine);

	// draw NULL tick
	graph->drawLine ( nullX, graphProps.h-graphProps.bottomMargin, nullX
		, graphProps.h-graphProps.bottomMargin + graphProps.bottomTickH, histNullColor, 1, NVFalse, Qt::SolidLine);

	// draw Null label
	graph->drawText ( lblNull, NINT(nullX - lblNullW/2.0f)
		, graphProps.h - graphProps.bottomMargin + graphProps.bottomTickH + graphProps.lblPixSpacing + graphProps.pt8Height
		, 90.0, 8, histNullColor, NVTrue);  
}


//
// method:		DrawShotsGraph
//
// This method will essentially draw the individual shots within the drawing canvas.  Overplotting
// is handled here; however, at the moment, it is disabled.  Also if a selection has
// been started it is handled here as well.
//

void attributeViewer::DrawShotsGraph( int dataIndex, int numShots ) {
	
	static QColor colors[] = { Qt::white, Qt::blue, Qt::red, Qt::green };	

	DrawLeftScale( (graphProps.overplot)? .0f : graphProps.minScale, (graphProps.overplot)?1.0f : graphProps.maxScale, 2 , 11, graphProps.h, graphProps.drawHeight
		, graphProps.leftMargin, graphProps.bottomMargin, graphProps.leftTickW, graphProps.lblPixSpacing, graphProps.pt8Height, *graphProps.smFm);

	if (graphProps.overplot) 
		for (int di=0; di < NUM_ATTRIBUTES; di++) 
			if (di != dataIndex ) 
				DrawShots( di, numShots, colors[di]  );	

	DrawShots( dataIndex, numShots, colors[dataIndex]  ); // ensure that selected dataindex is on top

	DrawSelection();	
}


//
// method:			isShotFiltered
//
// This method will look to see if there are -1000000's in the screen position fields of the item.  This
// corresponds to a filter.
//

NV_BOOL attributeViewer::isShotFiltered (AVDataItem * item, NV_INT32 attIndex) {
	
	NV_BOOL result = NVFalse;
	
	if (item->xs[attIndex] == -1000000 && item->ys[attIndex] == -1000000) result = NVTrue;
	
	return result;
}


//
// method:			isAttributeLinkable 
//
// This method will ingest an index of the combo box and check it's title with the titles
// of the ndx_attr_name array buried down in the misc structure which holds the user-defined
// attributes.  If the combo box selection is a user-defined attribute return a true.
//

NV_BOOL attributeViewer::isAttributeLinkable (NV_INT32 index, NV_INT32 &resultNameIndex) {

	NV_BOOL result = NVFalse;
	resultNameIndex = -1;

	for (NV_INT32 j = 0 ; j < NUM_ATTR ; j++) {
			
		if (QString::compare (cbAOI->itemText(index), 
							QString (abe_share->open_args[0].head.ndx_attr_name[j])) == 0) {
			cbAOI->setItemIcon (index, QIcon (":/icons/chainLink.png"));
			result = NVTrue;
			resultNameIndex = j;
			
			break;
		}
	}
	
	return result;
}


//
// method:			setUserDefinedActionDefs
//
// This method will set the text for the 3 menu choices detailing the user scales.
//

void attributeViewer::setUserDefinedActionDefs () {

	QString msg;
	
	msg = QString ("%1 - min:  %2\nMax:  %3").arg("Profile", 9).arg(graphProps.userMinScale, 6, 'f', 2).arg(graphProps.userMaxScale, 6, 'f', 2);
	scaleProfileDef->setText (msg);
	
	msg = QString ("%1 - min:  %2\nMax:  %3").arg("Scatter X", 9).arg(graphProps.userScatXMinScale, 6, 'f', 2).arg(graphProps.userScatXMaxScale, 6, 'f', 2);
	scaleScatterXDef->setText (msg);
	
	msg = QString ("%1 - min:  %2\nMax:  %3").arg("Scatter Y", 9).arg(graphProps.userScatYMinScale, 6, 'f', 2).arg(graphProps.userScatYMaxScale, 6, 'f', 2);
	scaleScatterYDef->setText (msg);	
}


//
// method:		DrawShots
//
// All shots that pass the filter of not being invalid in some way will get rendered
// within the canvas area.  The shot is represented by a circle.  The highlighted shot
// (the one that the mouse cursor is closest to) gets another outlined circle rendered
// around the original circle to create a highlight affect.
//

void attributeViewer::DrawShots( int dataIndex, int numShots, QColor color ) {

	if (numShots == 0) return;
	
	/* color is ignored coming in if this is not an overplot */

	for (int i = 0; i < numShots; i++ ) {
	
		/* We look to see if this particular shot has been filtered by the bin index 
		   of 0 filter and others.  If it has, we disregard it. */
		
		if (isShotFiltered(&(avData[i]), dataIndex))
			continue;	// do not draw it, disregard		

		if (!graphProps.overplot) 
			color = QColor::fromHsv (avData[i].colorH, avData[i].colorS, avData[i].colorV);

		graph->fillCircle (avData[i].xs[dataIndex], avData[i].ys[dataIndex], graphProps.circleDiameter, .0f, 360.0f, color, NVTrue);
	}
		
	/* instead of checking each time in for loop simply draw a bigger circle for the highlight and redraw highlighted shot so its on top */
	if ( avData.highlightedShotIndex > -1 ) {
		
		graph->fillCircle (avData[avData.highlightedShotIndex].xs[dataIndex], avData[avData.highlightedShotIndex ].ys[dataIndex], graphProps.highlightCircleDiameter, .0f, 360.0f, Qt::white, NVTrue );
		
		if (!graphProps.overplot)
			graph->fillCircle (avData[avData.highlightedShotIndex].xs[dataIndex], avData[avData.highlightedShotIndex ].ys[dataIndex], 
								graphProps.circleDiameter, .0f, 360.0f, 
								QColor::fromHsv (avData[avData.highlightedShotIndex].colorH, avData[avData.highlightedShotIndex].colorS, 
								avData[avData.highlightedShotIndex].colorV), NVTrue );
		else
			graph->fillCircle (avData[avData.highlightedShotIndex].xs[dataIndex], avData[avData.highlightedShotIndex ].ys[dataIndex], 
								graphProps.circleDiameter, .0f, 360.0f, color, NVTrue );		
	}
}


//
// method:		select
//
// This method will check to see which screen coordinates of which shots are within the
// polygonal or rectangular selection area.  The master indexes of the shots which
// relates them to the point cloud within the editor are saved within the selection
// array.  
//

void AVData::select( GraphProps &graphProps __attribute__ ((unused)), int dataIndex, NV_INT32 count, NV_INT32 *xs, NV_INT32 *ys, NV_BOOL isPoly ) {

	/* have to account for the next marker spot */
	selection = (NV_INT32 *) malloc ((numItems + 1) * sizeof (NV_INT32));
	
	numSelected = 0;

	int firstSelected = -1, lastSelected = -1, firstNonSelected = -1;

	for (int i = 0; i < numItems; i++ ) {

		if ( inside(xs, ys, count, items[i]->xs[dataIndex], items[i]->ys[dataIndex], isPoly )  ) {

			selection[ numSelected++ ] = items[i]->masterIdx;
			if (firstSelected == -1 )  firstSelected = i;
			lastSelected = i;

		} else if ( firstNonSelected == -1 ) firstNonSelected = i;

	}

	// set last element so editor know which one to highlight
	selection[numSelected] = 
		( firstSelected > 0 ) ? items[firstSelected - 1]->masterIdx 
		: ( lastSelected > -1 && lastSelected < numItems - 1 ) ? items[lastSelected+1]->masterIdx 
		: ( firstNonSelected > -1 ) ? items[firstNonSelected]->masterIdx
		: -1;
}



//
// method:			lookForNeighbors
//
// This method will apply the slotting algorithm on the data entries within the Attribute 
// Viewer and return the shot indices of the neighbors.  A -1 as an index means there was
// no neighbor in that spatial cell.  We are looking for the eight neighbors where array
// index 0 = northwest, 1 = due north, 2 = northeast, 3 = due west, 4 = due east, 
// 5 = southwest, 6 = due south, 7 = southeast
//

NV_INT32 * AVData::lookForNeighbors () {

	/* allocate maximum number of cells. ordering will take care of itself */

	NV_INT32 * neighbors = new NV_INT32[NUM_SPATIAL_TBL_INDICES];
	NV_INT32 centerX, centerY;
	
	for (int i = 0; i < NUM_SPATIAL_TBL_INDICES; i++) neighbors[i] = -1;

	NV_FLOAT64 dummyZ = 0.0;
	
	NV_INT32 screenZ;
	refMap->map_to_screen (1, &items[highlightedShotIndex]->longitude, &items[highlightedShotIndex]->latitude, &dummyZ, &centerX, &centerY, &screenZ);

	AVDataItem ** sortedItems = sortByDistanceFromScreenCoord (centerX, centerY);

	updateNeighborTable (sortedItems, neighbors, centerX, centerY);

	NV_INT16 * tableMap, midPoint;
	getTableMapAndMidpoint (tableMap, midPoint);
	
	for (NV_INT32 i = 0; i < (spatialDimSize * spatialDimSize - 1); i++) {
		if (tableMap[i] < midPoint) neighborIdxs[tableMap[i]] = neighbors[i];
		else neighborIdxs[tableMap[i] + 1] = neighbors[i];
	}

	/* set the primary pixel in the center */
	neighborIdxs[midPoint] = highlightedShotIndex;
	
	/* free sorted list memory */

	for (NV_INT32 i = 0; i < numItems - 1; i++) free (sortedItems[i]);
	free (sortedItems);

	return neighbors;
}


//
// method:			getTableMapAndMidpoint
//
// This method will return the table indexing map based on the dimensions of the
// neighbor table.  The midpoint which references the index of the cell in the
// table which is the middle will also be returned.
//

void AVData::getTableMapAndMidpoint (NV_INT16 * &tableMap, NV_INT16 &midPoint) {

	if (spatialDimSize == 3) {
		midPoint = 4;
		tableMap = &(tableMapping3x3[0]);
	}
	else if (spatialDimSize == 5) {
		midPoint = 12;
		tableMap = &(tableMapping5x5[0]);
	}
	else {
		midPoint = 24;
		tableMap = &(tableMapping7x7[0]);
	}
}


//
// method:			sortByDistanceFromScreenCoord
//
// This method will load up the items list of AVData and sort all except that of
// the primary pixel by distance from the screen coordinate passed in.  It will 
// return a sorted list out.  
//
// Note:  The memory of this list should be eventually freed externally.
//

AVDataItem ** AVData::sortByDistanceFromScreenCoord (NV_INT32 ppX, NV_INT32 ppY) {

	AVDataItem ** sortedItems;

	sortedItems = (AVDataItem **) malloc ((numItems - 1) * sizeof (AVDataItem *));
	NV_FLOAT32 * dist = (NV_FLOAT32 *) malloc ((numItems - 1) * sizeof (NV_FLOAT32));

	for (NV_INT32 i = 0; i < (numItems - 1); i++) 
		sortedItems[i] = (AVDataItem *) malloc (sizeof (AVDataItem));

	int counter = 0;
	NV_FLOAT64 dummyZ = 0.0;
	NV_INT32 screenX, screenY, screenZ;

	Vector centerPt = Vector (ppX, ppY, 0.0f);
	Vector pt;

	// load existing list
	for (NV_INT32 i = 0; i < numItems; i++) {

		if (highlightedShotIndex != i) {
			
			*(sortedItems[counter]) = *(items[i]);
			refMap->map_to_screen (1, &items[i]->longitude, &items[i]->latitude, &dummyZ, 
									&screenX, &screenY, &screenZ);
			pt.Set (screenX, screenY, screenZ);

			dist[counter] = Vector::GetDistance (centerPt, pt);
			counter++;
		}
	}

	// use simple selection sort to sort array 
	//(not a ton of values so this sort is feasible)

	NV_FLOAT32 temp;
	AVDataItem temp2;

	for (NV_INT32 i = 0; i < numItems - 1; i++) {

		NV_INT32 min = i;

		for (NV_INT32 j = i + 1; j < numItems - 1; j++) {
			if (dist[j] < dist[min])
				min = j;
		}

		temp = dist[i];
		temp2 = *(sortedItems[i]);
		dist[i] = dist[min];
		*(sortedItems[i]) = *(sortedItems[min]);
		dist[min] = temp;
		*(sortedItems[min]) = temp2;
	}	

	return sortedItems;
}


//
// method:		DrawSelection
//
// This method will draw the polygon of the selection or rectangle.  It depends on the
// selection type that the user currently has selected.
//

void attributeViewer::DrawSelection() {

	if (selecting ) {
		if (graphProps.selectPoly) poly.draw(graph);
		else graph->drawRectangle (rectXs[0], rectYs[0], rectXs[1], rectYs[1], Qt::white, 2, Qt::SolidLine, NVFalse);
	}
}

//
// method:		DrawBottomScale
//
// This method will draw the tick marks and labels based on the number of tick marks
// desired and the label intervals of the bottom or X axis.
//

void attributeViewer::DrawBottomScale(NV_FLOAT32 minScale, NV_FLOAT32 maxScale, int numTicks, int lblInterval, int scalePrecision, int drawWidth, int left, int top, int bottomMargin __attribute__ ((unused)), int tickH, int lblPixSpacing, int fontHeight, QFontMetrics fm) {

	static QString lbl;
	static int tickX;
	static NV_FLOAT32 scaleRange; scaleRange = maxScale - minScale;

	static NV_FLOAT32 tickSpacing; tickSpacing = (NV_FLOAT32)drawWidth / (numTicks - 1.0f);

	for (int i=0; i<numTicks; i++) {

		tickX = (int) (left + i * tickSpacing);

		if ( scaleRange > .0f && lblInterval > 0 && i%lblInterval == 0 ) {
			lbl = QString("%1").arg(minScale + i*scaleRange/(numTicks-1),0,'f', scalePrecision);
			graph->drawText ( lbl, NINT(tickX  - fm.width(lbl) / 2.0f), top + tickH + lblPixSpacing + fontHeight , 90.0, 8, Qt::white, NVTrue);
		}
		graph->drawLine ( tickX, top, tickX, top + tickH, Qt::red, 1, NVFalse, Qt::SolidLine);
	}
}


//
// method:		DrawBottomScale
//
// This method will draw the tick marks and labels based on the number of tick marks
// desired and the label intervals of the left or Y axis.
//

void attributeViewer::DrawLeftScale(NV_FLOAT32 minScale, NV_FLOAT32 maxScale, int scalePreciesion, int numTicks, int graphHeight, int drawHeight, int leftMargin, int bottomMargin, int leftTickW, int lblPixSpacing, int fontHeight, QFontMetrics fm) {

	static QString lbl;
	static int tickY;
	static NV_FLOAT32 scaleRange; scaleRange = maxScale - minScale;

	for (int i=0; i<numTicks; i++) {

		tickY = graphHeight - bottomMargin - i * drawHeight / (numTicks-1);

		lbl = QString("%1").arg(minScale + i*scaleRange/(numTicks-1),0,'f',scalePreciesion);
		graph->drawText ( lbl, leftMargin - leftTickW - lblPixSpacing - fm.width(lbl), NINT(tickY+fontHeight/2.0), 90.0, 8, Qt::white, NVTrue);
		graph->drawLine ( leftMargin - leftTickW,  tickY, leftMargin, tickY, Qt::red, 1, NVFalse, Qt::SolidLine );
	}
}

//
// slot:		slotDrawGraph
//
// This slot is fired from a postRedrawSignal of the nvMap class.  The nvMap class is
// being used as our drawing canvas for the plots.  So on an update or refresh, this slot
// will be called.  This slot simply hands the drawing of the canvas area to 
// DrawGraph().
//

void attributeViewer::slotDrawGraph(NVMAP_DEF mapdef) { DrawGraph( mapdef ); }


//
// slot:		slotGraphOverplotAction
//
// This slot will be fired when the "Overplot" option is enabled.  Currently, this option
// is disabled as a GUI needs to be provided allowing the user to select different attributes
// to overplot on a normalized scale.  The functionality for this is just a Proof of Concept
// at this point.  
//

void attributeViewer::slotGraphOverplotAction() {
	
	graphProps.overplot = overplotOnAction->isChecked(); 

	/* scatterplot can be removed from this equation because overplot is */
	/* rendered useless in scatterplot mode */

	if( graphProps.overplot && graphProps.mode == GM_profile) 
		avData.CalcXYs(graphProps, cbAOI->currentIndex() ); /* if previously overplot xys will already be calced */

	graphProps.SetGraphLabels(cbAOI);
	
	graph->redrawMapArea(NVTrue); 
}

//
// slot:		slotGraphSelectionAction
//
// This slot is fired when either the "Polygon" or "Rectangle" option is selected under
// the Selection menu.  We update our graph properties object to reflect the selection.
//

void attributeViewer::slotGraphSelectionAction() {

	if(graphProps.selectPoly) poly.clear();
	graphProps.selectPoly = selectionPolyAction->isChecked();
	graph->redrawMapArea(NVTrue); 
}


//
// slot:			slotChangeUserDefinedScales
//
// This slot will be fired when the "Change User Defined Scales" menu option
// is clicked.  We will open the User-Defined Scales dialog and process
// accordingly.
//

void attributeViewer::slotChangeUserDefinedScales () {

	if (scaleDlg) {
		scaleDlg->close ();
		delete scaleDlg;
	}
		
	scaleDlg = new UserScaleDlg(graphProps.userMinScale, graphProps.userMaxScale,
								graphProps.userScatXMinScale, graphProps.userScatXMaxScale,
								graphProps.userScatYMinScale, graphProps.userScatYMaxScale, this);
								
	connect (scaleDlg, SIGNAL (sigUserScaleDefined (NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, 
													NV_FLOAT32, NV_FLOAT32, NV_FLOAT32)), this,
				SLOT (slotUserScaleDefined (NV_FLOAT32, NV_FLOAT32, NV_FLOAT32, 
												NV_FLOAT32, NV_FLOAT32, NV_FLOAT32)));
					
	scaleDlg->show();
}

//
// slot:		slotGraphScaleAction
//
// This slot will be fired when we select either Absolute, ROI, or User-Defined scaling.
//

void attributeViewer::slotGraphScaleAction() {

    SetScaling();
	graph->redrawMapArea(NVTrue); 
}

//
// slot:		slotGraphModeAction
//
// This slot will be fired when we select either Profile, Scatterplot, or Histogram.  We
// update the graph properties object accordingly and refresh the canvas area.
//

void attributeViewer::slotGraphModeAction() {

	GraphMode mode = (modeProfileAction->isChecked() ) ? GM_profile 
		: ( modeScatterPlotAction->isChecked() ) ? GM_scatterPlot 
		: GM_histogram ;

	/* we will not automatically change the AOI parameter when moving to scatterplot */

	graphProps.SetGraphMode(mode, cbAOI, avData.GetNumItems() );
	
	SetScaling();
	graph->redrawMapArea(NVTrue); 
}


//
// slot:			slotUserScaleDefined
//
// This slot will be fired when the user-defined signal from the UserScaleDlg class
// is emitted.  We will store the user defined scale in a variable and update the tool
// tip for the scaleUserAction.
//

void attributeViewer::slotUserScaleDefined (NV_FLOAT32 min, NV_FLOAT32 max,
											NV_FLOAT32 min2, NV_FLOAT32 max2,
											NV_FLOAT32 min3, NV_FLOAT32 max3) {

	graphProps.userMinScale = min;
	graphProps.userMaxScale = max;
	
	graphProps.userScatXMinScale = min2;
	graphProps.userScatXMaxScale = max2;
	
	graphProps.userScatYMinScale = min3;
	graphProps.userScatYMaxScale = max3;
	
	setUserDefinedActionDefs();
	
	scaleUserAction->setChecked(NVTrue);
	
	SetScaling();
	graph->redrawMapArea(NVTrue); 
}


//
// method:		SetScaling
//
// This method will update the graph properties object of the scaling and grab the 
// minimum values for the type of scaling.  Also, screen coordinates of all shots
// within this scaling will be calculated.
//

void attributeViewer::SetScaling() {

	if ( graphProps.mode == GM_profile || graphProps.mode == GM_scatterPlot ) {

		NV_INT32 dataIndex = cbAOI->currentIndex();		
		
		GraphScale scale;
		if (scaleAbsoluteAction->isChecked()) scale = GS_Absolute;		
		else if (scaleUserAction->isChecked()) scale = GS_User_Defined;		
		else scale = GS_Roi;
				
		graphProps.SetScaling(scale, avData.GetScaleMin(dataIndex, scale, &graphProps), 
								avData.GetScaleMax(dataIndex, scale, &graphProps) );		
		
		avData.CalcXYs(graphProps, dataIndex );
	}
}


//
// method:		AddAttributeScreen
//
// This method will add all of the widgets to the parent QMainWindow.  It will 
// initalize the graph properties object as well.
//

void attributeViewer::AddAttributeScreen(QBoxLayout *parentBox) {

	/* some options within the combo box are longer now, so we */
	/* need to increase the spacing */

	NV_INT32 labelWidth = 120, valueWidth = 240, spacingSize = 10;

	/* we set up a no projection map so we can get screen coordinates to */
	/* run our spatial sorting algorithm */	
	
	setupReferenceMap ();	
	
	AddGraph( parentBox );	

	parentBox->addSpacing(spacingSize);

	QVBoxLayout *col1 = new QVBoxLayout(), *col2 = new QVBoxLayout();

	col1->setAlignment(Qt::AlignLeft | Qt::AlignTop );
	AddAOI( col1, labelWidth, valueWidth );	
	
	col1->addSpacing(spacingSize);
	AddLabels( col1, labelWidth, valueWidth );	

	lblTrackingOff = new QLabel(tr("« Tracking Off »"), this);
	lblTrackingOff->setStyleSheet("color: #c00000; font-weight:bold; padding-left:2px; padding-right:2px");
	col2->setAlignment(Qt::AlignCenter | Qt::AlignTop );
	col2->addWidget ( lblTrackingOff );

	QHBoxLayout *hb1 = new QHBoxLayout();
	hb1->addLayout(col1);
	hb1->addSpacing(spacingSize);
	hb1->addLayout(col2);

	/* add our shot table which is a table listing */

	addShotTable ();	
	
	QHBoxLayout * hb = new QHBoxLayout ();

	/* build the spatial neighbor table */
	addNeighborTable ();	
	
	/* we build a table group box that houses both tables.  only one is */
	/* visible at a time.  we just hide/show the correct one. */
	
	tableGrpBox = new QGroupBox (this);

	QVBoxLayout * vBox = new QVBoxLayout ();
	
	vBox->addWidget (shotTable, Qt::AlignTop);
	vBox->addWidget (neighborTable, Qt::AlignVCenter);
			
	if (tableStyle == LISTING) {
		tableGrpBox->setTitle ("Table Listing");
		neighborTable->hide();
	}
	else {
		tableGrpBox->setTitle ("Neighbor Table");	
		shotTable->hide();
	}

	tableGrpBox->setLayout (vBox);	

	hb->addLayout (hb1);

	/* we add some space between the attribute metrics and the table */

	hb->addSpacing (75);
	hb->addWidget (tableGrpBox);
	hb->addStretch (1);	

	parentBox->addLayout( hb );
	
	InitGraphProps();	
}

//
// method:			addShotTable
//
// This method builds the table consisting of all of the values of the current
// attribute in a table.  The table consists of a shot number and the attribute
// value.  The items in the table are ordered from a north-south, east-west 
// direction.  Synchronization between the AV's shot pane and the editing interface
// exist when items in the table are selected.
//

void attributeViewer::addShotTable () {

	shotTable = new QTableWidget ();
	shotTable->setRowCount (10);
	shotTable->setColumnCount (2);
	shotTable->setSelectionBehavior (QAbstractItemView::SelectRows);
	shotTable->setAutoScrollMargin (false);
	shotTable->setSelectionMode (QAbstractItemView::SingleSelection);
	shotTable->verticalHeader()->hide();
	
	connect (shotTable, SIGNAL (itemClicked(QTableWidgetItem *)), this, SLOT (slotShotTblHighlighted(QTableWidgetItem *)));

	connect (shotTable, SIGNAL (cellEntered (int, int)), this, SLOT (slotShotItemSelectionChanged (int, int)));
	
	QTableWidgetItem * headers[2];

	headers[0] = new QTableWidgetItem ("Shot #");
	headers[1] = new QTableWidgetItem ("Attribute Value");

	QFont font = headers[0]->font ();
	font.setBold (true);

	for (int i = 0; i < 2; i++) {
		headers[i]->setFont (font);
		shotTable->setHorizontalHeaderItem (i, headers[i]);
	}	
}




//
// method:			addNeighborTable
//
// This method will build the spatial neighbor table.  It is a 3 x 3 table with 
// no headers.  When a value within the shot graphic pane within the AV is highlighted
// the center square of this table reflects the value.  All other cells show the spatial
// neighbor of the center square and it's value.
//

void attributeViewer::addNeighborTable () {	

	neighborTable = new QTableWidget ();

	neighborTable->setRowCount (spatialDimSize);
	neighborTable->setColumnCount (spatialDimSize);

	neighborTable->setSelectionMode (QAbstractItemView::SingleSelection);
	neighborTable->verticalHeader()->hide();
	neighborTable->horizontalHeader()->hide();

	/* we fix the size of the table to make it look sleeker. */
	neighborTable->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	NV_INT32 widthPerCell = 306 / 3, heightPerCell = 96 / 3;
	neighborTable->setFixedSize (widthPerCell * spatialDimSize, heightPerCell * spatialDimSize);	

	/* this slot is fired when an item is clicked in the table */
	connect (neighborTable, SIGNAL (itemClicked(QTableWidgetItem *)), this, SLOT (slotNeighborTblHighlighted(QTableWidgetItem *)));
}



//
// method:			setupReferenceMap
//
// In this method, we are creating a no projection map so we can later convert lat/lon values
// to screen coordinates for our slotting algorithm to be used to populate the spatial neighbor 
// table 

void attributeViewer::setupReferenceMap () {

	NVMAP_DEF mapdef;

	mapdef.projection = NO_PROJECTION;
	
	mapdef.overlap_percent = 5;
	mapdef.grid_inc_x = 0.0;
	mapdef.grid_inc_y = 0.0;

 	mapdef.initial_bounds.min_x = abe_share->displayed_area.min_x;
	mapdef.initial_bounds.min_y = abe_share->displayed_area.min_y;
	mapdef.initial_bounds.max_x = abe_share->displayed_area.max_x;
	mapdef.initial_bounds.max_y = abe_share->displayed_area.max_y;

	NV_FLOAT64 diffLon, diffLat;

	diffLon = mapdef.initial_bounds.max_x - mapdef.initial_bounds.min_x;
	diffLat = mapdef.initial_bounds.max_y - mapdef.initial_bounds.min_y;

	/* we are making our scales for width and height to be the maximum of */
	/* 1000 (arbitrary hard-coded value) but keeping the aspect ratio of the */
	/* initial bounds */

	if (diffLon > diffLat) {
		mapdef.draw_width = SCALE_MAX;
		mapdef.draw_height = (NV_INT32)(SCALE_MAX * diffLat / diffLon); 
	}
	else {
		mapdef.draw_height = SCALE_MAX;
		mapdef.draw_width = (NV_INT32)(SCALE_MAX * diffLon / diffLat);
	}

	referenceMap = new nvMap (this, &mapdef);	

	avData.setReferenceMap (referenceMap);
}


//
// method:		AddAOI
//
// This method will initialize the combo box containing all attributes.  A chain link will
// appear for any attributes connected to the user-defined attributes within the pfmLoad. 
// This linking will cause the color-coding to automatically be linked.
//

void attributeViewer::AddAOI(QBoxLayout *parentBox, NV_INT32 labelWidth, NV_INT32 valueWidth) {

	lblAOI = new QLabel(tr("Attribute"), this);
	lblAOI->setFixedWidth( labelWidth );

	cbAOI = new QComboBox (this);
	cbAOI->setToolTip (tr ("Choose Attribute"));
	cbAOI->setWhatsThis (tr ("Attribute on which all metrics are calculated.") );
	cbAOI->setEditable (FALSE);

	/* the labels between this revision will be changed to match the ndx_attr_name array
	   found in pfm.h.  These names for the attributes must match potential values within
	   the ndx_attr_name array in order to provide color coding linking between the two */
	 
	cbAOI->addItem (tr("CZMIL Depth[0]"));
	cbAOI->addItem (tr("CZMIL Depth[1]"));
	cbAOI->addItem (tr("CZMIL Depth[2]"));
	cbAOI->addItem (tr("CZMIL KGPS Elevation"));
	cbAOI->addItem (tr("CZMIL Waveform class"));	
	
	/* read through the list and place any links next to user-defined attributes */
	/* the user-defined attributes must match the combo box labels exactly */
	
	NV_INT32 nameIndex;
	
	for (NV_INT32 i = 0; i < cbAOI->count(); i++) {	
		if (isAttributeLinkable(i, nameIndex)) 
			cbAOI->setItemIcon (i, QIcon (":/icons/chainLink.png"));	
	}
	
	/* we do not want to automatically set the AOI to a paramter because of scatterplot mode */	

	connect (cbAOI, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotCBAOICurrentIndexChanged(int) ) );	
	
	QHBoxLayout *hb = new QHBoxLayout();
	hb->setAlignment(Qt::AlignLeft);
	hb->addWidget(lblAOI);
	hb->addWidget(cbAOI);

	parentBox->addLayout( hb );
}


//
// method:		AddLabels
//
// This method will add the labels of the statistics section of the Attribute Viewer.
//

void attributeViewer::AddLabels(QBoxLayout *parentBox, NV_INT32 labelWidth, NV_INT32 valueWidth) {

	QString caps[5] = { tr("Primary Pixel"), tr("Mean"), tr("SD"), tr("Min"), tr("Max") };

	QLabel *lbls[5];
	QLabel *lblCaps[5];
	QHBoxLayout *hb;

	for (int i=0; i<5; i++) {

		lblCaps[i] = new QLabel(caps[i], this);
		lblCaps[i]->setFixedWidth( labelWidth );

		lbls[i] = new QLabel("0", this);
		lbls[i]->setAlignment(Qt::AlignRight);
		lbls[i]->setFixedWidth( valueWidth );


		hb = new QHBoxLayout();
		hb->setAlignment(Qt::AlignLeft);
		hb->addWidget(lblCaps[i]);
		hb->addWidget(lbls[i]);

		parentBox->addLayout( hb );
	}

	lblCenterPixel = lbls[0];
	lblMean = lbls[1];
	lblSD = lbls[2];
	lblMin = lbls[3];
	lblMax  = lbls[4];

	lblCenterPixelCaption = lblCaps[0];
	lblMeanCaption = lblCaps[1];
	lblSDCaption = lblCaps[2];
	lblMinCaption = lblCaps[3];
	lblMaxCaption = lblCaps[4];
}


//
// method:		AddGraph
//
// This method will initialize the nvMap object that is the plotting canvas and hook up
// signal/slot routines to it.
//

void attributeViewer::AddGraph(QBoxLayout *parentBox) {

	InitGraphMapDef();	

	graph = new nvMap (this, &graphMapDef);

	parentBox->addWidget( graph, 1 );
	graph->setCursor (Qt::PointingHandCursor);
	graph->enableSignals ();
	connect (graph, SIGNAL (postRedrawSignal (NVMAP_DEF)), this, SLOT (slotDrawGraph (NVMAP_DEF)));

	connect (graph, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotGraphResize (QResizeEvent *)));

	connect (graph, SIGNAL (mouseMoveSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotGraphMouseMove (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));

	connect (graph, SIGNAL (mousePressSignal (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)), this, SLOT (slotGraphMousePress (QMouseEvent *, NV_FLOAT64, NV_FLOAT64)));
}


//
// method:		InitGraphMapDef
//
// This method will define the canvas area as being a NO_PROJECTION map with some basic
// initialization properties.
//

void attributeViewer::InitGraphMapDef () {

	graphMapDef.projection = NO_PROJECTION;
	graphMapDef.draw_width = width;
	graphMapDef.draw_height = height;
	graphMapDef.overlap_percent = 5;
	graphMapDef.grid_inc_x = 0.0;
	graphMapDef.grid_inc_y = 0.0;

	graphMapDef.coasts = NVFalse;
	graphMapDef.landmask = NVFalse;

	graphMapDef.border = 0;
	graphMapDef.coast_color = Qt::white;
	graphMapDef.grid_color = QColor (160, 160, 160, 127);
	graphMapDef.background_color = Qt::black;


	graphMapDef.initial_bounds.min_x = 0;
	graphMapDef.initial_bounds.min_y = 0;

	graphMapDef.initial_bounds.max_x = 500;
	graphMapDef.initial_bounds.max_y = 500;
}


//
// method:		InitGraphProps
//
// This method will initialize the properties concerning the graph such as overplotting
// mode (disabled currently), graphing mode, and selection mode.
//

void attributeViewer::InitGraphProps() {

	graphProps.overplot = overplotOnEnvIn; // note: this overplot must be initialized before profilemode

	graphProps.SetGraphMode(graphModeEnvIn, cbAOI, avData.GetNumItems() );
	
	graphProps.selectPoly = selectionPolyEnvIn;
}

//
// event:		closeEvent
//
// This event will be fired when the window is closed.  This event hands processing over
// to slotQuit.
//

void attributeViewer::closeEvent (QCloseEvent * event __attribute__ ((unused))) { 

	slotQuit (); 
}


//
// slot:		slotGraphResize
//
// This slot will fire when the canvas area is resized in the event of creation or the
// dragging of the window.  The dimensions will be recalculated and stored and the canvas
// will be re-rendered.  For the first time, we will load up the attribute that was last
// saved within the settings.
//

void attributeViewer::slotGraphResize (QResizeEvent *e) {    

	if (abe_share->avShare.avRunning ) {
		graphProps.CalcResizableDims( e->size().width(), e->size().height(), avData.GetNumItems() );
		avData.CalcXYs( graphProps, cbAOI->currentIndex() );
		graph->redrawMapArea(NVTrue);
	}
	
	/* chl revision 08_24_2011 */
	/* we need to have the dimensions set in order for us to set the attribute to the */
	/* last saved attribute.  The reason is that the combo box slot redraws a new canvas */
	/* and dimensions must be known. */
	
	static NV_BOOL firstTime = NVTrue;
	
	if (firstTime) {
		cbAOI->setCurrentIndex (selectIndex);
		firstTime = NVFalse;
	}
	
	/* end revision */
}


// 
// slot:		slotHelp
//
// This slot will enter the What's This mode.
//

void attributeViewer::slotHelp ()
{
	QWhatsThis::enterWhatsThisMode ();
}



//
// slot:		slotGraphMouseMove
//
// This slot will fire when the mouse cursor is moved within the canvas pane.  If a selection
// is happening, the coordinates are updated.  If no selection is being made, the closest
// shot within the pane is calculated based on screen coordinates.  The canvas is then
// re-rendered so a highlight or selection box can be updated.
//

void attributeViewer::slotGraphMouseMove (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused))) {

	if (graphProps.overplot || avData.GetNumItems() == 0) {		
		return;	
	}
	
	if ( selecting) {
		if ( graphProps.selectPoly ) polyMouseMove(e->x(), e->y());
		else rectMouseMove(e->x(), e->y());
	} 	
	else if (graphProps.mode == GM_profile || graphProps.mode == GM_scatterPlot ) 
		CalcHighlightedIndex(e->x(), e->y());

	graph->redrawMapArea(NVTrue);
	graph->flush();	
}


//
// method:		polyMouseMove
//
// this method will add a vertex to the polygon already stored for a selection polygon.
//

void attributeViewer::polyMouseMove(NV_INT32 x, NV_INT32 y) {

	poly.update(x,y);
}


//
// method:		rectMouseMove
//
// This method will store the new corner point based on the screen x and y coming in
// as arguments.
//

void attributeViewer::rectMouseMove( NV_INT32 x, NV_INT32 y ) {
	rectXs[1] = x;
	rectYs[1] = y;
}


//
// method:		CalcHighlightedIndex
//
// This method will ingest mouse screen coordinates and see if they are within the axis
// margins.  If so, it will calculate the distance to all shots and figure out the closest
// shot so it can be highlighted.  Once found, an AVA_HIGHLIGHT message is sent via
// shared memory for the editing interface to process.  The point cloud index of the shot
// is attached to this message so the map cursors can synchronize accordingly.  Also,
// the table, whether listing or neighbor, is updated to focus on the newly highlighted
// shot.
//

void attributeViewer::CalcHighlightedIndex(NV_INT32 mouseX, NV_INT32 mouseY) {

	int numItems, closestItem = 0, dataIndex, hoverMargin=10;
	if (mouseX < graphProps.leftMargin - hoverMargin || mouseX > graphProps.leftMargin + graphProps.drawWidth  + hoverMargin
		|| mouseY < graphProps.topMargin - hoverMargin || mouseY > graphProps.topMargin  + graphProps.drawHeight + hoverMargin
		|| (numItems = avData.GetNumItems()) <= 0 ) return;


	/* if we are in profile mode, take the index of the combo box */
	/* however, if we are in scatterplot, set dataIndex to our special entry at the end */
	/* managing the scatterplot */

	if (graphProps.mode == GM_profile)
		dataIndex = cbAOI->currentIndex();
	else /* scatterplot mode */
		dataIndex = NUM_ATTRIBUTES;	

	NV_FLOAT64 curD, closestD=(NV_FLOAT64)graphProps.h;

	for (int i=0; i<numItems; i++ ) {
		curD = calcDist(mouseX, mouseY, avData[i].xs[dataIndex], avData[i].ys[dataIndex] );
		if (curD < closestD) {
			closestD=curD;
			closestItem = i;
		}
	}

	if (avData.highlightedShotIndex == closestItem )		
		return;	

	avData.highlightedShotIndex = closestItem;
	SaveAVActionToShare( AVA_HIGHLIGHT, &avData[closestItem].masterIdx, 1);
	lblCenterPixel->setNum( (avData[closestItem])[cbAOI->currentIndex()] ); 
	
	/* if it is the standard listing table, we select the highlighted value */
	if (tableStyle == LISTING)	
		shotTable->selectRow (closestItem);
	/* if it is the spatial neighbor table, we have to run our spatial slot */
	/* algorithm and populate the table */
	else { // tableStyle == NEIGHBOR

		NV_INT32 * neighbors = avData.lookForNeighbors ();
		populateNeighborTable (neighbors);

		delete neighbors;
	}		
}



//
// method:			populateNeighborTable
//
// This method will ingest the indices of the neighbors, clear the spatial neighbor table
// and place the values of the neighbors in their appropriate cells.  The indices of the
// neighbors array correspond to a spatial cell.
//

void attributeViewer::populateNeighborTable (NV_INT32 * neighbors) {

	neighborTable->clearContents();

	// handle the primary or highlighted pixel first
	
	neighborTable->setCurrentCell (spatialDimSize / 2, spatialDimSize / 2);
	
	// load up the center box which is the primary pixel

	QTableWidgetItem * pp = new QTableWidgetItem();
	pp->setTextAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
	pp->setFlags (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
	
	/* check for bin filtered attribute */
				
	if (avData.isBinAttribute(cbAOI->currentIndex()) && (NV_INT32)(avData[avData.highlightedShotIndex][cbAOI->currentIndex()]) == 0) 
		pp->setText("0 (invalid)");
	else if (avData.isDepthAttribute (cbAOI->currentIndex()) && 
			((NV_INT32)(avData[avData.highlightedShotIndex][cbAOI->currentIndex()] - 0.5) == -998))
		pp->setText("-998 (invalid)");
	else	
		pp->setText (QString::number (avData[avData.highlightedShotIndex][cbAOI->currentIndex()]));

	neighborTable->setItem (spatialDimSize / 2, spatialDimSize / 2, pp);
		
	// populate neighbors

	NV_INT16 * tableMap, midPoint;

	avData.getTableMapAndMidpoint (tableMap, midPoint);
	
	for (int i = 0; i < (spatialDimSize * spatialDimSize - 1); i++) {

		if (neighbors[i] != -1) {

			QTableWidgetItem * tItem = new QTableWidgetItem();
			tItem->setTextAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
			tItem->setFlags (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
			
			/* check for bin filtered attribute */
				
			if (avData.isBinAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)(avData[neighbors[i]][cbAOI->currentIndex()]) == 0) 
				tItem->setText("0 (invalid)");
			else if (avData.isDepthAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)(avData[neighbors[i]][cbAOI->currentIndex()] - 0.5) == -998) 
				tItem->setText("-998 (invalid");
			else			
				tItem->setText (QString::number (avData[neighbors[i]][cbAOI->currentIndex()]));

			if (tableMap[i] < midPoint) neighborTable->setItem (tableMap[i] / spatialDimSize, tableMap[i] % spatialDimSize, tItem);
			else neighborTable->setItem((tableMap[i] + 1) / spatialDimSize, (tableMap[i] + 1) % spatialDimSize, tItem);				
		}		
	}
}




//
// slot:			slotShotTblHighlighted
//
// This slot is fired when an item within the listing table is clicked.  We set our highlighted
// index to be of the table row that was clicked since they are ordered in the table as they are
// internally.  We send a HIGHLIGHT message through shared memory to our editing interface and
// reset our center pixel value label.  Finally, we redraw the shot pane so our new highlight 
// can take affect.
//

void attributeViewer::slotShotTblHighlighted (QTableWidgetItem * item) {

	avData.highlightedShotIndex = item->row();
	SaveAVActionToShare( AVA_HIGHLIGHT, &avData[avData.highlightedShotIndex].masterIdx, 1);
	
	/* check for bin filtered attribute */
				
	if (avData.isBinAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)((avData[avData.highlightedShotIndex])[cbAOI->currentIndex()]) == 0) 
		lblCenterPixel->setText("0 (invalid)");
	else if (avData.isDepthAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)((avData[avData.highlightedShotIndex])[cbAOI->currentIndex()] - 0.5) == -998) 
		lblCenterPixel->setText("-998 (invalid)");
	else	
		lblCenterPixel->setNum( (avData[avData.highlightedShotIndex])[cbAOI->currentIndex()] ); 	

	graph->redrawMapArea(NVTrue);
	graph->flush();	
}




//
// method:			slotNeighborTblHighlighted
//
// This method is fired when an item within the spatial neighbor table is clicked.  We find
// the cell index of the clicked entry and look up the index of the shot that that cell
// represents so we can get the right highlight index.  We send the HIGHLIGHT message through
// shared memory and reset our center pixel label.  We update our shot pane so the highlight
// can take affect.
//

void attributeViewer::slotNeighborTblHighlighted (QTableWidgetItem * item) {

	NV_INT32 cellIndex = item->row() * spatialDimSize + item->column();
	
	avData.highlightedShotIndex = avData.getNeighborIdxs()[cellIndex];
	
	SaveAVActionToShare( AVA_HIGHLIGHT, &avData[avData.highlightedShotIndex].masterIdx, 1);
	
	/* check for bin filtered attribute */
				
	if (avData.isBinAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)((avData[avData.highlightedShotIndex])[cbAOI->currentIndex()]) == 0) 
		lblCenterPixel->setText("0 (invalid)");
	else if (avData.isDepthAttribute(cbAOI->currentIndex()) && 
						(NV_INT32)((avData[avData.highlightedShotIndex])[cbAOI->currentIndex()] - 0.5) == -998) 
		lblCenterPixel->setText("-998 (invalid)");
	else
		lblCenterPixel->setNum( (avData[avData.highlightedShotIndex])[cbAOI->currentIndex()] );

	graph->redrawMapArea (NVTrue);
	graph->flush();
}



//
// slot:			slotShotItemSelectionChanged
//
// This slot is fired when a cell in the listing table is entered but more to the point, when 
// the user holds the mouse down and runs through the table this slot will be fired.  We just 
// call our slot that handles clicked items for the listing table which will update the 
// highlight (sorry, run on sentence there)
//

void attributeViewer::slotShotItemSelectionChanged (int row, int col) {

	slotShotTblHighlighted (shotTable->item(row, col));
}


//
// method:		calcDist
//
// This method is the standard distance calculation of 2 dimensional points.
//

NV_FLOAT64 attributeViewer::calcDist(int x1, int y1, int x2, int y2) { 

	return sqrt( (double)((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)) ); 
}


//
// slot:		slotGraphMousePress
//
// This slot will be fired when the mouse button is pressed within the canvas area.  We only
// process if we have at least a shot within the area.  We also only handle a right or left 
// click.
//

void attributeViewer::slotGraphMousePress (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused))) {

	/* don't bother if we are empty with shots */
	if (avData.GetNumItems() == 0) return;

	if (e->button () == Qt::LeftButton ) graphLeftMousePress(e);
	if (e->button () == Qt::RightButton) graphRightMousePress(e);	
}


//
// method:		graphLeftMousePress
//
// This method is responsible for a left click.  If we are profiling or scatterplotting, 
// we begin the starting pivot point of a selection.
//

void attributeViewer::graphLeftMousePress (QMouseEvent *e) {

	if ( graphProps.mode != GM_histogram && !graphProps.overplot ) {

		if ( graphProps.selectPoly ) polyClick(e);
		else rectClick(e);
	}	
}


//
// method:		rectClick
//
// This method will store the opening or final click of a rectangular selection box.  
// If we are closing the selection, a context menu will be shown asking if we wish to
// delete or cancel the selection.
//

void attributeViewer::rectClick(QMouseEvent *e) {

	if (selecting) {	

		rectXs[1] = e->x();
		rectYs[1] = e->y();

		if( rectXs[0] > rectXs[1] ) {
			NV_INT32 t = rectXs[1];
			rectXs[1] = rectXs[0];
			rectXs[0] = t;
		}

		if( rectYs[0] > rectYs[1] ) {
			NV_INT32 t = rectYs[1];
			rectYs[1] = rectYs[0];
			rectYs[0] = t;
		}

		graphSelectionContextMenu->move(e->globalX(),e->globalY());
		graphSelectionContextMenu->show();
	}
	else {

		selecting =  NVTrue;
		rectXs[0] = e->x();
		rectYs[0] = e->y();
	}
}


//
// method:		polyClick
//
// This method will add vertices to the current selection.  If this is the initial click,
// it turns the selecting flag on.  If it is the final click, the context menu is shown
// asking to either delete or cancel.
//

void attributeViewer::polyClick(QMouseEvent *e) {

	poly.addVertex(e->x(), e->y() );

	if (selecting) {
		graphSelectionContextMenu->move(e->globalX(),e->globalY());
		graphSelectionContextMenu->show();
	} 

	else {
		selecting = NVTrue;
	}
}


//
// method:		graphRightMousePress
//
// this method handles a right mouse press within the canvas area.  If we are in profile
// or scatterplot mode a context menu is shown.  The type of context menu depends on if
// we are simply highlighting a shot or we are in the middle of a selection.
//

void attributeViewer::graphRightMousePress (QMouseEvent *e) {

	if (graphProps.mode != GM_histogram && !graphProps.overplot) {
		QMenu *m = (selecting) ? graphSelectionContextMenu : graphContextMenu;
		m->move(e->globalX(), e->globalY());
		m->show();
	}
}


//
// slot:		slotGraphDelete
//
// This slot will be fired when the delete option is selected from the context menu.  This 
// will delete a single shot.  An AVA_DELETE message will be sent via shared memory and the
// information attached to it will be the point cloud index of the shot to delete and the 
// point cloud index of the shot to highlight after the delete.
//

void attributeViewer::slotGraphDelete() { 	

	NV_INT32 idxs[2];

	idxs[0] = avData[avData.highlightedShotIndex].masterIdx;

	if (avData.GetNumItems() == 1) idxs[1] = -1;
	else if (avData.highlightedShotIndex > 0) 
		idxs[1] = avData[--avData.highlightedShotIndex].masterIdx;

	else idxs[1] = avData[avData.highlightedShotIndex + 1].masterIdx;

	SaveAVActionToShare(AVA_DELETE, idxs, 2);
}


//
// slot:		slotGraphDeleteAll
//
// This slot will fire when the Delete All option from the context menu is selected.  All
// of the shots are calculated that fall within the polygonal or rectangular selection box.
// Once calculated all of the point cloud indexes are sent over for deletion along with the
// AVA_DELETE message via shared memory.  avData.numSelected + 1 entries are sent over because
// the last entry is the new shot to highlight once all of the previous shots have been
// deleted.  A value of -1 in this last shot means that all shots were deleted and nothing
// should be highlighted.
//

void attributeViewer::slotGraphDeleteAll() { 

	if ( selecting ) {		
		
		/* set the index for deleting.  for profile, it will be combobox current index */
		/* for scatterplot it will be NUM_ATTRIBUTES entry, or last entry */
		/* also change the select method argument to indexToDelete */

		NV_INT32 indexToDelete;

		if (graphProps.mode == GM_profile) indexToDelete = cbAOI->currentIndex();
		else /*scatterplot mode */ indexToDelete = NUM_ATTRIBUTES;		

		if ( graphProps.selectPoly ) 			
			avData.select( graphProps, indexToDelete, poly.verts, poly.xs, poly.ys, NVTrue);
		else 			
			avData.select( graphProps, indexToDelete, 2, rectXs, rectYs, NVFalse );

		SaveAVActionToShare(AVA_DELETE, avData.selection, avData.numSelected + 1); /* plus one is for -1 which is not telling pfmedit which one to hightlight*/

		clearSelection();		
	}
}

//
// slot:		slotGraphCancel
//
// This slot will fire when the Cancel option is selected from the context menu.  This 
// will simply clear the vertices or array elements of the selection.

void attributeViewer::slotGraphCancel() {

	clearSelection();
}


//
// method:		clearSelection
//
// This method will clear the selection and turn the selecting flag off.  The canvas
// will be re-rendered causing the disappearance of the selection polygon.
//

void attributeViewer::clearSelection() {

	if( selecting ) {

		avData.clearSelection();

		if ( graphProps.selectPoly ) poly.clear();

		selecting = NVFalse;
		graph->redrawMapArea (NVTrue);
		graph->flush ();
	}
}


// 
// method:		SaveAVActionToShare
//
// This method will store the type of action, the number of action indices (point cloud
// indexes) that need to be parsed, and the indices themselves.
//

void attributeViewer::SaveAVActionToShare( AV_SHARE_ACTION action, NV_INT32 *actionIdxs, NV_INT32 numIdxs ) {

	static NV_INT32 prevIdx = -1;
	static AV_SHARE_ACTION prevAction = AVA_NONE;

	if ( (numIdxs == 1 && actionIdxs && prevIdx==actionIdxs[0] && prevAction==action) /* prevent dupe single item actions saves */
		|| ( action == prevAction && action == AVA_NONE ) /* prevent dupe AVA_NONE save */
		) return; 

	else {
		prevAction = action;
		prevIdx = ( actionIdxs )? actionIdxs[0] : -1;
	}

	abeShare->lock();
	abe_share->avShare.action = action;
	abe_share->avShare.numActionIdxs = numIdxs;

	if( action != AVA_NONE ) 
		for (NV_INT32 i=0; i<numIdxs; i++ ) abe_share->avShare.actionIdxs[i] = actionIdxs[i]; 

	abeShare->unlock();
}


//
// slot:		trackCursor
//
// This slot will fire on a timeout every 16 milliseconds.  It is responsible for checking
// for new data, loading that data, calculating it, and redisplaying the canvas window
// accordingly.
//

void attributeViewer::trackCursor () {  

	//  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
	//  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit) or if our own personal kill signal
	//  has been placed in abe_share->key.	
	
	if (abe_share->key == CHILD_PROCESS_FORCE_EXIT || abe_share->key == ANCILLARY_FORCE_EXIT ||
		abe_share->key == kill_switch) slotQuit ();
	
	static AV_SHARE avShare;
	static int dataIndex;
	static NV_BOOL newData;

	newData = NVFalse;	

	if ( abe_share->avShare.avTracking ) {

		if ( abe_share->avShare.avNewData) {			

			abeShare->lock ();
			avShare = abe_share->avShare;			
			
			if ( abe_share->avShare.avNewData ) { 					

				newData = NVTrue; 
				abe_share->avShare.avNewData = NVFalse; 

				dataIndex = cbAOI->currentIndex();	

				/* different load mechanisms depending on the table and style */				
				
				if (tableStyle == LISTING) 
					avData.Load (avShare, dataIndex, shotTable, tableStyle, histInterval);
				else 
					avData.Load (avShare, dataIndex, neighborTable, tableStyle, histInterval);
					
				graphProps.CalcSpacing(avData.GetNumItems() );
				
				SetScaling();			
				
				DisplayData(dataIndex);					
			}			

			abeShare->unlock ();
		}			
	}		
}


//
// slot:		slotQuit
//
// This slot is fired when the Quit option is selected from the File menu.  AV flags
// are toggled so the editing interface can depress buttons on it's end.
//

void attributeViewer::slotQuit ()
{
	envout ();


	abeShare->lock ();


	//  Let the parent program know that we have died from something other than the kill switch from the parent.

	if (abe_share->key != kill_switch) abe_share->killed = kill_switch;

	/* chl revision 08_25_2011 */
	/* Bit 0 is CZMIL AV now */
	//abe_share->avShare.avRunning &= 0xfd;	
	abe_share->avShare.avRunning &= 0xfe;	
	/* end revision */
	
	abe_share->avShare.avNewData = NVTrue;  

	abeShare->unlock ();

	//  Let go of the shared memory.

	abeShare->detach ();

	exit (0);
}


//
// slot:		slotChangeScatterAxes
//
// This slot is fired when the Change Scatterplot Axes option under the Options menu
// is selected.  A special dialog is shown allowing the user to change the attributes
// for the scatterplot axes.
//

void attributeViewer::slotChangeScatterAxes () {

	if (spAxesDlg) {
		spAxesDlg->close ();
		delete spAxesDlg;
	}
	
	QStringList paramList;

	for (int i = 0; i < cbAOI->count(); i++)
		paramList << cbAOI->itemText (i);

	spAxesDlg = new AxesDlg (paramList, graphProps.scatterXIndex, graphProps.scatterYIndex, this);

	connect (spAxesDlg, SIGNAL (sigAxesApply (NV_INT32, NV_INT32)), this, SLOT (slotSetAxesIndices (NV_INT32, NV_INT32)));
	
	spAxesDlg->show ();
}


//
// slot:		slotSetAxesIndices
// 
// This slot is fired when some changes are made within the Change Scatterplot Axes dialog
// and the changes are applied.  We update the graph properties object accordingly and
// redraw the scatterplot if we are in that mode.
//

void attributeViewer::slotSetAxesIndices (NV_INT32 xState, NV_INT32 yState) {

	graphProps.scatterXIndex = xState;
	graphProps.scatterYIndex = yState;
	
	if (graphProps.mode == GM_scatterPlot) {
		
		NV_INT32 dataIndex = cbAOI->currentIndex();	
		graphProps.SetGraphLabels (cbAOI);
		SetScaling();			
		DisplayData(dataIndex);		
	}
}


//
// slot:		slotPrefs
//
// This slot fires when the user selects the Preferences menu item from the File menu.  The
// Preferences dialog is instantiated and the widgets of this dialog are connected here to
// respective slots.
//

void attributeViewer::slotPrefs ()
{
	// delete what is currently commented on your end and insert the following

	if (attPrefs) attPrefs->close ();

	attPrefs = new AttPrefs (this);

	connect (attPrefs->histIntSpin, SIGNAL (valueChanged (double)), this, SLOT (slotHistogramIntervalChanged (double)));
	connect (attPrefs->pbHistBarColor, SIGNAL ( clicked() ), this, SLOT( slotHistBarColorChanged() ) );
	connect (attPrefs->pbHistBarOutlineColor, SIGNAL ( clicked() ), this, SLOT( slotHistBarOutlineColorChanged() ) );
	connect (attPrefs->pbHistNullColor, SIGNAL ( clicked() ), this, SLOT( slotHistNullColorChanged() ) );

	/* hook any change in table style to the slotTableSyleClicked() slot */
	connect (attPrefs->listingBtn, SIGNAL (clicked ()), this, SLOT (slotTableStyleClicked ()));
	connect (attPrefs->neighborBtn, SIGNAL (clicked ()), this, SLOT (slotTableStyleClicked ()));

	/* connect the user-emitted signal that will change the spatial neighbor table size */
	connect (attPrefs, SIGNAL (spatialNeighborSizeSig (int)), this, SLOT (slotSpatialNeighborSizeChanged (int)));
		
	/* check the appropriate radio button */
	if (tableStyle == LISTING) attPrefs->listingBtn->setChecked (NVTrue);
	else attPrefs->neighborBtn->setChecked (NVTrue);
	
	attPrefs->show ();
}

//
// slot:			slotSpatialNeighborSizeChanged
//
// This slot will ulitmately get here by changing the combo box controlling the size of
// the spatial neighbor table within the Preferences section.  We will change the row
// and column dimension's here depending on the selection chosen.
//

void attributeViewer::slotSpatialNeighborSizeChanged (int index) {

	int widthPerCell = 306 / 3, heightPerCell = 96 / 3;

	switch (index) {

		case 0:		
				spatialDimSize = 3;
				break;

		case 1:
				spatialDimSize = 5;				
				break;

		case 2:
				spatialDimSize = 7;				
				break;
	}

	avData.setSpatialDimSize (spatialDimSize);

	neighborTable->setRowCount (spatialDimSize);
	neighborTable->setColumnCount (spatialDimSize);

	neighborTable->setFixedSize (widthPerCell * spatialDimSize, heightPerCell * spatialDimSize);	

	NV_INT32 * neighbors = avData.lookForNeighbors ();
	populateNeighborTable (neighbors);

	delete neighbors;
}




//
// slot:			slotTableStyleClicked
//
// This slot will be fired when the user clicks on a table style in the Preferences
// dialog.  We update the table style variable, change the group box title, hide/unhide
// the appropriate tables, and then repopulate the new table by calling CalcStats.
//

void attributeViewer::slotTableStyleClicked () {

	if (attPrefs->listingBtn->isChecked()) {

		tableStyle = LISTING;
		tableGrpBox->setTitle ("Table Listing");		
	
		neighborTable->hide();
		shotTable->show();		

		avData.CalcStats (cbAOI->currentIndex(), shotTable, tableStyle);		
	}
	else {

		tableStyle = NEIGHBOR;		
		tableGrpBox->setTitle ("Neighbor Table");
		
		shotTable->hide();
		neighborTable->show();	
		
		avData.CalcStats (cbAOI->currentIndex(), neighborTable, tableStyle);		
	}
}

//
// slot:		slotHistBarColorChanged
//
// This slot is fired when the user changes the color of the histogram bar within the
// Preference dialog.  The color is noted and the canvas area is redrawn to update.
//

void attributeViewer::slotHistBarColorChanged() {

	histBarColor = attPrefs->pbHistBarColor->palette().color(QPalette::Normal, QPalette::Button);
	graph->redrawMapArea(NVTrue);
}

//
// slot:		slotHistBarOutlineColorChanged
//
// This slot is fired when the user changes the color of the histogram bar outline within 
// the Preference dialog.  The color is noted and the canvas area is redrawn to update.
//

void attributeViewer::slotHistBarOutlineColorChanged() {

	histBarOutlineColor = attPrefs->pbHistBarOutlineColor->palette().color(QPalette::Normal, QPalette::Button);
	graph->redrawMapArea(NVTrue);
}

//
// slot:		slotHistNullColorChanged
//
// This slot is fired when the user changes the color of the null bar within the
// Preference dialog.  The color is noted and the canvas area is redrawn to update.
//

void attributeViewer::slotHistNullColorChanged() {

	histNullColor = attPrefs->pbHistNullColor->palette().color(QPalette::Normal, QPalette::Button);
	graph->redrawMapArea(NVTrue);
}


void attributeViewer::slotHistogramIntervalChanged (double histInterval) {

	this->histInterval = (NV_FLOAT32) histInterval;

	avData.TagHistogramDataForReload();
	// whatever it takes to update the histogram, you may play off of a flag
	// depending on if you are in histogram/linear profile mode.  it may be
	// already be accounted for in your DrawGraph () method.

	graph->redrawMapArea(NVTrue);
}


//
// slot:		slotClosePrefs
//
// When the Close button on the Preferences dialog is pressed, this slot will fire and it
// will simply close the dialog.
//

void attributeViewer::slotClosePrefs () {

	prefsD->close ();
}


//
// slot:		slotRestoreDefaults
//
// This slot is fired when defaults are wanted.  All modes, scales, dimension defaults
// reside here.
//

void attributeViewer::slotRestoreDefaults () {

	width = 324;
	height = 600;
	window_x = 0;
	window_y = 0;

	// x: depth, y: depth 1
	
	graphProps.scatterXIndex = 0;
	graphProps.scatterYIndex = 1;	

	/* also set the default graph mode to profile */

	graphModeEnvIn = GM_profile;
	
	/* chl revision 08_24_2011 */
	
	overplotOnEnvIn = NVFalse;
	
	graphProps.userMinScale = -50.0f;
	graphProps.userMaxScale = 50.0f;
	
	graphProps.userScatXMinScale = -50.0f;
	graphProps.userScatXMaxScale = 50.0f;
	
	graphProps.userScatYMinScale = -50.0f;
	graphProps.userScatYMaxScale = 50.0f;
	
	selectIndex = 3;	// KGPS Elevation index
	
	/* end revision */
	
	spatialDimSize = 5;
	histInterval = .01f;	
	
	histBarColor.setRgbF (.0f, .0f, 1.0f,  .4f);
	histBarOutlineColor.setRgbF (.0f, .0f, 1.0f,  .65f);
	histNullColor.setRgbF (1.0f, .0f,  .0f, 1.0f);	

	/* default table style will be the spatial neighbor */
	tableStyle = NEIGHBOR;
	
	force_redraw = NVTrue;
}

//
// slot:		slotHalfSecond
//
// This slot will fire every half second and is in charge of updating the user whether
// the AV is being tracked or not.
//

void attributeViewer::slotHalfSecond() {

	// a timer that occurs every half second

	if ( !abe_share->avShare.avTracking ) lblTrackingOff->setVisible( !lblTrackingOff->isVisible() );
	else if ( lblTrackingOff->isVisible() ) lblTrackingOff->setVisible( NVFalse );

}

//
// method:		about
//
// This method simply displays a message box about the CZMIL Attribute Viewer.
//

void attributeViewer::about () { 

    QMessageBox::about (this, VERSION, "CZMIL Attribute Viewer." "\n\nCHL 08/09/2011"); 	
}


//
// method:		aboutQt
//
// This method simply displays a pre-canned message box about QT.
//

void attributeViewer::aboutQt () { QMessageBox::aboutQt (this, VERSION); }



//
// method:		envin
//
// This method will load up the default values and then if a prior history has been
// saved, the settings will be recalled to mimick the last session of the AV.
//

void attributeViewer::envin () {

	/* chl revision 08_24_2011 */
	//NV_FLOAT64 saved_version = 1.08;
	NV_FLOAT64 saved_version = 1.10;
	/* end revision */
	
	// Set Defaults so the if keys don't exist the parms are defined

	slotRestoreDefaults ();
	force_redraw = NVFalse;
	
	QSettings settings (tr ("navo.navy.mil"), tr ("CZMILattributeViewer"));		
	
	settings.beginGroup (tr ("CZMILattributeViewer"));
	
	saved_version = settings.value (tr ("settings version"), saved_version).toDouble ();

	//  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

	if (settings_version != saved_version) {		
		return;
	}

	width = settings.value (tr ("width"), width).toInt ();

	height = settings.value (tr ("height"), height).toInt ();

	window_x = settings.value (tr ("window x"), window_x).toInt ();

	window_y = settings.value (tr ("window y"), window_y).toInt ();

	
	/* bring in the scatterplot axis settings */

	graphProps.scatterXIndex = settings.value (tr ("scatterplot x axis"), graphProps.scatterXIndex).toInt();
	graphProps.scatterYIndex = settings.value (tr ("scatterplot y axis"), graphProps.scatterYIndex).toInt();

	graphProps.scale = (GraphScale) settings.value ( tr("graph scaling"), GS_Absolute).toInt();
	graphProps.userMinScale = (NV_FLOAT32)settings.value (tr("user defined min scale"), -500.0f).toDouble();
	graphProps.userMaxScale = (NV_FLOAT32)settings.value (tr("user defined max scale"), 500.0f).toDouble();
	graphProps.userScatXMinScale = (NV_FLOAT32)settings.value (tr("user defined Scatter X min scale"), -500.0f).toDouble();
	graphProps.userScatXMaxScale = (NV_FLOAT32)settings.value (tr("user defined Scatter X max scale"), 500.0f).toDouble();
	graphProps.userScatYMinScale = (NV_FLOAT32)settings.value (tr("user defined Scatter Y min scale"), -500.0f).toDouble();
	graphProps.userScatYMaxScale = (NV_FLOAT32)settings.value (tr("user defined Scatter Y max scale"), 500.0f).toDouble();
	
	overplotOnEnvIn = settings.value ( tr("overplot on"), NVFalse ).toBool();

	graphModeEnvIn = (GraphMode) ( ( settings.value( tr("graph mode"), GM_profile)).toInt() );	
	
	/* chl revision 08_24_2011 */
	selectIndex = settings.value (tr ("selection index"), 3).toInt();
	/* end revision */

	/* inherit the saved table style */
	tableStyle = (TableStyle)((settings.value (tr ("table style"), NEIGHBOR)).toInt());

	spatialDimSize = settings.value (tr("spatial dim size"), 5).toInt();
		
	selectionPolyEnvIn = settings.value ( tr("Poly Selection"), NVTrue).toBool();

	/* must match default in preferences */	
	histInterval = (NV_FLOAT32)settings.value ( tr("histInterval"), .01 ).toDouble();
		
	histBarColor.setRgba        ( settings.value( tr("hist bar color")        , QColor::fromRgbF(  .0f, .0f, 1.0f,  .4f  ).rgba() ).toUInt() );
	histBarOutlineColor.setRgba ( settings.value( tr("hist bar outline color"), QColor::fromRgbF(  .0f, .0f, 1.0f,  .65f ).rgba() ).toUInt() );
	histNullColor.setRgba       ( settings.value( tr("hist null color")       , QColor::fromRgbF( 1.0f, .0f,  .0f, 1.0f  ).rgba() ).toUInt() );

	this->restoreState (settings.value (tr ("main window state")).toByteArray (), (NV_INT32) (settings_version * 100.0));

	settings.endGroup ();
}


//
// slot:		slotCBAOICurrentIndexChanged
//
// this slot will fire when the user changes the attribute of focus within the combo
// box.  We will first check to see if it is a user-defined attribute.  If so, we send
// a message to the editing interface issuing a color link.  We calculate new statistics
// on the new attribute and update the tables.  We set new scales and then redraw the 
// canvas area to place the new shot values.
//

void attributeViewer::slotCBAOICurrentIndexChanged(int index) {

	/* check to see if we have a color-coding link for the parameter change. */
	
	NV_INT32 nameIndex;
	
	if (isAttributeLinkable (index, nameIndex)) 	
		SaveAVActionToShare (AVA_COLOR_LINK, &nameIndex, 1);		
		
	/* if we have a highlight, move the highlight to the proper spot upon */
	/* switching of attribute */

	for (int i = 0; i < avData.GetNumItems(); i++) {

		if (avData[avData.highlightedShotIndex].masterIdx == avData[i].masterIdx) {
			avData.highlightedShotIndex = i;
			break;
		}
	}	

	/* call CalcStats with the right table style */

	if (tableStyle == LISTING) avData.CalcStats(index, shotTable, tableStyle);
	else avData.CalcStats (index, neighborTable, tableStyle);

	graphProps.SetGraphLabels(cbAOI);
	
	SetScaling();	
	
	DisplayData(index);	
}




//
// method:		envout
//
// This method will save out the settings so we can begin where we left next time.
//

void attributeViewer::envout ()
{
	//  Use frame geometry to get the absolute x and y.

	QRect tmp = this->frameGeometry ();

	window_x = tmp.x ();
	window_y = tmp.y ();

	//  Use geometry to get the width and height.

	tmp = this->geometry ();
	width = tmp.width ();
	height = tmp.height ();
	
	QSettings settings (tr ("navo.navy.mil"), tr ("CZMILattributeViewer"));		
	
	settings.beginGroup (tr ("CZMILattributeViewer"));	

	settings.setValue (tr ("settings version"), settings_version);

	settings.setValue (tr ("width"), width);

	settings.setValue (tr ("height"), height);

	settings.setValue (tr ("window x"), window_x);

	settings.setValue (tr ("window y"), window_y);

	/* save the scatterplot axis settings */

	settings.setValue (tr ("scatterplot x axis"), graphProps.scatterXIndex);
	settings.setValue (tr ("scatterplot y axis"), graphProps.scatterYIndex);

	settings.setValue (tr ("main window state"), this->saveState ((NV_INT32) (settings_version * 100.0)));
	settings.setValue (tr ("graph scaling"),  graphProps.scale );
	
	settings.setValue (tr("user defined min scale"), graphProps.userMinScale);
	settings.setValue (tr("user defined max scale"), graphProps.userMaxScale);
	
	settings.setValue (tr("user defined Scatter X min scale"), graphProps.userScatXMinScale);
	settings.setValue (tr("user defined Scatter X max scale"), graphProps.userScatXMaxScale);
	
	settings.setValue (tr("user defined Scatter Y min scale"), graphProps.userScatYMinScale);
	settings.setValue (tr("user defined Scatter Y max scale"), graphProps.userScatYMaxScale);
	
	settings.setValue (tr ("overplot on"), overplotOnAction->isChecked() );

	settings.setValue (tr ("graph mode"), graphProps.mode );
	
	/* chl revision 08_24_2011 */
	settings.setValue (tr ("selection index"), cbAOI->currentIndex());
	/* end revision */

	/* save out the table style */
	settings.setValue (tr ("table style"), tableStyle);

	settings.setValue (tr ("spatial dim size"), spatialDimSize);
		
	settings.setValue (tr ("poly selection"), selectionPolyAction->isChecked() );

	settings.setValue (tr ("histInterval"), histInterval );

	settings.setValue (tr ("hist bar color"), histBarColor.rgba() );
	settings.setValue (tr ("hist bar outline color"), histBarOutlineColor.rgba() );
	settings.setValue (tr ("hist null color"), histNullColor.rgba() );

	settings.endGroup ();
}


//
// operator:		[]
//
// This overload will return the proper data member of the AVDataItem class.  As more 
// members to the class are added, this method will grow.
//

NV_FLOAT32 &AVDataItem::operator[] ( int i ) { 
	return 
	
		(i == 0) ? simDepth :
		(i == 1) ? simDepth2 :
		(i == 2) ? simDepth3 :
		(i == 3) ? simKGPSElev :
		simWaveformClass;		
}


//
// construct:		AVData
//
// This constructor will initialize some key members and set the absolute scales for the
// as of now 5 attributes of interest.
//

AVData::AVData() { 

	items = NULL; numItems=0; isLoaded = NVFalse; 	
			
    /* chl revision 08_29_2011 */
	
	// static NV_FLOAT32 amins [] =  {0.0f, 0.0f, 0.0f, -997.0f, 0.0f };
	// static NV_FLOAT32 amaxs [] =  {60.0f, 60.0f, 60.0f, 999.0f, 255.0f};			

	static NV_FLOAT32 amins [] =  {0.0f, .0f, 0.0f, -50.0f, 0.0f };
	static NV_FLOAT32 amaxs [] =  {50.0f, 50.0f, 50.0f, 50.0f, 255.0f};		

	/* end revision */
									
	absmins = amins;
	absmaxs = amaxs;	
}

//
// deconstruct:		~AVData
//
// This deconstructor will free the previously allocated items.
//

AVData::~AVData() { free( items ); }


//
// method:		Load
//
// This method will take new data, mine into the CZMIL files and pull out the data.  Next,
// for the attribute of interest, statistics are calculated and a histogram is initiated.
//

void AVData::Load(AV_SHARE &avShare, int dataIndex, QTableWidget * table, TableStyle tStyle, NV_FLOAT32 histInterval __attribute__ ((unused)) ) {

	loadFromCZMIL (avShare);	

	/* call CalcStats with proper table and style */
	
	CalcStats(dataIndex, table, tStyle);	
	
	TagHistogramDataForReload();		
}

//
// method:		LoadHistogramData
//
// This method will load the histogram data for a particular attribute as referenced by
// dataIndex.
//

void AVData::LoadHistogramData(NV_FLOAT32 histInterval, int dataIndex) {
	
	histData[dataIndex].Load( histInterval, dataIndex, items, mins[dataIndex], maxs[dataIndex], numItems );	
}

//
// method:		TagHistogramDataForReload
//
// This method will initialize all attribute histogram data items.
//

void AVData::TagHistogramDataForReload() {

	for (int i = 0; i< NUM_ATTRIBUTES; i++) 
		histData[i].TagForReload();
}


//
// method:		allocateCZMILShotMemory
//
// This method will allocate the amount of memory only for CZMIL shots coming through shared
// memory.
//

NV_INT32 AVData::allocateCZMILShotMemory (AV_SHARE avShare) {

	NV_INT32 numCZMILShots = 0;
	
	for (NV_INT32 i = 0; i < avShare.numShots; i++) {
	
		if (avShare.shots[i].type == PFM_CZMIL_DATA)
			numCZMILShots++;
	}
	
	if (numCZMILShots != 0) {
	
		items = (AVDataItem **)calloc(numCZMILShots, sizeof(AVDataItem *) );	
	
		for (NV_INT32 i = 0; i < numCZMILShots; i++) {
			items[i] = (AVDataItem *) calloc(1, sizeof(AVDataItem) );			
		}
	}
	
	return numCZMILShots;
}	


//
// method:		loadFromCZMIL
//
// This method will free the previous items and allocate new memory for the new data.  For
// each shot, it will use the CZMIL library to find the CZMIL record of interest within the
// CXY file and load the shot information.  Minimum and maximum ranges for the attributes
// are calculated for ROI purposes.  Lastly, the shots are sorted in a northing then easting 
// pattern.
//

void AVData::loadFromCZMIL (AV_SHARE &avShare) {

	NV_INT32 numRead = 0;	
	NV_CHAR filename[512], filename2[512];	
	NV_INT32 czmilHnd;	
	NV_INT16 type;
	
	for (int i=0; i<numItems; i++) {

		if (items[i] != NULL) {
			free( items[i] );
			items[i] = NULL;
		}
	}

	if (items != NULL) {
		free(items);
		items = NULL;
	}		
	
	NV_INT32 numCZMILShots = allocateCZMILShotMemory (avShare);
	
	/* chl revision 08_30_2011 */
	
	//if (numCZMILShots == 0) return;	
	
	if (numCZMILShots == 0) {
		numItems = 0;
		return;
	}
	
	/* end revision */
	
	NV_BOOL isLoaded[avShare.numShots];
	
	for (NV_INT32 i = 0; i < avShare.numShots; i++) {	
		isLoaded[i] = NVFalse;
	}		
	
	NV_INT32 czmilCounter = 0;	

	hitMax = avShare.hitMax;	

	for (int i = 0; (i< avShare.numShots) && (numRead < numCZMILShots); i++ ) {			
		
		if (avShare.shots[i].type == PFM_CZMIL_DATA) {
	    			
			if (isLoaded[i]) continue;				
			
			read_list_file( avShare.shots[i].pfmHandle, avShare.shots[i].fileNo, filename, &type );
					
			CZMIL_CXY_Header czmilHeader;
			czmilHnd = czmil_open_file (filename, &czmilHeader, CZMIL_READONLY, NVFalse);			
			
			if (czmilHnd >= 0) {

				LoadShot(czmilHnd, avShare, isLoaded, czmilCounter, i, numRead );
				czmilCounter++;
				
				for (int j = i + 1; (j < avShare.numShots) && (numRead < numCZMILShots); j++ ) {
				
					read_list_file(avShare.shots[j].pfmHandle, avShare.shots[j].fileNo, filename2, &type );					
						
					if ((strcmp(filename2, filename) == 0) && (avShare.shots[j].type == PFM_CZMIL_DATA)) {
						LoadShot(czmilHnd, avShare, isLoaded, czmilCounter, j, numRead);
						czmilCounter++;
					}
				}				
				
				czmil_close_file (czmilHnd);
			}				
		}		
	}		
	
	numItems = numCZMILShots;	
	
	centerPixel = items[0];	

	CalcMinsAndMaxs();		

	qsort (items, numItems, sizeof(AVDataItem *), CompareAVDataItemsNorthingEasting);

	/* now that we have sorted based on a northing/easting paradigm, set the index ids */
	/* of all of the items so we have a basis to fall back on in the event that other */
	/* algorithms need to re-sort by a different protocol.  (i.e.  sorting by distance */
	/* to calculate spatial neighbors */

	for (NV_INT32 i = 0; i < numItems; i++) 
		items[i]->indexID = i;
		
	this->isLoaded = NVTrue;	
}


//
// method:		CalcMinsAndMaxs
//
// This method will calculate the minimum and maximum values for the attributes.  Note,
// that an invalid value (-998) is not included in the process.  Therefore, if the invalid
// value changes as CZMIL develops, the -998's will need to be replaced.
//

void AVData::CalcMinsAndMaxs() {

	static NV_FLOAT32 val;
	
	for (int di = 0; di < NUM_ATTRIBUTES; di++) mins[di] = maxs[di] = (*items[0])[di];
	
	for (int i = 1; i < numItems; i++) 

		for (int di = 0; di < NUM_ATTRIBUTES; di++ ) {

			val = (*items[i])[di];			
			
			/* account for the bin/used parameters that may have a 0 as an invalid index
			   and disregard them. */
			   
			if (isBinAttribute(di)) {
			
				if (mins[di] == 0 && val != 0) mins[di] = val;
				else if (val != 0 && val < mins[di]) mins[di] = val;
			}
			else if (isDepthAttribute (di)) {
				if (((NV_INT32)(mins[di] - 0.5) == -998) && ((NV_INT32)(val - 0.5) != -998))
					mins[di] = val;
				else if (((NV_INT32)(val - 0.5) != -998) && val < mins[di])
					mins[di] = val;
			}
			else if (val < mins[di] ) {
				mins[di] = val;				
			}
			if (val > maxs[di] ) maxs[di] = val;
		}	
}


//
// method:		LoadShot
//
// This method will read an individual CXY record and then store the values from the
// deep channel into an AVDataItem object.
//

void AVData::LoadShot(NV_INT32 czmilHnd, AV_SHARE &avShare, NV_BOOL *isLoaded, 
						NV_INT32 czmilIndex, NV_INT32 index, NV_INT32 &numRead ) {
						
	CZMIL_CXY_Data czmilRec;
	
	czmil_read_cxy_record (czmilHnd, avShare.shots[index].recordNumber, &czmilRec);	
	
	/* chl revision 08_25_2011 */
	items[czmilIndex]->recNum = avShare.shots[index].recordNumber;
	/* end revision */
	
	items[czmilIndex]->simDepth = czmilRec.channel[CZMIL_DEEP_CHANNEL].return_elevation[1];
	items[czmilIndex]->simDepth2 = czmilRec.channel[CZMIL_DEEP_CHANNEL].return_elevation[2];
	items[czmilIndex]->simDepth3 = czmilRec.channel[CZMIL_DEEP_CHANNEL].return_elevation[3];
	items[czmilIndex]->simKGPSElev = czmilRec.channel[CZMIL_DEEP_CHANNEL].return_elevation[0];
	items[czmilIndex]->simWaveformClass = czmilRec.channel[CZMIL_DEEP_CHANNEL].waveform_class;

	items[czmilIndex]->latitude = czmilRec.channel[CZMIL_DEEP_CHANNEL].latitude;
	items[czmilIndex]->longitude = czmilRec.channel[CZMIL_DEEP_CHANNEL].longitude;	
	items[czmilIndex]->masterIdx = avShare.shots[index].masterIdx;

	items[czmilIndex]->colorH = avShare.shots[index].colorH;
	items[czmilIndex]->colorS = avShare.shots[index].colorS;
	items[czmilIndex]->colorV = avShare.shots[index].colorV;
	
	isLoaded[index] = NVTrue;	
	
	numRead++;	
}


//
// method:		CalcStats
//
// This method will calculate the statistics for the attribute noted by dataIndex.  These
// stats include mean, SD, average, etc.  Also, the tables (whether listing or neighbor) 
// are populated with the attribute values of other shots.
//

void AVData::CalcStats (int dataIndex, QTableWidget * table, TableStyle tStyle) {

	static NV_FLOAT32 sum, sumOfSquares, val;
	static NV_INT32 itemsInCalc;	

	/* these are related arrays used to hold the neighbors around the primary pixel */
	/* in the event that we have a neighbor table. array indices are as follows */
	/* as they relate to the primary pixel: 0 = top left, 1 = directly above */
	/* 2 = top right, 3 = left, 4 = right, 5 = bottom left, 6 = below, 7 = bottom right */

	NV_INT32 * indices = NULL;
	NV_INT16 * tableMap, midPoint;	

	sum = sumOfSquares = 0.0f;
	itemsInCalc = 0;
	
	/* set the number of rows in the table for our new data */	
	/* remove references of just shot table, the function has to be versatile to handle */
	/* both types of tables.  we also calculate end points of our rays that emanate from */
	/* the primary pixel's cell */	

	table->clearContents();

	if (tStyle == LISTING) table->setRowCount (numItems);
	else {		
		getTableMapAndMidpoint (tableMap, midPoint);		
	}	

	for (int i=0; i< numItems; i++) {		
		
		val = (*items[i])[dataIndex];		
		
		/* filter out any bin/used type parameters that have a 0 */		
		if ( val > -990.0f && !(isBinAttribute(dataIndex) && (int)val == 0)) {		
		
			sum+=val;
			sumOfSquares += val * val;
			itemsInCalc++;
		}		
		
		/* we need to properly highlight the primary pixel in the graphics pane as */
		/* well as the table so we will check the center pixel's master idx with each */
		/* item's master idx to find the right one */

		if (items[i]->masterIdx == centerPixel->masterIdx) {
		
			highlightedShotIndex = i;
			
			/* 2 branches for the different table styles */			
			
			if (tStyle == LISTING) table->selectRow (i);
			else {	
			
				table->setCurrentCell (spatialDimSize / 2, spatialDimSize / 2);
				
			    // load up the center box which is the primary pixel

				QTableWidgetItem * pp = new QTableWidgetItem();
				pp->setTextAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
				pp->setFlags (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
				
				/* check for bin filtered attribute */
				
				if (isBinAttribute(dataIndex) && (NV_INT32)(val) == 0) 
					pp->setText("0 (invalid)");
				else if (isDepthAttribute (dataIndex) && (NV_INT32)(val - 0.5) == -998)
					pp->setText("-998 (invalid)");
				else				
					pp->setText (QString::number (val));

				table->setItem (spatialDimSize / 2, spatialDimSize / 2, pp);				
			}			
		}			
	  
		if (tStyle == LISTING) {	
			
			/* rather than wasting cycles on another loop, i am going to piggy */
			/* back on this one and fill the shot table up here. */

			QTableWidgetItem * tblRecord[2];
		
			for (int k = 0; k < 2; k++) {

				tblRecord[k] = new QTableWidgetItem ();	
				tblRecord[k]->setTextAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
				
				/* we can select the items, but we can not edit them */
				tblRecord[k]->setFlags (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);				
			}

			tblRecord[0]->setText (QString::number (i));			
			
			/* account for bin/used filtered attributes */
			if (isBinAttribute(dataIndex) && (NV_INT32)(val) == 0) 
					tblRecord[1]->setText("0 (invalid)");
			else if(isDepthAttribute (dataIndex) && (NV_INT32)(val - 0.5) == -998)
					tblRecord[1]->setText("-998 (invalid)");
			else			
				tblRecord[1]->setText (QString::number (val));

			for (int j = 0; j < 2; j++)				
				/* remove reference of shotTable */				
				table->setItem (i, j, tblRecord[j]);		
		}		
	}		

	if (numItems > 0) {
		
		if (tStyle != LISTING) {		

			indices = lookForNeighbors ();
		
			/* populate the table */
			
			for (int i = 0; i < (spatialDimSize * spatialDimSize - 1); i++) {

				if (indices[i] != -1) {

					QTableWidgetItem * tItem = new QTableWidgetItem();
					tItem->setTextAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
					tItem->setFlags (Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
					
					/* check for bin filtered attribute */
					
					if (isBinAttribute(dataIndex) && (NV_INT32)((*items[indices[i]])[dataIndex]) == 0) 
						tItem->setText("0 (invalid)");
					else if (isDepthAttribute (dataIndex) && 
							(NV_INT32)(((*items[indices[i]])[dataIndex]) - 0.5) == -998)
						tItem->setText("-998 (invalid)");
					else				
						tItem->setText (QString::number ((*items[indices[i]])[dataIndex]));

					if (tableMap[i] < midPoint) table->setItem (tableMap[i] / spatialDimSize, tableMap[i] % spatialDimSize, tItem);
					else table->setItem((tableMap[i] + 1) / spatialDimSize, (tableMap[i] + 1) % spatialDimSize, tItem);				
				}			
			}	

			if (indices != NULL) delete indices;
		}	
	}	
		
	if (itemsInCalc == 0) {
		if (isBinAttribute (dataIndex)) mean = 0;
		else if (isDepthAttribute (dataIndex)) mean = -998;
		else mean = 0;
	}
	else mean = (NV_FLOAT32) sum / itemsInCalc;	
	
	std = (itemsInCalc<=1) ? 0.0f : (NV_FLOAT32)sqrt( (double)( ( sumOfSquares - ( sum*sum/numItems)) / (itemsInCalc-1) ) );
}


/* The new updateNeighborTable() takes in a sorted list and we calculate the neighbors until
   we run out of items or all of the neighbor cells are filled */

//
// method:			updateNeighborTable
//
// This method is the guts of the spatial slotting alogrithm.  A basic description of it is detailed
// below.  This method will evaluate all of the sorted shot and sees if they fall in any cell around
// the primary pixel.  Because the shots are already sorted by distance, it automatically fills
// an empty cell.
//

void AVData::updateNeighborTable (AVDataItem ** sortedItems, NV_INT32 * neighbors, 
								  NV_INT32 ppX, NV_INT32 ppY) {

    // convert shot referenced by index into screen coordinates

    NV_FLOAT64 dummyZ = 0.0;
	NV_INT32 screenX, screenY, screenZ;

	Vector centerPt = Vector (ppX, ppY, 0.0f);
	
	NV_INT32 cellsFilled = 0;
	NV_INT32 numPasses = spatialDimSize / 2;

	Vertex endPts[spatialDimSize * spatialDimSize - 1];

	for (NV_INT32 i = 0; i < (spatialDimSize * spatialDimSize - 1); i++) 
		endPts[i] = centerPt + Vector (unitRays[i][0], unitRays[i][1], 0.0) * SCALE_MAX;

	for (NV_INT32 i = 0; i < numItems - 1; i++) {

		refMap->map_to_screen (1, &sortedItems[i]->longitude, &sortedItems[i]->latitude, 
								&dummyZ, &screenX, &screenY, &screenZ);

			
		/*            --------------------------------
					  |   0     |   1     |   2      |
					  |         |         |          |
					  -------------------------------
					  |   3     | primary |   4      |
					  |         |  pixel  |          |
					  --------------------------------
					  |   5     |   6     |   7      |
					  |         |         |          |
					  --------------------------------

		we draw virtual screen coordinate lines from the primary pixels cutting through each cell and 
		for each shot passed in, we will look at which line the shot lies closest too.  that line 
		represents the cell that the shot is slotted too.  If it is the closest to the primary pixel 
		in regards to that cell then it is the current neighbor within that cell. The lines cutting
		through the cells are depicted as unit vectors.  More information can be read in UnitRays.hpp.
		Also, the graphic above depicts a 3x3 table, a 5x5 and 7x7 table are also suppported. */

	
		Vector shotPt = Vector (screenX, screenY, 0.0f);

		// prime the leading votegetter

		Vertex lineOnPt;
		NV_INT32 leadingIndex = 0;
		NV_FLOAT32 minDist; 
		NV_INT32 winnersTaken[numPasses];

		for (NV_INT32 x = 0; x < numPasses; x++) winnersTaken[x] = -1;

		for (NV_INT32 passes = 0; passes < numPasses; passes++) {

			leadingIndex = -1;
			minDist = 9999999999.0f;

			// check the cells
			for (NV_INT32 j = 0; j < (spatialDimSize * spatialDimSize - 1); j++) {

				if (!leadingIndexTaken (j, winnersTaken, numPasses - 1)) {
				
					lineOnPt = Vector::ClosestPtOnLine (centerPt, endPts[j], shotPt);
					NV_FLOAT32 dist = Vector::GetDistance (lineOnPt, shotPt);		

					if (dist < minDist) {
						minDist = dist;
						leadingIndex = j;
					}	
				}
			}	

			if (neighbors[leadingIndex] == -1) break;
			else   // another pass for the backup spot
				winnersTaken[passes] = leadingIndex;
		}

		if (neighbors[leadingIndex] == -1) {			
			neighbors[leadingIndex] = sortedItems[i]->indexID;
			cellsFilled++;
		}

		if (cellsFilled == (spatialDimSize * spatialDimSize - 1)) break;
	}	
}


//
// method:			leadingIndexTaken
//
// This method will search arg2 which is the current indices taken and return a
// true/false based on whether arg1 is in that list.
//

NV_BOOL AVData::leadingIndexTaken (NV_INT32 curIndex, NV_INT32 * winners, NV_INT32 numWinners) {

	NV_BOOL temp = NVFalse;

	for (NV_INT32 i = 0; i < numWinners; i++) {

		if (curIndex == winners[i]) return true;
	}

	return temp;
}


//
// method:		inside
//
// This method will return a true/false if a shot's screen coordinates are within the 
// selection.
//

NV_BOOL AVData::inside(NV_INT32 *xs, NV_INT32 *ys, NV_INT32 count, NV_INT32 x, NV_INT32 y, NV_BOOL isPoly ) {

	return (isPoly) ? inside_polygon3(xs,ys,count,x,y) : ( x >= xs[0] && x <= xs[1] && y >= ys[0] && y <= ys[1] );
}

//
// method:		clearSelection
//
// This method will free the selection memory thereby clearing the selection with
// the numSelected data member.
//

void AVData::clearSelection() {

	free(selection);
	numSelected = 0;
}

//
// self explanatory accessor functions
//

NV_INT32 AVData::GetNumItems() { return numItems; }
NV_FLOAT32 AVData::GetMean() { return mean; }
NV_FLOAT32 AVData::GetMin(int dataIndex) { return mins[dataIndex]; }
NV_FLOAT32 AVData::GetMax(int dataIndex) { return maxs[dataIndex]; }
NV_FLOAT32 AVData::GetStd() { return std; }


//
// method:		GetScaleMax
//
// This method will return the maximum scale value depending on the scale type whether it
// is ROI, Absolute, or User Defined.
//

NV_FLOAT32 AVData::GetScaleMax(int dataIndex, GraphScale scale, GraphProps * gp, NV_INT32 axis) { 
	
	if (scale == GS_Roi) return maxs[dataIndex];
	else if (scale == GS_Absolute) {
	
		if (maxs[dataIndex] > absmaxs[dataIndex])
			return maxs[dataIndex];
		else return absmaxs[dataIndex];
	}
	else if (scale == GS_User_Defined) {
		if (gp->mode == GM_profile) return gp->userMaxScale;
		else if (gp->mode == GM_scatterPlot) {
			if (axis == 0) return gp->userScatXMaxScale;
			else return gp->userScatYMaxScale;
		}
		else return 0.0f;	// histogram
	}
	
	return 0.0f;	// should never get here, just making the compiler happy
}


//
// method:		GetScaleMin
//
// This method will return the minimum scale value depending on the scale type whether it
// is ROI, Absolute, or User Defined.
//

NV_FLOAT32 AVData::GetScaleMin(int dataIndex, GraphScale scale, GraphProps * gp, NV_INT32 axis) { 

    if (scale == GS_Roi) return mins[dataIndex];
	else if (scale == GS_Absolute) {
	
		if (mins[dataIndex] < absmins[dataIndex])
			return mins[dataIndex];
		else return absmins[dataIndex];
	}
	else if (scale == GS_User_Defined) {
		if (gp->mode == GM_profile) return gp->userMinScale;
		else if (gp->mode == GM_scatterPlot) {
			if (axis == 0) return gp->userScatXMinScale;
			else return gp->userScatYMinScale;
		}
		else return 0.0f;	// histogram
	}
		
	return 0.0f;	// should never get here, just making the compiler more happy
}

//
// method:		GetCenterPixel
//
// This method will return an AVDataItem corresponding to the highlighted item in the
// editing interface.
//

AVDataItem &AVData::GetCenterPixel( ) { return *centerPixel; }

//
// method:		GetHistItem
//
// This method will return an AVHistItem corelating to the attribute of focus.
//

AVHistItem *AVData::GetHistItem( int dataIndex ) { return &histData[dataIndex] ; }


//
// method:		CalcXYs
//
// This method will calculate screen coordinates for all of the shots for a particular
// attribute referenced by dataIndex.  If a filter catches a shot, we set the x and y
// screen coordinates to a ridiculous -1000000 making it virtually nonexistant on the 
// screen and it cannot cause problems.
//

void AVData::CalcXYs( GraphProps &gp, int dataIndex ) {

	/* don't waste time in here if in histogram mode */

	if (!isLoaded || gp.mode == GM_histogram) return; 

	NV_FLOAT32 minY, rangeY, minX = 0.0, rangeX = 0.0;
	
	/* we want to see if any of the indices are bin-index-filter worthy.  These parameters
	   include the bin and used parameters within a hof file. */
	NV_BOOL bfEnabled = binFilterEnabled (&gp, dataIndex);		
		
	NV_BOOL dfEnabled = depthFilterEnabled (&gp, dataIndex);
	
	/* all of the following really only applies to the profile method.  the scatterplot portion */
	/* will be it's own branch */

	if (gp.mode == GM_profile) {

		int startIndex, endIndex;

		/* set the end to be NUM_ATTRIBUTES -1 instead of a hard code. */

		if ( gp.overplot ) { startIndex = 0; endIndex = NUM_ATTRIBUTES - 1; }
		else startIndex = endIndex = dataIndex;

		for (int di = startIndex; di <= endIndex; di++)	{

			minY = GetScaleMin(di, gp.scale, &gp);
			rangeY = GetScaleMax(di, gp.scale, &gp) - minY;		
			
			for (int i = 0; i < numItems; i++ ) {
			
				// perform our bin filter - no bin (Deep, Shallow, or IR) parameter can contain an index of
				// 0 because it doesn't exist.  It will be disregarded in the scatterplot.  We will place
				// -1000000's as position values to indicate this.
			
				if ((bfEnabled && binIndexFilter(&gp, items[i], di)) || 
					(dfEnabled && depthFilter (&gp, items[i], di)) ||
					userDefinedScaleFilter(&gp, items[i], di)) {
					
					items[i]->ys[di] = -1000000;
					items[i]->xs[di] = -1000000;				
				}
				else {				

					items[i]->ys[di] = (int) (gp.h - gp.bottomMargin - gp.drawHeight * ( (rangeY==.0f) ? .5f : ((*items[i])[di] - minY) / rangeY) );

					/* only need GM_profile stuff in this branch */
				
					items[i]->xs[di] = NINT(gp.leftMargin + i*gp.shotSpacing);				
				}				
			}
		}
	}
	else {	/* scatterplot branch */
		
		minY = GetScaleMin(gp.scatterYIndex, gp.scale, &gp, 1);
		rangeY = GetScaleMax (gp.scatterYIndex, gp.scale, &gp, 1) - minY;

		minX = GetScaleMin (gp.scatterXIndex, gp.scale, &gp, 0);
		rangeX = GetScaleMax (gp.scatterXIndex, gp.scale, &gp, 0) - minX;		

		// we have NUM_ATTRIBUTES + 1 entries for each item
		// the last item refers to the scatterplot indices	

		for (int i = 0; i < numItems; i++ ) {		    
			
			// perform our bin filter - no bin (Deep, Shallow, or IR) parameter can contain an index of
			// 0 because it doesn't exist.  It will be disregarded in the scatterplot.  We will place
			// -1000000's as position values to indicate this.
			
			if ((bfEnabled && binIndexFilter(&gp, items[i])) ||
				(dfEnabled && depthFilter (&gp, items[i])) ||
				userDefinedScaleFilter(&gp, items[i])) {
				items[i]->ys[NUM_ATTRIBUTES] = -1000000;
				items[i]->xs[NUM_ATTRIBUTES] = -1000000;				
			}
			else {			

				items[i]->ys[NUM_ATTRIBUTES] = (int) (gp.h - gp.bottomMargin - gp.drawHeight * ( (rangeY==.0f) ? .5f 
									: ((*items[i])[gp.scatterYIndex] - minY) / rangeY) );
			
				items[i]->xs[NUM_ATTRIBUTES] = (int) (gp.leftMargin + gp.drawWidth * ( (rangeX==.0f) ? .5f 
									: ((*items[i])[gp.scatterXIndex] - minX) / rangeX) );		    
			}						
		}
	}	
}



// 
// method:			binFilterEnabled
//
// This function will check to see if the indices passed in through GraphProps will correspond to the 
// attributes within the HOF file that are capable of having a bin index of 0.  There are 9 possible 
// parameters that meet this criteria.  
//

NV_BOOL AVData::binFilterEnabled (GraphProps *gp, NV_INT32 dataIndex) {

	NV_BOOL result = NVFalse;	
	
	if (gp->mode == GM_scatterPlot) {	
		if (isBinAttribute (gp->scatterXIndex)) result = NVTrue;
		else if (isBinAttribute (gp->scatterYIndex)) result = NVTrue;						
	}
	else if (gp->mode == GM_profile) {	
		if (isBinAttribute(dataIndex)) result = NVTrue;
	}
	
	return result;
}





//
// method:			depthFilterEnabled
//
// This function will check to see if the indices passed in through GraphProps will correspond to the
// attributes within the HOF file that are depth/KGPS parameters that may contain a -998.0.
// There are 12 parameters that meet this criteria.
//

NV_BOOL AVData::depthFilterEnabled (GraphProps * gp, NV_INT32 dataIndex) {

	NV_BOOL result = NVFalse;
	
	if (gp->mode == GM_scatterPlot) {
		if (isDepthAttribute (gp->scatterXIndex)) result = NVTrue;
		else if (isDepthAttribute (gp->scatterYIndex)) result = NVTrue;
	}
	else if (gp->mode == GM_profile) {
		if (isDepthAttribute (dataIndex)) result = NVTrue;
	}
	
	return result;
}



//
// method:			isBinAttribute 
//
// This method will return a true if this attribute is a "bin" or "used" type attribute that do
// not contain indexes of 0 in their waveform description.  A false will be returned otherwise.
//
// NOTE:  THESE ATTRIBUTE ID'S ARE HARDCODED IF ATTRIBUTES ARE ADDED/DELETED YOU MAY NEED TO UPDATE
// THIS PORTION OF CODE FOR PROPER IDS

NV_BOOL AVData::isBinAttribute (NV_INT32 index) {

	NV_BOOL result = false;	
	
	return result;
}


//
// method:			isDepthAttribute 
//
// This method will return a true if this attribute is a depth or KGPS type attribute.  
// A false will be returned otherwise.
//
// NOTE:  THESE ATTRIBUTE ID'S ARE HARDCODED IF ATTRIBUTES ARE ADDED/DELETED YOU MAY NEED TO UPDATE
// THIS PORTION OF CODE FOR PROPER IDS

NV_BOOL AVData::isDepthAttribute (NV_INT32 index) {

	static const int SIM_DEPTH = 0;
	static const int SIM_DEPTH1 = 1;
	static const int SIM_DEPTH2 = 2;
	static const int SIM_KGPS_ELEV = 3;		

	NV_BOOL result = false;	
	
	if (index == SIM_DEPTH || index == SIM_DEPTH1 || index == SIM_DEPTH2 ||
			index == SIM_KGPS_ELEV) {
			result = NVTrue;
	}	
		
	return result;
}



// 
// method:			binIndexFilter
//
// This method will take the GraphProps argument and look to see if the item passed in contains a 0
// as it's value.  If so, it will return a true that the filter caught it.  A false is a legitmate
// non-zero value.
//

NV_BOOL AVData::binIndexFilter (GraphProps * gp, AVDataItem * item, NV_INT32 attIndex) {

	NV_BOOL result = NVFalse;	
	NV_INT32 indices[2], numIndices = 1;

	if (gp->mode == GM_scatterPlot) {
	
		indices[0] = gp->scatterXIndex;
		indices[1] = gp->scatterYIndex;
		numIndices = 2;		
	}
	else if (gp->mode == GM_profile) 
		indices[0] = attIndex;
	
	NV_FLOAT32 value;
	
	for (NV_INT32 i = 0; i < numIndices; i++) {
	
		value = item[0][indices[i]];		
		
		if ((NV_INT32)value == 0) return NVTrue;
	}	
	
	return result;
}



// 
// method:			depthFilter
//
// This method will take the GraphProps argument and look to see if the item passed in contains a -998
// as it's value.  If so, it will return a true that the filter caught it.  A false is a legitmate
// non-zero value.
//

NV_BOOL AVData::depthFilter (GraphProps * gp, AVDataItem * item, NV_INT32 attIndex) {

	NV_BOOL result = NVFalse;	
	NV_INT32 indices[2], numIndices = 1;

	if (gp->mode == GM_scatterPlot) {
	
		indices[0] = gp->scatterXIndex;
		indices[1] = gp->scatterYIndex;
		numIndices = 2;		
	}
	else if (gp->mode == GM_profile) 
		indices[0] = attIndex;
	
	NV_FLOAT32 value;
	
	for (NV_INT32 i = 0; i < numIndices; i++) {
		value = item[0][indices[i]];		
		if (((NV_INT32)(value - 0.5)) == -998) return NVTrue;
	}	
	
	return result;
}


// method:		userDefinedScaleFilter
//
// This method will check to see if the taget value for a linear profile or a scatterplot
// is within the bounds of the user defined scale.  If so, a true is returned; otherwise,
// a false.
//

NV_BOOL AVData::userDefinedScaleFilter (GraphProps * gp, AVDataItem *item, NV_INT32 attIndex) {

    NV_BOOL result = NVFalse;
	
	if (gp->scale != GS_User_Defined) return result;
	
	if (gp->mode == GM_scatterPlot) {
	
		if ((item[0][gp->scatterXIndex] < gp->userScatXMinScale) ||
			(item[0][gp->scatterXIndex] > gp->userScatXMaxScale))
			return NVTrue;
			
		if ((item[0][gp->scatterYIndex] < gp->userScatYMinScale) ||
			(item[0][gp->scatterYIndex] > gp->userScatYMaxScale))
			return NVTrue;		
	}
	else if (gp->mode == GM_profile) {
	
		if ((item[0][attIndex] < gp->userMinScale) ||
			(item[0][attIndex] > gp->userMaxScale))
			return NVTrue;
	}
		
	return result;
}


//
// method:		IsLoaded
//
// This method simply returns a true/false indicating whether the current data received
// has been loaded.
//

NV_BOOL AVData::IsLoaded() { return isLoaded; }

//
// operator:		[]
//
// This overload returns an AVDataItem for an individual shot that is referenced by the
// index within the brackets.
//

AVDataItem &AVData::operator[] ( NV_INT32 i ) { return *items[ (i>=numItems || i<0)? numItems-1 : i ]; }


//
// function:		CompareAVDataItemsNorthingEasting
//
// This function is a sorting comparision function that takes two items and returns a -1
// for less than, 0 for equal, or 1 for greater than.  Since we are sorting by northing
// then easting, we compare latitudes of the two objects and if item1 is more northern
// then we return a -1.  If for some reason, they are equal in latitude, then compare
// longitudes and take the most eastern one.
//

int CompareAVDataItemsNorthingEasting (const void * item1, const void * item2) {

	// order by lat descending longitude ascending
	const AVDataItem ** i1 = (const AVDataItem **) item1;
	const AVDataItem ** i2 = (const AVDataItem **) item2;
	static NV_FLOAT32 latDiff, longDiff;

	latDiff = (*i1)->latitude - (*i2)->latitude;

	if (latDiff != .0f ) return (latDiff <0)? 1: -1;

	longDiff = (*i1)->longitude - (*i2)->longitude;

	return (longDiff < .0f)? -1: (longDiff > .0f)? 1 : 0;
}


//
// method:		Load
//
// This method will run through the shots and take a look at the attribute values.  Next
// based on the histogram intervals, a count tally for each interval is maintained and
// the histogram calculations are resolved.
//

void AVHistItem::Load( NV_FLOAT32 histInterval, NV_INT32 dataIndex, AVDataItem **items, NV_FLOAT32 min, NV_FLOAT32 max, NV_INT32 numItems ) {

	if (isLoaded) return; // prevent unnecessary loading 
	
	this->histInterval = histInterval;

	static NV_FLOAT32 range; range = max - min;
	numIntervals = (int)ceil( range / histInterval );
	if (numIntervals==0) numIntervals = 1; // make sure at least one interval
	
	static NV_FLOAT32 adj; adj = ((numIntervals * histInterval) - range ) / 2.0f;
	minScale = min - adj;
	maxScale = max + adj;


	if ( counts ) free(counts);
	counts = (NV_INT32 *)calloc(numIntervals+1, sizeof(NV_INT32) ); // depending on zeroing.  add 1 extra for NULLS

	// perform counts for each interval
	for (int iidx=0; iidx < numItems; iidx++)  		
		
		if ( (NV_INT32)((*items[iidx])[dataIndex] - 0.5f) == -998) counts[numIntervals]++; // NULL count		
		else 
			for (int i = 0; i < numIntervals; i++) 
			
				if(((*items[iidx])[dataIndex] >= minScale + i*histInterval && 
						(*items[iidx])[dataIndex] < minScale + (i+1)*histInterval) ) { 
					counts[i]++; 
					break; 
				}

	// calc max scale value for counts (multiple of 10)

	static int maxCount; maxCount = counts[0];
	for (int i=1; i<=numIntervals; i++) if (counts[i]>maxCount) maxCount = counts[i];
	countMaxScale = 10 * (int)floor( maxCount/10.0f ) + 10;

	isLoaded = NVTrue;	
}

//
// method:		TagForReload
//
// This method simply intializes the isLoaded flag for a histogram data item.
//

void AVHistItem::TagForReload() { isLoaded = NVFalse; }


//
// self explanatory accessor functions for AVHistItem
//

NV_INT32 AVHistItem::GetNumIntervals() { return numIntervals; }
NV_INT32 * AVHistItem::GetCounts() { return counts; }
NV_FLOAT32 AVHistItem::GetScaleMin() { return minScale; }
NV_FLOAT32 AVHistItem::GetScaleMax() { return maxScale; }
NV_INT32 AVHistItem::GetCountScaleMax() {return countMaxScale; }


//
// construct:		GraphProps
//
// This constructore will intialize fonts, font metrics, drawing dimensions, and margin
// space.
//

GraphProps::GraphProps() {

	font = QFont("Charter", 12);
	smFont = QFont("Charter", 8);

	fm = new QFontMetrics(font);
	smFm = new QFontMetrics(smFont);

	lblPixSpacing = 5;
	pt12Height = fm->height();
	pt8Height = smFm->height();

	dataScaleLblW=smFm->width("XXXXXXXX"); 
	leftTickW = 4;
	bottomTickH = 3;

	circleDiameter = 3;
	highlightCircleDiameter = 7;

	leftMargin = lblPixSpacing * 3 + pt12Height + dataScaleLblW + leftTickW;
	bottomMargin = lblPixSpacing * 2 + pt12Height * 2 + bottomTickH;
	topMargin = pt12Height;
	rightMargin = pt12Height;
}


//
// deconstruct:		~GraphProps
//
// Fonts are deleted from memory here.
//

GraphProps::~GraphProps() {
	delete fm;
	delete smFm;
}


//
// method:		SetScaling
//
// This method simply sets some data members and calculates a scale range.
//

void GraphProps::SetScaling(GraphScale scale, NV_FLOAT32 minScale, NV_FLOAT32 maxScale  ) { 

	this->scale = scale;
	this->minScale = minScale; 
	this->maxScale = maxScale; 
	scaleRange = maxScale - minScale;
}


//
// method:		SetGraphMode
//
// This method will set the graph mode data member, call to have the axis labels updated, 
// and calculate spacing for shot intervals for profiling mode.
//

void GraphProps::SetGraphMode( enum GraphMode mode, QComboBox * cbAOI, NV_INT32 numItems ) {

	this->mode = mode;

	SetGraphLabels(cbAOI); 
	
	if ( mode == GM_profile ) CalcSpacing( numItems);
}


//
// method:		SetGraphLabels
//
// This method will set the labels depending on the type of graphing mode we are in.
//

void GraphProps::SetGraphLabels(QComboBox * cbAOI) {
	
	/* for scatterplot, use the scatter indices for labels */

	QString selectedAttr = cbAOI->currentText();

	bottomLbl=(mode == GM_profile) ? "Shots" 
					: (mode == GM_scatterPlot) ? (QString)(cbAOI->itemText (scatterXIndex))
					: "Values";	

	bottomLblW = fm->width(bottomLbl);

	this->leftLbl = (mode == GM_histogram) ?  attributeViewer::tr("Counts") 
						: (mode == GM_profile && overplot) ? attributeViewer::tr("Intensities") 
						: (mode == GM_profile && !overplot) ? selectedAttr
						: cbAOI->itemText (scatterYIndex); 

	leftLblW = fm->width (leftLbl);
}


//
// method:		CalcResizableDims
//
// This method will store the updated dimensions from a resize or window creation. Margins
// are recalculated meaning spacings need to be recalculated.
//

void GraphProps::CalcResizableDims(NV_INT32 graphWidth, NV_INT32 graphHeight, NV_INT32 numShots) {

	w = graphWidth;
	h = graphHeight;	
	
	// update left and right margin with 2% of width view added in on each side
	
	leftMargin = (NV_INT32) ((w * 0.02f) + lblPixSpacing * 3 + pt12Height + dataScaleLblW + leftTickW);
	rightMargin = (NV_INT32) ((w * 0.02f) + pt12Height);

	drawHeight = h - bottomMargin - topMargin;
	drawWidth  = w - leftMargin   - rightMargin;

	if ( mode == GM_profile ) CalcSpacing( numShots );
}

//
// method:		CalcSpacing
//
// This method will divide the drawable canvas width by numshots - 1 to find the lateral
// spacing between shots.
//

void GraphProps::CalcSpacing( NV_INT32 numShots) {

	shotSpacing = (numShots <= 1) ? .0f : drawWidth / (numShots - 1.0f);	
}


//
// construct:		PolySelection
//
// This constructor initializes the size and vertices to 0 and the screen coordinate
// arrays to NULL.
//

PolySelection::PolySelection() {
	verts = size = 0;
	xs = ys = NULL;
}


//
// deconstruct:		~PolySelection
//
// This deconstructor simpy frees up the polygon vertices and screen coordinate arrays.
//

PolySelection::~PolySelection() { clear(); } 


//
// method:		draw
//
// This method will loop through the vertices drawing connected lines through the screen
// coordinate array.
//

void PolySelection::draw(nvMap *map) {

	for(int i=0, j=verts-1; i<verts; j=i++) map->drawLine (xs[i], ys[i], xs[j], ys[j], Qt::white, 2, NVFalse, Qt::SolidLine);
}


//
// method:		addVertex
//
// This method will add a screen coordinate to the PolySelection screen coordinate arrays.
//

void PolySelection::addVertex(NV_INT32 x, NV_INT32 y) {

	if ( verts && xs[verts-1] == x && ys[verts-1] == y ) return; // prevent adding same point twice or more in a row
	if (verts >= size) {
		if ( (xs = (NV_INT32 *)realloc(xs, (size+10) * sizeof(NV_INT32) ) ) == NULL ) return; // simply stop if mem alloc
		if ( (ys = (NV_INT32 *)realloc(ys, (size+10) * sizeof(NV_INT32) ) ) == NULL ) return; // fails
		size+=10;
	}

	xs[verts] = x;
	ys[verts++] = y;

}


//
// method:		update
//
// This method will update the vertices on a timer to form the polygonal selection.
//

void PolySelection::update(NV_INT32 x, NV_INT32 y) {

	static QTime t;

	if ( t.isNull() ) t.start();

	if (t.elapsed() >= 50) {
		addVertex(x,y);
		t.restart();
	}
	else {
		xs[verts-1] = x;
		ys[verts-1] = y;
	}
}


// 
// method:		clear
//
// this method clears all the polygonal vertices from memory.  
//

void PolySelection::clear() {

	verts = size = 0;

	free(xs);
	free(ys);

	xs = ys = NULL;
}
