/*  CZMILwaveMonitor class definitions.  */

#ifndef CZMILwaveMonitor_H
#define CZMILwaveMonitor_H

using namespace std;

#include <stdio.h>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include <cmath>

// included for file open and save

#include <QFileDialog>

#include "nvtypes.h"
#include "pfm.h"
#include "fixpos.h"
#include "pfm_extras.h"
#include "nvmap.hpp"

#include "ABE.h"
#include "version.hpp"

#include "czmil.h"

#ifdef NVWIN3X
    #include "windows_getuid.h"
#endif

#include <QtCore>
#include <QtGui>

// channel ID's

#define CZMIL_DEEP			0
#define CZMIL_SHALLOW1		1
#define CZMIL_SHALLOW2		2
#define CZMIL_SHALLOW3		3
#define CZMIL_SHALLOW4		4
#define CZMIL_SHALLOW5		5
#define CZMIL_SHALLOW6		6
#define CZMIL_SHALLOW7		7
#define CZMIL_IR			8

// axis bounds (time/count)

#define LOWER_TIME_BOUND	1
#define	UPPER_TIME_BOUND	640

#define LOWER_COUNT_BOUND	0
#define	UPPER_COUNT_BOUND	1023

#define WAVE_X_SIZE         600
#define WAVE_Y_SIZE         270
#define SPOT_SIZE           2

// possible definitions for color codes

#define CC_BY_CURSOR		0
#define CC_BY_FLIGHTLINE	1

#define NUMSHADES			100

#define NUM_WAVEFORMS		9

#define WAVE_ALPHA          64                 // alpha value applied to some colors for the waveforms
#define LBL_PIX_SPACING     5                  // arbitrary buffer space for legend between pane and axis
#define MAX_HISTORY_ENTRIES 100                // number of maximum entries for hash table

typedef struct
{
  NV_INT32            min_x;
  NV_INT32            max_x;
  NV_INT32            min_y;
  NV_INT32            max_y;
  NV_INT32            range_x;
  NV_INT32            range_y;
  NV_INT32            length;
} BOUNDS;


struct SavedSettings {
	
	bool visible[NUM_WAVEFORMS];		
	QString saveDirectory, openDirectory;
};


class CZMILwaveMonitor:public QMainWindow
{
  Q_OBJECT 


public:

  CZMILwaveMonitor (NV_INT32 *argc = 0, NV_CHAR **argv = 0, QWidget *parent = 0);
  ~CZMILwaveMonitor ();


protected:
  
  NV_INT32            key, pfm_handle[MAX_ABE_PFMS], num_pfms;
  NV_U_INT32          kill_switch;

  QSharedMemory       *abeShare;        
  PFM_OPEN_ARGS       open_args[MAX_ABE_PFMS];   
  
  BOUNDS			  chanBounds[NUM_WAVEFORMS];
  
  NV_INT32            width, height, window_x, window_y;
 
  NV_CHAR             wave_path[512], filename[512];
  NV_INT32            flightline, recnum;  
  
  CZMIL_CXY_Data	  czmil_record;  
  
  NV_BOOL             wave_line_mode, startup_message;   

  NV_BOOL             showIntMeter;        
  NV_INT32            sigBegin;                 
  NV_INT32            sigEnd;  
  
  // we made the binHighlighted and scaleBinHiglighted class variables arrays corresponding with
  // the sensor panes.  axisPixel* arrays indicate pixel positions of the beginning and end of the
  // time axis for the intensity meter marker.

  NV_INT32			  axisPixelBegin[NUM_WAVEFORMS];
  NV_INT32			  axisPixelEnd[NUM_WAVEFORMS];  

  NV_INT32            binHighlighted[NUM_WAVEFORMS];
  NV_INT32            scaledBinHighlighted[NUM_WAVEFORMS];    

  // we need a variable to hold the current color coding mode

  NV_INT32			  ccMode;    
  
  QString             binTxtLbl;  
  QString			  chanTxtLbl[NUM_WAVEFORMS];
  
  NV_INT32            pt12Height, pt8Height, verLblSpace, pt12Width, gridWidth, horLblSpace, timeWidth, countsWidth;

  // need a width for potential reference file label as well as the label itself

  QString			  refFileLbl;
  NV_INT32			  refFileWidth;   
  
  NV_INT32            active_window_id;
  QMenu               *contextMenu;
  NV_BOOL             zoomFlag;
  NV_BOOL             zoomIsActive;
  nvMap               *zoomMap;
  QPoint              zoomFirstPos;
  QPoint              zoomSecondPos;
  
  CZMIL_CWF_Data	  wave_data;
    
  NV_INT32            wave_read;		
  
  /* chl revision 08_18_2011 */
  //NV_INT16			  ***referenceData;  
  NV_U_INT16			  ***referenceData;  
  /* end revision */
  
  NV_BOOL							drawReference;
  int								numReferences;	 
	
  // addition of reference color

  QColor			  referenceColor;

  QMessageBox         *filError;

  QCheckBox           *sMessage;

  QStatusBar          *statusBar;

  QColor              surfaceColor, primaryColor, secondaryColor, backgroundColor;

  QPalette            bSurfacePalette, bPrimaryPalette, 
                      bSecondaryPalette, bBackgroundPalette;

  QPushButton         *bSurfaceColor, *bPrimaryColor, *bSecondaryColor, *bBackgroundColor,
                      *bRestoreDefaults; 

  // adding a button and palette for the Prefs dialog for the reference mark color

  QPushButton		  * bReferenceColor;
  QPalette			  bReferencePalette;  

  QAction             *nearAct, *noneAct;
  
  NV_BOOL             force_redraw, display_wave[NUM_WAVEFORMS], lock_track;
  
  NV_BOOL			  nonFileIO_force_redraw;
  
  nvMap               *map[NUM_WAVEFORMS];
  
  QColor              waveColor;

  NVMAP_DEF           mapdef;
  NV_U_INT32          ac[8];

  QButtonGroup        *bGrp;

  QDialog             *prefsD;

  QToolButton         *bQuit, *bPrefs, *bMode;

  QLabel              *nameLabel, *simDepth, *simDepth1, *simDepth2, *simKGPSElev;  

  QString             sim_depth;

  QString             db_name, date_time, sim_kgps_elev, sim_depth1;

  QString             sim_depth2;  

  QToolButton         *bIntMeter;

  ABE_SHARE           *abe_share, l_abe_share;
  NV_INT32            wave_read_multi[MAX_STACK_POINTS];
  
  NV_CHAR             czmil_path_multi[MAX_STACK_POINTS][512];
   
  NV_CHAR             line_name_multi[MAX_STACK_POINTS][128];
  NV_CHAR             wave_path_multi[MAX_STACK_POINTS][512];  
  
  NV_INT32            recnum_multi[MAX_STACK_POINTS];  
  
  CZMIL_CXY_Data      czmil_record_multi[MAX_STACK_POINTS]; 
    
  CZMIL_CWF_Data      wave_data_multi[MAX_STACK_POINTS]; 

  QColor              waveColorNear, primaryColorNear;
  QColor              secondaryColorNear, transWave2ColorNear;
  QColor              transPrimaryColorNear, transWaveColorNear;
  QColor              transSecondaryColorNear, wave2ColorNear;

  NV_INT32            multiWaveMode;
  
  NV_INT32            lTimeBound[NUM_WAVEFORMS];
  NV_INT32            uTimeBound[NUM_WAVEFORMS];
  NV_INT32            lCountBound[NUM_WAVEFORMS];
  NV_INT32            uCountBound[NUM_WAVEFORMS];

  // array for sensor maps denoting them being blocked or not
  
  NV_BOOL			  mapBlocked[NUM_WAVEFORMS];
    
  // color array

  QColor			  colorArray[NUMSHADES * 2];

  // Right now, SavedSettings is a structure containing the pane visibilities of the sensors

  SavedSettings		  savedSettings;  

  QString             pos_format, parentName;

  // action items that need to be class variables because we will be enabling/disabling based
  // on user interaction.

  QAction * toggleReference, * clearReference; 

  void envin ();
  void envout ();

  void leftMouse (NV_INT32 x, NV_INT32 y, nvMap * map);
  void midMouse (NV_FLOAT64 x, NV_FLOAT64 y);

  void rightMouse (NV_INT32 x, NV_INT32 y, nvMap * l_map);

  void scaleWave (NV_INT32 x, NV_INT32 y, NV_INT32 *new_x, NV_INT32 *new_y, NV_INT32 type, NVMAP_DEF l_mapdef);
  void closeEvent (QCloseEvent *);

  void drawX (NV_INT32 x, NV_INT32 y, NV_INT32 size, NV_INT32 width, QColor color, nvMap *);
	
  // separated drawing code out slotplotwave

  /* chl revision 08_18_2011 */
  
  // void drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, NV_INT16 * dat, 
						// NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor );

  /* chl revision 08_22_2011 */
  
   void drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, NV_U_INT16 * dat, 
						 NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor );
						
  void drawSingleWave(nvMap *l_map, NVMAP_DEF &l_mapdef, CZMIL_CWF_Data * wData, 
						NV_INT32 mapIndex, NV_INT32 mapLength, QColor waveColor );
						
  /* end revision */
						
  /* end revision */
  
  /* chl revision 08_22_2011 */
  void getWaveformData (NV_INT32 mapIndex, NV_U_INT16 * &data, NV_U_BYTE * &ndx, CZMIL_CWF_Data * wData);
  NV_BOOL trimWaveformNdxs (NV_U_BYTE * ndx, NV_INT32 &start, NV_INT32 &end);
  /* end revision */
    
  void setFields ();
  
  void setZoomBounds ( NV_INT32 mapIndex, NV_INT32 zoomLength, NV_INT32 timeBin, NV_INT32 timeBin2, NV_INT32 timeBin3, NV_INT32 timeBin4  );
 	
	void writeWaveformHeader(ofstream &outFile, QString &fileName);
	void writeWaveformData (ofstream &outFile);
	void writeSingleTableEntry (ofstream &outfile, NV_INT32 timeIndex);
	void writeNNTableEntry (ofstream &outfile, NV_INT32 timeIndex);

	void loadReferenceData(QTextStream &fileDat, int numCols);
	void allocateReferenceData(int numWaves);
	void clearReferenceData();

protected slots:

  void slotMousePress (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);
  void slotMouseMove (QMouseEvent *e, NV_FLOAT64 x, NV_FLOAT64 y);

  void slotResize (QResizeEvent *e);
  
  // This slot catches the pane display toggles of the menu via a lone signal mapper

  void slotPaneToggle (QObject *); 
  
  // slots for the color coding menu options

  void slotColorByCursor (void);
  void slotColorByFlightline (void);  

  void slotPlotWaves (NVMAP_DEF l_mapdef);

  void trackCursor ();

  void slotKeyPress (QKeyEvent *e);

  void slotHelp ();

  void slotQuit (); 

  void slotMode ();  
  void slotIntMeter ();
  void slotNear ();
  void slotNone();
  void slotZoomIn();
  void slotResetZoom();	
	
  // slot to be call from right contextMenu's resetZoom Action
  // different from slotResetZoom in that this function only
  // reset zoom for map over whick right click occurs

  void slotContextResetZoom(); 

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();
	
	// Added slot for opening and saveing waveform files
	
	void slotOpen();
	void slotSave();	
	
	// slots for the Reference marks menu options and color button in Prefs dialog

	void slotToggleReference (bool toggleStatus);
	void slotClearReference ();
	void slotReferenceColor ();

  void slotSurfaceColor ();
  void slotPrimaryColor ();
  void slotSecondaryColor ();
  void slotBackgroundColor ();
  void slotRestoreDefaults ();
  
  void about ();
  void aboutQt ();

  void resetBounds(NV_INT32 mi, NV_INT32 mapLength);

 private:  

  void clampToLegend(NV_INT32, NV_INT32&, NV_INT32&, NV_INT32&, NV_INT32&);
  void GetLabelSpacing (void);
  void InsertGridTicks (NV_INT32, NV_INT32, NV_CHAR, NV_INT32, NV_INT32, NV_INT32, nvMap *);
  void storeZoomBounds(nvMap*, NV_INT32, NV_INT32);
  void processMultiWaves (ABE_SHARE);
  void drawMultiWaves (nvMap *, NVMAP_DEF);
  
  /* chl revision 08_24_2011 */
  //void drawMultiGrid  (NV_INT16 wfData[][64], NVMAP_DEF mapDef, nvMap * map);
  void drawMultiGrid  (CZMIL_CWF_Data wfData[], NVMAP_DEF mapDef, nvMap * map);
  /* end revision */
    
  void GetTickSpacing (NV_INT32, NV_INT32, NV_INT32, NV_INT32, NV_INT32 &, NV_INT32 &);
  QPoint *loadRectangleCoords(QPoint);

  // Function that parses out the gps time from the .hof file for the waveform data write

  QString RetrieveGPSTime (int index);

  // A much needed function that just returns the map type (PMT, APD, TBDD, RAMAN, or IR).
  // This helps us not to duplicate so much code.

  NV_INT32 GetSensorMapType (nvMap *);

	// variable used to store current zooming pane for zoom out purposes
	// used in rightMouse and slotContextResetZoom

	nvMap * zoomOutMap;
};

#endif
