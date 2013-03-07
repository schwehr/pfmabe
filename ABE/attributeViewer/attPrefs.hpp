//
// header:		attPrefs.hpp
// author:		Gary Morris
//
// This header file contains the class, structures, and definitions that support
// a Preferences dialog for the Attribute Viewer module.
//


#ifndef ATT_PREFS_H
#define ATT_PREFS_H

// include files

#include <QtCore>
#include <QtGui>

#include "nvutility.h"

/* we have 2 different table formats:  a simple listing of all values within the Attribute */
/* Viewer and a 3 x 3 spatial neighboring table */

enum TableStyle { LISTING = 0, NEIGHBOR = 1 };


// 
// struct:		AttPrefSettings
//
// All settings that we want to retain as a history are within this structure
//

struct AttPrefSettings {

	NV_FLOAT32	histInterval;
	QColor		histBarColor, histBarOutlineColor, histNullColor;

	/* we record the style of the table*/
	TableStyle	tStyle;	
};


//
// class:		AttPrefs
//
// This class inherits the QDialog class and contains all of the widgets with
// their respective signal/slot representations.
//

class AttPrefs : public QDialog {

  Q_OBJECT 


    public:

	AttPrefs (QWidget *parent = 0);
	~AttPrefs ();

	QDoubleSpinBox *histIntSpin;
	QPushButton *pbHistBarColor, *pbHistBarOutlineColor, *pbHistNullColor;
	
	QComboBox * sizeCombo;
	NV_INT32 sizeComboSelect;
	
	/* two radio buttons within the Preferences dialog */
	QRadioButton * listingBtn;
	QRadioButton * neighborBtn;	

	signals:

	void spatialNeighborSizeSig (int index);

	private:

	AttPrefSettings settings;

	protected:

	void closeEvent (QCloseEvent * event);
	void AddLayouts (void);
	void RecallSettings (AttPrefSettings &settings);
	void SaveSettings (AttPrefSettings settings);

	void CreateColorButton(QPushButton *&pb, QString caption, QString toolTip, QColor color);
	void SetColorButtonColor( QPushButton *pb, QColor color);

	protected slots:

	void slotClosePrefs ();
	void slotHistIntervalChanged (double histInterval);
	void slotColorButtonClicked();

	/* slot for the table style radio buttons */
	void slotTableStyleClicked (int style);	

	void slotSpatialNeighborSizeChanged (int index);	
};

#endif
