//
// header:		attributeViewer.hpp
// author:		Gary Morris
//
// This header file will support the Attribute Viewer module which will interface
// to the CZMIL Manual Editor's editing interfaces.  Shots will be received via shared
// memory and the Attribute Viewer will display these shots in attribute form along with
// statistical anaylysis and plotting mechanisms.
//

#ifndef attributeViewer_H
#define attributeViewer_H

// include files

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

#include "nvtypes.h"
#include "pfm.h"
#include "fixpos.h"
#include "pfm_extras.h"
#include "nvmap.hpp"

#include "ABE.h"
#include "version.hpp"

#include "nvutility.h"
#include "attPrefs.hpp"

#include "AxesDlg.hpp"
#include "UserScaleDlg.hpp"

#include "czmil.h"

/* we need vectors for our "closest point on a line" routine and others */
#include "Vector.hpp"
#include "UnitRays.hpp"

#ifdef NVWIN3X
#include "windows_getuid.h"
#endif

#include <QtCore>
#include <QtGui>

// definitions

#define			NUM_ATTRIBUTES		5

/* spatial neighbor table size constants */
#define			MAX_SPATIAL_TABLE_DIM	7
#define			NUM_SPATIAL_TBL_INDICES	 (MAX_SPATIAL_TABLE_DIM * MAX_SPATIAL_TABLE_DIM - 1)

/* our no projection map will have a maximum dimension of 500 */
#define		SCALE_MAX			500

//
// enum:		GraphMode
//
// This enumeration deals with the various plotting methods.
//

enum GraphMode { GM_profile, GM_scatterPlot, GM_histogram} ;

//
// enum:		GraphScale
//
// This enumeration deals with the various scaling methods.
//

enum GraphScale { GS_Absolute, GS_Roi, GS_User_Defined } ;


// forward declaration

class GraphProps;

// function definitions

int CompareAVDataItemsNorthingEasting (const void * item1, const void * item2); 


//
// class:		PolySelection
//
// This class represents a selection that is formed by a connection of vertices
// forming a polygon.  Once the first click starts, vertices are added to the
// selection based on a timer.  The second click will close the polygon.
//

class PolySelection {
	
public:

	NV_INT32 *xs, *ys, verts, lastX, lastY, size;

	PolySelection();
	~PolySelection();

	void draw(nvMap *map);
	void update(NV_INT32 x, NV_INT32 y);
	void addVertex(NV_INT32 x, NV_INT32 y);
	void clear();

};


//
// class:		AVDataItem
//
// This class represents all of the information for a single shot item.  This 
// includes all of the attributes within the CXY record of the shot.  This class
// will expand with the CZMIL library.
//

class AVDataItem {

public:    
	
	NV_FLOAT32 simDepth, simDepth2, simDepth3, simKGPSElev, simWaveformClass;	
	NV_FLOAT64 latitude, longitude;		

	NV_INT32   colorH;
	NV_INT32   colorS;
	NV_INT32   colorV;
	
	/* used to store master index of item in editor */
	NV_INT32 masterIdx;	
	
	/* chl revision 08_25_2011 */
	NV_INT32 recNum;
	/* end revision */

	NV_INT32 indexID;		// needed for if we sort, we can retain the original order to
							// store proper indices to properly highlight or delete
	
	/* physical x y graph coords. stored to prevent necessity of recalc on selection or other events */
	
	/* adding NUM_ATTRIBUTES + 1 to account for the scatterplot screen coordinates */

	NV_INT32 xs[NUM_ATTRIBUTES + 1], ys[NUM_ATTRIBUTES + 1];

	NV_FLOAT32 &operator[] ( NV_INT32 i );	
};


//
// class:		AVHistItem
//
// This class contains all of the information about a histogram for a single 
// attribute.  Therefore, a single AVHistItem refers to a single attribute.
//

class AVHistItem {

private:
	NV_INT32 numIntervals, *counts, countMaxScale;
	NV_FLOAT32 histInterval, minScale, maxScale;
	NV_BOOL isLoaded;

public:
	
	void Load( NV_FLOAT32 histInterval, NV_INT32 dataIndex, AVDataItem **items, NV_FLOAT32 min, NV_FLOAT32 max, NV_INT32 numItems );
	void TagForReload();

	NV_INT32 GetNumIntervals();
	NV_INT32 *GetCounts();
	NV_FLOAT32 GetScaleMin();
	NV_FLOAT32 GetScaleMax();
	NV_INT32 GetCountScaleMax();


};


//
// class:		GraphProps
//
// This class represents the properties of the canvas area where the shots are
// plotted.  Dimensions, labels, margins, fonts, spacing, scales, etc. are
// all kept here to describe how to draw the plot area.
//

class GraphProps {

public:
	GraphProps();
	~GraphProps();
	
	QFont font, smFont;
    QFontMetrics *fm, *smFm;
	QString leftLbl, bottomLbl;

	NV_INT32 
		w, h
		, drawHeight, drawWidth
		, lblPixSpacing, pt12Height, pt8Height
		, leftLblW, bottomLblW, dataScaleLblW 
		, leftMargin, bottomMargin, topMargin, rightMargin
		, leftTickW, bottomTickH		
		, lblInterval // interval in which to label x vals
		, circleDiameter // circle of actual shot plot		
		, highlightCircleDiameter		
		;


	NV_FLOAT64 shotSpacing, minScale, maxScale, scaleRange;
	
	/* these indices represent the attributes making up the axes of the scatterplot */

	NV_INT32			scatterXIndex;
	NV_INT32			scatterYIndex;			
	
	bool overplot, selectPoly;

	/* added graphMode to handle more modes */	
	enum GraphMode mode;
	enum GraphScale scale;	
	NV_FLOAT32 userMinScale, userMaxScale;
	
	NV_FLOAT32 userScatXMinScale, userScatXMaxScale;
	NV_FLOAT32 userScatYMinScale, userScatYMaxScale;

	void SetGraphMode(enum GraphMode graphMode, QComboBox * cbAOI, NV_INT32 numItems);	
	void SetGraphLabels(QComboBox * cbAOI);	
	
	void SetScaling(GraphScale scale, NV_FLOAT32 minScale, NV_FLOAT32 maxScale );
	
	void CalcResizableDims(NV_INT32 graphWidth, NV_INT32 graphHeight, NV_INT32 numShots);
	void CalcSpacing( NV_INT32 numShots);	
};


//
// class:		AVData
//
// This class encapsulates all of the shots with their corresponding information.
// It also contains the statistics that are displayed in the statistics portion
// of the Attribute Viewer.  All of the scaling values, neighboring point cloud
// indices, etc. are kept in this class.
//

class AVData {
private:

	NV_INT32 numItems;
	AVDataItem **items;

	AVDataItem *centerPixel;

	NV_BOOL hitMax, isLoaded;

	NV_FLOAT32 mean, std;

	NV_INT32 spatialDimSize;
	
	/* these are the shot indexes of all of the neighbor tables cells including the center */
	NV_INT32 neighborIdxs[MAX_SPATIAL_TABLE_DIM * MAX_SPATIAL_TABLE_DIM];
		
	/* the reference map which will enable us to perform the spatial slotting algorithm by */
	/* converting lat/lons to y/x screen coordinates */
	nvMap * refMap;
	
	/* allocate NUM_ATTRIBUTES times for the min's and max's */	
	NV_FLOAT32 mins[NUM_ATTRIBUTES], maxs[NUM_ATTRIBUTES], *absmins, *absmaxs; 	
	
	AVHistItem histData[NUM_ATTRIBUTES];
	
	void loadFromCZMIL (AV_SHARE &avShare);
	
	NV_INT32 allocateCZMILShotMemory (AV_SHARE avShare);
	
	void LoadShot(NV_INT32 czmilHnd, AV_SHARE &avShare, NV_BOOL *isLoaded, NV_INT32 czmilIndex, 
					NV_INT32 index, NV_INT32 &numRead );
	
	void updateNeighborTable (AVDataItem ** sortedItems, NV_INT32 * neighbors, NV_INT32 ppX, NV_INT32 ppY);

	AVDataItem ** sortByDistanceFromScreenCoord (NV_INT32 ppX, NV_INT32 ppY);	
	NV_BOOL leadingIndexTaken (NV_INT32 curIndex, NV_INT32 * winners, NV_INT32 numWinners);	
		
	NV_BOOL binFilterEnabled (GraphProps *gp, NV_INT32 dataIndex = -1);
	NV_BOOL binIndexFilter (GraphProps *gp, AVDataItem * item, NV_INT32 attIndex = -1);	
	
	NV_BOOL depthFilterEnabled (GraphProps * gp, NV_INT32 dataIndex = -1);
	NV_BOOL depthFilter (GraphProps * gp, AVDataItem * item, NV_INT32 attIndex = -1);	
	
	NV_BOOL userDefinedScaleFilter (GraphProps * gp, AVDataItem *item, NV_INT32 attIndex = -1);
   	
public:
	
	AVData();
	~AVData();
	
	/* current highlighted master idx may be passed in share at later time
	currently used to calc and pass as deleteIdx when shot is deleted by user */
	NV_INT32 highlightedShotIndex;	
	NV_INT32 *selection, numSelected;

	NV_INT32 GetNumItems();
	NV_FLOAT32 GetMean();
	NV_FLOAT32 GetMin(int dataIndex);
	NV_FLOAT32 GetMax(int dataIndex);
	NV_FLOAT32 GetStd();	
	
	NV_FLOAT32 GetScaleMax( int dataIndex, GraphScale scale, GraphProps * gp, NV_INT32 axis = -1);
	NV_FLOAT32 GetScaleMin( int dataIndex, GraphScale scale, GraphProps * gp, NV_INT32 axis = -1);
	
	AVDataItem &GetCenterPixel( );
	AVHistItem *GetHistItem( int dataIndex );

	NV_BOOL IsLoaded();

	void setReferenceMap (nvMap * map) { refMap = map; }	

	void setSpatialDimSize (NV_INT32 dim) { spatialDimSize = dim; }
	void getTableMapAndMidpoint (NV_INT16 * &tableMap, NV_INT16 &midPoint);
		
	nvMap * getReferenceMap () { return refMap; }
	NV_INT32 * getNeighborIdxs () { return neighborIdxs; }
	
	void Load(AV_SHARE &avShare, int dataIndex, QTableWidget * table, TableStyle tStyle, NV_FLOAT32 histInterval );

	void CalcMinsAndMaxs();
	void CalcXYs( GraphProps &gp, int dataIndex ) ;

	void LoadHistogramData(NV_FLOAT32 histInterval, int dataIndex);
	void TagHistogramDataForReload();	

	void CalcStats(int dataIndex, QTableWidget * table, TableStyle tStyle);
	
	NV_INT32 * lookForNeighbors ();
		
	void select (GraphProps &graphProps, int dataIndex, NV_INT32 count, NV_INT32 *xs, NV_INT32 *ys, NV_BOOL isPoly);
	NV_BOOL inside(NV_INT32 *xs, NV_INT32 *ys, NV_INT32 count, NV_INT32 x, NV_INT32 y, NV_BOOL isPoly );
	void clearSelection();		
	
	NV_BOOL isBinAttribute(NV_INT32 index);
	
	NV_BOOL isDepthAttribute (NV_INT32 index);
	
	// accessor for data items;
	AVDataItem &operator[] ( int i );
};


//
// class:		attributeViewer
//
// This class represents the AV window.  All of the child dialogs are within
// this class as well as interfaces to shared memory, graph properties, pfm
// structures, etc.
//

class attributeViewer:public QMainWindow
{
	Q_OBJECT 


public:

	attributeViewer (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
	~attributeViewer ();


protected:
	
	GraphProps			graphProps;	

	NV_INT32            key, pfm_handle[MAX_ABE_PFMS], num_pfms;

	QSharedMemory       *abeShare;

	ABE_SHARE           *abe_share;
	AVData				avData;
    NV_U_INT32          kill_switch;

	NV_FLOAT32			histInterval;
	QColor				histBarColor;
	QColor				histBarOutlineColor;
	QColor				histNullColor;

	PFM_OPEN_ARGS       open_args[MAX_ABE_PFMS];        

	NVMAP_DEF           graphMapDef;
	nvMap               *graph;
	
	QMenu               *graphContextMenu, *graphSelectionContextMenu;

	NV_INT32            width, height, window_x, window_y;

	/* listing-style table */
	QTableWidget		* shotTable;
	
	QTableWidget		* neighborTable;
	TableStyle			tableStyle;
	nvMap				* referenceMap;

	NV_INT32			spatialDimSize;
		
	QGroupBox			* tableGrpBox;
	
	NV_FLOAT32          tide;

	QPushButton         *bRestoreDefaults; 

	NV_BOOL             force_redraw, lock_track;	

	QButtonGroup        *bGrp;

	QDialog             *prefsD;

	QToolButton         *bQuit, *bPrefs;

	QLabel				*lblCenterPixel, *lblMean, *lblSD, *lblMin, *lblMax
						, *lblCenterPixelCaption, *lblMeanCaption, *lblSDCaption, *lblMinCaption, *lblMaxCaption
						, *lblAOI
						, *lblTrackingOff;
	
	QAction				*scaleAbsoluteAction, *scaleROIAction, *overplotOnAction, *overplotOffAction
						, *modeProfileAction, *modeScatterPlotAction, *modeHistogramAction
						, *selectionPolyAction, *selectionRectAction;
						
	QAction				* scaleUserAction, * scaleProfileDef, * scaleScatterXDef, * scaleScatterYDef;
		
	NV_BOOL				overplotOnEnvIn, selectionPolyEnvIn;
	
	GraphMode			graphModeEnvIn;

	PolySelection		poly;
	
	NV_BOOL             selecting;
	
	NV_INT32			selectIndex;
	
	NV_INT32			rectXs[2], rectYs[2];
	
	QComboBox			*cbAOI; // attribute of interest
	
	// dialog box class that will contain preferences
	AttPrefs			* attPrefs;	
	
	/* dialog for changing the scatterplot axes */
	AxesDlg				* spAxesDlg;
	
	UserScaleDlg		* scaleDlg;
		
	void envin ();
	void envout ();

	void DisplayData(int dataIndex);
	void DisplayLabels(int dataIndex);
	void DrawGraph(NVMAP_DEF graphMapDef);
	
	void DrawGraphLabelsAndLines();	

	void DrawLeftScale(NV_FLOAT32 minScale, NV_FLOAT32 maxScale, int scalePreciesion, int numTicks, int graphHeight, int drawHeight, int leftMargin, int bottomMargin, int leftTickW, int lblPixSpacing, int fontHeight, QFontMetrics fm);
	void DrawBottomScale(NV_FLOAT32 minScale, NV_FLOAT32 maxScale, int numTicks, int lblInterval, int scalePrecision, int drawWidth, int left, int top, int bottomMargin, int tickH, int lblPixSpacing, int fontHeight, QFontMetrics fm);

	void DrawShotsGraph( int dataIndex, int numShots ) ;	
	void DrawScatterplot (NV_INT32 numShots);	
	void DrawSelection();	
	
	void DrawShots( int dataIndex, int numShots, QColor color );
	
	void SaveAVActionToShare( AV_SHARE_ACTION action, NV_INT32 *actionIdxs, NV_INT32 numIdxs );	
	
	void DrawHistogram( int dataIndex );
	
	void AddAttributeScreen(QBoxLayout *parentBox);
	void AddGraph(QBoxLayout *parentBox);
	void AddAOI(QBoxLayout *parentBox, NV_INT32 labelWidth, NV_INT32 valueWidth);
	void AddLabels(QBoxLayout *parentBox, NV_INT32 labelWidth, NV_INT32 valueWidth);

	void InitGraphProps();
	void InitGraphMapDef();
	void CalcHighlightedIndex(NV_INT32 mouseX, NV_INT32 mouseY);
	
	NV_FLOAT64 calcDist(int x1, int y1, int x2, int y2);
	void SetScaling();
	
	void closeEvent (QCloseEvent *);

	void graphRightMousePress (QMouseEvent *e);
	
	void graphLeftMousePress (QMouseEvent *e);
	
	void rectClick(QMouseEvent *e);
	void polyClick(QMouseEvent *e);
	
	void polyMouseMove(NV_INT32 x, NV_INT32 y);
	void rectMouseMove(NV_INT32 x, NV_INT32 y);

	void clearSelection();
	
	void graphModeAction();

	void addShotTable ();
	
	void addNeighborTable ();
	void setupReferenceMap ();
	
	void populateNeighborTable (NV_INT32 * neighbors);
	
	NV_BOOL isShotFiltered (AVDataItem * item, int attIndex);
		
	NV_BOOL isAttributeLinkable (NV_INT32 index, NV_INT32 &resultNameIndex);
	
	void setUserDefinedActionDefs ();
		
	protected slots:
		
		void slotGraphResize (QResizeEvent *e __attribute__ ((unused)) );
		
		void trackCursor ();

		void slotHelp ();

		void slotQuit ();

		void slotPrefs ();
		
		void slotClosePrefs ();

		void slotRestoreDefaults ();

		void slotHalfSecond();


		void slotCBAOICurrentIndexChanged(int index);
		void slotDrawGraph(NVMAP_DEF mapdef);
		
		void slotGraphOverplotAction();	
		void slotGraphSelectionAction();
		void slotGraphScaleAction();
		void slotGraphModeAction();
		
		void slotChangeUserDefinedScales ();
				
		void slotGraphMouseMove (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)));
		
		void slotGraphMousePress (QMouseEvent *e, NV_FLOAT64 lon __attribute__ ((unused)), NV_FLOAT64 lat __attribute__ ((unused)));
		
		void slotGraphDelete();		
		
		void slotGraphDeleteAll();
		void slotGraphCancel();
		
		void slotHistogramIntervalChanged (double histInterval);
		void slotHistBarColorChanged();
		void slotHistBarOutlineColorChanged();
		void slotHistNullColorChanged();		
		void slotChangeScatterAxes ();		
		void slotSetAxesIndices (NV_INT32 xState, NV_INT32 yState);		

		void slotShotTblHighlighted (QTableWidgetItem * item);
		
		void slotNeighborTblHighlighted (QTableWidgetItem * item);
		
		void slotShotItemSelectionChanged (int row, int col);		
		
		void slotTableStyleClicked ();

		void slotSpatialNeighborSizeChanged (int index);
				
		void slotUserScaleDefined (NV_FLOAT32 min, NV_FLOAT32 max,
									NV_FLOAT32 min2, NV_FLOAT32 max2,
									NV_FLOAT32 min3, NV_FLOAT32 max3);
				
		void about ();
		void aboutQt ();

private:

};

#endif
